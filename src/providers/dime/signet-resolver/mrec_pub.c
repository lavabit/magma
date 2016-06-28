#include "dime/signet-resolver/mrec.h"

void destroy_dime_record(dime_record_t *record) {
    PUBLIC_FUNC_IMPL_VOID(destroy_dime_record, record);
}

dime_record_t *parse_dime_record(const char *txt, size_t len) {
    PUBLIC_FUNC_IMPL(parse_dime_record, txt, len);
}

dime_record_t *get_dime_record(const char *domain, unsigned long *ttl, int use_cache) {
    PUBLIC_FUNC_IMPL(get_dime_record, domain, ttl, use_cache);
}

int validate_dime_record(const dime_record_t *record) {
    PUBLIC_FUNC_IMPL(validate_dime_record, record);
}

dime_record_t *get_dime_record_from_file(const char *filename, const char *domain) {
    PUBLIC_FUNC_IMPL(get_dime_record_from_file, filename, domain);
}
