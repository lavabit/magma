#include "dime/util/encrypt.h"

#include <stdio.h>
#include <string.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

struct encrypt_ctx {
    EC_GROUP *encryption_group;
    EVP_MD const *ecies_envelope_evp;
};

static void
get_openssl_error(
    char *result,
    size_t result_length);

void
get_openssl_error(
    char *result,
    size_t result_length)
{
    char const *ssl_filename;
    char const *ssl_data;
    char tmpbuf[512];
    char tmpbuf2[512];
    int ssl_line;
    int ssl_flags;
    int first_pass = 1;
    static int loaded = 0;

    if (!loaded) {
        ERR_load_crypto_strings_d();
        SSL_load_error_strings_d();
        loaded = 1;
    }

    memset(result, 0, result_length);

    // Keep doing this as long as there's more errors there.
    while (
        ERR_peek_error_line_data_d(
            &ssl_filename,
            &ssl_line,
            &ssl_data,
            &ssl_flags))
    {
        if (!first_pass) {
            BUF_strlcat_d(result, " | ", result_length);
        } else {
            first_pass = 0;
        }

        memset(tmpbuf, 0, sizeof(tmpbuf));
        snprintf(
            tmpbuf,
            sizeof(tmpbuf),
            ":%s:%d",
            ssl_filename,
            ssl_line);

        memset(tmpbuf2, 0, sizeof(tmpbuf2));
        ERR_error_string_n_d(ERR_get_error_d(), tmpbuf2, sizeof(tmpbuf2));

        // Combine the two error strings and add them to the buffer.
        BUF_strlcat_d(result, tmpbuf2, result_length);
        BUF_strlcat_d(result, tmpbuf, result_length);
    }
}

#define LOG_OPENSSL_ERROR(ctx, buf, msg) do {\
    get_openssl_error(buf, sizeof(buf)); \
    DIME_LOG_ERROR(dime_ctx, buf); \
    DIME_LOG_ERROR(dime_ctx, msg); \
} while (0)

/**
 * @brief
 *  Initialize the cryptographic subsystem.
 * @note
 *  This is currently NOT reentrant.  (Apparently OpenSSL likes global state
 *  :P)
 */
derror_t const *
encrypt_ctx_new(
    dime_ctx_t const *dime_ctx,
    encrypt_ctx_t **result)
{
    derror_t const *err = NULL;
    char errbuf[512];

    if (dime_ctx == NULL
        || result == NULL
        || *result != NULL)
    {
        err = ERR_BAD_PARAM;
        goto error;
    }

    *result = malloc(sizeof(encrypt_ctx_t));
    if (*result == NULL) {
        err = ERR_NOMEM;
        goto error;
    }
    memset(*result, 0, sizeof(encrypt_ctx_t));

    SSL_load_error_strings_d();
    SSL_library_init_d();
    OPENSSL_add_all_algorithms_noconf_d();

    (*result)->encryption_group =
        EC_GROUP_new_by_curve_name_d(NID_secp256k1);
    if ((*result)->encryption_group == NULL) {
        LOG_OPENSSL_ERROR(
            dime_ctx,
            errbuf,
            "couldn't create EC_GROUP object");
        err = ERR_CRYPTO;
        goto cleanup_evp;
    }

    (*result)->ecies_envelope_evp =
        EVP_get_digestbyname_d(OBJ_nid2sn_d(NID_sha512));
    if ((*result)->ecies_envelope_evp == NULL) {
        LOG_OPENSSL_ERROR(
            dime_ctx,
            errbuf,
            "couldn't get ecies envelope");
        err = ERR_CRYPTO;
        goto cleanup_group;
    }

    //EC_GROUP_set_point_conversion_form(group, POINT_CONVERSION_COMPRESSED);

    return NULL;

cleanup_group:
    EC_GROUP_clear_free_d((*result)->encryption_group);
cleanup_evp:
    EVP_cleanup_d();
    ERR_free_strings_d();
    free(*result);
    *result = NULL;
error:
    return err;
}

/**
 * @brief
 *  Shutdown the cryptographic subsystem.
 */
void
encrypt_ctx_free(encrypt_ctx_t *ctx)
{
    if (ctx->encryption_group != NULL) {
        EC_GROUP_clear_free_d(ctx->encryption_group);
    }

    EVP_cleanup_d();
    ERR_free_strings_d();

    free(ctx);
}

/**
 * @brief
 *  Generate an EC key pair.
 * @param result
 *  a newly allocated and generated EC key pair on success, or NULL on failure.
 */
derror_t const *
encrypt_keypair_generate(
    dime_ctx_t const *dime_ctx,
    encrypt_ctx_t const *encrypt_ctx,
    encrypt_keypair_t **result)
{
    derror_t const *err = NULL;
    EC_KEY **ec_key = (EC_KEY **)result;
    char errbuf[512];

    if (dime_ctx == NULL
        || encrypt_ctx == NULL
        || result == NULL
        || *result != NULL)
    {
        err = ERR_BAD_PARAM;
        goto error;
    }

    *ec_key = EC_KEY_new_d();
    if (*ec_key == NULL) {
        LOG_OPENSSL_ERROR(dime_ctx, errbuf,
            "unable to allocate new EC key pair for generation");
        err = ERR_CRYPTO;
        goto error;
    }

    if (EC_KEY_set_group_d(*ec_key, encrypt_ctx->encryption_group) != 1) {
        LOG_OPENSSL_ERROR(dime_ctx, errbuf,
            "unable to associate curve group with new EC key pair");
        err = ERR_CRYPTO;
        goto cleanup_ec_key;
    }

    if (EC_KEY_generate_key_d(*ec_key) != 1) {
        LOG_OPENSSL_ERROR(dime_ctx, errbuf,
            "unable to generate new EC key pair");
        err = ERR_CRYPTO;
        goto cleanup_ec_key;
    }

    return NULL;

cleanup_ec_key:
    EC_KEY_free_d(*ec_key);
    *ec_key = NULL;
error:
    return err;
}

/**
 * @brief
 *  Deserialize an EC public key stored in binary format.
 * @param result
 *  a pointer to the deserialized EC public key on success, or NULL on failure.
 * @param buf
 *  a pointer to the buffer holding the EC public key in binary format.
 * @param blen
 *  the length, in bytes, of the buffer holding the EC public key.
 */
derror_t const *
encrypt_deserialize_pubkey(
    dime_ctx_t const *dime_ctx,
    encrypt_keypair_t **result,
    unsigned char const *buf,
    size_t blen)
{
    derror_t const *err = NULL;
    EC_KEY **ec_key = (EC_KEY **)result;
    char errbuf[512];

    if (dime_ctx == NULL
        || result == NULL
        || *result != NULL
        || buf == NULL
        || blen == 0)
    {
        err = ERR_BAD_PARAM;
        goto error;
    }

    if (!(*ec_key = EC_KEY_new_d())) {
        LOG_OPENSSL_ERROR(dime_ctx, errbuf,
            "could not generate new EC key for deserialization");
        err = ERR_CRYPTO;
        goto error;
    }

    if (EC_KEY_set_group_d(
            *ec_key,
            EC_GROUP_new_by_curve_name_d(NID_secp256k1))
        != 1)
    {
        LOG_OPENSSL_ERROR(dime_ctx, errbuf,
            "could not get curve group for deserialization");
        err = ERR_CRYPTO;
        goto cleanup_ec_key;
    }

    if (!(*ec_key =
            o2i_ECPublicKey_d(
                ec_key,
                (const unsigned char **)&buf,
                blen)))
    {
        LOG_OPENSSL_ERROR(dime_ctx, errbuf,
            "deserialization of EC public key portion failed");
        err = ERR_CRYPTO;
        goto cleanup_ec_key;
    }

    return NULL;

cleanup_ec_key:
    EC_KEY_free_d(*ec_key);
    *ec_key = NULL;
error:
    return err;
}

/**
 * @brief
 *  Deserialize an EC private key stored in binary format.
 * @param result
 *  a pointer to the deserialized EC private key on success, or NULL on
 *  failure.
 * @param buf
 *  a pointer to the buffer holding the EC private key in binary format.
 * @param blen
 *  the length, in bytes, of the buffer holding the EC private key.
 */
derror_t const *
encrypt_deserialize_privkey(
    dime_ctx_t const *dime_ctx,
    encrypt_keypair_t **result,
    unsigned char const *buf,
    size_t blen)
{
    derror_t const *err = NULL;
    EC_KEY **result_key = (EC_KEY **)result;
    const unsigned char *bptr = buf;
    char errbuf[512];

    if (dime_ctx == NULL
        || result == NULL
        || *result != NULL
        || buf == NULL
        || blen == 0)
    {
        err = ERR_BAD_PARAM;
        goto error;
    }

    *result_key = EC_KEY_new_d();
    if (*result_key == NULL) {
        LOG_OPENSSL_ERROR(dime_ctx, errbuf,
            "could not generate new EC key for deserialization");
        err = ERR_CRYPTO;
        goto error;
    }

    if (EC_KEY_set_group_d(
            *result_key,
            EC_GROUP_new_by_curve_name_d(NID_secp256k1))
        != 1)
    {
        LOG_OPENSSL_ERROR(dime_ctx, errbuf,
            "could not get curve group for deserialization");
        err = ERR_CRYPTO;
        goto cleanup_ec_key;
    }

    *result_key = d2i_ECPrivateKey_d(result_key, &bptr, blen);
    if (*result_key == NULL) {
        LOG_OPENSSL_ERROR(dime_ctx, errbuf,
            "deserialization of EC public key portion failed");
        err = ERR_CRYPTO;
        goto cleanup_ec_key;
    }

    /*
     * At this point, in most cases bufptr == buf + blen. There may be cases
     * though where the private key is shorter than the provided buffer. This
     * is because DER is a variable-length encoding. Parsing any field behind
     * the privkey must take this into account.
     */

    return NULL;

cleanup_ec_key:
    EC_KEY_free_d(*result_key);
    *result_key = NULL;
error:
    return err;
}

///**
// * @brief
// *  Load an EC public key from a file.
// * @param result
// *  double pointer to the resulting public key
// * @param filename
// *  the name of the file from which the key should be loaded
// */
//derror_t const *
//load_ec_pubkey(
//    dime_ctx_t const *dime_ctx,
//    encrypt_keypair_t **result,
//    char const *filename)
//{
//    derror_t const *err;
//    char *filedata;
//    unsigned char *bin;
//    size_t binsize;
//
//    if (result == NULL
//        || *result != NULL
//        || filename == NULL)
//    {
//        err = ERR_BAD_PARAM;
//        goto out;
//    }
//
//    filedata = _read_pem_data(filename, "PUBLIC KEY", 1)
//    if (filedata == NULL) {
//        LOG_ERROR(dime_ctx,
//            "could not read ec pubkey pem file");
//        err = ERR_FILE_IO;
//        goto out;
//    }
//
//    bin = _b64decode(filedata, strlen(filedata), &binsize);
//    if(bin == NULL) {
//        LOG_ERROR(dime_ctx,
//            "could not decode b64 data");
//        err = ERR_ENCODING;
//        goto cleanup_filedata;
//    }
//
//    err = encrypt_deserialize_pubkey(
//        dime_ctx,
//        result,
//        bin,
//        binsize);
//
//    _secure_wipe(bin, binsize);
//    free(bin);
//cleanup_filedata:
//    _secure_wipe(filedata, strlen(filedata));
//    free(filedata);
//out:
//    return err;
//}

///**
// * @brief
// *  Verify that an elliptic curve signature for a given hashed data buffer is
// *  valid.
// * @param hash
// *  a pointer to the hashed data buffer used to generate the signature.
// * @param hlen
// *  the length, in bytes, of the hashed data buffer.
// * @param sig
// *  a pointer to the signature buffer to be verified against the input data.
// * @param slen
// *  the length, in bytes, of the signature buffer.
// * @param key
// *  the EC key which will have its public portion used to verify the signature
// *  of the supplied hashed data.
// * @return
// *  -1 on general failure, 0 if the signature did not match the hashed data, or
// *  1 if it did.
// */
//int
//_verify_ec_signature(
//    unsigned char const *hash,
//    size_t hlen,
//    unsigned char const *sig,
//    size_t slen,
//    EC_KEY *key)
//{
//    ECDSA_SIG *ec_sig;
//    int result;
//
//    if (!hash || !hlen || !sig || !slen || !key) {
//        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
//    }
//
//    if (!(ec_sig = d2i_ECDSA_SIG_d(NULL, &sig, slen))) {
//        PUSH_ERROR_OPENSSL();
//        RET_ERROR_INT(ERR_UNSPEC, "unable to read EC signature from buffer");
//    }
//
//    if ((result = ECDSA_do_verify_d(hash, hlen, ec_sig, key)) < 0) {
//        PUSH_ERROR_OPENSSL();
//        ECDSA_SIG_free_d(ec_sig);
//        RET_ERROR_INT(ERR_UNSPEC, "unable to complete ECDSA signature verification");
//    }
//
//    ECDSA_SIG_free_d(ec_sig);
//
//    return result;
//}
//
///**
// * @brief
// *  Verify that a signature for a given data buffer is valid.
// * @param data
// *  a pointer to the data buffer used to generate the signature.
// * @param dlen
// *  the length, in bytes, of the data buffer.
// * @param
// *  shabits the number of bits for the desired SHA hash (160, 256, or 512).
// * @param sig
// *  a pointer to the signature buffer to be verified against the input data.
// * @param slen
// *  the length, in bytes, of the signature buffer.
// * @param key
// *  the EC key which will have its public portion used to verify the signature
// *  of the supplied data.
// * @return
// *  -1 on general failure, 0 if the signature did not match the data, or 1 if
// *  it did.
// */
//int
//_verify_ec_sha_signature(
//    unsigned char const *data,
//    size_t dlen,
//    unsigned int shabits,
//    unsigned char const *sig,
//    size_t slen,
//    EC_KEY *key)
//{
//    unsigned char hashbuf[SHA_512_SIZE];
//
//    if (!data || !dlen || !sig || !slen || !key) {
//        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
//    } else if ((shabits != 160) && (shabits != 256) && (shabits != 512)) {
//        RET_ERROR_INT(
//            ERR_BAD_PARAM,
//            "ECDSA signature only accepts SHA hash sizes of 160, 256, or 512 bits");
//    }
//
//    if (_compute_sha_hash(shabits, data, dlen, hashbuf) < 0) {
//        RET_ERROR_INT(
//            ERR_UNSPEC,
//            "unable to compute SHA hash for ECDSA signature verification operation");
//    }
//
//    return (_verify_ec_signature(hashbuf, shabits / 8, sig, slen, key));
//}
//
///**
// * @brief
// *  Sign a body of data using the ECDSA algorithm.
// * @param hash
// *  a pointer to the hashed data buffer to be signed.
// * @param hlen
// *  the length, in bytes, of the hashed data to be signed.
// * @param key
// *  the EC key which will have its private portion used to sign the supplied data.
// * @param siglen
// *  a pointer to a variable that will receive the length of the data signature
// *  buffer on success.
// * @return
// *  NULL on failure, or a pointer to the newly allocated signature data buffer
// *  on success.
// */
//unsigned char *
//_ec_sign_data(
//    unsigned char const *hash,
//    size_t hlen,
//    EC_KEY *key,
//    size_t *siglen)
//{
//    ECDSA_SIG *signature;
//    unsigned char *buf = NULL;
//    int bsize;
//
//    if (!hash || !hlen || !key || !siglen) {
//        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
//    }
//
//    if (!(signature = ECDSA_do_sign_d(hash, hlen, key))) {
//        PUSH_ERROR_OPENSSL();
//        RET_ERROR_PTR(
//            ERR_UNSPEC,
//            "unable to take ECDSA signature of hash buffer");
//    }
//
//    if ((bsize = i2d_ECDSA_SIG_d(signature, &buf)) < 0) {
//        PUSH_ERROR_OPENSSL();
//        ECDSA_SIG_free_d(signature);
//        RET_ERROR_PTR(ERR_UNSPEC, "unable to serialize ECDSA signature");
//    }
//
//    ECDSA_SIG_free_d(signature);
//
//    *siglen = bsize;
//
//    return buf;
//}
//
///**
// * @brief
// *  Sign a SHA-hashed body of data using the ECDSA algorithm.
// * @param data
// *  a pointer to the data buffer to be signed.
// * @param dlen
// *  the length, in bytes, of the data buffer to be signed.
// * @param
// *  shabits the number of bits for the desired SHA hash (160, 256, or 512).
// * @param key
// *  the EC key which will have its private portion used to sign the supplied
// *  data.
// * @param siglen
// *  a pointer to a variable that will receive the length of the data signature
// *  buffer on success.
// * @return
// *  NULL on failure, or a pointer to the newly allocated signature data buffer
// *  on success.
// */
//unsigned char *
//_ec_sign_sha_data(
//    unsigned char const *data,
//    size_t dlen,
//    unsigned int shabits,
//    EC_KEY *key,
//    size_t *siglen)
//{
//    unsigned char hashbuf[SHA_512_SIZE];
//
//    if (!data || !dlen || !key || !siglen) {
//        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
//    } else if ((shabits != 160) && (shabits != 256) && (shabits != 512)) {
//        RET_ERROR_PTR(
//            ERR_BAD_PARAM,
//            "ECDSA signature only accepts SHA hash sizes of 160, 256, or 512 bits");
//    }
//
//    if (_compute_sha_hash(shabits, data, dlen, hashbuf) < 0) {
//        RET_ERROR_PTR(
//            ERR_UNSPEC,
//            "unable to compute SHA hash for ECDSA signature operation");
//    }
//
//    return (_ec_sign_data(hashbuf, shabits / 8, key, siglen));
//}
//
///**
// * @brief
// *  Serialize an EC public key to be shared.
// * @param
// *  key a pointer to the EC key pair to have its public key serialized.
// * @param
// *  outsize a pointer to a variable that will receive the length of the
// *  serialized key on success.
// * @return
// *  a pointer to the serialized EC public key on success, or NULL on failure.
// */
//unsigned char *
//_serialize_ec_pubkey(EC_KEY *key, size_t *outsize)
//{
//    unsigned char *buf = NULL;
//    int bsize;
//
//    if (!key || !outsize) {
//        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
//    }
//
//    if ((bsize = i2o_ECPublicKey_d(key, &buf)) < 0) {
//        PUSH_ERROR_OPENSSL();
//        RET_ERROR_PTR(ERR_UNSPEC, "unable to serialize EC public key");
//    }
//
//    *outsize = bsize;
//
//    return buf;
//}
//
///**
// * @brief
// *  Serialize an EC private key into a data buffer.
// * @param key
// *  a pointer to the EC key pair to have its private key serialized.
// * @param outsize
// *  a pointer to a variable that will receive the length of the serialized key
// *  on success.
// * @return
// *  a pointer to the serialized EC private key on success, or NULL on failure.
// */
//unsigned char *
//_serialize_ec_privkey(EC_KEY *key, size_t *outsize)
//{
//    unsigned char *buf = NULL;
//    int bsize;
//
//    if (!key || !outsize) {
//        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
//    }
//
//    if ((bsize = i2d_ECPrivateKey_d(key, &buf)) < 0) {
//        PUSH_ERROR_OPENSSL();
//        RET_ERROR_PTR(ERR_UNSPEC, "unable to serialize EC private key");
//    }
//
//    *outsize = bsize;
//
//    return buf;
//}
//
///**
// * @brief
// *  Load an EC private key from a file.
// * @param filename
// *  the name of the filename from which the key should be loaded
// * @return
// *  a pointer to the deserialized private key from the the file.
// */
//EC_KEY *
//_load_ec_privkey(char const *filename)
//{
//    char *filedata;
//    unsigned char *bin;
//    size_t binsize;
//    EC_KEY *result;
//
//    if(!filename) {
//        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
//    }
//
//    if(!(filedata = _read_pem_data(filename, "EC PRIVATE KEY", 1))) {
//        RET_ERROR_PTR(ERR_UNSPEC, "could not read ec pubkey pem file");
//    }
//
//    bin = _b64decode(filedata, strlen(filedata), &binsize);
//    _secure_wipe(filedata, strlen(filedata));
//    free(filedata);
//    if(!bin) {
//        RET_ERROR_PTR(ERR_UNSPEC, "could not decode b64 data");
//    }
//
//    result = _deserialize_ec_privkey(bin, binsize, 0);
//    _secure_wipe(bin, binsize);
//    free(bin);
//    if(!result) {
//        RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize binary ec pubkey");
//    }
//
//    return result;
//}
//
///**
// * @brief
// *  Free an EC keypair.
// * @param
// *  key a pointer to the EC keypair to be freed.
// */
//void
//_free_ec_key(EC_KEY *key)
//{
//    if (!key) {
//        //fprintf(stderr, "Error: Attempted to free NULL EC key.\n");
//        return;
//    }
//
//    EC_KEY_free_d(key);
//}
//
//void *
//_ecies_env_derivation(
//    void const *input,
//    size_t ilen,
//    void *output,
//    size_t *olen)
//{
//    if (EVP_Digest_d(
//            input,
//            ilen,
//            output,
//            (unsigned int *)olen,
//            _ecies_envelope_evp,
//            NULL)
//        != 1)
//    {
//        return NULL;
//    }
//
//    return output;
//}
//
///**
// * @brief
// *  Compute a derived AES-256 key from the intersection of a public EC key and
// *  a private EC key.
// */
//int
//_compute_aes256_kek(
//    EC_KEY *public_key,
//    EC_KEY *private_key,
//    unsigned char *keybuf)
//{
//    unsigned char aeskey[SHA_512_SIZE];
//
//    if (!public_key || !private_key || !keybuf) {
//        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
//    }
//
//    if (ECDH_compute_key_d(
//            aeskey,
//            sizeof(aeskey),
//            EC_KEY_get0_public_key_d(public_key),
//            private_key,
//            _ecies_env_derivation)
//        != SHA_512_SIZE)
//    {
//        PUSH_ERROR_OPENSSL();
//        RET_ERROR_INT(
//            ERR_UNSPEC,
//            "could not derive AES key from EC keypair");
//    }
//
//    for(size_t i = 0; i < 16; ++i) {
//        keybuf[i] = aeskey[i] ^ aeskey[i + 16];
//    }
//
//    memcpy(keybuf + 16, aeskey + 32, 32);
//    _secure_wipe(aeskey, sizeof(aeskey));
//
//    return 0;
//}
//
///**
// * @brief
// *  Fill a buffer with a sequence of (securely) random bytes.
// * @param buf
// *  a pointer to the buffer to be filled with random bytes.
// * @param len
// *  the length, in bytes, of the buffer to be filled.
// * @return
// *  0 on success or -1 on failure.
// */
//int
//_get_random_bytes(
//    void *buf,
//    size_t len)
//{
//    if (!buf) {
//        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
//    }
//
//    if (!RAND_bytes_d(buf, len)) {
//        PUSH_ERROR_OPENSSL();
//        RET_ERROR_INT(ERR_UNSPEC, "unable to generate random bytes");
//    }
//
//    return 0;
//}
//
///**
// * @brief
// *  Encrypt a data buffer using an AES-256 key (in CBC mode).
// * @param outbuf
// *  a pointer to the output buffer that will receive the encrypted data. NOTE:
// *  the size of this buffer must be successfully negotiated by the caller.
// * @param data
// *  a pointer to the data buffer to be encrypted.
// * @param dlen
// *  the size, in bytes, of the data buffer to be encrypted.
// * @param key
// *  a pointer to the 32-byte buffer holding the AES-256 encryption key for the
// *  operation.
// * @param iv
// *  a pointer to the 32-byte initialization vector to be used for the
// *  encryption process.
// * @return
// *  the number of bytes successfully encrypted on success, or -1 on failure.
// */
//int
//_encrypt_aes_256(
//    unsigned char *outbuf,
//    unsigned char const *data,
//    size_t dlen,
//    unsigned char const *key,
//    unsigned char const *iv)
//{
//    EVP_CIPHER_CTX *ctx;
//    int len, result;
//
//    if (!outbuf || !data || !dlen || !key || !iv) {
//        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
//    }
//
//    if (dlen % AES_256_PADDING_SIZE) {
//        RET_ERROR_INT(
//            ERR_BAD_PARAM,
//            "input data was not aligned to required padding size");
//    }
//
//    if (!(ctx = EVP_CIPHER_CTX_new_d())) {
//        PUSH_ERROR_OPENSSL();
//        RET_ERROR_INT(
//            ERR_UNSPEC,
//            "unable to create new context for AES-256 encryption");
//    }
//
//    if (EVP_EncryptInit_ex_d(ctx, EVP_aes_256_cbc_d(), NULL, key, iv) != 1) {
//        PUSH_ERROR_OPENSSL();
//        RET_ERROR_INT(
//            ERR_UNSPEC,
//            "unable to initialize context for AES-256 encryption");
//    }
//
//    if (EVP_CIPHER_CTX_set_padding_d(ctx, 0) != 1) {
//        PUSH_ERROR_OPENSSL();
//        RET_ERROR_INT(
//            ERR_UNSPEC,
//            "unable to set no padding for AES-256 encryption");
//    }
//
//    if (EVP_EncryptUpdate_d(ctx, outbuf, &len, data, dlen) != 1) {
//        PUSH_ERROR_OPENSSL();
//        RET_ERROR_INT(
//            ERR_UNSPEC,
//            "AES-256 encryption update failed");
//    }
//
//    result = len;
//
//    if (EVP_EncryptFinal_ex_d(ctx, (unsigned char *)outbuf + len, &len) != 1) {
//        PUSH_ERROR_OPENSSL();
//        RET_ERROR_INT(
//            ERR_UNSPEC,
//            "AES-256 encryption finalization failed");
//    }
//
//    result += len;
//    EVP_CIPHER_CTX_free_d(ctx);
//    ctx = NULL;
//
//    return result;
//}
//
///**
// * @brief
// *  Decrypt a data buffer using an AES-256 key (in CBC mode).
// * @param outbuf
// *  a pointer to the output buffer that will receive the decrypted data. NOTE:
// *  the size of this buffer must be successfully negotiated by the caller.
// * @param data
// *  a pointer to the data buffer to be decrypted.
// * @param dlen
// *  the size, in bytes, of the data buffer to be decrypted.
// * @param key
// *  a pointer to the 32-byte buffer holding the AES-256 decryption key for the
// *  operation.
// * @param iv
// *  a pointer to the 32-byte initialization vector to be used for the
// *  decryption process.
// * @return
// *  the number of bytes successfully decrypted on success, or -1 on failure.
// */
//int
//_decrypt_aes_256(
//    unsigned char *outbuf,
//    unsigned char const *data,
//    size_t dlen,
//    unsigned char const *key,
//    unsigned char const *iv)
//{
//    EVP_CIPHER_CTX *ctx = NULL;
//    int len, result;
//
//    if (!outbuf || !data || !dlen || !key || !iv) {
//        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
//    }
//
//    if (dlen % AES_256_PADDING_SIZE) {
//        RET_ERROR_INT(ERR_BAD_PARAM, "input data was not aligned to required padding size");
//    }
//
//    if (!(ctx = EVP_CIPHER_CTX_new_d())) {
//        PUSH_ERROR_OPENSSL();
//        RET_ERROR_INT(ERR_UNSPEC, "unable to create new context for AES-256 decryption");
//    }
//
//    if (EVP_DecryptInit_ex_d(ctx, EVP_aes_256_cbc_d(), NULL, key, iv) != 1) {
//        PUSH_ERROR_OPENSSL();
//        RET_ERROR_INT(ERR_UNSPEC, "unable to initialize context for AES-256 decryption");
//    }
//
//    if (EVP_CIPHER_CTX_set_padding_d(ctx, 0) != 1) {
//        PUSH_ERROR_OPENSSL();
//        RET_ERROR_INT(ERR_UNSPEC, "unable to set no padding for AES-256 decryption");
//    }
//
//    if (EVP_DecryptUpdate_d(ctx, outbuf, &len, data, dlen) != 1) {
//        PUSH_ERROR_OPENSSL();
//        RET_ERROR_INT(ERR_UNSPEC, "AES-256 decryption update failed");
//    }
//
//    result = len;
//
//    if (EVP_DecryptFinal_ex_d(ctx, (unsigned char *)outbuf + len, &len) != 1) {
//        PUSH_ERROR_OPENSSL();
//        RET_ERROR_INT(ERR_UNSPEC, "AES-256 decryption finalization failed");
//    }
//
//    result += len;
//
//    EVP_CIPHER_CTX_free_d(ctx);
//    ctx = NULL;
//
//    return result;
//}
