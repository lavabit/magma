extern "C" {
#include "dime/signet/keys.h"
#include "dime/signet/signet.h"
#include "dime/dmessage/parse.h"
#include "dime/dmessage/crypto.h"
}
#include "gtest/gtest.h"
#include "error-assert.h"

/**
 * Demonstrates how a message travels from the author to the recipient.
 */
TEST(DIME, message_encryption_and_decryption)
{
    EC_KEY *auth_enckey, *orig_enckey, *dest_enckey, *recp_enckey;
    ED25519_KEY *auth_signkey, *orig_signkey, *dest_signkey, *recp_signkey;
    const char *auth = "ivan@darkmail.info", *orig = "darkmail.info", *dest = "lavabit.com", *recp = "ryan@lavabit.com";
    const char *auth_keys = ".out/auth.keys", *orig_keys = ".out/orig.keys", *dest_keys = ".out/dest.keys", *recp_keys = ".out/recp.keys";
    const char *common_date = "12 minutes ago";
    const char *common_to = "Ryan <ryan@lavabit.com>";
    const char *common_from = "Ivan <ivan@darkmail.info>";
    const char *common_subject = "Mr.Watson - Come here - I want to see you";
    const char *common_organization = "Lavabit";
    const char *other_headers = "SECRET METADATA\r\n";
    const char *display = "This is a test\r\nCan you read this?\r\n";
    dmime_kek_t orig_kek, dest_kek, recp_kek;
    dmime_message_t *message;
    dmime_object_t *draft, *at_orig, *at_dest, *at_recp;
    int res;
    signet_t *signet_auth, *signet_orig, *signet_dest, *signet_recp;
    size_t from_auth_size, from_orig_size, from_dest_size;
    unsigned char *from_auth_bin, *from_orig_bin, *from_dest_bin;

    ASSERT_DIME_NO_ERROR();
    _crypto_init();
    ASSERT_DIME_NO_ERROR();

    memset(&orig_kek, 0, sizeof(dmime_kek_t));
    memset(&dest_kek, 0, sizeof(dmime_kek_t));
    memset(&recp_kek, 0, sizeof(dmime_kek_t));


    //create domain signets
    signet_orig = dime_sgnt_signet_create_w_keys(SIGNET_TYPE_ORG, orig_keys);
    ASSERT_TRUE(signet_orig != NULL) << "Failed to create origin signet.";
    signet_dest = dime_sgnt_signet_create_w_keys(SIGNET_TYPE_ORG, dest_keys);
    ASSERT_TRUE(signet_dest != NULL) << "Failed to create destination signet.";

    //create user signet signing requests
    signet_auth = dime_sgnt_signet_create_w_keys(SIGNET_TYPE_SSR, auth_keys);
    ASSERT_TRUE(signet_auth != NULL) << "Failed to create author signet.";
    signet_recp = dime_sgnt_signet_create_w_keys(SIGNET_TYPE_SSR, recp_keys);
    ASSERT_TRUE(signet_recp != NULL) << "Failed to create recipeint signet.";
    ASSERT_DIME_NO_ERROR();

    // retrieve all signing and encryption private keys ahead of time
    orig_enckey = dime_keys_enckey_fetch(orig_keys);
    ASSERT_TRUE(orig_enckey != NULL) << "Failed to retrieve origin encryption keys.";
    dest_enckey = dime_keys_enckey_fetch(dest_keys);
    ASSERT_TRUE(dest_enckey != NULL) << "Failed to retrieve destination encryption keys.";
    auth_enckey = dime_keys_enckey_fetch(auth_keys);
    ASSERT_TRUE(auth_enckey != NULL) << "Failed to retrieve author encryption keys.";
    recp_enckey = dime_keys_enckey_fetch(recp_keys);
    ASSERT_TRUE(recp_enckey != NULL) << "Failed to retrieve recipient encryption keys.";
    ASSERT_DIME_NO_ERROR();

    orig_signkey = dime_keys_signkey_fetch(orig_keys);
    ASSERT_TRUE(orig_signkey != NULL) << "Failed to retrieve origin signing keys.";
    dest_signkey = dime_keys_signkey_fetch(dest_keys);
    ASSERT_TRUE(dest_signkey != NULL) << "Failed to retrieve destination signing keys.";
    auth_signkey = dime_keys_signkey_fetch(auth_keys);
    ASSERT_TRUE(auth_signkey != NULL) << "Failed to retrieve author signing keys.";
    recp_signkey = dime_keys_signkey_fetch(recp_keys);
    ASSERT_TRUE(recp_signkey != NULL) << "Failed to retrieve recipient signing keys.";
    ASSERT_DIME_NO_ERROR();

    // sign domain signets with cryptographic signet signature
    res = dime_sgnt_sig_crypto_sign(signet_orig, orig_signkey);
    ASSERT_EQ(0, res) << "Failed to sign origin signet with cryptographic signature.";
    res = dime_sgnt_sig_crypto_sign(signet_dest, dest_signkey);
    ASSERT_EQ(0, res) << "Failed to sign destination signet with cryptographic signature.";
    ASSERT_DIME_NO_ERROR();

    // sign domain signets with full signet signature
    res = dime_sgnt_sig_full_sign(signet_orig, orig_signkey);
    ASSERT_EQ(0, res) << "Failed to sign origin signet with full signature.";
    res = dime_sgnt_sig_full_sign(signet_dest, dest_signkey);
    ASSERT_EQ(0, res) << "Failed to sign destination signet with full signature.";
    ASSERT_DIME_NO_ERROR();

    //add domain ids to domain signets
    res = dime_sgnt_id_set(signet_orig, strlen(orig), (const unsigned char *)orig);
    ASSERT_EQ(0, res) << "Failed to set the origin signet id to its domain name.";
    res = dime_sgnt_id_set(signet_dest, strlen(dest), (const unsigned char *)dest);
    ASSERT_EQ(0, res) << "Failed to set the destination signet id to its domain name.";
    ASSERT_DIME_NO_ERROR();

    //add final domain signet signature
    res = dime_sgnt_sig_id_sign(signet_orig, orig_signkey);
    ASSERT_EQ(0, res) << "Failed to sign the origin signet with identifiable signature.";
    res = dime_sgnt_sig_id_sign(signet_dest, dest_signkey);
    ASSERT_EQ(0, res) << "Failed to sign the destination signet with identifiable signature.";
    ASSERT_DIME_NO_ERROR();

    //sign user ssr's with user user keys
    res = dime_sgnt_sig_ssr_sign(signet_auth, auth_signkey);
    ASSERT_EQ(0, res) << "Failed to sign the author signet with SSR signature.";
    res = dime_sgnt_sig_ssr_sign(signet_recp, recp_signkey);
    ASSERT_EQ(0, res) << "Failed to sign the recipient signet with SSR signature.";
    ASSERT_DIME_NO_ERROR();

    //sign user ssr's with corresponding domain keys
    res = dime_sgnt_sig_crypto_sign(signet_auth, orig_signkey);
    ASSERT_EQ(0, res) << "Failed to sign the author signet with cryptographic siganture.";
    res = dime_sgnt_sig_crypto_sign(signet_recp, dest_signkey);
    ASSERT_EQ(0, res) << "Failed to sign the recipient signet with cryptographic siganture.";
    ASSERT_DIME_NO_ERROR();

    //sign user signets with corresponding domain keys
    res = dime_sgnt_sig_full_sign(signet_auth, orig_signkey);
    ASSERT_EQ(0, res) << "Failed to sign the author signet with full siganture.";
    res = dime_sgnt_sig_full_sign(signet_recp, dest_signkey);
    ASSERT_EQ(0, res) << "Failed to sign the recipient signet with full siganture.";
    ASSERT_DIME_NO_ERROR();

    //set user signet id's
    res = dime_sgnt_id_set(signet_auth, strlen(auth), (const unsigned char *)auth);
    ASSERT_EQ(0, res) << "Failed to set the author signet id its email address.";
    res = dime_sgnt_id_set(signet_recp, strlen(recp), (const unsigned char *)recp);
    ASSERT_EQ(0, res) << "Failed to set the recipient signet id its email address.";
    ASSERT_DIME_NO_ERROR();

    //add final user signet signature with corresponding domain keys
    res = dime_sgnt_sig_id_sign(signet_auth, orig_signkey);
    ASSERT_EQ(0, res) << "Failed to sign the author signet with identifiable siganture.";
    res = dime_sgnt_sig_id_sign(signet_recp, dest_signkey);
    ASSERT_EQ(0, res) << "Failed to sign the author signet with identifiable siganture.";
    ASSERT_DIME_NO_ERROR();

    //create object as a draft
    draft = (dmime_object_t *)malloc(sizeof(dmime_object_t));
    memset(draft, 0, sizeof(dmime_object_t));

    draft->common_headers = dime_prsr_headers_create();

    draft->actor = id_author;
    draft->author = sdsnew(auth);
    draft->recipient = sdsnew(recp);
    draft->origin = sdsnew(orig);
    draft->destination = sdsnew(dest);
    draft->signet_author = dime_sgnt_signet_dupe(signet_auth);
    draft->signet_origin = dime_sgnt_signet_dupe(signet_orig);
    draft->signet_destination = dime_sgnt_signet_dupe(signet_dest);
    draft->signet_recipient = dime_sgnt_signet_dupe(signet_recp);
    draft->common_headers->headers[HEADER_TYPE_DATE] = sdsnew(common_date);
    draft->common_headers->headers[HEADER_TYPE_FROM] = sdsnew(common_from);
    draft->common_headers->headers[HEADER_TYPE_ORGANIZATION] = sdsnew(common_organization);
    draft->common_headers->headers[HEADER_TYPE_SUBJECT] = sdsnew(common_subject);
    draft->common_headers->headers[HEADER_TYPE_TO] = sdsnew(common_to);
    draft->other_headers = sdsnew(other_headers);
    draft->display = dime_dmsg_object_chunk_create(CHUNK_TYPE_DISPLAY_CONTENT, (unsigned char *)display, strlen(display), DEFAULT_CHUNK_FLAGS);
    ASSERT_DIME_NO_ERROR();

    // turn object into message by encrypting and serialize
    message = dime_dmsg_message_encrypt(draft, auth_signkey);
    ASSERT_TRUE(message != NULL) << "Failed encrypt the message.";

    from_auth_bin = dime_dmsg_message_binary_serialize(message, 0xFF, 0, &from_auth_size);
    ASSERT_TRUE(from_auth_bin != NULL) << "Failed to serialize the encrypted message.";

    ASSERT_DIME_NO_ERROR();

    //destroy message and deserialize it again from the serialized form as if it was received over wire by the origin
    dime_dmsg_message_destroy(message);
    message = dime_dmsg_message_binary_deserialize(from_auth_bin, from_auth_size);
    ASSERT_TRUE(message != NULL) << "Failed to deserialize the encrypted message as origin.";

    //decrypt message as origin
    res = dime_dmsg_kek_in_derive(message, orig_enckey, &orig_kek);
    ASSERT_EQ(0, res) << "Failed to derive the origin key encryption key.";
    ASSERT_DIME_NO_ERROR();

    at_orig = dime_dmsg_message_envelope_decrypt(message, id_origin, &orig_kek);
    ASSERT_TRUE(at_orig != NULL) << "Failed to decrypt the message envelope as origin.";

    res = sdscmp(draft->author, at_orig->author);
    ASSERT_EQ(0, res) << "The message author was corrupted in the envelope.";

    res = sdscmp(draft->destination, at_orig->destination);
    ASSERT_EQ(0, res) << "The message destination was corrupted in the envelope.";
    ASSERT_DIME_NO_ERROR();

    at_orig->signet_author = dime_sgnt_signet_dupe(signet_auth);
    at_orig->signet_destination = dime_sgnt_signet_dupe(signet_dest);
    at_orig->origin = sdsnew(orig);
    at_orig->signet_origin = dime_sgnt_signet_dupe(signet_orig);

    res = dime_dmsg_message_decrypt_as_orig(at_orig, message, &orig_kek);
    ASSERT_EQ(0, res) << "Origin could not decrypt the chunks it needs access to.";

    //Add origin signatures and serialize the message again
    res = dime_dmsg_chunks_sig_origin_sign(message, (META_BOUNCE | DISPLAY_BOUNCE), &orig_kek, orig_signkey);
    ASSERT_EQ(0, res) << "Origin failed to sign the message.";

    from_orig_bin = dime_dmsg_message_binary_serialize(message, 0xFF, 0, &from_orig_size);
    ASSERT_TRUE(from_orig_bin != NULL) << "Failed to serialize the message as origin.";

    ASSERT_DIME_NO_ERROR();

    //destroy message and deserialize it again from the serialized form as if it was received over wire by the destination
    dime_dmsg_message_destroy(message);

    message = dime_dmsg_message_binary_deserialize(from_orig_bin, from_orig_size);
    ASSERT_TRUE(message != NULL) << "Failed to deserialize the message as destination.";

    //decrypt message as destination
    res = dime_dmsg_kek_in_derive(message, dest_enckey, &dest_kek);
    ASSERT_EQ(0, res) << "Failed to derive the destination key encryption key.";

    at_dest = dime_dmsg_message_envelope_decrypt(message, id_destination, &dest_kek);
    ASSERT_TRUE(at_dest != NULL) << "Failed to decrypt the message envelope as destination.";

    res = sdscmp(draft->origin, at_dest->origin);
    ASSERT_EQ(0, res) << "The message origin was corrupted in the envelope.";

    res = sdscmp(draft->recipient, at_dest->recipient);
    ASSERT_EQ(0, res) << "The message recipient was corrupted in the envelope.";
    ASSERT_DIME_NO_ERROR();

    at_dest->signet_origin = dime_sgnt_signet_dupe(signet_orig);
    at_dest->signet_recipient = dime_sgnt_signet_dupe(signet_recp);
    at_dest->destination = sdsnew(dest);
    at_dest->signet_destination = dime_sgnt_signet_dupe(signet_dest);

    res = dime_dmsg_message_decrypt_as_dest(at_dest, message, &dest_kek);
    ASSERT_EQ(0, res) << "Destination could not decrypt the chunks it needs access to.";

    //Serialize the message again
    from_dest_bin = dime_dmsg_message_binary_serialize(message, 0xFF, 0, &from_dest_size);
    ASSERT_TRUE(from_dest_bin != NULL) << "Failed to serialize the message as destination.";

    ASSERT_DIME_NO_ERROR();

    //destroy message and deserialize it again from the serialized form as if it was received over wire by the recipient
    dime_dmsg_message_destroy(message);

    message = dime_dmsg_message_binary_deserialize(from_dest_bin, from_dest_size);
    ASSERT_TRUE(message != NULL) << "Failed to deserialize encrypted message as recipient.";
    ASSERT_DIME_NO_ERROR();

    //decrypt message as recipient
    res = dime_dmsg_kek_in_derive(message, recp_enckey, &recp_kek);
    ASSERT_EQ(0, res) << "Failed to derive recipient key encryption key.";

    at_recp = dime_dmsg_message_envelope_decrypt(message, id_recipient, &recp_kek);
    ASSERT_TRUE(at_recp != NULL) << "Failed to decrypt the envelope as the recipient.";

    res = sdscmp(draft->author, at_recp->author);
    ASSERT_EQ(0, res) << "The message author was corrupted in the envelope.";
    res = sdscmp(draft->origin, at_recp->origin);
    ASSERT_EQ(0, res) << "The message origin was corrupted in the envelope.";
    res = sdscmp(draft->destination, at_recp->destination);
    ASSERT_EQ(0, res) << "The message destination was corrupted in the envelope.";
    res = sdscmp(draft->recipient, at_recp->recipient);
    ASSERT_EQ(0, res) << "The message recipient was corrupted in the envelope.";
    ASSERT_DIME_NO_ERROR();

    at_recp->signet_author = dime_sgnt_signet_dupe(signet_auth);
    at_recp->signet_origin = dime_sgnt_signet_dupe(signet_orig);
    at_recp->signet_destination = dime_sgnt_signet_dupe(signet_dest);
    at_recp->signet_recipient = dime_sgnt_signet_dupe(signet_recp);

    res = dime_dmsg_message_decrypt_as_recp(at_recp, message, &recp_kek);
    ASSERT_EQ(0, res) << "Failed to decrypt the message as recipient.";

    res = sdscmp(draft->common_headers->headers[HEADER_TYPE_DATE], at_recp->common_headers->headers[HEADER_TYPE_DATE]);
    ASSERT_EQ(0, res) << "DATE header was corrupted.";
    res = sdscmp(draft->common_headers->headers[HEADER_TYPE_FROM], at_recp->common_headers->headers[HEADER_TYPE_FROM]);
    ASSERT_EQ(0, res) << "FROM header was corrupted.";
    res = sdscmp(draft->common_headers->headers[HEADER_TYPE_ORGANIZATION], at_recp->common_headers->headers[HEADER_TYPE_ORGANIZATION]);
    ASSERT_EQ(0, res) << "ORGANIZATION header was corrupted.";
    res = sdscmp(draft->common_headers->headers[HEADER_TYPE_SUBJECT], at_recp->common_headers->headers[HEADER_TYPE_SUBJECT]);
    ASSERT_EQ(0, res) << "SUBJECT header was corrupted.";
    res = sdscmp(draft->common_headers->headers[HEADER_TYPE_TO], at_recp->common_headers->headers[HEADER_TYPE_TO]);
    ASSERT_EQ(0, res) << "TO header was corrupted.";
    res = sdscmp(draft->other_headers, at_recp->other_headers);
    ASSERT_EQ(0, res) << "Other headers were corrupted.";
    res = (draft->display->data_size == at_recp->display->data_size);
    ASSERT_EQ(1, res) << "Message body data size was corrupted.";
    res = memcmp(draft->display->data, at_recp->display->data, draft->display->data_size);
    ASSERT_EQ(0, res) << "Message body data was corrupted.";

    //destroy everything
    dime_sgnt_signet_destroy(signet_auth);
    dime_sgnt_signet_destroy(signet_orig);
    dime_sgnt_signet_destroy(signet_dest);
    dime_sgnt_signet_destroy(signet_recp);

    dime_dmsg_message_destroy(message);

    dime_dmsg_object_destroy(draft);
    dime_dmsg_object_destroy(at_orig);
    dime_dmsg_object_destroy(at_dest);
    dime_dmsg_object_destroy(at_recp);

    _free_ec_key(auth_enckey);
    _free_ec_key(orig_enckey);
    _free_ec_key(dest_enckey);
    _free_ec_key(recp_enckey);

    _free_ed25519_key(auth_signkey);
    _free_ed25519_key(orig_signkey);
    _free_ed25519_key(dest_signkey);
    _free_ed25519_key(recp_signkey);

    free(from_auth_bin);
    free(from_orig_bin);
    free(from_dest_bin);
    ASSERT_DIME_NO_ERROR();
}
