#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <netdb.h>

#include <openssl/buffer.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include "dime/common/error.h"
#include "dime/common/misc.h"

// sizeof(errinfo_t) and number elements chosen so the entire data structure fits inside one 4,096 byte page.
struct thread_err_stack {
    errinfo_t error_stack[ERR_STACK_SIZE];
    errinfo_t *error_stack_top;
    unsigned char error_stack_overflow;
} __attribute__((__packed__));


static __thread struct thread_err_stack _t_err_stack;


// The global error message string table.
static const err_desc_t err_desc_table[] = {
    { ERR_SYSCALL,          "error [errno] was returned by syscall" },
    { ERR_OPENSSL,          "an error occurred in -lopenssl" },
    { ERR_RESOLVER,         "an error occurred in the resolver library" },
    { ERR_UNSPEC,           "an unspecified error has occurred" },
    { ERR_BAD_PARAM,        "a bad parameter was supplied to the function" },
    { ERR_NOMEM,            "memory allocation prevented operation" },
    { ERR_PERM,             "permission was denied for the requested operation" }
};


void dump_last_error(void) {

    if (!_t_err_stack.error_stack_top) {
        fprintf(stderr, "Last error: error stack is empty.\n");
        return;
    }

    if (_t_err_stack.error_stack_overflow) {
        fprintf(stderr, "Warning: the error stack has overflowed, which has resulted in the loss of information.\n");
    }

    _dump_error(_t_err_stack.error_stack_top);

}


/**
 * @brief   Dump the entire contents of the error stack to the console.
 */
void dump_error_stack(void) {

    errinfo_t *errptr = &(_t_err_stack.error_stack[0]);
    size_t n = 0;

    if (!_t_err_stack.error_stack_top) {
        fprintf(stderr, "Error stack is empty.\n");
        return;
    }

    if (_t_err_stack.error_stack_overflow) {
        fprintf(stderr, "Warning: the error stack has overflowed, which has resulted in the loss of information.\n");
    }

    while (errptr <= _t_err_stack.error_stack_top) {
        fprintf(stderr, "[%zu]: ", n);
        _dump_error(errptr);
        errptr++;
        n++;
    }

}


/**
 * @brief   Dump an error stack entry to the console.
 * @param   error   a pointer to the error stack object to be displayed.
 */
void _dump_error(const errinfo_t *error) {

    const char *errstring;

    if (!error) {
        return;
    }

    if (!(errstring = get_error_string(error->errcode))) {
        errstring = "n/a";
    }

    fprintf(stderr, "%s:%d [%s()]: %u (%s), errno = %d, aux = \"%s\"\n",
            error->filename, error->lineno, error->funcname,
            error->errcode, errstring, error->xerrno,
            error->auxmsg);
}


/**
 * @brief   Get the error string corresponding to a particular error code.
 * @param   errcode     the value of the error code to be looked up.
 * @return  a null-terminated string containing a description of the specified error code, or NULL on failure.
 */
const char *get_error_string(unsigned int errcode) {

    for (size_t i = 0; i < sizeof(err_desc_table) / sizeof(err_desc_t); i++) {

        if (err_desc_table[i].errcode == errcode) {
            return err_desc_table[i].errmsg;
        }

    }

    return NULL;
}


/**
 * @brief   Get the value of the last error code on the error stack.
 * @return  the numerical error code of the last error in the error stack, or 0 if the stack was empty.
 */
unsigned int get_last_error_code(void) {

    if (!_t_err_stack.error_stack_top) {
        return 0;
    }

    return _t_err_stack.error_stack_top->errcode;
}


/**
 * @brief   Get the last error object on the error stack.
 * @return  a pointer to the last error object on the error stack, or NULL if the error stack is empty.
 */
const errinfo_t *get_last_error(void) {

    return _t_err_stack.error_stack_top;
}


errinfo_t *pop_last_error(void) {

    errinfo_t *error;

    if (!_t_err_stack.error_stack_top) {
        return NULL;
    }

    error = _t_err_stack.error_stack_top;

    if (_t_err_stack.error_stack_top == &(_t_err_stack.error_stack[0])) {
        _t_err_stack.error_stack_top = NULL;
    } else {
        _t_err_stack.error_stack_top--;
    }

    return error;
}


const errinfo_t *get_first_error(void) {

    if (!_t_err_stack.error_stack_top) {
        return NULL;
    }

    return &(_t_err_stack.error_stack[0]);
}


/**
 * @brief   Clear the calling thread's error stack.
 */
void _clear_error_stack(void) {

    memset(_t_err_stack.error_stack, 0, sizeof(_t_err_stack.error_stack));
    _t_err_stack.error_stack_top = NULL;
    _t_err_stack.error_stack_overflow = 0;

}


/**
 * @brief   Push an error onto the error stack.
 */
errinfo_t *_push_error_stack(const char *filename, const char *funcname, int lineno, unsigned int errcode, int xerrno, const char *auxmsg) {

    errinfo_t *err;

    if (_t_err_stack.error_stack_top == &(_t_err_stack.error_stack[ERR_STACK_SIZE - 1])) {
        _t_err_stack.error_stack_overflow = 1;
        fprintf(stderr, "Error stack overflow.\n");
        return NULL;
    }

    if (!_t_err_stack.error_stack_top) {
        _t_err_stack.error_stack_top = &(_t_err_stack.error_stack[0]);
    } else {
        _t_err_stack.error_stack_top++;
    }

    err = _create_new_error(_t_err_stack.error_stack_top, filename, funcname, lineno, errcode, xerrno, auxmsg);

    return err;
}


/**
 * @brief   Push an error onto the error stack with a format-string supplied aux message.
 */
errinfo_t *_push_error_stack_fmt(const char *filename, const char *funcname, int lineno, unsigned int errcode, int xerrno, const char *fmt, ...) {

    va_list ap;
    char auxmsg[1024];

    memset(auxmsg, 0, sizeof(auxmsg));

    va_start(ap, fmt);
    vsnprintf(auxmsg, sizeof(auxmsg), fmt, ap);
    va_end(ap);

    return (_push_error_stack(filename, funcname, lineno, errcode, xerrno, auxmsg));
}


/**
 * @brief   Push an error onto the error stack generated by a syscall (or anything that modifies errno).
 * @param   filename    the name of the source file in which the error occured.
 * @param   funcname    the name of the function in which the error occured.
 * @param   lineno      the line in the source file in which the error occured.
 * @param   xerrno      the immediate value of errno which was set by the error-returning function.
 * @param   errfunc     the name of the library call or syscall function responsible for setting errno.
 */
errinfo_t *_push_error_stack_syscall(const char *filename, const char *funcname, int lineno, int xerrno, const char *errfunc) {

    char auxmsg[256], *ptr, *auxmsg_end = auxmsg + sizeof(auxmsg);

    memset(auxmsg, 0, sizeof(auxmsg));
    snprintf(auxmsg, sizeof(auxmsg) - 1, "%s: ", errfunc);
    ptr = auxmsg + strlen(auxmsg);
    #ifdef STRERROR_R_CHAR_P
    ptr = strerror_r(xerrno, ptr, auxmsg_end - ptr);
    #else
    strerror_r(xerrno, ptr, auxmsg_end - ptr);
    #endif

    return (_push_error_stack(filename, funcname, lineno, ERR_SYSCALL, xerrno, auxmsg));
}


/**
 * @brief   Push an openssl-generated error onto the error stack.
 */
errinfo_t *_push_error_stack_openssl(const char *filename, const char *funcname, int lineno, unsigned int errcode, int xerrno) {

    const char *ssl_filename, *ssl_data;
    char auxmsg[512], tmpbuf[512], tmpbuf2[512];
    int ssl_line, ssl_flags, first_pass = 1;
    static int loaded = 0;

    if (!loaded) {
        ERR_load_crypto_strings();
        SSL_load_error_strings();
        loaded = 1;
    }

    memset(auxmsg, 0, sizeof(auxmsg));

    // Keep doing this as long as there's more errors there.
    while (ERR_peek_error_line_data(&ssl_filename, &ssl_line, &ssl_data, &ssl_flags)) {

        if (!first_pass) {
            BUF_strlcat(auxmsg, " | ", sizeof(auxmsg));
        } else {
            first_pass = 0;
        }

        memset(tmpbuf, 0, sizeof(tmpbuf));
        snprintf(tmpbuf, sizeof(tmpbuf), ":%s:%d", ssl_filename, ssl_line);

        memset(tmpbuf2, 0, sizeof(tmpbuf2));
        ERR_error_string_n(ERR_get_error(), tmpbuf2, sizeof(tmpbuf2));

        // Combine the two error strings and add them to the buffer.
        BUF_strlcat(auxmsg, tmpbuf2, sizeof(auxmsg));
        BUF_strlcat(auxmsg, tmpbuf, sizeof(auxmsg));
    }

    return (_push_error_stack(filename, funcname, lineno, errcode, xerrno, auxmsg));
}


/**
 * @brief   Push an error onto the error stack generated by a resolver library function.
 */
errinfo_t *_push_error_stack_resolver(const char *filename, const char *funcname, int lineno, int xerrno, int herrno, const char *errfunc) {

    char auxmsg[256];

    memset(auxmsg, 0, sizeof(auxmsg));
    snprintf(auxmsg, sizeof(auxmsg) - 1, "%s: [%d]: %s", errfunc, herrno, hstrerror(herrno));

    return (_push_error_stack(filename, funcname, lineno, ERR_RESOLVER, xerrno, auxmsg));
}


/**
 * @brief   Populate a new entry on the error stack.
 */
errinfo_t *_create_new_error(errinfo_t *errptr, const char *filename, const char *funcname, int lineno, unsigned int errcode, int xerrno, const char *auxmsg) {

    memset(errptr, 0, sizeof(errinfo_t));

    if (filename) {
        snprintf(errptr->filename, sizeof(errptr->filename), "%s", filename);
    }

    if (funcname) {
        snprintf(errptr->funcname, sizeof(errptr->funcname), "%s", funcname);
    }

    if (auxmsg) {
        snprintf(errptr->auxmsg, sizeof(errptr->auxmsg), "%s", auxmsg);
    }

    errptr->lineno = lineno;
    errptr->errcode = errcode;
    errptr->xerrno = xerrno;

    return errptr;
}
