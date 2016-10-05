
#include <stdio.h>
#include <openssl/ec.h>
#include <openssl/rand.h>

extern "C" {
#include "dime_check_params.h"
#include "dime/common/dcrypto.h"
#include "dime/common/misc.h"
#include "symbols.h"
}
#include "gtest/gtest.h"

#define N_SERIALIZATION_TESTS  20
#define N_SIGNATURE_TIER_TESTS 5

static unsigned char *gen_random_data(size_t minlen, size_t maxlen, size_t *outlen) {

    unsigned char *result;
    unsigned long rval;
    size_t rlen;
    int res;

    EXPECT_GE(maxlen, minlen);

    res = RAND_bytes_d((unsigned char *)&rval, sizeof(rval));
    EXPECT_TRUE(res == 0 || res == 1);
    rlen = minlen + (rval % (maxlen - minlen + 1));

    result = (unsigned char *)malloc(rlen);
    EXPECT_TRUE(result != NULL);

    res = RAND_bytes_d(result, rlen);
    EXPECT_TRUE(res == 0 || res == 1);
    *outlen = rlen;

    return result;
}

TEST(DIME, check_ec_signatures)
{
    EC_KEY *key;

    unsigned char *rdata, *sigdata;
    size_t dlens[] = { 16, 128, 1024, 65535 };
    size_t rsize, siglen, last_min = 1;
    int res;

    res = crypto_init();
    ASSERT_TRUE(!res) << "Crypto initialization routine failed.";

    key = generate_ec_keypair();
    ASSERT_TRUE(key != NULL) << "EC signature/verification check failed: could not generate key pair.";

    for (size_t i = 0; i < (sizeof(dlens) / sizeof(dlens[0])); i++) {

        for (size_t j = 0; j < N_SIGNATURE_TIER_TESTS; j++) {
            rdata = gen_random_data(last_min, dlens[i], &rsize);
            ASSERT_TRUE(rdata != NULL) << "EC signature/verification check failed: could not generate random data.";

            sigdata = ec_sign_data(rdata, rsize, key, &siglen);
            ASSERT_TRUE(sigdata != NULL) << "EC signature/verification check failed: could not sign data.";
            ASSERT_GT(siglen, 0U) << "EC signature/verification check failed: signature result had bad length.";

            res = verify_ec_signature(rdata, rsize, sigdata, siglen, key);
            ASSERT_EQ(1, res) << "EC signature/verification check failed: signature verification failed (" << res << ").";

            free(sigdata);
            free(rdata);
        }

        last_min = dlens[i];
    }

    free_ec_key(key);
}

TEST(DIME, check_ec_sha_signatures)
{
    EC_KEY *key;

    unsigned char *rdata, *sigdata;
    size_t dlens[] = { 16, 128, 1024, 65535 };
    size_t rsize, siglen, last_min = 1;
    unsigned int shabits;
    int res;

    res = crypto_init();
    ASSERT_TRUE(!res) << "Crypto initialization routine failed.";

    key = generate_ec_keypair();
    ASSERT_TRUE(key != NULL) << "EC SHA signature/verification check failed: could not generate key pair.";

    for (size_t i = 0; i < (sizeof(dlens) / sizeof(dlens[0])); i++) {

        for (size_t j = 0; j < N_SIGNATURE_TIER_TESTS; j++) {

            for (size_t k = 0; k < 3; k++) {

                if (!k) {
                    shabits = 160;
                } else if (k == 1) {
                    shabits = 256;
                } else {
                    shabits = 512;
                }

                rdata = gen_random_data(last_min, dlens[i], &rsize);
                ASSERT_TRUE(rdata != NULL) << "EC SHA signature/verification check failed: could not generate random data.";
                sigdata = ec_sign_sha_data(rdata, rsize, shabits, key, &siglen);
                ASSERT_TRUE(sigdata != NULL) << "EC SHA signature/verification check failed: could not sign data.";
                ASSERT_GT(siglen, 0U) << "EC SHA signature/verification check failed: signature result had bad length.";

                res = verify_ec_sha_signature(rdata, rsize, shabits, sigdata, siglen, key);
                ASSERT_EQ(1, res) << "EC SHA signature/verification check failed: signature verification failed (" << res << ").";

                free(sigdata);
                free(rdata);
            }

            last_min = dlens[i];
        }

    }


    free_ec_key(key);
}

TEST(DIME, load_ec_key_file)
{
    char filename[256], *b64key;
    EC_KEY *result, *key;
    size_t size;
    long crc;
    char *b64_crc_keys, holder[16];
    unsigned char *serial, be[3];
    int res;

    res = _crypto_init();

    ASSERT_TRUE(!res) << "Crypto initialization routine failed.";

    for (size_t i = 0; i < 5; ++i) {
        key = _generate_ec_keypair();
        snprintf(filename, sizeof(filename), DIME_CHECK_OUTPUT_PATH "ec-key-%zu-priv.pem", i + 1);
        serial = _serialize_ec_privkey(key, &size);
        b64key = _b64encode(serial, size);

		crc = _compute_crc24_checksum(serial, size);
		be[0] = ((unsigned char *)&crc)[2];
		be[1] = ((unsigned char *)&crc)[1];
		be[2] = ((unsigned char *)&crc)[0];

		b64_crc_keys = _b64encode((unsigned char *)&be, 3);

		if (snprintf(holder, 16, "\n=%s", b64_crc_keys) != 6) {
			free(b64_crc_keys);
			free(serial);
			ASSERT_TRUE(true) << "b64_crc failed for " << filename;
		}
		else {
			free(serial);
			free(b64_crc_keys);
		}

        _write_pem_data(b64key, holder, "EC PRIVATE KEY", filename);
        free(b64key);

        snprintf(filename, sizeof(filename), DIME_CHECK_OUTPUT_PATH "ec-key-%zu-pub.pem", i + 1);
        serial = _serialize_ec_pubkey(key, &size);
        free_ec_key(key);
        b64key = _b64encode(serial, size);

        crc = _compute_crc24_checksum(serial, size);

		be[0] = ((unsigned char *)&crc)[2];
		be[1] = ((unsigned char *)&crc)[1];
		be[2] = ((unsigned char *)&crc)[0];

		b64_crc_keys = _b64encode((unsigned char *)&be, 3);

		if (snprintf(holder, 16, "\n=%s", b64_crc_keys) != 6) {
			free(b64_crc_keys);
			free(serial);
			ASSERT_TRUE(true) << "b64_crc failed for " << filename;
		}
		else {
			free(b64_crc_keys);
			free(serial);
		}
        _write_pem_data(b64key, holder, "PUBLIC KEY", filename);
        free(b64key);
    }

    for (size_t i = 0; i < 5; i++) {
        snprintf(filename, sizeof(filename), DIME_CHECK_OUTPUT_PATH "ec-key-%zu-priv.pem", i + 1);
        result = _load_ec_privkey(filename);
        ASSERT_TRUE(result != NULL) << "load_ec_privkey failed for " << filename;
        free_ec_key(result);

        snprintf(filename, sizeof(filename), DIME_CHECK_OUTPUT_PATH "ec-key-%zu-pub.pem", i + 1);
        result = _load_ec_pubkey(filename);
        ASSERT_TRUE(result != NULL) << "load_ec_pubkey failed for " << filename;
        free_ec_key(result);
    }
}

TEST(DIME, check_ec_serialization)
{
    EC_KEY *pair, *pair2;
    unsigned char *sbuf, *sbuf2;
    int res;
    size_t ssize, ssize2;

    res = crypto_init();
    ASSERT_TRUE(!res) << "Crypto initialization routine failed.";

    for (size_t i = 0; i < N_SERIALIZATION_TESTS; i++) {
        pair = _generate_ec_keypair();
        ASSERT_TRUE(pair != NULL) << "EC serialization check failed: could not generate key pair.";

        sbuf = _serialize_ec_pubkey(pair, &ssize);
        ASSERT_TRUE(sbuf != NULL) << "EC serialization check failed: pubkey serialization error.";

        pair2 = _deserialize_ec_pubkey(sbuf, ssize, 0);
        ASSERT_TRUE(pair2 != NULL) << "EC serialization check failed: pubkey deserialization error.";

        sbuf2 = _serialize_ec_pubkey(pair, &ssize2);
        ASSERT_TRUE(sbuf2 != NULL) << "EC serialization check failed: pubkey serialization error [2].";

        ASSERT_EQ(ssize, ssize2) << "EC serialization check failed: serialized pubkeys had different serialized lengths {" << ssize << " vs " << ssize2 << "}";

        res = memcmp(sbuf, sbuf2, ssize);
        ASSERT_TRUE(!res) << "EC serialization check failed: serialized pubkeys had different data.";

        free(sbuf);
        free(sbuf2);

        _free_ec_key(pair2);

        sbuf = _serialize_ec_privkey(pair, &ssize);
        ASSERT_TRUE(sbuf != NULL) << "EC serialization check failed: pubkey serialization error.";

        pair2 = _deserialize_ec_privkey(sbuf, ssize, 0);
        ASSERT_TRUE(pair2 != NULL) << "EC serialization check failed: pubkey deserialization error.";

        sbuf2 = _serialize_ec_privkey(pair, &ssize2);
        ASSERT_TRUE(sbuf2 != NULL) << "EC serialization check failed: pubkey serialization error [2].";

        ASSERT_EQ(ssize, ssize2) << "EC serialization check failed: serialized pubkeys had different serialized lengths {" << ssize << " vs " << ssize2 << "}";

        res = memcmp(sbuf, sbuf2, ssize);
        ASSERT_TRUE(!res) << "EC serialization check failed: serialized pubkeys had different data.";

        free(sbuf);
        free(sbuf2);
        free_ec_key(pair);
    }
}

TEST(DIME, check_ecdh_kdf)
{

    EC_KEY *ec1, *ec2, *pub1, *pub2;
    int res;
    size_t serial_size;
    unsigned char *serial_temp, key1[48], key2[48];

    memset(key1, 0, 48);
    memset(key2, 0, 48);

    res = crypto_init();

    ec1 = _generate_ec_keypair();
    ec2 = _generate_ec_keypair();

    ASSERT_TRUE(ec1 != NULL) << "EC key generation failed.";
    ASSERT_TRUE(ec2 != NULL) << "EC key generation failed.";

    serial_temp = _serialize_ec_pubkey(ec1, &serial_size);

    ASSERT_TRUE(serial_temp != NULL) << "could not serialize public key.";

    pub1 = _deserialize_ec_pubkey(serial_temp, serial_size, 0);

    res = _compute_aes256_kek(pub1, ec2, key1);

    ASSERT_EQ(0, res) << "could not perform ECDH key exchange.";

    free(serial_temp);

    serial_temp = _serialize_ec_pubkey(ec2, &serial_size);

    ASSERT_TRUE(serial_temp != NULL) << "could not serialize public key.";

    pub2 = _deserialize_ec_pubkey(serial_temp, serial_size, 0);

    res = _compute_aes256_kek(pub2, ec1, key2);

    ASSERT_EQ(0, res) << "could not perform the second ECDH key exchange.";

    ASSERT_EQ(0, memcmp(key1, key2, 48)) << "the key derivation functions did not yield the correct result";
}

TEST(DIME, check_ed25519_signatures)
{
    ED25519_KEY *key;
    ed25519_signature sigbuf;
    unsigned char *rdata;
    size_t dlens[] = { 16, 128, 1024, 65535 };
    size_t rsize, last_min = 1;
    int res;

    res = crypto_init();
    ASSERT_TRUE(!res) << "Crypto initialization routine failed.";

    key = generate_ed25519_keypair();
    ASSERT_TRUE(key != NULL) << "ed25519 signature/verification check failed: could not generate key pair.";

    for (size_t i = 0; i < (sizeof(dlens) / sizeof(dlens[0])); i++) {

        for (size_t j = 0; j < N_SIGNATURE_TIER_TESTS; j++) {
            rdata = gen_random_data(last_min, dlens[i], &rsize);
            memset(sigbuf, 0, sizeof(sigbuf));
            ASSERT_TRUE(rdata != NULL) << "ed25519 signature/verification check failed: could not generate random data.";
            ed25519_sign_data(rdata, rsize, key, sigbuf);

            res = ed25519_verify_sig(rdata, rsize, key, sigbuf);
            ASSERT_EQ(1, res) << "ed25519 signature/verification check failed: signature verification failed (" << res << ").";

            free(rdata);
        }

        last_min = dlens[i];
    }

    free_ed25519_key(key);
}
