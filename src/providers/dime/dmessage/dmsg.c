#include "dime/common/misc.h"
#include "dime/dmessage/parse.h"
#include "dime/dmessage/crypto.h"
#include "dime/signet/signet.h"

typedef dmime_kek_t dmime_kekset_t[4];

//ephemeral payload structure
typedef unsigned char dmime_ephemeral_payload_t[EC_PUBKEY_SIZE];

//standard payload structure
typedef struct __attribute__((packed)) {
    unsigned char signature[ED25519_SIG_SIZE];
    unsigned char data_size[CHUNK_LENGTH_SIZE];
    unsigned char flags;
    unsigned char pad_len;
    unsigned char data[];
} dmime_standard_payload_t;

//signature payload structure
typedef unsigned char dmime_signature_payload_t[ED25519_SIG_SIZE];

//encrypted payload structure
typedef unsigned char *dmime_encrypted_payload_t;

// keyslot contains the aes keys. Each chunk can have between 0 and 4 keyslots.
typedef struct __attribute__((packed)) {
    unsigned char random[16];               // Random bytes.
    unsigned char iv[16];                   // Initialization vector.
    unsigned char aes_key[AES_256_KEY_SIZE];
} dmime_keyslot_t;


static void *
mm_set(void *block, unsigned char set, size_t len);

static const char *
dmsg_actor_to_string(
    dmime_actor_t actor);

static dmime_message_chunk_t **
dmsg_attach_encode(
    dmime_object_t *object);

static unsigned char *
dmsg_chunk_data_get(
    dmime_message_chunk_t *chunk,
    size_t *outsize);

static unsigned char *
dmsg_chunk_data_padded_get(
    dmime_message_chunk_t *chunk,
    size_t *outsize);

static int
dmsg_chunk_destination_decrypt(
    dmime_object_t *object,
    dmime_message_t const *msg,
    dmime_kek_t *kek);

static dmime_message_chunk_t *
dmsg_chunk_destination_encode(
    dmime_object_t *object);

static dmime_message_chunk_t *
dmsg_chunk_decrypt(
    dmime_message_chunk_t *chunk,
    dmime_actor_t actor,
    dmime_kek_t *kek);

static dmime_message_chunk_t *
dmsg_chunk_deserialize(
    unsigned char const *in,
    size_t insize,
    size_t *read);

static int
dmsg_chunk_encrypt(
    dmime_message_chunk_t *chunk,
    dmime_kekset_t *keks);

static unsigned char
dmsg_chunk_flags_get(
    dmime_message_chunk_t *chunk);

static int
dmsg_chunk_headers_common_decrypt(
    dmime_object_t *object,
    dmime_message_t const *msg,
    dmime_kek_t *kek);

static dmime_message_chunk_t *
dmsg_chunk_headers_common_encode(
    dmime_object_t *object);

static int
dmsg_chunk_headers_other_decrypt(
    dmime_object_t *object,
    dmime_message_t const *msg,
    dmime_kek_t *kek);

static dmime_message_chunk_t *
dmsg_chunk_headers_other_encode(
    dmime_object_t *object);

static dmime_keyslot_t *
dmsg_chunk_keyslot_get_by_num(
    dmime_message_chunk_t *chunk,
    size_t num);

static dmime_message_chunk_t *
dmsg_chunk_origin_encode(
    dmime_object_t *object);

static int
dmsg_chunk_origin_decrypt(
    dmime_object_t *object,
    dmime_message_t const *msg,
    dmime_kek_t *kek);

static int
dmsg_chunk_padlen_get(
    size_t dsize,
    unsigned char flags,
    unsigned int *padlen,
    unsigned char *padbyte);

static void *
dmsg_chunk_payload_get(
    dmime_message_chunk_t *chunk);

static dmime_message_chunk_t *
dmsg_chunk_payload_wrap(
    dmime_chunk_type_t type,
    unsigned char *payload,
    size_t insize);

static unsigned char *
dmsg_chunk_sig_plaintext_get(
    dmime_message_chunk_t *chunk);

static int
dmsg_chunk_sig_validate(
    dmime_message_chunk_t *chunk,
    signet_t *signet);

static int
dmsg_chunk_sign(
    dmime_message_chunk_t *chunk,
    ED25519_KEY *signkey);

static dmime_chunk_key_t *
dmsg_chunk_type_key_get(
    dmime_chunk_type_t type);

static int
dmsg_chunks_content_decrypt(
    dmime_object_t *object,
    const dmime_message_t *msg,
    dmime_kek_t *kek);

static int
dmsg_chunks_message_encrypt(
    dmime_message_t *message,
    dmime_kekset_t *keks);

static unsigned char *
dmsg_chunks_serialize(
    dmime_message_t const *msg,
    dmime_chunk_type_t first,
    dmime_chunk_type_t last,
    size_t *outsize);

static int
dmsg_chunks_sig_author_sign(
    dmime_message_t *message,
    ED25519_KEY *signkey,
    dmime_kekset_t *keks);

static int
dmsg_chunks_sig_author_validate(
    dmime_object_t *object,
    dmime_message_t const *msg,
    dmime_kek_t *kek);

static int
dmsg_chunks_sig_origin_sign(
    dmime_message_t *msg,
    unsigned char bounce_flags,
    dmime_kek_t *kek,
    ED25519_KEY *signkey);

static int
dmsg_chunks_sig_origin_validate(
    dmime_object_t *object,
    dmime_message_t const *msg,
    dmime_kek_t *kek);

static size_t
dmsg_chunks_size_get(
    dmime_message_t const *msg,
    dmime_chunk_type_t first,
    dmime_chunk_type_t last);

static dmime_message_chunk_t **
dmsg_display_encode(
    dmime_object_t *object);

static void
dmsg_message_chunk_chain_destroy(
    dmime_message_chunk_t **chunks);

static dmime_message_chunk_t *
dmsg_message_chunk_create(
    dmime_chunk_type_t type,
    unsigned char const *data,
    size_t insize,
    unsigned char flags);

static void
dmsg_message_chunk_destroy(
    dmime_message_chunk_t *chunk);

static int
dmsg_message_chunks_encode(
    dmime_object_t *object,
    dmime_message_t *message);

static int
dmsg_message_chunks_sign(
    dmime_message_t *message,
    ED25519_KEY *signkey);

static int
dmsg_message_decrypt_as_auth(
    dmime_object_t *obj,
    dmime_message_t const *msg,
    dmime_kek_t *kek);

static int
dmsg_message_decrypt_as_dest(
    dmime_object_t *obj,
    dmime_message_t const *msg,
    dmime_kek_t *kek);

static int
dmsg_message_decrypt_as_orig(
    dmime_object_t *obj,
    dmime_message_t const *msg,
    dmime_kek_t *kek);

static int
dmsg_message_decrypt_as_recp(
    dmime_object_t *obj,
    dmime_message_t const *msg,
    dmime_kek_t *kek);

static dmime_message_t *
dmsg_message_deserialize(
    unsigned char const *in,
    size_t insize);

static size_t
dmsg_message_deserialize_helper(
    dmime_message_t *msg,
    unsigned char const *in,
    size_t insize,
    dmime_chunk_type_t *last_type);

static void
dmsg_message_destroy(
    dmime_message_t *msg);

static dmime_message_t *
dmsg_message_encrypt(
    dmime_object_t *object,
    ED25519_KEY *signkey);

static dmime_object_t *
dmsg_message_envelope_decrypt(
    dmime_message_t const *msg,
    dmime_actor_t actor,
    dmime_kek_t *kek);

static unsigned char *
dmsg_message_serialize(
    dmime_message_t const *msg,
    unsigned char sections,
    unsigned char tracing,
    size_t *outsize);

static dmime_message_state_t
dmsg_message_state_get(
    dmime_message_t const *message);

static dmime_object_chunk_t *
dmsg_object_chunk_create(
    dmime_chunk_type_t type,
    unsigned char *data,
    size_t data_size,
    unsigned char flags);

static void
dmsg_object_chunklist_destroy(
    dmime_object_chunk_t *list);

static void
dmsg_object_destroy(
    dmime_object_t *object);

static int
dmsg_object_dump(
    dmime_object_t *object);

static dmime_object_state_t
dmsg_object_state_init(
    dmime_object_t *object);

static const char *
dmsg_object_state_to_string(
    dmime_object_state_t state);

static int
dmsg_kek_in_derive(
    dmime_message_t const *msg,
    EC_KEY *enckey,
    dmime_kek_t *kek);

static int
dmsg_kek_out_derive(
    EC_KEY *privkey,
    signet_t *signet,
    dmime_kek_t *kekbuf);

static int
dmsg_kek_out_derive_all(
    dmime_object_t *object,
    EC_KEY *ephemeral,
    dmime_kekset_t *kekset);

static int
dmsg_keyslot_decrypt(
    dmime_keyslot_t *encrypted,
    dmime_kek_t *kek,
    dmime_keyslot_t *decrypted);

static int
dmsg_keyslot_encrypt(
    dmime_keyslot_t *keyslot,
    dmime_kek_t *kek);

static dmime_message_chunk_t **
dmsg_section_deserialize(
    unsigned char const *in,
    size_t insize,
    dmime_chunk_section_t section,
    size_t *read);

static unsigned char *
dmsg_sections_serialize(
    dmime_message_t const *msg,
    unsigned char sections,
    size_t *outsize);

static size_t
dmsg_sections_size_get(
    dmime_message_t const *msg,
    unsigned char sections);

static size_t
dmsg_tracing_load(
    dmime_message_t *msg,
    unsigned char const *in,
    size_t insize);

static unsigned char *
dmsg_treesig_data_get(
    dmime_message_t const *msg,
    size_t *outsize);

/* PRIVATE FUNCTIONS */

/**
 * @brief   Sets a block of memory to a specified value.
 * @param   block   the block of memory to be set.
 * @param   set     the byte value to be written to block.
 * @param   len     the number of times to write the value of the byte repeatedly to block.
 * @return  a pointer to the block of memory passed to the function.
 */
void *
mm_set(void *block, unsigned char set, size_t len) {
    volatile char *ptr = block;

    asm ("");

    while (len--) {
        *ptr++ = set;
    }

    asm ("");

    return block;
}

/**
 * @brief
 *  Takes a dmime object and determines the state it is in.
 * @param object
 *  Dmime object, state of which will be retrieved.
 * @return
 *  The state of dmime object.
 */
static dmime_object_state_t
dmsg_object_state_init(dmime_object_t *object)
{
    if (!object) {
        RET_ERROR_CUST(DMIME_OBJECT_STATE_NONE, ERR_BAD_PARAM, NULL);
    }

    //TODO more comprehensive state checks/retrieval.
    if (
        !object->author
        || !object->signet_author
        || !object->origin
        || !object->signet_origin
        || !object->destination
        || !object->signet_destination
        || !object->recipient
        || !object->signet_recipient)
    {
        return object->state = DMIME_OBJECT_STATE_INCOMPLETE_ENVELOPE;
    }

    if (!object->common_headers) {
        return object->state = DMIME_OBJECT_STATE_INCOMPLETE_METADATA;
    }

    return object->state = DMIME_OBJECT_STATE_COMPLETE;
}

/**
 * @brief
 *  Retrieves dmime message state.
 * @param message
 *  Pointer to a dmime message.
 * @return
 *  dmime_message_state_t corresponding to the current state.
 */
static dmime_message_state_t
dmsg_message_state_get(const dmime_message_t *message)
{
    if (!message) {
        RET_ERROR_CUST(MESSAGE_STATE_NONE, ERR_BAD_PARAM, NULL);
    }
    //TODO Maybe needs a better implementation.
    return message->state;
}

/**
 * @brief
 *  Destroys dmime_message_t structure.
 * @param msg
 *  Pointer to the dmime message to be destroyed.
*/
static void
dmsg_message_destroy(dmime_message_t *msg)
{
    if (!msg) {
        return;
    }

    if (msg->tracing) {
        free(msg->tracing);
    }

    if (msg->ephemeral) {
        dmsg_message_chunk_destroy(msg->ephemeral);
    }

    if (msg->origin) {
        dmsg_message_chunk_destroy(msg->origin);
    }

    if (msg->destination) {
        dmsg_message_chunk_destroy(msg->destination);
    }

    if (msg->common_headers) {
        dmsg_message_chunk_destroy(msg->common_headers);
    }

    if (msg->other_headers) {
        dmsg_message_chunk_destroy(msg->other_headers);
    }

    if (msg->display) {
        dmsg_message_chunk_chain_destroy(msg->display);
    }

    if (msg->attach) {
        dmsg_message_chunk_chain_destroy(msg->attach);
    }

    if (msg->author_tree_sig) {
        dmsg_message_chunk_destroy(msg->author_tree_sig);
    }

    if (msg->author_full_sig) {
        dmsg_message_chunk_destroy(msg->author_full_sig);
    }

    if (msg->origin_meta_bounce_sig) {
        dmsg_message_chunk_destroy(msg->origin_meta_bounce_sig);
    }

    if (msg->origin_display_bounce_sig) {
        dmsg_message_chunk_destroy(msg->origin_display_bounce_sig);
    }

    if (msg->origin_full_sig) {
        dmsg_message_chunk_destroy(msg->origin_full_sig);
    }

    free(msg);
}


/**
 * @brief
 *  Takes a dmime object uses it to encode an origin dmime message chunk
 * @param object
 *  dmime object with information that will be encoded into the origin chunk
 * @return
 *  Pointer to a dmime message origin chunk
 * @free_using{dmsg_destroy_message_chunk}
*/
static dmime_message_chunk_t *
dmsg_chunk_origin_encode(dmime_object_t *object)
{
    char *author_signet_fingerprint_b64, *destination_signet_fingerprint_b64;
    dmime_message_chunk_t *result;
    sds data = NULL;

    if (!object) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!object->signet_author
        || !object->author
        || !object->signet_destination
        || !object->destination)
    {
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "the dmime object does not contain necessary information "
            "to encode an origin message chunk");
    }

    if (!(author_signet_fingerprint_b64 =
            dime_sgnt_fingerprint_crypto(object->signet_author)))
    {
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "could not calculate author signer cryptographic fingerprint");
    }

    if (!(destination_signet_fingerprint_b64 =
          dime_sgnt_fingerprint_crypto(object->signet_destination)))
    {
        free(author_signet_fingerprint_b64);
        RET_ERROR_PTR(ERR_UNSPEC, "could not calculate fingerprint");
    }

    data = dime_prsr_envelope_format(
        object->author,
        object->destination,
        (const char *)author_signet_fingerprint_b64,
        (const char *)destination_signet_fingerprint_b64,
        CHUNK_TYPE_ORIGIN);
    free(author_signet_fingerprint_b64);
    free(destination_signet_fingerprint_b64);

    if(!data) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not format origin chunk data");
    }

    result = dmsg_message_chunk_create(
        CHUNK_TYPE_ORIGIN,
        (unsigned char *)data,
        sdslen(data),
        DEFAULT_CHUNK_FLAGS);
    sdsfree(data);

    if(!result) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not create message chunk");
    }

    return result;
}


/**
 * @brief
 *  Takes a dmime object uses it to encode a destination dmime message chunk
 * @param object
 *  dmime object with information that will be encoded into the destination
 *  chunk
 * @return
 *  Pointer to a dmime message destination chunk
 * @free_using{dmsg_destroy_message_chunk}
*/
static dmime_message_chunk_t *
dmsg_chunk_destination_encode(dmime_object_t *object)
{
    char *recipient_signet_fingerprint_b64, *origin_signet_fingerprint_b64;
    dmime_message_chunk_t *result;
    sds data;

    if (!object) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!object->signet_recipient
        || !object->recipient
        || !object->signet_origin
        || !object->origin)
    {
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "the dmime object does not contain necessary information "
            "to encode an origin message chunk");
    }

    if (!(recipient_signet_fingerprint_b64 =
            dime_sgnt_fingerprint_crypto(object->signet_recipient)))
    {
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "failed to calculate recipient's cryptographic signet "
            "fingerprint");
    }

    if(!(origin_signet_fingerprint_b64 =
            dime_sgnt_fingerprint_crypto(object->signet_origin)))
    {
        free(recipient_signet_fingerprint_b64);
        RET_ERROR_PTR(ERR_UNSPEC, "could not calculate fingerprint");
    }

    data = dime_prsr_envelope_format(
        object->recipient,
        object->origin,
        recipient_signet_fingerprint_b64,
        origin_signet_fingerprint_b64,
        CHUNK_TYPE_DESTINATION);
    free(recipient_signet_fingerprint_b64);
    free(origin_signet_fingerprint_b64);

    if(!data) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not format origin chunk data");
    }

    result = dmsg_message_chunk_create(
        CHUNK_TYPE_DESTINATION,
        (unsigned char *)data,
        sdslen(data),
        DEFAULT_CHUNK_FLAGS);
    sdsfree(data);

    if(!result) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not create message chunk");
    }

    return result;
}


/**
 * @brief
 *  Takes a dmime object uses it to encode a common metadata dmime message
 *  chunk
 * @param object
 *  dmime object with information that will be encoded into the common headers
 *  chunk
 * @return
 *  Pointer to a dmime message common headers chunk
*/
static dmime_message_chunk_t *
dmsg_chunk_headers_common_encode(dmime_object_t *object)
{
    dmime_message_chunk_t *result;
    size_t data_size;
    unsigned char *data;

    if(!object) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!(data =
            dime_prsr_headers_format(
                object->common_headers,
                &data_size)))
    {
        RET_ERROR_PTR(ERR_UNSPEC, "could not format common headers data");
    }

    result =
        dmsg_message_chunk_create(
            CHUNK_TYPE_META_COMMON,
            data,
            data_size,
            DEFAULT_CHUNK_FLAGS);
    free(data);

    if(!result) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not create message chunk");
    }

    return result;
}


/**
 * @brief
 *  Takes a dmime object and uses it to encode other_headers metadata dmime
 *  message chunk
 * @param object
 *  dmime object with the other_headers data that will be encoded into the
 *  chunk
 * @return
 *  Pointer to a dmime message other headers chunk
 * @free_using{dmsg_destroy_message_chunk}
*/
static dmime_message_chunk_t *
dmsg_chunk_headers_other_encode(dmime_object_t *object)
{
    dmime_message_chunk_t *result;

    if (!object) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    //TODO right now we have all the non-common headers combined into one string

    if (!(result =
            dmsg_message_chunk_create(
                CHUNK_TYPE_META_OTHER,
                (unsigned char *)object->other_headers,
                sdslen(object->other_headers),
                DEFAULT_CHUNK_FLAGS)))
    {
        RET_ERROR_PTR(ERR_UNSPEC, "could not create message chunk");
    }

    return result;
}

/**
 * @brief
 *  Takes a dmime object and encodes the display chunk into an array of dmime
 *  message chunks.
 * @param object
 *  Pointer to the dmime object containing the display chunk data.
 * @return
 *  Returns a pointer to a NULL-pointer terminated array of dmime message
 *  chunks encoded with the display data.
 * @free_using{dmsg_destroy_message_chunk_chain}
*/
static dmime_message_chunk_t **
dmsg_display_encode(dmime_object_t *object)
{
    dmime_object_chunk_t *first_chunk, *temp;
    dmime_message_chunk_t **result;
    int counter = 0;

    if(!object) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!(temp = first_chunk = object->display)) {
        RET_ERROR_PTR(ERR_UNSPEC, "no display data in the dmime object");
    }

    while(temp) {
        ++counter;
        temp = temp->next;
    }

    temp = first_chunk;

    if(!(result = malloc((counter + 1) * sizeof(dmime_message_chunk_t *)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate array for chunk pointers");
    }

    memset(result, 0, ((counter + 1) * sizeof(dmime_message_chunk_t *)));

    for(int i = 0; i < counter; ++i) {

        if (!(result[i] =
                dmsg_message_chunk_create(
                    temp->type,
                    temp->data,
                    temp->data_size,
                    temp->flags)))
        {
            dmsg_message_chunk_chain_destroy(result);
            RET_ERROR_PTR(
                ERR_UNSPEC,
                "could not encode a display message chunk");
        }

        temp = temp->next;
    }

    return result;
}


/**
 * @brief
 *  Takes a dmime object and encodes the attachment chunk into an array of
 *  dmime message chunks.
 * @param object
 *  Pointer to the dmime object containing the attachment chunk data.
 * @return
 *  returns a pointer to a NULL-pointer terminated array of dmime message
 *  chunks encoded with the attachment data.
 * @free_using{dmsg_destroy_message_chunk_chain}
*/
static dmime_message_chunk_t **
dmsg_attach_encode(dmime_object_t *object)
{
    dmime_object_chunk_t *first_chunk, *temp;
    dmime_message_chunk_t **result;
    int counter = 0;

    if(!object) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!(temp = first_chunk = object->attach)) {
        RET_ERROR_PTR(ERR_UNSPEC, "no attachment data in the dmime object");
    }

    while(temp) {
        ++counter;
        temp = temp->next;
    }

    temp = first_chunk;

    if(!(result = malloc((counter + 1) * sizeof(dmime_message_chunk_t *)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(
            ERR_NOMEM,
            "could not allocate array for chunk pointers");
    }

    memset(result, 0, ((counter + 1) * sizeof(dmime_message_chunk_t *)));

    for(int i = 0; i < counter; ++i) {

        if(!(result[i] =
                dmsg_message_chunk_create(
                    temp->type,
                    temp->data,
                    temp->data_size,
                    temp->flags)))
        {
            dmsg_message_chunk_chain_destroy(result);
            RET_ERROR_PTR(
                ERR_UNSPEC,
                "could not encode an attachment message chunk");
        }

        temp = temp->next;
    }

    return result;
}


/**
 * @brief
 *  takes a dmime object and encodes the envelope, metadata, display and
 *  attachment data into a dmime message.
 * @param object
 *  pointer to a dmime object containing the envelope, metadata, display and
 *  attachment information.
 * @param message
 *  pointer to a dmime message into which the information gets encoded.
 * @return  0 on success, -1 on failure.
*/
static int
dmsg_message_chunks_encode(
    dmime_object_t *object,
    dmime_message_t *message)
{
    if(!object || !message) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if(dmsg_message_state_get(message) != MESSAGE_STATE_EMPTY) {
        RET_ERROR_INT(ERR_UNSPEC, "message should be empty to be encoded");
    }

    if(!(message->origin = dmsg_chunk_origin_encode(object))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not encode origin chunk");
    }

    if(!(message->destination = dmsg_chunk_destination_encode(object))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not encode destination chunk");
    }

    if(!(message->common_headers = dmsg_chunk_headers_common_encode(object))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not encode common headers chunk");
    }

    if (object->other_headers
        && !(message->other_headers = dmsg_chunk_headers_other_encode(object)))
    {
        RET_ERROR_INT(ERR_UNSPEC, "could not encode other headers chunk");
    }

    if (!(message->display = dmsg_display_encode(object))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not encode display chunks");
    }

    if(object->attach && !(message->attach = dmsg_attach_encode(object))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not encode attachment chunks");
    }

    message->state = MESSAGE_STATE_ENCODED;

    return 0;
}


/**
 * @brief
 *  signs a message chunk using the specified private signing key.
 * @param chunk
 *  pointer to a dmime message chunk to be signed.
 * @param signkey
 *  author's ed25519 private signing key.
 * @return  0 on success, -1 failure.
*/
static int
dmsg_chunk_sign(
    dmime_message_chunk_t *chunk,
    ED25519_KEY *signkey)
{
    dmime_chunk_key_t *key;
    size_t data_size;
    unsigned char *data, *signature;

    if(!chunk || !signkey) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if(chunk->state != MESSAGE_CHUNK_STATE_ENCODED) {
        RET_ERROR_INT(ERR_UNSPEC, "message chunk is not encoded");
    }

    if(!(key = dmsg_chunk_type_key_get((dmime_chunk_type_t)chunk->type))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not retrieve chunk key");
    }

    if(key->payload != PAYLOAD_TYPE_STANDARD) {
        RET_ERROR_INT(ERR_UNSPEC, "only standard payloads can be signed");
    }

    if(!(data = dmsg_chunk_data_padded_get(chunk, &data_size))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not retrieve chunk padded data");
    }

    if(!(signature = dmsg_chunk_sig_plaintext_get(chunk))) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not retrieve chunk plaintext signature buffer");
    }

    if(_ed25519_sign_data(data, data_size, signkey, signature)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not sign the payload data");
    }

    chunk->state = MESSAGE_CHUNK_STATE_SIGNED;

    return 0;
}


/**
 * @brief
 *  takes a dmime message that has only been encoded and signs every encoded
 *  chunk with the provided ed25519 private signing key.
 * @param message
 *  pointer to a dmime message, the chunks of which will be signed.
 * @param signkey
 *  a ed25519 private signing (supposedly the author's).
 * @return  0 on success, -1 on failure.
*/
static int
dmsg_message_chunks_sign(
    dmime_message_t *message,
    ED25519_KEY *signkey)
{
    if(!message || !signkey) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if(dmsg_message_state_get(message) != MESSAGE_STATE_ENCODED) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "you can only sign the chunks of a message that has been encoded");
    }

    if(dmsg_chunk_sign(message->origin, signkey)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not sign origin chunk");
    }

    if(dmsg_chunk_sign(message->destination, signkey)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not sign origin chunk");
    }

    if(dmsg_chunk_sign(message->common_headers, signkey)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not sign origin chunk");
    }

    if(dmsg_chunk_sign(message->other_headers, signkey)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not sign origin chunk");
    }

    if(message->display) {
        for (size_t i = 0; message->display[i]; i++) {

            if(dmsg_chunk_sign(message->display[i], signkey)) {
                RET_ERROR_INT(ERR_UNSPEC, "could not sign display chunk");
            }

        }

    }

    if(message->attach) {
        for (size_t i = 0; message->attach[i]; i++) {

            if(dmsg_chunk_sign(message->attach[i], signkey)) {
                RET_ERROR_INT(ERR_UNSPEC, "could not sign attachment chunk");
            }

        }

    }

    message->state = MESSAGE_STATE_CHUNKS_SIGNED;

    return 0;
}

/**
 * @brief
 *  uses the signet passed to function and an ec key to create
 *  key-encryption-key block via ecdh, with the public key being taken from the
 *  signet.
 * @param privkey
 *  pointer to an EC_KEY structure containing a private ec key used for the
 *  ecdh
 * @param signet
 *  pointer to a signet containing the public ec encryption key used for the
 *  ecdh
 * @param kekbuf
 *  key encryption key buffer tha will be set to the resulting 16 byte iv and
 *  32 byte aes256 key
 * @return
 *  0 on success, -1 on failure.
*/
static int
dmsg_kek_out_derive(
    EC_KEY *privkey,
    signet_t *signet,
    dmime_kek_t *kekbuf)
{
    EC_KEY *signetkey;

    if(!privkey || !signet || !kekbuf) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if(!(signetkey = dime_sgnt_enckey_fetch(signet))) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not retrieve author public encryption key");
    }

    if(_compute_aes256_kek(signetkey, privkey, (unsigned char *)kekbuf)) {
        _free_ec_key(signetkey);
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not compute aes256 kek and store it in the "
            "specified buffer");
    }

    _free_ec_key(signetkey);

    return 0;
}

/**
 * @brief
 *  populates the set of kek's (key encryption keys).
 * @param object
 *  pointer to the non-serialized dmime message that has had its ephemeral
 *  chunk allocated and is linked to all required signets.
 * @param ephemeral
 *  pointer to an ephemeral private encryption ec key used to generate the key
 *  encryption keys and initialization vectors.
 * @param kekset
 *  pointer to the set of key encryption keys to be populated.
 * @result  0 on success, -1 on failure.
 */
static int
dmsg_kek_out_derive_all(
    dmime_object_t *object,
    EC_KEY *ephemeral,
    dmime_kekset_t *kekset)
{
    if (!object || !ephemeral || !kekset) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    memset((*kekset), 0, sizeof(dmime_kekset_t));

    if (dmsg_kek_out_derive(
            ephemeral,
            object->signet_author,
            &((*kekset)[id_author])))
    {
        RET_ERROR_INT(ERR_UNSPEC, "could not set author kek");
    }

    if (dmsg_kek_out_derive(
            ephemeral,
            object->signet_origin,
            &((*kekset)[id_origin])))
    {
        RET_ERROR_INT(ERR_UNSPEC, "could not set recipient kek");
    }

    if (dmsg_kek_out_derive(
            ephemeral,
            object->signet_destination,
            &((*kekset)[id_destination])))
    {
        RET_ERROR_INT(ERR_UNSPEC, "could not set origin kek");
    }

    if (dmsg_kek_out_derive(
            ephemeral,
            object->signet_recipient,
            &((*kekset)[id_recipient])))
    {
        RET_ERROR_INT(ERR_UNSPEC, "could not set destination kek");
    }

    return 0;
}


/**
 * @brief
 *  encrypts keyslot with the specified aes256 key and initialization vector.
 * @param keyslot
 *  pointer to the keyslot to be encrypted.
 * @param kek
 *  pointer to the kek used for encrpypting the keyslot.
 * @return  0 on success, -1 on falure.
*/
static int
dmsg_keyslot_encrypt(
    dmime_keyslot_t *keyslot,
    dmime_kek_t *kek)
{
    dmime_keyslot_t slot;
    int result;

    if (!keyslot || !kek) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    mm_set(&slot, 0, sizeof(slot));

    for(size_t i = 0; i < 16; ++i) {
        keyslot->iv[i] = keyslot->random[i] ^ keyslot->iv[i];
    }

    if ((result =
            _encrypt_aes_256(
                (unsigned char *)&slot,
                (unsigned char *)keyslot,
                sizeof(dmime_keyslot_t),
                kek->key,
                kek->iv))
        < 0)
    {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "error occurred while encrypting chunk data");
    } else if (result != sizeof(slot)) {
        mm_set(&slot, 0, sizeof(slot));
        RET_ERROR_INT(
            ERR_UNSPEC,
            "chunk keyslot encryption operation did not return expected "
            "length");
    }

    // copy the newly encrypted information over the keyslot and return.
    memcpy(keyslot, &slot, sizeof(dmime_keyslot_t));
    mm_set(&slot, 0, sizeof(slot));

    return 0;
}


/**
 * @brief
 *  takes a dmime message chunk and a kekset, generates the aes256 chunk
 *  encryption keys for the keyslots, encrypts the message, then encrypts the
 *  keyslots with the kekset.
 * @param chunk
 *  pointer to the dmime message chunk to be encrypted.
 * @param keks
 *  pointer to the kekset for encrypting the key slots.
 * @return
 *  0 on success, -1 on failure.
*/
static int
dmsg_chunk_encrypt(
    dmime_message_chunk_t *chunk,
    dmime_kekset_t *keks)
{
    // TODO there may be some code reuse that could occur here, the function is
    // a bit long

    dmime_chunk_key_t *key;
    dmime_keyslot_t *keyslot, temp;
    int slot_count = 0;
    size_t data_size;
    int res;
    unsigned char *outbuf;

    if (!chunk || !keks) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (!((key = dmsg_chunk_type_key_get(chunk->type))->section)) {
        RET_ERROR_INT(ERR_UNSPEC, "chunk type is invalid");
    }

    if (!key->encrypted) {
        RET_ERROR_INT(ERR_UNSPEC, "this chunk does not get encrypted");
    }

    if (key->payload == PAYLOAD_TYPE_SIGNATURE
        && chunk->state != MESSAGE_CHUNK_STATE_ENCODED)
    {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "signature chunk should to be encoded before it can be encrypted");
    }

    if (key->payload == PAYLOAD_TYPE_STANDARD
        && chunk->state != MESSAGE_CHUNK_STATE_SIGNED)
    {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "standard payload chunk should be signed before it can be "
            "encrypted");
    }

    if (!(data_size =
            _int_no_get_3b(&(chunk->payload_size[0])))
        || (data_size % 16))
    {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "data to be encrypted must not be of size 0 and must be a multiple "
            "of 16");
    }

    //todo rng is used, needs review
    if (_get_random_bytes(&(temp.iv[0]), sizeof(temp.iv))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not generate initialization vector");
    }

    if (_get_random_bytes(&(temp.aes_key[0]), sizeof(temp.aes_key))) {
        _secure_wipe((unsigned char *)&temp, sizeof(temp));
        RET_ERROR_INT(ERR_UNSPEC, "could not generate random key");
    }

    if (!(outbuf = malloc(data_size))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_INT(ERR_NOMEM, "could not allocate buffer for encrypted data");
    }

    if ((res =
            _encrypt_aes_256(
                outbuf,
                &(chunk->data[0]),
                data_size,
                temp.aes_key,
                temp.iv))
        < 0)
    {
        _secure_wipe((unsigned char *)&temp, sizeof(temp));
        RET_ERROR_INT(ERR_UNSPEC, "error encrypting data");
    } else if ((size_t)res != data_size) {
        RET_ERROR_INT(ERR_UNSPEC, "encrypted an unexpected number of bytes");
    }

    memcpy(&(chunk->data[0]), outbuf, data_size);
    free(outbuf);
    chunk->state = MESSAGE_CHUNK_STATE_UNKNOWN;

    if (key->auth_keyslot) {
        keyslot = dmsg_chunk_keyslot_get_by_num(chunk, ++slot_count);

        //todo rng used needs review
        if (_get_random_bytes(
                &(temp.random[0]),
                sizeof(temp.random)))
        {
            _secure_wipe((unsigned char *)&temp, sizeof(temp));
            RET_ERROR_INT(ERR_UNSPEC, "could not generate random array");
        }

        memcpy(
            &(keyslot->random[0]),
            &(temp.random[0]),
            sizeof(temp.random));
        memcpy(
            &(keyslot->iv[0]),
            &(temp.iv[0]),
            sizeof(temp.iv));
        memcpy(
            &(keyslot->aes_key[0]),
            &(temp.aes_key[0]),
            sizeof(temp.aes_key));

        if(dmsg_keyslot_encrypt(keyslot, &((*keks)[id_author]))) {
            _secure_wipe(
                (unsigned char *)&temp,
                sizeof(temp));
            _secure_wipe(
                keyslot,
                sizeof(*keyslot));
            RET_ERROR_INT(ERR_UNSPEC, "could not encrypt keyslot");
        }

    }

    if(key->orig_keyslot) {
        keyslot = dmsg_chunk_keyslot_get_by_num(chunk, ++slot_count);

        //todo rng used needs review
        if (_get_random_bytes(
                &(temp.random[0]),
                sizeof(temp.random)))
        {
            _secure_wipe((unsigned char *)&temp, sizeof(temp));
            RET_ERROR_INT(ERR_UNSPEC, "could not generate random array");
        }

        memcpy(
            &(keyslot->random[0]),
            &(temp.random[0]),
            sizeof(temp.random));
        memcpy(
            &(keyslot->iv[0]),
            &(temp.iv[0]),
            sizeof(temp.iv));
        memcpy(
            &(keyslot->aes_key[0]),
            &(temp.aes_key[0]),
            sizeof(temp.aes_key));

        if (dmsg_keyslot_encrypt(
                keyslot,
                &((*keks)[id_origin])))
        {
            _secure_wipe(&temp, sizeof(temp));
            _secure_wipe(keyslot, sizeof(*keyslot));
            RET_ERROR_INT(ERR_UNSPEC, "could not encrypt keyslot");
        }

    }

    if (key->dest_keyslot) {
        keyslot = dmsg_chunk_keyslot_get_by_num(chunk, ++slot_count);

        //todo rng used needs review
        if (_get_random_bytes(
                &(temp.random[0]),
                sizeof(temp.random)))
        {
            _secure_wipe((unsigned char *)&temp, sizeof(temp));
            RET_ERROR_INT(ERR_UNSPEC, "could not generate random array");
        }

        memcpy(
            &(keyslot->random[0]),
            &(temp.random[0]),
            sizeof(temp.random));
        memcpy(
            &(keyslot->iv[0]),
            &(temp.iv[0]),
            sizeof(temp.iv));
        memcpy(
            &(keyslot->aes_key[0]),
            &(temp.aes_key[0]),
            sizeof(temp.aes_key));

        if (dmsg_keyslot_encrypt(
                keyslot,
                &((*keks)[id_destination])))
        {
            _secure_wipe(
                (unsigned char *)&temp,
                sizeof(temp));
            _secure_wipe(
                (unsigned char *)keyslot,
                sizeof(keyslot));
            RET_ERROR_INT(ERR_UNSPEC, "could not encrypt keyslot");
        }

    }

    if (key->recp_keyslot) {
        keyslot = dmsg_chunk_keyslot_get_by_num(chunk, ++slot_count);

        //todo rng used needs review
        if (_get_random_bytes(
                &(temp.random[0]),
                sizeof(temp.random)))
        {
            _secure_wipe(
                (unsigned char *)&temp,
                sizeof(temp));
            RET_ERROR_INT(ERR_UNSPEC, "could not generate random array");
        }

        memcpy(
            &(keyslot->random[0]),
            &(temp.random[0]),
            sizeof(temp.random));
        memcpy(
            &(keyslot->iv[0]),
            &(temp.iv[0]),
            sizeof(temp.iv));
        memcpy(
            &(keyslot->aes_key[0]),
            &(temp.aes_key[0]),
            sizeof(temp.aes_key));

        if (dmsg_keyslot_encrypt(
                keyslot,
                &((*keks)[id_recipient])))
        {
            _secure_wipe(&temp, sizeof(temp));
            _secure_wipe(keyslot, sizeof(*keyslot));
            RET_ERROR_INT(ERR_UNSPEC, "could not encrypt keyslot");
        }

    }

    _secure_wipe(&temp, sizeof(temp));
    chunk->state = MESSAGE_CHUNK_STATE_ENCRYPTED;

    return 0;
}


/**
 * @brief
 *  takes a dmime message with chunks that have already been signed and for
 *  each chunk: fills the keyslots, encrypts the chunk and then encrypts the
 *  keyslots.
 * @param message
 *  pointer to the dmime message to be encrypted.
 * @param keks
 *  pointer to the set of key-encryption-keys to be used for encrypting
 *  keyslots.
 * @return
 *  0 on success, -1 on failure.
*/
static int
dmsg_chunks_message_encrypt(
    dmime_message_t *message,
    dmime_kekset_t *keks)
{
    if (!message || !keks) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (dmsg_message_state_get(message) != MESSAGE_STATE_CHUNKS_SIGNED) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "the message chunks must be signed before they can be encrypted");
    }

    if (dmsg_chunk_encrypt(message->origin, keks)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not encrypt origin chunk");
    }

    if (dmsg_chunk_encrypt(message->destination, keks)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not encrypt destination chunk");
    }

    if (dmsg_chunk_encrypt(message->common_headers, keks)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not encrypt common headers chunk");
    }

    if (dmsg_chunk_encrypt(message->other_headers, keks)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not encrypt other headers chunk");
    }

    if (message->display) {
        for (size_t i = 0; message->display[i]; i++) {
            if(dmsg_chunk_encrypt(message->display[i], keks)) {
                RET_ERROR_INT(ERR_UNSPEC, "could not encrypt display chunks");
            }
        }
    }

    if(message->attach) {
        for (size_t i = 0; message->attach[i]; i++) {
            if(dmsg_chunk_encrypt(message->attach[i], keks)) {
                RET_ERROR_INT(ERR_UNSPEC, "could not encrypt attachment chunks");
            }
        }
    }

    message->state = MESSAGE_STATE_ENCRYPTED;

    return 0;
}


/**
 * @brief
 *  derives the data needed for signing the tree signature.
 * @param msg
 *  pointer to the dmime message.
 * @param outsize
 *  used to store the size of the result.
 * @return
 *  array of data that gets signed for the tree signature.
 * @free_using{free}
*/
static unsigned char *
dmsg_treesig_data_get(
    dmime_message_t const *msg,
    size_t *outsize)
{
    // TODO this is probably too long and can be shortened but not sure how.

    unsigned int chunk_count = 0;
    unsigned char *result;

    if (!msg || !outsize) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (dmsg_message_state_get(msg) < MESSAGE_STATE_ENCRYPTED) {
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "the message should be encrypted before it is signed with the tree "
            "signature");
    }

    if (msg->ephemeral) {
        ++chunk_count;
    }

    if (msg->origin) {
        ++chunk_count;
    }

    if (msg->destination) {
        ++chunk_count;
    }

    if (msg->common_headers) {
        ++chunk_count;
    }

    if (msg->other_headers) {
        ++chunk_count;
    }

    if (msg->display) {
        for (size_t i = 0; msg->display[i]; i++) {
            ++chunk_count;
        }
    }

    if (msg->attach) {
        for (size_t i = 0; msg->attach[i]; i++) {
            ++chunk_count;
        }
    }

    if (!(result = malloc(chunk_count * SHA_512_SIZE))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate memory for data");
    }

    memset(result, 0, chunk_count * SHA_512_SIZE);
    *outsize = chunk_count * SHA_512_SIZE;
    chunk_count = 0;

    if (msg->ephemeral) {
        if (_compute_sha_hash(
                512,
                &(msg->ephemeral->type),
                msg->ephemeral->serial_size,
                result + (SHA_512_SIZE * chunk_count)))
        {
            free(result);
            RET_ERROR_PTR(ERR_UNSPEC, "could not hash ephemeral chunk");
        }

        ++chunk_count;
    }

    if (msg->origin) {
        if (_compute_sha_hash(
                512,
                &(msg->origin->type),
                msg->origin->serial_size,
                result + (SHA_512_SIZE * chunk_count)))
        {
            free(result);
            RET_ERROR_PTR(ERR_UNSPEC, "could not hash origin chunk");
        }

        ++chunk_count;
    }

    if (msg->destination) {
        if (_compute_sha_hash(
                512,
                &(msg->destination->type),
                msg->destination->serial_size,
                result + (SHA_512_SIZE * chunk_count)))
        {
            free(result);
            RET_ERROR_PTR(ERR_UNSPEC, "could not hash destination chunk");
        }
    }

    if (msg->common_headers) {
        if (_compute_sha_hash(
                512,
                &(msg->common_headers->type),
                msg->common_headers->serial_size,
                result + (SHA_512_SIZE * chunk_count)))
        {
            free(result);
            RET_ERROR_PTR(ERR_UNSPEC, "could not hash common headers chunk");
        }

        ++chunk_count;
    }

    if (msg->other_headers) {
        if (_compute_sha_hash(
                512,
                &(msg->other_headers->type),
                msg->other_headers->serial_size,
                result + (SHA_512_SIZE * chunk_count)))
        {
            free(result);
            RET_ERROR_PTR(ERR_UNSPEC, "could not hash other headers chunk");
        }

        ++chunk_count;
    }

    if (msg->display) {
        for (size_t i = 0; msg->display[i]; i++) {
            if (_compute_sha_hash(
                    512,
                    &(msg->display[i]->type),
                    msg->display[i]->serial_size,
                    result + (SHA_512_SIZE * chunk_count)))
            {
                free(result);
                RET_ERROR_PTR(ERR_UNSPEC, "could not hash a display chunk");
            }
            ++chunk_count;
        }
    }

    if(msg->attach) {
        for (size_t i = 0; msg->attach[i]; i++) {
            if (_compute_sha_hash(
                    512,
                    &(msg->attach[i]->type),
                    msg->attach[i]->serial_size,
                    result + (SHA_512_SIZE * chunk_count)))
            {
                free(result);
                RET_ERROR_PTR(ERR_UNSPEC, "could not hash an attachment chunk");
            }
            ++chunk_count;
        }
    }

    return result;
}


/**
 * @brief
 *  calculates the serialized size of the specified sections of a dmime message.
 * @param msg
 *  dmime message
 * @param sections
 *  sections specified.
 * @return
 *  size, 0 on error.
*/
static size_t
dmsg_sections_size_get(
    dmime_message_t const *msg,
    unsigned char sections)
{
    size_t size = 0, last = 0;

    if (!msg) {
        RET_ERROR_UINT(ERR_BAD_PARAM, NULL);
    }

    if (msg->ephemeral && (CHUNK_SECTION_ENVELOPE & sections)) {
        size += msg->ephemeral->serial_size;
    }

    if (msg->origin && (CHUNK_SECTION_ENVELOPE & sections)) {
        size += msg->origin->serial_size;
    }

    if (msg->destination && (CHUNK_SECTION_ENVELOPE & sections)) {
        size += msg->destination->serial_size;
    }

    if (msg->common_headers && (CHUNK_SECTION_METADATA & sections)) {
        size += msg->common_headers->serial_size;
    }

    if (msg->other_headers && (CHUNK_SECTION_METADATA & sections)) {
        size += msg->other_headers->serial_size;
    }

    if ((CHUNK_SECTION_DISPLAY & sections) && msg->display) {
        for (size_t i = 0; msg->display[i]; i++) {
            last = size;            // last is used to check for size overflow
            size += msg->display[i]->serial_size;

            if(last > size) {
                RET_ERROR_UINT(
                    ERR_UNSPEC,
                    "message size is exceeding the maximum size");
            }
        }
    }

    if ((CHUNK_SECTION_ATTACH & sections) && msg->attach) {
        for (size_t i = 0; msg->attach[i]; i++) {
            last = size;
            size += msg->attach[i]->serial_size;

            if(last > size) {
                RET_ERROR_UINT(
                    ERR_UNSPEC,
                    "message size is exceeding the maximum size");
            }
        }
    }

    if (msg->author_tree_sig
        && (dmsg_chunk_type_key_get(CHUNK_TYPE_SIG_AUTHOR_TREE)->section
            & sections))
    {
        size += msg->author_tree_sig->serial_size;
    }

    if (msg->author_full_sig
        && (dmsg_chunk_type_key_get(CHUNK_TYPE_SIG_AUTHOR_FULL)->section
            & sections))
    {
        size += msg->author_full_sig->serial_size;
    }

    if (msg->origin_meta_bounce_sig
        && (dmsg_chunk_type_key_get(CHUNK_TYPE_SIG_ORIGIN_META_BOUNCE)->section
            & sections))
    {
        size += msg->origin_meta_bounce_sig->serial_size;
    }

    if (msg->origin_display_bounce_sig
        && (dmsg_chunk_type_key_get(
                CHUNK_TYPE_SIG_ORIGIN_DISPLAY_BOUNCE)
            ->section
            & sections))
    {
        size += msg->origin_display_bounce_sig->serial_size;
    }

    if (msg->origin_full_sig
        && (dmsg_chunk_type_key_get(CHUNK_TYPE_SIG_ORIGIN_FULL)->section
            & sections))
    {
        size += msg->origin_full_sig->serial_size;
    }

    return size;
}


/**
 * @brief
 *  serializes the specified sections of a dmime message (only if encrypted).
 * @param msg
 *  dmime message to be serialized.
 * @param sections
 *  the bitmask of sections to serialize. see ::dmime_chunk_section_t.
 * @param outsize
 *  stores the output size.
 * @return
 *  pointer to the binary array containing the binary message.
 * @free_using{free}
*/
static unsigned char *
dmsg_sections_serialize(
    dmime_message_t const *msg,
    unsigned char sections,
    size_t *outsize)
{
    size_t total_size;
    size_t at = 0;
    unsigned char *result;

    if (!msg || !outsize) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (dmsg_message_state_get(msg) < MESSAGE_STATE_ENCRYPTED) {
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "a message should be encrypted before it is signed");
    }

    if (!(total_size = dmsg_sections_size_get(msg, sections))) {
        RET_ERROR_PTR(ERR_UNSPEC, "the total sections size is 0");
    }

    if (!(result = malloc(total_size))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(
            ERR_NOMEM,
            "could not allocate memory for serialized message");
    }

    memset(result, 0, total_size);
    *outsize = total_size;

    if (msg->ephemeral && (CHUNK_SECTION_ENVELOPE & sections)) {
        memcpy(
            result + at,
            &(msg->ephemeral->type),
            msg->ephemeral->serial_size);
        at += msg->ephemeral->serial_size;
    }

    if (msg->origin && (CHUNK_SECTION_ENVELOPE & sections)) {
        memcpy(
            result + at,
            &(msg->origin->type),
            msg->origin->serial_size);
        at += msg->origin->serial_size;
    }

    if (msg->destination && (CHUNK_SECTION_ENVELOPE & sections)) {
        memcpy(
            result + at,
            &(msg->destination->type),
            msg->destination->serial_size);
        at += msg->destination->serial_size;
    }

    if (msg->common_headers && (CHUNK_SECTION_METADATA & sections)) {
        memcpy(
            result + at,
            &(msg->common_headers->type),
            msg->common_headers->serial_size);
        at += msg->common_headers->serial_size;
    }

    if (msg->other_headers && (CHUNK_SECTION_METADATA & sections)) {
        memcpy(
            result + at,
            &(msg->other_headers->type),
            msg->other_headers->serial_size);
        at += msg->other_headers->serial_size;
    }

    if ((CHUNK_SECTION_DISPLAY & sections) && msg->display) {
        for (size_t i = 0; msg->display[i]; i++) {
            memcpy(
                result + at,
                &(msg->display[i]->type),
                msg->display[i]->serial_size);
            at += msg->display[i]->serial_size;
        }
    }

    if ((CHUNK_SECTION_ATTACH & sections) && msg->attach) {
        for (size_t i = 0; msg->attach[i]; i++) {
            memcpy(
                result + at,
                &(msg->attach[i]->type),
                msg->attach[i]->serial_size);
            at += msg->attach[i]->serial_size;
        }
    }

    if (CHUNK_SECTION_SIG & sections) {

        if(msg->author_tree_sig) {
            memcpy(
                result + at,
                &(msg->author_tree_sig->type),
                msg->author_tree_sig->serial_size);
            at += msg->author_tree_sig->serial_size;
        }

        if(msg->author_full_sig) {
            memcpy(
                result + at,
                &(msg->author_full_sig->type),
                msg->author_full_sig->serial_size);
            at += msg->author_full_sig->serial_size;
        }

        if(msg->origin_meta_bounce_sig) {
            memcpy(
                result + at,
                &(msg->origin_meta_bounce_sig->type),
                msg->origin_meta_bounce_sig->serial_size);
            at += msg->origin_meta_bounce_sig->serial_size;
        }

        if(msg->origin_display_bounce_sig) {
            memcpy(
                result + at,
                &(msg->origin_display_bounce_sig->type),
                msg->origin_display_bounce_sig->serial_size);
            at += msg->origin_display_bounce_sig->serial_size;
        }

        if(msg->origin_full_sig) {
            memcpy(
                result + at,
                &(msg->origin_full_sig->type),
                msg->origin_full_sig->serial_size);
            at += msg->origin_full_sig->serial_size;
        }

    }

    return result;
}


/**
 * @brief
 *  calculates the serialized size of the specified chunks from the first to
 *  last specified chunk types.
 * @param msg
 *  dmime message containing the chunks, the total serialized size of which
 *  will be calculated in the specified chunk type range.
 * @param first
 *  lower bound chunk type the size of which will be calculated.
 * @param last
 *  upper bound chunk type the size of which will be calculated.
 * @return
 *  size, 0 on error.
*/
static size_t
dmsg_chunks_size_get(
    dmime_message_t const *msg,
    dmime_chunk_type_t first,
    dmime_chunk_type_t last)
{
    size_t size = 0, temp;

    if (!msg) {
        RET_ERROR_UINT(ERR_BAD_PARAM, NULL);
    }

    if (last < first) {
        RET_ERROR_UINT(ERR_UNSPEC, "invalid chunk type bounds");
    }

    if (msg->ephemeral
        && (first <= CHUNK_TYPE_EPHEMERAL)
        && (CHUNK_TYPE_EPHEMERAL <= last))
    {
        size += msg->ephemeral->serial_size;
    }

    if (msg->origin
        && (first <= CHUNK_TYPE_ORIGIN)
        && (CHUNK_TYPE_ORIGIN <= last))
    {
        size += msg->origin->serial_size;
    }

    if (msg->destination
        && (first <= CHUNK_TYPE_DESTINATION)
        && (CHUNK_TYPE_DESTINATION <= last))
    {
        size += msg->destination->serial_size;
    }

    if (msg->common_headers
        && (first <= CHUNK_TYPE_META_COMMON)
        && (CHUNK_TYPE_META_COMMON <= last))
    {
        size += msg->common_headers->serial_size;
    }

    if (msg->other_headers
        && (first <= CHUNK_TYPE_META_OTHER)
        && (CHUNK_TYPE_META_OTHER <= last))
    {
        size += msg->other_headers->serial_size;
    }

    if ((first <= CHUNK_TYPE_DISPLAY_CONTENT)
        && (CHUNK_TYPE_DISPLAY_CONTENT <= last)
        && msg->display)
    {
        for (size_t i = 0; msg->display[i]; i++) {
            temp = size; // last is used to check for size overflow
            size += msg->display[i]->serial_size;

            if(temp > size) {
                RET_ERROR_UINT(
                    ERR_UNSPEC,
                    "message size is exceeding the maximum size");
            }
        }
    }

    if ((first <= CHUNK_TYPE_ATTACH_CONTENT)
        && (CHUNK_TYPE_ATTACH_CONTENT <= last)
        && msg->attach)
    {
        for (size_t i = 0; msg->attach[i]; i++) {
            temp = size;
            size += msg->attach[i]->serial_size;

            if(temp > size) {
                RET_ERROR_UINT(
                    ERR_UNSPEC,
                    "message size is exceeding the maximum size");
            }
        }
    }

    if (msg->author_tree_sig
        && (first <= CHUNK_TYPE_SIG_AUTHOR_TREE)
        && (CHUNK_TYPE_SIG_AUTHOR_TREE <= last))
    {
        size += msg->author_tree_sig->serial_size;
    }

    if (msg->author_full_sig
        && (first <= CHUNK_TYPE_SIG_AUTHOR_FULL)
        && (CHUNK_TYPE_SIG_AUTHOR_FULL <= last))
    {
        size += msg->author_full_sig->serial_size;
    }

    if (msg->origin_meta_bounce_sig
        && (first <= CHUNK_TYPE_SIG_ORIGIN_META_BOUNCE)
        && (CHUNK_TYPE_SIG_ORIGIN_META_BOUNCE <= last))
    {
        size += msg->origin_meta_bounce_sig->serial_size;
    }

    if (msg->origin_display_bounce_sig
        && (first <= CHUNK_TYPE_SIG_ORIGIN_DISPLAY_BOUNCE)
        && (CHUNK_TYPE_SIG_ORIGIN_DISPLAY_BOUNCE <= last))
    {
        size += msg->origin_display_bounce_sig->serial_size;
    }

    if (msg->origin_full_sig
        && (first <= CHUNK_TYPE_SIG_ORIGIN_FULL)
        && (CHUNK_TYPE_SIG_ORIGIN_FULL <= last))
    {
        size += msg->origin_full_sig->serial_size;
    }

    return size;
}

/**
 * @brief
 *  takes an encrypted dmime message and serializes its chunks sequentially
 *  from the first specified to the last.
 * @param msg
 *  pointer to the dmime message that will be serialized.
 * @param first
 *  the first chunk type to be serialized.
 * @param last
 *  last chunk type to be serialized.
 * @param outsize
 *  stores the size of the serialized message.
 * @return
 *  pointer to the serialized message.
 * @free_using{free}
*/
static unsigned char *
dmsg_chunks_serialize(
    dmime_message_t const *msg,
    dmime_chunk_type_t first,
    dmime_chunk_type_t last,
    size_t *outsize)
{
    size_t total_size;
    size_t at = 0;
    unsigned char *result;

    if (!msg || !outsize) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (dmsg_message_state_get(msg) < MESSAGE_STATE_ENCRYPTED) {
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "a message should be encrypted before it is serialized");
    }

    if (first > last) {
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "the first chunk to be serialized is higher than the last");
    }

    if (!(total_size = dmsg_chunks_size_get(msg, first, last))) {
        RET_ERROR_PTR(ERR_UNSPEC, "the total sections size is 0");
    }

    if (!(result = malloc(total_size))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(
            ERR_NOMEM,
            "could not allocate memory for serialized message");
    }

    memset(result, 0, total_size);
    *outsize = total_size;

    if (msg->ephemeral
        && (CHUNK_TYPE_EPHEMERAL <= last)
        && (first <= CHUNK_TYPE_EPHEMERAL))
    {
        memcpy(
            result + at,
            &(msg->ephemeral->type),
            msg->ephemeral->serial_size);
        at += msg->ephemeral->serial_size;
    }

    if (msg->origin
        && (CHUNK_TYPE_ORIGIN <= last)
        && (first <= CHUNK_TYPE_ORIGIN))
    {
        memcpy(
            result + at,
            &(msg->origin->type),
            msg->origin->serial_size);
        at += msg->origin->serial_size;
    }

    if (msg->destination
        && (CHUNK_TYPE_DESTINATION <= last)
        && (first <= CHUNK_TYPE_DESTINATION))
    {
        memcpy(
            result + at,
            &(msg->destination->type),
            msg->destination->serial_size);
        at += msg->destination->serial_size;
    }

    if (msg->common_headers
        && (CHUNK_TYPE_META_COMMON <= last)
        && (first <= CHUNK_TYPE_META_COMMON))
    {
        memcpy(
            result + at,
            &(msg->common_headers->type),
            msg->common_headers->serial_size);
        at += msg->common_headers->serial_size;
    }

    if (msg->other_headers
        && (CHUNK_TYPE_META_OTHER <= last)
        && (first <= CHUNK_TYPE_META_OTHER))
    {
        memcpy(
            result + at,
            &(msg->other_headers->type),
            msg->other_headers->serial_size);
        at += msg->other_headers->serial_size;
    }

    if ((CHUNK_TYPE_DISPLAY_CONTENT <= last)
        && (first <= CHUNK_TYPE_DISPLAY_CONTENT)
        && msg->display)
    {
        for (size_t i = 0; msg->display[i]; i++) {
            memcpy(
                result + at,
                &(msg->display[i]->type),
                msg->display[i]->serial_size);
            at += msg->display[i]->serial_size;
        }
    }

    if ((CHUNK_TYPE_ATTACH_CONTENT <= last)
        && (first <= CHUNK_TYPE_ATTACH_CONTENT)
        && msg->attach)
    {
        for (size_t i = 0; msg->attach[i]; i++) {
            memcpy(
                result + at,
                &(msg->attach[i]->type),
                msg->attach[i]->serial_size);
            at += msg->attach[i]->serial_size;
        }
    }

    memset(result + at, 0, ED25519_SIG_SIZE + 5);

    if (msg->author_tree_sig
        && (CHUNK_TYPE_SIG_AUTHOR_TREE <= last)
        && (first <= CHUNK_TYPE_SIG_AUTHOR_TREE))
    {
        memcpy(
            result + at,
            &(msg->author_tree_sig->type),
            msg->author_tree_sig->serial_size);
        at += msg->author_tree_sig->serial_size;
    }

    if (msg->author_full_sig
        && (CHUNK_TYPE_SIG_AUTHOR_FULL <= last)
        && (first <= CHUNK_TYPE_SIG_AUTHOR_FULL))
    {
        memcpy(
            result + at,
            &(msg->author_full_sig->type),
            msg->author_full_sig->serial_size);
        at += msg->author_full_sig->serial_size;
    }

    if (msg->origin_meta_bounce_sig
        && (CHUNK_TYPE_SIG_ORIGIN_META_BOUNCE <= last)
        && (first <= CHUNK_TYPE_SIG_ORIGIN_META_BOUNCE))
    {
        memcpy(
            result + at,
            &(msg->origin_meta_bounce_sig->type),
            msg->origin_meta_bounce_sig->serial_size);
        at += msg->origin_meta_bounce_sig->serial_size;
    }

    if (msg->origin_display_bounce_sig
        && (CHUNK_TYPE_SIG_ORIGIN_DISPLAY_BOUNCE <= last)
        && (first <= CHUNK_TYPE_SIG_ORIGIN_DISPLAY_BOUNCE))
    {
        memcpy(
            result + at,
            &(msg->origin_display_bounce_sig->type),
            msg->origin_display_bounce_sig->serial_size);
        at += msg->origin_display_bounce_sig->serial_size;
    }

    if (msg->origin_full_sig
        && (CHUNK_TYPE_SIG_ORIGIN_FULL <= last)
        && (first <= CHUNK_TYPE_SIG_ORIGIN_FULL))
    {
        memcpy(
            result + at,
            &(msg->origin_full_sig->type),
            msg->origin_full_sig->serial_size);
        at += msg->origin_full_sig->serial_size;
    }

    return result;
}

/**
 * @brief
 *  takes an encrypted dmime message and adds the two mandatory author
 *  signature chunks (tree and full).
 * @param message
 *  pointer to the dmime message that will be signed.
 * @param signkey
 *  pointer to the author's ed25519 private signing key that will be used for
 *  signatures.
 * @param keks
 *  pointer to a set of key encryption keys used to encrypt the keyslots.
 * @return
 *  0 on success, -1 on failure.
*/
static int
dmsg_chunks_sig_author_sign(
    dmime_message_t *message,
    ED25519_KEY *signkey,
    dmime_kekset_t *keks)
{
    unsigned char *data, sigbuf[ED25519_SIG_SIZE];
    size_t data_size;

    if (!message || !signkey || !keks) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (dmsg_message_state_get(message) != MESSAGE_STATE_ENCRYPTED) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "signature chunks can not be added to a message with unencrypted "
            "chunks");
    }

    memset(sigbuf, 0, sizeof(sigbuf));

    if (!(data = dmsg_treesig_data_get(message, &data_size))) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could retrieve input for the tree signature");
    }

    if (_ed25519_sign_data(data, data_size, signkey, sigbuf)) {
        free(data);
        RET_ERROR_INT(ERR_UNSPEC, "could not sign tree data");
    }

    free(data);

    if (!(message->author_tree_sig =
            dmsg_message_chunk_create(
                CHUNK_TYPE_SIG_AUTHOR_TREE,
                sigbuf,
                ED25519_SIG_SIZE,
                DEFAULT_CHUNK_FLAGS)))
    {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not create author tree signature chunk");
    }

    if (dmsg_chunk_encrypt(message->author_tree_sig, keks)) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not encrypt author tree signature chunk");
    }

    if (!(data =
            dmsg_chunks_serialize(
                message,
                CHUNK_TYPE_EPHEMERAL,
                CHUNK_TYPE_SIG_AUTHOR_TREE,
                &data_size)))
    {
        RET_ERROR_INT(ERR_UNSPEC, "could not serialize dmime message");
    }

    if (_ed25519_sign_data(data, data_size, signkey, sigbuf)) {
        free(data);
        RET_ERROR_INT(ERR_UNSPEC, "could not sign dmime message");
    }

    free(data);

    if (!(message->author_full_sig =
            dmsg_message_chunk_create(
                CHUNK_TYPE_SIG_AUTHOR_FULL,
                sigbuf,
                ED25519_SIG_SIZE,
                DEFAULT_CHUNK_FLAGS)))
    {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not not create author full signature chunk");
    }

    if (dmsg_chunk_encrypt(message->author_full_sig, keks)) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not encrypt author full signature chunk");
    }

    message->state = MESSAGE_STATE_AUTHOR_SIGNED;

    return 0;
}


/**
 * @brief
 *  takes an author signed dmime message and adds the three mandatory origin
 *  signature fields where each signature is filled with zeros.
 * @param message
 *  pointer to the dmime message that will be signed.
 * @param keks
 *  pointer to a set of key encryption keys used to encrypt the keyslots.
 * @return
 *  0 on success, -1 on failure.
*/
static int
dmsg_encode_origin_sig_chunks(
    dmime_message_t *message,
    dmime_kekset_t *keks)
{
    unsigned char blank_buf[ED25519_SIG_SIZE];

    if (!message || !keks) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (dmsg_message_state_get(message) != MESSAGE_STATE_AUTHOR_SIGNED) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "in order to add the origin signature chunks the message must "
            "already include author's signatures");
    }

    memset(blank_buf, 0, sizeof(blank_buf));
    message->state = MESSAGE_STATE_INCOMPLETE;

    if (!(message->origin_meta_bounce_sig =
            dmsg_message_chunk_create(
                CHUNK_TYPE_SIG_ORIGIN_META_BOUNCE,
                blank_buf,
                ED25519_SIG_SIZE,
                DEFAULT_CHUNK_FLAGS)))
    {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not create an origin meta bounce signature chunk");
    }

    if (dmsg_chunk_encrypt(message->origin_meta_bounce_sig, keks)) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not encrypt the origin meta bounce signature chunk");
    }

    if (!(message->origin_display_bounce_sig =
            dmsg_message_chunk_create(
                CHUNK_TYPE_SIG_ORIGIN_DISPLAY_BOUNCE,
                blank_buf,
                ED25519_SIG_SIZE,
                DEFAULT_CHUNK_FLAGS)))
    {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not create an origin display bounce signature chunk");
    }

    if (dmsg_chunk_encrypt(message->origin_display_bounce_sig, keks)) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not encrypt the origin display bounce signature chunk");
    }

    if (!(message->origin_full_sig =
            dmsg_message_chunk_create(
                CHUNK_TYPE_SIG_ORIGIN_FULL,
                blank_buf,
                ED25519_SIG_SIZE,
                DEFAULT_CHUNK_FLAGS)))
    {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not create an origin full signature chunk");
    }

    if(dmsg_chunk_encrypt(message->origin_full_sig, keks)) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not encrypt the origin full signature chunk");
    }

    message->state = MESSAGE_STATE_COMPLETE;

    return 0;
}


/**
 * @brief
 *  converts a dmime object to a dmime message, fully encrypting and signing
 *  the message !!as an author!!
 * @param object
 *  dmime object which contains all the envelope, metadata, display and
 *  attachment information.  as well as pointers to signets of author, origin,
 *  destination and recipient.
 * @param signkey
 *  the author's private ed25519 signing key which will be used.
 * @return
 *  a pointer to a fully signed and encrypted dmime message.
 * @free_using{dmsg_destroy_message}
*/
static dmime_message_t *
dmsg_message_encrypt(
    dmime_object_t *object,
    ED25519_KEY *signkey)
{
    EC_KEY *ephemeral;
    dmime_kekset_t kekset;
    dmime_message_t *result;
    size_t ecsize;
    unsigned char *bin_pub;

    if (!object || !signkey) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (dmsg_object_state_init(object) != DMIME_OBJECT_STATE_COMPLETE) {
        RET_ERROR_PTR(ERR_UNSPEC, "dmime object is not complete");
    }

    if (!(result = malloc(sizeof(dmime_message_t)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for message");
    }

    memset(result, 0, sizeof(dmime_message_t));
    result->state = MESSAGE_STATE_EMPTY;

    if (dmsg_message_chunks_encode(object, result)) {
        dmsg_message_destroy(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not encode message chunks");
    }

    if (dmsg_message_chunks_sign(result, signkey)) {
        dmsg_message_destroy(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not sign message chunks");
    }

    if (!(ephemeral = _generate_ec_keypair())) {
        dmsg_message_destroy(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not generate ephemeral encryption key");
    }

    if (dmsg_kek_out_derive_all(object, ephemeral, &kekset)) {
        dmsg_message_destroy(result);
        _free_ec_key(ephemeral);
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "could not derive kekset from signets and ephemeral key");
    }

    if (dmsg_chunks_message_encrypt(result, &kekset)) {
        _secure_wipe(kekset, sizeof(dmime_kekset_t));
        dmsg_message_destroy(result);
        _free_ec_key(ephemeral);
        RET_ERROR_PTR(ERR_UNSPEC, "could not encrypt message chunks");
    }

    if (!(bin_pub = _serialize_ec_pubkey(ephemeral, &ecsize))) {
        _secure_wipe(kekset, sizeof(dmime_kekset_t));
        _free_ec_key(ephemeral);
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "could not serialize public ephemeral ec key");
    }

    _free_ec_key(ephemeral);

    if (ecsize != EC_PUBKEY_SIZE) {
        _secure_wipe(kekset, sizeof(dmime_kekset_t));
        free(bin_pub);
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "serialized public key size did not match expected length");
    }

    if (!(result->ephemeral =
            dmsg_message_chunk_create(
                CHUNK_TYPE_EPHEMERAL,
                bin_pub,
                ecsize,
                DEFAULT_CHUNK_FLAGS)))
    {
        _secure_wipe(kekset, sizeof(dmime_kekset_t));
        free(bin_pub);
        RET_ERROR_PTR(ERR_UNSPEC, "could not create an ephemeral chunk");
    }

    free(bin_pub);

    if (dmsg_chunks_sig_author_sign(result, signkey, &kekset)) {
        _secure_wipe(kekset, sizeof(dmime_kekset_t));
        dmsg_message_destroy(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not add author signatures");
    }

    if (dmsg_encode_origin_sig_chunks(result, &kekset)) {
        _secure_wipe(kekset, sizeof(dmime_kekset_t));
        dmsg_message_destroy(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not add origin sig chunks");
    }

    //todo crypto information on the stack. should this have been memlocked?
    _secure_wipe(kekset, sizeof(dmime_kekset_t));

    return result;
}


/**
 * @brief
 *  converts the specified sections of a dmime message to a complete binary
 *  form. the message must be at least signed by author.
 * @param msg
 *  dmime message to be converted.
 * @param sections
 *  sections to be included.
 * @param tracing
 *  if set, include tracing, if clear don't include tracing.
 * @param outsize
 *  stores the output size of the binary.
 * @free_using{free}
*/
static unsigned char *
dmsg_message_serialize(
    dmime_message_t const *msg,
    unsigned char sections,
    unsigned char tracing,
    size_t *outsize)
{
    size_t trc_size = 0, msg_size, total_size, at = 0;
    unsigned char *result, *ser;

    if (!msg || !outsize) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (dmsg_message_state_get(msg) < MESSAGE_STATE_AUTHOR_SIGNED) {
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "the message must be at least signed by author in order "
            "to be converted to complete binary form");
    }

    if (tracing && msg->tracing) {
        trc_size = _int_no_get_2b(&(msg->tracing->size[0]));
    }

    if (!(ser = dmsg_sections_serialize(msg, sections, &msg_size))) {
        RET_ERROR_PTR(ERR_NOMEM, "could not serialize message sections");
    }

    total_size = MESSAGE_HEADER_SIZE + msg_size;

    if (tracing && msg->tracing) {
        total_size += TRACING_HEADER_SIZE + trc_size;
    }

    *outsize = total_size;

    if (!(result = malloc(total_size))) {
        PUSH_ERROR_SYSCALL("malloc");
        free(ser);
        RET_ERROR_PTR(
            ERR_NOMEM,
            "could not allocate memory for binary message");
    }

    memset(result, 0, total_size);

    if (tracing && msg->tracing) {
        _int_no_put_2b(result, (uint16_t)DIME_MSG_TRACING);
        at += 2;
        memcpy(
            result + at,
            (unsigned char *)msg->tracing,
            trc_size + TRACING_LENGTH_SIZE);
        at += trc_size + TRACING_LENGTH_SIZE;
    }

    _int_no_put_2b(result + at, (uint16_t)DIME_ENCRYPTED_MSG);
    at += 2;
    _int_no_put_4b(result + at, (uint32_t)msg_size);
    at += MESSAGE_LENGTH_SIZE;
    memcpy(result + at, ser, msg_size);
    free(ser);

    return result;
}


/**
 * @brief
 *  deserializes and adds a tracing object to the dmime message from the
 *  provided binary input.
 * @param msg
 *  dmime message object to which the tracing object will be attached.
 * @param in
 *  points to the first length byte of the array containing the binary tracing
 *  (points to the first character after the dime magic number for tracing
 *  objects).
 * @param insize
 *  maximum size of the input array.
 * @return  number of characters read as the tracing, 0 on error.
 */
static size_t
dmsg_tracing_load(
    dmime_message_t *msg,
    unsigned char const *in,
    size_t insize)
{
    size_t trc_size;

    if (!msg || !in || !insize) {
        RET_ERROR_UINT(ERR_BAD_PARAM, NULL);
    }

    trc_size = ((size_t)_int_no_get_2b(in) + TRACING_LENGTH_SIZE);
    if (trc_size > insize
        || trc_size <= TRACING_LENGTH_SIZE)
    {
        RET_ERROR_UINT(ERR_UNSPEC, "invalid message tracing length");
    }

    if(!(msg->tracing = malloc(trc_size))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_UINT(
            ERR_NOMEM,
            "could not allocate memory for tracing object");
    }

    memset(msg->tracing, 0, trc_size);
    memcpy(msg->tracing, in, trc_size);

    return trc_size;
}


/**
 * @brief
 *  Deserializes binary message input into an array of chunks of a specified
 *  section, regardless of chunk order.  This should only be used for display
 *  and attachment sections, because all other chunks need to maintain the
 *  correct sequence.
 * @param in
 *  pointer to the binary data.
 * @param insize
 *  size of the input array.
 * @param section
 *  the specified section from which the chunks should be deserialized.
 * @param read
 *  number of bytes read for all the chunks.
 * @return
 *  a pointer to a NULL-pointer terminated array of display chunk pointers.
 * @free_using{dmsg_destroy_message_chunk_chain}
*/
static dmime_message_chunk_t **
dmsg_section_deserialize(
    unsigned char const *in,
    size_t insize,
    dmime_chunk_section_t section,
    size_t *read)
{
    dmime_chunk_key_t *key;
    dmime_message_chunk_t **result;
    int num_keyslots, atchunk = 0;
    size_t num_chunks = 0, at = 0, serial_size, payload_size;

    if (!in || !insize || !read) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    while (
        at + CHUNK_HEADER_SIZE < insize
        && (key = dmsg_chunk_type_key_get(in[at]))->section
            == section)
    {

        num_keyslots =
            key->auth_keyslot + key->orig_keyslot
            + key->dest_keyslot + key->recp_keyslot;
        payload_size = _int_no_get_3b(in + 1);
        serial_size =
            CHUNK_HEADER_SIZE + payload_size
            + num_keyslots * sizeof(dmime_keyslot_t);
        at += serial_size;

        if(at > insize) {
            RET_ERROR_PTR(ERR_UNSPEC, "invalid chunk size");
        }

        ++num_chunks;
    }

    if (!(result =
            malloc(
                sizeof(dmime_message_chunk_t *)
                * (num_chunks + 1))))
    {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(
            ERR_NOMEM,
            "could not allocate memory for array of chunks");
    }

    memset(
        result,
        0,
        sizeof(dmime_message_chunk_t *) * (num_chunks + 1));
    at = 0;

    while (
        at + CHUNK_HEADER_SIZE < insize
        && (key = dmsg_chunk_type_key_get(in[at]))->section == section) {

        if (!(result[atchunk] =
                dmsg_chunk_deserialize(
                    in + at,
                    insize - at,
                    &serial_size)))
        {
            dmsg_message_chunk_chain_destroy(result);
            RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize a message chunk");
        }

        ++atchunk;
        at += serial_size;
    }

    *read = at;

    return result;
}


/**
 * @brief
 *  deserializes and adds a dmime message chunk object to the specified dmime
 *  message from the provided binary input.
 * @param msg
 *  dmime message object into which the message chunk will be deserialized.
 * @param in
 *  pointer to the first byte of the binary message chunk (the first byte
 *  should be the chunk type character).
 * @param insize
 *  size of the input array.
 * @param last_type
 *  pointer to the previous chunk type that was serialized.
 * @return
 *  number of characters read as the chunk, 0 on error
*/
static size_t
dmsg_message_deserialize_helper(
    dmime_message_t *msg,
    unsigned char const *in,
    size_t insize,
    dmime_chunk_type_t *last_type)
{
    dmime_chunk_key_t *key;
    dmime_chunk_section_t section;
    dmime_chunk_type_t type;
    dmime_message_chunk_t *chunk;
    size_t read = 0;

    if (!msg || !in || !insize || !last_type) {
        RET_ERROR_UINT(ERR_BAD_PARAM, NULL);
    }

    type = (dmime_chunk_type_t)in[0];

    if (type < *last_type) {
        RET_ERROR_UINT(ERR_UNSPEC, "invalid chunk order");
    }

    if (!((key = dmsg_chunk_type_key_get(type))->section)) {
        RET_ERROR_UINT(ERR_UNSPEC, "chunk type is invalid");
    }

    section = key->section;

    if (section != CHUNK_SECTION_DISPLAY
        && section != CHUNK_SECTION_ATTACH)
    {
        if(!(chunk = dmsg_chunk_deserialize(in, insize, &read))) {
            RET_ERROR_UINT(
                ERR_UNSPEC,
                "could not deserialize encrypted chunk");
        }

        switch(type) {
        // TODO support for alternate chunks needed
        case CHUNK_TYPE_EPHEMERAL:
            msg->ephemeral = chunk;
            break;
        case CHUNK_TYPE_ORIGIN:
            msg->origin = chunk;
            break;
        case CHUNK_TYPE_DESTINATION:
            msg->destination = chunk;
            break;
        case CHUNK_TYPE_META_COMMON:
            msg->common_headers = chunk;
            break;
        case CHUNK_TYPE_META_OTHER:
            msg->other_headers = chunk;
            break;
        case CHUNK_TYPE_SIG_AUTHOR_TREE:
            msg->author_tree_sig = chunk;
            break;
        case CHUNK_TYPE_SIG_AUTHOR_FULL:
            msg->author_full_sig = chunk;
            break;
        case CHUNK_TYPE_SIG_ORIGIN_META_BOUNCE:
            msg->origin_meta_bounce_sig = chunk;
            break;
        case CHUNK_TYPE_SIG_ORIGIN_DISPLAY_BOUNCE:
            msg->origin_display_bounce_sig = chunk;
            break;
        case CHUNK_TYPE_SIG_ORIGIN_FULL:
            msg->origin_full_sig = chunk;
            break;
        default:
            dmsg_message_chunk_destroy(chunk);
            RET_ERROR_UINT(ERR_UNSPEC, "invalid chunk type");
            break;

        }

    } else if(section == CHUNK_SECTION_DISPLAY) {
        if (!(msg->display =
                dmsg_section_deserialize(
                    in,
                    insize,
                    CHUNK_SECTION_DISPLAY,
                    &read)))
        {
            RET_ERROR_UINT(ERR_UNSPEC, "could not deserialize display chunks");
        }
    } else {
        if (!(msg->attach =
                dmsg_section_deserialize(
                    in,
                    insize,
                    CHUNK_SECTION_ATTACH, &read)))
        {
            RET_ERROR_UINT(
                ERR_UNSPEC,
                "could not deserialize attachment chunks");
        }
    }

    return read;
}


/**
 * @brief
 *  converts a binary message into a dmime message. the message is assumed to
 *  be encrypted.
 * @param in
 *  pointer to the binary message.
 * @param insize
 *  pointer to the binary size.
 * @return
 *  pointer to a dmime message structure.
 * @free_using{dmsg_destroy_message}
*/
static dmime_message_t *
dmsg_message_deserialize(
    unsigned char const *in,
    size_t insize)
{
    dime_number_t dime_num;
    dmime_chunk_type_t last_type = CHUNK_TYPE_NONE;
    dmime_message_t *result;
    int tracing;
    size_t read = 0, at = 0, msg_size;

    if (!in || !insize) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (insize < DIME_NUMBER_SIZE) {
        RET_ERROR_PTR(ERR_UNSPEC, "invalid message size");
    }

    if (!(result = malloc(sizeof(dmime_message_t)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(
            ERR_NOMEM,
            "could not allocate memory for message structure");
    }

    memset(result, 0, sizeof(dmime_message_t));

    if ((dime_num = _int_no_get_2b(in + at))
        == DIME_MSG_TRACING)
    {
        tracing = 1;
    } else if (dime_num == DIME_ENCRYPTED_MSG) {
        tracing = 0;
    } else {
        dmsg_message_destroy(result);
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "invalid dime magic number for an encrypted message");
    }

    at += DIME_NUMBER_SIZE;

    if (tracing
        && !(read
            = dmsg_tracing_load(
                result,
                in + at,
                insize - at)))
    {
        dmsg_message_destroy(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize tracing");
    }

    at += read;

    if (insize < DIME_NUMBER_SIZE + at) {
        dmsg_message_destroy(result);
        RET_ERROR_PTR(ERR_UNSPEC, "invalid message size");
    }

    if (tracing
        && ((dime_num = _int_no_get_2b(in + at))
            != DIME_ENCRYPTED_MSG))
    {
        dmsg_message_destroy(result);
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "invalid dime magic number for an ecnrypted message");
    }

    if (tracing) {
        at += DIME_NUMBER_SIZE;
    }

    if ((msg_size = _int_no_get_4b(in + at))
        != (insize - at - MESSAGE_LENGTH_SIZE))
    {
        dmsg_message_destroy(result);
        RET_ERROR_PTR(ERR_UNSPEC, "invalid message size");
    }

    at += 4;

    while (at < insize) {
        if (!(read =
                dmsg_message_deserialize_helper(
                    result,
                    in + at,
                    insize - at,
                    &last_type)))
        {
            dmsg_message_destroy(result);
            RET_ERROR_PTR(ERR_UNSPEC, "could not read chunk data");
        }

        at += read;
    }

    if (at != insize) {
        dmsg_message_destroy(result);
        RET_ERROR_PTR(ERR_UNSPEC, "invalid message size");
    }

    if (result->ephemeral
        && result->origin
        && result->destination
        && result->common_headers
        && result->author_tree_sig
        && result->author_full_sig
        && result->origin_full_sig)
    {
        result->state = MESSAGE_STATE_COMPLETE;
    } else {
        result->state = MESSAGE_STATE_INCOMPLETE;
    }

    return result;
}


/**
 * @brief
 *  calculates the key encryption key for a given private encryption key and
 *  dmime message, using the ephemeral key chunk in the message
 * @param msg
 *  pointer to the dmime message, which has the ephemeral key chunk to be used.
 * @param enckey
 *  private ec encryption key.
 * @param kek
 *  pointer to a dmime_kek_t - a key encryption key object that can be used to
 *  decrypt the keyslots.
 * @return
 *  returns 0 on success, -1 on failure.
 */
static int
dmsg_kek_in_derive(
    dmime_message_t const *msg,
    EC_KEY *enckey,
    dmime_kek_t *kek)
{
    dmime_ephemeral_payload_t *payload;
    EC_KEY *ephemeral;

    if (!msg || !enckey || !kek) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (!msg->ephemeral) {
        RET_ERROR_INT(ERR_UNSPEC, "no ephemeral chunk in specified message");
    }

    if (!(payload = dmsg_chunk_payload_get(msg->ephemeral))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not get ephemeral chunk payload");
    }

    if (!(ephemeral =
            _deserialize_ec_pubkey(
                *payload,
                EC_PUBKEY_SIZE,
                0)))
    {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not deserialize public ephemeral key");
    }

    if (_compute_aes256_kek(
            ephemeral,
            enckey,
            (unsigned char *)(kek)))
    {
        _free_ec_key(ephemeral);
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not compute aes256 kek and store it in the "
            "specified buffer");
    }

    return 0;
}


/**
 * @brief
 *  decrypts a keyslot using the specified key encryption key.
 * @param encrypted
 *  encrypted keyslot.
 * @param kek
 *  key encryption key.
 * @param decrypted
 *  keyslot into which the decrypted key is saved.
 * @return  0 on success, -1 on failure.
*/
static int
dmsg_keyslot_decrypt(
    dmime_keyslot_t *encrypted,
    dmime_kek_t *kek,
    dmime_keyslot_t *decrypted)
{
    dmime_keyslot_t temp;
    int result;

    if (!encrypted || !kek || !decrypted) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if ((result =
            _decrypt_aes_256(
                (unsigned char *)&temp,
                (unsigned char *)encrypted,
                sizeof(dmime_keyslot_t),
                kek->key, kek->iv))
        < 0)
    {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "an error occurred while decrypting keyslot");
    } else if(result != sizeof(dmime_keyslot_t)) {
        RET_ERROR_INT(ERR_UNSPEC, "decrypted an unexpected amount of bytes");
    }

    memcpy(
        decrypted->aes_key,
        temp.aes_key,
        AES_256_KEY_SIZE);
    memcpy(
        decrypted->random,
        temp.random,
        16);

    for (size_t i = 0; i < 16; ++i) {
        decrypted->iv[i] = temp.random[i] ^ temp.iv[i];
    }

    _secure_wipe(&temp, sizeof(dmime_keyslot_t));

    return 0;
}


/**
 * @brief
 *  decrypts specified chunk as specified actor with specified key encryption
 *  key.
 * @param chunk
 *  chunk to be decrypted.
 * @param actor
 *  actor doing the decryption.
 * @param kek
 *  key encryption key of the actor.
 * @return
 *  pointer to a new chunk with a decrypted payload and empty keyslots. don't
 *  leak!
 * @free_using{dmsg_destroy_message_chunk}
*/
static dmime_message_chunk_t *
dmsg_chunk_decrypt(
    dmime_message_chunk_t *chunk,
    dmime_actor_t actor,
    dmime_kek_t *kek)
{
    dmime_chunk_key_t *key;
    dmime_encrypted_payload_t payload;
    dmime_keyslot_t *keyslot_enc, keyslot_dec;
    dmime_message_chunk_t *result;
    int keyslot_num;
    size_t payload_size;
    int res;
    unsigned char *data;

    if (!chunk || !kek) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(key = dmsg_chunk_type_key_get(chunk->type))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not retrieve chunk type key");
    }

    if (!key->encrypted) {
        RET_ERROR_PTR(ERR_UNSPEC, "this chunk type does not get encrypted");
    }

    if (chunk->state != MESSAGE_CHUNK_STATE_ENCRYPTED) {
        RET_ERROR_PTR(ERR_UNSPEC, "this chunk is not encrypted");
    }

    switch(actor) {

    case id_author:
        if(!key->auth_keyslot) {
            RET_ERROR_PTR(ERR_UNSPEC, "invalid actor for specified chunk");
        }

        keyslot_num = 1;
        break;
    case id_origin:
        if(!key->orig_keyslot) {
            RET_ERROR_PTR(ERR_UNSPEC, "invalid actor for specified chunk");
        }

        keyslot_num = key->auth_keyslot + key->orig_keyslot;
        break;
    case id_destination:
        if(!key->dest_keyslot) {
            RET_ERROR_PTR(ERR_UNSPEC, "invalid actor for specified chunk");
        }

        keyslot_num =
            key->auth_keyslot
            + key->orig_keyslot
            + key->dest_keyslot;
        break;
    case id_recipient:
        if(!key->recp_keyslot) {
            RET_ERROR_PTR(ERR_UNSPEC, "invalid actor for specified chunk");
        }

        keyslot_num =
            key->auth_keyslot
            + key->orig_keyslot
            + key->dest_keyslot
            + key->recp_keyslot;
        break;
    default:
        RET_ERROR_PTR(ERR_UNSPEC, "invalid dmime actor");
        break;
    }

    if ((payload_size =
            _int_no_get_3b(chunk->payload_size))
            % 16)
    {
        RET_ERROR_PTR(ERR_UNSPEC, "invalid chunk payload size");
    }

    if (!(payload =
            (dmime_encrypted_payload_t)dmsg_chunk_payload_get(chunk)))
    {
        RET_ERROR_PTR(ERR_UNSPEC, "could not retrieve payload");
    }

    if (!(keyslot_enc =
            dmsg_chunk_keyslot_get_by_num(
                chunk,
                (size_t)keyslot_num)))
    {
        RET_ERROR_PTR(ERR_UNSPEC, "could not retrieve chunk keyslot");
    }

    if(dmsg_keyslot_decrypt(keyslot_enc, kek, &keyslot_dec)) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not decrypt keyslot");
    }

    if(!(data = malloc(payload_size))) {
        PUSH_ERROR_SYSCALL("malloc");
        _secure_wipe(&keyslot_dec, sizeof(dmime_keyslot_t));
        RET_ERROR_PTR(
            ERR_NOMEM,
            "could not allocate memory for decrypted data");
    }

    memset(data, 0, payload_size);

    if ((res =
            _decrypt_aes_256(
                data,
                payload,
                payload_size,
                keyslot_dec.aes_key,
                keyslot_dec.iv))
        < 0)
    {
        _secure_wipe(&keyslot_dec, sizeof(dmime_keyslot_t));
        free(data);
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "an error occurred while decrypting a chunk payload");
    } else if ((size_t)res != payload_size) {
        _secure_wipe(&keyslot_dec, sizeof(dmime_keyslot_t));
        free(data);
        RET_ERROR_PTR(ERR_UNSPEC, "decrypted an unexpected number of bytes");
    }

    _secure_wipe(&keyslot_dec, sizeof(dmime_keyslot_t));
    result =
        dmsg_chunk_payload_wrap(
            chunk->type,
            data,
            payload_size);
    free(data);

    if(!result) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not load data into message chunk");
    }

    return result;
}


/**
 * @brief
 *  destroy dmime object chunk list.
 * @param list
 *  pointer to a dmime object chunk list to be destroyed.
 */
static void
dmsg_object_chunklist_destroy(dmime_object_chunk_t *list)
{
    if (list)
    {
        dmsg_object_chunklist_destroy(list->next);

        if(list->data) {
            _secure_wipe(&(list->data[0]), list->data_size);
            free(list->data);
        }

        free(list);
    }

}


/**
 * @brief
 *  destroy a dmime object.
 * @param object
 *  pointer to dmime object to be destroyed.
 */
static void
dmsg_object_destroy(dmime_object_t *object)
{
    if(!object) {
        return;
    }

    sdsfree(object->author);
    sdsfree(object->recipient);
    sdsfree(object->origin);
    sdsfree(object->destination);
    sdsfree(object->fp_author);
    sdsfree(object->fp_origin);
    sdsfree(object->fp_destination);
    sdsfree(object->fp_recipient);

    if(object->signet_author) {
        dime_sgnt_signet_destroy(object->signet_author);
    }

    if(object->signet_origin) {
        dime_sgnt_signet_destroy(object->signet_origin);
    }

    if(object->signet_destination) {
        dime_sgnt_signet_destroy(object->signet_destination);
    }

    if(object->signet_recipient) {
        dime_sgnt_signet_destroy(object->signet_recipient);
    }

    dime_prsr_headers_destroy(object->common_headers);
    sdsfree(object->other_headers);
    dmsg_object_chunklist_destroy(object->display);
    dmsg_object_chunklist_destroy(object->attach);
    free(object);
}


/**
 * @brief
 *  retrieves author name for the following actors: author, origin, recipient.
 * @param msg
 *  dmime message the author of which is retrieved.
 * @param actor
 *  who is trying to get the message author.
 * @param kek
 *  key encryption key for the specified actor.
 * @return
 *  a newly allocated dmime object containing the envelope ids available to the
 *  actor.
 * @free_using{dmsg_destroy_object}
*/
static dmime_object_t *
dmsg_message_envelope_decrypt(
    dmime_message_t const *msg,
    dmime_actor_t actor,
    dmime_kek_t *kek)
{
    dmime_envelope_object_t *parsed;
    dmime_message_chunk_t *decrypted;
    dmime_object_t *result;
    size_t size;
    unsigned char *chunk_data;

    if (!msg || !kek) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (msg->state != MESSAGE_STATE_COMPLETE) {
        RET_ERROR_PTR(ERR_UNSPEC, "provided dmime message is not complete");
    }

    if (!(result = malloc(sizeof(dmime_object_t)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate memory for dmime object");
    }

    memset(result, 0, sizeof(dmime_object_t));
    result->state = DMIME_OBJECT_STATE_CREATION;
    result->actor = actor;

    if (actor != id_destination) {

        if (!(decrypted = dmsg_chunk_decrypt(msg->origin, actor, kek))) {
            dmsg_object_destroy(result);
            RET_ERROR_PTR(ERR_UNSPEC, "could not decrypt origin chunk");
        }

        if (!(chunk_data = dmsg_chunk_data_get(decrypted, &size))) {
            dmsg_message_chunk_destroy(decrypted);
            dmsg_object_destroy(result);
            RET_ERROR_PTR(ERR_UNSPEC, "could not retrieve chunk data");
        }

        if (!(parsed =
                dime_prsr_envelope_parse(
                    (char *)chunk_data,
                    size,
                    CHUNK_TYPE_ORIGIN)))
        {
            dmsg_message_chunk_destroy(decrypted);
            dmsg_object_destroy(result);
            RET_ERROR_PTR(ERR_UNSPEC, "could not parse origin chunk");
        }

        dmsg_message_chunk_destroy(decrypted);
        result->author = sdsdup(parsed->auth_recp);
        result->fp_author = sdsdup(parsed->auth_recp_fp);
        result->destination = sdsdup(parsed->dest_orig);
        result->fp_destination = sdsdup(parsed->dest_orig_fp);
        dime_prsr_envelope_destroy(parsed);
    }

    if (actor != id_origin) {

        if (!(decrypted = dmsg_chunk_decrypt(msg->destination, actor, kek))) {
            dmsg_object_destroy(result);
            RET_ERROR_PTR(ERR_UNSPEC, "could not decrypt destination chunk");
        }

        if (!(chunk_data = dmsg_chunk_data_get(decrypted, &size))) {
            dmsg_message_chunk_destroy(decrypted);
            dmsg_object_destroy(result);
            RET_ERROR_PTR(ERR_UNSPEC, "could not retrieve chunk data");
        }

        if (!(parsed =
                dime_prsr_envelope_parse(
                    (char *)chunk_data,
                    size,
                    CHUNK_TYPE_DESTINATION)))
        {
            dmsg_message_chunk_destroy(decrypted);
            dmsg_object_destroy(result);
            RET_ERROR_PTR(ERR_UNSPEC, "could not parse destination chunk");
        }

        dmsg_message_chunk_destroy(decrypted);
        result->recipient = sdsdup(parsed->auth_recp);
        result->fp_author = sdsdup(parsed->auth_recp_fp);
        result->origin = sdsdup(parsed->dest_orig);
        result->fp_origin = sdsdup(parsed->dest_orig_fp);
        dime_prsr_envelope_destroy(parsed);
    }

    result->state = DMIME_OBJECT_STATE_LOADED_ENVELOPE;

    return result;
}


/**
 * @brief
 *  verify chunk plaintext signature using the author's signet.
 * @param chunk
 *  pointer to a dmime message chunk, the plaintext signature of which will be
 *  verified.
 * @param signet
 *  author's signet used to verify signature.
 * @return
 *  1 if signature is valid, 0 if invalid, -1 if validation failed as a result
 *  of an error.
*/
static int
dmsg_chunk_sig_validate(
    dmime_message_chunk_t *chunk,
    signet_t *signet)
{
    int result;
    size_t data_size;
    unsigned char *data, *sig;

    if(!chunk || !signet) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if(chunk->state == MESSAGE_CHUNK_STATE_ENCRYPTED) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "can not verify plaintext signature of an encrypted chunk");
    }

    if(!(sig = dmsg_chunk_sig_plaintext_get(chunk))) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not retrieve plaintext signature from chunk");
    }

    if(!(data = dmsg_chunk_data_padded_get(chunk, &data_size))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not retrieve chunk padded data");
    }

    result = dime_sgnt_msg_sig_verify(signet, sig, data, data_size);

    if(result < 0) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "an error occurred while verifying plaintext signature");
    }

    return result;
}


/**
 * @brief
 *  decrypts, verifies and loads all contents of the origin chunk into the
 *  dmime object.
 * @param object
 *  pointer to the dmime object into which the chunk data will be loaded.
 * @param msg
 *  pointer to the dmime message containing the origin chunk.
 * @param kek
 *  the actor's key encryption key.
 * @return
 *  0 on success, -1 on failure.
*/
static int
dmsg_chunk_origin_decrypt(
    dmime_object_t *object,
    dmime_message_t const *msg,
    dmime_kek_t *kek)
{
    // TODO pull out reusuable code for dmsg_decrypt_destination ???

    dmime_actor_t actor;
    dmime_message_chunk_t *decrypted;
    int res;

    if(!object || !msg || !kek) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if((actor = object->actor) == id_destination) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "the destination can not decrypt origin chunk");
    }

    if(object->state != DMIME_OBJECT_STATE_LOADED_SIGNETS) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "the state of this dmime object does not indicate that "
            "the signets have been loaded");
    }

    if(!(decrypted = dmsg_chunk_decrypt(msg->origin, actor, kek))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not decrypt origin chunk");
    }

    if((res = dmsg_chunk_sig_validate(decrypted, object->signet_author)) < 0) {
        dmsg_message_chunk_destroy(decrypted);
        RET_ERROR_INT(
            ERR_UNSPEC,
            "error during validation of origin chunk signature");
    } else if (!res) {
        dmsg_message_chunk_destroy(decrypted);
        RET_ERROR_INT(
            ERR_UNSPEC,
            "origin chunk plaintext signature is invalid");
    }

    dmsg_message_chunk_destroy(decrypted);

    return 0;
}


/**
 * @brief
 *  decrypts, verifies and loads all contents of the destination chunk into the
 *  dmime object.
 * @param object
 *  pointer to the dmime object into which the chunk data will be loaded.
 * @param msg
 *  pointer to the dmime message containing the destination chunk.
 * @param kek
 *  the actor's key encryption key.
 * @return
 *  0 on success, -1 on failure.
*/
static int
dmsg_chunk_destination_decrypt(
    dmime_object_t *object,
    dmime_message_t const *msg,
    dmime_kek_t *kek)
{
    dmime_actor_t actor;
    dmime_message_chunk_t *decrypted;
    int res;

    if(!object || !msg || !kek) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if((actor = object->actor) == id_origin) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "the origin can not decrypt destination chunk");
    }

    if(object->state != DMIME_OBJECT_STATE_LOADED_SIGNETS) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "the state of this dmime object does not indicate that the "
            "signets have been loaded");
    }

    if(actor == id_destination) {
        return 0;
    }

    if(!(decrypted = dmsg_chunk_decrypt(msg->destination, actor, kek))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not decrypt destination chunk");
    }

    if((res = dmsg_chunk_sig_validate(decrypted, object->signet_author)) < 0) {
        dmsg_message_chunk_destroy(decrypted);
        RET_ERROR_INT(
            ERR_UNSPEC,
            "error during validation of destination chunk signature");
    } else if(!res) {
        dmsg_message_chunk_destroy(decrypted);
        RET_ERROR_INT(
            ERR_UNSPEC,
            "destination chunk plaintext signature is invalid");
    }

    dmsg_message_chunk_destroy(decrypted);

    return 0;
}


/**
 * @brief
 *  verify the signatures in author tree and full signature chunks.
 * @param object
 *  dmime object containing the ids and signets that the specified actor
 *  requires in order to complete message decryption and verification.
 * @param msg
 *  dmime message containing the signature chunks to be verified.
 * @param kek
 *  the current actor's key encryption key.
 * @return
 *  0 on success, -1 on failure.
 */
static int
dmsg_chunks_sig_author_validate(
    dmime_object_t *object,
    dmime_message_t const *msg,
    dmime_kek_t *kek)
{
    dmime_actor_t actor;
    dmime_message_chunk_t *decrypted;
    int result;
    size_t data_size, sig_size;
    unsigned char *data, *signature;

    if(!object || !msg || !kek) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if((actor = object->actor) == id_destination) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "destination domain can not verify author signatures");
    }

    if(object->state != DMIME_OBJECT_STATE_LOADED_SIGNETS) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "the state of this dmime object does not indicate that "
            "the signets have been loaded");
    }

    if(!(data = dmsg_treesig_data_get(msg, &data_size))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not computer tree sig data");
    }

    if(!(decrypted = dmsg_chunk_decrypt(msg->author_tree_sig, actor, kek))) {
        free(data);
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not decrypt author tree signature chunk");
    }

    if(!(signature = dmsg_chunk_data_get(decrypted, &sig_size))) {
        dmsg_message_chunk_destroy(decrypted);
        free(data);
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not retrieve author tree signature chunk data");
    } else if(sig_size != ED25519_SIG_SIZE) {
        dmsg_message_chunk_destroy(decrypted);
        free(data);
        RET_ERROR_INT(ERR_UNSPEC, "signature chunk has data of invalid size");
    }

    result =
        dime_sgnt_msg_sig_verify(
            object->signet_author,
            signature,
            data,
            data_size);
    dmsg_message_chunk_destroy(decrypted);
    free(data);

    if (result < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "error verifying author tree signature");
    } else if(!result) {
        RET_ERROR_INT(ERR_UNSPEC, "author tree signature is invalid");
    }

    if (!(data =
            dmsg_chunks_serialize(
                msg,
                CHUNK_TYPE_EPHEMERAL,
                CHUNK_TYPE_SIG_AUTHOR_TREE,
                &data_size)))
    {
        RET_ERROR_INT(ERR_UNSPEC, "could not serialize dmime message");
    }

    if (!(decrypted =
            dmsg_chunk_decrypt(
                msg->author_full_sig,
                actor,
                kek)))
    {
        free(data);
        RET_ERROR_INT(ERR_UNSPEC, "could not decrypt author full signature chunk");
    }

    if (!(signature = dmsg_chunk_data_get(decrypted, &sig_size))) {
        dmsg_message_chunk_destroy(decrypted);
        free(data);
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not retrieve author tree signature chunk data");
    } else if(sig_size != ED25519_SIG_SIZE) {
        dmsg_message_chunk_destroy(decrypted);
        free(data);
        RET_ERROR_INT(ERR_UNSPEC, "signature chunk has data of invalid size");
    }

    result =
        dime_sgnt_msg_sig_verify(
            object->signet_author,
            signature,
            data,
            data_size);
    dmsg_message_chunk_destroy(decrypted);
    free(data);

    if(result < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "error verifying author full signature");
    } else if(!result) {
        RET_ERROR_INT(ERR_UNSPEC, "author full signature is invalid");
    }

    return 0;
}

/**
 * @brief
 *  decrypts and verifies the common headers metadata chunk and loads it into
 *  the dmime object.
 * @param object
 *  dmime object that will have the contents of the common headers chunk loaded
 *  into it.
 * @param msg
 *  dmime object which contains the common headers chunk to be decrypted and
 *  verified.
 * @param kek
 *  the key encryption key for the current actor.
 * @return
 *  0 on success, -1 on failure.
 */
static int
dmsg_chunk_headers_common_decrypt(
    dmime_object_t *object,
    dmime_message_t const *msg,
    dmime_kek_t *kek)
{
    dmime_actor_t actor;
    dmime_message_chunk_t *decrypted;
    int res;
    size_t data_size;
    unsigned char *data;

    if(!object || !msg || !kek) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if(((actor = object->actor) == id_origin) || actor == id_destination) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "only the author and recipient have access to the metadata");
    }

    if(object->state != DMIME_OBJECT_STATE_LOADED_SIGNETS) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "the state of this dmime object does not indicate that the actor "
            "signets have been loaded");
    }

    if(!(decrypted = dmsg_chunk_decrypt(msg->common_headers, actor, kek))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not decrypt common headers chunk");
    }

    res = dmsg_chunk_sig_validate(decrypted, object->signet_author);

    if(res < 0) {
        dmsg_message_chunk_destroy(decrypted);
        RET_ERROR_INT(
            ERR_UNSPEC,
            "error during validation of common headers chunk signature");
    } else if(!res) {
        dmsg_message_chunk_destroy(decrypted);
        RET_ERROR_INT(
            ERR_UNSPEC,
            "common headers chunk plaintext signature is invalid");
    }

    if(!(data = dmsg_chunk_data_get(decrypted, &data_size))) {
        dmsg_message_chunk_destroy(decrypted);
        RET_ERROR_INT(ERR_UNSPEC, "could not retrieve chunk data");
    }

    if(!(object->common_headers = dime_prsr_headers_parse(data, data_size))) {
        dmsg_message_chunk_destroy(decrypted);
        RET_ERROR_INT(ERR_UNSPEC, "could not parse common headers chunk data");
    }

    dmsg_message_chunk_destroy(decrypted);

    return 0;
}


/**
 * @brief
 *  decrypts and verifies the other headers metadata chunk and loads it into
 *  the dmime object.
 * @param object
 *  dmime object that will have the contents of the other headers chunk loaded
 *  into it.
 * @param msg
 *  dmime object which contains the other headers chunk to be decrypted and
 *  verified.
 * @param kek
 *  the key encryption key for the current actor.
 * @return
 *  0 on success, -1 on failure.
 */
static int
dmsg_chunk_headers_other_decrypt(
    dmime_object_t *object,
    dmime_message_t const *msg,
    dmime_kek_t *kek)
{
    dmime_actor_t actor;
    dmime_message_chunk_t *decrypted;
    int res;
    size_t data_size;
    unsigned char *data;

    if(!object || !msg || !kek) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if(((actor = object->actor) == id_origin) || actor == id_destination) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "only the author and recipient have access to the metadata");
    }

    if(object->state != DMIME_OBJECT_STATE_LOADED_SIGNETS) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "the state of this dmime object does not indicate that the actor "
            "signets have been loaded");
    }

    if(!(decrypted = dmsg_chunk_decrypt(msg->other_headers, actor, kek))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not decrypt common headers chunk");
    }

    res = dmsg_chunk_sig_validate(decrypted, object->signet_author);

    if(res < 0) {
        dmsg_message_chunk_destroy(decrypted);
        RET_ERROR_INT(
            ERR_UNSPEC,
            "error during validation of other headers chunk signature");
    } else if(!res) {
        dmsg_message_chunk_destroy(decrypted);
        RET_ERROR_INT(
            ERR_UNSPEC,
            "other headers chunk plaintext signature is invalid");
    }

    if(!(data = dmsg_chunk_data_get(decrypted, &data_size))) {
        dmsg_message_chunk_destroy(decrypted);
        RET_ERROR_INT(ERR_UNSPEC, "could not retrieve chunk data");
    }

    object->other_headers = sdsnewlen(data, data_size);
    dmsg_message_chunk_destroy(decrypted);

    return 0;
}

/**
 * @brief
 *  creates a dmime object chunk with the specified type, data and flags.
 * @param type
 *  chunk type.
 * @param data
 *  pointer to an array that gets copied into newly allocated memory.
 * @param data_size
 *  length of data array.
 * @param flags
 *  specified flags for the object chunk.
 * @free_using{dmsg_destroy_object_chunk_list}
*/
static dmime_object_chunk_t *
dmsg_object_chunk_create(
    dmime_chunk_type_t type,
    unsigned char *data,
    size_t data_size,
    unsigned char flags)
{
    dmime_object_chunk_t *result;

    if(!data || !data_size) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!(result = malloc(sizeof(dmime_object_chunk_t)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(
            ERR_NOMEM,
            "could not allocate memory for a dmime object chunk");
    }

    memset(result, 0, sizeof(dmime_object_chunk_t));
    result->type = type;

    if(!(result->data = malloc(data_size))) {
        dmsg_object_chunklist_destroy(result);
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(
            ERR_NOMEM,
            "could not allocate memory for dmime object chunk data");
    }

    memset(result->data, 0, data_size);
    memcpy(result->data, data, data_size);
    result->flags = flags;
    result->data_size = data_size;

    return result;
}


/**
 * @brief
 *  decrypts and verifies all the available display and attachment chunks and
 *  loads them into the dmime object.
 * @param object
 *  dmime object that will contain the display and attachment data.
 * @param msg
 *  an encrypted dmime message from which display and attachment data is taken.
 * @param kek
 *  the key encryption key for the current actor.
 * @return  0 on success, -1 on failure.
*/
static int
dmsg_chunks_content_decrypt(
    dmime_object_t *object,
    dmime_message_t const *msg,
    dmime_kek_t *kek)
{
    dmime_actor_t actor;
    dmime_message_chunk_t *decrypted;
    dmime_object_chunk_t *chunk, *last = NULL;
    int res;
    unsigned char *data;
    size_t data_size;

    if(!object || !msg || !kek) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if(object->state != DMIME_OBJECT_STATE_LOADED_SIGNETS) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "the state of the object does not indicate that all necessary "
            "signets were loaded");
    }

    if(((actor = object->actor) == id_destination) || actor == id_origin) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "origin and destination do not have access to the content chunks");
    }

    if(object->display || object->attach) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "this object already contains data in its content chunks");
    }

    if(msg->display) {

        for (size_t i = 0; msg->display[i]; i++) {

            if (!(decrypted =
                    dmsg_chunk_decrypt(
                        msg->display[i],
                        actor,
                        kek)))
            {
                dmsg_object_chunklist_destroy(object->display);
                RET_ERROR_INT(ERR_UNSPEC, "could not decrypt display chunk");
            }

            res =
                dmsg_chunk_sig_validate(
                    decrypted,
                    object->signet_author);

            if (res < 0) {
                dmsg_object_chunklist_destroy(object->display);
                dmsg_message_chunk_destroy(decrypted);
                RET_ERROR_INT(
                    ERR_UNSPEC,
                    "error during validation of display chunk signature");
            } else if(!res) {
                dmsg_object_chunklist_destroy(object->display);
                dmsg_message_chunk_destroy(decrypted);
                RET_ERROR_INT(
                    ERR_UNSPEC,
                    "display chunk plaintext signature is invalid");
            }

            if (!(data = dmsg_chunk_data_get(decrypted, &data_size))) {
                dmsg_object_chunklist_destroy(object->display);
                dmsg_message_chunk_destroy(decrypted);
                RET_ERROR_INT(
                    ERR_UNSPEC,
                    "could not retrieve decrypted display chunk data");
            }

            if (!(chunk =
                    dmsg_object_chunk_create(
                        decrypted->type,
                        data,
                        data_size,
                        dmsg_chunk_flags_get(decrypted))))
            {
                dmsg_object_chunklist_destroy(object->display);
                dmsg_message_chunk_destroy(decrypted);
                RET_ERROR_INT(
                    ERR_UNSPEC,
                    "could not create an object chunk with the contents from "
                    "the message chunk");
            }

            dmsg_message_chunk_destroy(decrypted);

            if (!i) {
                object->display = chunk;
                last = object->display;
            } else if (chunk) {
                last->next = chunk;
                last = chunk;
            }
        }

    }

    if (msg->attach) {

        for (size_t i = 0; msg->attach[i]; i++) {

            if (!(decrypted = dmsg_chunk_decrypt(msg->attach[i], actor, kek))) {
                dmsg_object_chunklist_destroy(object->attach);
                RET_ERROR_INT(ERR_UNSPEC, "could not decrypt display chunk");
            }

            res = dmsg_chunk_sig_validate(decrypted, object->signet_author);

            if (res < 0) {
                dmsg_object_chunklist_destroy(object->attach);
                dmsg_message_chunk_destroy(decrypted);
                RET_ERROR_INT(
                    ERR_UNSPEC,
                    "error during validation of attachment chunk signature");
            } else if (!res) {
                dmsg_object_chunklist_destroy(object->attach);
                dmsg_message_chunk_destroy(decrypted);
                RET_ERROR_INT(
                    ERR_UNSPEC,
                    "attachment chunk plaintext signature is invalid");
            }

            if (!(data = dmsg_chunk_data_get(decrypted, &data_size))) {
                dmsg_object_chunklist_destroy(object->attach);
                dmsg_message_chunk_destroy(decrypted);
                RET_ERROR_INT(
                    ERR_UNSPEC,
                    "could not retrieve decrypted display chunk data");
            }

            if (!(chunk =
                    dmsg_object_chunk_create(
                        decrypted->type,
                        data,
                        data_size,
                        dmsg_chunk_flags_get(decrypted))))
            {
                dmsg_object_chunklist_destroy(object->attach);
                dmsg_message_chunk_destroy(decrypted);
                RET_ERROR_INT(
                    ERR_UNSPEC,
                    "could not create an object chunk with the contents "
                    "from the message chunk");
            }

            dmsg_message_chunk_destroy(decrypted);

            if(!i) {
                object->attach = chunk;
                last = object->attach;
            } else if (chunk) {
                last->next = chunk;
                last = chunk;
            }
        }
    }

    return 0;
}


/**
 * @brief
 *  decrypts, verifies and extracts all the information available to the author
 *  from the message.
 * @param obj
 *  dmime object into which the information is extracted, it must already
 *  contain the ids and signets of all the actors available to the author.
 * @param msg
 *  dmime message to be decrypted.
 * @param kek
 *  author's key encryption key.
 * @return  0 on success, -1 on failure.
*/
static int
dmsg_message_decrypt_as_auth(
    dmime_object_t *obj,
    dmime_message_t const *msg,
    dmime_kek_t *kek)
{
    if (!obj || !msg || !kek) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (msg->state != MESSAGE_STATE_COMPLETE) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "the specified dmime message is not complete");
    }

    if (obj->actor != id_author) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "the dmime object specifies actor other than author");
    }

    if (obj->state < DMIME_OBJECT_STATE_LOADED_ENVELOPE
        || !(obj->author && obj->signet_author)
        || !(obj->origin && obj->signet_origin)
        || !(obj->destination && obj->signet_destination)
        || !(obj->recipient && obj->signet_recipient))
    {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "not all necessary signets were retrieved to decrypt the message");
    }

    obj->state = DMIME_OBJECT_STATE_LOADED_SIGNETS;

    if (dmsg_chunk_origin_decrypt(obj, msg, kek)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not load origin chunk contents");
    }

    if (dmsg_chunk_destination_decrypt(obj, msg, kek)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not load destination chunk contents");
    }

    // TODO this needs to be changed for when not the entire message was
    // downloaded. author/recipient needs to be able to request the combined
    // hashes of all the chunks from their domain to verify the tree signature,
    // but the full author signature can't always be verified.
    // TODO technically author/recipients should only have to verify the tree
    // signature.
    if (dmsg_chunks_sig_author_validate(obj, msg, kek)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not verify author signature chunks");
    }

    // TODO this has similar issue as above. technically these signatures are
    // only for the destination to verify unless it's a bounce and then the
    // appropriate bounce signature needs to be verified by the recipient. how
    // do we know if it's a bounce?!
    //if (dmsg_verify_origin_sig_chunks(obj, msg, kek)) {
    //    RET_ERROR_INT(ERR_UNSPEC, "could not verify author signature chunks");
    //}

    if(dmsg_chunk_headers_common_decrypt(obj, msg, kek)) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not load common headers chunk contents");
    }

    if(dmsg_chunk_headers_other_decrypt(obj, msg, kek)) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not load common headers chunk contents");
    }

    if(dmsg_chunks_content_decrypt(obj, msg, kek)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not load mesage content");
    }

    obj->state = DMIME_OBJECT_STATE_COMPLETE;

    return 0;
}


/**
 * @brief
 *  decrypts, verifies and extracts all the information available to the origin
 *  from the message.
 * @param obj
 *  dmime object into which the information is extracted, it must already
 *  contain the ids and signets of all the actors available to the origin.
 * @param msg
 *  dmime message to be decrypted.
 * @param kek
 *  origin's key encryption key.
 * @return
 *  0 on success, -1 on failure.
*/
static int
dmsg_message_decrypt_as_orig(
    dmime_object_t *obj,
    dmime_message_t const *msg,
    dmime_kek_t *kek)
{
    if (!obj || !msg || !kek) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (msg->state != MESSAGE_STATE_COMPLETE) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "the specified dmime message is not complete");
    }

    if (obj->actor != id_origin) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "the dmime object specifies actor other than origin");
    }

    if (obj->state < DMIME_OBJECT_STATE_LOADED_ENVELOPE
        || !(obj->author && obj->signet_author)
        || !(obj->origin && obj->signet_origin)
        || !(obj->destination && obj->signet_destination))
    {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "not all necessary signets were retrieved to decrypt the message");
    }

    obj->state = DMIME_OBJECT_STATE_LOADED_SIGNETS;

    if (dmsg_chunk_origin_decrypt(obj, msg, kek)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not load origin chunk contents");
    }

    // TODO this needs to be changed for when not the entire message was
    // downloaded. author/recipient needs to be able to request the combined
    // hashes of all the chunks from their domain to verify the tree signature,
    // but the full author signature can't always be verified.
    // TODO technically author/recipients should only have to verify the tree
    // signature.
    if (dmsg_chunks_sig_author_validate(obj, msg, kek)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not verify author signature chunks");
    }

    obj->state = DMIME_OBJECT_STATE_COMPLETE;

    return 0;
}


/**
 * @brief
 *  signs the encrypted, author signed dmime message with the origin
 *  signatures. the origin signature chunks must already exist in order for the
 *  signing to occur.
 * @param msg
 *  dmime message that will be signed by the origin.
 * @param bounce_flags
 *  flags indicating bounce signatures that the origin will sign.
 * @param kek
 *  origin's key encryption key.
 * @param signkey
 *  origin's private signing key that will be used to sign the message. the
 *  public part of this key must be included in the origin signet either as the
 *  pok or one of the soks with the message signing flag.
 * @return
 *  0 on success, -1 on failure.
 */
static int
dmsg_chunks_sig_origin_sign(
    dmime_message_t *msg,
    unsigned char bounce_flags,
    dmime_kek_t *kek,
    ED25519_KEY *signkey)
{
    // TODO some code reusability is possible with a subroutine.

    dmime_keyslot_t *keyslot_enc, keyslot_dec;
    ed25519_signature sig;
    int res;
    size_t data_size, chunk_data_size;
    unsigned char *data, *chunk_data;

    if (!msg || !kek) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (!msg->origin_full_sig) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "the message does not have a chunk for origin full signature");
    }

    if (msg->origin_meta_bounce_sig) {

        if (bounce_flags & META_BOUNCE) {

            if (!(data =
                    dmsg_sections_serialize(
                        msg,
                        (CHUNK_SECTION_ENVELOPE | CHUNK_SECTION_METADATA),
                        &data_size)))
            {
                RET_ERROR_INT(
                    ERR_UNSPEC,
                    "could not serialize message for bounce metadata "
                    "signature");
            }

            res = _ed25519_sign_data(data, data_size, signkey, sig);
            free(data);

            if (res) {
                RET_ERROR_INT(
                    ERR_UNSPEC,
                    "could not sign data with origin's message signing key");
            }

            if (!(chunk_data =
                    dmsg_chunk_data_get(
                        msg->origin_meta_bounce_sig,
                        &chunk_data_size))
                || (chunk_data_size != ED25519_SIG_SIZE))
            {
                _secure_wipe(sig, sizeof(ed25519_signature));
                RET_ERROR_INT(
                    ERR_UNSPEC,
                    "could not locate chunk data segment");
            }

            if (!(keyslot_enc =
                    dmsg_chunk_keyslot_get_by_num(
                        msg->origin_meta_bounce_sig,
                        id_origin + 1)))
            {
                _secure_wipe(sig, sizeof(ed25519_signature));
                RET_ERROR_INT(
                    ERR_UNSPEC,
                    "can not retrieve origin meta bounce chunk keyslot");
            }

            if(dmsg_keyslot_decrypt(keyslot_enc, kek, &keyslot_dec)) {
                _secure_wipe(sig, sizeof(ed25519_signature));
                RET_ERROR_INT(ERR_UNSPEC, "can not decrypt keyslot");
            }

            if ((res =
                    _encrypt_aes_256(
                        chunk_data,
                        (unsigned char *)sig,
                        ED25519_SIG_SIZE,
                        keyslot_dec.aes_key,
                        keyslot_dec.iv))
                < 0)
            {
                _secure_wipe(&keyslot_dec, sizeof(dmime_keyslot_t));
                _secure_wipe(sig, sizeof(ed25519_signature));
                RET_ERROR_INT(
                    ERR_UNSPEC,
                    "error occurred while encrypting chunk data");
            } else if (res != ED25519_SIG_SIZE) {
                _secure_wipe(&keyslot_dec, sizeof(dmime_keyslot_t));
                _secure_wipe(sig, sizeof(ed25519_signature));
                mm_set(chunk_data, 0, ED25519_SIG_SIZE);
                RET_ERROR_INT(
                    ERR_UNSPEC,
                    "chunk data encryption operation did not return "
                    "expected length");
            }

            _secure_wipe(&keyslot_dec, sizeof(dmime_keyslot_t));
            _secure_wipe(sig, sizeof(ed25519_signature));
        } else {
            dmsg_message_chunk_destroy(msg->origin_meta_bounce_sig);
            msg->origin_meta_bounce_sig = NULL;
        }

    }

    if (msg->origin_display_bounce_sig && (bounce_flags & DISPLAY_BOUNCE)) {

        if (bounce_flags & DISPLAY_BOUNCE) {

            if (!(data =
                    dmsg_sections_serialize(
                        msg,
                        (CHUNK_SECTION_ENVELOPE
                            | CHUNK_SECTION_METADATA
                            | CHUNK_SECTION_DISPLAY),
                        &data_size))) {
                RET_ERROR_INT(
                    ERR_UNSPEC,
                    "could not serialize message for bounce display "
                    "signature");
            }

            res = _ed25519_sign_data(data, data_size, signkey, sig);
            free(data);

            if (res) {
                RET_ERROR_INT(
                    ERR_UNSPEC,
                    "could not sign data with origin's message signing key");
            }

            if (!(chunk_data =
                    dmsg_chunk_data_get(
                        msg->origin_display_bounce_sig,
                        &chunk_data_size))
                || (chunk_data_size != ED25519_SIG_SIZE))
            {
                _secure_wipe(sig, sizeof(ed25519_signature));
                RET_ERROR_INT(
                    ERR_UNSPEC,
                    "could not locate chunk data segment");
            }

            if (!(keyslot_enc =
                    dmsg_chunk_keyslot_get_by_num(
                        msg->origin_display_bounce_sig,
                        id_origin + 1)))
            {
                _secure_wipe(sig, sizeof(ed25519_signature));
                RET_ERROR_INT(
                    ERR_UNSPEC,
                    "can not retrieve origin display bounce chunk keyslot");
            }

            if (dmsg_keyslot_decrypt(keyslot_enc, kek, &keyslot_dec)) {
                _secure_wipe(sig, sizeof(ed25519_signature));
                RET_ERROR_INT(ERR_UNSPEC, "can not decrypt keyslot");
            }

            if ((res =
                    _encrypt_aes_256(
                        chunk_data,
                        (unsigned char *)sig,
                        ED25519_SIG_SIZE,
                        keyslot_dec.aes_key,
                        keyslot_dec.iv))
                < 0)
            {
                _secure_wipe(&keyslot_dec, sizeof(dmime_keyslot_t));
                _secure_wipe(sig, sizeof(ed25519_signature));
                RET_ERROR_INT(
                    ERR_UNSPEC,
                    "error occurred while encrypting chunk data");
            } else if (res != ED25519_SIG_SIZE) {
                _secure_wipe(&keyslot_dec, sizeof(dmime_keyslot_t));
                _secure_wipe(sig, sizeof(ed25519_signature));
                mm_set(chunk_data, 0, ED25519_SIG_SIZE);
                RET_ERROR_INT(
                    ERR_UNSPEC,
                    "chunk data encryption operation did not return "
                    "expected length");
            }

            _secure_wipe(&keyslot_dec, sizeof(dmime_keyslot_t));
            _secure_wipe(sig, sizeof(ed25519_signature));
        } else {
            dmsg_message_chunk_destroy(msg->origin_display_bounce_sig);
            msg->origin_display_bounce_sig = NULL;
        }

    }

    if (!(data =
            dmsg_chunks_serialize(
                msg,
                CHUNK_TYPE_EPHEMERAL,
                CHUNK_TYPE_SIG_ORIGIN_DISPLAY_BOUNCE,
                &data_size)))
    {
        RET_ERROR_INT(ERR_UNSPEC, "could not serialize dmime message");
    }

    res = _ed25519_sign_data(data, data_size, signkey, sig);
    free(data);

    if (res) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not sign data with origin's message signing key");
    }

    if (!(chunk_data =
            dmsg_chunk_data_get(
                msg->origin_full_sig,
                &chunk_data_size))
        || (chunk_data_size != ED25519_SIG_SIZE))
    {
        _secure_wipe(sig, sizeof(ed25519_signature));
        RET_ERROR_INT(ERR_UNSPEC, "could not retrieve chunk data segment");
    }

    if (!(keyslot_enc =
            dmsg_chunk_keyslot_get_by_num(
                msg->origin_full_sig,
                id_origin + 1)))
    {
        _secure_wipe(sig, sizeof(ed25519_signature));
        RET_ERROR_INT(
            ERR_UNSPEC,
            "can not retrieve origin full signature chunk keyslot");
    }

    if (dmsg_keyslot_decrypt(keyslot_enc, kek, &keyslot_dec)) {
        _secure_wipe(sig, sizeof(ed25519_signature));
        RET_ERROR_INT(ERR_UNSPEC, "can not decrypt keyslot");
    }

    if ((res =
            _encrypt_aes_256(
                chunk_data,
                (unsigned char *)sig,
                ED25519_SIG_SIZE,
                keyslot_dec.aes_key,
                keyslot_dec.iv))
        < 0)
    {
        _secure_wipe(&keyslot_dec, sizeof(dmime_keyslot_t));
        _secure_wipe(sig, sizeof(ed25519_signature));
        RET_ERROR_INT(
            ERR_UNSPEC,
            "error occurred while encrypting chunk data");
    } else if (res != ED25519_SIG_SIZE) {
        _secure_wipe(&keyslot_dec, sizeof(dmime_keyslot_t));
        _secure_wipe(sig, sizeof(ed25519_signature));
        mm_set(chunk_data, 0, ED25519_SIG_SIZE);
        RET_ERROR_INT(
            ERR_UNSPEC,
            "chunk data encryption operation did not return expected length");
    }

    _secure_wipe(&keyslot_dec, sizeof(dmime_keyslot_t));
    _secure_wipe(sig, sizeof(ed25519_signature));

    return 0;
}


/**
 * @brief
 *  verify the signatures in origin bounce and full signature chunks.
 * @param object
 *  dmime object containing the ids and signets that the specified actor
 *  requires in order to complete message decryption and verification.
 * @param msg
 *  dmime message containing the signature chunks to be verified.
 * @param kek
 *  the current actor's key encryption key.
 * @return
 *  0 on success, -1 on failure.
 */
// TODO Change the signature verification to use the routine from
// dime/sgnt/signet.h like in dmsg_chunks_sig_author_validate()
static int
dmsg_chunks_sig_origin_validate(
    dmime_object_t *object,
    dmime_message_t const *msg,
    dmime_kek_t *kek)
{
    ED25519_KEY *signkey;
    dmime_actor_t actor;
    dmime_message_chunk_t *decrypted;
    int result;
    size_t data_size, sig_size;
    unsigned char *data, *signature;

    if (!object || !msg || !kek) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (object->state != DMIME_OBJECT_STATE_LOADED_SIGNETS) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "the state of this dmime object does not indicate that the "
            "signets have been loaded");
    }

    actor = object->actor;

    if (!(signkey = dime_sgnt_signkey_fetch(object->signet_origin))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not retrieve author signing key");
    }

    if (msg->origin_meta_bounce_sig) {

        if (!(data =
                dmsg_sections_serialize(
                    msg,
                    (CHUNK_SECTION_ENVELOPE
                        | CHUNK_SECTION_METADATA),
                    &data_size)))
        {
            _free_ed25519_key(signkey);
            RET_ERROR_INT(
                ERR_UNSPEC,
                "could not serialize envelope and metadata message chunks");
        }

        if (!(decrypted =
                dmsg_chunk_decrypt(
                    msg->origin_meta_bounce_sig,
                    actor,
                    kek)))
        {
            free(data);
            _free_ed25519_key(signkey);
            RET_ERROR_INT(
                ERR_UNSPEC,
                "could not decrypt origin meta bounce chunk");
        }

        if (!(signature =
                dmsg_chunk_data_get(
                    decrypted,
                    &sig_size))
            || (sig_size != ED25519_SIG_SIZE))
        {
            dmsg_message_chunk_destroy(decrypted);
            free(data);
            _free_ed25519_key(signkey);
            RET_ERROR_INT(
                ERR_UNSPEC,
                "could not retrieve meta bounce chunk data");
        }

        result = _ed25519_verify_sig(data, data_size, signkey, signature);
        dmsg_message_chunk_destroy(decrypted);
        free(data);

        if (result < 0) {
            _free_ed25519_key(signkey);
            RET_ERROR_INT(
                ERR_UNSPEC,
                "error during validation of meta bounce origin signature");
        } else if(!result) {
            _free_ed25519_key(signkey);
            RET_ERROR_INT(
                ERR_UNSPEC,
                "meta bounce origin signature is invalid");
        }

    }

    if (msg->origin_display_bounce_sig) {

        if (!(data =
                dmsg_sections_serialize(
                    msg,
                    (CHUNK_SECTION_ENVELOPE
                        | CHUNK_SECTION_METADATA
                        | CHUNK_SECTION_DISPLAY),
                    &data_size)))
        {
            free_ed25519_key(signkey);
            RET_ERROR_INT(
                ERR_UNSPEC,
                "could not serialize envelope metadata and display "
                "message chunks");
        }

        if (!(decrypted =
                dmsg_chunk_decrypt(
                    msg->origin_display_bounce_sig,
                    actor,
                    kek)))
        {
            free(data);
            _free_ed25519_key(signkey);
            RET_ERROR_INT(
                ERR_UNSPEC,
                "could not decrypt origin display bounce chunk");
        }

        if (!(signature =
                dmsg_chunk_data_get(decrypted, &sig_size))
            || (sig_size != ED25519_SIG_SIZE))
        {
            dmsg_message_chunk_destroy(decrypted);
            free(data);
            _free_ed25519_key(signkey);
            RET_ERROR_INT(
                ERR_UNSPEC,
                "could not retrieve dispaly bounce chunk data");
        }

        result = _ed25519_verify_sig(data, data_size, signkey, signature);
        dmsg_message_chunk_destroy(decrypted);
        free(data);

        if (result < 0) {
            _free_ed25519_key(signkey);
            RET_ERROR_INT(
                ERR_UNSPEC,
                "error during validation of origin display bounce signaure");
        } else if (!result) {
            _free_ed25519_key(signkey);
            RET_ERROR_INT(
                ERR_UNSPEC,
                "origin display bounce signature is invalid");
        }

    }

    if (!(data =
            dmsg_chunks_serialize(
                msg,
                CHUNK_TYPE_EPHEMERAL,
                CHUNK_TYPE_SIG_ORIGIN_DISPLAY_BOUNCE,
                &data_size)))
    {
        _free_ed25519_key(signkey);
        RET_ERROR_INT(ERR_UNSPEC, "could not serialize the dmime message");
    }

    if (!(decrypted =
            dmsg_chunk_decrypt(
                msg->origin_full_sig,
                actor,
                kek)))
    {
        free(data);
        _free_ed25519_key(signkey);
        RET_ERROR_INT(ERR_UNSPEC, "could not decrypt chunk");
    }

    if (!(signature =
            dmsg_chunk_data_get(decrypted, &sig_size))
        || (sig_size != ED25519_SIG_SIZE))
    {
        dmsg_message_chunk_destroy(decrypted);
        free(data);
        _free_ed25519_key(signkey);
        RET_ERROR_INT(
            ERR_UNSPEC,
            "could not retrieve origin full sig chunk data");
    }

    result = _ed25519_verify_sig(data, data_size, signkey, signature);
    dmsg_message_chunk_destroy(decrypted);
    free(data);
    _free_ed25519_key(signkey);

    if(result < 0) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "error during validation of display bounce origin signature");
    } else if(!result) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "display bounce origin signature is invalid");
    }

    return 0;
}


/**
 * @brief
 *  decrypts, verifies and extracts all the information available to the
 *  destination from the message.
 * @param obj
 *  dmime object into which the information is extracted, it must already
 *  contain the ids and signets of all the actors available to the destination.
 * @param msg
 *  dmime message to be decrypted.
 * @param kek
 *  destination's key encryption key.
 * @return
 *  0 on success, -1 on failure.
*/
static int
dmsg_message_decrypt_as_dest(
    dmime_object_t *obj,
    dmime_message_t const *msg,
    dmime_kek_t *kek)
{
    if (!obj || !msg || !kek) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (msg->state != MESSAGE_STATE_COMPLETE) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "the specified dmime message is not complete");
    }

    if (obj->actor != id_destination) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "the dmime object specifies actor other than destination");
    }

    if (obj->state < DMIME_OBJECT_STATE_LOADED_ENVELOPE
        || !(obj->recipient && obj->signet_recipient)
        || !(obj->origin && obj->signet_origin)
        || !(obj->destination && obj->signet_destination))
    {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "not all necessary signets were retrieved to decrypt the message");
    }

    obj->state = DMIME_OBJECT_STATE_LOADED_SIGNETS;

    if(dmsg_chunk_destination_decrypt(obj, msg, kek)) {
        RET_ERROR_INT(
            ERR_UNSPEC, "could not load destination chunk contents");
    }

    // TODO Handle cases where the message is a bounce.
    if(dmsg_chunks_sig_origin_validate(obj, msg, kek)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not verify origin signature chunks");
    }

    obj->state = DMIME_OBJECT_STATE_COMPLETE;

    return 0;
}


/**
 * @brief
 *  decrypts, verifies and extracts all the information available to the
 *  recipient from the message.
 * @param obj
 *  dmime object into which the information is extracted, it must already
 *  contain the ids and signets of all the actors available to the recipient.
 * @param msg
 *  dmime message to be decrypted.
 * @param kek
 *  recipient's key encryption key.
 * @return
 *  0 on success, -1 on failure.
*/
static int
dmsg_message_decrypt_as_recp(
    dmime_object_t *obj,
    dmime_message_t const *msg,
    dmime_kek_t *kek)
{
    if (!obj || !msg || !kek) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (msg->state != MESSAGE_STATE_COMPLETE) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "the specified dmime message is not complete");
    }

    if (obj->actor != id_recipient) {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "the dmime object specifies actor other than recipient");
    }

    if (obj->state < DMIME_OBJECT_STATE_LOADED_ENVELOPE
        || !(obj->author && obj->signet_author)
        || !(obj->origin && obj->signet_origin)
        || !(obj->destination && obj->signet_destination)
        || !(obj->recipient && obj->signet_recipient))
    {
        RET_ERROR_INT(
            ERR_UNSPEC,
            "not all necessary signets were retrieved to decrypt the message");
    }

    obj->state = DMIME_OBJECT_STATE_LOADED_SIGNETS;

    if(dmsg_chunk_origin_decrypt(obj, msg, kek)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not load origin chunk contents");
    }

    if(dmsg_chunk_destination_decrypt(obj, msg, kek)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not load destination chunk contents");
    }

    // TODO this needs to be changed for when not the entire message was
    // downloaded. author/recipient needs to be able to request the combined
    // hashes of all the chunks from their domain to verify the tree signature,
    // but the full author signature can't always be verified.
    // TODO technically author/recipients should only have to verify the tree
    // signature.
    if (dmsg_chunks_sig_author_validate(obj, msg, kek)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not verify author signature chunks");
    }

    if(dmsg_chunks_sig_origin_validate(obj, msg, kek)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not verify recipient signature chunks");
    }

    if(dmsg_chunk_headers_common_decrypt(obj, msg, kek)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not load common headers chunk contents");
    }

    if(dmsg_chunk_headers_other_decrypt(obj, msg, kek)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not load common headers chunk contents");
    }

    if(dmsg_chunks_content_decrypt(obj, msg, kek)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not load mesage content");
    }

    obj->state = DMIME_OBJECT_STATE_COMPLETE;

    return 0;
}

/**
 * @brief
 *  dumps the contents of the dmime object.
 * @param object
 *  dmime object to be dumped.
 * @return
 *  0 on success, -1 on failure.
*/
static int
dmsg_object_dump(dmime_object_t *object)
{
    dmime_object_chunk_t *display;

    if (!object) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    printf("message viewer: %s\n", dmsg_actor_to_string(object->actor));
    printf("message state : %s\n", dmsg_object_state_to_string(object->state));

    if ((object->actor != id_destination) && object->author) {
        printf(
            "message auth  : %.*s\n",
            (int)sdslen(object->author),
            object->author);
    }

    if ((object->actor != id_origin) && object->origin) {
        printf(
            "message orig  : %.*s\n",
            (int)sdslen(object->origin),
            object->origin);
    }

    if ((object->actor != id_destination) && object->destination) {
        printf(
            "message dest  : %.*s\n",
            (int)sdslen(object->destination),
            object->destination);
    }

    if ((object->actor != id_origin) && object->recipient) {
        printf(
            "message recp  : %.*s\n",
            (int)sdslen(object->recipient),
            object->recipient);
    }

    if ((object->actor == id_author)
        || (object->actor == id_recipient))
    {

        for (unsigned int i = 0; i < DMIME_NUM_COMMON_HEADERS; ++i) {

            if (object->common_headers->headers[i]
                && dmime_header_keys[i].label)
            {
                printf(
                    "%s%.*s\r\n",
                    dmime_header_keys[i].label,
                    (int)sdslen(object->common_headers->headers[i]),
                    object->common_headers->headers[i]);
            }

        }

        printf(
            "other headers :\n %.*s\n",
            (int)sdslen(object->other_headers),
            object->other_headers);
        display = object->display;

        for (unsigned int i = 0; display; i++) {
            printf(
                "display %d     :\n %.*s\n",
                i + 1,
                (int)display->data_size, display->data);
            display = display->next;
        }

    }

    return 0;
}

/**
 * @brief
 *  retrieves pointer to the specified chunk type key from the global chunk key
 *  structure.
 * @param type
 *  specified chunk type.
 * @return
 *  returns pointer to a dmime_chunk_key_t structure.
*/
static dmime_chunk_key_t *
dmsg_chunk_type_key_get(dmime_chunk_type_t type)
{
    return &(dmime_chunk_keys[type]);
}

/**
 * @brief
 *  generates a random value and calculates the padding byte and padding length
 *  for a given input size and padding algorithm
 * @param dsize
 *  input size
 * @param flags
 *  chunk flags containing the flag which specifies which padding algorithm is
 *  used
 * @param padlen
 *  receives the length of the padding
 * @param padbyte
 *  receives the byte with which to do the padding
 * @return
 *  0 on success, -1 on failure.
*/
static int
dmsg_chunk_padlen_get(
    size_t dsize,
    unsigned char flags,
    unsigned int *padlen,
    unsigned char *padbyte)
{
    unsigned char rand;
    unsigned char temp;

    if (!padlen || !padbyte) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    // TODO fixme use of random number generator, will need some modification
    // in the future for seeding, thread-safety, etc.

    if (_get_random_bytes(&rand, 1)) {
        RET_ERROR_INT(ERR_UNSPEC, "could not generate a random byte");
    }

    if (flags & ALTERNATE_PADDING_ALGORITHM_ENABLED) {
        *padlen = 16 * rand + 16 - (dsize % 16);
        *padbyte = rand;
    } else {
        if (dsize < MINIMUM_PAYLOAD_SIZE) {
            temp = MINIMUM_PAYLOAD_SIZE - dsize;
            *padlen = temp + (16 * (rand % ((dsize / 16) + 1)));
            *padbyte = (unsigned char)(*padlen);
        } else {
            *padlen = 16 - (dsize % 16) + (16 * (rand % 16));
            *padbyte = (unsigned char)(*padlen);
        }
    }

    return 0;
}

/**
 * @brief
 *  returns the payload of the specified dmime message chunk.
 * @param chunk
 *  pointer to the dmime chunk. its type, state and payload size must be
 *  initialized.
 * @return
 *  void pointer to be casted to the appropriate payload structure, NULL on
 *  failure. do not free!
*/
static void *
dmsg_chunk_payload_get(dmime_message_chunk_t *chunk)
{
    dmime_chunk_key_t *key;

    if (!chunk) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (chunk->state < MESSAGE_CHUNK_STATE_CREATION) {
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "cannot retrieve the payload structure from an "
            "uninitialized chunk");
    }

    if (!((key = dmsg_chunk_type_key_get(chunk->type))->section)) {
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "cannot retrieve the chunk type key for the specified chunk");
    }

    if (key->payload == PAYLOAD_TYPE_NONE) {
        RET_ERROR_PTR(ERR_UNSPEC, "specified chunk is of unknown type");
    }

    return &(chunk->data[0]);
}

/**
 * @brief
 *  returns pointer to the specified keyslot of the dmime message chunk.
 * @param chunk
 *  pointer to the dmime message chunk.
 * @param num
 *  number of the desired keyslot.
 * @return
 *  pointer to the keyslot. do not free!
*/
static dmime_keyslot_t *
dmsg_chunk_keyslot_get_by_num(
    dmime_message_chunk_t *chunk,
    size_t num)
{
    dmime_chunk_key_t *key;
    size_t num_slots;

    if (!chunk || num < 1 || num > 4) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!((key = dmsg_chunk_type_key_get(chunk->type))->section)) {
        RET_ERROR_PTR(ERR_UNSPEC, "specified chunk type is invalid");
    }

    if ((num_slots =
            key->auth_keyslot
            + key->orig_keyslot
            + key->dest_keyslot
            + key->recp_keyslot)
        < num)
    {
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "there are less keyslots than the keyslot number specified");
    }

    switch(key->payload) {

    case PAYLOAD_TYPE_SIGNATURE:
        return
            (dmime_keyslot_t *)(&(chunk->data[0])
            + ED25519_SIG_SIZE
            + ((num - 1) * sizeof(dmime_keyslot_t)));
    case PAYLOAD_TYPE_STANDARD:
        return
            (dmime_keyslot_t *)(&(chunk->data[0])
            + _int_no_get_3b(&(chunk->payload_size[0]))
            + ((num - 1) * sizeof(dmime_keyslot_t)));
    default:
        RET_ERROR_PTR(ERR_UNSPEC, "there are no keyslots for this chunk");
    }
}


/**
 * @brief
 *  destroys dmime message chunk.
 * @param chunk
 *  dmime message chunk to be destroyed.
*/
static void
dmsg_message_chunk_destroy(dmime_message_chunk_t *chunk)
{
    if(!chunk) {
        return;
    }

    _secure_wipe(
        chunk,
        sizeof(dmime_message_chunk_state_t)
        + sizeof(size_t)
        + chunk->serial_size);
    free(chunk);
}

/**
 * @brief
 *  destroys dmime message chunk ptr chain.
 * @param chunks
 *  dmime message chunk pointer chain.
*/
static void
dmsg_message_chunk_chain_destroy(dmime_message_chunk_t **chunks)
{
    if(!chunks) {
        return;
    }

    for(size_t i = 0; chunks[i]; ++i) {
        dmsg_message_chunk_destroy(chunks[i]);
    }

    free(chunks);
}

/**
 * @brief
 *  allocates memory for and encodes a dmime_message_chunk_t structure with
 *  data provided.
 * @param type
 *  type of chunk being created. is necessary to calculate total number of
 *  bytes that must be allocated.
 * @param data
 *  data that will be encoded into the chunk.
 * @param insize
 *  size of data.
 * @param flags
 *  flags to be set for chunk, only relevant for standard payload chunk types.
 * @return
 *  pointer to the newly allocated and encoded dmime_message_chunk_t structure.
 * @free_using{dmsg_destroy_message_chunk}
*/
static dmime_message_chunk_t *
dmsg_message_chunk_create(
    dmime_chunk_type_t type,
    unsigned char const *data,
    size_t insize,
    unsigned char flags)
{
    dmime_chunk_key_t *key;
    void *payload;
    dmime_message_chunk_t *result;
    // total_size is the amount of memory needed to be allocated to the
    // dmime_message_chunk_t structure
    // serial_size is the size of the serialized result, corresponds to
    // result->serial_size
    size_t total_size = 0;
    size_t serial_size = CHUNK_HEADER_SIZE;
    // payload_size needs to be less than or equal to UNSIGNED_MAX_3_BYTE,
    // corresponds to result->payload_size
    // data_size is specific only to the standard payload, corresponds to
    uint32_t payload_size = 0;
    uint32_t data_size = 0;
    unsigned char padbyte = 0;
    unsigned int num_keyslots = 0, padlen = 0;

    if(!data || !insize) {
        // currently we do not support chunks with empty payloads todo
        RET_ERROR_PTR(ERR_BAD_PARAM, "no data provided for the chunk");
    }

    //check that the input size is not too big
    if(insize > UNSIGNED_MAX_3_BYTE) {
        RET_ERROR_PTR(ERR_UNSPEC, "the input data size is too large");
    }

    //get the chunk type key
    if(!((key = dmsg_chunk_type_key_get(type))->section)) {
        RET_ERROR_PTR(ERR_UNSPEC, "specified chunk type is invalid");
    }

    //switch statement for payload type to determine the payload size, payload
    //size will be finalized after this
    switch(key->payload) {

    case PAYLOAD_TYPE_SIGNATURE:
        // check that signature length is appropriate
        if(insize != ED25519_SIG_SIZE) {
            RET_ERROR_PTR(
                ERR_UNSPEC,
                "provided data does not have correct size to be a signature");
        }
        // payload size will be equal to the input size
        payload_size = insize;
        break;
    case PAYLOAD_TYPE_EPHEMERAL:
        // check that the public encryption key length is appropriate
        if(insize != EC_PUBKEY_SIZE) {
            RET_ERROR_PTR(
                ERR_UNSPEC,
                "provided data does not have correct size to be a public "
                "encryption key");
        }
        // payload size will be equal to the input size
        payload_size = insize;
        break;
    case PAYLOAD_TYPE_STANDARD:
        // calculate padding length and padding byte according to the specified
        // flag
        if(dmsg_chunk_padlen_get(insize + 69, flags, &padlen, &padbyte)) {
            RET_ERROR_PTR(ERR_UNSPEC, "could not calculate padding");
        }
        //payload size will be equal to the sum of the following:
        //64 bytes for payload signature, 3 bytes for the length, 1 byte for
        //flags, 1 byte for padding byte, input size, padding size
        payload_size = 69 + insize + padlen;
        //data size is the input size
        data_size = insize;
        break;
    default:
        RET_ERROR_PTR(ERR_UNSPEC, "unsupported payload type");
        break;

    }

    // if the payload size is greater than the largest number that can be
    // represented by 3 bytes, it's too big //TODO FIXME what if the padding
    // made it too big?
    if(payload_size > UNSIGNED_MAX_3_BYTE) {
        RET_ERROR_PTR(ERR_UNSPEC, "chunk size is too large");
    }

    // add the payload size to the serialized size
    serial_size += payload_size;
    // add the sizes of state and serial_size structure members to total_size
    total_size += sizeof(dmime_message_chunk_state_t) + sizeof(size_t);

    // use the key to find the number of keyslots for particular chunk_type
    if(key->encrypted) {
        num_keyslots +=
            (key->auth_keyslot
            + key->orig_keyslot
            + key->dest_keyslot
            + key->recp_keyslot);
    }

    // add the length for all the needed keyslots to the serialized size,
    // serialized size is now finalized
    serial_size += (num_keyslots * sizeof(dmime_keyslot_t));
    // add the serialized size to the total size, total size is now finalized
    total_size += serial_size;

    // allocate memory to the message chunk structure
    if(!(result = malloc(total_size))) {
        RET_ERROR_PTR(
            ERR_NOMEM,
            "could not allocate memory for message chunk structure");
    }

    //flush it and set the static members, set state to creation as we have not
    //finished encoding the chunk
    memset(result, 0, total_size);
    result->state = MESSAGE_CHUNK_STATE_NONE;
    result->serial_size = serial_size;
    result->type = (unsigned char)type;
    _int_no_put_3b(&(result->payload_size[0]), payload_size);
    result->state = MESSAGE_CHUNK_STATE_CREATION;

    // get chunk payload
    if(!(payload = dmsg_chunk_payload_get(result))) {
        dmsg_message_chunk_destroy(result);
        RET_ERROR_PTR(ERR_UNSPEC, "could not retrieve standard chunk");
    }

    //this switch statement will encode the data into the chunk
    switch(key->payload) {

    case PAYLOAD_TYPE_SIGNATURE:
        // copy the data into the payload
        memcpy(payload, data, ED25519_SIG_SIZE);
        break;
    case PAYLOAD_TYPE_EPHEMERAL:
        //copy the data into the payload
        memcpy(payload, data, EC_PUBKEY_SIZE);
        break;
    case PAYLOAD_TYPE_STANDARD:
        //set the data segment 3 byte size
        _int_no_put_3b(
            &(((dmime_standard_payload_t *)payload)->data_size[0]),
            data_size);
        //set the flags byte
        ((dmime_standard_payload_t *)payload)->flags = flags;
        //set the pad byte
        ((dmime_standard_payload_t *)payload)->pad_len = padbyte;
        //copy the data into the payload
        memcpy(
            &(((dmime_standard_payload_t *)payload)->data[0]),
            data,
            data_size);
        //pad the payload
        memset(
            &(((dmime_standard_payload_t *)payload)->data[data_size]),
            padbyte,
            padlen);
        break;
    default:
        dmsg_message_chunk_destroy(result);
        RET_ERROR_PTR(ERR_UNSPEC, "unsupported payload type");
        break;
    }

    // encoding is complete
    result->state = MESSAGE_CHUNK_STATE_ENCODED;

    return result;
}


/**
 * @brief
 *  deserializes an encrypted chunk from binary data.
 * @param in
 *  pointer to the binary data of an encrypted chunk.
 * @param insize
 *  maximum size of provided data (not guaranteed to contain only the chunk
 *  specified by the provided pointer).
 * @param read
 *  stores number of bytes read for this chunk.
 * @return
 *  pointer to a dmime message chunk object in encrypted state, NULL on error.
 * free_using{dmsg_destroy_message_chunk}
*/
static dmime_message_chunk_t *
dmsg_chunk_deserialize(
    unsigned char const *in,
    size_t insize,
    size_t *read)
{
    dmime_chunk_key_t *key;
    dmime_chunk_type_t type;
    dmime_message_chunk_t *result;
    int num_keyslots;
    size_t payload_size, serial_size, chunk_size;

    if(!in || !insize || !read) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    type = (dmime_chunk_type_t)in[0];

    if(!((key = dmsg_chunk_type_key_get(type))->section)) {
        RET_ERROR_PTR(ERR_UNSPEC, "chunk type is invalid");
    }

    payload_size = _int_no_get_3b((void *)(in + 1));

    num_keyslots =
        key->auth_keyslot
        + key->orig_keyslot
        + key->dest_keyslot
        + key->recp_keyslot;
    // total number of bytes to be read:
    if ((serial_size =
            CHUNK_HEADER_SIZE
            + payload_size
            + num_keyslots * sizeof(dmime_keyslot_t))
        > insize)
    {
        RET_ERROR_PTR(ERR_UNSPEC, "invalid input or chunk size");
    }

    // size of chunk object:
    chunk_size =
        serial_size
        + sizeof(dmime_message_chunk_state_t)
        + sizeof(size_t);

    if(!(result = malloc(chunk_size))) {
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate memory for chunk");
    }

    memset(result, 0, chunk_size);
    result->state = MESSAGE_CHUNK_STATE_CREATION;
    result->serial_size = serial_size;
    memcpy(&(result->type), in, serial_size);
    if(key->encrypted) {
        result->state = MESSAGE_CHUNK_STATE_ENCRYPTED;
    } else {
        result->state = MESSAGE_CHUNK_STATE_ENCODED;
    }
    *read = serial_size;

    return result;
}


/**
 * @brief
 *  wraps a binary payload in a chunk of specified type. only the validity of
 *  size is verified.
 * @note
 *  do not use this as a general constructor.
 * @param type
 *  specified chunk type.
 * @param payload
 *  array to binary payload to be wrapped in the message chunk.
 * @param insize
 *  length of input payload.
 * @return
 *  an allocated and encoded dmime message chunk.
 * @free_using{dmsg_destroy_message_chunk}
 */
static dmime_message_chunk_t *
dmsg_chunk_payload_wrap(
    dmime_chunk_type_t type,
    unsigned char *payload,
    size_t insize)
{
    dmime_chunk_key_t *key;
    dmime_message_chunk_t *result;
    int num_keyslots;
    size_t total_size, serial_size;

    if(!payload || !insize) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!((key = dmsg_chunk_type_key_get(type))->section)) {
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "could not retrieve key for specified chunk type");
    }

    if(key->payload == PAYLOAD_TYPE_EPHEMERAL && insize != EC_PUBKEY_SIZE) {
        RET_ERROR_PTR(ERR_UNSPEC, "invalid ephemeral payload size");
    }

    if(key->payload == PAYLOAD_TYPE_SIGNATURE && insize != ED25519_SIG_SIZE) {
        RET_ERROR_PTR(ERR_UNSPEC, "invalid signature payload size");
    }

    if(key->payload == PAYLOAD_TYPE_STANDARD && (insize % 16)) {
        RET_ERROR_PTR(ERR_UNSPEC, "invalid standard payload size");
    }

    num_keyslots =
        key->auth_keyslot
        + key->orig_keyslot
        + key->dest_keyslot
        + key->recp_keyslot;
    serial_size =
        CHUNK_HEADER_SIZE
        + insize
        + num_keyslots * sizeof(dmime_keyslot_t);
    total_size =
        serial_size
        + sizeof(dmime_message_chunk_state_t)
        + sizeof(size_t);

    if(!(result = malloc(total_size))) {
        RET_ERROR_PTR(
            ERR_NOMEM,
            "could not allocate memory for message chunk");
    }

    memset(result, 0, total_size);
    result->state = MESSAGE_CHUNK_STATE_CREATION;
    result->serial_size = serial_size;
    result->type = type;
    _int_no_put_3b(&(result->payload_size[0]), insize);
    memcpy(&(result->data[0]), payload, insize);

    if(key->payload == PAYLOAD_TYPE_STANDARD) {
        result->state = MESSAGE_CHUNK_STATE_SIGNED;
    } else {
        result->state = MESSAGE_CHUNK_STATE_ENCODED;
    }

    return result;
}

/**
 * @brief
 *  returns the location of the data segment of the specified chunk and stores
 *  its size.
 * @note
 *  for ephemeral and signature payload chunks it is the entire payload, for
 *  standard payload chunks it is the data section.
 * @param chunk
 *  pointer to a chunk from which the data is copied.
 * @param outsize
 *  stores the length of chunk data.
 * @return
 *  pointer to the chunk data. do not free!
 */
static unsigned char *
dmsg_chunk_data_get(
    dmime_message_chunk_t *chunk,
    size_t *outsize)
{
    dmime_chunk_key_t *key;
    size_t size;
    unsigned char *result, pad_byte, pad_len;
    void *payload;

    if(!chunk || !outsize) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!((key = dmsg_chunk_type_key_get(chunk->type))->section)) {
        RET_ERROR_PTR(ERR_UNSPEC, "invalid chunk type");
    }

    if(!(payload = dmsg_chunk_payload_get(chunk))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could retrieve chunk payload");
    }

    switch(key->payload) {

    case PAYLOAD_TYPE_EPHEMERAL:
        result = &(chunk->data[0]);

        if ((size = _int_no_get_3b(&(chunk->payload_size[0])))
            != EC_PUBKEY_SIZE)
        {
            RET_ERROR_PTR(
                ERR_UNSPEC,
                "the ephemeral chunk contains data of invalid size");
        }

        break;
    case PAYLOAD_TYPE_SIGNATURE:
        result = &(chunk->data[0]);

        if ((size = _int_no_get_3b(&(chunk->payload_size[0])))
            != ED25519_SIG_SIZE)
        {
            RET_ERROR_PTR(
                ERR_UNSPEC,
                "the signature chunk contains data of invalid size");
        }

        size = _int_no_get_3b(&(chunk->payload_size[0]));
        break;
    case PAYLOAD_TYPE_STANDARD:

        if(chunk->state == MESSAGE_CHUNK_STATE_ENCRYPTED) {
            result = (unsigned char *)payload;
            size = _int_no_get_3b(&(chunk->payload_size[0]));
        } else {
            result = &(((dmime_standard_payload_t *)payload)->data[0]);
            size =_int_no_get_3b(
                &(((dmime_standard_payload_t *)payload)->data_size[0]));
        }

        break;
    default:
        RET_ERROR_PTR(ERR_UNSPEC, "invalid chunk type");
        break;

    }

    if (key->payload == PAYLOAD_TYPE_STANDARD
        && chunk->state != MESSAGE_CHUNK_STATE_ENCRYPTED)
    {
        pad_byte = ((dmime_standard_payload_t *)payload)->pad_len;
        pad_len = _int_no_get_3b(&(chunk->payload_size)) - (size + 69);

        if (((dmime_standard_payload_t *)payload)->flags
            & ALTERNATE_PADDING_ALGORITHM_ENABLED)
        {
            if(pad_byte != (pad_len - (pad_len % 16))) {
                RET_ERROR_PTR(
                    ERR_UNSPEC,
                    "invalid padding length using alternative "
                    "padding algorithm");
            }
        } else {
            if(pad_byte != pad_len) {
                RET_ERROR_PTR(
                    ERR_UNSPEC,
                    "invalid padding length using primary padding algorithm");
            }

        }

        for(unsigned int i = 0; i < pad_len; ++i) {
            if (((dmime_standard_payload_t *)payload)->data[size + i]
                != pad_byte)
            {
                RET_ERROR_PTR(ERR_UNSPEC, "payload padded with invalid byte values");
            }
        }

    }

    *outsize = size;

    return result;
}


/**
 * @brief
 *  returns the pointer to the data of the chunk.
 * @note
 *  the size will include the padding.
 * @param chunk
 *  pointer to the dmime message chunk the data of which will be retrieved.
 * @param outsize
 *  the size of the returned buffer.
 * @return
 *  pointer to the chunk data (does not allocate new memory). do not free!
 */
static unsigned char *
dmsg_chunk_data_padded_get(
    dmime_message_chunk_t *chunk,
    size_t *outsize)
{
    dmime_chunk_key_t *key;
    size_t size;
    unsigned char *result;
    void *payload;

    if(!chunk || !outsize) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!((key = dmsg_chunk_type_key_get(chunk->type))->section)) {
        RET_ERROR_PTR(ERR_UNSPEC, "invalid chunk type");
    }

    if(!(payload = dmsg_chunk_payload_get(chunk))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not retrieve chunk payload");
    }

    switch(key->payload) {

    case PAYLOAD_TYPE_EPHEMERAL:
    case PAYLOAD_TYPE_SIGNATURE:
        result = &(chunk->data[0]);
        size = _int_no_get_3b(&(chunk->payload_size[0]));
        break;
    case PAYLOAD_TYPE_STANDARD:

        if(chunk->state == MESSAGE_CHUNK_STATE_ENCRYPTED) {
            result = (unsigned char *)payload;
            size = _int_no_get_3b(&(chunk->payload_size[0]));
        } else {
            result = &(((dmime_standard_payload_t *)payload)->data_size[0]);
            size =
                _int_no_get_3b(&(chunk->payload_size[0]))
                - ED25519_SIG_SIZE;
        }

        break;
    default:
        RET_ERROR_PTR(ERR_UNSPEC, "invalid chunk type");
        break;
    }

    *outsize = size;

    return result;
}

/**
 * @brief
 *  returns the pointer to the plaintext signature of a standard payload chunk.
 * @note
 *  signatures are ED25519_SIG_SIZE
 * @param chunk
 *  pointer to the dmime message chunk with standard payload type from which
 *  the signature will be retrieved.
 * @return
 *  pointer to the plaintext signature. do not free!
 */
static unsigned char *
dmsg_chunk_sig_plaintext_get(dmime_message_chunk_t *chunk)
{
    dmime_chunk_key_t *key;
    unsigned char *result;

    if(!chunk) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!((key = dmsg_chunk_type_key_get(chunk->type))->section)) {
        RET_ERROR_PTR(ERR_UNSPEC, "invalid chunk type");
    }

    if(key->payload != PAYLOAD_TYPE_STANDARD) {
        RET_ERROR_PTR(
            ERR_UNSPEC,
            "the chunk type does not have a plaintext signature");
    }

    if(chunk->state > MESSAGE_CHUNK_STATE_SIGNED) {
        RET_ERROR_PTR(ERR_UNSPEC, "the chunk is already encrypted");
    }

    if(!(result = dmsg_chunk_payload_get(chunk))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not retrieve chunk payload");
    }

    return result;
}

/**
 * @brief
 *  returns the flags character of a standard payload chunk.
 * @param chunk
 *  pointer to a dmime message chunk with standard payload type from which the
 *  signature will be retrieved.
 * @return
 *  the flags byte of the chunk or default flags on error.
 */
static unsigned char
dmsg_chunk_flags_get(dmime_message_chunk_t *chunk)
{
    dmime_chunk_key_t *key;
    dmime_standard_payload_t *payload;

    if(!chunk) {
        return DEFAULT_CHUNK_FLAGS;
    }

    if(!((key = dmsg_chunk_type_key_get(chunk->type))->section)) {
        return DEFAULT_CHUNK_FLAGS;
    }

    if(key->payload != PAYLOAD_TYPE_STANDARD) {
        return DEFAULT_CHUNK_FLAGS;
    }

    if(chunk->state > MESSAGE_CHUNK_STATE_SIGNED) {
        return DEFAULT_CHUNK_FLAGS;
    }

    if(!(payload = dmsg_chunk_payload_get(chunk))) {
        return DEFAULT_CHUNK_FLAGS;
    }

    return payload->flags;
}


/**
 * @brief
 *  returns a string from dmime_actor_t.
 * @param actor
 *  actor value.
 * @return
 *  string containing human readable actor.
*/
static const char *
dmsg_actor_to_string(dmime_actor_t actor)
{
    switch(actor) {

    case id_author:
        return "author";
    case id_origin:
        return "origin";
    case id_destination:
        return "destination";
    case id_recipient:
        return "recipient";
    default:
        return "invalid dmime actor";

    }

}

/**
 * @brief
 *  returns a string from dmime_object_state_t.
 * @param state
 *  object state value.
 * @return
 *  string containing human readable dmime object state.
*/
static const char *
dmsg_object_state_to_string(dmime_object_state_t state)
{
    switch(state) {

    case DMIME_OBJECT_STATE_NONE:
        return "none";
    case DMIME_OBJECT_STATE_CREATION:
        return "creation";
    case DMIME_OBJECT_STATE_LOADED_ENVELOPE:
        return "loaded envelope";
    case DMIME_OBJECT_STATE_LOADED_SIGNETS:
        return "loaded signets";
    case DMIME_OBJECT_STATE_INCOMPLETE_ENVELOPE:
        return "incomplete envelope";
    case DMIME_OBJECT_STATE_INCOMPLETE_METADATA:
        return "incomplete metadata";
    case DMIME_OBJECT_STATE_COMPLETE:
        return "complete";
    default:
        return "unknown";

    }

}

/* public functions */

/**
 * @brief
 *  returns a string from dmime_actor_t.
 * @param actor
 *  actor value.
 * @return
 *  string containing human readable actor.
*/
char const *
dime_dmsg_actor_to_string(dmime_actor_t actor)
{
    PUBLIC_FUNCTION_IMPLEMENT(dmsg_actor_to_string, actor);
}

/**
 * @brief
 *  signs the encrypted, author signed dmime message with the origin
 *  signatures. the origin signature chunks must already exist in order for the
 *  signing to occur.
 * @param msg
 *  dmime message that will be signed by the origin.
 * @param bounce_flags
 *  flags indicating bounce signatures that the origin will sign.
 * @param kek
 *  origin's key encryption key.
 * @param signkey
 *  origin's private signing key that will be used to sign the message. the
 *  public part of this key must be included in the origin signet either as the
 *  pok or one of the soks with the message signing flag.
 * @return
 *  0 on success, anything else indicates failure.
*/
int
dime_dmsg_chunks_sig_origin_sign(
    dmime_message_t *msg,
    unsigned char bounce_flags,
    dmime_kek_t *kek,
    ED25519_KEY *signkey)
{
    PUBLIC_FUNCTION_IMPLEMENT(
        dmsg_chunks_sig_origin_sign,
        msg,
        bounce_flags,
        kek,
        signkey);
}

/**
 * @brief
 *  calculates the key encryption key for a given private encryption key and
 *  dmime message, using the ephemeral key chunk in the message
 * @param msg
 *  pointer to the dmime message, which has the ephemeral key chunk to be used.
 * @param enckey
 *  private ec encryption key.
 * @param kek
 *  pointer to a dmime_kek_t - a key encryption key object that can be used to
 *  decrypt the keyslots.
 * @return
 *  returns 0 on success, all other values indicate failure.
 */
int
dime_dmsg_kek_in_derive(
    dmime_message_t const *msg,
    EC_KEY *enckey,
    dmime_kek_t *kek)
{
    PUBLIC_FUNCTION_IMPLEMENT(dmsg_kek_in_derive, msg, enckey, kek);
}

/**
 * @brief
 *  converts a binary message into a dmime message. the message is assumed to
 *  be encrypted.
 * @param in
 *  pointer to the binary message.
 * @param insize
 *  pointer to the binary size.
 * @return
 *  pointer to a dmime message structure.
 * @free_using{dime_dmsg_destroy_message}
*/
dmime_message_t *
dime_dmsg_message_binary_deserialize(
    unsigned char const *in,
    size_t insize)
{
    PUBLIC_FUNCTION_IMPLEMENT(dmsg_message_deserialize, in, insize);
}

/**
 * @brief
 *  converts the specified sections of a dmime message to a complete binary
 *  form. the message must be at least signed by author.
 * @param msg
 *  dmime message to be converted.
 * @param sections
 *  sections to be included.
 * @param tracing
 *  if set, include tracing, if clear don't include tracing.
 * @param outsize
 *  stores the output size of the binary.
 * @free_using{free}
*/
unsigned char *
dime_dmsg_message_binary_serialize(
    dmime_message_t const *msg,
    unsigned char sections,
    unsigned char tracing,
    size_t *outsize)
{
    PUBLIC_FUNCTION_IMPLEMENT(
        dmsg_message_serialize,
        msg,
        sections,
        tracing,
        outsize);
}

/**
 * @brief
 *  decrypts, verifies and extracts all the information available to the author
 *  from the message.
 * @param obj
 *  dmime object into which the information is extracted, it must already
 *  contain the ids and signets of all the actors available to the author.
 * @param msg
 *  dmime message to be decrypted.
 * @param kek
 *  author's key encryption key.
 * @return
 *  0 on success, all other output values indicate failure.
*/
int
dime_dmsg_message_decrypt_as_auth(
    dmime_object_t *obj,
    dmime_message_t const *msg,
    dmime_kek_t *kek)
{
    PUBLIC_FUNCTION_IMPLEMENT(
        dmsg_message_decrypt_as_auth,
        obj,
        msg,
        kek);
}

/**
 * @brief
 *  decrypts, verifies and extracts all the information available to the
 *  destination from the message.
 * @param obj
 *  dmime object into which the information is extracted, it must already
 *  contain the ids and signets of all the actors available to the destination.
 * @param msg
 *  dmime message to be decrypted.
 * @param kek
 *  destination's key encryption key.
 * @return
 *  0 on success, all other output values indicate failure.
*/
int
dime_dmsg_message_decrypt_as_dest(
    dmime_object_t *obj,
    dmime_message_t const *msg,
    dmime_kek_t *kek)
{
    PUBLIC_FUNCTION_IMPLEMENT(dmsg_message_decrypt_as_dest, obj, msg, kek);
}

/**
 * @brief
 *  decrypts, verifies and extracts all the information available to the origin
 *  from the message.
 * @param obj
 *  dmime object into which the information is extracted, it must already
 *  contain the ids and signets of all the actors available to the origin.
 * @param msg
 *  dmime message to be decrypted.
 * @param kek
 *  origin's key encryption key.
 * @return
 *  0 on success, all other output values indicate failure.
*/
int
dime_dmsg_message_decrypt_as_orig(
    dmime_object_t *obj,
    dmime_message_t const *msg,
    dmime_kek_t *kek)
{
    PUBLIC_FUNCTION_IMPLEMENT(dmsg_message_decrypt_as_orig, obj, msg, kek);
}

/**
 * @brief
 *  decrypts, verifies and extracts all the information available to the
 *  recipient from the message.
 * @param obj
 *  dmime object into which the information is extracted, it must already
 *  contain the ids and signets of all the actors available to the recipient.
 * @param msg
 *  dmime message to be decrypted.
 * @param kek
 *  recipient's key encryption key.
 * @return
 *  0 on success, all other output values indicate failure.
*/
int
dime_dmsg_message_decrypt_as_recp(
    dmime_object_t *obj,
    dmime_message_t const *msg,
    dmime_kek_t *kek)
{
    PUBLIC_FUNCTION_IMPLEMENT(dmsg_message_decrypt_as_recp, obj, msg, kek);
}

/**
 * @brief
 *  destroys dmime_message_t structure.
 * @param msg
 *  pointer to the dmime message to be destroyed.
*/
void
dime_dmsg_message_destroy(dmime_message_t *msg)
{
    PUBLIC_FUNCTION_IMPLEMENT_VOID(dmsg_message_destroy, msg);
}

/**
 * @brief
 *  converts a dmime object to a dmime message, fully encrypting and signing
 *  the message !!as an author!!
 * @param object
 *  dmime object which contains all the envelope, metadata, display and
 *  attachment information.  as well as pointers to signets of author, origin,
 *  destination and recipient.
 * @param signkey
 *  the author's private ed25519 signing key which will be used.
 * @return
 *  a pointer to a fully signed and encrypted dmime message.
 * @free_using{dime_dmsg_destroy_message}
*/
dmime_message_t *
dime_dmsg_message_encrypt(
    dmime_object_t *object,
    ED25519_KEY *signkey)
{
    PUBLIC_FUNCTION_IMPLEMENT(dmsg_message_encrypt, object, signkey);
}

/**
 * @brief
 *  retrieves author name for the following actors: author, origin, recipient.
 * @param msg
 *  dmime message the author of which is retrieved.
 * @param actor
 *  who is trying to get the message author.
 * @param kek
 *  key encryption key for the specified actor.
 * @return
 *  a newly allocated dmime object containing the envelope ids available to the
 *  actor.
 * @free_using{dime_dmsg_destroy_object}
 */
dmime_object_t *
dime_dmsg_message_envelope_decrypt(
    dmime_message_t const *msg,
    dmime_actor_t actor,
    dmime_kek_t *kek)
{
    PUBLIC_FUNCTION_IMPLEMENT(dmsg_message_envelope_decrypt, msg, actor, kek);
}

/**
 * @brief
 *  retrieves dmime message state.
 * @param message
 *  pointer to a dmime message.
 * @return
 *  dmime_message_state_t corresponding to the current state.
 */
dmime_message_state_t
dime_dmsg_message_state_get(
    dmime_message_t const *message)
{
    PUBLIC_FUNCTION_IMPLEMENT(dmsg_message_state_get, message);
}

/**
 * @brief
 *  creates a dmime object chunk with the specified type, data and flags.
 * @param type
 *  chunk type.
 * @param data
 *  pointer to an array that gets copied into newly allocated memory.
 * @param data_size
 *  length of data array.
 * @param flags
 *  specified flags for the object chunk.
 * @free_using{dime_dmsg_object_chunklist_destroy}
*/
dmime_object_chunk_t *
dime_dmsg_object_chunk_create(
    dmime_chunk_type_t type,
    unsigned char *data,
    size_t data_size,
    unsigned char flags)
{
    PUBLIC_FUNCTION_IMPLEMENT(
        dmsg_object_chunk_create,
        type,
        data,
        data_size,
        flags);
}

/**
 * @brief
 *  destroy dmime object chunk list.
 * @param list
 *  pointer to a dmime object chunk list to be destroyed.
 */
void
dime_dmsg_object_chunklist_destroy(dmime_object_chunk_t *list)
{
    PUBLIC_FUNCTION_IMPLEMENT(dmsg_object_chunklist_destroy, list);
}

/**
 * @brief
 *  destroy a dmime_object_t structure.
 * @param object
 *  pointer to dmime object to be destroyed.
 */
void
dime_dmsg_object_destroy(dmime_object_t *object)
{
    PUBLIC_FUNCTION_IMPLEMENT_VOID(dmsg_object_destroy, object);
}

/**
 * @brief
 *  dumps the contents of the dmime object.
 * @param object
 *  dmime object to be dumped.
 * @return
 *  0 on success, all other values indicate failure.
*/
int
dime_dmsg_object_dump(dmime_object_t *object)
{
    PUBLIC_FUNCTION_IMPLEMENT(dmsg_object_dump, object);
}

/**
 * @brief
 *  takes a dmime object and determines the state it is in.
 * @param object
 *  dmime object, state of which will be retrieved.
 * @return
 *  the state of dmime object.
 */
dmime_object_state_t
dime_dmsg_object_state_init(dmime_object_t *object)
{
    PUBLIC_FUNCTION_IMPLEMENT(dmsg_object_state_init, object);
}

/**
 * @brief
 *  returns a string from dmime_object_state_t.
 * @param state
 *  object state value.
 * @return
 *  string containing human readable dmime object state.
*/
const char *
dime_dmsg_object_state_to_string(dmime_object_state_t state)
{
    PUBLIC_FUNCTION_IMPLEMENT(dmsg_object_state_to_string, state);
}

// TODO - not implemented yet
//int
//dime_dmsg_file_create(
//    dmime_message_t const *msg,
//    char const *filename);
//
//dmime_message_t *
//dime_dmsg_file_to_message(
//    const char *filename);


//{
//  .required,
//  .unique,
//  .encrypted,
//  .sequential,
//  .section,
//  .payload,
//  .auth_keyslot,
//  .orig_keyslot,
//  .dest_keyslot,
//  .recp_keyslot,
//  .name,
//  .description
//}
#define CKEY_EMPTY { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, NULL }
// TODO - add display_multi, display_alt, attach_multi, attach_alt chunks
dmime_chunk_key_t
dmime_chunk_keys[DMIME_CHUNK_TYPE_MAX] = {
    CKEY_EMPTY, CKEY_EMPTY,
    // field 2
    {
        1, 0, 0, 1,
        CHUNK_SECTION_ENVELOPE, PAYLOAD_TYPE_EPHEMERAL,
        0, 0, 0, 0,
        "ephemeral", NULL
    },
    // field 3
    {
        0, 0, 1, 1,
        CHUNK_SECTION_ENVELOPE, PAYLOAD_TYPE_STANDARD,
        1, 0, 0, 1,
        "alternate",   NULL
    },
    // field 4
    {
        1, 0, 1, 1,
        CHUNK_SECTION_ENVELOPE, PAYLOAD_TYPE_STANDARD,
        1, 1, 0, 1,
        "origin", NULL
    },
    // field 5
    {
        1, 0, 1, 1,
        CHUNK_SECTION_ENVELOPE, PAYLOAD_TYPE_STANDARD,
        1, 0, 1, 1,
        "destination", NULL
    },
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    // field 33
    {
        1, 0, 1, 1,
        CHUNK_SECTION_METADATA, PAYLOAD_TYPE_STANDARD,
        1, 0, 0, 1,
        "common",  NULL
    },
    // field 34
    {
        0, 0, 1, 1,
        CHUNK_SECTION_METADATA, PAYLOAD_TYPE_STANDARD,
        1, 0, 0, 1,
        "headers", NULL
    },
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY,
    // field 67
    {
        0, 1, 1, 0,
        CHUNK_SECTION_DISPLAY, PAYLOAD_TYPE_STANDARD,
        1, 0, 0, 1,
        "display-content", NULL
    },
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    // field 131
    {
        0, 1, 1, 0,
        CHUNK_SECTION_ATTACH, PAYLOAD_TYPE_STANDARD,
        1, 0, 0, 1,
        "attachments-content", NULL
    },
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    // field 225
    {
        1, 0, 1, 1,
        CHUNK_SECTION_SIG, PAYLOAD_TYPE_SIGNATURE,
        1, 1, 0, 1,
        "author-tree-signature", NULL
    },
    // field 226
    {
        1, 0, 1, 1,
        CHUNK_SECTION_SIG, PAYLOAD_TYPE_SIGNATURE,
        1, 1, 0, 1,
        "author-signature", NULL
    },
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    // field 248
    {
        0, 0, 1, 1,
        CHUNK_SECTION_SIG, PAYLOAD_TYPE_SIGNATURE,
        1, 1, 1, 1,
        "organizational-metadata-bounce-signature", NULL
    },
    // field 249
    {
        0, 0, 1, 1,
        CHUNK_SECTION_SIG, PAYLOAD_TYPE_SIGNATURE,
        1, 1, 1, 1,
        "organizational-display-bounce-signature",  NULL
    },
    CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY, CKEY_EMPTY,
    // field 255
    {
        1, 0, 1, 1,
        CHUNK_SECTION_SIG, PAYLOAD_TYPE_SIGNATURE,
        1, 1, 1, 1,
        "Organizational-Signature", NULL
    }
};
