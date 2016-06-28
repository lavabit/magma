extern "C" {
#include "dime/dmessage/parse.h"
}
#include "gtest/gtest.h"

TEST(DIME, check_parser_envelope)
{
    sds usrsgnt1 = sdsnew("nDjalkzxpqmviqwnrPIOSDFnasdfkadsdfa");
    sds orgfp1 = sdsnew("fasdlk;otrhnvgauisgfa;nmdg;iajgio;ewj;kaji8jetioajwetiewhyenbns");
    sds usrsgnt2 = sdsnew("newtiuanhdgfnaietnnastawetpoajweothqwtyqmvdigta");
    sds orgfp2 = sdsnew("netuiafnmadi9tawejasd'as;djgtai9wejtianmsdgna;sgaklsnqqsdkfathbnvfadsfa");
    sds usrid1 = sdsnew("abcdeffedcba");
    sds orgid1 = sdsnew("NKDLASIDFK12d");
    sds usrid2 = sdsnew("1sdfkasd@fpioasdwq");
    sds orgid2 = sdsnew("vapsdqwpiorqwrpndkd");
    sds formatted = sdsempty();
    dmime_envelope_object_t *envelope = NULL;
    int res;

    formatted = dime_prsr_envelope_format(usrid1, orgid1, usrsgnt1, orgfp1, CHUNK_TYPE_ORIGIN);
    ASSERT_TRUE(formatted != NULL) << "Failed to format origin chunk data.";

    envelope = dime_prsr_envelope_parse(formatted, sdslen(formatted), CHUNK_TYPE_DESTINATION);
    ASSERT_TRUE(envelope == NULL) << "Was able to parse an origin chunk as a destination.";

    envelope = dime_prsr_envelope_parse(formatted, sdslen(formatted), CHUNK_TYPE_ORIGIN);
    ASSERT_TRUE(envelope != NULL) << "Was unable to parse an origin chunk.";

    res = sdscmp(usrid1, envelope->auth_recp);
    ASSERT_EQ(0, res) << "Data was corrupted during formatting and parsing.";

    res = sdscmp(orgid1, envelope->dest_orig);
    ASSERT_EQ(0, res) << "Data was corrupted during formatting and parsing.";

    res = sdscmp(usrsgnt1, envelope->auth_recp_fp);
    ASSERT_EQ(0, res) << "Data was corrupted during formatting and parsing.";

    res = sdscmp(orgfp1, envelope->dest_orig_fp);
    ASSERT_EQ(0, res) << "Data was corrupted during formatting and parsing.";

    sdsfree(formatted);
    dime_prsr_envelope_destroy(envelope);

    formatted = dime_prsr_envelope_format(usrid2, orgid2, usrsgnt2, orgfp2, CHUNK_TYPE_DESTINATION);
    ASSERT_TRUE(formatted != NULL) << "Failed to format destination chunk data.";

    envelope = dime_prsr_envelope_parse(formatted, sdslen(formatted), CHUNK_TYPE_ORIGIN);
    ASSERT_TRUE(envelope == NULL) << "Was able to parse an destination chunk as a origin.";

    envelope = dime_prsr_envelope_parse(formatted, sdslen(formatted), CHUNK_TYPE_DESTINATION);
    ASSERT_TRUE(envelope != NULL) << "Was unable to parse an destination chunk.";

    res = sdscmp(usrid2, envelope->auth_recp);
    ASSERT_EQ(0, res) << "Data was corrupted during formatting and parsing.";

    res = sdscmp(orgid2, envelope->dest_orig);
    ASSERT_EQ(0, res) << "Data was corrupted during formatting and parsing.";

    res = sdscmp(usrsgnt2, envelope->auth_recp_fp);
    ASSERT_EQ(0, res) << "Data was corrupted during formatting and parsing.";

    res = sdscmp(orgfp2, envelope->dest_orig_fp);
    ASSERT_EQ(0, res) << "Data was corrupted during formatting and parsing.";

    sdsfree(formatted);
    dime_prsr_envelope_destroy(envelope);

    formatted = dime_prsr_envelope_format(usrid2, orgid2, usrsgnt2, orgfp2, CHUNK_TYPE_EPHEMERAL);
    ASSERT_TRUE(formatted == NULL) << "Failed to format destination chunk data.";

    sdsfree(usrsgnt1);
    sdsfree(orgfp1);
    sdsfree(usrsgnt2);
    sdsfree(orgfp2);
    sdsfree(usrid1);
    sdsfree(orgid1);
    sdsfree(usrid2);
    sdsfree(orgid2);
    sdsfree(formatted);
}

TEST(DIME, check_parser_header) {

    dmime_common_headers_t *header1, *header2;
    int res = 0;
    size_t outsize;
    unsigned char *formatted;

    header1 = dime_prsr_headers_create();
    header1->headers[HEADER_TYPE_DATE] = sdsnew("11:34:12 AM March 12, 2004");
    header1->headers[HEADER_TYPE_TO] = sdsnew("abc@hello.com");
    header1->headers[HEADER_TYPE_CC] = sdsnew("a312@goodbye.com");
    header1->headers[HEADER_TYPE_FROM] = sdsnew("author@authorplace.com");
    header1->headers[HEADER_TYPE_ORGANIZATION] = sdsnew("Cool people organization");
    header1->headers[HEADER_TYPE_SUBJECT] = sdsnew("here's stuff");

    formatted = dime_prsr_headers_format(header1, &outsize);
    ASSERT_TRUE(formatted != NULL) << "Failed to format common headers.";

    header2 = dime_prsr_headers_parse(formatted, outsize);
    ASSERT_TRUE(header2 != NULL) << "Failed to parse common headers.";

    res = sdscmp(header1->headers[HEADER_TYPE_DATE], header2->headers[HEADER_TYPE_DATE]);
    ASSERT_EQ(0, res) << "Date header was corrupted.";

    res = sdscmp(header1->headers[HEADER_TYPE_TO], header2->headers[HEADER_TYPE_TO]);
    ASSERT_EQ(0, res) << "To header was corrupted.";

    res = sdscmp(header1->headers[HEADER_TYPE_CC], header2->headers[HEADER_TYPE_CC]);
    ASSERT_EQ(0, res) << "CC header was corrupted.";

    res = sdscmp(header1->headers[HEADER_TYPE_FROM], header2->headers[HEADER_TYPE_FROM]);
    ASSERT_EQ(0, res) << "From header was corrupted.";

    res = sdscmp(header1->headers[HEADER_TYPE_ORGANIZATION], header2->headers[HEADER_TYPE_ORGANIZATION]);
    ASSERT_EQ(0, res) << "Organization header was corrupted.";

    res = sdscmp(header1->headers[HEADER_TYPE_SUBJECT], header2->headers[HEADER_TYPE_SUBJECT]);
    ASSERT_EQ(0, res) << "Subject header was corrupted.";

    dime_prsr_headers_destroy(header1);
    dime_prsr_headers_destroy(header2);
    free(formatted);
}
