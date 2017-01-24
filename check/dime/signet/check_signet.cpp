#include <unistd.h>
extern "C" {
#include "dime_check_params.h"
#include "dime/common/misc.h"
#include "dime/signet/keys.h"
#include "dime/signet/signet.h"
}
#include "gtest/gtest.h"

TEST(DIME, check_signet_creation)
{
    signet_t *signet;
    signet_type_t type;

    signet = dime_sgnt_signet_create(SIGNET_TYPE_ORG);
    ASSERT_TRUE(signet != NULL) << "Failureto create organizational signet.";

    type = dime_sgnt_type_get(signet);
    ASSERT_EQ(SIGNET_TYPE_ORG, type) << "Corrupted signet type.";

    dime_sgnt_signet_destroy(signet);

    signet = dime_sgnt_signet_create(SIGNET_TYPE_USER);
    ASSERT_TRUE(signet != NULL) << "Failure to create user signet.";

    type = dime_sgnt_type_get(signet);
    ASSERT_EQ(SIGNET_TYPE_USER, type) << "Corrupted signet type.";

    dime_sgnt_signet_destroy(signet);

    signet = dime_sgnt_signet_create(SIGNET_TYPE_SSR);
    ASSERT_TRUE(signet != NULL) << "Failure to create SSR.";

    type = dime_sgnt_type_get(signet);
    ASSERT_EQ(SIGNET_TYPE_SSR, type) << "Corrupted signet type.";

    dime_sgnt_signet_destroy(signet);

    signet = dime_sgnt_signet_create(SIGNET_TYPE_ERROR);
    ASSERT_EQ(NULL, signet) << "Unintended creation of signet with invalid type SIGNET_TYPE_ERROR";

    signet = dime_sgnt_signet_create(static_cast<signet_type_t>(52));
    ASSERT_EQ(NULL, signet) << "Unintended creation of signet with invalid type 52";
}

TEST(DIME, check_signet_keys_pairing)
{
    const char *filename_u = DIME_CHECK_OUTPUT_PATH "keys_user.keys",
    	*filename_o = DIME_CHECK_OUTPUT_PATH "keys_org.keys",
        *filename_s = DIME_CHECK_OUTPUT_PATH "keys_ssr.keys",
		*filename_w = DIME_CHECK_OUTPUT_PATH "keys_wrong.keys",
        *to_sign = "AbcDEFghijKLMNOpqrstuVWXYZ";
    EC_KEY *priv_enckey, *pub_enckey;
    ED25519_KEY *priv_signkey, *pub_signkey;
    ed25519_signature sigbuf;
    int res = 0;
    signet_t *signet;
    signet_type_t type;
    size_t enc1_size, enc2_size;
    unsigned char *enc1_pub, *enc2_pub;

    _crypto_init();

/* creating user signet with keys */
    signet = dime_sgnt_signet_create_w_keys(SIGNET_TYPE_USER, filename_u);
    ASSERT_TRUE(signet != NULL) << "Failure to create user signet.";

    type = dime_sgnt_type_get(signet);
    ASSERT_EQ(SIGNET_TYPE_USER, type) << "Corrupted signet type.";

    priv_signkey = dime_keys_signkey_fetch(filename_u);
    ASSERT_TRUE(priv_signkey != NULL) << "Failure to fetch private signing key from file.";

    res = _ed25519_sign_data((const unsigned char *)to_sign, strlen(to_sign), priv_signkey, sigbuf);
    ASSERT_EQ(0, res) << "Failure to sign data buffer.";

    pub_signkey = dime_sgnt_signkey_fetch(signet);
    ASSERT_TRUE(pub_signkey != NULL) << "Failure to fetch public signing key from signet.";

    res = _ed25519_verify_sig((const unsigned char *)to_sign, strlen(to_sign), pub_signkey, sigbuf);
    ASSERT_EQ(1, res) << "Failure to verify signature";

    priv_enckey = dime_keys_enckey_fetch(filename_u);
    ASSERT_TRUE(priv_enckey != NULL) << "Failure to fetch private encryption key from file.";

    enc1_pub = _serialize_ec_pubkey(priv_enckey, &enc1_size);
    ASSERT_TRUE(enc1_pub != NULL) << "Failure to serialize public portion of the private encryption key.";

    pub_enckey = dime_sgnt_enckey_fetch(signet);
    ASSERT_TRUE(pub_enckey != NULL) << "Failure to fetch public encryption key from signet.";

    enc2_pub = _serialize_ec_pubkey(pub_enckey, &enc2_size);
    ASSERT_EQ(enc1_size, enc2_size) << "Corrupted public encryption key size.";
    ASSERT_EQ(0, memcmp(enc1_pub, enc2_pub, enc1_size)) << "Corrupted public encryption key data.";

    _free_ed25519_key(priv_signkey);
    _free_ed25519_key(pub_signkey);
    _free_ec_key(pub_enckey);
    _free_ec_key(priv_enckey);
    free(enc1_pub);
    free(enc2_pub);
    dime_sgnt_signet_destroy(signet);

/* creating organizational signet with keys */
    signet = dime_sgnt_signet_create_w_keys(SIGNET_TYPE_ORG, filename_o);
    ASSERT_TRUE(signet != NULL) << "Failure to create organizational signet.";

    type = dime_sgnt_type_get(signet);
    ASSERT_EQ(SIGNET_TYPE_ORG, type) << "Corrupted signet type.";

    priv_signkey = dime_keys_signkey_fetch(filename_o);
    ASSERT_TRUE(priv_signkey != NULL) << "Failure to fetch private signing key from file.";

    res = _ed25519_sign_data((const unsigned char *)to_sign, strlen(to_sign), priv_signkey, sigbuf);
    ASSERT_EQ(0, res) << "Failure to sign data buffer.";

    pub_signkey = dime_sgnt_signkey_fetch(signet);
    ASSERT_TRUE(pub_signkey != NULL) << "Failure to fetch public signing key from signet.";

    res = _ed25519_verify_sig((const unsigned char *)to_sign, strlen(to_sign), pub_signkey, sigbuf);
    ASSERT_EQ(1, res) << "Failure to verify signature";

    priv_enckey = dime_keys_enckey_fetch(filename_o);
    ASSERT_TRUE(priv_enckey != NULL) << "Failure to fetch private encryption key from file.";

    enc1_pub = _serialize_ec_pubkey(priv_enckey, &enc1_size);
    ASSERT_TRUE(enc1_pub != NULL) << "Failure to serialize public portion of the private encryption key.";

    pub_enckey = dime_sgnt_enckey_fetch(signet);
    ASSERT_TRUE(pub_enckey != NULL) << "Failure to fetch public encryption key from signet.";

    enc2_pub = _serialize_ec_pubkey(pub_enckey, &enc2_size);
    ASSERT_EQ(enc1_size, enc2_size) << "Corrupted public encryption key size.";
    ASSERT_EQ(0, memcmp(enc1_pub, enc2_pub, enc1_size)) << "Corrupted public encryption key data.";

    _free_ed25519_key(priv_signkey);
    _free_ed25519_key(pub_signkey);
    _free_ec_key(priv_enckey);
    _free_ec_key(pub_enckey);
    free(enc1_pub);
    free(enc2_pub);
    dime_sgnt_signet_destroy(signet);

    /* creating ssr signet with keys */
    signet = dime_sgnt_signet_create_w_keys(SIGNET_TYPE_SSR, filename_s);
    ASSERT_TRUE(signet != NULL) << "Failure to create SSR.";

    type = dime_sgnt_type_get(signet);
    ASSERT_EQ(SIGNET_TYPE_SSR, type) << "Corrupted signet type.";

    priv_signkey = dime_keys_signkey_fetch(filename_s);
    ASSERT_TRUE(priv_signkey != NULL) << "Failure to fetch private signing key from file.";

    res = _ed25519_sign_data((const unsigned char *)to_sign, strlen(to_sign), priv_signkey, sigbuf);
    ASSERT_EQ(0, res) << "Failure to sign data buffer.";

    pub_signkey = dime_sgnt_signkey_fetch(signet);
    ASSERT_TRUE(pub_signkey != NULL) << "Failure to fetch public signing key from signet.";

    res = _ed25519_verify_sig((const unsigned char *)to_sign, strlen(to_sign), pub_signkey, sigbuf);
    ASSERT_EQ(1, res) << "Failure to verify signature";

    priv_enckey = dime_keys_enckey_fetch(filename_s);
    ASSERT_TRUE(priv_enckey != NULL) << "Failure to fetch private encryption key from file.";

    enc1_pub = _serialize_ec_pubkey(priv_enckey, &enc1_size);
    ASSERT_TRUE(enc1_pub != NULL) << "Failure to serialize public portion of the private encryption key.";

    pub_enckey = dime_sgnt_enckey_fetch(signet);
    ASSERT_TRUE(pub_enckey != NULL) << "Failure to fetch public encryption key from signet.";

    enc2_pub = _serialize_ec_pubkey(pub_enckey, &enc2_size);
    ASSERT_EQ(enc1_size, enc2_size) << "Corrupted public encryption key size.";
    ASSERT_EQ(0, memcmp(enc1_pub, enc2_pub, enc1_size)) << "Corrupted public encryption key data.";

    _free_ed25519_key(priv_signkey);
    _free_ed25519_key(pub_signkey);
    _free_ec_key(priv_enckey);
    _free_ec_key(pub_enckey);
    free(enc1_pub);
    free(enc2_pub);
    dime_sgnt_signet_destroy(signet);

/*creating invalid signet types*/
    signet = dime_sgnt_signet_create_w_keys(SIGNET_TYPE_ERROR, filename_w);
    ASSERT_EQ(NULL, signet) << "Unintended creation of signet with invalid type SIGNET_TYPE_ERROR.";
    ASSERT_EQ(-1, access(filename_w, F_OK)) << "Unintended creation of keys file for signet with invalid type SIGNET_TYPE_ERROR.";

    signet = dime_sgnt_signet_create_w_keys(static_cast<signet_type_t>(31), filename_w);
    ASSERT_EQ(NULL, signet) << "Unintended creation of signet with invalid type 31.";
    ASSERT_EQ(-1, access(filename_w, F_OK)) << "Unintended creation of keys file for signet with invalid type 31.";
}

TEST(DIME, check_signet_modification)
{
    const char *phone1 = "1SOMENUMBER", *phone2 = "15124123529",
        *name1 = "check undef", *data1 = "undef data",
        *name2 = "check name", *data2 = "check check", *id = "thisid";
    char *idout;
    int res, count;
    signet_t *signet;
    size_t data_size;
    unsigned char *data;

    signet = dime_sgnt_signet_create(SIGNET_TYPE_ORG);
    ASSERT_TRUE(signet != NULL) << "Failure to create signet.";

    res = dime_sgnt_field_undefined_create(signet, strlen(name1), (const unsigned char *)name1, strlen(data1), (const unsigned char *)data1);
    ASSERT_EQ(0, res) << "Failure to create undefined field.";

    res = dime_sgnt_fid_exists(signet, SIGNET_ORG_UNDEFINED);
    ASSERT_EQ(1, res) << "Failure to confirm existence of undefined field.";

    count = dime_sgnt_fid_count_get(signet, SIGNET_ORG_UNDEFINED);
    ASSERT_EQ(1, count) << "Failure to count number of undefined fields.";

    data = dime_sgnt_field_undefined_fetch(signet, strlen(name1), (const unsigned char *)name1, &data_size);
    ASSERT_TRUE(data != NULL) << "Failure to fetch undefined field.";
    ASSERT_EQ(data_size, strlen(data1)) << "Corrupted undefined field size.";
    ASSERT_EQ(0, memcmp(data, (unsigned char *)data1, data_size)) << "Corrupted undefined field data.";

    free(data);

    res = dime_sgnt_field_undefined_create(signet, strlen(name2), (const unsigned char *)name2, strlen(data2), (const unsigned char *)data2);
    ASSERT_EQ(0, res) << "Failure to create undefined field.";

    count = dime_sgnt_fid_count_get(signet, SIGNET_ORG_UNDEFINED);
    ASSERT_EQ(2, count) << "Failure to count number of undefined fields.";

    data = dime_sgnt_field_undefined_fetch(signet, strlen(name2), (const unsigned char*)name2, &data_size);
    ASSERT_TRUE(data != NULL) << "Failure to fetch undefined field.";
    ASSERT_EQ(data_size, strlen(data2)) << "Corrupted undefined field size.";
    ASSERT_EQ(0, memcmp(data, (unsigned char *)data2, data_size)) << "Corrupted undefined field data.";

    free(data);

    res = dime_sgnt_id_set(signet, strlen(id), (unsigned char const *)id);
    ASSERT_EQ(0, res) << "Failed to set id of signet.";

    idout = dime_sgnt_id_fetch(signet);
    ASSERT_TRUE(idout != NULL) << "Failed o retrieve id of signet.";

    res = (strlen(idout) == strlen(id));
    ASSERT_EQ(1, res) << "Setting and retrieving signet id corrupted its size.";

    res = memcmp(idout, id, strlen(id));
    ASSERT_EQ(0, res) << "Setting and retrieving signet id corrupted its data.";

    res = dime_sgnt_field_undefined_remove(signet, strlen(name1), (const unsigned char *)name1);
    ASSERT_EQ(0, res) << "Failure to remove undefined field.";

    count = dime_sgnt_fid_count_get(signet, SIGNET_ORG_UNDEFINED);
    ASSERT_EQ(1, count) << "Failure to count number of undefined fields.";

    data = dime_sgnt_field_undefined_fetch(signet, strlen(name1), (const unsigned char *)name1, &data_size);
    ASSERT_EQ(NULL, data) << "Unintended existence of undefined field after removal.";

    data = dime_sgnt_field_undefined_fetch(signet, strlen(name2), (const unsigned char *)name2, &data_size);
    ASSERT_TRUE(data != NULL) << "Failure to fetch undefined field.";
    ASSERT_EQ(data_size, strlen(data2)) << "Corrupted undefined field size.";
    ASSERT_EQ(0, memcmp(data, (unsigned char *)data2, data_size)) << "Corrupted undefined field data.";

    free(data);

    res = dime_sgnt_field_defined_set(signet, SIGNET_ORG_PHONE, strlen(phone1), (const unsigned char *)phone1);
    ASSERT_EQ(0, res) << "Failure to create phone number field.";

    res = dime_sgnt_fid_exists(signet, SIGNET_ORG_PHONE);
    ASSERT_EQ(1, res) << "Failure to confirm existence of phone number field.";

    count = dime_sgnt_fid_count_get(signet, SIGNET_ORG_PHONE);
    ASSERT_EQ(1, count) << "Failure to count number of org phone fields.";

    data = dime_sgnt_fid_num_fetch(signet, SIGNET_ORG_PHONE, 1, &data_size);
    ASSERT_TRUE(data != NULL) << "Failure to fetch phone number field";
    ASSERT_EQ(data_size, strlen(phone1)) << "Corrupted phone number field size.";
    ASSERT_EQ(0, memcmp(data, (unsigned char *)phone1, data_size)) << "Corrupted phone number field data.";

    free(data);

    res = dime_sgnt_field_defined_create(signet, SIGNET_ORG_PHONE, strlen(phone2), (const unsigned char *)phone2);
    ASSERT_EQ(0, res) << "Failure to create phone number field.";

    count = dime_sgnt_fid_count_get(signet, SIGNET_ORG_PHONE);
    ASSERT_EQ(2, count) << "Failure to count number of org phone fields.";

    data = dime_sgnt_fid_num_fetch(signet, SIGNET_ORG_PHONE, 2, &data_size);
    ASSERT_TRUE(data != NULL) << "Failure to fetch phone number field.";
    ASSERT_EQ(data_size, strlen(phone2)) << "Corrupted phone number field size.";
    ASSERT_EQ(0, memcmp(data, (unsigned char *)phone2, data_size)) << "Corrupted phone number field data.";

    free(data);

    res = dime_sgnt_fid_num_remove(signet, SIGNET_ORG_PHONE, 1);
    ASSERT_EQ(0, res) << "Failure to remove phone number field.";

    data = dime_sgnt_fid_num_fetch(signet, SIGNET_ORG_PHONE, 1, &data_size);
    ASSERT_TRUE(data != NULL) << "Failure to fetch phone number field.";
    ASSERT_EQ(data_size, strlen(phone2)) << "Corrupted phone number field size.";
    ASSERT_EQ(0, memcmp(data, (unsigned char *)phone2, data_size)) << "Corrupted phone number field data.";

    free(data);
    dime_sgnt_signet_destroy(signet);
}

#if 0
static void signet_dump(const signet_t *signet) {

    fprintf(stderr, "signet size: %d\nsignet data: ", signet->size);
    for(size_t i = 0; i < signet->size; ++i) {
        fprintf(stderr, "%d ", signet->data[i]);
    }
    fprintf(stderr, "\nfield indexes: ");
    for(size_t i = 0; i <= SIGNET_FID_MAX; ++i) {
        fprintf(stderr, "%d ", signet->fields[i]);
    }
    fprintf(stderr, "\n");
}
#endif

TEST(DIME, check_signet_parsing)
{
    char *b64_sigone, *b64_sigtwo;
    const char *filename = DIME_CHECK_OUTPUT_PATH "check.signet", *name = "some name",
               *phone1 = "phonenum1", *phone2 = "someotherphone",
               *name1 = "field name", *name2 = "other field name",
               *name3 = "last name of field", *data1 = "some field",
               *data2 = "check fields", *data3 = "check check check";
    int res;
    signet_t *sigone, *sigtwo;
    uint32_t len;
    unsigned char *ser_sigone, *ser_sigtwo;

    sigone = dime_sgnt_signet_create(SIGNET_TYPE_ORG);
    dime_sgnt_field_defined_create(sigone, SIGNET_ORG_NAME, strlen(name), (const unsigned char *)name);
    dime_sgnt_field_defined_create(sigone, SIGNET_ORG_PHONE, strlen(phone1), (const unsigned char *)phone1);
    dime_sgnt_field_defined_create(sigone, SIGNET_ORG_PHONE, strlen(phone2), (const unsigned char *)phone2);
    dime_sgnt_field_undefined_create(sigone, strlen(name1), (const unsigned char *)name1, strlen(data1), (const unsigned char *)data1);
    dime_sgnt_field_undefined_create(sigone, strlen(name2), (const unsigned char *)name2, strlen(data2), (const unsigned char *)data2);
    dime_sgnt_field_undefined_create(sigone, strlen(name3), (const unsigned char *)name3, strlen(data3), (const unsigned char *)data3);

    ser_sigone = dime_sgnt_signet_binary_serialize(sigone, &len);
    ASSERT_TRUE(ser_sigone != NULL) << "Failure to serialize signet.";

    sigtwo = dime_sgnt_signet_binary_deserialize(ser_sigone, len);
    ASSERT_TRUE(sigtwo != NULL) << "Failure to deserialize signet.";

    ser_sigtwo = dime_sgnt_signet_binary_serialize(sigtwo, &len);
    ASSERT_TRUE(ser_sigtwo != NULL) << "Failure to re-serialized the signet.";
    ASSERT_EQ(0, memcmp(ser_sigone, ser_sigtwo, len)) << "Corrupted serialized signet data.";

    free(ser_sigone);
    free(ser_sigtwo);
    dime_sgnt_signet_destroy(sigtwo);

    b64_sigone = dime_sgnt_signet_b64_serialize(sigone);
    ASSERT_TRUE(b64_sigone != NULL) << "Failure to convert signet to base64 encoded string.";

    sigtwo = dime_sgnt_signet_b64_deserialize(b64_sigone);
    ASSERT_TRUE(sigtwo != NULL) << "Failure to convert base64 string to signet.";

    b64_sigtwo = dime_sgnt_signet_b64_serialize(sigtwo);
    ASSERT_TRUE(b64_sigtwo != NULL) << "Failure to re-convert signet to base64 encoded string.";
    ASSERT_EQ(0, strcmp(b64_sigone, b64_sigtwo)) << "Corrupted base64 string signet data.";

    free(b64_sigtwo);
    dime_sgnt_signet_destroy(sigtwo);

    res = dime_sgnt_file_create(sigone, filename);
    ASSERT_EQ(0, res) << "Failure to write signet to file.";

    sigtwo = dime_sgnt_signet_load(filename);
    ASSERT_TRUE(sigtwo != NULL) << "Failure to read signet from file.";

    res = dime_sgnt_file_create(sigtwo, filename);
    ASSERT_EQ(0, res) << "Failure to re-write signet to file.";

    b64_sigtwo = _read_pem_data(filename, SIGNET_ORG, 1);
    ASSERT_TRUE(b64_sigtwo != NULL) << "Failure to read b64 string from signet file.";
    ASSERT_EQ(0, strcmp(b64_sigone, b64_sigtwo)) << "Corrupted signet file data.";

    free(b64_sigone);
    free(b64_sigtwo);
    dime_sgnt_signet_destroy(sigtwo);
    dime_sgnt_signet_destroy(sigone);
}

TEST(DIME, check_signet_validation)
{
    const char *org_keys = DIME_CHECK_OUTPUT_PATH "check_org.keys",
    	*user_keys = DIME_CHECK_OUTPUT_PATH "check_user.keys",
		*newuser_keys = DIME_CHECK_OUTPUT_PATH "check_newuser.keys";
    ED25519_KEY *orgkey, *userkey, **keys_obj;
    int res;
    signet_state_t state;
    signet_t *org_signet, *user_signet, *newuser_signet, *split, *split2;
    unsigned char **org_signet_sign_keys;
    size_t keysnum = 1;

    _crypto_init();
    //create org signet and keys file
    org_signet = dime_sgnt_signet_create_w_keys(SIGNET_TYPE_ORG, org_keys);
    ASSERT_TRUE(org_signet != NULL) << "Failure to create signet with keys file.";
    //retrieve org private signing key
    orgkey = dime_keys_signkey_fetch(org_keys);
    //ASSERT_TRUE(orgkey != NULL) << "Failure to fetch private signing key from keys file.";
    //sign org cryptographic signet signature
    res = dime_sgnt_sig_crypto_sign(org_signet, orgkey);
    //ASSERT_EQ(0, res) << "Failure to create organizational cryptographic signet signature.\n";
    //retrieve the list of all org signet-signing keys (note we're using this instead of retrieving the list of POKs from the dime record just to have a list of keys, 1 of which will be valid.)
    keys_obj = dime_sgnt_signkeys_signet_fetch(org_signet);
    res = dime_sgnt_sig_crypto_sign(org_signet, orgkey);
    ASSERT_TRUE(keys_obj != NULL) << "Failure to retrieve organizational signet signing keys.";
    //convert ed25519 pointer chain to serialized ed25519 public key pointer chain

    for(size_t i = 0; keys_obj[i]; ++i) {
        ++keysnum;
    }

    org_signet_sign_keys = (unsigned char **)malloc(sizeof(unsigned char *) * keysnum);
    memset(org_signet_sign_keys, 0, sizeof(unsigned char *) * keysnum);

    for(size_t i = 0; keys_obj[i]; ++i) {

        org_signet_sign_keys[i] = (unsigned char *)malloc(ED25519_KEY_SIZE);
        memcpy(org_signet_sign_keys[i], keys_obj[i]->public_key, ED25519_KEY_SIZE);
    }
    //verify that the org crypto signet is valid
    state = dime_sgnt_validate_all(org_signet, NULL, NULL, (const unsigned char **)org_signet_sign_keys);
    ASSERT_EQ(SS_CRYPTO, state) << "Failure to correctly validate organizational signet as a cryptographic signet.";
    //sign org full signet signature
    res = dime_sgnt_sig_full_sign(org_signet, orgkey);
    ASSERT_EQ(0, res) << "Failure to create organizational full signet signature.";
    //verify that the org full signet is valid
    state = dime_sgnt_validate_all(org_signet, NULL, NULL, (const unsigned char **)org_signet_sign_keys);
    ASSERT_EQ(SS_FULL, state) << "Failure to correctly validate organizational signet as a full signet.";
    //set organizational signet id
    res = dime_sgnt_id_set(org_signet, strlen("test_org_signet"), (const unsigned char *)"test_org_signet");
    ASSERT_EQ(0, res) << "Failure to set organizational signet id.";
    //sign identified signet signature
    res = dime_sgnt_sig_id_sign(org_signet, orgkey);
    ASSERT_EQ(0, res) << "Failure to create organizational identifiable signet signature field.";
    //verify that the org signet is a valid identifiable signet
    state = dime_sgnt_validate_all(org_signet, NULL, NULL, (const unsigned char **)org_signet_sign_keys);
    ASSERT_EQ(SS_ID, state) << "Failure to correctly validate organizational signet as an identifiable signet.";
    //create ssr signet and user keys file
    user_signet = dime_sgnt_signet_create_w_keys(SIGNET_TYPE_SSR, user_keys);
    ASSERT_TRUE(user_signet != NULL) << "Failure to create ssr with keys file.";
    //retrieve user private signing key
    userkey = dime_keys_signkey_fetch(user_keys);
    ASSERT_TRUE(userkey != NULL) << "Failure to fetch user's private signing key from keys file.";
    //sign the ssr signature with user keys
    res = dime_sgnt_sig_ssr_sign(user_signet, userkey);
    ASSERT_EQ(0, res) << "Failure to sign ssr with the user's private signing key.";
    //verify that the signet is a valid ssr
    state = dime_sgnt_validate_all(user_signet, NULL, NULL, NULL);
    ASSERT_EQ(SS_SSR, state) << "Failure to correctly validate ssr.";
    //sign ssr with org signing key
    res = dime_sgnt_sig_crypto_sign(user_signet, orgkey);
    ASSERT_EQ(0, res) << "Failure to sign ssr into a user cryptographic signet using organizational private signing key.";
    //verify that the signet is now a valid user core signet
    state = dime_sgnt_validate_all(user_signet, NULL, org_signet, NULL);
    ASSERT_EQ(SS_CRYPTO, state) << "Failure to correctly validate user cryptographic signet.";
    //sign the full signature with org key
    res = dime_sgnt_sig_full_sign(user_signet, orgkey);
    ASSERT_EQ(0, res) << "Failure to sign user signet with the full signet signature.";
    //verify that the user signet is now a valid core signet
    state = dime_sgnt_validate_all(user_signet, NULL, org_signet, NULL);
    ASSERT_EQ(SS_FULL, state) << "Failure to correctly validate user full signet.";
    //set user signet id (address)
    res = dime_sgnt_id_set(user_signet, strlen("user@test.org"), (const unsigned char *)"user@test.org");
    ASSERT_EQ(0, res) << "Failure to set user signet id.";
    //sign the user signature with the identifiable signet signature
    res = dime_sgnt_sig_id_sign(user_signet, orgkey);
    ASSERT_EQ(0, res) << "Failure to sign user signet with the identifiable signet signature.";
    //verify that the user signet is a valid full signet
    state = dime_sgnt_validate_all(user_signet, NULL, org_signet, NULL);
    ASSERT_EQ(SS_ID, state) << "Failure to correctly validate user identifiable signet.";
    //create new ssr and keys file
    newuser_signet = dime_sgnt_signet_create_w_keys(SIGNET_TYPE_SSR, newuser_keys);
    ASSERT_TRUE(newuser_signet != NULL) << "Failure to create ssr with keys file.";
    //sign the new ssr with a chain of custody signature using the user's old signing key
    res = dime_sgnt_sig_coc_sign(newuser_signet, userkey);
    ASSERT_EQ(0, res) << "Failure to create the chain of custody signature.";

    _free_ed25519_key(userkey);

    //retrieve user's new private signing key
    userkey = dime_keys_signkey_fetch(newuser_keys);
    ASSERT_TRUE(userkey != NULL) << "Failure to retrieve user's new private signing key.";
    //perform all the signatures on the new ssr (adding an address as the id is required for the identifiable signet signature.
    dime_sgnt_sig_ssr_sign(newuser_signet, userkey);
    dime_sgnt_sig_crypto_sign(newuser_signet, orgkey);
    dime_sgnt_sig_full_sign(newuser_signet, orgkey);
    dime_sgnt_id_set(newuser_signet, strlen("user@test.com"), (const unsigned char *)"user@test.com");
    dime_sgnt_sig_id_sign(newuser_signet, orgkey);
    //Confirm that without using the previous signet to verify the chain of custody, the signet validation returns broken chain of custody
    state = dime_sgnt_validate_all(newuser_signet, NULL, org_signet, NULL);
    ASSERT_EQ(SS_BROKEN_COC, state) << "Failure to invalidate signet due to no parent signet being provided to validate chain of custody signature.";
    //Config that by using the previous signet to verify chain of custody, the new user signet is validate as identifiable signet.
    //TODO it may be necessary to test intermediate states with presence of chain of custody also
    state = dime_sgnt_validate_all(newuser_signet, user_signet, org_signet, NULL);
    ASSERT_EQ(SS_ID, state) << "Failure to validate an identifiable signet with a chain of custody signature.";

    //Now, lets test splitting.
    split = dime_sgnt_signet_full_split(newuser_signet);
    ASSERT_TRUE(split != NULL) << "Failed to split identifiable user signet into a full user signet.";

    dime_sgnt_signet_destroy(newuser_signet);

    state = dime_sgnt_validate_all(split, user_signet, org_signet, NULL);
    ASSERT_EQ(SS_FULL, state) << "Failure to validate full user signet with chain of custody.";

    split2 = dime_sgnt_signet_crypto_split(split);
    ASSERT_TRUE(split2 != NULL) << "Failed to split full user signet into a cryptographic user signet.";

    dime_sgnt_signet_destroy(split);

    state = dime_sgnt_validate_all(split2, user_signet, org_signet, NULL);
    ASSERT_EQ(SS_CRYPTO, state) << "Failure to validate cryptographic user signet with chain of custody.";

    dime_sgnt_signet_destroy(split2);

    _ptr_chain_free(org_signet_sign_keys);
    _free_ed25519_key_chain(keys_obj);
    _free_ed25519_key(orgkey);
    _free_ed25519_key(userkey);
    dime_sgnt_signet_destroy(user_signet);
    dime_sgnt_signet_destroy(org_signet);
}

TEST(DIME, check_signet_sok)
{

    ED25519_KEY *sok, *sok_from_signet;
    int res;
    signet_t *signet;

    _crypto_init();

    signet = dime_sgnt_signet_create(SIGNET_TYPE_USER);
    ASSERT_TRUE(signet != NULL) << "Failure to create user signet.";

    sok = generate_ed25519_keypair();
    ASSERT_TRUE(sok != NULL) << "Failure to generate ed25519 key pair.";

    res = dime_sgnt_sok_create(signet, sok, (unsigned char) SIGNKEY_DEFAULT_FORMAT, (SIGNET_SOK_SIGNET | SIGNET_SOK_MSG | SIGNET_SOK_TLS | SIGNET_SOK_SOFTWARE) );
    ASSERT_EQ(-1, res) << "Error cause by inserting a SOK inside a user signet.";

    dime_sgnt_signet_destroy(signet);

    signet = dime_sgnt_signet_create(SIGNET_TYPE_ORG);
    ASSERT_TRUE(signet != NULL) << "Failure to create organizational signet.";

    res = dime_sgnt_sok_create(signet, sok, 214, (SIGNET_SOK_SIGNET | SIGNET_SOK_MSG) );
    ASSERT_EQ(-1, res) << "Error caused by inserting a SOK with an invalid format.";

    res = dime_sgnt_sok_create(signet, sok, (unsigned char) SIGNKEY_DEFAULT_FORMAT, (SIGNET_SOK_SIGNET | SIGNET_SOK_MSG | SIGNET_SOK_TLS | SIGNET_SOK_SOFTWARE) );
    ASSERT_EQ(0, res) << "Failure to add a SOK field to signet.";

    sok_from_signet = dime_sgnt_sok_num_fetch(signet, 1);
    ASSERT_TRUE(sok_from_signet != NULL) << "Failure to fetch SOK from signet.";

    res = memcmp(sok->public_key, sok_from_signet->public_key, ED25519_KEY_SIZE);
    ASSERT_EQ(0, res) << "SOK was corrupted during inserting and fetching into and from the signet.";

    free_ed25519_key(sok_from_signet);
    dime_sgnt_signet_destroy(signet);
}

TEST(DIME, check_signet_multi_signkey)
{

    EC_KEY *eckey;
    ED25519_KEY *keys[5], **fetched;
    int res;
    signet_t *signet;

    _crypto_init();

    for(int i = 0; i < 5; ++i) {
        keys[i] = generate_ed25519_keypair();
    }

    eckey = generate_ec_keypair();

    signet = dime_sgnt_signet_create(SIGNET_TYPE_ORG);
    ASSERT_TRUE(signet != NULL) << "Failed to create organizational signet.";

    res = dime_sgnt_signkey_set(signet, keys[0], SIGNKEY_DEFAULT_FORMAT);
    ASSERT_EQ(0, res) << "Failed to set signet POK.";

    res += dime_sgnt_sok_create(signet, keys[1], SIGNKEY_DEFAULT_FORMAT, SIGNET_SOK_SIGNET);
    ASSERT_EQ(0, res) << "Failed to create SOK 1.";

    res += dime_sgnt_sok_create(signet, keys[2], SIGNKEY_DEFAULT_FORMAT, SIGNET_SOK_MSG);
    ASSERT_EQ(0, res) << "Failed to create SOK 2.";

    res += dime_sgnt_sok_create(signet, keys[3], SIGNKEY_DEFAULT_FORMAT, SIGNET_SOK_TLS);
    ASSERT_EQ(0, res) << "Failed to create SOK 3.";

    res += dime_sgnt_sok_create(signet, keys[4], SIGNKEY_DEFAULT_FORMAT, SIGNET_SOK_SOFTWARE);
    ASSERT_EQ(0, res) << "Failed to create SOK 4.";

    res = dime_sgnt_enckey_set(signet, eckey, 0);
    ASSERT_EQ(0, res) << "Failed to set signet encryption key.";

    free_ec_key(eckey);

    res = dime_sgnt_sig_crypto_sign(signet, keys[0]);
    ASSERT_EQ(0, res) << "Failed to sign organizational signet with its private POK.";

    fetched = dime_sgnt_signkeys_signet_fetch(signet);
    ASSERT_TRUE(fetched != NULL) << "Failed to fetch signing keys.";
    ASSERT_TRUE(fetched[0] != NULL) << "Failed to fetch signing keys.";
    ASSERT_TRUE(fetched[1] != NULL) << "Failed to fetch signing keys.";
    ASSERT_EQ(NULL, fetched[2]) << "Failed to fetch signing keys.";

    res = memcmp(fetched[0]->public_key, keys[0]->public_key, ED25519_KEY_SIZE);
    ASSERT_EQ(0, res) << "POK was corrupted.";

    res = memcmp(fetched[1]->public_key, keys[1]->public_key, ED25519_KEY_SIZE);
    ASSERT_EQ(0, res) << "SOK 1 was corrupted.";

    free_ed25519_key_chain(fetched);
    fetched = NULL;
    fetched = dime_sgnt_signkeys_msg_fetch(signet);
    ASSERT_TRUE( (fetched != NULL) &&
                       (fetched[0] != NULL) &&
                       (fetched[1] != NULL) &&
                       (fetched[2] == NULL)) << "Failed to fetch signing keys.";
    res = memcmp(fetched[0]->public_key, keys[0]->public_key, ED25519_KEY_SIZE);
    ASSERT_EQ(0, res) << "POK was corrupted.";

    res = memcmp(fetched[1]->public_key, keys[2]->public_key, ED25519_KEY_SIZE);
    ASSERT_EQ(0, res) << "SOK 2 was corrupted.";

    free_ed25519_key_chain(fetched);
    fetched = NULL;

    fetched = dime_sgnt_signkeys_tls_fetch(signet);
    ASSERT_TRUE( (fetched != NULL) &&
                       (fetched[0] != NULL) &&
                       (fetched[1] != NULL) &&
                       (fetched[2] == NULL)) << "Failed to fetch signing keys.";

    res = memcmp(fetched[0]->public_key, keys[0]->public_key, ED25519_KEY_SIZE);
    ASSERT_EQ(0, res) << "POK was corrupted.";

    res = memcmp(fetched[1]->public_key, keys[3]->public_key, ED25519_KEY_SIZE);
    ASSERT_EQ(0, res) << "SOK 3 was corrupted.";

    free_ed25519_key_chain(fetched);
    fetched = NULL;

    fetched = dime_sgnt_signkeys_software_fetch(signet);
    ASSERT_TRUE( (fetched != NULL) &&
                       (fetched[0] != NULL) &&
                       (fetched[1] != NULL) &&
                       (fetched[2] == NULL)) << "Failed to fetch signing keys.";

    res = memcmp(fetched[0]->public_key, keys[0]->public_key, ED25519_KEY_SIZE);
    ASSERT_EQ(0, res) << "POK was corrupted.";

    res = memcmp(fetched[1]->public_key, keys[4]->public_key, ED25519_KEY_SIZE);
    ASSERT_EQ(0, res) << "SOK 4 was corrupted.";

    free_ed25519_key_chain(fetched);
    fetched = NULL;

    for(int i = 0; i < 5; ++i) {
        free_ed25519_key(keys[i]);
    }

    dime_sgnt_signet_destroy(signet);
}

TEST(DIME, check_signet_fingerprint)
{
    char *fp1, *fp2;
    int res;
    signet_t *signet;

    _crypto_init();

    signet = dime_sgnt_signet_create_w_keys(SIGNET_TYPE_USER, DIME_CHECK_OUTPUT_PATH "fp_test.keys");
    ASSERT_TRUE(signet != NULL) << "Failed to create signet with keys.";

    fp1 = dime_sgnt_fingerprint_ssr(signet);
    ASSERT_TRUE(signet != NULL) << "Failed to fingerprint signet.";

    fp2 = dime_sgnt_fingerprint_ssr(signet);
    ASSERT_TRUE(signet != NULL) << "Failed to fingerprint signet.";

    ASSERT_EQ(strlen(fp1), strlen(fp2)) << "Inconsistent fingerprinting.";

    res = memcmp(fp1, fp2, strlen(fp1));
    ASSERT_EQ(0, res) << "Inconsistent fingerprinting.";

    free(fp2);

    fp2 = dime_sgnt_fingerprint_crypto(signet);
    ASSERT_TRUE(signet != NULL) << "Failed to fingerprint signet.";

    ASSERT_EQ(strlen(fp1),  strlen(fp2)) << "Inconsistent fingerprinting.";

    res = memcmp(fp1, fp2, strlen(fp1));
    ASSERT_EQ(0, res) << "Inconsistent fingerprinting.";

    free(fp2);

    fp2 = dime_sgnt_fingerprint_full(signet);
    ASSERT_TRUE(signet != NULL) << "Failed to fingerprint signet.";

    ASSERT_EQ(strlen(fp1), strlen(fp2)) << "Inconsistent fingerprinting.";

    res = memcmp(fp1, fp2, strlen(fp1));
    ASSERT_EQ(0, res) << "Inconsistent fingerprinting.";

    free(fp2);

    fp2 = dime_sgnt_fingerprint_id(signet);
    ASSERT_TRUE(signet != NULL) << "Failed to fingerprint signet.";

    ASSERT_EQ(strlen(fp1), strlen(fp2)) << "Inconsistent fingerprinting.";

    res = memcmp(fp1, fp2, strlen(fp1));
    ASSERT_EQ(0, res) << "Inconsistent fingerprinting.";

    free(fp2);

    res = dime_sgnt_id_set(signet, 7, (const unsigned char *)"some id");
    ASSERT_EQ(0, res) << "Failed to set signet id.";

    fp2 = dime_sgnt_fingerprint_id(signet);
    ASSERT_TRUE(signet != NULL) << "Failed to fingerprint signet.";

    ASSERT_EQ(strlen(fp1), strlen(fp2)) << "Inconsistent fingerprinting.";

    res = memcmp(fp1, fp2, strlen(fp1));
    ASSERT_NE(0, res) << "Either a sha512 hash collision occurred or fingerprinting is broken.";

    free(fp2);
    free(fp1);
    dime_sgnt_signet_destroy(signet);
}

TEST(DIME, check_signet_signature_verification)
{
    char *fp;
    const char *org_keys = DIME_CHECK_OUTPUT_PATH "check_org.keys";
    unsigned char signature[ED25519_SIG_SIZE];
    ED25519_KEY *orgkey;
    int res;
    signet_t *org_signet;

    _crypto_init();

    org_signet = dime_sgnt_signet_create_w_keys(SIGNET_TYPE_ORG, org_keys);
    ASSERT_TRUE(org_signet != NULL) << "Failure to create signet with keys file.";

    orgkey = dime_keys_signkey_fetch(org_keys);
    ASSERT_TRUE(orgkey != NULL) << "Failure to fetch private signing key from keys file.";

    res = dime_sgnt_sig_crypto_sign(org_signet, orgkey);
    ASSERT_EQ(0, res) << "Failure to create organizational cryptographic signet signature.";

    fp = dime_sgnt_fingerprint_crypto(org_signet);
    ASSERT_TRUE(fp != NULL) << "Failed to fingerprint organiational signet.";

    res = ed25519_sign_data((const unsigned char *)fp, strlen(fp), orgkey, signature);
    ASSERT_EQ(0, res) << "Failed to provided data with ed25519 key.";

    res = dime_sgnt_msg_sig_verify(org_signet, signature, (const unsigned char *)fp, strlen(fp));
    ASSERT_EQ(1, res) << "Failed to verify signature using signet.";
}
