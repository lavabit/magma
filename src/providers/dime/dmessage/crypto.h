#ifndef DIME_DMSG_CRYPTO_H
#define DIME_DMSG_CRYPTO_H

#include "dime/signet/signet.h"
#include "dime/dmessage/common.h"

#define TRACING_LENGTH_SIZE 2

// Actor type, used to encrypt and retrieve the correct keyslot, kek, maybe
// more
typedef enum {
    id_author = 0,
    id_origin = 1,
    id_destination = 2,
    id_recipient = 3
} dmime_actor_t;

// State of dmime_content_t object, used to identify if it is ready to be
// turned into a dmime_message_t object, maybe more.
typedef enum {
    DMIME_OBJECT_STATE_NONE = 0,
    DMIME_OBJECT_STATE_CREATION,
    DMIME_OBJECT_STATE_LOADED_ENVELOPE,
    DMIME_OBJECT_STATE_LOADED_SIGNETS,
    DMIME_OBJECT_STATE_INCOMPLETE_ENVELOPE,
    DMIME_OBJECT_STATE_INCOMPLETE_METADATA,
    DMIME_OBJECT_STATE_COMPLETE
} dmime_object_state_t;

// State of dmime_message_t, used to identify if the encrypted message contains
// the user and domain signatures.  This is used by origin to determine whether
// the message is ready to be signed and by destination and recipient whether
// the message contains required chunks to be valid.
typedef enum {
    MESSAGE_STATE_NONE = 0,
    MESSAGE_STATE_INCOMPLETE,
    MESSAGE_STATE_EMPTY,
    MESSAGE_STATE_ENCODED,
    MESSAGE_STATE_CHUNKS_SIGNED,
    MESSAGE_STATE_ENCRYPTED,
    MESSAGE_STATE_AUTHOR_SIGNED,
    MESSAGE_STATE_COMPLETE
} dmime_message_state_t;

// structure to easily store the KEKs and IVs
typedef struct __attribute__((packed)) {
    unsigned char iv[16];
    unsigned char key[AES_256_KEY_SIZE];
} dmime_kek_t;

typedef struct object_chunk {
    struct object_chunk *next;
    dmime_chunk_type_t type;
    unsigned char flags;
    size_t data_size;
    unsigned char *data;
} dmime_object_chunk_t;

typedef struct {
    // The current actor on the object.
    dmime_actor_t actor;
    // The author and recipient's dmail addresses.
    sds author;
    sds recipient;
    // The origin and destination domains.
    sds origin;
    sds destination;
    // Cryptographic signet fingerprint.
    sds fp_author;
    sds fp_recipient;
    sds fp_origin;
    sds fp_destination;
    // Signets for the author and recipient and their orgs.
    signet_t *signet_author;
    signet_t *signet_recipient;
    signet_t *signet_origin;
    signet_t *signet_destination;
    // Common headers.
    dmime_common_headers_t *common_headers;
    // Other headers
    sds other_headers;
    // display and attachment chunks
    dmime_object_chunk_t *display;
    dmime_object_chunk_t *attach;
    // object state
    dmime_object_state_t state;
} dmime_object_t;

//tracing structure
typedef struct __attribute__((packed)) {
    unsigned char size[TRACING_LENGTH_SIZE];
    unsigned char data[];
} dmime_tracing_t;

// message chunk state used by message chunks to keep track of encrypted and
// unencrypted chunks.  Must be used in conjunction with the 'encrypted' flag
// in the global table of chunk types to determine if encryption is necessary.
typedef enum {
    MESSAGE_CHUNK_STATE_NONE = 0,
    MESSAGE_CHUNK_STATE_UNKNOWN,
    MESSAGE_CHUNK_STATE_CREATION,
    MESSAGE_CHUNK_STATE_ENCODED,
    MESSAGE_CHUNK_STATE_SIGNED,
    MESSAGE_CHUNK_STATE_ENCRYPTED
} dmime_message_chunk_state_t;

// Chunk of a DIME message.
typedef struct __attribute__((packed)) {
    dmime_message_chunk_state_t state;
    // this size is used to serialize the chunk which follows
    size_t serial_size;
    unsigned char type;
    unsigned char payload_size[CHUNK_LENGTH_SIZE];
    unsigned char data[];
} dmime_message_chunk_t;

typedef struct {
    // DIME magic number for current version of dmime messages
    dime_number_t dime_num;
    // message size
    uint32_t size;
    // tracing
    dmime_tracing_t *tracing;
    // ephemeral chunk
    dmime_message_chunk_t *ephemeral;
    // origin chunk
    dmime_message_chunk_t *origin;
    // destination chunk
    dmime_message_chunk_t *destination;
    // common headers chunk
    dmime_message_chunk_t *common_headers;
    // other headers chunk
    dmime_message_chunk_t *other_headers;
    // pointer to an array of display chunks terminated by a NULL pointer
    dmime_message_chunk_t **display;
    // pointer to an array of attachment  chunks terminated by a NULL pointer
    dmime_message_chunk_t **attach;
    // author tree sig chunk
    dmime_message_chunk_t *author_tree_sig;
    // author full sig chunk
    dmime_message_chunk_t *author_full_sig;
    // origin meta bounce sig chunk
    dmime_message_chunk_t *origin_meta_bounce_sig;
    // origin display bounce sig chunk
    dmime_message_chunk_t *origin_display_bounce_sig;
    // origin full sig chunk
    dmime_message_chunk_t *origin_full_sig;
    //state
    dmime_message_state_t state;
} dmime_message_t;

char const *
dime_dmsg_actor_to_string(
    dmime_actor_t actor);

int
dime_dmsg_chunks_sig_origin_sign(
    dmime_message_t *msg,
    unsigned char bounce_flags,
    dmime_kek_t *kek,
    ED25519_KEY *signkey);

int
dime_dmsg_kek_in_derive(
    dmime_message_t const *msg,
    EC_KEY *enckey,
    dmime_kek_t *kek);

dmime_message_t *
dime_dmsg_message_binary_deserialize(
    unsigned char const *in,
    size_t insize);

unsigned char *
dime_dmsg_message_binary_serialize(
    dmime_message_t const *msg,
    unsigned char sections,
    unsigned char tracing,
    size_t *outsize);

int
dime_dmsg_message_decrypt_as_auth(
    dmime_object_t *obj,
    dmime_message_t const *msg,
    dmime_kek_t *kek);

int
dime_dmsg_message_decrypt_as_dest(
    dmime_object_t *obj,
    dmime_message_t const *msg,
    dmime_kek_t *kek);

int dime_dmsg_message_decrypt_as_orig(
    dmime_object_t *obj,
    dmime_message_t const *msg,
    dmime_kek_t *kek);

int dime_dmsg_message_decrypt_as_recp(
    dmime_object_t *obj,
    dmime_message_t const *msg,
    dmime_kek_t *kek);

void
dime_dmsg_message_destroy(
    dmime_message_t *msg);

dmime_message_t *
dime_dmsg_message_encrypt(
    dmime_object_t *object,
    ED25519_KEY *signkey);

dmime_object_t *
dime_dmsg_message_envelope_decrypt(
    dmime_message_t const *msg,
    dmime_actor_t actor,
    dmime_kek_t *kek);

dmime_message_state_t
dime_dmsg_message_state_get(
    dmime_message_t const *message);

dmime_object_chunk_t *
dime_dmsg_object_chunk_create(
    dmime_chunk_type_t type,
    unsigned char *data,
    size_t data_size,
    unsigned char flags);

void
dime_dmsg_object_chunklist_destroy(
    dmime_object_chunk_t *list);

void
dime_dmsg_object_destroy(
    dmime_object_t *object);

int
dime_dmsg_object_dump(
    dmime_object_t *object);

dmime_object_state_t
dime_dmsg_object_state_init(
    dmime_object_t *object);

const char *
dime_dmsg_object_state_to_string(
    dmime_object_state_t state);

// TODO not implemented yet
//int
//dime_dmsg_file_create(
//    const dmime_message_t *msg,
//    const char *filename);
//
//dmime_message_t *
//dime_dmsg_file_to_message(
//    const char *filename);

// TODO public interface for dmime_object_t !!  TODO Review of message and
// object states (I think at least one of them doesn't need to be a structure
// member.)

#endif
