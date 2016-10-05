#include "string.h"
#include "dime/common/misc.h"
#include "dime/signet/keys.h"
#include "dime/signet/signet.h"

/** A signet field index structure for temporary convenience organization of field data */
typedef struct signet_field_t {

    const signet_t *signet;
    signet_field_key_t *key;
    unsigned char name_size;
    unsigned int data_size;

    unsigned int id_offset;
    unsigned int name_offset;
    unsigned int data_offset;

    struct signet_field_t *next;
} signet_field_t;

/* PRIVATE FUNCTIONS */

static EC_KEY *                sgnt_enckey_fetch(const signet_t *signet);
static int                     sgnt_enckey_set(signet_t *signet, EC_KEY *key, unsigned char format);
static int                     sgnt_fid_count_get(const signet_t *signet, unsigned char fid);
static int                     sgnt_fid_dump(FILE *fp, const signet_t *signet, unsigned int fid);
static int                     sgnt_fid_exists(const signet_t *signet, unsigned char fid);
static unsigned char *         sgnt_fid_num_fetch(const signet_t *signet, unsigned char fid, unsigned int num, size_t *data_size);
static int                     sgnt_fid_num_remove(signet_t *signet, unsigned char fid, int num);
static int                     sgnt_field_create_at(signet_t *signet, unsigned int offset, size_t field_size, const unsigned char *field_data);
static int                     sgnt_field_defined_create(signet_t *signet, unsigned char fid, size_t data_size, const unsigned char *data);
static int                     sgnt_field_defined_set(signet_t *signet, unsigned char fid, size_t data_size, const unsigned char *data);
static int                     sgnt_field_dump(FILE *fp, const signet_field_t *field);
static int                     sgnt_field_remove_at(signet_t *signet, unsigned int offset, size_t field_size);
static int                     sgnt_field_sign(signet_t *signet, unsigned char signet_fid, ED25519_KEY *key);
static int                     sgnt_field_size_serial_get(const signet_field_t *field);
static int                     sgnt_field_undefined_create(signet_t *signet, size_t name_size, const unsigned char *name, size_t data_size, const unsigned char *data);
static unsigned char *         sgnt_field_undefined_fetch(const signet_t *signet, size_t name_size, const unsigned char *name, size_t *data_size);
static int                     sgnt_field_undefined_remove(signet_t *signet, size_t name_size, const unsigned char *name);
static signet_field_t *        sgnt_fieldlist_create(const signet_t *signet, unsigned char fid);
static void                    sgnt_fieldlist_destroy(signet_field_t *field);
static signet_field_t *        sgnt_fieldnode_create(const signet_t *signet, uint32_t offset, signet_field_key_t *key);
static signet_field_t *        sgnt_fieldnode_destroy(signet_field_t *field);
static int                     sgnt_file_create(signet_t *signet, const char *filename);
static char *                  sgnt_fingerprint_crypto(const signet_t *signet);
static char *                  sgnt_fingerprint_full(const signet_t *signet);
static char *                  sgnt_fingerprint_id(const signet_t *signet);
static char *                  sgnt_fingerprint_ssr(const signet_t *signet);
static char *                  sgnt_fingerprint_upto_fid(const signet_t *signet, unsigned char fid);
static char *                  sgnt_id_fetch(signet_t *signet);
static int                     sgnt_id_set(signet_t *signet, size_t id_size, const unsigned char *id);
static int                     sgnt_length_serial_check(const unsigned char *in, uint32_t slen);
static int                     sgnt_msg_sig_verify(const signet_t *signet, ed25519_signature sig, const unsigned char *buf, size_t buf_len);  /* TODO verify function for each type of signatures (message, signet, TLS certificate, software)*/
static int                     sgnt_sig_coc_sign(signet_t *signet, ED25519_KEY *key);
static int                     sgnt_sig_crypto_sign(signet_t *signet, ED25519_KEY *key);
static int                     sgnt_sig_full_sign(signet_t *signet, ED25519_KEY *key);
static int                     sgnt_sig_id_sign(signet_t *signet, ED25519_KEY *key);
static int                     sgnt_sig_ssr_sign(signet_t *signet, ED25519_KEY *key);
static signet_t *              sgnt_signet_binary_deserialize(const unsigned char *in, size_t len);
static unsigned char *         sgnt_signet_binary_serialize(signet_t *signet, uint32_t *serial_size);
static signet_t *              sgnt_signet_b64_deserialize(const char *b64_in);
static char *                  sgnt_signet_b64_serialize(signet_t *signet);
static char *                  sgnt_signet_crc24_serialize(signet_t *signet);
static signet_t *              sgnt_signet_create(signet_type_t type);
static signet_t *              sgnt_signet_create_w_keys(signet_type_t type, const char *keysfile);
static signet_t *              sgnt_signet_crypto_split(const signet_t *signet);
static void                    sgnt_signet_destroy(signet_t *signet);
static void                    sgnt_signet_dump(FILE *fp, signet_t *signet);
static signet_t *              sgnt_signet_dupe(signet_t *signet);
static signet_t *              sgnt_signet_full_split(const signet_t *signet);
static int                     sgnt_signet_index(signet_t *signet);
static signet_t *              sgnt_signet_load(const char *filename);
static unsigned char *         sgnt_signet_serialize_upto_fid(const signet_t *signet, unsigned char fid, size_t *data_size);
static int                     sgnt_signet_size_serial_get(const signet_t *signet);
static signet_t *              sgnt_signet_split(const signet_t *signet, unsigned char fid);
static ED25519_KEY *           sgnt_signkey_fetch(const signet_t *signet);
static ED25519_KEY **          sgnt_signkey_fetch_by_perm(const signet_t *signet, sok_permissions_t perm);
static unsigned char *         sgnt_signkey_format(ED25519_KEY *key, unsigned char format, size_t *key_size);
static ED25519_KEY *           sgnt_signkey_parse(const unsigned char *serial_key, size_t key_size);
static int                     sgnt_signkey_set(signet_t *signet, ED25519_KEY *key, unsigned char format);
static ED25519_KEY **          sgnt_signkeys_msg_fetch(const signet_t *signet);
static ED25519_KEY **          sgnt_signkeys_signet_fetch(const signet_t *signet);
static ED25519_KEY **          sgnt_signkeys_software_fetch(const signet_t *signet);
static ED25519_KEY **          sgnt_signkeys_tls_fetch(const signet_t *signet);
static int                     sgnt_sok_create(signet_t *signet, ED25519_KEY *key, unsigned char format, uint8_t perm);
static ED25519_KEY *           sgnt_sok_num_fetch(const signet_t *signet, unsigned int num);
static const char *            sgnt_state_to_str(signet_state_t state);
static signet_type_t           sgnt_type_get(const signet_t *signet);
static int                     sgnt_type_set(signet_t *signet, signet_type_t type);
static signet_state_t          sgnt_validate_all(const signet_t *signet, const signet_t *previous, const signet_t *orgsig, const unsigned char **dime_pok);
static int                     sgnt_validate_pok(const signet_t *signet, const unsigned char **dime_pok);
static int                     sgnt_validate_required_upto_fid(const signet_t *signet, signet_field_key_t *keys, unsigned char fid);
static int                     sgnt_validate_sig_field(const signet_t *signet, unsigned char sigfid, const unsigned char *key);
static int                     sgnt_validate_sig_field_key(const signet_t *signet, unsigned char sigfid, ED25519_KEY *key);
static int                     sgnt_validate_sig_field_multikey(const signet_t *signet, unsigned char sig_fid, ED25519_KEY **keys);
static signet_state_t          sgnt_validate_structure(const signet_t *signet);

#if 0 /* currently unused */
static int                     sgnt_fid_get_size(const signet_t *signet, unsigned char fid);
#endif
#if 0 /* currently unused */
static unsigned char *sgnt_serial_from_fid(const signet_t *signet, unsigned char fid, size_t *fid_size);
#endif


/**
 * @brief   Returns a new signet_t structure.
 * @param   type    signet type user org or sss (SIGNET_TYPE_USER, SIGNET_TYPE_ORG or SIGNET_TYPE_SSR)
 * @return  A pointer to a newly allocated signet_t structure type, NULL if failure.
 * @free_using{sgnt_destroy_signet}
*/
static signet_t *sgnt_signet_create(signet_type_t type) {

    signet_t *signet;

    if(type != SIGNET_TYPE_ORG && type != SIGNET_TYPE_USER && type != SIGNET_TYPE_SSR) {
        RET_ERROR_PTR(ERR_BAD_PARAM, "invalid signet type");
        //PUSH_ERROR(ERR_BAD_PARAM, "invalid signet type");
        //goto error;
    }

    if(!(signet = malloc(sizeof(signet_t)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for new signet");
    }

    memset(signet, 0, sizeof(signet_t));
    signet->type = type;

    return signet;
}


/**
 * @brief   Takes a binary signet input buffer and its length. Checks for signet size inconsistencies.
 * @param   in  binary signet buffer
 * @param   slen    length of in
 * @return  0 if the checks passed, -1 if at least one failed.
*/
static int sgnt_length_serial_check(const unsigned char *in, uint32_t slen) {

    uint32_t signet_length;

    if (!in || (slen < SIGNET_HEADER_SIZE)) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    signet_length = _int_no_get_3b(in + 2);

    if ((slen - SIGNET_HEADER_SIZE) != signet_length) {
        RET_ERROR_INT(ERR_UNSPEC, "input length did not match signet");
    }

    return 0;
}


/**
 * @brief   Parses the fields of a signet object and assigns the offsets to the byte following the field id byte of the first instance of a field type to signet-fields[]
 * @param   signet  A pointer to a signet_t object to be parsed.
 * @return  0 if parsing finished successfully, -1 if it failed.
*/
static int sgnt_signet_index(signet_t *signet) {

    uint32_t field_size, name_size;
    unsigned int at = 0;
    signet_field_key_t key, *keys;

    if (!signet) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        keys = signet_org_field_keys;
        break;
    case SIGNET_TYPE_USER:
        keys = signet_user_field_keys;
        break;
    case SIGNET_TYPE_SSR:
        keys = signet_ssr_field_keys;
        break;
    default:
        RET_ERROR_INT(ERR_UNSPEC, "incorrect signet type");
        break;

    }

    for(int i = 0; i < SIGNET_FID_MAX + 1; ++i) {

        if(at == signet->size) {
            break;
        }

        if(at > signet->size) {
            RET_ERROR_INT(ERR_UNSPEC, "invalid signet size");
        }

        if(!keys[i].name) {

            if(i == signet->data[at]) {
                RET_ERROR_INT(ERR_UNSPEC, "a field in this signet file is disallowed by the current version");
            }

            signet->fields[i] = 0;
            continue;
        }

        if(keys[i].name) {
            key = keys[i];

            if(i > signet->data[at]) {
                RET_ERROR_INT(ERR_UNSPEC, "signet fields are not in numerical order or a unique field appears more than once");
            }

            if(i < signet->data[at]) {
                signet->fields[i] = 0;
                continue;
            }

            if(i == signet->data[at]) {

                if(at + 1 >= signet->size) {
                    RET_ERROR_INT(ERR_UNSPEC, "signet size error");
                }

                signet->fields[i] = (at + 1);

                while(at < signet->size && i == signet->data[at]) {
                    ++at;
                    field_size = 0;

                    if(key.bytes_name_size) {

                        if(at + 1 >= signet->size) {
                            RET_ERROR_INT(ERR_UNSPEC, "signet size error");
                        }

                        name_size = (unsigned char)signet->data[at++];

                        if(at + name_size >= signet->size) {
                            RET_ERROR_INT(ERR_UNSPEC, "signet size error");
                        }

                        at += name_size;
                    }

                    if (key.bytes_data_size && (at + key.bytes_data_size >= signet->size)) {
                        RET_ERROR_INT(ERR_UNSPEC, "signet size error");
                    }

                    if(key.bytes_data_size == 0) {
                        field_size = key.data_size;
                    } else if(key.bytes_data_size == 1) {
                        field_size = (unsigned char)signet->data[at];
                    } else if(key.bytes_data_size == 2) {
                        field_size = _int_no_get_2b(signet->data + at);
                    } else if(key.bytes_data_size == 3) {
                        field_size = _int_no_get_3b(signet->data + at);
                    }

                    at += key.bytes_data_size;

                    if(at + field_size > signet->size) {
                        RET_ERROR_INT(ERR_UNSPEC, "signet size error");
                    }

                    at += field_size;

                    if(key.unique) {
                        break;
                    }

                }

            }

        }

    }

    return 0;
}


#if 0 /* currently unused */
/**
 * @brief   Retrieves the length of all fields in the signet with the specified field id in serial form.
 * @param   signet  Pointer to the target signet.
 * @param   fid The target field id.
 * @return  The length of the serialized fields, returns -1 on errors and on non-existing fields.
 *              NOTE: int overflow should not occur because field size and signet size are bounded well below 2^31 bits.
*/
static int sgnt_fid_get_size(const signet_t *signet, unsigned char fid) {

    int res;
    size_t start, end;

    if(!signet) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if((res = sgnt_fid_exists(signet, fid)) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "error searching for field in signet");
    } else if(!res) {
        RET_ERROR_INT(ERR_UNSPEC, "specified field does not exist");
    }

    start = signet->fields[fid] - 1;
    end = signet->size;

    for(int i = fid + 1; i <= SIGNET_FID_MAX; ++i) {

        if((res = sgnt_fid_exists(signet, i))) {

            if(res < 0) {
                RET_ERROR_INT(ERR_UNSPEC, "error searching for field in signet");
            }

            end = signet->fields[i] - 1;
            break;
        }
    }

    if(end < start) {
        RET_ERROR_INT(ERR_UNSPEC, "signet is corrupted");
    }

    return end - start;
}
#endif

/**
 * @brief   Calculates the size of the target signet when serialized.
 * @param   signet  Pointer to the target signet.
 * @return  The size of signet when serialized, if no signet was passed returns -1.
 *              NOTE: int overflow should not occur signet size s bounded well below 2^31 bits.
*/
static int sgnt_signet_size_serial_get(const signet_t *signet) {

    if(!signet) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    return signet->size + SIGNET_HEADER_SIZE;
}


/**
 * @brief   Compares the POK in the signet to an array of POKs from the dime record
 * @param   signet      Pointer to the target signet.
 * @param   dime_pok    A NULL pointer terminated array of pointers to POKs from the dime record.
 * @return  The index + 1 number of the POK from dime_pok that matches the signet POK. If an error occurs returns -1. If no POKs match returns 0.
 *              NOTE: Could int overflow if more than 2^31 elements are passed in the dime_pok array.
*/
static int sgnt_validate_pok(const signet_t *signet, const unsigned char **dime_pok) {

    ED25519_KEY *signet_pok;
    int i = 0, res;

    if(!signet || !dime_pok) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if(sgnt_type_get(signet) != SIGNET_TYPE_ORG) {
        RET_ERROR_INT(ERR_BAD_PARAM, "signet must be org signet");
    }

    if((res = sgnt_fid_exists(signet, SIGNET_ORG_POK)) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "error searching for field in signet");
    } else if(!res) {
        RET_ERROR_INT(ERR_UNSPEC, "signet was missing Primary-Org-Key field");
    }

    if(!(signet_pok = sgnt_signkey_fetch(signet))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not retrieve signet signing key");
    }

    while(dime_pok[i]) {

        if(i == 0x7FFFFFFF) {
            _free_ed25519_key(signet_pok);
            RET_ERROR_INT(ERR_UNSPEC, "input overflow");
        }

        if(!(memcmp(dime_pok[i], (unsigned char *)signet_pok->public_key, ED25519_KEY_SIZE))) {
            _free_ed25519_key(signet_pok);
            return i + 1;
        }

        ++i;
    }

    _free_ed25519_key(signet_pok);

    return 0;
}


/**
 * @brief   Checks for the existence of all required fields with field ids less than the specified field id.
 * @param   signet  Pointer to the target signet.
 * @param   keys    Array of structure that describe field types
 * @param   fid The field upto and including which the signet checks whether all the required fields are present.
 * @return  1 if all required fields were present, 0 if at least one is missing. -1 if an error occurred.
*/
static int sgnt_validate_required_upto_fid(const signet_t *signet, signet_field_key_t *keys, unsigned char fid) {

    int res;

    if(!signet || !keys) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    for(int i = fid - 1; i >= 0; --i) {

        if(keys[i].required && (((res = sgnt_fid_exists(signet, i))) <= 0)) {

            if(res < 0) {
                RET_ERROR_INT(ERR_UNSPEC, "error searching for field in signet");
            }

            return 0;
        }
    }

    return 1;
}


/**
 * @brief
 *  Verifies a field specified by a field id in the target signet by using
 *  multiple keys.
 * @param   signet  Pointer to the target signet.
 * @param   sig_fid Field id of the field which contains the signature intended for verification.
 * @param   keys    A NULL pointer terminated array of ed25519 public key objects which are used to verify the signet signature.
 * @return  1 if one of the keys was able to verify the signature. 0 if none were able to verify. -1 if error.
*/
static int sgnt_validate_sig_field_multikey(const signet_t *signet, unsigned char sig_fid, ED25519_KEY **keys) {

    int i = 0, res = 0;

    if(!signet || !keys) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    while(keys[i]) {

        if((res = sgnt_validate_sig_field_key(signet, sig_fid, keys[i])) < 0) {
            RET_ERROR_INT(ERR_UNSPEC, "error during signet signature field verification");
        } else if(res) {
            return 1;
        }

        ++i;
    }

    return 0;
}


/**
 * @brief   Perform SHA512 fingerprint on all the fields up to the specified field.
 * @param   signet  Pointer to the target signet.
 * @param   fid Field id upto and including which the signet should be fingerprinted.
 * @return  NULL terminated string to the SHA512 base64 encoded unpadded fingerprint. NULL on error.
*/
static char *sgnt_fingerprint_upto_fid(const signet_t *signet, unsigned char fid) {

    char *b64_fingerprint;
    unsigned char *data, hash[SHA_512_SIZE];
    size_t data_size;

    if(!signet) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    memset(hash, 0, SHA_512_SIZE);

    if(!(data = sgnt_signet_serialize_upto_fid(signet, fid, &data_size))) {
        RET_ERROR_PTR(ERR_UNSPEC, "no signet data to fingerprint");
    }

    if(_compute_sha_hash(512, data, data_size, hash) < 0) {
        free(data);
        RET_ERROR_PTR(ERR_UNSPEC, "could not compute SHA-512 hash of full signet data");
    }

    free(data);

    if(!(b64_fingerprint = _b64encode_nopad(hash, SHA_512_SIZE))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not base-64 encode full signet fingerprint");
    }

    return b64_fingerprint;
}


/* signet_field_t creators, destructors and related functions*/

/**
 * @brief   Creates a chain of signet_field_t structures for all fields in the provided signet with the specified field id. The fields are in a linked list, preserving the order in which they are in the signet.
 * @param   signet  Pointer to the target signet.
 * @param   fid Field id that specifies the target fields.
 * @return  Pointer to the first signet_field_t structure in the created linked list. Null if error occurred.
*/
static signet_field_t *sgnt_fieldlist_create(const signet_t *signet, unsigned char fid) {

    int res;
    uint32_t offset;
    signet_field_t *field;
    signet_field_t *temp;
    signet_field_key_t *key;

    if(!signet) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if((res = sgnt_fid_exists(signet, fid)) < 0) {
        RET_ERROR_PTR(ERR_UNSPEC, "error searching for field in signet");
    } else if(!res) {
        RET_ERROR_PTR(ERR_UNSPEC, "specified field data does not exist");
    }

    offset = signet->fields[fid] - 1;

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        key = &(signet_org_field_keys[fid]);
        break;
    case SIGNET_TYPE_USER:
        key = &(signet_user_field_keys[fid]);
        break;
    case SIGNET_TYPE_SSR:
        key = &(signet_ssr_field_keys[fid]);
        break;
    default:
        RET_ERROR_PTR(ERR_UNSPEC, "invalid signet type");
        break;
    }

    if(!key->name) {
        RET_ERROR_PTR(ERR_UNSPEC, "signet field id does not support creation");
    }

    if(!(field = sgnt_fieldnode_create(signet, offset, key))) {
        RET_ERROR_PTR(ERR_UNSPEC, "error creating signet field");
    }

    offset = field->data_offset + field->data_size;
    temp = field;

    while(offset < signet->size && signet->data[offset] == fid) {

        if(!(temp->next = sgnt_fieldnode_create(signet, offset, key))) {
            sgnt_fieldlist_destroy(field);
            RET_ERROR_PTR(ERR_UNSPEC, "error creating signet field");
        }

        temp = temp->next;
        offset = temp->data_offset + temp->data_size;
    }

    return field;
}


/**
 * @brief   Creates signet_field_t type structure from the data in the signet at the specified offset. Explicit call creates an unchained structure.
 * @param   signet  Pointer to the signet that contains the field that is having a signet_field_t indexing structure created for it.
 * @param   offset  The offset at which the field physically begins in the signet->data array.
 * @param   key Pointer to the field id specific key that contains information on the format of the field data.
 * @return  Pointer to the created field structure, NULL on failure.
*/
static signet_field_t *sgnt_fieldnode_create(const signet_t *signet, uint32_t offset, signet_field_key_t *key) {

    uint32_t at = offset;
    signet_field_t *field;

    if (!signet) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if((at + 1) >= signet->size) {
        RET_ERROR_PTR(ERR_UNSPEC, "offset exceeded signet size");
    }

    if (!(field = malloc(sizeof(signet_field_t)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, NULL);
    }

    memset(field, 0, sizeof(signet_field_t));
    field->signet = signet;
    field->key = key;
    field->id_offset = at++;

    if(key->bytes_name_size) {
        field->name_size = signet->data[at++];
        field->name_offset = at++;
        at = field->name_offset + field->name_size;

        if(at >= signet->size) {
            sgnt_fieldnode_destroy(field);
            RET_ERROR_PTR(ERR_UNSPEC, "offset exceeded signet size");
        }
    }

    switch(key->bytes_data_size) {

    case 0:
        field->data_size = key->data_size;
        break;
    case 1:
        field->data_size = (uint32_t)signet->data[at];
        break;
    case 2:
        if(at + 1 >= signet->size) {
            sgnt_fieldnode_destroy(field);
            RET_ERROR_PTR(ERR_UNSPEC, "buffer overflow in signet");
        }

        field->data_size = _int_no_get_2b(signet->data + at);
        break;
    case 3:
        if(at + 2 >= signet->size) {
            sgnt_fieldnode_destroy(field);
            RET_ERROR_PTR(ERR_UNSPEC, "buffer overflow in signet");
        }

        field->data_size = _int_no_get_3b(signet->data + at);
        break;

    }

    at += key->bytes_data_size;
    field->data_offset = at;

    if(at + field->data_size - 1 >= signet->size) {
        sgnt_fieldnode_destroy(field);
        RET_ERROR_PTR(ERR_UNSPEC, "buffer overflow in signet operation");
    }

    field->next = NULL;

    return field;
}


/**
 * @brief   Destroys a chain of signet_field_t structures starting with provided structure, does not effect the linked signet_t structure.
 * @param   field   Pointer to the first signet_field_t in the linked list to be deleted.
*/
static void sgnt_fieldlist_destroy(signet_field_t *field) {

    signet_field_t *temp = field;

    if(!field) {
        return;
    }

    while(temp) {
        temp = sgnt_fieldnode_destroy(temp);
    }

}


/**
 * @brief   Destroys signet_field_t type structure.
 * @param   field   Pointer to the signet_field_t to be destroyed.
 * @return  Pointer to the signet_field_t that was the destroyed structure was linked to, NULL if the destroyed structure was the last/only structure in the linked list, or error.
*/
static signet_field_t *sgnt_fieldnode_destroy(signet_field_t *field) {

    signet_field_t *next;

    if(!field) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    next = field->next;
    free(field);
    return next;
}


/**
 * @brief   Dumps the field indexed by the specified signet_field_t. Mainly called by _signet_fid_dump(...), may be poorly formatted for an explicit single-field dump.
 * @param   fp  Dump target stream.
 * @param   field   Pointer to signet_field_t structure that indexes a signet field to be dumped.
 * @return  0 on success -1 on failure.
*/

static int sgnt_field_dump(FILE *fp, const signet_field_t *field) {

    char *name, *data = NULL;
    const char *png_name = "PNG file", *nbuf;

    if(!fp || !field) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    fprintf(fp, "--- %-*d ", 3, field->signet->data[field->id_offset]);

    if(!(field->key->bytes_name_size)) {

        if(!(name = malloc(strlen(field->key->name) + 1))) {
            PUSH_ERROR_SYSCALL("malloc");
            RET_ERROR_INT(ERR_NOMEM, NULL);
        }

        memset(name, 0, strlen(field->key->name) + 1);
        memcpy(name, field->key->name, strlen(field->key->name));
    } else {

        if(!(name = malloc(field->name_size + 1))) {
            PUSH_ERROR_SYSCALL("malloc");
            RET_ERROR_INT(ERR_NOMEM, NULL);
        }

        memset(name, 0, field->name_size + 1);
        memcpy(name, field->signet->data + field->name_offset, field->name_size);
    }

    switch(field->key->data_type) {

    case UNICODE:                                                                   // TODO Unicode currently same as ASCII

        if(!(data = malloc(field->data_size + 1))) {
            PUSH_ERROR_SYSCALL("malloc");
            free(name);
            RET_ERROR_INT(ERR_NOMEM, NULL);
        }

        memset(data, 0, field->data_size + 1);
        memcpy(data, field->signet->data + field->data_offset, field->data_size);
        break;
    case HEX:                                                                       // TODO
    case B64:

        if(!(data = _b64encode_nopad(field->signet->data + field->data_offset, (size_t)field->data_size))) {
            free(name);
            RET_ERROR_INT(ERR_UNSPEC, "could not base64-encode signet field data");
        }

        break;
    case PNG:

        if(!(data = strdup(png_name))) {
            PUSH_ERROR_SYSCALL("strdup");
            free(name);
            RET_ERROR_INT(ERR_NOMEM, NULL);
        }

        break;

    }

    if (!strlen(name)) {
        nbuf = "----------------";
    } else {
        nbuf = name;
    }

    fprintf(fp, "%-26.26s -> %-90.90s%s\n", nbuf, data ? data : "(null)", data && strlen(data) > 90 ? "..." : "");
    free(name);
    free(data);

    return 0;
}


/**
 * @brief       Retrieves the total length of a serialized field specified by a signet_field_t structure.
 * @param   field   Pointer to the signet_field_t structure that indexes the field, the size of which is retrieved.
 * @return  Length of the serialized field. -1 if error.
 *              NOTE: int overflow should not occur because field size and signet size are bounded well below 2^31 bits.
*/

static int sgnt_field_size_serial_get(const signet_field_t *field) {

    int field_size = 1;

    if(!field) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
        return 0;
    }

    if(field->key->bytes_name_size) {
        field_size += (int)(1 + field->name_size);
    }

    field_size += (field->key->bytes_data_size + field->data_size);

    return field_size;
}


/* signet field data retrieval functions */

/**
 * @brief   Dumps all fields with specified field id.
 * @param   fp  Dump target stream.
 * @param   signet  Pointer to signet from which fields are dumped.
 * @param   fid Field id that specifies which fields are dumped.
 * @return  0 on success, -1 on failure.
*/
static int sgnt_fid_dump(FILE *fp, const signet_t *signet, unsigned int fid) {

    signet_field_t *field, *temp;

    if(!fp || !signet) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if(!(field = sgnt_fieldlist_create(signet, fid))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not create signet field for data dump");
    }

    temp = field;

    while(temp) {

        if(sgnt_field_dump(fp, temp) < 0) {
            sgnt_fieldlist_destroy(field);
            RET_ERROR_INT(ERR_UNSPEC, "could not dump field");
        }

        temp = temp->next;
    }

    sgnt_fieldlist_destroy(field);

    return 0;
}


#if 0 /* currently unused */
/**
 * @brief   Retrieves the serialized representation of all fields with the specified field id in the signet.
 * @param   signet  Pointer to the target signet.
 * @param   fid Specified field id.
 * @param   out_len Pointer to the length of returned array.
 * @return  Array containing serialized fields with the specified field id, NULL if an error occurs. Caller is responsible for freeing memory after use.
*/
static unsigned char *sgnt_serial_from_fid(const signet_t *signet, unsigned char fid, size_t *fid_size) {

    int res;
    unsigned char *data;

    if(!signet || !fid_size) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if((res = sgnt_fid_exists(signet, fid)) < 0) {
        RET_ERROR_PTR(ERR_UNSPEC, "error searching for field in signet");
    } else if(!res) {
        RET_ERROR_PTR(ERR_UNSPEC, "defined field does not exist");
    }

    if ((res = sgnt_fid_get_size(signet, fid)) < 0) {
        RET_ERROR_PTR_FMT(ERR_UNSPEC, "error retrieving size of field %u", fid);
    }

    if(!(data = malloc(*fid_size))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, NULL);
    }

    memset(data, 0, res);
    memcpy(data, &(signet->data[signet->fields[fid] - 1]), res);
    *fid_size = res;

    return data;
}
#endif

/**
 * @brief   Allocates memory for and serializes all fields from field id = 0 upto and including the specified field id.
 * @param   signet      Pointer to target signet.
 * @param   fid     Specified field.
 * @param   data_size   Pointer to the length of the returned array.
 * @return  Allocated array of serialized fields, NULL on failure or if the signet was empty.
*/
static unsigned char *sgnt_signet_serialize_upto_fid(const signet_t *signet, unsigned char fid, size_t *data_size) {

    unsigned char *data;

    if(!signet || !data_size) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    *data_size = signet->size;

    for(int i = fid + 1; i <= SIGNET_FID_MAX; ++i) {

        if(signet->fields[i]) {
            *data_size = signet->fields[i] - 1;
            break;
        }
    }

    if(!(*data_size)) {
        return NULL;
    }

    if(!(data = malloc(*data_size))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, NULL);
    }

    memset(data, 0, *data_size);
    memcpy(data, signet->data, *data_size);

    return data;
}


/* signet content modification and related functions */

/**
 * @brief   Helper function which removes a substring of length field_size from the target signet at offset.
 * @param   signet      Pointer to target signet.
 * @param   offset      Offset at which the field intended for removal begins in the target signet.
 * @param   data_size   Size of field to be removed.
 * @return  0 on success, -1 on failure.
*/
static int sgnt_field_remove_at(signet_t *signet, unsigned int offset, size_t data_size) {

    size_t signet_size;

    if(!signet) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if(signet->size < offset + data_size) {
        RET_ERROR_INT(ERR_UNSPEC, "signet field position exceeded signet size");
    }

    if (!(signet_size = signet->size - data_size)) {
        signet->size = 0;
        free(signet->data);
        signet->data = NULL;
        return 0;
    }

    if(offset + data_size != signet->size) {
        memmove(signet->data + offset, signet->data + offset + data_size, signet->size - offset - data_size);
    }

    if(!(signet->data = realloc(signet->data, signet_size))) {
        PUSH_ERROR_SYSCALL("realloc");
        signet->size = 0;
        RET_ERROR_INT(ERR_NOMEM, NULL);
    }

    signet->size = signet_size;

    return 0;
}


/**
 * @brief   Helper function which inserts an array of field data into the signet data array.
 * @param   signet      Pointer to target signet.
 * @param   offset      Offset at which the field data must be inserted.
 * @param   field_size  Size of field data to be inserted.
 * @param   field_data  Field data to be inserted into the signet.
 * @return  0 on success, -1 on failure.
*/

static int sgnt_field_create_at(signet_t *signet, unsigned int offset, size_t field_size, const unsigned char *field_data) {

    uint32_t signet_size;

    if(!signet || (signet->size + field_size) > UNSIGNED_MAX_3_BYTE || !field_size || !field_data) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    signet_size = signet->size + field_size;

    if(!(signet->data = realloc(signet->data, signet_size))) {
        PUSH_ERROR_SYSCALL("realloc");
        signet->size = 0;
        RET_ERROR_INT(ERR_NOMEM, NULL);
    }

    memset(signet->data + signet->size, 0, field_size);

    if(offset != (size_t)signet->size) {
        memmove(signet->data + offset + field_size, signet->data + offset, signet->size - offset);
    }

    signet->size = signet_size;
    memcpy(signet->data + offset, field_data, field_size);

    return 0;
}


/**
 * @brief   Uses specified ED25519 key to sign the target signet.
 *              The signature is placed into the field specified by the signet_fid and the signature is taken of all the fields that come before signet_fid.
 * @param   signet  Pointer to the target signet to be signed.
 * @param   signet_fid  Target field which will hold the signature, it also specifies which fields are to be signed (all the fields that come before it.)
 * @param   key ed25519 key object containing the private key, which is used for signing.
 * @return  0 if signing was successful, otherwise -1.
*/
static int sgnt_field_sign(signet_t *signet, unsigned char signet_fid, ED25519_KEY *key) {

    int res;
    size_t data_size;
    unsigned char *data;
    ed25519_signature sig;
    signet_field_key_t *keys;

    if(!signet || !key) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        keys = signet_org_field_keys;
        break;
    case SIGNET_TYPE_USER:
        keys = signet_user_field_keys;
        break;
    case SIGNET_TYPE_SSR:
        keys = signet_ssr_field_keys;
        break;
    default:
        RET_ERROR_INT(ERR_UNSPEC, "invalid signet type");
        break;

    }

    if((res = sgnt_validate_required_upto_fid(signet, keys, signet_fid)) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "an error occurred while checking for required fields");
    } else if(!res) {
        RET_ERROR_INT(ERR_UNSPEC, "required fields for signet signing were missing");
    }

    if(!(data = sgnt_signet_serialize_upto_fid(signet, signet_fid - 1, &data_size))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not get signet data for signature");
    }

    if(_ed25519_sign_data(data, data_size, key, sig) < 0) {
        free(data);
        RET_ERROR_INT(ERR_UNSPEC, "could not sign signet data");
    }

    free(data);

    while((res = sgnt_fid_exists(signet, signet_fid))) {

        if(res < 0) {
            RET_ERROR_INT(ERR_UNSPEC, "error searching for field in signet");
        }

        if(sgnt_fid_num_remove(signet, signet_fid, 1) < 0) {
            RET_ERROR_INT(ERR_UNSPEC, "could not remove signature field from signet");
        }
    }

    if ((res = sgnt_field_defined_set(signet, signet_fid, ED25519_SIG_SIZE, (const unsigned char *)sig)) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "could not add signature field to signet");
    }

    return res;
}


/* Create new signet and load signet from file */

/**
 * @brief   Creates a signet structure with public signing and encyption keys. Also creates a keys file in which the private keys are stored.
 * @param   type        Signet type, org, user or ssr (SIGNET_TYPE_ORG, SIGNET_TYPE_USER or SIGNET_TYPE_SSR).
 * @param   keysfile    Null terminated string containing the name of the keyfile to be created.
 * @return  Pointer to the newly created and allocated signet_t structure or NULL on error.
 * @free_using{sgnt_destroy_signet}
*/
static signet_t *sgnt_signet_create_w_keys(signet_type_t type, const char *keysfile) {

    EC_KEY *enc_key;
    ED25519_KEY *sign_key;
    int res;
    keys_type_t keys_type;
    signet_t *signet;
    size_t f_len;

    if(!keysfile) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(f_len = strlen(keysfile))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not write data to empty file path");
    }

    switch(type) {

    case SIGNET_TYPE_ORG:
        keys_type = KEYS_TYPE_ORG;
        break;
    case SIGNET_TYPE_USER:
        keys_type = KEYS_TYPE_USER;
        break;
    case SIGNET_TYPE_SSR:
        keys_type = KEYS_TYPE_USER;
        break;
    default:
        RET_ERROR_PTR(ERR_BAD_PARAM, "invalid signet type");
        break;

    }

    if(!(sign_key = _generate_ed25519_keypair())) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not generate ed25519 key pair");
    }

    if(!(enc_key = _generate_ec_keypair())) {
        _free_ed25519_key(sign_key);
        RET_ERROR_PTR(ERR_UNSPEC, "could not generate elliptic curve key pair");
    }

    if(dime_keys_file_create(keys_type, sign_key, enc_key, keysfile) < 0) {
        _free_ec_key(enc_key);
        _free_ed25519_key(sign_key);
        RET_ERROR_PTR(ERR_UNSPEC, "could not write private keys to file");
    }

    if(!(signet = sgnt_signet_create(type))) {
        _free_ec_key(enc_key);
        _free_ed25519_key(sign_key);
        RET_ERROR_PTR(ERR_UNSPEC, "could not create signet object");
    }

    res = sgnt_signkey_set(signet, sign_key, SIGNKEY_DEFAULT_FORMAT);
    _free_ed25519_key(sign_key);

    if(res < 0) {
        sgnt_signet_destroy(signet);
        _free_ec_key(enc_key);
        RET_ERROR_PTR(ERR_UNSPEC, "could not add signing key field to signet");
    }

    res = sgnt_enckey_set(signet, enc_key, 0);
    _free_ec_key(enc_key);

    if(res < 0) {
        sgnt_signet_destroy(signet);
        RET_ERROR_PTR(ERR_UNSPEC, "could not add encryption key field to signet");
    }

    return signet;
}


/* Loading signet from and saving to file */

/**
 * @brief   Loads signet_t structure from a PEM formatted file specified by filename.
 * @param   filename    Null terminated string containing the filename of the file containing the signet.
 * @return  Pointer to a newly created signet_t structure loaded from the file, NULL on failure.
 * @free_using{sgnt_destroy_signet}
*/
static signet_t *sgnt_signet_load(const char *filename) {

    char *b64_signet = NULL;
    signet_t *signet;

    if(!filename) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!(b64_signet = _read_pem_data(filename, SIGNET_USER, 1)) &&
    	!(b64_signet = _read_pem_data(filename, SIGNET_ORG, 1))) {
        RET_ERROR_PTR_FMT(ERR_UNSPEC, "could not load signet from file: %s", filename);
    }

    if(!(signet = sgnt_signet_b64_deserialize(b64_signet))) {
        free(b64_signet);
        RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize signet from base64 encoded data");
    }

    free(b64_signet);

    return signet;
}


/**
 * @brief   Stores a signet from the signet_t structure in a PEM formatted file specified by the filename.
 * @param   signet      Pointer to the signet_t structure containing the signet.
 * @param   filename    Null terminated string containing the desired filename for the signet.
 * @return  0 on success, -1 on failure.
*/
static int sgnt_file_create(signet_t *signet, const char *filename) {

    char *armored, *checksum;

    if(!signet || !filename) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (!(checksum = sgnt_signet_crc24_serialize(signet))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not serialize the signet checksum");
    }

    if(!(armored = sgnt_signet_b64_serialize(signet))) {
    	free(checksum);
        RET_ERROR_INT(ERR_UNSPEC, "could not serialize the armored signet");
    }

    if(_write_pem_data(armored, checksum, signet->type == SIGNET_TYPE_USER ? SIGNET_USER : SIGNET_ORG, filename) < 0) {
        free(armored);
        RET_ERROR_INT(ERR_UNSPEC, "could not write signet to PEM file");
    }

    free(armored);

    return 0;
}


/* Initializing and destroying signets*/

/**
 * @brief   Returns a new signet_t structure that gets deserialized from a data buffer
 * @param   in  data buffer that should contain the binary form of a signet
 * @param   in_len  length of data buffer
 * @return  A pointer to a newly allocated signet_t structure type, NULL on failure.
 * @free_using{sgnt_destroy_signet}
 */
static signet_t *sgnt_signet_binary_deserialize(const unsigned char *in, size_t in_len) {

    size_t data_size = 0;
    dime_number_t magic_num;
    signet_t *signet;
    signet_type_t type;

    if(!in || !in_len) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(sgnt_length_serial_check(in, in_len) < 0) {
        RET_ERROR_PTR(ERR_UNSPEC, "supplied buffer length was too small for signet input");
    }

    magic_num = (dime_number_t)_int_no_get_2b((void *)in);

    switch(magic_num) {

    case DIME_ORG_SIGNET:
        type = SIGNET_TYPE_ORG;
        break;
    case DIME_USER_SIGNET:
        type = SIGNET_TYPE_USER;
        break;
    case DIME_SSR:
        type = SIGNET_TYPE_SSR;
        break;
    default:
        RET_ERROR_PTR(ERR_UNSPEC, "input buffer is not a signet");
        break;

    }

    if(!(signet = sgnt_signet_create(type))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not create new signet");
    }

    data_size = (size_t)_int_no_get_3b(in + 2);

    if(!(signet->data = malloc(data_size))) {
        PUSH_ERROR_SYSCALL("malloc");
        sgnt_signet_destroy(signet);
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate memory for signet data");
    }

    memset(signet->data, 0, data_size);
    memcpy(signet->data, in + SIGNET_HEADER_SIZE, data_size);
    signet->size = (uint32_t)data_size;

    if(sgnt_signet_index(signet) < 0) {
        sgnt_signet_destroy(signet);
        RET_ERROR_PTR(ERR_UNSPEC, "could not parse input buffer into signet");
    }

    return signet;
}


/**
 * @brief   Deserializes a b64 signet into a signet structure.
 * @param   b64_in  Null terminated array of b64 signet data.
 * @return  Pointer to newly allocated signet structure, NULL if failure.
 * @free_using{sgnt_destroy_signet}
*/
static signet_t *sgnt_signet_b64_deserialize(const char *b64_in) {

    unsigned char *in;
    size_t size = 0;
    signet_t *signet;

    if (!b64_in) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(in = _b64decode(b64_in, strlen(b64_in), &size))) {
        RET_ERROR_PTR(ERR_UNSPEC, "base64 decoding of armored signet failed");
    }

    if (!(signet = sgnt_signet_binary_deserialize(in, size))) {
        free(in);
        RET_ERROR_PTR(ERR_UNSPEC, "unable to initialize signet from data");
    }

    free(in);

    return signet;
}


/**
 * @brief   Destroys a signet and frees the memory.
 * @param   signet  Pointer to signet to be destroyed.
*/
static void sgnt_signet_destroy(signet_t *signet) {

    if(!signet) {
        return;
    }

    if(signet->data) {
        free(signet->data);
    }

    free(signet);

}


/* Serializing signet into binary and b64 */

/**
 * @brief   Serializes a signet structure into binary data.
 * @param   signet      Pointer to the target signet.
 * @param   serial_size Pointer to the value that stores the length of the array returned.
 * @return  Signet serialized into binary data. Null on error.
 * @free_using{free}
*/
static unsigned char *sgnt_signet_binary_serialize(signet_t *signet, uint32_t *serial_size) {

    unsigned char *serial;
    dime_number_t number;

    if(!signet || !serial_size) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        number = DIME_ORG_SIGNET;
        break;
    case SIGNET_TYPE_USER:
        number = DIME_USER_SIGNET;
        break;
    case SIGNET_TYPE_SSR:
        number = DIME_SSR;
        break;
    default:
        RET_ERROR_PTR(ERR_UNSPEC, "invalid signet type");
        break;

    }

    *serial_size = signet->size + SIGNET_HEADER_SIZE;

    if(!(serial = malloc(*serial_size))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, NULL);
    }

    memset(serial, 0, *serial_size);
    _int_no_put_2b(serial, (uint16_t)number);
    _int_no_put_3b(serial + 2, signet->size);
    memcpy(serial + SIGNET_HEADER_SIZE, signet->data, signet->size);

    return serial;
}

/**
 * @brief   Serializes a signet structure into b64 data.
 * @param   signet      Pointer to the target signet.
 * @return  Signet serialized into b64 data. Null on error.
 * @free_using{free}
*/
static char *sgnt_signet_b64_serialize(signet_t *signet) {

    unsigned char *serial;
    uint32_t serial_size = 0;
    char *base64;

    if (!signet) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!(serial = sgnt_signet_binary_serialize(signet, &serial_size))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not serialize signet");
    }

    base64 = _b64encode(serial, (size_t)serial_size);
    free(serial);

    if(!base64) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not base64 encode serialized signet");
    }

    return base64;
}

/**
 * @brief   Serializes a signet structure and calculates the crc24 checksum then returns it in base64 form.
 * @param   signet      Pointer to the target signet.
 * @return  CRC24 serialized into b64 data. Null on error.
 * @free_using{free}
*/
static char *sgnt_signet_crc24_serialize(signet_t *signet) {

    unsigned char *serial, be[3];
    uint32_t serial_size = 0, crc;
    char *base64, *result;

    if (!signet) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!(serial = sgnt_signet_binary_serialize(signet, &serial_size))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not serialize signet");
    }

    crc = _compute_crc24_checksum(serial, serial_size);
    free(serial);

	be[0] = ((unsigned char *)&crc)[2];
	be[1] = ((unsigned char *)&crc)[1];
	be[2] = ((unsigned char *)&crc)[0];

	if (!(base64 = _b64encode((unsigned char *)&be, 3))) {
		RET_ERROR_PTR(ERR_UNSPEC, "could not encode the crc value into a base64 string");
	}
    if(!(result = malloc(strlen(base64) + 3)) || (snprintf(result, strlen(base64) + 3, "\n=%s", base64) != 6)) {
    	if (result) free(result);
    	free(base64);
    	RET_ERROR_PTR(ERR_UNSPEC, "could not base64 encode the signet crc");
    }

    free(base64);
    return result;
}



/* Dump signet */

/**
 * @brief   Dumps signet into the specified file descriptor.
 * @param   fp  File descriptor the signet is dumped to.
 * @param   signet  Pointer to the signet_t structure to be dumped.
*/
static void sgnt_signet_dump(FILE *fp, signet_t *signet) {

    const char *type;
    int res;
    unsigned int version = SIGNET_VER_NO;
    signet_type_t signet_type;

    if(!signet || !fp) {
        return;
    }

    signet_type = sgnt_type_get(signet);

    switch(signet_type) {

    case SIGNET_TYPE_ORG:
        type = "organizational";
        break;
    case SIGNET_TYPE_USER:
        type = "user";
        break;
    case SIGNET_TYPE_SSR:
        type = "SSR";
        break;
    default:
        fprintf(fp, "--- Unrecognized signet type (%u) could not be dumped.\n", signet_type);

        if (get_last_error()) {
            dump_error_stack();
            _clear_error_stack();
        }

        return;
        break;

    }

    fprintf(fp, "--- version: %d, size = %d, signet type = %s\n", version, signet->size + SIGNET_HEADER_SIZE, type);

    for(int i = 0; i < SIGNET_FID_MAX + 1; ++i) {

        if((res = sgnt_fid_exists(signet, i)) < 0) {
            fprintf(fp, "Error: field existence check failed.\n");
            dump_error_stack();
            _clear_error_stack();
            return;
        } else if(res == 1) {

            if(sgnt_fid_dump(fp, signet, i) < 0) {
                _clear_error_stack();
                return;
            }

        }

        if((i == SIGNET_USER_CRYPTO_SIG && signet_type == SIGNET_TYPE_USER) || (i == SIGNET_ORG_CRYPTO_SIG && signet_type == SIGNET_TYPE_ORG)) {
            fprintf(fp, "%-34s    %s\n", "------- End cryptographic signet", "------------------------------------------------------------------------------------------");
        }

        if((i == SIGNET_USER_FULL_SIG && signet_type == SIGNET_TYPE_USER) || (i == SIGNET_ORG_FULL_SIG && signet_type == SIGNET_TYPE_ORG)) {
            fprintf(fp, "%-34s    %s\n", "------- End full signet", "------------------------------------------------------------------------------------------");
        }

    }

}


/* Retrieving signet states */

/**
 * @brief   Retrieves the number of fields with the specified field id.
 * @param   signet  Pointer to the target signet.
 * @param   fid The target field id.
 * @return  The number of fields with specified field id. On various errors returns -1.
 *              NOTE: int overflow should not occur because of field size lower and signet size upper bounds.
*/
static int sgnt_fid_count_get(const signet_t *signet, unsigned char fid) {

    int count = 0, res;
    signet_field_t *field, *temp;

    if(!signet) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if((res = sgnt_fid_exists(signet, fid)) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "error searching for field in signet");
    } else if(!res) {
        return count;
    }

    if(!(field = sgnt_fieldlist_create(signet, fid))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not create signet field object");
    }

    temp = field;

    while(temp) {
        temp = (signet_field_t *)temp->next;
        ++count;
    }

    sgnt_fieldlist_destroy(field);

    return count;
}


/**
 * @brief   Checks for presence of field with specified id in the signet
 * @param   signet  The signet to be checked
 * @param   fid Specified field id
 * @return  1 if such a field exists, 0 if it does not exist, -1 if error.
*/
static int sgnt_fid_exists(const signet_t *signet, unsigned char fid) {

    if(!signet) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    return (signet->fields[fid] ? 1 : 0);
}


/**
 * @brief   Checks the state of the specified signet, performing all non-cryptographic checks.
 * @param   signet  Pointer to the target signet.
 * @return  Signet state, SS_UNKNOWN on failure.
*/
static signet_state_t sgnt_validate_structure(const signet_t *signet) {

    int res;
    unsigned char full_sig, id_sig, crypto_sig;
    signet_field_key_t *keys;
    signet_field_t *field;
    signet_type_t type;

    if (!signet) {
        RET_ERROR_CUST(SS_UNKNOWN, ERR_BAD_PARAM, NULL);
    }

    /* check legal signet type */
    type = sgnt_type_get(signet);

    switch(type) {

    case SIGNET_TYPE_ORG:
        keys = signet_org_field_keys;
        crypto_sig = SIGNET_ORG_CRYPTO_SIG;
        id_sig = SIGNET_ORG_ID_SIG;
        full_sig = SIGNET_ORG_FULL_SIG;
        break;
    case SIGNET_TYPE_USER:
        keys = signet_user_field_keys;
        crypto_sig = SIGNET_USER_CRYPTO_SIG;
        id_sig = SIGNET_USER_ID_SIG;
        full_sig = SIGNET_USER_FULL_SIG;
        break;
    case SIGNET_TYPE_SSR:
        keys = signet_ssr_field_keys;
        full_sig = 0;
        id_sig = 0;
        crypto_sig = 0;
        break;
    default:
        RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "invalid signet type");
        break;

    }

    /* check against presence of illegal fields and presence of multiple unique fields
    Also, check field format */

    for(int i = 0; i < SIGNET_FID_MAX + 1; ++i) {

        if((res = sgnt_fid_exists(signet, i))) {

            if(res < 0) {
                RET_ERROR_UINT(ERR_UNSPEC, "could not determine existence of specified field in signet");
            }

            if(!keys[i].name) {
                return SS_MALFORMED;
            }

            if(!(field = sgnt_fieldlist_create(signet, i))) {
                return SS_MALFORMED;
            }

            if(keys[i].unique && field->next) {
                sgnt_fieldlist_destroy(field);
                return SS_MALFORMED;
            }

            sgnt_fieldlist_destroy(field);
        }
    }

    /* check to avoid signet exceeding maximum size */
    if(sgnt_signet_size_serial_get(signet) > SIGNET_MAX_SIZE) {
        RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "signet size exceeds maximum size");
    }

    /* Check signatures and required fields to determine the signet state*/
    if(type == SIGNET_TYPE_USER || type == SIGNET_TYPE_ORG) {

        if((res = sgnt_fid_exists(signet, id_sig))) {

            if(res < 0) {
                RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "error searching for identifiable signature field in signet");
            }

            if((res = sgnt_validate_required_upto_fid(signet, keys, id_sig)) < 0) {
                RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "could not determine existence of required field");
            } else if (res) {
                return SS_ID;
            } else {
                return SS_INCOMPLETE;
            }
        }

        if((res = sgnt_fid_exists(signet, full_sig))) {

            if(res < 0) {
                RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "error searching for full signature field in signet");
            }

            if((res = sgnt_validate_required_upto_fid(signet, keys, full_sig)) < 0) {
                RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "could not determine existence of required field");
            } else if (res) {
                return SS_FULL;
            } else {
                return SS_INCOMPLETE;
            }
        }

        if((res = sgnt_fid_exists(signet, crypto_sig))) {

            if(res < 0) {
                RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "error searching for crypto signature field in signet");
            }

            if((res = sgnt_validate_required_upto_fid(signet, keys, crypto_sig)) < 0) {
                RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "could not determine existence of required fields");
            } else if(res) {
                return SS_CRYPTO;
            } else {
                return SS_INCOMPLETE;
            }
        }
    }

    if(type == SIGNET_TYPE_SSR) {

        if((res = sgnt_fid_exists(signet, SIGNET_USER_SSR_SIG))) {

            if(res < 0) {
                RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "error searching for ssr signature field in signet");
            }

            if((res = sgnt_validate_required_upto_fid(signet, keys, SIGNET_USER_SSR_SIG)) < 0) {
                RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "could not determine existence of required fields");
            } else if (res) {
                return SS_SSR;
            } else {
                return SS_INCOMPLETE;
            }
        }
    }

    return SS_INCOMPLETE;
}


/**
 * @brief   Retrieves the signet type, org or user (SIGNET_TYPE_ORG or SIGNET_TYPE_USER)
 * @param   signet  Pointer to the target signet.
 * @return  A signet_type_t enum type with the signet type, SIGNET_TYPE_ERROR on failure.
*/
static signet_type_t sgnt_type_get(const signet_t *signet) {

    if (!signet) {
        RET_ERROR_CUST(SIGNET_TYPE_ERROR, ERR_BAD_PARAM, NULL);
    }

    return (signet_type_t)signet->type;
}


/* Retrieving field data */

/**
 * @brief   Fetches the binary data value of the field specified by field id and the number at which it appears in the signet amongst fields with the same field id (1, 2, ...).
 * @param   signet  Pointer to the target signet.
 * @param   fid Specified field id.
 * @param   num Specified field number based on the order in which it appears in the signet.
 * @param   out_len Pointer to the length of returned array.
 * @return  Array containing the binary data of the specified field, NULL if an error occurs. Caller is responsible for freeing memory.
*/
static unsigned char *sgnt_fid_num_fetch(const signet_t *signet, unsigned char fid, uint32_t num, size_t *out_len) {

    int res;
    unsigned char *data;
    signet_field_t *field, *temp;

    if(!signet || !out_len) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if((res = sgnt_fid_exists(signet, fid)) < 0) {
        RET_ERROR_PTR(ERR_UNSPEC, "error searching for field in signet");
    } else if (!res) {
        RET_ERROR_PTR(ERR_UNSPEC, "specified field does not exist");
    }

    if(!(field = sgnt_fieldlist_create(signet, fid))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not create signet field");
    }

    temp = field;

    for(uint32_t i = 1; i < num; ++i) {
        temp = temp->next;

        if(!temp) {
            sgnt_fieldlist_destroy(field);
            RET_ERROR_PTR(ERR_UNSPEC, "signet field index exceeded number of present field elements");
        }
    }

    if(!(data = malloc(temp->data_size))) {
        PUSH_ERROR_SYSCALL("malloc");
        sgnt_fieldlist_destroy(field);
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for signet data");
    }

    *out_len = temp->data_size;
    memset(data, 0, *out_len);
    memcpy(data, &(signet->data[temp->data_offset]), *out_len);
    sgnt_fieldlist_destroy(field);

    return data;
}


/**
 * @brief   Fetches the first undefined field with the specified field name.
 * @param   signet      Pointer to the target signet.
 * @param   name_size   Length of the passed array containing the length of the target field name.
 * @param   name        Array containing the name of the desired undefined field.
 * @param   data_size       Pointer to the size of the array that gets returned by the function.
 * @return  The array containing the data from the specified field or NULL in case of failure such as if the field was not found.
*/
static unsigned char *sgnt_field_undefined_fetch(const signet_t *signet, size_t name_size, const unsigned char *name, size_t *data_size) {

    int res;
    unsigned char undef_id, *data;
    signet_field_t *field, *temp;

    if(!signet || !name || !data_size) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        undef_id = SIGNET_ORG_UNDEFINED;
        break;
    case SIGNET_TYPE_USER:
        undef_id = SIGNET_USER_UNDEFINED;
        break;
    default:
        RET_ERROR_PTR(ERR_UNSPEC, "signet type does not support undefined fields");
        break;

    }

    if((res = sgnt_fid_exists(signet, undef_id)) < 0) {
        RET_ERROR_PTR(ERR_UNSPEC, "error finding undefined fields in signet");
    } else if(!res) {
        RET_ERROR_PTR(ERR_UNSPEC, "no undefined fields exist in signet");
    }

    if(!(field = sgnt_fieldlist_create(signet, undef_id))) {
        RET_ERROR_PTR(ERR_UNSPEC, "failed to create signet field object");
    }

    temp = field;

    while(temp) {

        if(temp->name_size == name_size) {

            if(!memcmp(temp->signet->data + temp->name_offset, name, name_size)) {

                if(!(data = malloc(temp->data_size))) {
                    PUSH_ERROR_SYSCALL("malloc");
                    sgnt_fieldlist_destroy(field);
                    RET_ERROR_PTR(ERR_NOMEM, "could not allocate memory for signet field data");
                }

                memset(data, 0, temp->data_size);
                memcpy(data, &(signet->data[temp->data_offset]), temp->data_size);
                *data_size = temp->data_size;
                sgnt_fieldlist_destroy(field);

                return data;
            }
        }

        temp = (signet_field_t *)temp->next;
    }

    sgnt_fieldlist_destroy(field);

    RET_ERROR_PTR(ERR_UNSPEC, "could not find undefined field with requested name");
}


/**
 * @brief   Converts a signing key from a signet serial format to a ed25519 public signing key object.
 * @param   serial_key  Ed25519 signing key in a signet serial format.
 * @param   key_size    Serial size.
 * @return  ED25519_KEY object.
 * @free_using{free_ed25519_key}
*/
static ED25519_KEY *sgnt_signkey_parse(const unsigned char *serial_key, size_t key_size) {

    ED25519_KEY *key;

    if(!serial_key || !key_size) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    switch(serial_key[0]) {

    case SIGNKEY_DEFAULT_FORMAT:

        if(key_size != ED25519_KEY_SIZE + 1) {
            PUSH_ERROR(ERR_UNSPEC, "invalid signet signing key size or format");
            key = NULL;
            break;
        }

        key = _deserialize_ed25519_pubkey(serial_key + 1);
        break;
    default:
        key = NULL;
        PUSH_ERROR_FMT(ERR_UNSPEC, "unsupported format specifier for signing key: %u", serial_key[0]);
        break;

    }

    if(!key) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not convert signet serial format key to object");
    }

    return key;
}

/**brief    Converts a public signing key object to a signet specified format.
 * @param   key     Ed25519 signing key object.
 * @param   format      Format specifier.
 * @param   key_size    Output serial size.
 * @return  Serial data of the provided signing key in specified format.
 * @free_using{free}
*/
static unsigned char *sgnt_signkey_format(ED25519_KEY *key, unsigned char format, size_t *key_size) {

    unsigned char *serial_key;
    size_t serial_size;

    switch(format) {

    case SIGNKEY_DEFAULT_FORMAT:
        serial_size = ED25519_KEY_SIZE + 1;

        if(!(serial_key = malloc(serial_size))) {
            PUSH_ERROR_SYSCALL("malloc");
            RET_ERROR_PTR(ERR_NOMEM, "could not allocate memory for serialized signing key");
        }

        serial_key[0] = format;
        memcpy(serial_key + 1, key->public_key, ED25519_KEY_SIZE);
        break;
    default:
        RET_ERROR_PTR(ERR_UNSPEC, "unsupported signing key format specifer");

    }

    *key_size = serial_size;

    return serial_key;
}


/**
 * @brief   Retrieves the public signing key from the signet, if the signet is an org signet only retrieves the POK.
 * @param   signet  Pointer to the target signet.
 * @return  Pointer to the target ed25519 public key.
 * @free_using{free_ed25519_key}
*/
static ED25519_KEY *sgnt_signkey_fetch(const signet_t *signet) {

    size_t key_size;
    unsigned char fid, *serial_key;
    ED25519_KEY *key;

    if(!signet) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        fid = SIGNET_ORG_POK;
        break;
    case SIGNET_TYPE_USER:
        fid = SIGNET_USER_SIGN_KEY;
        break;
    case SIGNET_TYPE_SSR:
        fid = SIGNET_SSR_SIGN_KEY;
        break;
    default:
        RET_ERROR_PTR(ERR_UNSPEC, "invalid signet type");
        break;

    }

    if(!(serial_key = sgnt_fid_num_fetch(signet, fid, 1, &key_size))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not retrieve signing key");
    }

    key = sgnt_signkey_parse(serial_key, key_size);
    free(serial_key);

    if(!key) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize signet signing key");
    }

    return key;
}


/**
 * @brief
 *  Retrieves the public encryption key from the signet, if the signet is a
 *  user signet only retrieves the main encryption key (not alternate).
 * @param signet
 *  Pointer to the target signet.
 * @return
 *  Pointer to the target encryption public key.
 * @free_using{free_ec_key}
*/
static EC_KEY *sgnt_enckey_fetch(const signet_t *signet) {

    size_t key_size;
    unsigned char fid, *serial_key;
    EC_KEY *key;

    if(!signet) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        fid = SIGNET_ORG_ENC_KEY;
        break;
    case SIGNET_TYPE_USER:
        fid = SIGNET_USER_ENC_KEY;
        break;
    case SIGNET_TYPE_SSR:
        fid = SIGNET_SSR_ENC_KEY;
        break;
    default:
        RET_ERROR_PTR(ERR_UNSPEC, "invalid signet type");
        break;

    }

    if(!(serial_key = sgnt_fid_num_fetch(signet, fid, 1, &key_size))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not retrieve signing key");
    }

    if(!(key = _deserialize_ec_pubkey(serial_key, key_size, 0))) {
        free(serial_key);
        RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize signing key");
    }

    free(serial_key);

    return key;
}

/**
 * @brief   Retrieves all signing keys from an ORG signet that have a specific set of permissions
 * @param   signet  Pointer to target organizational signet.
 * @param   perm    Permissions by which the keys are filtered.
 * @return  A NULL pointer terminated array of ed25519 public signing key objects.
 * @note    Always returns at least POK.
 * @free_using{free_ed25519_key_chain}
*/
static ED25519_KEY **sgnt_signkey_fetch_by_perm(const signet_t *signet, sok_permissions_t perm) {

    int res;
    ED25519_KEY **keys;
    signet_field_t *field, *list = NULL;
    size_t buflen, list_count = 1, key_count = 1;
    unsigned char bin_perm;
    unsigned int num_keys = 1;

    if(!signet) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(sgnt_type_get(signet) != SIGNET_TYPE_ORG) {
        RET_ERROR_PTR(ERR_UNSPEC, "target must be an organizational signet");
    }

    if(sgnt_validate_structure(signet) < SS_CRYPTO) {
        RET_ERROR_PTR(ERR_UNSPEC, "signet structure is not valid for signing key retrieval");
    }

    bin_perm = (unsigned char) perm;

    if((res = sgnt_fid_exists(signet, SIGNET_ORG_SOK))) {

        if(res < 0) {
            RET_ERROR_PTR(ERR_UNSPEC, "error checking for existence of SOKs in signet");
        }

        if(!(field = list = sgnt_fieldlist_create(signet, SIGNET_ORG_SOK))) {
            RET_ERROR_PTR(ERR_UNSPEC, "error creating field list of signet SOKs");
        }

        while(field) {
            // select the keys that have AT LEAST all the specified permissions
            if(!((signet->data[field->data_offset] ^ bin_perm) & bin_perm)) {
                ++num_keys;
            }

            field = (signet_field_t *)field->next;
        }

    }

    buflen = sizeof(ED25519_KEY *) * (num_keys + 1);

    if(!(keys = malloc(buflen))) {
        PUSH_ERROR_SYSCALL("malloc");
        sgnt_fieldlist_destroy(list);
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate memory for array of keys");
    }

    memset(keys, 0, buflen);
    field = list;

    if(!(keys[0] = sgnt_signkey_fetch(signet))) {
        _free_ed25519_key_chain(keys);
        sgnt_fieldlist_destroy(list);
        RET_ERROR_PTR(ERR_UNSPEC, "could not fetch signet signing key");
    }

    while(field) {
        // select the keys that have AT LEAST all the specified permissions
        if(!((signet->data[field->data_offset] ^ bin_perm) & bin_perm)) {

            if(!(keys[key_count] = sgnt_sok_num_fetch(signet, list_count))) {
                _free_ed25519_key_chain(keys);
                sgnt_fieldlist_destroy(list);
                RET_ERROR_PTR(ERR_UNSPEC, "could not fetch signet sok");
            }

            ++key_count;
        }

        ++list_count;
        field = (signet_field_t *)field->next;
    }

    if(list) {
        sgnt_fieldlist_destroy(list);
    }

    return keys;
}


/**
 * @brief   Retrieves all the signing keys from an org signet that can be used to sign a message.
 * @param   signet  Pointer to target organizational signet.
 * @return  A NULL pointer terminated array of ed25519 public signing key objects.
 * @note    Always returns at least POK.
 * @free_using{free_ed25519_key_chain}
*/
static ED25519_KEY **sgnt_signkeys_msg_fetch(const signet_t *signet) {

    ED25519_KEY **res;

    if(!signet) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!(res = sgnt_signkey_fetch_by_perm(signet, SIGNET_SOK_MSG))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not fetch list of message signing keys");
    }

    return res;
}


/**
 * @brief   Retrieves all the signing keys from an org signet that can be used to sign a signet.
 * @param   signet  Pointer to target organizational signet.
 * @return  A NULL pointer terminated array of ed25519 public signing key objects.
 * @note    Always returns at least POK.
 * @free_using{free_ed25519_key_chain}
*/
static ED25519_KEY **sgnt_signkeys_signet_fetch(const signet_t *signet) {

    ED25519_KEY **res;

    if(!signet) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!(res = sgnt_signkey_fetch_by_perm(signet, SIGNET_SOK_SIGNET))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not fetch list of message signing keys");
    }

    return res;
}


/**
 * @brief   Retrieves all the signing keys from an org signet that can be used to sign a TLS certificate.
 * @param   signet  Pointer to target organizational signet.
 * @return  A NULL pointer terminated array of ed25519 public signing key objects.
 * @note    Always returns at least POK.
 * @free_using{free_ed25519_key_chain}
*/
static ED25519_KEY **sgnt_signkeys_tls_fetch(const signet_t *signet) {

    ED25519_KEY **res;

    if(!signet) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!(res = sgnt_signkey_fetch_by_perm(signet, SIGNET_SOK_TLS))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not fetch list of message signing keys");
    }

    return res;
}


/**
 * @brief   Retrieves all the signing keys from an org signet that can be used to sign software.
 * @param   signet  Pointer to target organizational signet.
 * @return  A NULL pointer terminated array of ed25519 public signing key objects.
 * @note    Always returns at least POK.
 * @free_using{free_ed25519_key_chain}
*/
static ED25519_KEY **sgnt_signkeys_software_fetch(const signet_t *signet) {

    ED25519_KEY **res;

    if(!signet) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!(res = sgnt_signkey_fetch_by_perm(signet, SIGNET_SOK_SOFTWARE))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not fetch list of message signing keys");
    }

    return res;
}


/**
 * @brief   Fetch the secondary organizational signing key from the signet by number (starting at 1)
 * @param   signet  Pointer to the target organizational signet.
 * @param   num The sok number to be fetched.
 * @return  Retrieved ED25519 key.
 * @free_using{free_ed25519_key}
*/
static ED25519_KEY *sgnt_sok_num_fetch(const signet_t *signet, unsigned int num) {

    ED25519_KEY *key;
    size_t key_size;
    unsigned char *serial_key;

    if(!signet || !num) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(sgnt_type_get(signet) != SIGNET_TYPE_ORG) {
        RET_ERROR_PTR(ERR_UNSPEC, "only organizational signets can have SOKs");
    }

    if((unsigned int)sgnt_fid_count_get(signet, SIGNET_ORG_SOK) < num) {
        RET_ERROR_PTR(ERR_UNSPEC, "there are less SOKs than the specified number");
    }

    if(!(serial_key = sgnt_fid_num_fetch(signet, SIGNET_ORG_SOK, num, &key_size))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not retrieve signing key");
    }

    key = sgnt_signkey_parse(serial_key + 1, key_size - 1);
    free(serial_key);

    if(!key) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize signet signing key");
    }

    return key;
}


/* Modifying the signet */

/**
 * @brief   Adds a field to the target field.
 * @param   signet      Pointer to the target signet.
 * @param   fid     Field id of the field to be added.
 * @param   data_size   Size of the array containing the field data.
 * @param   data        Field data.
 * @return  0 on success, -1 on failure.
*/
static int sgnt_field_defined_create(signet_t *signet, unsigned char fid, size_t data_size, const unsigned char *data) {

    int res;
    size_t at = 0, field_size = 1;
    signet_field_key_t *keys;
    uint32_t maxsize = 0;
    unsigned char *field_data;
    unsigned int offset;

    if(!signet || (!data && data_size) || (data && !data_size)) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        keys = signet_org_field_keys;

        if(fid == SIGNET_ORG_UNDEFINED) {
            RET_ERROR_INT(ERR_UNSPEC, "incorrect function for undefined field creation");
        }

        break;
    case SIGNET_TYPE_USER:
        keys = signet_user_field_keys;

        if(fid == SIGNET_USER_UNDEFINED) {
            RET_ERROR_INT(ERR_UNSPEC, "incorrect function for undefined field creation");
        }

        break;
    case SIGNET_TYPE_SSR:
        keys = signet_ssr_field_keys;
        break;
    default:
        RET_ERROR_INT(ERR_UNSPEC, "invalid signet type");
        break;

    }

    switch(keys[fid].bytes_data_size) {

    case 0:

        if(data_size != keys[fid].data_size) {
            RET_ERROR_INT(ERR_BAD_PARAM, "signet field data size did not match required field data size");
        }

        field_size += keys[fid].data_size;
        break;
    case 1:
        maxsize = UNSIGNED_MAX_1_BYTE;
        break;
    case 2:
        maxsize = UNSIGNED_MAX_2_BYTE;
        break;
    case 3:
        maxsize = UNSIGNED_MAX_3_BYTE;
        break;
    default:
        RET_ERROR_INT(ERR_UNSPEC, "inalid signet field data size");
        break;

    }

    if (maxsize) {

        if (data_size > maxsize) {
            RET_ERROR_INT(ERR_BAD_PARAM, "the specified data size is too large for the field type");
        }

        field_size += keys[fid].bytes_data_size + data_size;
    }

    if(!(field_data = malloc(field_size))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_INT(ERR_NOMEM, NULL);
    }

    memset(field_data, 0, field_size);
    field_data[at++] = fid;

    switch(keys[fid].bytes_data_size) {

    case 1:
        field_data[at] = (unsigned char)data_size;
        break;
    case 2:
        _int_no_put_2b(field_data + at, (uint16_t)data_size);
        break;
    case 3:
        _int_no_put_3b(field_data + at, (uint32_t)data_size);
        break;
    default:
        break;

    }

    at += keys[fid].bytes_data_size;

    if (data != NULL) {
        memcpy(field_data + at, data, data_size);
    }

    offset = (unsigned int)signet->size;

    for(int i = fid + 1; i < SIGNET_FID_MAX; ++i) {

        if((res = sgnt_fid_exists(signet, i))) {

            if(res < 0) {
                RET_ERROR_INT(ERR_UNSPEC, "error searching for field in signet");
            }

            offset = (unsigned int)signet->fields[i] - 1;
            break;
        }
    }

    res = sgnt_field_create_at(signet, offset, field_size, (const unsigned char *)field_data);
    free(field_data);

    if(res) {
        RET_ERROR_INT(ERR_UNSPEC, "error inserting field data into signet");
    }

    if((res = sgnt_fid_exists(signet, fid)) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "error searching for field in signet");
    } else if(!res) {
        signet->fields[fid] = offset + 1;
    }

    for(int i = fid + 1; i <= SIGNET_FID_MAX; ++i) {

        if((res = sgnt_fid_exists(signet, i))) {

            if(res < 0) {
                RET_ERROR_INT(ERR_UNSPEC, "error searching for field in signet");
            }

            signet->fields[i] += field_size;
        }
    }

    return 0;
}


/**
 * @brief   Adds a SOK (Secondary Organizational Signing Key) to an organizational signet.
 * @param   signet      Pointer to the target org signet.
 * @param   key     ED25519 key to be added as a SOK to the signet.
 * @param   format      Format specifier byte dictating the format.
 * @param   perm        Permissions for the usage of the SOK.
 * @return  0 on success, -1 on failure.
*/
static int sgnt_sok_create(signet_t *signet, ED25519_KEY *key, unsigned char format, uint8_t perm) {

    int res;
    unsigned char *serial_key, *serial_field;
    size_t serial_size;

    if(!signet || !key) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if(sgnt_type_get(signet) != SIGNET_TYPE_ORG) {
        RET_ERROR_INT(ERR_UNSPEC, "only organizational signets have a secondary organizational key field type");
    }

    if(!(serial_key = sgnt_signkey_format(key, format, &serial_size))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not serialize ed25519 public key to specified signet format");
    }

    if(!(serial_field = malloc(serial_size + 1))) {
        free(serial_key);
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_INT(ERR_NOMEM, "could not reallocate memory for signet serial SOK representation");
    }

    memcpy(serial_field + 1, serial_key, serial_size);
    free(serial_key);
    serial_field[0] = (unsigned char) perm;
    ++serial_size;
    res = sgnt_field_defined_create(signet, SIGNET_ORG_SOK, serial_size, serial_field);
    free(serial_field);

    if(res) {
        RET_ERROR_INT(ERR_UNSPEC, "could not add SOK to signet");
    }

    return 0;
}


/**
 * @brief   Adds an undefined field to signet with specified name and data.
 * @param   signet      Pointer to the target signet to which the field is added.
 * @param   name_size   Size of field name.
 * @param   name        Pointer to  field name.
 * @param   data_size   Size of field data.
 * @param   data        Pointer to field data.
 * @return  0 on success, -1 on failure.
*/
static int sgnt_field_undefined_create(signet_t *signet, size_t name_size, const unsigned char *name, size_t data_size, const unsigned char *data) {

    int res;
    size_t at = 0, field_size = 1;
    unsigned char *field_data, fid;
    unsigned int offset;

    if(!signet || (!data && data_size) || (data && !data_size) || !name || !name_size) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        fid = SIGNET_ORG_UNDEFINED;
        break;
    case SIGNET_TYPE_USER:
        fid = SIGNET_USER_UNDEFINED;
        break;
    case SIGNET_TYPE_SSR:
        RET_ERROR_INT(ERR_UNSPEC, "signet signing request has no allowed undefined fields");
        break;
    default:
        RET_ERROR_INT(ERR_UNSPEC, "invalid signet type");
        break;

    }

    if(name_size > UNSIGNED_MAX_1_BYTE || data_size > UNSIGNED_MAX_2_BYTE) {
        RET_ERROR_INT(ERR_UNSPEC, "invalid name or data size for undefined field");
    }

    field_size += 1 + name_size + 2 + data_size;

    if(!(field_data = malloc(field_size))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_INT(ERR_NOMEM, NULL);
    }

    memset(field_data, 0, field_size);
    field_data[at++] = fid;
    field_data[at++] = (unsigned char)name_size;
    memcpy(field_data + at, name, name_size);
    at += name_size;
    _int_no_put_2b(field_data + at, (uint16_t)data_size);
    at += 2;
    memcpy(field_data + at, data, data_size);

    offset = (unsigned int)signet->size;

    for(int i = fid + 1; i < SIGNET_FID_MAX; ++i) {

        if((res = sgnt_fid_exists(signet, i))) {

            if(res < 0) {
                RET_ERROR_INT(ERR_UNSPEC, "error searching for field in signet");
            }

            offset = (unsigned int)signet->fields[i] - 1;
            break;
        }
    }

    res = sgnt_field_create_at(signet, offset, field_size, (const unsigned char *)field_data);
    free(field_data);

    if(res) {
        RET_ERROR_INT(ERR_UNSPEC, "error inserting field data into signet");
    }

    if((res = sgnt_fid_exists(signet, fid)) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "error searching for field in signet");
    } else if(!res) {
        signet->fields[fid] = offset + 1;
    }

    for(int i = fid + 1; i <= SIGNET_FID_MAX; ++i) {

        if((res = sgnt_fid_exists(signet, i))) {

            if(res < 0) {
                RET_ERROR_INT(ERR_UNSPEC, "error searching for field in signet");
            }

            signet->fields[i] += field_size;
        }
    }

    return 0;
}


/**
 * @brief   Removes the field specified by a field id and the number in which it appears in the target signet amongst fields with the same field id from the target signet.
 * @param   signet  Pointer to the target signet.
 * @param   fid Field id of the field to be removed.
 * @param   num The number in which the field to be removed appears amongst other fields with the same field id in the target signet, (1, 2, ...).
 * @return  Number of fields with specified id on success, -1 on failure.
*/
static int sgnt_fid_num_remove(signet_t *signet, unsigned char fid, int num) {

    int num_fields, res;
    int field_size;
    unsigned int offset;
    signet_field_t *field, *temp;

    if(!signet) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if((res = sgnt_fid_exists(signet, fid)) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "error searching for field in signet");
    } else if(!res) {
        RET_ERROR_INT(ERR_UNSPEC, "field not found in signet");
    }

    if((num_fields = sgnt_fid_count_get(signet, fid)) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "could not calculate signet field count with specified field id");
    }

    if(num_fields < num) {
        RET_ERROR_INT(ERR_UNSPEC, "signet field index exceeds field count");
    }

    if(!(field = sgnt_fieldlist_create(signet, fid))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not retrieve fields from signet");
    }

    temp = field;

    for(int i = 1; i < num; ++i) {

        if(!temp) {
            sgnt_fieldlist_destroy(field);
            RET_ERROR_INT(ERR_UNSPEC, "signet field index does not exist");
        }

        temp = (signet_field_t *)temp->next;
    }

    offset = temp->id_offset;

    if((field_size = sgnt_field_size_serial_get((const signet_field_t *)temp)) < 0) {
        sgnt_fieldlist_destroy(field);
        RET_ERROR_INT(ERR_UNSPEC, "could not calculate signet field size");
    }

    sgnt_fieldlist_destroy(field);

    if(num_fields == 1) {
        signet->fields[fid] = 0;
    }

    for(int i = fid + 1; i <= SIGNET_FID_MAX; ++i) {

        if((res = sgnt_fid_exists(signet, i))) {

            if(res < 0) {
                RET_ERROR_INT(ERR_UNSPEC, "error searching for field in signet");
            }

            signet->fields[i] -= field_size;
        }
    }

    if(sgnt_field_remove_at(signet, offset, field_size) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "could not remove specified field from signet");
    }

    return 0;
}


/**
 * @brief   Removes an undefined field from the target signet by name.
 * @param   signet      Pointer to the target signet.
 * @param   name_size   Size of field name.
 * @param   name        Name of the field to be removed.
 * @return  0 on success, -1 on failure.
*/
static int sgnt_field_undefined_remove(signet_t *signet, size_t name_size, const unsigned char *name) {

    int num_fields, res;
    size_t field_size;
    unsigned char fid;
    unsigned int offset;
    signet_field_t *field, *temp;

    if(!signet) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        fid = SIGNET_ORG_UNDEFINED;
        break;
    case SIGNET_TYPE_USER:
        fid = SIGNET_USER_UNDEFINED;
        break;
    default:
        RET_ERROR_INT(ERR_UNSPEC, "invalid signet type");
        break;

    }

    if((res = sgnt_fid_exists(signet, fid)) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "error searching for field in signet");
    } else if(!res) {
        RET_ERROR_INT(ERR_UNSPEC, "field id not found in signet");
    }

    if((num_fields = sgnt_fid_count_get(signet, fid)) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "could not calculate field count for signet");
    }

    if(!(field = sgnt_fieldlist_create(signet, fid))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not retrieve signet fields with specified field id");
    }

    temp = field;

    while(temp) {

        if(temp->name_size == (unsigned char)name_size) {

            if(memcmp(temp->signet->data + temp->name_offset, name, name_size) == 0) {
                offset = temp->id_offset;
                field_size = sgnt_field_size_serial_get((const signet_field_t *)temp);
                break;
            }
        }
    }

    sgnt_fieldlist_destroy(field);

    if(num_fields == 1) {
        signet->fields[fid] = 0;
    }

    for(int i = fid + 1; i <= SIGNET_FID_MAX; ++i) {

        if((res = sgnt_fid_exists(signet, i))) {

            if(res < 0) {
                RET_ERROR_INT(ERR_UNSPEC, "error searching for field in signet");
            }

            signet->fields[i] -= field_size;
        }
    }

    if(sgnt_field_remove_at(signet, offset, field_size) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "could not remove specified field from signet");
    }

    return 0;
}


/**
 * @brief   Replaces all fields in the target signet with the specified field id with a new field specified by the parameters.
 * @param   signet      Pointer to the target signet_t structure.
 * @param   fid         Field id which specifies the fields to be replaced with the new field.
 * @param   data_size   Size of field data array.
 * @param   data        Array contaning field data.
 * @return  0 on success, -1 on failure.
*/
static int sgnt_field_defined_set(signet_t *signet, unsigned char fid, size_t data_size, const unsigned char *data) {

    int result;

    if(!signet) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    while((result = sgnt_fid_exists(signet, fid))) {

        if(result < 0) {
            RET_ERROR_INT(ERR_UNSPEC, "error searching for field in signet");
        }

        if(sgnt_fid_num_remove(signet, fid, 1) < 0) {
            RET_ERROR_INT(ERR_UNSPEC, "could not remove old signet field value");
        }
    }

    if(sgnt_field_defined_create(signet, fid, data_size, data) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "could not add field to signet");
    }

    return 0;
}


/**
 * @brief   Sets the ID of the signet to the specified NULL terminated string.
 * @param   signet  Pointer to the target signet.
 * @param   id_size Size of signet id.
 * @param   id      Signet id.
 * @return  0 on success, -1 on failure.
*/
static int sgnt_id_set(signet_t *signet, size_t id_size, const unsigned char *id) {

    int result;
    unsigned char fid;

    if(!signet || !id || !id_size) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        fid = SIGNET_ORG_ID;
        break;
    case SIGNET_TYPE_USER:
        fid = SIGNET_USER_ID;
        break;
    case SIGNET_TYPE_SSR:
        RET_ERROR_INT(ERR_UNSPEC, "SSR does not have an id field");
        break;
    default:
        RET_ERROR_INT(ERR_UNSPEC, "invalid signet type");
        break;

    }

    if((result = sgnt_field_defined_set(signet, fid, id_size, id)) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "could not set signet id field");
    }

    return 0;
}


/**
 * @brief   Sets the public encryption key (non-alterante encryption key) for the signet.
 * @param   signet  Target signet.
 * @param   key Public encryption key.
 * @param   format  Format specifier. TODO currently unused! (spec requires 0x04 but openssl natively serializes it to 0x02).
 * @return  0 on success, -1 on failure.
*/
static int sgnt_enckey_set(signet_t *signet, EC_KEY *key, unsigned char format) {

    int res;
    unsigned char *serial_key = NULL, fid;
    size_t serial_size;

    if(!signet || !key) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        fid = SIGNET_ORG_ENC_KEY;
        break;
    case SIGNET_TYPE_USER:
        fid = SIGNET_USER_ENC_KEY;
        break;
    case SIGNET_TYPE_SSR:
        fid = SIGNET_SSR_ENC_KEY;
        break;
    default:
        RET_ERROR_INT(ERR_UNSPEC, "invalid signet type");

    }

    switch (format) { //TODO to avoid format being unused:

    default:
        serial_key = _serialize_ec_pubkey(key, &serial_size);
        break;

    }

    if(!serial_key) {
        RET_ERROR_INT(ERR_UNSPEC, "failed serialiation of public EC encryption key");
    }

    res = sgnt_field_defined_set(signet, fid, serial_size, serial_key);
    free(serial_key);

    if(res < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "could not set signet encryption key");
    }

    return 0;
}


/**
 * @brief   Sets the signing key (pok - primary signing key in case of an org signet).
 * @param   signet  Pointer to the target signet.
 * @param   key Public signing key to be set as the signing key of the signet.
 * @param   format  Format specifier byte, dictating the format.
 * @return  0 on success, -1 on failure.
*/
static int sgnt_signkey_set(signet_t *signet, ED25519_KEY *key, unsigned char format) {

    int res;
    unsigned char *serial_key, fid;
    size_t serial_size;

    if(!signet || !key) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        fid = SIGNET_ORG_POK;
        break;
    case SIGNET_TYPE_USER:
        fid = SIGNET_USER_SIGN_KEY;
        break;
    case SIGNET_TYPE_SSR:
        fid = SIGNET_SSR_SIGN_KEY;
        break;
    default:
        RET_ERROR_INT(ERR_UNSPEC, "invalid signet type");

    }

    if(!(serial_key = sgnt_signkey_format(key, format, &serial_size))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not serialize provided public ed25519 key to signet format");
    }

    res = sgnt_field_defined_set(signet, fid, serial_size, serial_key);
    free(serial_key);

    if(res < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "could not set signet signing key");
    }

    return 0;
}


/**
 * @brief   Sets the target signet to a specified type.
 * @param   signet  Pointer to the target signet.
 * @param   type    Specified signet type.
 * @return  0 on success, -1 on error.
*/
static int sgnt_type_set(signet_t *signet, signet_type_t type) {

    if(!signet) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    } else if(type != SIGNET_TYPE_ORG && type != SIGNET_TYPE_USER && type != SIGNET_TYPE_SSR) {
        RET_ERROR_INT(ERR_BAD_PARAM, "unsupported signet type");
    }

    signet->type = type;

    return 0;
}


/* signet splits */

/**
 * @brief   Strips off all fields from the signet past the specified one and updates the header.
 * @param   signet  Pointer to target signet.
 * @param   fid The last field id to not be stripped.
 * @return  Pointer to a fresh allocated signet.
 * @free_using{sgnt_destroy_signet}
*/
static signet_t *sgnt_signet_split(const signet_t *signet, unsigned char fid) {

    unsigned char *data = NULL, *split;
    size_t data_size, split_size;
    dime_number_t number;
    signet_t *split_signet;

    if(!signet) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        number = DIME_ORG_SIGNET;
        break;
    case SIGNET_TYPE_USER:
        number = DIME_USER_SIGNET;
        break;
    default:
        RET_ERROR_PTR(ERR_UNSPEC, "unsupported signet type");
        break;

    }

    if(!(data = sgnt_signet_serialize_upto_fid(signet, fid, &data_size))) {

        if(get_last_error()) {
            RET_ERROR_PTR(ERR_UNSPEC, "could not serialize specified signet fields");
        }

    }

    split_size = data_size + SIGNET_HEADER_SIZE;

    if(!(split = malloc(split_size))) {
        PUSH_ERROR_SYSCALL("malloc");

        if (data) {
            free(data);
        }

        RET_ERROR_PTR(ERR_NOMEM, NULL);
    }

    memset(split, 0, split_size);
    _int_no_put_2b(split, (uint16_t)number);
    _int_no_put_3b(split + 2, (uint32_t)data_size);

    if(data) {
        memcpy(split + SIGNET_HEADER_SIZE, data, data_size);
        free(data);
    }

    split_signet = sgnt_signet_binary_deserialize(split, split_size);
    free(split);

    if(!split_signet) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize split signet");
    }

    return split_signet;
}


/**
 * @brief   Creates a copy of the target signet with the ID field and the FULL signature stripped off.
 * @param   signet  Pointer to the target signet.
 * @return  Pointer to a stripped signet on success, NULL on failure.
 * @free_using{sgnt_destroy_signet}
*/
static signet_t *sgnt_signet_full_split(const signet_t *signet) {

    unsigned char fid;
    signet_t *split_signet;

    if(!signet) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        fid = SIGNET_ORG_FULL_SIG;
        break;
    case SIGNET_TYPE_USER:
        fid = SIGNET_USER_FULL_SIG;
        break;
    default:
        RET_ERROR_PTR(ERR_UNSPEC, "unsupported signet type");
        break;

    }

    if(!(split_signet = sgnt_signet_split(signet, fid))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize split signet");
    }

    return split_signet;
}


/**
 * @brief   Creates a copy of the target user signet with all fields beyond the INITIAL signature stripped off.
 * @param   signet  Pointer to the target signet.
 * @return  Pointer to a stripped signet on success, NULL on failure.
 * @free_using{sgnt_destroy_signet}
*/
static signet_t *sgnt_signet_crypto_split(const signet_t *signet) {

    unsigned char fid;
    signet_t *split_signet;

    if(!signet) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        fid = SIGNET_ORG_CRYPTO_SIG;
        break;
    case SIGNET_TYPE_USER:
        fid = SIGNET_USER_CRYPTO_SIG;
        break;
    default:
        RET_ERROR_PTR(ERR_UNSPEC, "unsupported signet type");
        break;

    }

    if(!(split_signet = sgnt_signet_split(signet, fid))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not deserialize split signet");
    }

    return split_signet;
}


/* Signet Fingerprints */

/**
 * @brief   Takes a SHA512 fingerprint of the entire user or org signet.
 * @param   signet  Pointer to the target signet.
 * @return  Allocated NULL terminated buffer to a base64 encoded unpadded fingerprint. Null on failure;
 * @free_using{free}
*/
static char *sgnt_fingerprint_id(const signet_t *signet) {

    char *b64_fingerprint;

    if(!signet) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(sgnt_type_get(signet) == SIGNET_TYPE_SSR) {
        RET_ERROR_PTR(ERR_UNSPEC, "unsupported signet type");
    }

    if(!(b64_fingerprint = sgnt_fingerprint_upto_fid(signet, SIGNET_FID_MAX))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not base-64 encode full signet fingerprint");
    }

    return b64_fingerprint;
}


/**
 * @brief   Takes a SHA512 fingerprint of the user or org signet with the ID and FULL signature fields stripped off.
 * @note    To take an SSR fingerprint, use the signet_ssr_fingerprint() function.
 * @param   signet  Pointer to the target signet.
 * @return  Allocated NULL terminated buffer to a base64 encoded unpadded fingerprint. Null on failure.
 * @free_using{free}
*/
static char *sgnt_fingerprint_full(const signet_t *signet) {

    char *b64_fingerprint;
    unsigned char fid;

    if(!signet) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        fid = SIGNET_ORG_FULL_SIG;
        break;
    case SIGNET_TYPE_USER:
        fid = SIGNET_USER_FULL_SIG;
        break;
    default:
        RET_ERROR_PTR(ERR_UNSPEC, "unsupported signet type");
        break;

    }

    if(!(b64_fingerprint = sgnt_fingerprint_upto_fid(signet, fid))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not base-64 encode full signet fingerprint");
    }

    return b64_fingerprint;
}


/**
 * @brief   Takes a SHA512 fingerprint of a signet with all fields after the cryptographic signature field stripped off.
 * @note    To take an SSR fingerprint, use the signet_ssr_fingerprint() function.
 * @param   signet  Pointer to the target signet.
 * @return  Allocated NULL terminated string to a base64 encoded unpadded fingerprint. Null on error.
 * @free_using{free}
*/
static char *sgnt_fingerprint_crypto(const signet_t *signet) {

    char *b64_fingerprint;
    unsigned char fid;

    if(!signet) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        fid = SIGNET_ORG_CRYPTO_SIG;
        break;
    case SIGNET_TYPE_USER:
        fid = SIGNET_USER_CRYPTO_SIG;
        break;
    default:
        RET_ERROR_PTR(ERR_UNSPEC, "unsupported signet type for cryptographic fingerprint");

    }

    if(!(b64_fingerprint = sgnt_fingerprint_upto_fid(signet, fid))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not base-64 encode full signet fingerprint");
    }

    return b64_fingerprint;
}


/**
 * @brief   Takes a SHA512 fingerprint of a user signet or an ssr with all fields after the SSR signature stripped off.
 * @param   signet  Pointer to the target signet.
 * @return  Allocated NULL terminated buffer to a base64 encoded unpadded fingerprint.
 * @free_using{free}
*/
static char *sgnt_fingerprint_ssr(const signet_t *signet) {

    char *b64_fingerprint;
    signet_type_t type;

    if(!signet) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if ((type = sgnt_type_get(signet)) == SIGNET_TYPE_ERROR) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not determine signet type");
    } else if(type != SIGNET_TYPE_USER && type != SIGNET_TYPE_SSR) {
        RET_ERROR_PTR(ERR_UNSPEC, "invalid signet type");
    }

    if(!(b64_fingerprint = sgnt_fingerprint_upto_fid(signet, SIGNET_USER_SSR_SIG))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not base-64 encode full signet fingerprint");
    }

    return b64_fingerprint;
}


/* Signet verification */

/**
 * @brief   Verifies a user signet, org signet or ssr for syntax, context and cryptographic validity. Does NOT perform chain of custody validation.
 * @param   signet      Pointer to the target signet_t structure.
 * @param   orgsig      Pointer to the org signet associated with the target signet IF the target signet is a user signet.
 *                              If target signet is not a user signet, orgsig should be passed as NULL.
 * @param   previous    Pointer to the previous user signet, which will be used validate the chain of custody signature, if such is available.
 * @param   dime_pok    A NULL terminated array of pointers to ed25519 POKs from the dime record associated with the target signet if the target signet is an org signet.
 *                              If the target signet is not an org signet dime_pok should be passed as NULL;
 * @return  Signet state as a signet_state_t enum type. SS_UNKNOWN on error.
*/
static signet_state_t  sgnt_validate_all(const signet_t *signet, const signet_t *previous, const signet_t *orgsig, const unsigned char **dime_pok) {

    ED25519_KEY **org_keys, *user_key, *prev_key;
    int res, res2, res3, pok_num;
    const char *errmsg = NULL;
    signet_state_t signet_state, result = SS_ID;
    signet_type_t type;

    if(!signet) {
        RET_ERROR_CUST(SS_UNKNOWN, ERR_BAD_PARAM, NULL);
    }

    signet_state = sgnt_validate_structure(signet);

    if(signet_state <= SS_INVALID) {
        return signet_state;
    }

    type = sgnt_type_get(signet);

    if(type == SIGNET_TYPE_SSR) {

        if(!(user_key = sgnt_signkey_fetch(signet))) {
            RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "could not retrieve signing key");
        }

        res = sgnt_validate_sig_field_key(signet, SIGNET_SSR_SSR_SIG, user_key);
        _free_ed25519_key(user_key);

        if(res < 0) {
            RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "error during signature field validation");
        } else if(!res) {
            return SS_INVALID;
        }

        if((res = sgnt_fid_exists(signet, SIGNET_SSR_COC_SIG))) {

            if(res < 0) {
                RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "error while checking existence of coc signature");
            }

            if(!previous) {
                return SS_BROKEN_COC;
            } else {

                if(!(prev_key = sgnt_signkey_fetch(previous))) {
                    RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "error while retrieving signet signing key");
                }

                res = sgnt_validate_sig_field_key(signet, SIGNET_USER_SSR_SIG, prev_key);
                _free_ed25519_key(prev_key);

                if(res < 0) {
                    RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "error during signature field validation");
                } else if(!res) {
                    return SS_BROKEN_COC;
                }
            }
        } else if(!res && previous) {
            return SS_BROKEN_COC;
        }

        return SS_SSR;
    } else if(type == SIGNET_TYPE_ORG) {

        if(!dime_pok) {
            RET_ERROR_CUST(SS_UNKNOWN, ERR_BAD_PARAM, NULL);
        }

        if(signet_state <= SS_SSR) {
            RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "invalid state for organizational signet");
        }

        pok_num = sgnt_validate_pok(signet, dime_pok);

        if(pok_num < 0) {
            RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "error matching signet POK with DIME management record");
        }

        if(!pok_num) {
            RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "no DIME record POKs match the signet signing key");
        }

        pok_num -= 1;

        if((res = sgnt_validate_sig_field(signet, SIGNET_ORG_CRYPTO_SIG, dime_pok[pok_num])) == 1) {

            if(signet_state == SS_CRYPTO) {
                result = SS_CRYPTO;
            } else if((res2 = sgnt_validate_sig_field(signet, SIGNET_ORG_FULL_SIG, dime_pok[pok_num])) == 1) {

                if(signet_state == SS_FULL) {
                    result = SS_FULL;
                } else if((res3 = sgnt_validate_sig_field(signet, SIGNET_ORG_ID_SIG, dime_pok[pok_num])) < 0) {
                    result = SS_UNKNOWN;
                    errmsg = "encountered error during id signature field validation";
                } else if(!res3) {
                    result = SS_INVALID;
                }

            } else if(res2 < 0) {
                result = SS_UNKNOWN;
                errmsg = "encountered error during full signature field validation";
            } else {
                result = SS_INVALID;
            }

        } else if(res < 0) {
            result = SS_UNKNOWN;
            errmsg = "encountered error during crypto signature field validation";
        } else {
            result = SS_INVALID;
        }

    } else if(type == SIGNET_TYPE_USER) {

        if(!orgsig) {
            RET_ERROR_CUST(SS_UNKNOWN, ERR_BAD_PARAM, NULL);
        }

        if(signet_state <= SS_SSR) {
            RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "invalid state for user signet");
        }

        if(sgnt_type_get(orgsig) != SIGNET_TYPE_ORG) {
            RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "the signet passed to verify the user signet was not an org signet");
        }

        if(!(org_keys = sgnt_signkeys_signet_fetch(orgsig))) {
            RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "could not retrieve signing keys from organizational signet");
        }

        if((res = sgnt_validate_sig_field_multikey(signet, SIGNET_USER_CRYPTO_SIG, org_keys)) == 1) {

            if(signet_state == SS_CRYPTO) {
                result = SS_CRYPTO;
            } else if((res2 = sgnt_validate_sig_field_multikey(signet, SIGNET_USER_FULL_SIG, org_keys)) == 1) {

                if(signet_state == SS_FULL) {
                    result = SS_FULL;
                } else if ((res3 = sgnt_validate_sig_field_multikey(signet, SIGNET_USER_ID_SIG, org_keys)) < 0) {
                    result = SS_UNKNOWN;
                    errmsg = "encountered error during id signature field validation";
                } else if(!res3) {
                    result = SS_INVALID;
                }

            } else if (res2 < 0) {
                result = SS_UNKNOWN;
                errmsg = "encountered error during full signature field validation";
            } else {
                result = SS_INVALID;
            }

        } else if (res < 0) {
            result = SS_UNKNOWN;
            errmsg = "encountered error during crypto signature field validation";
        } else {
            result = SS_INVALID;
        }

        _free_ed25519_key_chain(org_keys);

        if(result >= SS_CRYPTO && (res = sgnt_fid_exists(signet, SIGNET_USER_COC_SIG))) {

            if(res < 0) {
                RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "error while checking existence of coc signature");
            }

            if(!previous || sgnt_type_get(previous) != SIGNET_TYPE_USER) {
                return SS_BROKEN_COC;
            } else {

                if(!(prev_key = sgnt_signkey_fetch(previous))) {
                    RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "error while retrieving previous signet public signing key");
                }

                res = sgnt_validate_sig_field_key(signet, SIGNET_USER_COC_SIG, prev_key);
                free(prev_key);

                if(res < 0) {
                    RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "encountered error during chain of custody signature field validation");
                } else if(!res) {
                    return SS_BROKEN_COC;
                }
            }
        } else if(!res && previous) {
            return SS_BROKEN_COC;
        }

    } else {
        RET_ERROR_CUST(SS_UNKNOWN, ERR_UNSPEC, "invalid signet type");
    }

    if(result == SS_UNKNOWN) {
        RET_ERROR_CUST(result, ERR_UNSPEC, errmsg);
    }

    return result;
}


/**
 * @brief   Verifies a specified signet signature using the key passed to the function. Assumes that both key and signature are ed25519.
 * @param   signet  Pointer to the target signet.
 * @param   sig_fid The field id of the field which contains the signature intended for verification.
 * @param   key Array containing the public ed25519 signing key used to verify the signature.
 * @return  1 if signature verification was successful, 0 if verification failed. -1 if error occurred.
*/
static int sgnt_validate_sig_field(const signet_t *signet, unsigned char sig_fid, const unsigned char *key) {

    int res;
    ED25519_KEY *pub_key;

    if(!signet || !key) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if(!(pub_key = _deserialize_ed25519_pubkey(key))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not deserialize user signing key");
    }

    res = sgnt_validate_sig_field_key(signet, sig_fid, pub_key);
    _free_ed25519_key(pub_key);

    if(res < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "error verifying signet signature field");
    }

    return res;
}


/**
 * @brief   Verifies a specified signet signature using the ed25519 key structure passed to the function.
 * @param   signet  Pointer to the target signet.
 * @param   sig_fid The field id of the field which contains the signature intended for verification.
 * @param   key Array containing the public ed25519 signing key used to verify the signature.
 * @return  1 if signature verification was successful, 0 if verification failed. -1 if error occurred.
*/
static int sgnt_validate_sig_field_key(const signet_t *signet, unsigned char sig_fid, ED25519_KEY *key) {

    int res;
    size_t data_size, signet_size;
    unsigned char *sig, *data;
    ed25519_signature ed_sig;

    if(!signet || !key) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if(!(data = sgnt_signet_serialize_upto_fid(signet, sig_fid - 1, &data_size))) {
        RET_ERROR_INT(ERR_UNSPEC, "could not get signet fields for signature operation");
    }

    if(!(sig = sgnt_fid_num_fetch(signet, sig_fid, 1, &signet_size))) {
        free(data);
        RET_ERROR_INT(ERR_UNSPEC, "could not retrieve user signet signature field");
    }

    memcpy(&(ed_sig[0]), sig, signet_size);

    res = _ed25519_verify_sig(data, data_size, key, sig);
    free(data);
    free(sig);

    if(res < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "error encountered in signet signature verification");
    }

    return res;
}


/**
 * @brief   Uses a signet's signing keys to verify a signature.
 * @param   signet  Pointer to the signet.
 * @param   sig ed25519 signature buffer to be verified.
 * @param   buf Data buffer over which the signature was taken.
 * @param   buf_len Length of data buffer.
 * @return  1 on successful verification, 0 if the signature was invalid, -1 if an error occurred.
*/
static int sgnt_msg_sig_verify(const signet_t *signet, ed25519_signature sig, const unsigned char *buf, size_t buf_len) {

    int res, result = 0;
    ED25519_KEY **keys = NULL;
    ED25519_KEY *key = NULL;
    signet_type_t sigtype;

    if(!signet || !sig || !buf || !buf_len) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if ((sigtype = sgnt_type_get(signet)) == SIGNET_TYPE_SSR) {
        RET_ERROR_INT(ERR_UNSPEC, "SSR cannot be used for user message signature verification");
    } else if (sigtype == SIGNET_TYPE_USER) {

        if(!(key = sgnt_signkey_fetch(signet))) {
            RET_ERROR_INT(ERR_UNSPEC, "error retrieving signing key from signet");
        }

        res = _ed25519_verify_sig(buf, buf_len, key, sig);
        _free_ed25519_key(key);

        if(res < 0) {
            RET_ERROR_INT(ERR_UNSPEC, "error occurred while verifying signature");
        }

        return res;
    } else if (sigtype != SIGNET_TYPE_ORG) {
        RET_ERROR_INT(ERR_UNSPEC, "invalid signet type");
    }

    if(!(keys = sgnt_signkeys_msg_fetch(signet))) {
        RET_ERROR_INT(ERR_UNSPEC, "error retrieving msg signing keys from signet");
    }

    for(size_t i = 0; keys[i]; ++i) {

        if((res = _ed25519_verify_sig(buf, buf_len, keys[i], sig)) < 0) {
            PUSH_ERROR(ERR_UNSPEC, "error occurred during signature verification");
            result = -1;
            break;
        }

        if(res) {
            result = 1;
            break;
        }
    }

    _free_ed25519_key_chain(keys);

    if (result < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "error occurred while verifying signature");
    }

    return result;
}


/* Signet Builder Sign */

/**
 * @brief   Checks for the presence of all required fields that come before the FULL signature and signs the entire target signet using the specified key.
 * @param   signet  Pointer to the target signet_t structure.
 * @param   key Specified ed25519 key used for signing.
 * @return  0 on success, -1 on failure.
*/
static int sgnt_sig_id_sign(signet_t *signet, ED25519_KEY *key) {

    unsigned char fid;

    if(!signet || !key) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        fid = SIGNET_ORG_ID_SIG;
        break;
    case SIGNET_TYPE_USER:
        fid = SIGNET_USER_ID_SIG;
        break;
    default:
        RET_ERROR_INT(ERR_UNSPEC, "unsupported signet type");
        break;

    }

    if((sgnt_field_sign(signet, fid, key)) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "could not take full signet signature");
    }

    return 0;
}


/**
 * @brief   Checks for the presence of all required fields that come before the CORE signature and signs all the fields that come before the CORE signature field
 * @param   signet  Pointer to the target signet_t structure.
 * @param   key Specified ed25519 key used for signing.
 * @return  0 on success, -1 on failure.
*/
static int
sgnt_sig_full_sign(
    signet_t *signet,
    ED25519_KEY *key)
{
    unsigned char fid;

    if(!signet || !key) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        fid = SIGNET_ORG_FULL_SIG;
        break;
    case SIGNET_TYPE_USER:
        fid = SIGNET_USER_FULL_SIG;
        break;
    default:
        RET_ERROR_INT(ERR_UNSPEC, "unsupported signet type");
        break;

    }

    if((sgnt_field_sign(signet, fid, key)) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "could not sign signet");
    }

    return 0;
}


/**
 * @brief   Signs an incomplete user signet with the INITIAL signature after checking for the presence of all previous required fields.
 * @param   signet  Pointer to the target signet_t structure.
 * @param   key Specified ed25519 key used for signing.
 * @return  0 on success, -1 on failure.
*/
static int
sgnt_sig_crypto_sign(
    signet_t *signet,
    ED25519_KEY *key)
{
    signet_type_t type;
    unsigned char fid;

    if(!signet || !key) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    switch(type = sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        fid = SIGNET_ORG_CRYPTO_SIG;
        break;
    case SIGNET_TYPE_SSR:
        fid = SIGNET_USER_CRYPTO_SIG;

        if(sgnt_type_set(signet, SIGNET_TYPE_USER) < 0) {
            RET_ERROR_INT(ERR_UNSPEC, "could not change signet type to user from ssr");
        }

        break;
    default:
        RET_ERROR_INT(ERR_UNSPEC, "invalid signet type for crypto signature");

    }

    if((sgnt_field_sign(signet, fid, key)) < 0) {

        if(sgnt_type_set(signet, SIGNET_TYPE_SSR) < 0) {
            RET_ERROR_INT(ERR_UNSPEC, "could not change signet type from user back to ssr after signing failed");
        }

        RET_ERROR_INT(ERR_UNSPEC, "error encountered in signet signing operation");
    }

    return 0;
}


/**
 * @brief   Checks for the presence of all required fields that come before the SSR signature field and adds the SSR signature.
 * @param   signet  Pointer to the target signet_t structure.
 * @param   key Specified ed25519 key used for signing.
 * @return  0 on success, -1 on failure.
*/
static int
sgnt_sig_ssr_sign(
    signet_t *signet,
    ED25519_KEY *key)
{
    if(!signet || !key) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if(sgnt_type_get(signet) != SIGNET_TYPE_SSR) {
        RET_ERROR_INT(ERR_UNSPEC, "invalid signet type");
    }

    if((sgnt_field_sign(signet, SIGNET_SSR_SSR_SIG, key)) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "error encountered in signet signing operation");
    }

    return 0;
}


/**
 * @brief
 *  Checks for the presence of all required fields that come before the chain
 *  of custody signature field and adds the SSR signature.
 * @param signet
 *  Pointer to the target signet_t structure.
 * @param key
 *  Specified ed25519 key used for signing.
 * @return
 *  0 on success, -1 on failure.
*/
static int
sgnt_sig_coc_sign(
    signet_t *signet,
    ED25519_KEY *key)
{
    if(!signet || !key) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if(sgnt_type_get(signet) != SIGNET_TYPE_SSR) {
        RET_ERROR_INT(ERR_UNSPEC, "invalid signet type");
    }

    if((sgnt_field_sign(signet, SIGNET_SSR_COC_SIG, key)) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "error encountered in signet signing operation");
    }

    return 0;
}


/**
 * @brief
 *  Returns a string from a signet_state_t enum type.
 * @param state
 *  Signet state.
 * @return
 *  Null terminated string corresponding to the state.
*/
char const *
sgnt_state_to_str(signet_state_t state)
{
    switch(state) {

    case SS_UNKNOWN:
        return "unknown";
    case SS_MALFORMED:
        return "malformed";
    case SS_OVERFLOW:
        return "overflow";
    case SS_INCOMPLETE:
        return "incomplete";
    case SS_BROKEN_COC:
        return "broken chain of custody";
    case SS_INVALID:
        return "unverified";
    case SS_SSR:
        return "SSR";
    case SS_CRYPTO:
        return "user cryptographic portion";
    case SS_FULL:
        return "id-stripped signet";
    case SS_ID:
        return "full signet";

    }

    return NULL;
}


/**
 * @brief
 *  Create a copy of the provided signet.
 * @param signet
 *  Signet to be copied.
 * @return
 *  The copy.
 * @free_using{sgnt_destroy_signet}
*/
static signet_t *
sgnt_signet_dupe(signet_t *signet)
{
    signet_t *copy;

    if(!signet) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
    }

    if(!(copy = sgnt_signet_create(sgnt_type_get(signet)))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to create new signet");
        goto error;
    }

    copy->size = signet->size;

    if(!(copy->data = malloc(copy->size))) {
        PUSH_ERROR(ERR_NOMEM, "failed to allocate data buffer for signet copy");
        goto cleanup_copy;
    }

    memcpy(copy->data, signet->data, copy->size);

    for(unsigned int i = 0; i < 256; ++i) {
        copy->fields[i] = signet->fields[i];
    }

    return copy;

cleanup_copy:
    sgnt_signet_destroy(copy);
error:
    return NULL;
}

/**
 * @brief   Retrieves signet id.
 * @param   signet      Signet from which the id is retrieved.
 * @return  Cstring containing ID or NULL on error.
 * @free_using{free}
*/
static char *
sgnt_id_fetch(signet_t *signet)
{
    char * result;
    size_t id_size;
    unsigned char *bin_id, fid;

    if(!signet) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    switch(sgnt_type_get(signet)) {

    case SIGNET_TYPE_ORG:
        fid = SIGNET_ORG_ID;
        break;
    case SIGNET_TYPE_USER:
        fid = SIGNET_USER_ID;
        break;
    case SIGNET_TYPE_SSR:
        PUSH_ERROR(ERR_UNSPEC, "SSR signet can not have an ID field");
        goto error;
    default:
        PUSH_ERROR(ERR_UNSPEC, "invalid signet type");
        goto error;

    }

    if(!(bin_id = sgnt_fid_num_fetch(signet, fid, 1, &id_size))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to retrieve the first instance of id field in signet");
        goto error;
    }

    if(!(result = malloc(id_size + 1))) {
        PUSH_ERROR(ERR_NOMEM, "failed to allocate memory for signet id");
        PUSH_ERROR_SYSCALL("malloc");
        goto cleanup_bin_id;
    }

    memset(result, 0, id_size + 1);
    memcpy(result, bin_id, id_size);
    free(bin_id);

    return result;

cleanup_bin_id:
    free(bin_id);
error:
    return NULL;
}

/* PUBLIC FUNCTIONS */

/**
 * @brief
 *  Retrieves the public encryption key from the signet, if the signet is a
 *  user signet only retrieves the main encryption key (not alternate).
 * @param signet
 *  Pointer to the target signet.
 * @return
 *  Pointer to the target encryption public key.
 * @free_using{free_ec_key}
*/
EC_KEY *
dime_sgnt_enckey_fetch(
    signet_t const *signet)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_enckey_fetch, signet);
}

/**
 * @brief
 *  Sets the public encryption key (non-alterante encryption key) for the
 *  signet.
 * @param signet
 *  Target signet.
 * @param key
 *  Public encryption key.
 * @param format
 *  Format specifier. TODO currently unused! (spec requires 0x04 but openssl
 *  natively serializes it to 0x02).
 * @return  0 on success, -1 on failure.
*/
int
dime_sgnt_enckey_set(
    signet_t *signet,
    EC_KEY *key,
    unsigned char format)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_enckey_set, signet, key, format);
}

/**
 * @brief
 *  Retrieves the number of fields with the specified field id.
 * @param signet
 *  Pointer to the target signet.
 * @param fid
 *  The target field id.
 * @return
 *  The number of fields with specified field id. On various errors returns -1.
 *  NOTE: int overflow should not occur because of field size lower and signet
 *  size upper bounds.
*/
int
dime_sgnt_fid_count_get(
    signet_t const *signet,
    unsigned char fid)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_fid_count_get, signet, fid);
}

/**
 * @brief
 *  Checks for presence of field with specified id in the signet
 * @param signet
 *  The signet to be checked
 * @param fid
 *  Specified field id
 * @return
 *  1 if such a field exists, 0 if it does not exist, -1 if error.
*/
int
dime_sgnt_fid_exists(
    signet_t const *signet,
    unsigned char fid)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_fid_exists, signet, fid);
}

/**
 * @brief
 *  Fetches the binary data value of the field specified by field id and the
 *  number at which it appears in the signet amongst fields with the same field
 *  id (1, 2, ...).
 * @param signet
 *  Pointer to the target signet.
 * @param fid
 *  Specified field id.
 * @param num
 *  Specified field number based on the order in which it appears in the
 *  signet.
 * @param data_size
 *  Pointer to the length of returned array.
 * @return  Array containing the binary data of the specified field, NULL if an error occurs.
 * @free_using{free}
*/
unsigned char *
dime_sgnt_fid_num_fetch(
    signet_t const *signet,
    unsigned char fid,
    unsigned int num,
    size_t *data_size)
{
    PUBLIC_FUNCTION_IMPLEMENT(
        sgnt_fid_num_fetch,
        signet,
        fid,
        num,
        data_size);
}

/**
 * @brief
 *  Removes the field specified by a field id and the number in which it
 *  appears in the target signet amongst fields with the same field id from the
 *  target signet.
 * @param signet
 *  Pointer to the target signet.
 * @param fid
 *  Field id of the field to be removed.
 * @param num
 *  The number in which the field to be removed appears amongst other fields
 *  with the same field id in the target signet, (1, 2, ...).
 * @return  0 on success, -1 on failure.
*/
int
dime_sgnt_fid_num_remove(
    signet_t *signet,
    unsigned char fid,
    int num)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_fid_num_remove, signet, fid, num);
}

/**
 * @brief
 *  Adds a field to the target field.
 * @param signet
 *  Pointer to the target signet.
 * @param fid
 *  Field id of the field to be added.
 * @param data_size
 *  Size of the array containing the field data.
 * @param   data        Field data.
 * @return  0 on success, -1 on failure.
*/
int
dime_sgnt_field_defined_create(
    signet_t *signet,
    unsigned char fid,
    size_t data_size,
    unsigned char const *data)
{
    PUBLIC_FUNCTION_IMPLEMENT(
        sgnt_field_defined_create,
        signet,
        fid,
        data_size,
        data);
}

/**
 * @brief
 *  Replaces all fields in the target signet with the specified field id with a
 *  new field specified by the parameters.
 * @param signet
 *  Pointer to the target signet_t structure.
 * @param fid
 *  Field id which specifies the fields to be replaced with the new field.
 * @param data_size
 *  Size of field data array.
 * @param data
 *  Array contaning field data.
 * @return  0 on success, -1 on failure.
*/
int
dime_sgnt_field_defined_set(
    signet_t *signet,
    unsigned char fid,
    size_t data_size,
    const unsigned char *data)
{
    PUBLIC_FUNCTION_IMPLEMENT(
        sgnt_field_defined_set,
        signet,
        fid,
        data_size,
        data);
}

/**
 * @brief
 *  Adds an undefined field to signet with specified name and data.
 * @param signet
 *  Pointer to the target signet to which the field is added.
 * @param name_size
 *  Size of field name.
 * @param name
 *  Pointer to  field name.
 * @param data_size
 *  Size of field data.
 * @param data
 *  Pointer to field data.
 * @return
 *  0 on success, -1 on failure.
*/
int
dime_sgnt_field_undefined_create(
    signet_t *signet,
    size_t name_size,
    unsigned char const *name,
    size_t data_size,
    unsigned char const *data)
{
    PUBLIC_FUNCTION_IMPLEMENT(
        sgnt_field_undefined_create,
        signet,
        name_size,
        name,
        data_size,
        data);
}

/**
 * @brief
 *  Fetches the first undefined field with the specified field name.
 * @param signet
 *  Pointer to the target signet.
 * @param name_size
 *  Length of the passed array containing the length of the target field name.
 * @param name
 *  Array containing the name of the desired undefined field.
 * @param data_size
 *  Pointer to the size of the array that gets returned by the function.
 * @return  The array containing the data from the specified field or NULL in case of failure such as if the field was not found.
 * @free_using{free}
*/
unsigned char *
dime_sgnt_field_undefined_fetch(
    signet_t const *signet,
    size_t name_size,
    unsigned char const *name,
    size_t *data_size)
{
    PUBLIC_FUNCTION_IMPLEMENT(
        sgnt_field_undefined_fetch,
        signet,
        name_size,
        name, data_size);
}

/**
 * @brief
 *  Removes an undefined field from the target signet by name.
 * @param signet
 *  Pointer to the target signet.
 * @param name_size
 *  Size of field name.
 * @param name
 *  Name of the field to be removed.
 * @return
 *  0 on success, -1 on failure.
*/
int
dime_sgnt_field_undefined_remove(
    signet_t *signet,
    size_t name_size,
    unsigned char const *name)
{
    PUBLIC_FUNCTION_IMPLEMENT(
        sgnt_field_undefined_remove,
        signet,
        name_size,
        name);
}

/**
 * @brief
 *  Stores a signet from the signet_t structure in a PEM formatted file
 *  specified by the filename.
 * @param signet
 *  Pointer to the signet_t structure containing the signet.
 * @param filename
 *  NULL terminated string containing the desired filename for the signet.
 * @return
 *  0 on success, -1 on failure.
*/
int dime_sgnt_file_create(
    signet_t *signet,
    char const *filename)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_file_create, signet, filename);
}

/**
 * @brief
 *  Takes a SHA512 fingerprint of a signet with all fields after the
 *  cryptographic signature field stripped off.
 * @note
 *  To take an SSR fingerprint, use the signet_ssr_fingerprint() function.
 * @param signet
 *  Pointer to the target signet.
 * @return
 *  Allocated NULL terminated string to a base64 encoded unpadded fingerprint.
 *  NULL on error.
 * @free_using{free}
*/
char *
dime_sgnt_fingerprint_crypto(
    signet_t const *signet)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_fingerprint_crypto, signet);
}

/**
 * @brief
 *  Takes a SHA512 fingerprint of the user or org signet with the ID and FULL
 *  signature fields stripped off.
 * @note
 *  To take an SSR fingerprint, use the signet_ssr_fingerprint() function.
 * @param signet
 *  Pointer to the target signet.
 * @return
 *  Allocated NULL terminated buffer to a base64 encoded unpadded fingerprint.
 *  NULL on failure.
 * @free_using{free}
*/
char *
dime_sgnt_fingerprint_full(
    signet_t const *signet)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_fingerprint_full, signet);
}

/**
 * @brief
 *  Takes a SHA512 fingerprint of the entire user or org signet.
 * @param signet
 *  Pointer to the target signet.
 * @return
 *  Allocated NULL terminated buffer to a base64 encoded unpadded fingerprint.
 *  NULL on failure;
 * @free_using{free}
*/
char *
dime_sgnt_fingerprint_id(
    signet_t const *signet)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_fingerprint_id, signet);
}

/**
 * @brief
 *  Takes a SHA512 fingerprint of a user signet or an ssr with all fields after
 *  the SSR signature stripped off.
 * @param signet
 *  Pointer to the target signet.
 * @return
 *  Allocated NULL terminated buffer to a base64 encoded unpadded fingerprint.
 * @free_using{free}
*/
char *
dime_sgnt_fingerprint_ssr(
    signet_t const *signet)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_fingerprint_ssr, signet);
}

/**
 * @brief
 *  Retrieves signet id.
 * @param signet
 *  Signet from which the id is retrieved.
 * @return
 *  Cstring containing ID or NULL on error.
 * @free_using{free}
*/
char *
dime_sgnt_id_fetch(signet_t *signet)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_id_fetch, signet);
}

/**
 * @brief
 *  Sets the ID of the signet to the specified NULL terminated string.
 * @param signet
 *  Pointer to the target signet.
 * @param id_size
 *  Size of signet id.
 * @param id
 *  Signet id.
 * @return
 *  0 on success, -1 on failure.
*/
int
dime_sgnt_id_set(
    signet_t *signet,
    size_t id_size,
    unsigned char const *id)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_id_set, signet, id_size, id);
}

/**
 * @brief
 *  Uses a signet's signing keys to verify a signature.
 * @param signet
 *  Pointer to the signet.
 * @param sig
 *  ed25519 signature buffer to be verified.
 * @param buf
 *  Data buffer over which the signature was taken.
 * @param buf_len
 *  Length of data buffer.
 * @return
 *  1 on successful verification, 0 if the signature could not be verified, -1
 *  if an error occurred.
*/
int
dime_sgnt_msg_sig_verify(
    signet_t const *signet,
    ed25519_signature sig,
    unsigned char const *buf,
    size_t buf_len)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_msg_sig_verify, signet, sig, buf, buf_len);
}

/**
 * @brief
 *  Checks for the presence of all required fields that come before the chain
 *  of custody signature field and adds the SSR signature.
 * @param signet
 *  Pointer to the target signet_t structure.
 * @param key
 *  Specified ed25519 key used for signing.
 * @return
 *  0 on success, -1 on failure.
*/
int
dime_sgnt_sig_coc_sign(
    signet_t *signet,
    ED25519_KEY *key)
{
    PUBLIC_FUNCTION_IMPLEMENT(
        sgnt_sig_coc_sign,
        signet,
        key);
}

/**
 * @brief
 *  Signs an SSR or an incomplete ORG signet with the cryptographic signature
 *  after checking for the presence of all previous required fields.
 * @param signet
 *  Pointer to the target signet_t structure.
 * @param key
 *  Specified ed25519 key used for signing.
 * @return
 *  0 on success, -1 on failure.
*/
int
dime_sgnt_sig_crypto_sign(
    signet_t *signet,
    ED25519_KEY *key)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_sig_crypto_sign, signet, key);
}

/**
 * @brief
 *  Checks for the presence of all required fields that come before the full
 *  signature and signs all the fields that come before the CORE signature
 *  field
 * @param signet
 *  Pointer to the target signet_t structure.
 * @param key
 *  Specified ed25519 key used for signing.
 * @return
 *  0 on success, -1 on failure.
*/
int
dime_sgnt_sig_full_sign(
    signet_t *signet,
    ED25519_KEY *key)
{
    PUBLIC_FUNCTION_IMPLEMENT(
        sgnt_sig_full_sign,
        signet,
        key);
}

/**
 * @brief
 *  Checks for the presence of all required fields that come before the FULL
 *  signature and signs the entire target signet using the specified key.
 * @param signet
 *  Pointer to the target signet_t structure.
 * @param key
 *  Specified ed25519 key used for signing.
 * @return  0 on success, -1 on failure.
*/
int dime_sgnt_sig_id_sign(signet_t *signet, ED25519_KEY *key) {
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_sig_id_sign, signet, key);
}

/**
 * @brief
 *  Checks for the presence of all required fields that come before the SSR
 *  signature field and adds the SSR signature.
 * @param signet
 *  Pointer to the target signet_t structure.
 * @param key
 *  Specified ed25519 key used for signing.
 * @return
 *  0 on success, -1 on failure.
*/
int
dime_sgnt_sig_ssr_sign(
    signet_t *signet,
    ED25519_KEY *key)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_sig_ssr_sign, signet, key);
}

/**
 * @brief
 *  Returns a new signet_t structure that gets deserialized from a data buffer
 * @param in
 *  data buffer containing the binary form of a signet
 * @param in_len
 *  length of data buffer
 * @return
 *  A pointer to a newly allocated signet_t structure type, NULL on failure.
 * @free_using{dime_sgnt_destroy_signet}
*/
signet_t *
dime_sgnt_signet_binary_deserialize(
    unsigned char const *in,
    size_t len)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_signet_binary_deserialize, in, len);
}

/**
 * @brief
 *  Serializes a signet structure into binary data.
 * @param signet
 *  Pointer to the target signet.
 * @param serial_size
 *  Pointer to the value that stores the length of the array returned.
 * @return
 *  Signet serialized into binary data. NULL on error.
 * @free_using{free}
*/
unsigned char *
dime_sgnt_signet_binary_serialize(
    signet_t *signet,
    uint32_t *serial_size)
{
    PUBLIC_FUNCTION_IMPLEMENT(
        sgnt_signet_binary_serialize,
        signet, serial_size);
}

/**
 * @brief
 *  Deserializes a b64 signet into a signet structure.
 * @param b64_in
 *  NULL terminated array of b64 signet data.
 * @return
 *  Pointer to newly allocated signet structure, NULL if failure.
 * @free_using{dime_sgnt_destroy_signet}
*/
signet_t *
dime_sgnt_signet_b64_deserialize(const char *b64_in) {
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_signet_b64_deserialize, b64_in);
}

/**
 * @brief
 *  Serializes a signet structure into b64 data.
 * @param signet
 *  Pointer to the target signet.
 * @return
 *  Signet serialized into b64 data. NULL on error.
 * @free_using{free}
*/
char *
dime_sgnt_signet_b64_serialize(signet_t *signet) {
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_signet_b64_serialize, signet);
}

/**
 * @brief
 *  Returns a new signet_t structure.
 * @param type
 *  signet type user org or sss (SIGNET_TYPE_USER, SIGNET_TYPE_ORG or
 *  SIGNET_TYPE_SSR)
 * @return
 *  A pointer to a newly allocated signet_t structure type, NULL if failure.
 * @free_using{dime_sgnt_destroy_signet}
*/
signet_t *dime_sgnt_signet_create(signet_type_t type) {
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_signet_create, type);
}

/**
 * @brief
 *  Creates a signet structure with public signing and encyption keys. Also
 *  creates a keys file in which the private keys are stored.
 * @param type
 *  Signet type, org, user or ssr (SIGNET_TYPE_ORG, SIGNET_TYPE_USER or
 *  SIGNET_TYPE_SSR).
 * @param keysfile
 *  NULL terminated string containing the name of the keyfile to be created.
 * @return
 *  Pointer to the newly created and allocated signet_t structure or NULL on
 *  error.
 * @free_using{dime_sgnt_destroy_signet}
*/
signet_t *
dime_sgnt_signet_create_w_keys(
    signet_type_t type,
    char const *keysfile)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_signet_create_w_keys, type, keysfile);
}

/**
 * @brief
 *  Creates a copy of the target user signet with all fields beyond the INITIAL
 *  signature stripped off.
 * @param signet
 *  Pointer to the target signet.
 * @return
 *  Pointer to a stripped signet on success, NULL on failure.
 * @free_using{dime_sgnt_destroy_signet}
*/
signet_t *
dime_sgnt_signet_crypto_split(
    signet_t const *signet)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_signet_crypto_split, signet);
}

/**
 * @brief
 *  Destroys a signet_t structure.
 * @param signet
 *  Pointer to the signet to be destroyed.
*/
void
dime_sgnt_signet_destroy(signet_t *signet) {
    PUBLIC_FUNCTION_IMPLEMENT_VOID(sgnt_signet_destroy, signet);
}

/**
 * @brief
 *  Dumps signet into the specified file descriptor.
 * @param fp
 *  File descriptor the signet is dumped to.
 * @param signet
 *  Pointer to the signet_t structure to be dumped.
*/
void
dime_sgnt_signet_dump(FILE *fp, signet_t *signet) {
    PUBLIC_FUNCTION_IMPLEMENT_VOID(sgnt_signet_dump, fp, signet);
}

/**
 * @brief
 *  Create a copy of the provided signet.
 * @param signet
 *  Signet to be copied.
 * @return
 *  The copy.
 * @free_using{dime_sgnt_destroy_signet}
*/
signet_t *
dime_sgnt_signet_dupe(signet_t *signet) {
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_signet_dupe, signet);
}

/**
 * @brief
 *  Creates a copy of the target signet with the ID field and the FULL
 *  signature stripped off.
 * @param signet
 *  Pointer to the target signet.
 * @return
 *  Pointer to a stripped signet on success, NULL on failure.
 * @free_using{dime_sgnt_destroy_signet}
*/
signet_t *
dime_sgnt_signet_full_split(
    signet_t const *signet)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_signet_full_split, signet);
}

/**
 * @brief
 *  Loads signet_t structure from a PEM formatted file specified by filename.
 * @param filename
 *  NULL terminated string containing the filename of the file containing the
 *  signet.
 * @return
 *  Pointer to a newly created signet_t structure loaded from the file, NULL on
 *  failure.
 * @free_using{dime_sgnt_destroy_signet}
*/
signet_t *
dime_sgnt_signet_load(
    char const *filename) {
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_signet_load, filename);
}

/**
 * @brief
 *  Retrieves the public signing key from the signet, if the signet is an org
 *  signet only retrieves the POK.
 * @param signet
 *  Pointer to the target signet.
 * @return
 *  Pointer to the target ed25519 public key.
 * @free_using{free_ed25519_key}
*/
ED25519_KEY *
dime_sgnt_signkey_fetch(
    signet_t const *signet)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_signkey_fetch, signet);
}

/**
 * @brief
 *  Retrieves all the signing keys from an org signet that can be used to sign
 *  a message.
 * @param signet
 *  Pointer to target organizational signet.
 * @return
 *  A NULL pointer terminated array of ed25519 public signing key objects.
 * @note
 *  Always returns at least POK.
 * @free_using{free_ed25519_key_chain}
*/
int
dime_sgnt_signkey_set(
    signet_t *signet,
    ED25519_KEY *key,
    unsigned char format)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_signkey_set, signet, key, format);
}

/**
 * @brief
 *  Retrieves all the signing keys from an org signet that can be used to sign
 *  a signet.
 * @param signet
 *  Pointer to target organizational signet.
 * @return
 *  A NULL pointer terminated array of ed25519 public signing key objects.
 * @note
 *  Always returns at least POK.
 * @free_using{free_ed25519_key_chain}
*/
ED25519_KEY **
dime_sgnt_signkeys_msg_fetch(
    signet_t const *signet)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_signkeys_msg_fetch, signet);
}

/**
 * @brief
 *  Retrieves all the signing keys from an org signet that can be used to sign
 *  software.
 * @param signet
 *  Pointer to target organizational signet.
 * @return
 *  A NULL pointer terminated array of ed25519 public signing key objects.
 * @note
 *  Always returns at least POK.
 * @free_using{free_ed25519_key_chain}
*/
ED25519_KEY **
dime_sgnt_signkeys_signet_fetch(signet_t const *signet)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_signkeys_signet_fetch, signet);
}

/**
 * @brief
 *  Retrieves all the signing keys from an org signet that can be used to sign
 *  a TLS certificate.
 * @param signet
 *  Pointer to target organizational signet.
 * @return
 *  A NULL pointer terminated array of ed25519 public signing key objects.
 * @note
 *  Always returns at least POK.
 * @free_using{free_ed25519_key_chain}
*/
ED25519_KEY **
dime_sgnt_signkeys_software_fetch(
    signet_t const *signet)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_signkeys_software_fetch, signet);
}

/**
 * @brief
 *  sets the signing key (pok - primary signing key in case of an org signet).
 * @param signet
 *  pointer to the target signet.
 * @param key
 *  public signing key to be set as the signing key of the signet.
 * @param format
 *  format specifier byte, dictating the format.
 * @return
 *  0 on success, -1 on failure.
*/
ED25519_KEY **
dime_sgnt_signkeys_tls_fetch(
    signet_t const *signet)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_signkeys_tls_fetch, signet);
}

/**
 * @brief
 *  adds a sok (secondary organizational signing key) to an organizational
 *  signet.
 * @param signet
 *  pointer to the target org signet.
 * @param key
 *  ed25519 key to be added as a sok to the signet.
 * @param format
 *  format specifier byte dictating the format.
 * @param perm
 *  permissions for the usage of the sok.
 * @return
 *  0 on success, -1 on failure.
*/
int
dime_sgnt_sok_create(
    signet_t *signet,
    ED25519_KEY *key,
    unsigned char format,
    uint8_t perm)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_sok_create, signet, key, format, perm);
}

/**
 * @brief
 *  fetch the secondary organizational signing key from the signet by number
 *  (starting at 1)
 * @param signet
 *  pointer to the target organizational signet.
 * @param
 *  num the sok number to be fetched.
 * @return
 *  retrieved ed25519 key.
 * @free_using{free_ed25519_key}
*/
ED25519_KEY *
dime_sgnt_sok_num_fetch(
    signet_t const *signet,
    unsigned int num)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_sok_num_fetch, signet, num);
}

/**
 * @brief
 *  returns a string from a signet_state_t enum type.
 * @param state
 *  signet state.
 * @return
 *  null terminated string corresponding to the state.
*/
char const *
dime_sgnt_state_to_str(signet_state_t state) {
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_state_to_str, state);
}

/**
 * @brief
 *  retrieves the signet type, org or user (signet_type_org or
 *  signet_type_user)
 * @param signet
 *  pointer to the target signet.
 * @return
 *  a signet_type_t enum type with the signet type, signet_type_error on
 *  failure.
*/
signet_type_t dime_sgnt_type_get(const signet_t *signet) {
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_type_get, signet);
}

/**
 * @brief
 *  sets the target signet to a specified type.
 * @param signet
 *  pointer to the target signet.
 * @param type
 *  specified signet type.
 * @return
 *  0 on success, -1 on error.
*/
int
dime_sgnt_type_set(
    signet_t *signet,
    signet_type_t type)
{
    PUBLIC_FUNCTION_IMPLEMENT(sgnt_type_set, signet, type);
}

/**
 * @brief
 *  verifies a user signet, org signet or ssr for syntax, context and
 *  cryptographic validity. does not perform chain of custody validation.
 * @param signet
 *  pointer to the target signet_t structure.
 * @param orgsig
 *  pointer to the org signet associated with the target signet if the target
 *  signet is a user signet.  if target signet is not a user signet, orgsig
 *  should be passed as null.
 * @param previous
 *  pointer to the previous user signet, which will be used validate the chain
 *  of custody signature, if such is available.
 * @param dime_pok
 *  a null terminated array of pointers to ed25519 poks from the dime record
 *  associated with the target signet if the target signet is an org signet.
 *  if the target signet is not an org signet dime_pok should be passed as
 *  null;
 * @return
 *  signet state as a signet_state_t enum type. ss_unknown on error.
*/
signet_state_t
dime_sgnt_validate_all(
    signet_t const *signet,
    signet_t const *previous,
    signet_t const *orgsig,
    unsigned char const **dime_pok)
{
    PUBLIC_FUNCTION_IMPLEMENT(
        sgnt_validate_all,
        signet,
        previous,
        orgsig,
        dime_pok);
}
