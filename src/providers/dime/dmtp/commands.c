#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dime/sds/sds.h"
#include "dime/dmtp/commands.h"
#include "dime/common/error.h"

#define DMTP_IGNORE_ARG { NULL, 0,  DMTP_ARG_PLAIN, 0 }
#define DMTP_EMPTY_ARG  { NULL, 0,  DMTP_ARG_NONE,  0 }

static int                  dmtp_command_argument_parse(const char *line, size_t insize, unsigned int arg_index, size_t *parsed, dmtp_command_t *command);
static dmtp_command_t *     dmtp_command_create(dmtp_command_type_t type);
static void                 dmtp_command_destroy(dmtp_command_t *command);
static int                  dmtp_command_end_check(sds line, size_t at);
static sds                  dmtp_command_format(dmtp_command_t *command);
static int                  dmtp_command_is_valid(dmtp_command_t *command);
static dmtp_command_key_t * dmtp_command_key_get(dmtp_command_type_t type);
static dmtp_parse_state_t   dmtp_command_parse(sds comm_line, dmtp_command_t **comm_struct);
static int                  dmtp_command_type_cmp(sds command, dmtp_command_type_t type);
static dmtp_command_type_t  dmtp_command_type_get(sds command);
static int                  dmtp_command_whitespace_parse(sds line, size_t at);



/**
 * @brief
 * Parse the next command string argument.
 * @param line
 * Pointer to the current location in the command string.
 * @param insize
 * Pointer to the remaining size of command string.
 * @param arg_index
 * The index of the least numbered argument to be checked for.
 * @param parsed
 * Stores the length of the data that gets parsed.
 * @param command
 * dmtp command structure where the parsed argument gets stored.
 * @return
 * Index of the next argument type to be parsed on success, -1 on failure.
*/
static int
dmtp_command_argument_parse(
    const char *line,
    size_t insize,
    unsigned int arg_index,
    size_t *parsed,
    dmtp_command_t *command)
{

   char const *arg_start;
   char arg_end;
   dmtp_command_key_t *key;
   unsigned int arg_num, at, arg_offset;

   if(!line) {
       PUSH_ERROR(ERR_BAD_PARAM, NULL);
       goto error;
   }

   if(!insize) {
       PUSH_ERROR(ERR_BAD_PARAM, NULL);
       goto error;
   }

   if(arg_index >= DMTP_MAX_ARGUMENT_NUM) {
       PUSH_ERROR(ERR_BAD_PARAM, NULL);
       goto error;
   }

   if(!command) {
       PUSH_ERROR(ERR_BAD_PARAM, NULL);
       goto error;
   }

   if(!(key = dmtp_command_key_get(command->type))) {
       PUSH_ERROR(ERR_UNSPEC, "failed to retrieve command key");
       goto error;
   }

   for(arg_num = arg_index; arg_num < DMTP_MAX_ARGUMENT_NUM; ++arg_num) {

       if(key->args[arg_num].arg_name_len > insize) {
           continue;
       }

       if(memcmp(line, key->args[arg_num].arg_name, key->args[arg_num].arg_name_len) == 0) {
           break;
       }

   }

   if(arg_num >= DMTP_MAX_ARGUMENT_NUM) {
       PUSH_ERROR(ERR_UNSPEC, "no valid arguments could be parsed");
       goto error;
   }

   at = key->args[arg_num].arg_name_len;

   switch(key->args[arg_num].type) {

   case DMTP_ARG_ANGULAR_BRCKT:
       arg_start = "=<";
       arg_end = '>';
       break;
   case DMTP_ARG_SQUARE_BRCKT:
       arg_start = "=[";
       arg_end = ']';
       break;
   case DMTP_ARG_PLAIN:
       arg_start = "=";
       arg_end = ' ';  // special case
       break;
   default:
       PUSH_ERROR(ERR_UNSPEC, "invalid argument type");
       goto error;
   }

   if(at + strlen(arg_start) > insize) {
       PUSH_ERROR(ERR_UNSPEC, "invalid command string size");
       goto error;
   }

   if(memcmp(line + at, arg_start, strlen(arg_start)) != 0) {
       PUSH_ERROR(ERR_UNSPEC, "invalid argument syntax");
       goto error;
   }

   at += strlen(arg_start);
   arg_offset = at;

   if(key->args[arg_num].type != DMTP_ARG_PLAIN) {

       while(line[at] != arg_end && at < insize) {
           ++at;
       }

       if(at >= insize) {
           PUSH_ERROR(ERR_UNSPEC, "argument line too short");
           goto error;
       }

       command->args[arg_num] = sdsnewlen(line + arg_offset, at - arg_offset);
       *parsed = at+1;
   }
   else {

       while(!isspace(line[at]) && at < insize) {
           ++at;
       }

       if(at >= insize) {
           PUSH_ERROR(ERR_UNSPEC, "argument line too short");
           goto error;
       }

       command->args[arg_num] = sdsnewlen(line + arg_offset, at - arg_offset);
       *parsed = at;

   }

   return arg_num + 1;

error:
   return -1;
}


/**
 * @brief
 * Create a dmtp command structure.
 * @param type
 * dmtp command enum.
 * @return
 * A newly created dmtp command structure with specified type and NULL args.
 * @free_using
 * dmtp_command_destroy
*/
static dmtp_command_t *
dmtp_command_create(
    dmtp_command_type_t type)
{

    dmtp_command_t *result;

    if(type >= DMTP_COMMANDS_NUM) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(!(result = malloc(sizeof(dmtp_command_t)))) {
        PUSH_ERROR(ERR_NOMEM, "failed to allocate memory for a dmtp command");
        PUSH_ERROR_SYSCALL("malloc");
        goto error;
    }

    memset(result, 0, sizeof(dmtp_command_t));
    result->type = type;

    return result;

error:
    return NULL;
}


/**
 * @brief
 * Is the command string at the ending character(s).
 * @param sds
 * sds command string.
 * @param at
 * Current place in the string.
 * @return
 * 1 if true (at the end) 0 if false, -1 if an error occurred.
*/
static int
dmtp_command_end_check(
    sds line,
    size_t at)
{

    int result = 0;

    if(!line) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(at >= sdslen(line)) {
        PUSH_ERROR(ERR_UNSPEC, "command string is too short to contain ending character(s)");
        goto error;
    }

    if(line[at] == '\n') {

        if(sdslen(line) - 1 > at) {
            PUSH_ERROR(ERR_UNSPEC, "the command string did not end after the ending character was found");
            goto error;
        }

        result = 1;
        goto out;
    }

    if((sdslen(line) - 1) == at) {
        PUSH_ERROR(ERR_UNSPEC, "the command string is too short to contain the ending characters");
        goto error;
    }

    if(memcmp(line+at, "\r\n", 2) == 0) {

        if(sdslen(line) - 2 > at) {
            PUSH_ERROR(ERR_UNSPEC, "the command string did not end after the ending characters were found");
            goto error;
        }

        result = 1;
        goto out;
    }

out:
    return result;
error:
    return -1;
}


/**
 * @brief
 * Destroy a dmtp command structure.
 * @param command
 * dmtp command structure to be destroyed.
*/
static void
dmtp_command_destroy(
    dmtp_command_t *command)
{

    if(!command) {
        return;
    }

    for(int i = 0; i < DMTP_MAX_ARGUMENT_NUM; ++i) {
        sdsfree(command->args[i]);
    }

    free(command);
}


/**
 * @brief
 * Formats the specified dmtp command into a sds string.
 * @param command
 * Command to be formatted.
 * @return
 * sds string containing formatted command.
 * @free_using
 * sdsfree
*/
static sds
dmtp_command_format(
    dmtp_command_t *command)
{

    dmtp_command_key_t *key;
    sds result;

    if(!command) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(!(dmtp_command_is_valid(command))) {
        PUSH_ERROR(ERR_UNSPEC, "invalid dmtp command");
        goto error;
    }

    if(!(key = dmtp_command_key_get(command->type))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to retrieve dmtp command key");
        goto error;
    }

    if(!(result = sdsempty())) {
        PUSH_ERROR(ERR_UNSPEC, "failed to allocate sds string for command");
        goto error;
    }

    if(!(result = sdsMakeRoomFor(result, 512))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to make room for arguments");
        goto cleanup_result;
    }

    if(!(result = sdscatlen(result, key->com_name, key->com_name_len))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to concatenate command name");
        goto cleanup_result;
    }

    for(int i = 0; i < DMTP_MAX_ARGUMENT_NUM; ++i) {

        if( command->args[i] && (key->args[i].type != DMTP_ARG_NONE) ) {

            if(!(result = sdscatlen(result, " ", 1))) {
                PUSH_ERROR(ERR_UNSPEC, "failed to concatenate white space");
                goto cleanup_result;
            }

            if(!(result = sdscatlen(result, key->args[i].arg_name, key->args[i].arg_name_len))) {
                PUSH_ERROR(ERR_UNSPEC, "failed to concatenate argument name");
                goto cleanup_result;
            }

            if(!(result = sdscatlen(result, "=", 1))) {
                PUSH_ERROR(ERR_UNSPEC, "failed to concatenate argument equals sign");
                goto cleanup_result;
            }

            if(key->args[i].type == DMTP_ARG_ANGULAR_BRCKT) {

                if(!(result = sdscatlen(result, "<", 1))) {
                    PUSH_ERROR(ERR_UNSPEC, "failed to concatenate angular bracket");
                    goto cleanup_result;
                }

            }

            if(key->args[i].type == DMTP_ARG_SQUARE_BRCKT) {

                if(!(result = sdscatlen(result, "[", 1))) {
                    PUSH_ERROR(ERR_UNSPEC, "failed to concatenate square bracket");
                    goto cleanup_result;
                }

            }

            if(!(result = sdscatsds(result, command->args[i]))) {
                PUSH_ERROR(ERR_UNSPEC, "failed to concatenate argument");
                goto cleanup_result;
            }

            if(key->args[i].type == DMTP_ARG_ANGULAR_BRCKT) {

                if(!(result = sdscatlen(result, ">", 1))) {
                    PUSH_ERROR(ERR_UNSPEC, "failed to concatenate angular bracket");
                    goto cleanup_result;
                }

            }

            if(key->args[i].type == DMTP_ARG_SQUARE_BRCKT) {

                if(!(result = sdscatlen(result, "]", 1))) {
                    PUSH_ERROR(ERR_UNSPEC, "failed to concatenate square bracket");
                    goto cleanup_result;
                }

            }

        }

    }

#ifndef MICROSOFT_COMMAND_FORMATTING
    if(!(result = sdscatlen(result, "\n", 1))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to concatenate end of command line character");
        goto cleanup_result;
    }
#else
    if(!(result = sdscatlen(result, "\r\n", 1))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to concatenate end of command line characters");
        goto cleanup_result;
    }
#endif

    if(sdslen(result) > 512) {
        PUSH_ERROR(ERR_UNSPEC, "command is too long");
        goto error;
    }

    return result;

cleanup_result:
    sdsfree(result);
error:
    return NULL;
}


/**
 * @brief
 * Is the provided dmtp command structure a valid dmtp command.
 * @param command
 * dmtp command structure to be validated.
 * @return 1 if valid (true) 0 if invalid (false).
*/
static int
dmtp_command_is_valid(
    dmtp_command_t *command)
{

    dmtp_command_key_t *key;
    int arg1, arg2, arg3, result;

    if(!command) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(!(key = dmtp_command_key_get(command->type))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to retrieve dmtp command key");
        goto error;
    }

    for(int i = 0; i < DMTP_MAX_ARGUMENT_NUM; ++i) {

        if( command->args[i] && (key->args[i].type == DMTP_ARG_NONE) ) {
            PUSH_ERROR(ERR_UNSPEC, "invalid argument type provided");
            goto error;
        }

        if( key->args[i].size && ( key->args[i].size != sdslen(command->args[i])) ) {
            PUSH_ERROR(ERR_UNSPEC, "invalid argument size");
            goto error;
        }

    }

    arg1 = (command->args[0] != NULL);
    arg2 = (command->args[1] != NULL);
    arg3 = (command->args[2] != NULL);

    switch(command->type) {

    case DMTP_STARTTLS:
        result = arg1 && !arg3;
        break;
    case DMTP_HELO:
        result = arg1 && !arg2 && !arg3;
        break;
    case DMTP_EHLO:
        result = arg1 && !arg2 && !arg3;
        break;
    case DMTP_MODE:
        result = !arg1 && !arg2 && !arg3;
        break;
    case DMTP_RSET:
        result = !arg1 && !arg2 && !arg3;
        break;
    case DMTP_NOOP:
        result = 1;
        break;
    case DMTP_HELP:
        result = !arg1 && !arg2 && !arg3;
        break;
    case DMTP_QUIT:
        result = !arg1 && !arg2 && !arg3;
        break;
    case DMTP_MAIL:
        result = arg1 && arg2 && !arg3;
        break;
    case DMTP_RCPT:
        result = arg1 && arg2 && !arg3;
        break;
    case DMTP_DATA:
        result = !arg1 && !arg2 && !arg3;
        break;
    case DMTP_SGNT:
        result = arg1 ^ arg2;
        break;
    case DMTP_HIST:
        result = arg1;
        break;
    case DMTP_VRFY:
        result = (arg1 ^ arg2) && arg3;
        break;
    default:
        PUSH_ERROR(ERR_UNSPEC, "invalid command type");
        goto error;

    }

    return result;

error:
    return 0;
}

/**
 * @brief
 * Retrieve the dmtp command key for the specific dmtp command type from the global table.
 * @param type
 * Specified dmtp command type.
 * @return
 * Pointer to the dmtp command key structure.
 * @NOTE: do not free!
*/
static dmtp_command_key_t *
dmtp_command_key_get(
    dmtp_command_type_t type)
{

    if(type >= DMTP_COMMANDS_NUM) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    return &(dmtp_command_list[type]);

error:
    return NULL;
}

/**
 * @brief
 * Parse the provided sds string into a dmtp command structure.
 * @param comm_str
 * sds dmtp command string.
 * @param comm_struct
 * @return
 * A valid dmtp command structure on success, NULL on failure.
 * @free_using
 * dmtp_command_destroy
*/
static dmtp_parse_state_t
dmtp_command_parse(
    sds comm_line,
    dmtp_command_t **comm_struct)
{

    dmtp_command_key_t *key;
    dmtp_command_type_t type;
    dmtp_parse_state_t result;
    size_t at = 0, len, parsed;
    int i = 0, ws_count;

    if(!comm_line) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        result = DMTP_PARSE_INVALID_CALL;
        goto error;
    }

    if(!comm_struct) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        result = DMTP_PARSE_INVALID_CALL;
        goto error;
    }

    if((len = sdslen(comm_line)) > 512) {
        PUSH_ERROR(ERR_UNSPEC, "command line is too long");
        result = DMTP_PARSE_INVALID_CALL;
        goto error;
    }

    if( (type = dmtp_command_type_get(comm_line)) == DMTP_COMMAND_INVALID ) {
        PUSH_ERROR(ERR_UNSPEC, "failed find a valid command");
        result = DMTP_PARSE_COMMAND_ERROR;
        goto error;
    }

    if(!(key = dmtp_command_key_get(type))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to find command key");
        result = DMTP_PARSE_COMMAND_ERROR;
        goto error;
    }

    at = key->com_name_len;

    if(!((*comm_struct) = dmtp_command_create(type))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to create a new command object");
        result = DMTP_PARSE_INTERNAL_ERROR;
        goto error;
    }

    switch(dmtp_command_end_check(comm_line, at)) {

    case -1:
        PUSH_ERROR(ERR_UNSPEC, "an error occurred while checking for ending characters");
        result = DMTP_PARSE_COMMAND_ERROR;
        goto cleanup_comm_struct;
    case 0:
        break;
    case 1:
        goto out;

    }

    if((ws_count = dmtp_command_whitespace_parse(comm_line, at)) < 0) {
        PUSH_ERROR(ERR_UNSPEC, "an error occurred while parsing whitespace");
        result = DMTP_PARSE_COMMAND_ERROR;
        goto cleanup_comm_struct;
    }
    else if(!ws_count) {
        PUSH_ERROR(ERR_UNSPEC, "expected at least a single white space after command name");
        result = DMTP_PARSE_COMMAND_ERROR;
        goto cleanup_comm_struct;
    }
    else {
        at += ws_count;
    }

    switch(dmtp_command_end_check(comm_line, at)) {

    case -1:
        PUSH_ERROR(ERR_UNSPEC, "an error occurred while checking for ending characters");
        result = DMTP_PARSE_COMMAND_ERROR;
        goto cleanup_comm_struct;
    case 0:
        break;
    case 1:
        goto out;

    }

    while(i < DMTP_MAX_ARGUMENT_NUM) {

        if(key->args[i].type == DMTP_ARG_NONE) {
            PUSH_ERROR(ERR_UNSPEC, "unexpected command argument");
            result = DMTP_PARSE_COMMAND_ERROR;
            goto cleanup_comm_struct;
        }

        if((i = dmtp_command_argument_parse(comm_line + at, len - at, i, &parsed, *comm_struct)) == -1) {
            PUSH_ERROR(ERR_UNSPEC, "error occurred while parsing an argument");
            result = DMTP_PARSE_ARGUMENT_ERROR;
            goto cleanup_comm_struct;
        }

        at += parsed;

        switch(dmtp_command_end_check(comm_line, at)) {

        case -1:
            PUSH_ERROR(ERR_UNSPEC, "an error occurred while checking for ending characters");
            result = DMTP_PARSE_COMMAND_ERROR;
            goto cleanup_comm_struct;
        case 0:
            break;
        case 1:
            goto out;

        }

        if((ws_count = dmtp_command_whitespace_parse(comm_line, at)) < 0) {
            PUSH_ERROR(ERR_UNSPEC, "an error occurred while parsing whitespace");
            result = DMTP_PARSE_COMMAND_ERROR;
            goto cleanup_comm_struct;
        }
        else if(!ws_count) {
            PUSH_ERROR(ERR_UNSPEC, "expected at least a single white space after command name");
            result = DMTP_PARSE_COMMAND_ERROR;
            goto cleanup_comm_struct;
        }
        else {
            at += ws_count;
        }

        switch(dmtp_command_end_check(comm_line, at)) {

        case -1:
            PUSH_ERROR(ERR_UNSPEC, "an error occurred while checking for ending characters");
            result = DMTP_PARSE_COMMAND_ERROR;
            goto cleanup_comm_struct;
        case 0:
            break;
        case 1:
            goto out;

        }

    }

// We need to check for ending characters here one more time, because if they're not found it is an error.
    switch(dmtp_command_end_check(comm_line, at)) {

    case -1:
        PUSH_ERROR(ERR_UNSPEC, "an error occurred while checking for ending characters");
        result = DMTP_PARSE_COMMAND_ERROR;
        goto cleanup_comm_struct;
    case 0:
        PUSH_ERROR(ERR_UNSPEC, "no ending characters were found at the end of the command string");
        result = DMTP_PARSE_COMMAND_ERROR;
        goto cleanup_comm_struct;
    case 1:
        break;

    }

out:
    if(!(dmtp_command_is_valid(*comm_struct))) {
        PUSH_ERROR(ERR_UNSPEC, "invalid dmtp command");
        result = DMTP_PARSE_COMMAND_ERROR;
        goto cleanup_comm_struct;
    }

    return DMTP_PARSE_SUCCESS;

cleanup_comm_struct:
    dmtp_command_destroy(*comm_struct);
error:
    return result;
}


/**
 * @brief
 * Compare the provided command string start with a valid dmtp command type string.
 * @param command
 * sds command string.
 * @param type
 * dmtp command type.
 * @return
 * 0 if comparison was successful, -1 if it was not.
*/
static int
dmtp_command_type_cmp(
    sds command,
    dmtp_command_type_t type)
{

    dmtp_command_key_t *key;

    if(!command) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(type >= DMTP_COMMANDS_NUM) {
        PUSH_ERROR(ERR_UNSPEC, "invalid dmtp command");
        goto error;
    }

    if(!(key = dmtp_command_key_get(type))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to retrieve command key");
        goto error;
    }

    if(sdslen(command) < key->com_name_len) {
        PUSH_ERROR(ERR_UNSPEC, "command line is too short to match a valid command");
        goto error;
    }

    if((memcmp(command, key->com_name, key->com_name_len) != 0)) {
        PUSH_ERROR(ERR_UNSPEC, "command line did not match the expected command");
        goto error;
    }

    return 0;

error:
    return -1;
}


/**
 * @brief
 * Determine the dmtp command type of the provided command string.
 * @command
 * sds command string.
 * @return
 * A valid dmtp type on success, DMTP_COMMAND_INVALID on failure.
*/
static dmtp_command_type_t
dmtp_command_type_get(
    sds command)
{

    dmtp_command_type_t result;

    if(!command) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(sdslen(command) < 4) {
        PUSH_ERROR(ERR_UNSPEC, "DMTP command-line is too short");
        goto error;
    }

    switch(command[0]) {

    case 'S':

        switch(command[1]) {

        case 'T':

            if(dmtp_command_type_cmp(command, DMTP_STARTTLS) != 0) {
                PUSH_ERROR(ERR_UNSPEC, "command looked like STARTTLS but comparison failed");
                goto error;
            }

            result = DMTP_STARTTLS;
            break;
        case 'G':

            if(dmtp_command_type_cmp(command, DMTP_SGNT) != 0) {
                PUSH_ERROR(ERR_UNSPEC, "command looked like SGNT but comparison failed");
                goto error;
            }

            result = DMTP_SGNT;
            break;
        default:
            PUSH_ERROR(ERR_UNSPEC, "invalid command");
            goto error;

        }

        break;
    case 'H':

        switch(command[1])
        {

        case 'E':

            switch(command[2])
            {

                case 'L':

                    switch(command[3])
                    {

                        case 'O':

                            if(dmtp_command_type_cmp(command, DMTP_HELO) != 0) {
                                PUSH_ERROR(ERR_UNSPEC, "command looked like HELO but comparison failed");
                                goto error;
                            }

                            result = DMTP_HELO;
                            break;
                        case 'P':

                            if(dmtp_command_type_cmp(command, DMTP_HELP) != 0) {
                                PUSH_ERROR(ERR_UNSPEC, "command looked like HELP but comparison failed");
                                goto error;
                            }

                            result = DMTP_HELP;
                            break;
                        default:
                            PUSH_ERROR(ERR_UNSPEC, "invalid command");
                            goto error;

                    }

                    break;
                default:
                    PUSH_ERROR(ERR_UNSPEC, "invalid command");
                    goto error;

            }

            break;
        case 'I':

            if(dmtp_command_type_cmp(command, DMTP_HIST) != 0) {
                PUSH_ERROR(ERR_UNSPEC, "command looked like HIST but comparison failed");
                goto error;
            }

            result = DMTP_HIST;
            break;
        default:
            PUSH_ERROR(ERR_UNSPEC, "invalid command");
            goto error;
        }

        break;
    case 'E':

        if(dmtp_command_type_cmp(command, DMTP_EHLO) != 0) {
            PUSH_ERROR(ERR_UNSPEC, "command looked like EHLO but comparison failed");
            goto error;
        }

        result = DMTP_EHLO;
        break;
    case 'M':

        switch(command[1]) {

        case 'O':

            if(dmtp_command_type_cmp(command, DMTP_MODE) != 0) {
                PUSH_ERROR(ERR_UNSPEC, "command looked like MODE but comparison failed");
                goto error;
            }

            result = DMTP_MODE;
            break;
        case 'A':

            if(dmtp_command_type_cmp(command, DMTP_MAIL) != 0) {
                PUSH_ERROR(ERR_UNSPEC, "command looked like MAIL but comparison failed");
                goto error;
            }

            result = DMTP_MAIL;
            break;
        default:
            PUSH_ERROR(ERR_UNSPEC, "invalid command");
            goto error;

        }

        break;
    case 'R':

        switch(command[1]) {

        case 'S':

            if(dmtp_command_type_cmp(command, DMTP_RSET) != 0) {
                PUSH_ERROR(ERR_UNSPEC, "command looked like RSET but comparison failed");
                goto error;
            }

            result = DMTP_RSET;
            break;
        case 'C':

            if(dmtp_command_type_cmp(command, DMTP_RCPT) != 0) {
                PUSH_ERROR(ERR_UNSPEC, "command looked like RCPT but comparison failed");
                goto error;
            }

            result = DMTP_RCPT;
            break;
        default:
            PUSH_ERROR(ERR_UNSPEC, "invalid command");
            goto error;

        }

        break;
    case 'N':

        if(dmtp_command_type_cmp(command, DMTP_NOOP) != 0) {
            PUSH_ERROR(ERR_UNSPEC, "command looked like NOOP but comparison failed");
            goto error;
        }

        result = DMTP_NOOP;
        break;
    case 'Q':

        if(dmtp_command_type_cmp(command, DMTP_QUIT) != 0) {
            PUSH_ERROR(ERR_UNSPEC, "command looked like QUIT but comparison failed");
            goto error;
        }

        result = DMTP_QUIT;
        break;
    case 'D':

        if(dmtp_command_type_cmp(command, DMTP_DATA) != 0) {
            PUSH_ERROR(ERR_UNSPEC, "command looked like DATA but comparison failed");
            goto error;
        }

        result = DMTP_DATA;
        break;
    case 'V':

        if(dmtp_command_type_cmp(command, DMTP_VRFY) != 0) {
            PUSH_ERROR(ERR_UNSPEC, "command looked like VRFY but comparison failed");
            goto error;
        }

        result = DMTP_VRFY;
        break;
    default:
        PUSH_ERROR(ERR_UNSPEC, "invalid command");
        goto error;

    }

    return result;

error:
    return DMTP_COMMAND_INVALID;
}


/**
 * @brief
 * Parse the white space in the command string.
 * @param line
 * sds command string.
 * @param at
 * Current position in the command string.
 * @return
 * Number of whitespace parsed. -1 if an error occurred. only ' ' and '\t' count as whitespaces.
*/
static int
dmtp_command_whitespace_parse(
    sds line,
    size_t at)
{

    int result = 0;

    if(!line) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(at >= sdslen(line)) {
        PUSH_ERROR(ERR_UNSPEC, "current position in string is outside the command line bounds");
        goto error;
    }

    while(at < sdslen(line)) {

        if((line[at] == ' ') || (line[at] == '\t')) {
            ++result;
            ++at;
        }
        else {
            break;
        }

    }

    return result;

error:
    return -1;
}


/**
 * @brief
 * Create a dmtp command structure.
 * @param type
 * dmtp command enum.
 * @return
 * A newly created dmtp command structure with specified type and NULL args.
 * @free_using
 * dime_dmtp_command_destroy
*/
dmtp_command_t *
dime_dmtp_command_create(
    dmtp_command_type_t type)
{
    PUBLIC_FUNCTION_IMPLEMENT(dmtp_command_create, type);
}

/**
 * @brief
 * Destroy a dmtp command structure.
 * @param command
 * dmtp command structure to be destroyed.
*/
void
dime_dmtp_command_destroy(
    dmtp_command_t *command)
{
    PUBLIC_FUNCTION_IMPLEMENT(dmtp_command_destroy, command);
}

/**
 * @brief
 * Formats the specified dmtp command into a sds string.
 * @param command
 * Command to be formatted.
 * @return
 * sds string containing formatted command.
 * @free_using
 * sdsfree
*/
sds
dime_dmtp_command_format(
    dmtp_command_t *command)
{
    PUBLIC_FUNCTION_IMPLEMENT(dmtp_command_format, command);
}

/**
 * @brief
 * Parse the provided sds string into a dmtp command structure.
 * @param command
 * sds dmtp command string.
 * @return
 * A valid dmtp command structure on success, NULL on failure.
 * @free_using
 * dime_dmtp_command_destroy
*/
dmtp_parse_state_t
dime_dmtp_command_parse(
    sds comm_line,
    dmtp_command_t **comm_struct)
{
    PUBLIC_FUNCTION_IMPLEMENT(dmtp_command_parse, comm_line, comm_struct);
}


/**
 * @brief
 * Create a string containing dmtp STARTTLS command with specified arguments.
 * @param host
 * Name of the domain that is being connected to.
 * @param mode
 * optional mode parameter (DMTP or SMTP). DMTP_MODE_NONE to not specify mode.
 * @return
 * sds string containing the desired command.
 * @free_using
 * sdsfree
*/
sds
dime_dmtp_command_starttls(
    sds host,
    dmtp_mode_t mode)
{

    dmtp_command_t *command;
    sds result;

    PUBLIC_FUNC_PROLOGUE();

    if(!host) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(!(command = dmtp_command_create(DMTP_STARTTLS))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to create dmtp starttls command struct");
        goto error;
    }

    switch(mode) {

    case DMTP_MODE_DMTP:

        if(!(command->args[1] = sdsnewlen("DMTP", 4))) {
            PUSH_ERROR(ERR_UNSPEC, "failed to add mode argument to starttls command");
            goto cleanup_command;
        }

        break;
    case DMTP_MODE_SMTP:

        if(!(command->args[1] = sdsnewlen("SMTP", 4))) {
            PUSH_ERROR(ERR_UNSPEC, "failed to add mode argument to starttls command");
            goto cleanup_command;
        }

        break;
    case DMTP_MODE_NONE:
        break;
    default:
        PUSH_ERROR(ERR_UNSPEC, "invalid dmtp mode");
        goto cleanup_command;

    }

    if(!(command->args[0] = sdsdup(host))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add host argument to starttls command");
        goto cleanup_command;
    }

    if(!(result = dmtp_command_format(command))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to format the starttls command");
        goto cleanup_command;
    }

    dmtp_command_destroy(command);

    return result;

cleanup_command:
    dmtp_command_destroy(command);
error:
    return NULL;
}


/**
 * @brief
 * Create a string containing dmtp HELO command with specified arguments.
 * @param host
 * Name of the domain that is being connected to.
 * @return
 * sds string containing the desired command.
 * @free_using
 * sdsfree
*/
sds
dime_dmtp_command_helo(
    sds host)
{

    dmtp_command_t *command;
    sds result;

    PUBLIC_FUNC_PROLOGUE();

    if(!host) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(!(command = dmtp_command_create(DMTP_HELO))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to create dmtp helo command struct");
        goto error;
    }

    if(!(command->args[0] = sdsdup(host))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add host argument to helo command");
        goto cleanup_command;
    }

    if(!(result = dmtp_command_format(command))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to format the helo command");
        goto cleanup_command;
    }

    dmtp_command_destroy(command);

    return result;

cleanup_command:
    dmtp_command_destroy(command);
error:
    return NULL;
}


/**
 * @brief
 * Create a string containing dmtp EHLO command with specified arguments.
 * @param host
 * Name of the domain that is being connected to.
 * @return
 * sds string containing the desired command.
 * @free_using
 * sdsfree
*/
sds
dime_dmtp_command_ehlo(
    sds host)
{

    dmtp_command_t *command;
    sds result;

    PUBLIC_FUNC_PROLOGUE();

    if(!host) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(!(command = dmtp_command_create(DMTP_EHLO))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to create dmtp ehlo command struct");
        goto error;
    }

    if(!(command->args[0] = sdsdup(host))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add host argument to ehlo command");
        goto cleanup_command;
    }

    if(!(result = dmtp_command_format(command))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to format the ehlo command");
        goto cleanup_command;
    }

    dmtp_command_destroy(command);

    return result;

cleanup_command:
    dmtp_command_destroy(command);
error:
    return NULL;
}


/**
 * @brief
 * Create a string containing dmtp MODE command with specified arguments.
 * @return
 * sds string containing the desired command.
 * @free_using
 * sdsfree
*/
sds
dime_dmtp_command_mode() {

    dmtp_command_t *command;
    sds result;

    PUBLIC_FUNC_PROLOGUE();

    if(!(command = dmtp_command_create(DMTP_MODE))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to create dmtp mode command struct");
        goto error;
    }

    if(!(result = dmtp_command_format(command))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to format the mode command");
        goto cleanup_command;
    }

    dmtp_command_destroy(command);

    return result;

cleanup_command:
    dmtp_command_destroy(command);
error:
    return NULL;
}


/**
 * @brief
 * Create a string containing dmtp RSET command with specified arguments.
 * @return
 * sds string containing the desired command.
 * @free_using
 * sdsfree
*/
sds
dime_dmtp_command_rset() {

    dmtp_command_t *command;
    sds result;

    PUBLIC_FUNC_PROLOGUE();

    if(!(command = dmtp_command_create(DMTP_RSET))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to create dmtp rset command struct");
        goto error;
    }

    if(!(result = dmtp_command_format(command))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to format the rset command");
        goto cleanup_command;
    }

    dmtp_command_destroy(command);

    return result;

cleanup_command:
    dmtp_command_destroy(command);
error:
    return NULL;
}


/**
 * @brief
 * Create a string containing dmtp NOOP command with specified arguments.
 * @param optarg1
 * Optional allowed argument 1. NULL if unwanted.
 * @param optarg2
 * Optional allowed argument 2. NULL if unwanted.
 * @param optarg3
 * Optional allowed argument 3. NULL if unwanted.
 * @return
 * sds string containing the desired command.
 * @free_using
 * sdsfree
*/
sds
dime_dmtp_command_noop(
    sds optarg1,
    sds optarg2,
    sds optarg3)
{

    dmtp_command_t *command;
    sds result;

    PUBLIC_FUNC_PROLOGUE();

    if(!(command = dmtp_command_create(DMTP_NOOP))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to create dmtp noop command struct");
        goto error;
    }

    if(!(command->args[0] = sdsdup(optarg1))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add optional argument to noop command");
        goto cleanup_command;
    }

    if(!(command->args[1] = sdsdup(optarg2))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add optional argument to noop command");
        goto cleanup_command;
    }

    if(!(command->args[2] = sdsdup(optarg3))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add optional argument to noop command");
        goto cleanup_command;
    }

    if(!(result = dmtp_command_format(command))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to format the noop command");
        goto cleanup_command;
    }

    dmtp_command_destroy(command);

    return result;

cleanup_command:
    dmtp_command_destroy(command);
error:
    return NULL;
}


/**
 * @brief
 * Create a string containing dmtp HELP command with specified arguments.
 * @return
 * sds string containing the desired command.
 * @free_using
 * sdsfree
*/
sds
dime_dmtp_command_help() {

    dmtp_command_t *command;
    sds result;

    PUBLIC_FUNC_PROLOGUE();

    if(!(command = dmtp_command_create(DMTP_HELP))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to create dmtp help command struct");
        goto error;
    }

    if(!(result = dmtp_command_format(command))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to format the help command");
        goto cleanup_command;
    }

    dmtp_command_destroy(command);

    return result;

cleanup_command:
    dmtp_command_destroy(command);
error:
    return NULL;
}


/**
 * @brief
 * Create a string containing dmtp QUIT command with specified arguments.
 * @return
 * sds string containing the desired command.
 * @free_using
 * sdsfree
*/
sds
dime_dmtp_command_quit() {

    dmtp_command_t *command;
    sds result;

    PUBLIC_FUNC_PROLOGUE();

    if(!(command = dmtp_command_create(DMTP_QUIT))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to create dmtp quit command struct");
        goto error;
    }

    if(!(result = dmtp_command_format(command))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to format the quit command");
        goto cleanup_command;
    }

    dmtp_command_destroy(command);

    return result;

cleanup_command:
    dmtp_command_destroy(command);
error:
    return NULL;
}



/**
 * @brief
 * Create a string containing dmtp MAIL command with specified arguments.
 * @param from
 * sds string containg origin domain.
 * @param fingerprint
 * sds string containing origin signet full fingerprint.
 * @return
 * sds string containing the desired command.
 * @free_using
 * sdsfree
*/
sds
dime_dmtp_command_mail(
    sds from,
    sds fingerprint)
{

    dmtp_command_t *command;
    sds result;

    PUBLIC_FUNC_PROLOGUE();

    if(!from) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(!fingerprint) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(!(command = dmtp_command_create(DMTP_MAIL))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to create dmtp mail command struct");
        goto error;
    }

    if(!(command->args[0] = sdsdup(from))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add from argument to mail command");
        goto cleanup_command;
    }

    if(!(command->args[1] = sdsdup(fingerprint))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add fingerprint argument to mail command");
        goto cleanup_command;
    }

    if(!(result = dmtp_command_format(command))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to format the mail command");
        goto cleanup_command;
    }

    dmtp_command_destroy(command);

    return result;

cleanup_command:
    dmtp_command_destroy(command);
error:
    return NULL;
}


/**
 * @brief
 * Create a string containing dmtp RCPT command with specified arguments.
 * @param to
 * sds string containg destination domain.
 * @param fingerprint
 * sds string containing destination signet full fingerprint.
 * @return
 * sds string containing the desired command.
 * @free_using
 * sdsfree
*/
sds
dime_dmtp_command_rcpt(
    sds to,
    sds fingerprint)
{

    dmtp_command_t *command;
    sds result;

    PUBLIC_FUNC_PROLOGUE();

    if(!to) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(!fingerprint) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(!(command = dmtp_command_create(DMTP_RCPT))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to create dmtp rcpt command struct");
        goto error;
    }

    if(!(command->args[0] = sdsdup(to))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add to argument to rcpt command");
        goto cleanup_command;
    }

    if(!(command->args[1] = sdsdup(fingerprint))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add fingerprint argument to rcpt command");
        goto cleanup_command;
    }

    if(!(result = dmtp_command_format(command))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to format the rcpt command");
        goto cleanup_command;
    }

    dmtp_command_destroy(command);

    return result;

cleanup_command:
    dmtp_command_destroy(command);
error:
    return NULL;
}


/**
 * @brief
 * Create a string containing dmtp DATA command with specified arguments.
 * @return
 * sds string containing the desired command.
 * @free_using
 * sdsfree
*/
sds
dime_dmtp_command_data() {

    dmtp_command_t *command;
    sds result;

    PUBLIC_FUNC_PROLOGUE();

    if(!(command = dmtp_command_create(DMTP_DATA))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to create dmtp data command struct");
        goto error;
    }

    if(!(result = dmtp_command_format(command))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to format tdata help command");
        goto cleanup_command;
    }

    dmtp_command_destroy(command);

    return result;

cleanup_command:
    dmtp_command_destroy(command);
error:
    return NULL;
}


/**
 * @brief
 * Create a string containing dmtp SGNT command with specified arguments.
 * @param address
 * sds string containing mail address of the user.
 * @param fingerprint
 * sds string containing optional user signet fingerprint. NULL if unwanted.
 * @return
 * sds string containing the desired command.
 * @free_using
 * sdsfree
*/
sds
dime_dmtp_command_sgnt_user(
    sds address,
    sds fingerprint)
{

    dmtp_command_t *command;
    sds result;

    PUBLIC_FUNC_PROLOGUE();

    if(!address) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(!(command = dmtp_command_create(DMTP_SGNT))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to create dmtp sgnt command struct");
        goto error;
    }

    if(!(command->args[0] = sdsdup(address))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add address argument to sgnt command");
        goto cleanup_command;
    }

    if(fingerprint && !(command->args[2] = sdsdup(fingerprint))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add fingerprint argument to sgnt command");
        goto cleanup_command;
    }

    if(!(result = dmtp_command_format(command))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to format the sgnt command");
        goto cleanup_command;
    }

    dmtp_command_destroy(command);

    return result;

cleanup_command:
    dmtp_command_destroy(command);
error:
    return NULL;
}


/**
 * @brief
 * Create a string containing dmtp SGNT command with specified arguments.
 * @param domain
 * sds string containing domain name.
 * @param fingerprint
 * sds string containing optional organizational signet fingerprint. NULL if unwanted.
 * @return
 * sds string containing the desired command.
 * @free_using
 * sdsfree
*/
sds
dime_dmtp_command_sgnt_domain(
    sds domain,
    sds fingerprint)
{

    dmtp_command_t *command;
    sds result;

    PUBLIC_FUNC_PROLOGUE();

    if(!domain) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(!(command = dmtp_command_create(DMTP_SGNT))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to create dmtp sgnt command struct");
        goto error;
    }

    if(!(command->args[1] = sdsdup(domain))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add domain argument to sgnt command");
        goto cleanup_command;
    }

    if(fingerprint && !(command->args[2] = sdsdup(fingerprint))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add fingerprint argument to sgnt command");
        goto cleanup_command;
    }

    if(!(result = dmtp_command_format(command))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to format the sgnt command");
        goto cleanup_command;
    }

    dmtp_command_destroy(command);

    return result;

cleanup_command:
    dmtp_command_destroy(command);
error:
    return NULL;
}


/**
 * @brief
 * Create a string containing dmtp HIST command with specified arguments.
 * @param address
 * sds string containing user mail address.
 * @param start
 * sds string containing optional starting user signet fingerprint. NULL if unwanted.
 * @param stop
 * sds string containing optional ending user signet fingerprint. NULL if unwanted.
 * @return
 * sds string containing the desired command.
 * @free_using
 * sdsfree
*/
sds
dime_dmtp_command_hist(
    sds address,
    sds start,
    sds stop)
{

    dmtp_command_t *command;
    sds result;

    PUBLIC_FUNC_PROLOGUE();

    if(!address) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(!(command = dmtp_command_create(DMTP_HIST))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to create dmtp hist command struct");
        goto error;
    }

    if(!(command->args[0] = sdsdup(address))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add domain argument to hist command");
        goto cleanup_command;
    }

    if(start && !(command->args[1] = sdsdup(start))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add start fingerprint argument to hist command");
        goto cleanup_command;
    }

    if(stop && !(command->args[2] = sdsdup(stop))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add stop fingerprint argument to hist command");
        goto cleanup_command;
    }

    if(!(result = dmtp_command_format(command))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to format the hist command");
        goto cleanup_command;
    }

    dmtp_command_destroy(command);

    return result;

cleanup_command:
    dmtp_command_destroy(command);
error:
    return NULL;
}


/**
 * @brief
 * Create a string containing dmtp VRFY command with specified arguments.
 * @param address
 * sds string containing user mail address.
 * @param start
 * sds string containing user signet fingerprint.
 * @return
 * sds string containing the desired command.
 * @free_using
 * sdsfree
*/
sds
dime_dmtp_command_vrfy_user(
    sds address,
    sds fingerprint)
{

    dmtp_command_t *command;
    sds result;

    PUBLIC_FUNC_PROLOGUE();

    if(!address) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(!fingerprint) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(!(command = dmtp_command_create(DMTP_VRFY))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to create dmtp vrfy command struct");
        goto error;
    }

    if(!(command->args[0] = sdsdup(address))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add address argument to vrfy command");
        goto cleanup_command;
    }

    if(!(command->args[2] = sdsdup(fingerprint))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add fingerprint argument to vrfy command");
        goto cleanup_command;
    }

    if(!(result = dmtp_command_format(command))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to format the vrfy command");
        goto cleanup_command;
    }

    dmtp_command_destroy(command);

    return result;

cleanup_command:
    dmtp_command_destroy(command);
error:
    return NULL;
}


/**
 * @brief
 * Create a string containing dmtp VRFY command with specified arguments.
 * @param domain
 * sds string containing domain name.
 * @param start
 * sds string containing organizational signet fingerprint.
 * @return
 * sds string containing the desired command.
 * @free_using
 * sdsfree
*/
sds
dime_dmtp_command_vrfy_domain(
    sds domain,
    sds fingerprint)
{

    dmtp_command_t *command;
    sds result;

    PUBLIC_FUNC_PROLOGUE();

    if(!domain) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(!fingerprint) {
        PUSH_ERROR(ERR_BAD_PARAM, NULL);
        goto error;
    }

    if(!(command = dmtp_command_create(DMTP_VRFY))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to create dmtp vrfy command struct");
        goto error;
    }

    if(!(command->args[1] = sdsdup(domain))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add domain argument to vrfy command");
        goto cleanup_command;
    }

    if(!(command->args[2] = sdsdup(fingerprint))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to add fingerprint argument to vrfy command");
        goto cleanup_command;
    }

    if(!(result = dmtp_command_format(command))) {
        PUSH_ERROR(ERR_UNSPEC, "failed to format the vrfy command");
        goto cleanup_command;
    }

    dmtp_command_destroy(command);

    return result;

cleanup_command:
    dmtp_command_destroy(command);
error:
    return NULL;
}



dmtp_command_key_t dmtp_command_list[DMTP_COMMANDS_NUM] = {
//  {   .com_name,   .com_name_len,
//      {
//          {.arg_name,         .arg_name_len, .type             .size },
//          {.arg_name,         .arg_name_len, .type             .size },
//          {.arg_name,         .arg_name_len, .type             .size }
//       }
//  }
    {
        "STARTTLS",  8,
        {
            { "HOST",           4,             DMTP_ARG_ANGULAR_BRCKT, 0 },
            { "MODE",           4,             DMTP_ARG_PLAIN,   0 },
            DMTP_EMPTY_ARG
        }
    },

    {
        "HELO",      4,
        {
            { "HOST",           4,             DMTP_ARG_ANGULAR_BRCKT, 0 },
            DMTP_EMPTY_ARG,
            DMTP_EMPTY_ARG
        }
    },

    {
        "EHLO",      4,
        {
            { "HOST",           4,             DMTP_ARG_ANGULAR_BRCKT, 0 },
            DMTP_EMPTY_ARG,
            DMTP_EMPTY_ARG
        }
    },

    {
        "MODE",      4,
        {
            DMTP_EMPTY_ARG,
            DMTP_EMPTY_ARG,
            DMTP_EMPTY_ARG
        }
    },

    {
        "RSET",      4,
        {
            DMTP_EMPTY_ARG,
            DMTP_EMPTY_ARG,
            DMTP_EMPTY_ARG
        }
    },

    {
        "NOOP",      4,
        {
            DMTP_IGNORE_ARG,
            DMTP_IGNORE_ARG,
            DMTP_IGNORE_ARG
        }
    },

    {
        "HELP",      4,
        {
            DMTP_EMPTY_ARG,
            DMTP_EMPTY_ARG,
            DMTP_EMPTY_ARG
        }
    },

    {
        "QUIT",      4,
        {
            DMTP_EMPTY_ARG,
            DMTP_EMPTY_ARG,
            DMTP_EMPTY_ARG
        }
    },

    {
        "MAIL",      4,
        {
            { "FROM",           4,             DMTP_ARG_ANGULAR_BRCKT, 0 },
            { "FINGERPRINT",   11,             DMTP_ARG_SQUARE_BRCKT, 0 },
            DMTP_EMPTY_ARG
        }
    },

    {
        "RCPT",      4,
        {
            { "TO",             2,             DMTP_ARG_ANGULAR_BRCKT, 0 },
            { "FINGERPRINT",   11,             DMTP_ARG_SQUARE_BRCKT, 0 },
            DMTP_EMPTY_ARG
        }
    },

    {
        "DATA",      4,
        {
            DMTP_EMPTY_ARG,
            DMTP_EMPTY_ARG,
            DMTP_EMPTY_ARG
        }
    },

    {
        "SGNT",      4,
        {
            { "USER",           4,             DMTP_ARG_ANGULAR_BRCKT, 0 },
            { "DOMAIN",         6,             DMTP_ARG_ANGULAR_BRCKT, 0 },
            { "FINGERPRINT",   11,             DMTP_ARG_SQUARE_BRCKT, 0 }
        }
    },

    {
        "HIST",      4,
        {
            { "USER",           4,             DMTP_ARG_ANGULAR_BRCKT, 0 },
            { "START",          5,             DMTP_ARG_SQUARE_BRCKT, 0 },
            { "STOP",           4,             DMTP_ARG_SQUARE_BRCKT, 0 }
        }
    },

    {
        "VRFY",      4,
        {
            { "USER",           4,             DMTP_ARG_ANGULAR_BRCKT, 0 },
            { "DOMAIN",         6,             DMTP_ARG_ANGULAR_BRCKT, 0 },
            { "FINGERPRINT",   11,             DMTP_ARG_SQUARE_BRCKT, 0 }
        }
    }

};
