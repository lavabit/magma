#!/usr/bin/python
# encoding: utf-8
'''
Created on Apr 2, 2015

@author: ladar
'''

import os
import sys
import base64
import struct
import getopt
import binascii
import operator
from Crypto.Random import get_random_bytes
from Crypto.Hash import SHA512
from Crypto.Hash.HMAC import HMAC
import warnings
with warnings.catch_warnings():
    warnings.filterwarnings("ignore",category=DeprecationWarning)
    from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
    from cryptography.hazmat.backends import default_backend

def PrintUsage():
    print ("Usage: stacie.py [--hex|--help] username password [salt]")

def base64url_encode(binary):
    # Encodes a string using the standard base64 method, and
    # converts the output into the proper format.

    # Encode the binary input using standard base64 method.
    output = base64.b64encode(binary)

    # Swap ‘+‘ (plus) with ‘-‘ (minus).
    output = output.replace('+', '-')

    # Swap ‘/‘ (slash) with ‘_‘ (underscore).
    output = output.replace('/', '_')

    # Remove the padding ‘=‘ (equal).
    output = output.replace('=', '')

    # Remove line breaks and other whitespace.
    return output.join((output.split()))

def base64url_decode(string):
    # Converts the string into the standard base64 format, and
    # then uses the standard method to convert the string.

    # Swap ‘-‘ (minus) with ‘+‘ (plus).
    string = string.replace('-', '+');

    # Swap ‘_‘ (underscore) with ‘/‘ (slash).
    string = string.replace('_', '/');

    # Determine if padding must be appended to the string.
    if operator.mod(len(string), 4) == 3:
        string = string + "="
    elif operator.mod(len(string), 4) == 2:
        string = string + "=="

    # Finally, convert the string using a standard base64 decoder.
    return base64.b64decode(string)

def hex_encode(binary):
    # Encodes a string using the standard base64 method, and
    # converts the output into the proper format.

    # Encode the binary input using standard base64 method.
    output =  binascii.hexlify(binary)

    # Remove line breaks and other whitespace.
    return output.join((output.split()))

def CalculateHashRounds(password, bonus):
    # Accepts a user password and bonus value, and calculates
    # the number of iterative rounds required. This function will
    # always return a value between 8 and 16,777,216.

    # Identify the number of Unicode characters.
    characters = len(password.decode("utf-8"))

    # Calculate the difficulty exponent by subtracting 1
    # for each Unicode character in a password.
    dynamic = operator.sub(24, characters)

    # Use a minimum exponent value of 1 for passwords
    # equal to, or greater than, 24 characters.
    dynamic = max(1, dynamic)

    # Derive the variable number of rounds based on the length.
    # Raise 2 using the dynamic exponent determined above.
    variable = pow(2, dynamic)

    # If applicable, add the fixed number of bonus rounds.
    total = operator.add(variable, bonus)

    # If the value of rounds is smaller than 8, reset
    # the value to 8.
    total = max(8, total)

    # If the value of rounds is larger than 16,777,216, reset
    # the value to 16,777,216.
    total = min(pow(2, 24), total)

    return total

def ExtractEntropySeed(rounds, username, password, salt=None):
    # Concentrates and then extracts the random entropy provided
    # by the password into a seed value for the first hash stage.

    # If if an explicit salt value is missing, use a hash of
    # the username as if it were the salt.
    if salt is None:
        salt = SHA512.new(username).digest()

    # Confirm the supplied salt meets the minimum length of 64
    # octets required, is aligned to a 32 octet boundary and does not
    # exceed 1,024 octets. Some implementations may not handle salt
    # values longer than 1,024 octets properly.
    elif len(salt) < 64:
        raise ValueError("The salt, if supplied, must be at least " \
          "64 octets in length.")
    elif operator.mod(len(salt), 32) != 0:
        warnings.warn("The salt, if longer than 64 octets, should " \
          "be aligned to a 32 octet boundary.")
    elif len(salt) > 1024:
        warnings.warn("The salt should not exceed 1,024 octets.")

    # For salt values which don't match the 128 octets required for
    # an HMAC key value, the salt is hashed twice using a 3 octet
    # counter value of 0 and 1, and the outputs are concatenated.
    if len(salt) != 128:
        key = \
            SHA512.new(salt + struct.pack('>I', 0)[1:4]).digest() + \
            SHA512.new(salt + struct.pack('>I', 1)[1:4]).digest()
    # If the supplied salt is 128 octets use it directly as the key value.
    else:
        key = salt

    # Initialize the HMAC instance using the key created above.
    hmac = HMAC(key, None, SHA512)

    # Repeat the plaintext password successively based on
    # the number of instances specified by the rounds variable.
    for unused in range(0, rounds):
        hmac.update(password)

    # Create the 64 octet seed value.
    seed = hmac.digest()

    return seed

def HashedKeyDerivation(seed, rounds, username, password, salt=""):

    count = 0
    hashed = ""

    while count < rounds:
        hashed = SHA512.new(hashed + seed + username + salt + password + struct.pack('>I', count)[1:4]).digest()
        count = operator.add(count, 1)

    return hashed

def HashedTokenDerivation(seed, username, salt="", nonce=""):

    count = 0
    rounds = 8
    hashed = ""

    # Confirm the supplied nonce meets the minimum length of 64
    # octets, does not exceed 1,024 octets, and is aligned
    # along a 32 octet boundary. Implementations may not handle
    # nonce values larger than 1,024 octets properly.
    if len(nonce) > 0 and len(nonce) < 64:
        raise ValueError("Nonce values must be at least " \
          "64 octets in length.")
    elif operator.mod(len(nonce), 32) != 0:
        warnings.warn("The nonce value, if longer than 64 octets, " \
          "should be aligned to a 32 octet boundary.")
    elif len(nonce) > 1024:
        warnings.warn("The nonce should not exceed 1,024 octets.")

    while count < rounds:
        hashed = SHA512.new(hashed + seed + username + salt + nonce + struct.pack('>I', count)[1:4]).digest()
        count = operator.add(count, 1)

    return hashed

def RealmKeyDerivation(master_key, realm="", shard=""):

    if len(realm) < 1:
        raise ValueError("The realm label is missing or invalid.")
    elif len(shard) != 64:
        raise ValueError("The shard length is not 64 octets.")
    elif len(master_key) != 64 != 64:
        raise ValueError("The master key length is not 64 octets.")

    hashed = SHA512.new(master_key + realm + shard).digest()
    realm_key = str().join(chr(operator.xor(ord(a), ord(b))) \
        for a,b in zip(hashed, shard))

    return realm_key

def ExtractRealmVectorKey(realm_key):
    realm_vector_key = realm_key[0:16]

    return realm_vector_key

def ExtractRealmTagKey(realm_key):
    realm_tag_key = realm_key[16:32]

    return realm_tag_key

def ExtractRealmCipherKey(realm_key):
    realm_cipher_key = realm_key[32:64]

    return realm_cipher_key

def RealmEncrypt(vector_key, tag_key, cipher_key, buffer, serial=0):

    count = 0

    if serial < 0 or serial >= pow(2, 16):
        raise ValueError("Serial numbers must be greater than 0 " \
            "and less than 65,536.")
    elif len(cipher_key) != 32:
        raise ValueError("The encryption key must be 32 octets in " \
            "length.")
    elif len(vector_key) != 16:
        raise ValueError("The vector key must be 16 octets in length.")
    elif len(buffer) == 0:
        raise ValueError("The secret being encrypted must be at " \
            "least 1 octet in length.")
    elif len(buffer) >= pow(2, 24):
        raise ValueError("The secret being encrypted must be at " \
            "less than 16,777,216 in length.")

    vector_shard = get_random_bytes(16)

    iv = str().join(chr(operator.xor(ord(a), ord(b))) \
        for a,b in zip(vector_key, vector_shard))

    size = len(buffer)
    pad = (16 - operator.mod(size + 4, 16))

    while count < pad:
        buffer += struct.pack(">I", pad)[3:4]
        count = operator.add(count, 1)

    encryptor = Cipher(algorithms.AES(cipher_key), modes.GCM(iv), \
        backend=default_backend()).encryptor()
    ciphertext = encryptor.update(struct.pack(">I", size)[1:4] \
        + struct.pack(">I", pad)[3:4] + buffer) \
        + encryptor.finalize()

    tag_shard = str().join(chr(operator.xor(ord(a), ord(b))) \
        for a,b in zip(tag_key, encryptor.tag))

    return struct.pack(">H", serial) + vector_shard + tag_shard \
        + ciphertext

def RealmDecrypt(vector_key, tag_key, cipher_key, buffer):

    count = 0

    # Sanity check the input values.
    if len(cipher_key) != 32:
        raise ValueError("The encryption key must be 32 octets in length.")
    elif len(tag_key) != 16:
        raise ValueError("The tag key must be 16 octets in length.")
    elif len(vector_key) != 16:
        raise ValueError("The vector key must be 16 octets in length.")
    elif len(buffer) < 54:
        raise ValueError("The minimum length of a correctly formatted cipher text is 54 octets.")
    elif operator.mod(len(buffer) - 34, 16) != 0:
        raise ValueError("The cipher text was not aligned to a 16 octet boundary or some of the data is missing.")

    # Parse the envelope.
    vector_shard = buffer[2:18]
    tag_shard = buffer[18:34]
    ciphertext = buffer[34:]

    # Combine the shard values with the key to device the iv and tag.
    iv = str().join(chr(operator.xor(ord(a), ord(b))) \
        for a,b in zip(vector_key, vector_shard))

    tag = str().join(chr(operator.xor(ord(a), ord(b))) \
        for a,b in zip(tag_key, tag_shard))

    # Decrypt the payload.
    decryptor = Cipher(algorithms.AES(cipher_key), modes.GCM(iv, tag), backend=default_backend()).decryptor()
    plaintext = decryptor.update(ciphertext) + decryptor.finalize()

    # Parse the prefix.
    size = struct.unpack(">I", '\x00' + plaintext[0:3])[0]
    pad = struct.unpack(">I", '\x00' + '\x00' + '\x00' + plaintext[3:4])[0]

    # Validate the prefix values.
    if operator.mod(size + pad + 4, 16) != 0 or len(plaintext) != size + pad + 4:
        raise ValueError("The encrypted buffer is invalid.")

    # Confirm the suffix values.
    for offset in xrange(size + 4, size + pad + 4, 1):
        if struct.unpack(">I", '\x00' + '\x00' + '\x00' + plaintext[offset: offset + 1])[0] != pad:
            raise ValueError("The encrypted buffer contained an invalid padding value.")

    # Return just the plain text value.
    return plaintext[4:size + 4]

# User Inputs
# @username = The normalized username.
# @password = The plaintext user password.
#
# Server Inputs
# @salt = Additional non-secret, per-site, or per-user entropy.
# @bonus = Additional hash rounds added beyond those determined by a password's length.
# @nonce = A non-secret ephemerally generated string of random octets, which are combined with a verification token to derive a login token.
#
# @rounds = Required number of hash rounds during each key derivation stage.
# @master_key = The derived key required to decrypt and use realm specific keys.
# @password_key = The output from the second key derivation phase, and required to authenticate password update requests.
# @verification_token = The persistent token stored by a server and used to authenticate future login requests.
# @ephemeral_login_token = The ephemeral value used to authenticate a session or connection.
#
# Realm Inputs
# @realm = The category and/or type of data.
# @shard = A non-secret fragment required to derive the key associated with a given realm.
#

hex = 0

bonus = 131072
password = "password"
username = "user@example.tld"

salt = "lyrtpzN8cBRZvsiHX6y4j-pJOjIyJeuw5aVXzrItw1G4EOa-6CA4R" \
    "9BhVpinkeH0UeXyOeTisHR3Ik3yuOhxbWPyesMJvfp0IBtx0f0uorb8w" \
    "Pnhw5BxDJVCb1TOSE50PFKGBFMkc63Koa7vMDj-WEoDj2X0kkTtlW6cU" \
    "vF8i-M"

nonce = "oDdYAHOsiX7Nl2qTwT18onW0hZdeTO3ebxzZp6nXMTo__0_vr_" \
    "AsmAm3vYRwWtSCPJz0sA2o66uhNm6YenOGz0NkHcSAVgQhKdEBf_BT" \
    "YkyULDuw2fSkbO7mlnxEhxqrJEc27ZVam6ogYABfHZjgVUTAi_SICy" \
    "KAN7KOMuImL2g"

realm = "mail"
shard = "gD65Kdeda1hB2Q6gdZl0fetGg2viLXWG0vmKN4HxE3Jp3Z" \
    "0Gkt5prqSmcuY2o8t24iGSCOnFDpP71c3xl9SX9Q"

secret_message = "Attack at dawn!"

#print (os.linesep + "Inputs")
print ("username: " + username)
print ("password: " + password)
print ("salt: " + (hex_encode(base64url_decode(salt)) if hex == 1 else salt) + os.linesep)

#print ("Derived")
rounds = CalculateHashRounds(password, bonus)
print ("bonus: " + repr(bonus))
print ("rounds: " + repr(rounds))

seed = ExtractEntropySeed(rounds, username, password, base64url_decode(salt))
print ("seed: " + (hex_encode(seed) if hex == 1 else base64url_encode(seed)) + os.linesep)

# print ("Keys")
master_key = HashedKeyDerivation(seed, rounds, username, password, base64url_decode(salt))
print ("master-key: " + (hex_encode(master_key) if hex == 1 else base64url_encode(master_key)))

password_key = HashedKeyDerivation(master_key, rounds, username, password, base64url_decode(salt))
print ("password-key: " + (hex_encode(password_key) if hex == 1 else base64url_encode(password_key)) + os.linesep)

# print ("Tokens")
verification_token = HashedTokenDerivation(password_key, username, base64url_decode(salt))
print ("verification-token: " + (hex_encode(verification_token) if hex == 1 else base64url_encode(verification_token)))

ephemeral_login_token = HashedTokenDerivation(verification_token, username, base64url_decode(salt), base64url_decode(nonce))
print ("nonce: " + (hex_encode(base64url_decode(nonce)) if hex == 1 else nonce) + os.linesep)
print ("ephemeral-login-token: " + (hex_encode(ephemeral_login_token) if hex == 1 else base64url_encode(ephemeral_login_token)) + os.linesep)

realm_key = RealmKeyDerivation(master_key, realm, base64url_decode(shard))
realm_tag_key = ExtractRealmTagKey(realm_key)
realm_vector_key = ExtractRealmVectorKey(realm_key)
realm_cipher_key = ExtractRealmCipherKey(realm_key)

print ("realm-tag-key: " + (hex_encode(realm_tag_key) if hex == 1 else base64url_encode(realm_tag_key)))
print ("realm-vector-key: " + (hex_encode(realm_vector_key) if hex == 1 else base64url_encode(realm_vector_key)))
print ("realm-cipher-key: " + (hex_encode(realm_cipher_key) if hex == 1 else base64url_encode(realm_cipher_key)) + os.linesep)

encrypted_buffer = RealmEncrypt(realm_vector_key, realm_tag_key, realm_cipher_key, secret_message)
decrypted_buffer = RealmDecrypt(realm_vector_key, realm_tag_key, realm_cipher_key, encrypted_buffer)

print ("encrypted-data: " + (hex_encode(encrypted_buffer) if hex == 1 else base64url_encode(encrypted_buffer)))
print ("decrypted-data: " + decrypted_buffer + os.linesep)

