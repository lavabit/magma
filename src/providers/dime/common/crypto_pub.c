#include "dime/common/dcrypto.h"
#include "dime/common/error.h"

int crypto_init(void) {
    PUBLIC_FUNC_IMPL(crypto_init, );
}

void crypto_shutdown(void) {
    PUBLIC_FUNC_IMPL_VOID(crypto_shutdown, );
}

int verify_ec_signature(const unsigned char *hash, size_t hlen, const unsigned char *sig, size_t slen, EC_KEY *key) {
    PUBLIC_FUNC_IMPL(verify_ec_signature, hash, hlen, sig, slen, key);
}

int verify_ec_sha_signature(const unsigned char *data, size_t dlen, unsigned int shabits, const unsigned char *sig, size_t slen, EC_KEY *key) {
    PUBLIC_FUNC_IMPL(verify_ec_sha_signature, data, dlen, shabits, sig, slen, key);
}

unsigned char *ec_sign_data(const unsigned char *hash, size_t hlen, EC_KEY *key, size_t *siglen) {
    PUBLIC_FUNC_IMPL(ec_sign_data, hash, hlen, key, siglen);
}

unsigned char *ec_sign_sha_data(const unsigned char *data, size_t dlen, unsigned int shabits, EC_KEY *key, size_t *siglen) {
    PUBLIC_FUNC_IMPL(ec_sign_sha_data, data, dlen, shabits, key, siglen);
}

unsigned char *serialize_ec_pubkey(EC_KEY *key, size_t *outsize) {
    PUBLIC_FUNC_IMPL(serialize_ec_pubkey, key, outsize);
}

EC_KEY *deserialize_ec_pubkey(const unsigned char *buf, size_t blen, int signing) {
    PUBLIC_FUNC_IMPL(deserialize_ec_pubkey, buf, blen, signing);
}

unsigned char *serialize_ec_privkey(EC_KEY *key, size_t *outsize) {
    PUBLIC_FUNC_IMPL(serialize_ec_privkey, key, outsize);
}

EC_KEY *deserialize_ec_privkey(const unsigned char *buf, size_t blen, int signing) {
    PUBLIC_FUNC_IMPL(deserialize_ec_privkey, buf, blen, signing);
}

EC_KEY *load_ec_privkey(const char *filename) {
    PUBLIC_FUNC_IMPL(load_ec_privkey, filename);
}

EC_KEY *load_ec_pubkey(const char *filename) {
    PUBLIC_FUNC_IMPL(load_ec_pubkey, filename);
}

EC_KEY *generate_ec_keypair(void) {
    PUBLIC_FUNC_IMPL(generate_ec_keypair);
}

void free_ec_key(EC_KEY *key) {
    PUBLIC_FUNC_IMPL_VOID(free_ec_key, key);
}

ED25519_KEY *generate_ed25519_keypair(void) {
    PUBLIC_FUNC_IMPL(generate_ed25519_keypair, );
}

int ed25519_sign_data(const unsigned char *data, size_t dlen, ED25519_KEY *key, ed25519_signature sigbuf) {
    PUBLIC_FUNC_IMPL(ed25519_sign_data, data, dlen, key, sigbuf);
}

int ed25519_verify_sig(const unsigned char *data, size_t dlen, ED25519_KEY *key, ed25519_signature sigbuf) {
    PUBLIC_FUNC_IMPL(ed25519_verify_sig, data, dlen, key, sigbuf);
}

void free_ed25519_key(ED25519_KEY *key) {
    PUBLIC_FUNC_IMPL_VOID(free_ed25519_key, key);
}

void free_ed25519_key_chain(ED25519_KEY **keys) {
    PUBLIC_FUNC_IMPL_VOID(free_ed25519_key_chain, keys);
}

ED25519_KEY *load_ed25519_privkey(const char *filename) {
    PUBLIC_FUNC_IMPL(load_ed25519_privkey, filename);
}

ED25519_KEY *deserialize_ed25519_pubkey(const unsigned char *serial_pubkey) {
    PUBLIC_FUNC_IMPL(deserialize_ed25519_pubkey, serial_pubkey);
}

ED25519_KEY *deserialize_ed25519_privkey(const unsigned char *serial_privkey) {
    PUBLIC_FUNC_IMPL(deserialize_ed25519_privkey, serial_privkey);
}

void *ecies_env_derivation(const void *input, size_t ilen, void *output, size_t *olen) {
    PUBLIC_FUNC_IMPL(ecies_env_derivation, input, ilen, output, olen);
}

int compute_aes256_kek(EC_KEY *public_key, EC_KEY *private_key, unsigned char *keybuf) {
    PUBLIC_FUNC_IMPL(compute_aes256_kek, public_key, private_key, keybuf);
}

int get_random_bytes(void *buf, size_t len) {
    PUBLIC_FUNC_IMPL(get_random_bytes, buf, len);
}

int encrypt_aes_256(unsigned char *outbuf, const unsigned char *data, size_t dlen, const unsigned char *key, const unsigned char *iv) {
    PUBLIC_FUNC_IMPL(encrypt_aes_256, outbuf, data, dlen, key, iv);
}

int decrypt_aes_256(unsigned char *outbuf, const unsigned char *data, size_t dlen, const unsigned char *key, const unsigned char *iv) {
    PUBLIC_FUNC_IMPL(decrypt_aes_256, outbuf, data, dlen, key, iv);
}
