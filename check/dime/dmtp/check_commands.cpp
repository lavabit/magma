extern "C" {
#include "dime/dmtp/commands.h"
#include "dime/common/error.h"
}
#include "gtest/gtest.h"

/**
 * Command parsing and formatting
 */
TEST(DIME, dmtp_command_formatting_and_parsing)
{
    dmtp_command_t *command, *parsed;
    dmtp_parse_state_t state;
    int res;
    sds formatted;

    //STARTTLS
    command = dime_dmtp_command_create(DMTP_STARTTLS);
    ASSERT_TRUE(command != NULL) << "Failed to create DMTP_STARTTLS command.";

    command->args[0] = sdsnew("somehost");
    command->args[1] = sdsnew("somemode");

    formatted = dime_dmtp_command_format(command);
    ASSERT_TRUE(command != NULL) << "Failed to format DMTP_STARTTLS command.";

    state = dime_dmtp_command_parse(formatted, &parsed);
    ASSERT_TRUE(command != NULL) << "Failed to parse DMTP_STARTTLS command.";
    ASSERT_TRUE(state == DMTP_PARSE_SUCCESS) << "Command parsing returned a fail state.";

    res = sdscmp(command->args[0], parsed->args[0]);
    ASSERT_EQ(0, res) << "Argument 1 data formatting and parsing DMTP_STARTTLS command.";

    res = sdscmp(command->args[1], parsed->args[1]);
    ASSERT_EQ(0, res) << "Argument 2 data formatting and parsing DMTP_STARTTLS command.";

    sdsfree(formatted);
    dime_dmtp_command_destroy(command);
    dime_dmtp_command_destroy(parsed);


    //HELO
    command = dime_dmtp_command_create(DMTP_HELO);
    ASSERT_TRUE(command != NULL) << "Failed to create DMTP_HELO command.";

    command->args[0] = sdsnew("somehost");

    formatted = dime_dmtp_command_format(command);
    ASSERT_TRUE(command != NULL) << "Failed to format DMTP_HELO command.";

    state = dime_dmtp_command_parse(formatted, &parsed);
    ASSERT_TRUE(command != NULL) << "Failed to parse DMTP_HELO command.";
    ASSERT_TRUE(state == DMTP_PARSE_SUCCESS) << "Command parsing returned a fail state.";

    res = sdscmp(command->args[0], parsed->args[0]);
    ASSERT_EQ(0, res) << "Argument 1 data formatting and parsing DMTP_HELO command.";

    sdsfree(formatted);
    dime_dmtp_command_destroy(command);
    dime_dmtp_command_destroy(parsed);



    //EHLO
    command = dime_dmtp_command_create(DMTP_EHLO);
    ASSERT_TRUE(command != NULL) << "Failed to create DMTP_EHLO command.";

    command->args[0] = sdsnew("somehost");

    formatted = dime_dmtp_command_format(command);
    ASSERT_TRUE(command != NULL) << "Failed to format DMTP_EHLO command.";

    state = dime_dmtp_command_parse(formatted, &parsed);
    ASSERT_TRUE(command != NULL) << "Failed to parse DMTP_EHLO command.";
    ASSERT_TRUE(state == DMTP_PARSE_SUCCESS) << "Command parsing returned a fail state.";

    res = sdscmp(command->args[0], parsed->args[0]);
    ASSERT_EQ(0, res) << "Argument 1 data formatting and parsing DMTP_EHLO command.";

    sdsfree(formatted);
    dime_dmtp_command_destroy(command);
    dime_dmtp_command_destroy(parsed);



    //MODE
    command = dime_dmtp_command_create(DMTP_MODE);
    ASSERT_TRUE(command != NULL) << "Failed to create DMTP_MODE command.";

    formatted = dime_dmtp_command_format(command);
    ASSERT_TRUE(command != NULL) << "Failed to format DMTP_MODE command.";

    state = dime_dmtp_command_parse(formatted, &parsed);
    ASSERT_TRUE(command != NULL) << "Failed to parse DMTP_MODE command.";

    sdsfree(formatted);
    dime_dmtp_command_destroy(command);
    dime_dmtp_command_destroy(parsed);


    //RSET
    command = dime_dmtp_command_create(DMTP_RSET);
    ASSERT_TRUE(command != NULL) << "Failed to create DMTP_RSET command.";

    formatted = dime_dmtp_command_format(command);
    ASSERT_TRUE(command != NULL) << "Failed to format DMTP_RSET command.";

    state = dime_dmtp_command_parse(formatted, &parsed);
    ASSERT_TRUE(command != NULL) << "Failed to parse DMTP_RSET command.";
    ASSERT_TRUE(state == DMTP_PARSE_SUCCESS) << "Command parsing returned a fail state.";

    sdsfree(formatted);
    dime_dmtp_command_destroy(command);
    dime_dmtp_command_destroy(parsed);


    //NOOP
    command = dime_dmtp_command_create(DMTP_NOOP);
    ASSERT_TRUE(command != NULL) << "Failed to create DMTP_NOOP command.";

    command->args[0] = sdsnew("someargumentasdfa");
    command->args[1] = sdsnew("someotheargumentasdf");
    command->args[2] = sdsnew("asdklfja;sdfjakl;sdfjl;kj");

    formatted = dime_dmtp_command_format(command);
    ASSERT_TRUE(command != NULL) << "Failed to format DMTP_NOOP command.";

    state = dime_dmtp_command_parse(formatted, &parsed);
    ASSERT_TRUE(command != NULL) << "Failed to parse DMTP_NOOP command.";
    ASSERT_TRUE(state == DMTP_PARSE_SUCCESS) << "Command parsing returned a fail state.";

    res = sdscmp(command->args[0], parsed->args[0]);
    ASSERT_EQ(0, res) << "Argument 1 data formatting and parsing DMTP_NOOP command.";

    res = sdscmp(command->args[1], parsed->args[1]);
    ASSERT_EQ(0, res) << "Argument 2 data formatting and parsing DMTP_NOOP command.";

    res = sdscmp(command->args[2], parsed->args[2]);
    ASSERT_EQ(0, res) << "Argument 3 data formatting and parsing DMTP_NOOP command.";

    sdsfree(formatted);
    dime_dmtp_command_destroy(command);
    dime_dmtp_command_destroy(parsed);


    //HELP
    command = dime_dmtp_command_create(DMTP_HELP);
    ASSERT_TRUE(command != NULL) << "Failed to create DMTP_HELP command.";

    formatted = dime_dmtp_command_format(command);
    ASSERT_TRUE(command != NULL) << "Failed to format DMTP_HELP command.";

    state = dime_dmtp_command_parse(formatted, &parsed);
    ASSERT_TRUE(command != NULL) << "Failed to parse DMTP_HELP command.";
    ASSERT_TRUE(state == DMTP_PARSE_SUCCESS) << "Command parsing returned a fail state.";

    sdsfree(formatted);
    dime_dmtp_command_destroy(command);
    dime_dmtp_command_destroy(parsed);


    //MAIL
    command = dime_dmtp_command_create(DMTP_MAIL);
    ASSERT_TRUE(command != NULL) << "Failed to create DMTP_MAIL command.";

    command->args[0] = sdsnew("ourdomain");
    command->args[1] = sdsnew("oursignetfingerprint");

    formatted = dime_dmtp_command_format(command);
    ASSERT_TRUE(command != NULL) << "Failed to format DMTP_MAIL command.";

    state = dime_dmtp_command_parse(formatted, &parsed);
    ASSERT_TRUE(command != NULL) << "Failed to parse DMTP_MAIL command.";
    ASSERT_TRUE(state == DMTP_PARSE_SUCCESS) << "Command parsing returned a fail state.";

    res = sdscmp(command->args[0], parsed->args[0]);
    ASSERT_EQ(0, res) << "Argument 1 data formatting and parsing DMTP_MAIL command.";

    res = sdscmp(command->args[1], parsed->args[1]);
    ASSERT_EQ(0, res) << "Argument 2 data formatting and parsing DMTP_MAIL command.";

    sdsfree(formatted);
    dime_dmtp_command_destroy(command);
    dime_dmtp_command_destroy(parsed);


    //RCPT
    command = dime_dmtp_command_create(DMTP_RCPT);
    ASSERT_TRUE(command != NULL) << "Failed to create DMTP_RCPT command.";

    command->args[0] = sdsnew("theirdomain");
    command->args[1] = sdsnew("theirsignetfingerprint");

    formatted = dime_dmtp_command_format(command);
    ASSERT_TRUE(command != NULL) << "Failed to format DMTP_RCPT command.";

    state = dime_dmtp_command_parse(formatted, &parsed);
    ASSERT_TRUE(command != NULL) << "Failed to parse DMTP_RCPT command.";
    ASSERT_TRUE(state == DMTP_PARSE_SUCCESS) << "Command parsing returned a fail state.";

    res = sdscmp(command->args[0], parsed->args[0]);
    ASSERT_EQ(0, res) << "Argument 1 data formatting and parsing DMTP_RCPT command.";

    res = sdscmp(command->args[1], parsed->args[1]);
    ASSERT_EQ(0, res) << "Argument 2 data formatting and parsing DMTP_RCPT command.";

    sdsfree(formatted);
    dime_dmtp_command_destroy(command);
    dime_dmtp_command_destroy(parsed);


    //DATA
    command = dime_dmtp_command_create(DMTP_DATA);
    ASSERT_TRUE(command != NULL) << "Failed to create DMTP_DATA command.";

    formatted = dime_dmtp_command_format(command);
    ASSERT_TRUE(command != NULL) << "Failed to format DMTP_DATA command.";

    state = dime_dmtp_command_parse(formatted, &parsed);
    ASSERT_TRUE(command != NULL) << "Failed to parse DMTP_DATA command.";
    ASSERT_TRUE(state == DMTP_PARSE_SUCCESS) << "Command parsing returned a fail state.";

    sdsfree(formatted);
    dime_dmtp_command_destroy(command);
    dime_dmtp_command_destroy(parsed);


    //SGNT
    command = dime_dmtp_command_create(DMTP_SGNT);
    ASSERT_TRUE(command != NULL) << "Failed to create DMTP_SGNT command.";

    command->args[1] = sdsnew("theirdomain");
    command->args[2] = sdsnew("theirsignetfingerprint");

    formatted = dime_dmtp_command_format(command);
    ASSERT_TRUE(command != NULL) << "Failed to format DMTP_SGNT command.";

    state = dime_dmtp_command_parse(formatted, &parsed);
    ASSERT_TRUE(command != NULL) << "Failed to parse DMTP_SGNT command.";
    ASSERT_TRUE(state == DMTP_PARSE_SUCCESS) << "Command parsing returned a fail state.";

    res = sdscmp(command->args[1], parsed->args[1]);
    ASSERT_EQ(0, res) << "Argument 2 data formatting and parsing DMTP_SGNT command.";

    res = sdscmp(command->args[2], parsed->args[2]);
    ASSERT_EQ(0, res) << "Argument 3 data formatting and parsing DMTP_SGNT command.";

    sdsfree(formatted);
    dime_dmtp_command_destroy(command);
    dime_dmtp_command_destroy(parsed);

    //SGNT
    command = dime_dmtp_command_create(DMTP_SGNT);
    ASSERT_TRUE(command != NULL) << "Failed to create DMTP_SGNT command.";

    command->args[0] = sdsnew("someemail");
    command->args[2] = sdsnew("somesignetfingerprint");

    formatted = dime_dmtp_command_format(command);
    ASSERT_TRUE(command != NULL) << "Failed to format DMTP_SGNT command.";

    state = dime_dmtp_command_parse(formatted, &parsed);
    ASSERT_TRUE(command != NULL) << "Failed to parse DMTP_SGNT command.";
    ASSERT_TRUE(state == DMTP_PARSE_SUCCESS) << "Command parsing returned a fail state.";

    res = sdscmp(command->args[0], parsed->args[0]);
    ASSERT_EQ(0, res) << "Argument 1 data formatting and parsing DMTP_SGNT command.";

    res = sdscmp(command->args[2], parsed->args[2]);
    ASSERT_EQ(0, res) << "Argument 3 data formatting and parsing DMTP_SGNT command.";

    sdsfree(formatted);
    dime_dmtp_command_destroy(command);
    dime_dmtp_command_destroy(parsed);


    //HIST
    command = dime_dmtp_command_create(DMTP_HIST);
    ASSERT_TRUE(command != NULL) << "Failed to create DMTP_HIST command.";

    command->args[0] = sdsnew("someemail");
    command->args[1] = sdsnew("startingsignetfingerprint");
    command->args[2] = sdsnew("stoppingsignetfingerprint");

    formatted = dime_dmtp_command_format(command);
    ASSERT_TRUE(command != NULL) << "Failed to format DMTP_HIST command.";

    state = dime_dmtp_command_parse(formatted, &parsed);
    ASSERT_TRUE(command != NULL) << "Failed to parse DMTP_HIST command.";
    ASSERT_TRUE(state == DMTP_PARSE_SUCCESS) << "Command parsing returned a fail state.";

    res = sdscmp(command->args[0], parsed->args[0]);
    ASSERT_EQ(0, res) << "Argument 1 data formatting and parsing DMTP_HIST command.";

    res = sdscmp(command->args[1], parsed->args[1]);
    ASSERT_EQ(0, res) << "Argument 2 data formatting and parsing DMTP_HIST command.";

    res = sdscmp(command->args[2], parsed->args[2]);
    ASSERT_EQ(0, res) << "Argument 3 data formatting and parsing DMTP_HIST command.";

    sdsfree(formatted);
    dime_dmtp_command_destroy(command);
    dime_dmtp_command_destroy(parsed);


    //VRFY
    command = dime_dmtp_command_create(DMTP_VRFY);
    ASSERT_TRUE(command != NULL) << "Failed to create DMTP_VRFY command.";

    command->args[0] = sdsnew("someemail");
    command->args[2] = sdsnew("somesignetfingerprint");

    formatted = dime_dmtp_command_format(command);
    ASSERT_TRUE(command != NULL) << "Failed to format DMTP_VRFY command.";

    state = dime_dmtp_command_parse(formatted, &parsed);
    ASSERT_TRUE(command != NULL) << "Failed to parse DMTP_VRFY command.";
    ASSERT_TRUE(state == DMTP_PARSE_SUCCESS) << "Command parsing returned a fail state.";

    res = sdscmp(command->args[0], parsed->args[0]);
    ASSERT_EQ(0, res) << "Argument 1 data formatting and parsing DMTP_VRFY command.";

    res = sdscmp(command->args[2], parsed->args[2]);
    ASSERT_EQ(0, res) << "Argument 3 data formatting and parsing DMTP_VRFY command.";

    sdsfree(formatted);
    dime_dmtp_command_destroy(command);
    dime_dmtp_command_destroy(parsed);


    //VRFY
    command = dime_dmtp_command_create(DMTP_VRFY);
    ASSERT_TRUE(command != NULL) << "Failed to create DMTP_VRFY command.";

    command->args[1] = sdsnew("theirdomain");
    command->args[2] = sdsnew("theirsignetfingerprint");

    formatted = dime_dmtp_command_format(command);
    ASSERT_TRUE(command != NULL) << "Failed to format DMTP_VRFY command.";

    state = dime_dmtp_command_parse(formatted, &parsed);
    ASSERT_TRUE(command != NULL) << "Failed to parse DMTP_VRFY command.";
    ASSERT_TRUE(state == DMTP_PARSE_SUCCESS) << "Command parsing returned a fail state.";

    res = sdscmp(command->args[1], parsed->args[1]);
    ASSERT_EQ(0, res) << "Argument 2 data formatting and parsing DMTP_VRFY command.";

    res = sdscmp(command->args[2], parsed->args[2]);
    ASSERT_EQ(0, res) << "Argument 3 data formatting and parsing DMTP_VRFY command.";

    sdsfree(formatted);
    dime_dmtp_command_destroy(command);
    dime_dmtp_command_destroy(parsed);
}
