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
import warnings
from Crypto.Random import get_random_bytes
from Crypto.Hash import SHA512
from Crypto.Hash.HMAC import HMAC

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

def ExtractRealmCipherKey(realm_key):
    realm_cipher_key = realm_key[0:32]
    return realm_cipher_key

def ExtractRealmVectorKey(realm_key):
    realm_vector_key = str().join(chr(operator.xor(ord(a), ord(b))) \
        for a,b in zip(realm_key[32:48], realm_key[48:64]))
    return realm_vector_key


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

def PrintUsage():
    print ("Usage: stacie.py [--hex|--help] username password [salt]")

# User Inputs
# @username = The normalized username.
# @password = The plaintext user password.

# Server Inputs
# @salt = Additional non-secret, per-site, or per-user entropy.
# @bonus = Additional hash rounds added beyond those determined by a password's length.
# @nonce = A non-secret ephemerally generated string of random octets, which are combined with a verification token to derive a login token.

# @rounds = Required number of hash rounds during each key derivation stage.
# @master_key = The derived key required to decrypt and use realm specific keys.
# @password_key = The output from the second key derivation phase, and required to authenticate password update requests.
# @verification_token = The persistent token stored by a server and used to authenticate future login requests.
# @ephemeral_login_token = The ephemeral value used to authenticate a session or connection.

hex = 0
bonus = 0
username = None
password = None
salt = base64url_encode(get_random_bytes(128))
nonce = base64url_encode(get_random_bytes(128))

try:
    opts, args = getopt.getopt(sys.argv[1:], "hx", ["help", "hex"])
except getopt.GetoptError:
    PrintUsage()
    sys.exit(2)
for opt, arg in opts:
    if opt in ("-h", "--help"):
        PrintUsage()
        sys.exit(0)
    elif opt in ("-x", "--hex"):
        hex = 1

if len(args) != 2 and len(args) != 3:
    PrintUsage()
    sys.exit(2)

username = args[0]
password = args[1]
if len(args) == 3:
    salt = args[2]


#print (os.linesep + "Inputs")
print ("username: " + username)
print ("password: " + password)
print ("salt: " + (hex_encode(salt) if hex == 1 else base64url_encode(salt)) + os.linesep)

#print ("Derived")
rounds = CalculateHashRounds(password, bonus)
print ("bonus: " + repr(bonus))
print ("rounds: " + repr(rounds))

seed = ExtractEntropySeed(rounds, username, password, base64url_decode(salt))
print ("seed: " + (hex_encode(seed) if hex == 1 else base64url_encode(seed)) + os.linesep)

#print ("Keys")
master_key = HashedKeyDerivation(seed, rounds, username, password, base64url_decode(salt))
print ("master-key: " + (hex_encode(master_key) if hex == 1 else base64url_encode(master_key)))

password_key = HashedKeyDerivation(master_key, rounds, username, password, base64url_decode(salt))
print ("password-key: " + (hex_encode(password_key) if hex == 1 else base64url_encode(password_key)) + os.linesep)

#print ("Tokens")
verification_token = HashedTokenDerivation(password_key, username, base64url_decode(salt))
print ("verification-token: " + (hex_encode(verification_token) if hex == 1 else base64url_encode(verification_token)))

ephemeral_login_token = HashedTokenDerivation(verification_token, username, base64url_decode(salt), base64url_decode(nonce))
print ("nonce: " + (hex_encode(nonce) if hex == 1 else base64url_encode(nonce)))
print ("ephemeral-login-token: " + (hex_encode(ephemeral_login_token) if hex == 1 else base64url_encode(ephemeral_login_token)))

#
# # Realm Inputs
# # @realm = The category and/or type of data.
# # @shard = A non-secret fragment required to derive the key associated with a given realm.
#
# realm = "mail"
# shard = "gD65Kdeda1hB2Q6gdZl0fetGg2viLXWG0vmKN4HxE3Jp3Z" \
#     "0Gkt5prqSmcuY2o8t24iGSCOnFDpP71c3xl9SX9Q"
#
# realm_key = RealmKeyDerivation(master_key, realm, base64url_decode(shard))
#
# realm_cipher_key = ExtractRealmCipherKey(realm_key)
# realm_vector_key = ExtractRealmVectorKey(realm_key)
#
# print ("realm-vector-key: " + base64url_encode(realm_vector_key))
# print ("realm-cipher-key: " + base64url_encode(realm_cipher_key))
