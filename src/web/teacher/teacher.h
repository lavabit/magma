
/**
 * @file /magma/web/teacher/teacher.h
 *
 * @brief	A facility for allowing users to train the statistical mail filter.
 */

#ifndef MAGMA_WEB_TEACHER_H
#define MAGMA_WEB_TEACHER_H

// Temporarily relocated from the now-defunct advertising.h
typedef struct {
        int_t completed, disposition;
        uint64_t usernum, signum, keynum;
        stringer_t *signature, *username, *password;
} teacher_data_t;


/// datatier.c
void              teacher_data_delete(teacher_data_t *teach);
teacher_data_t *  teacher_data_fetch(uint64_t signum);
void              teacher_data_free(teacher_data_t *teach);
teacher_data_t *  teacher_data_get(uint64_t signum);
void              teacher_data_save(teacher_data_t *teach);

/// teacher.c
void           teacher_add_cookie(connection_t *con, teacher_data_t *teach);
http_page_t *  teacher_add_error(chr_t *xpath, chr_t *id, chr_t *message);
void           teacher_print_form(connection_t *con, http_page_t *page, teacher_data_t *teach);
void           teacher_print_message(connection_t *con, chr_t *message);
void           teacher_process(connection_t *con);

#endif

