#include <unistd.h>
extern "C" {
#include "dime_check_params.h"
#include "dime/common/misc.h"
#include "dime/signet/keys.h"
#include "dime/signet/signet.h"
}
#include "gtest/gtest.h"


TEST(DIME, check_keys_file_handling)
{

    const char *filename_u = DIME_CHECK_OUTPUT_PATH "keys_user.keys",
    	*filename_o = DIME_CHECK_OUTPUT_PATH "keys_org.keys",
		*filename_w = DIME_CHECK_OUTPUT_PATH "keys_wrong.keys";
    EC_KEY *enckey, *enckey2;
    ED25519_KEY *signkey, *signkey2;
    int res;
    size_t enc1_size, enc2_size;
    unsigned char *ser_enc1, *ser_enc2;

    _crypto_init();

    enckey = _generate_ec_keypair();
    signkey = _generate_ed25519_keypair();

    ser_enc1 = _serialize_ec_privkey(enckey, &enc1_size);

    /* testing user keys file */
    res = dime_keys_file_create(KEYS_TYPE_USER, signkey, enckey, filename_u);
    ASSERT_EQ(0, res) << "Failure creating user keys file.";

    signkey2 = dime_keys_signkey_fetch(filename_u);
    ASSERT_TRUE(signkey2 != NULL) << "Failure fetching signing key.";

    res = memcmp(signkey->private_key, signkey2->private_key, ED25519_KEY_SIZE);
    ASSERT_EQ(0, res) << "Corruption of signing key data.";

    _free_ed25519_key(signkey2);

    enckey2 = dime_keys_enckey_fetch(filename_u);
    ASSERT_TRUE(enckey2 != NULL) << "Failure fetching encryption key.";

    ser_enc2 = _serialize_ec_privkey(enckey2, &enc2_size);
    _free_ec_key(enckey2);
    ASSERT_EQ(enc1_size, enc2_size) << "Corruption of serialized encryption key size.";

    res = memcmp(ser_enc1, ser_enc2, enc1_size);
    ASSERT_EQ(0, res) << "Corruption of encryption key data.";

    free(ser_enc2);

/* testing organizational keys file */
    res = dime_keys_file_create(KEYS_TYPE_ORG, signkey, enckey, filename_o);
    ASSERT_EQ(0, res) << "Failure to create organizational keys file.";

    signkey2 = dime_keys_signkey_fetch(filename_o);
    ASSERT_TRUE(signkey2 != NULL) << "Failure to fetch signing key.";

    res = memcmp(signkey->private_key, signkey2->private_key, ED25519_KEY_SIZE);
    ASSERT_EQ(0, res) << "Corruption of signing key data.";

    _free_ed25519_key(signkey2);

    enckey2 = dime_keys_enckey_fetch(filename_o);
    ASSERT_TRUE(enckey2 != NULL) << "Failure to fetch encryption key.";

    ser_enc2 = _serialize_ec_privkey(enckey2, &enc2_size);
    ASSERT_EQ(enc1_size, enc2_size) << "Corruption of serialized encryption key size.";

    res = memcmp(ser_enc1, ser_enc2, enc1_size);
    ASSERT_EQ(0, res) << "Corruption of encryption key data.";

    _free_ec_key(enckey2);
    free(ser_enc1);
    free(ser_enc2);

/* testing invalid keys file types */
    res = dime_keys_file_create(KEYS_TYPE_ERROR, signkey, enckey, filename_w);
    ASSERT_TRUE(res != 0) << "Failure to trigger error creating keys file type KEYS_TYPE_ERROR.";

    ASSERT_EQ(-1, access(filename_w, F_OK)) << "Unintended creation of keys file with invalid type.";

    res = dime_keys_file_create(static_cast<keys_type_t>(4), signkey, enckey, filename_w);
    ASSERT_NE(0, res) << "Failure to trigger error creating keys file type 4.";

    ASSERT_EQ(-1, access(filename_w, F_OK)) << "Unintended creation of keys file with invalid type.";

    _free_ed25519_key(signkey);
    _free_ec_key(enckey);
}
