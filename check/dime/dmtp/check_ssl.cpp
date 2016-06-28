extern "C" {
#include "dime/signet-resolver/signet-ssl.h"
}
#include "gtest/gtest.h"

TEST(DIME, domain_wildcard) {
    ASSERT_EQ(1, _domain_wildcard_check("www.google.com", "www.google.com"));
    ASSERT_EQ(1, _domain_wildcard_check("*.google.com", "abc.google.com"));
    ASSERT_EQ(1, _domain_wildcard_check("*.google.com", "abc.def.google.com"));
    ASSERT_EQ(0, _domain_wildcard_check("*.google.com", "google.com"));
}
