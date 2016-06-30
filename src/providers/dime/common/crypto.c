#include <stdio.h>
#include <string.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include "dime/common/dcrypto.h"
#include "dime/common/misc.h"
#include "dime/common/error.h"

#include "providers/symbols.h"

static EC_GROUP *_encryption_group = NULL;
static EVP_MD const *_ecies_envelope_evp = NULL;

/**
 * @brief
 *  Initialize the cryptographic subsystem.
 * @return
 *  -1 if any part of the initialization process failed, or 0 on success.
 */
int
_crypto_init(void)
{
    SSL_load_error_strings_d();
    SSL_library_init_d();
    OPENSSL_add_all_algorithms_noconf_d();

    if (!(_encryption_group = EC_GROUP_new_by_curve_name_d(EC_ENCRYPT_CURVE))) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(ERR_UNSPEC, "could not initialize encryption curve");
    }

    if (!(_ecies_envelope_evp = EVP_get_digestbyname_d(OBJ_nid2sn_d(NID_sha512)))) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(ERR_UNSPEC, "unable to get SHA-512 digest by NID");
    }

    //EC_GROUP_set_point_conversion_form(group, POINT_CONVERSION_COMPRESSED);

    return 0;
}

/**
 * @brief
 *  Shutdown the cryptographic subsystem.
 */
void
_crypto_shutdown(void)
{
    if (_encryption_group) {
        EC_GROUP_clear_free_d(_encryption_group);
        _encryption_group = NULL;
    }

    EVP_cleanup_d();
    ERR_free_strings_d();
}

/**
 * @brief
 *  Verify that an elliptic curve signature for a given hashed data buffer is
 *  valid.
 * @param hash
 *  a pointer to the hashed data buffer used to generate the signature.
 * @param hlen
 *  the length, in bytes, of the hashed data buffer.
 * @param sig
 *  a pointer to the signature buffer to be verified against the input data.
 * @param slen
 *  the length, in bytes, of the signature buffer.
 * @param key
 *  the EC key which will have its public portion used to verify the signature
 *  of the supplied hashed data.
 * @return
 *  -1 on general failure, 0 if the signature did not match the hashed data, or
 *  1 if it did.
 */
int
_verify_ec_signature(
    unsigned char const *hash,
    size_t hlen,
    unsigned char const *sig,
    size_t slen,
    EC_KEY *key)
{
    ECDSA_SIG *ec_sig;
    int result;

    if (!hash || !hlen || !sig || !slen || !key) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (!(ec_sig = d2i_ECDSA_SIG_d(NULL, &sig, slen))) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(ERR_UNSPEC, "unable to read EC signature from buffer");
    }

    if ((result = ECDSA_do_verify(hash, hlen, ec_sig, key)) < 0) {
        PUSH_ERROR_OPENSSL();
        ECDSA_SIG_free(ec_sig);
        RET_ERROR_INT(ERR_UNSPEC, "unable to complete ECDSA signature verification");
    }

    ECDSA_SIG_free(ec_sig);

    return result;
}

/**
 * @brief
 *  Verify that a signature for a given data buffer is valid.
 * @param data
 *  a pointer to the data buffer used to generate the signature.
 * @param dlen
 *  the length, in bytes, of the data buffer.
 * @param
 *  shabits the number of bits for the desired SHA hash (160, 256, or 512).
 * @param sig
 *  a pointer to the signature buffer to be verified against the input data.
 * @param slen
 *  the length, in bytes, of the signature buffer.
 * @param key
 *  the EC key which will have its public portion used to verify the signature
 *  of the supplied data.
 * @return
 *  -1 on general failure, 0 if the signature did not match the data, or 1 if
 *  it did.
 */
int
_verify_ec_sha_signature(
    unsigned char const *data,
    size_t dlen,
    unsigned int shabits,
    unsigned char const *sig,
    size_t slen,
    EC_KEY *key)
{
    unsigned char hashbuf[SHA_512_SIZE];

    if (!data || !dlen || !sig || !slen || !key) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    } else if ((shabits != 160) && (shabits != 256) && (shabits != 512)) {
        RET_ERROR_INT(
            ERR_BAD_PARAM,
            "ECDSA signature only accepts SHA hash sizes of 160, 256, or 512 bits");
    }

    if (_compute_sha_hash(shabits, data, dlen, hashbuf) < 0) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "unable to compute SHA hash for ECDSA signature verification operation");
    }

    return (_verify_ec_signature(hashbuf, shabits / 8, sig, slen, key));
}

/**
 * @brief
 *  Sign a body of data using the ECDSA algorithm.
 * @param hash
 *  a pointer to the hashed data buffer to be signed.
 * @param hlen
 *  the length, in bytes, of the hashed data to be signed.
 * @param key
 *  the EC key which will have its private portion used to sign the supplied data.
 * @param siglen
 *  a pointer to a variable that will receive the length of the data signature
 *  buffer on success.
 * @return
 *  NULL on failure, or a pointer to the newly allocated signature data buffer
 *  on success.
 */
unsigned char *
_ec_sign_data(
    unsigned char const *hash,
    size_t hlen,
    EC_KEY *key,
    size_t *siglen)
{
    ECDSA_SIG *signature;
    unsigned char *buf = NULL;
    int bsize;

    if (!hash || !hlen || !key || !siglen) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(signature = ECDSA_do_sign(hash, hlen, key))) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "unable to take ECDSA signature of hash buffer");
    }

    if ((bsize = i2d_ECDSA_SIG_d(signature, &buf)) < 0) {
        PUSH_ERROR_OPENSSL();
        ECDSA_SIG_free(signature);
        RET_ERROR_PTR(ERR_UNSPEC, "unable to serialize ECDSA signature");
    }

    ECDSA_SIG_free(signature);

    *siglen = bsize;

    return buf;
}

/**
 * @brief
 *  Sign a SHA-hashed body of data using the ECDSA algorithm.
 * @param data
 *  a pointer to the data buffer to be signed.
 * @param dlen
 *  the length, in bytes, of the data buffer to be signed.
 * @param
 *  shabits the number of bits for the desired SHA hash (160, 256, or 512).
 * @param key
 *  the EC key which will have its private portion used to sign the supplied
 *  data.
 * @param siglen
 *  a pointer to a variable that will receive the length of the data signature
 *  buffer on success.
 * @return
 *  NULL on failure, or a pointer to the newly allocated signature data buffer
 *  on success.
 */
unsigned char *
_ec_sign_sha_data(
    unsigned char const *data,
    size_t dlen,
    unsigned int shabits,
    EC_KEY *key,
    size_t *siglen)
{
    unsigned char hashbuf[SHA_512_SIZE];

    if (!data || !dlen || !key || !siglen) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    } else if ((shabits != 160) && (shabits != 256) && (shabits != 512)) {
        RET_ERROR_PTR(
            ERR_BAD_PARAM,
            "ECDSA signature only accepts SHA hash sizes of 160, 256, or 512 bits");
    }

    if (_compute_sha_hash(shabits, data, dlen, hashbuf) < 0) {
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "unable to compute SHA hash for ECDSA signature operation");
    }

    return (_ec_sign_data(hashbuf, shabits / 8, key, siglen));
}

/**
 * @brief
 *  Serialize an EC public key to be shared.
 * @param
 *  key a pointer to the EC key pair to have its public key serialized.
 * @param
 *  outsize a pointer to a variable that will receive the length of the
 *  serialized key on success.
 * @return
 *  a pointer to the serialized EC public key on success, or NULL on failure.
 */
unsigned char *
_serialize_ec_pubkey(EC_KEY *key, size_t *outsize)
{
    unsigned char *buf = NULL;
    int bsize;

    if (!key || !outsize) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if ((bsize = i2o_ECPublicKey_d(key, &buf)) < 0) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_PTR(ERR_UNSPEC, "unable to serialize EC public key");
    }

    *outsize = bsize;

    return buf;
}

/**
 * @brief
 *  Deserialize an EC public key stored in binary format.
 * @param buf
 *  a pointer to the buffer holding the EC public key in binary format.
 * @param blen
 *  the length, in bytes, of the buffer holding the EC public key.
 * @param signing
 *  if set, generate a key from the pre-defined EC signing curve; if zero, the
 *  default encryption curve will be used instead.
 * @return
 *  a pointer to the deserialized EC public key on success, or NULL on failure.
 */
EC_KEY *
_deserialize_ec_pubkey(
    unsigned char const *buf,
    size_t blen,
    int signing)
{
    EC_KEY *result;
    int nid;

    if (!buf || !blen) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    } else if (signing) {
        RET_ERROR_PTR(
            ERR_BAD_PARAM,
            "deserialization of signing keys is not supported");
    }

    nid = signing ? EC_SIGNING_CURVE : EC_ENCRYPT_CURVE;

    if (!(result = EC_KEY_new_d())) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "could not generate new EC key for deserialization");
    }

    if (EC_KEY_set_group_d(result, EC_GROUP_new_by_curve_name_d(nid)) != 1) {
        PUSH_ERROR_OPENSSL();
        EC_KEY_free_d(result);
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "could not get curve group for deserialization");
    }

    if (!(result =
            o2i_ECPublicKey_d(
                &result,
                (const unsigned char **)&buf,
                blen)))
    {
        PUSH_ERROR_OPENSSL();
        EC_KEY_free_d(result);
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "deserialization of EC public key portion failed");
    }

    return result;
}

/**
 * @brief
 *  Serialize an EC private key into a data buffer.
 * @param key
 *  a pointer to the EC key pair to have its private key serialized.
 * @param outsize
 *  a pointer to a variable that will receive the length of the serialized key
 *  on success.
 * @return
 *  a pointer to the serialized EC private key on success, or NULL on failure.
 */
unsigned char *
_serialize_ec_privkey(EC_KEY *key, size_t *outsize)
{
    unsigned char *buf = NULL;
    int bsize;

    if (!key || !outsize) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if ((bsize = i2d_ECPrivateKey_d(key, &buf)) < 0) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_PTR(ERR_UNSPEC, "unable to serialize EC private key");
    }

    *outsize = bsize;

    return buf;
}

/**
 * @brief
 *  Deserialize an EC private key stored in binary format.
 * @param buf
 *  a pointer to the buffer holding the EC private key in binary format.
 * @param blen
 *  the length, in bytes, of the buffer holding the EC private key.
 * @param signing
 *  if set, generate a key from the pre-defined EC signing curve; if zero, the
 *  default encryption curve will be used instead.
 * @return
 *  a pointer to the deserialized EC private key on success, or NULL on
 *  failure.
 */
EC_KEY *
_deserialize_ec_privkey(
    unsigned char const *buf,
    size_t blen,
    int signing)
{
    EC_KEY *result;
    int nid;
    const unsigned char *bufptr = buf;

    if (!buf || !blen) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    } else if (signing) {
        RET_ERROR_PTR(
            ERR_BAD_PARAM,
            "deserialization of signing keys is not supported");
    }

    nid = signing ? EC_SIGNING_CURVE : EC_ENCRYPT_CURVE;

    if (!(result = EC_KEY_new_d())) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "could not generate new EC key for deserialization");
    }

    if (EC_KEY_set_group_d(
            result,
            EC_GROUP_new_by_curve_name_d(nid))
        != 1)
    {
        PUSH_ERROR_OPENSSL();
        EC_KEY_free_d(result);
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "could not get curve group for deserialization");
    }

    if (!(result = d2i_ECPrivateKey_d(&result, &bufptr, blen))) {
        PUSH_ERROR_OPENSSL();
        EC_KEY_free_d(result);
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "deserialization of EC public key portion failed");
    }

    /*
     * At this point, in most cases bufptr == buf + blen. There may be cases
     * though where the private key is shorter than the provided buffer. This
     * is because DER is a variable-length encoding. Parsing any field behind
     * the privkey must take this into account.
     */

    return result;
}

/**
 * @brief
 *  Load an EC private key from a file.
 * @param filename
 *  the name of the filename from which the key should be loaded
 * @return
 *  a pointer to the deserialized private key from the the file.
 */
EC_KEY *
_load_ec_privkey(char const *filename)
{
    char *filedata;
    unsigned char *bin;
    size_t binsize;
    EC_KEY *result;

    if(!filename) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!(filedata = _read_pem_data(filename, "EC PRIVATE KEY", 1))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not read ec pubkey pem file");
    }

    bin = _b64decode(filedata, strlen(filedata), &binsize);
    _secure_wipe(filedata, strlen(filedata));
    free(filedata);
    if(!bin) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not decode b64 data");
    }

    result = _deserialize_ec_privkey(bin, binsize, 0);
    _secure_wipe(bin, binsize);
    free(bin);
    if(!result) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize binary ec pubkey");
    }

    return result;
}

/**
 * @brief
 *  Load an EC public key from a file.
 * @param filename
 *  the name of the file from which the key should be loaded
 * @return
 *  a pointer to the deserialized public key from the the file.
 */
EC_KEY *
_load_ec_pubkey(char const *filename)
{
    char *filedata;
    unsigned char *bin;
    size_t binsize;
    EC_KEY *result;

    if(!filename) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!(filedata = _read_pem_data(filename, "PUBLIC KEY", 1))) {
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "could not read ec pubkey pem file");
    }

    bin = _b64decode(filedata, strlen(filedata), &binsize);
    _secure_wipe(filedata, strlen(filedata));
    free(filedata);
    if(!bin) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not decode b64 data");
    }

    result = _deserialize_ec_pubkey(bin, binsize, 0);
    _secure_wipe(bin, binsize);
    free(bin);
    if(!result) {
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "could not deserialize binary ec pubkey");
    }

    return result;
}

/**
 * @brief
 *  Generate an EC key pair.
 * @return
 *  a newly allocated and generated EC key pair on success, or NULL on failure.
 */
EC_KEY *
_generate_ec_keypair(void)
{
    EC_GROUP *group;
    EC_KEY *result;

    group = _encryption_group;

    if (!group) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not determine curve group for operation");
    }

    if (!(result = EC_KEY_new_d())) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_PTR(ERR_UNSPEC, "unable to allocate new EC key pair for generation");
    }

    if (EC_KEY_set_group_d(result, group) != 1) {
        PUSH_ERROR_OPENSSL();
        EC_KEY_free_d(result);
        RET_ERROR_PTR(ERR_UNSPEC, "unable to associate curve group with new EC key pair");
    }

    if (EC_KEY_generate_key_d(result) != 1) {
        PUSH_ERROR_OPENSSL();
        EC_KEY_free_d(result);
        RET_ERROR_PTR(ERR_UNSPEC, "unable to generate new EC key pair");
    }

    return result;
}

/**
 * @brief
 *  Free an EC keypair.
 * @param
 *  key a pointer to the EC keypair to be freed.
 */
void
_free_ec_key(EC_KEY *key)
{
    if (!key) {
        //fprintf(stderr, "Error: Attempted to free NULL EC key.\n");
        return;
    }

    EC_KEY_free_d(key);
}

/**
 * @brief
 *  Generate an ed25519 key pair.
 * @return
 *  a newly allocated and generated ed25519 key pair on success, or NULL on
 *  failure.
 */
ED25519_KEY *
_generate_ed25519_keypair(void)
{
    ED25519_KEY *result;

    if (!(result = malloc(sizeof(ED25519_KEY)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(
            ERR_NOMEM,
            "could not generate ed25519 key because of "
            "memory allocation error");
    }

    memset(result, 0, sizeof(ED25519_KEY));

    if (RAND_bytes_d(
            result->private_key,
            sizeof(result->private_key))
        != 1)
    {
        PUSH_ERROR_OPENSSL();
        _secure_wipe(result, sizeof(ED25519_KEY));
        free(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not generate ed25519 secret key");
    }

    ed25519_publickey(result->private_key, result->public_key);

    return result;
}

/**
 * @brief
 *  Take an ed25519 signature of a data buffer.
 * @return
 *  0 on success or -1 on failure.
 */
int _ed25519_sign_data(
    unsigned char const *data,
    size_t dlen,
    ED25519_KEY *key,
    ed25519_signature sigbuf)
{
    if (!data || !dlen || !key || !sigbuf) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    ed25519_sign(data, dlen, key->private_key, key->public_key, sigbuf);

    return 0;
}

/**
 * @brief
 *  Verify an ed25519 signature taken over a data buffer.
 * @return
 *  1 if the signature matched the buffer, 0 if it did not, or -1 on failure.
 */
int
_ed25519_verify_sig(
    unsigned char const *data,
    size_t dlen,
    ED25519_KEY *key,
    ed25519_signature sigbuf)
{
    int result;

    if (!data || !dlen || !key || !sigbuf) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    result = ed25519_sign_open(data, dlen, key->public_key, sigbuf);

    if (!result) {
        return 1;
    }

    return 0;
}

/**
 * @brief
 *  Free an ed25519 keypair.
 * @param key
 *  a pointer to the ed25519 keypair to be freed.
 */
void
_free_ed25519_key(ED25519_KEY *key)
{
    if (!key) {
        return;
    }

    _secure_wipe(key, sizeof(ED25519_KEY));
    free(key);
}

/**
 * @brief
 *  Free a list of ed25519 keypairs.
 * @param keys
 *  a pointer to a NULL terminated list of ed25519 keypair to be freed.
 */
void
_free_ed25519_key_chain(ED25519_KEY **keys)
{
    if (!keys) {
        return;
    }

    for(size_t i = 0; keys[i]; ++i) {
        _free_ed25519_key(keys[i]);
    }

    free(keys);
}

/**
 * @brief
 *  Load an ed25519 private key from a file.
 * @param filename
 *  the path of the armored file from which the ed25519 private key will be
 *  loaded.
 * @return
 *  a pointer to a newly allocated ed25519 keypair on success, or NULL on
 *  failure.
 */
ED25519_KEY *
_load_ed25519_privkey(char const *filename)
{
    ED25519_KEY *result;
    unsigned char *keydata;
    char *pemdata;
    size_t klen;

    if (!filename) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(pemdata =
            _read_pem_data(filename, "ED25519 PRIVATE KEY", 1))) {
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "unable to read ed25519 private key data from PEM file");
    }

    keydata = _b64decode(pemdata, strlen(pemdata), &klen);
    _secure_wipe(pemdata, strlen(pemdata));
    free(pemdata);

    if (!keydata || (klen != ED25519_KEY_SIZE)) {

        if (keydata) {
            _secure_wipe(keydata, klen);
            free(keydata);
        }

        RET_ERROR_PTR(ERR_UNSPEC, "bad ED25519 key data was read from file");
    }

    if (!(result = malloc(sizeof(ED25519_KEY)))) {
        PUSH_ERROR_SYSCALL("malloc");
        _secure_wipe(keydata, klen);
        free(keydata);
        RET_ERROR_PTR(ERR_NOMEM, "unable to allocate space for ED25519 key");
    }

    memset(result, 0, sizeof(ED25519_KEY));
    memcpy(result->private_key, keydata, sizeof(result->private_key));
    _secure_wipe(keydata, klen);
    free(keydata);
    ed25519_publickey(result->private_key, result->public_key);

    return result;
}

/**
 * @brief
 *  Deserializes an ed25519 public key into a public-only ED25519_KEY structure
 *  that can only be used for signature verification, not signing.
 * @param serial_pubkey
 *  Serialized ed25519 public key.
 * @return
 *  Pointer to ED25519_KEY structure.
*/
ED25519_KEY *
_deserialize_ed25519_pubkey(
    unsigned char const *serial_pubkey)
{
    ED25519_KEY *key;

    if(!serial_pubkey) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!(key = malloc(sizeof(ED25519_KEY)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, NULL);
    }

    memset(key, 0, sizeof(ED25519_KEY));
    memcpy(key->public_key, serial_pubkey, ED25519_KEY_SIZE);

    return key;
}

/**
 * @brief
 *  Deserializes an ed25519 private key into a ED25519_KEY structure.
 * @param serial_privkey
 *  Serialized ed25519 private key.
 * @return
 *  Pointer to the ED25119_KEY structure.
*/
ED25519_KEY *
_deserialize_ed25519_privkey(
    unsigned char const *serial_privkey)
{
    ED25519_KEY *key;

    if(!serial_privkey) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!(key = malloc(sizeof(ED25519_KEY)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, NULL);
    }

    memset(key, 0, sizeof(ED25519_KEY));
    memcpy(key->private_key, serial_privkey, ED25519_KEY_SIZE);
    ed25519_publickey(key->private_key, key->public_key);

    return key;
}

/**
 * @note
 *  This function was taken from providers/cryptography/openssl.c
 */
void *
_ecies_env_derivation(
    void const *input,
    size_t ilen,
    void *output,
    size_t *olen)
{
    if (EVP_Digest_d(
            input,
            ilen,
            output,
            (unsigned int *)olen,
            _ecies_envelope_evp,
            NULL)
        != 1)
    {
        return NULL;
    }

    return output;
}


/**
 * @brief
 *  Compute a derived AES-256 key from the intersection of a public EC key and
 *  a private EC key.
 */
int
_compute_aes256_kek(
    EC_KEY *public_key,
    EC_KEY *private_key,
    unsigned char *keybuf)
{
    unsigned char aeskey[SHA_512_SIZE];

    if (!public_key || !private_key || !keybuf) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (ECDH_compute_key(
            aeskey,
            sizeof(aeskey),
            EC_KEY_get0_public_key_d(public_key),
            private_key,
            _ecies_env_derivation)
        != SHA_512_SIZE)
    {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not derive AES key from EC keypair");
    }

    for(size_t i = 0; i < 16; ++i) {
        keybuf[i] = aeskey[i] ^ aeskey[i + 16];
    }

    memcpy(keybuf + 16, aeskey + 32, 32);
    _secure_wipe(aeskey, sizeof(aeskey));

    return 0;
}


/**
 * @brief
 *  Fill a buffer with a sequence of (securely) random bytes.
 * @param buf
 *  a pointer to the buffer to be filled with random bytes.
 * @param len
 *  the length, in bytes, of the buffer to be filled.
 * @return
 *  0 on success or -1 on failure.
 */
int
_get_random_bytes(
    void *buf,
    size_t len)
{
    if (!buf) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (!RAND_bytes_d(buf, len)) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(ERR_UNSPEC, "unable to generate random bytes");
    }

    return 0;
}

/**
 * @brief
 *  Encrypt a data buffer using an AES-256 key (in CBC mode).
 * @param outbuf
 *  a pointer to the output buffer that will receive the encrypted data. NOTE:
 *  the size of this buffer must be successfully negotiated by the caller.
 * @param data
 *  a pointer to the data buffer to be encrypted.
 * @param dlen
 *  the size, in bytes, of the data buffer to be encrypted.
 * @param key
 *  a pointer to the 32-byte buffer holding the AES-256 encryption key for the
 *  operation.
 * @param iv
 *  a pointer to the 32-byte initialization vector to be used for the
 *  encryption process.
 * @return
 *  the number of bytes successfully encrypted on success, or -1 on failure.
 */
int
_encrypt_aes_256(
    unsigned char *outbuf,
    unsigned char const *data,
    size_t dlen,
    unsigned char const *key,
    unsigned char const *iv)
{
    EVP_CIPHER_CTX *ctx;
    int len, result;

    if (!outbuf || !data || !dlen || !key || !iv) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (dlen % AES_256_PADDING_SIZE) {
        RET_ERROR_INT(
            ERR_BAD_PARAM,
            "input data was not aligned to required padding size");
    }

    if (!(ctx = EVP_CIPHER_CTX_new_d())) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(
            ERR_UNSPEC,
            "unable to create new context for AES-256 encryption");
    }

    if (EVP_EncryptInit_ex_d(ctx, EVP_aes_256_cbc_d(), NULL, key, iv) != 1) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(
            ERR_UNSPEC,
            "unable to initialize context for AES-256 encryption");
    }

    if (EVP_CIPHER_CTX_set_padding_d(ctx, 0) != 1) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(
            ERR_UNSPEC,
            "unable to set no padding for AES-256 encryption");
    }

    if (EVP_EncryptUpdate_d(ctx, outbuf, &len, data, dlen) != 1) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(
            ERR_UNSPEC,
            "AES-256 encryption update failed");
    }

    result = len;

    if (EVP_EncryptFinal_ex_d(ctx, (unsigned char *)outbuf + len, &len) != 1) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(
            ERR_UNSPEC,
            "AES-256 encryption finalization failed");
    }

    result += len;
    EVP_CIPHER_CTX_free_d(ctx);
    ctx = NULL;

    return result;
}

/**
 * @brief
 *  Decrypt a data buffer using an AES-256 key (in CBC mode).
 * @param outbuf
 *  a pointer to the output buffer that will receive the decrypted data. NOTE:
 *  the size of this buffer must be successfully negotiated by the caller.
 * @param data
 *  a pointer to the data buffer to be decrypted.
 * @param dlen
 *  the size, in bytes, of the data buffer to be decrypted.
 * @param key
 *  a pointer to the 32-byte buffer holding the AES-256 decryption key for the
 *  operation.
 * @param iv
 *  a pointer to the 32-byte initialization vector to be used for the
 *  decryption process.
 * @return
 *  the number of bytes successfully decrypted on success, or -1 on failure.
 */
int
_decrypt_aes_256(
    unsigned char *outbuf,
    unsigned char const *data,
    size_t dlen,
    unsigned char const *key,
    unsigned char const *iv)
{
    EVP_CIPHER_CTX *ctx = NULL;
    int len, result;

    if (!outbuf || !data || !dlen || !key || !iv) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (dlen % AES_256_PADDING_SIZE) {
        RET_ERROR_INT(ERR_BAD_PARAM, "input data was not aligned to required padding size");
    }

    if (!(ctx = EVP_CIPHER_CTX_new_d())) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(ERR_UNSPEC, "unable to create new context for AES-256 decryption");
    }

    if (EVP_DecryptInit_ex_d(ctx, EVP_aes_256_cbc_d(), NULL, key, iv) != 1) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(ERR_UNSPEC, "unable to initialize context for AES-256 decryption");
    }

    if (EVP_CIPHER_CTX_set_padding_d(ctx, 0) != 1) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(ERR_UNSPEC, "unable to set no padding for AES-256 decryption");
    }

    if (EVP_DecryptUpdate_d(ctx, outbuf, &len, data, dlen) != 1) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(ERR_UNSPEC, "AES-256 decryption update failed");
    }

    result = len;

    if (EVP_DecryptFinal_ex_d(ctx, (unsigned char *)outbuf + len, &len) != 1) {
        PUSH_ERROR_OPENSSL();
        RET_ERROR_INT(ERR_UNSPEC, "AES-256 decryption finalization failed");
    }

    result += len;

    EVP_CIPHER_CTX_free_d(ctx);
    ctx = NULL;

    return result;
}
