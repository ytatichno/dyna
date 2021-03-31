#include "gtest/gtest.h"
#include "string_utils.h"
using namespace std;

static inline const map<string,string>& vars()
{
  static map<string, string> ret = {
    {"var1", "foo"},
    {"var2", "bar"},
    {"x", "baz"},
  };
  return ret;
}

TEST(StringUtils, SubstituteVars)
{
  EXPECT_EQ(string_substitute_vars("abc", vars()), "abc");
  EXPECT_EQ(string_substitute_vars("${x}", vars()), "baz");
  EXPECT_EQ(string_substitute_vars("\\${x}", vars()), "${x}");
  EXPECT_EQ(string_substitute_vars("${abc}", vars()), "");
  EXPECT_EQ(string_substitute_vars("${var1}${var2}", vars()), "foobar");
  EXPECT_EQ(string_substitute_vars("abc${var1}def", vars()), "abcfoodef");

  EXPECT_EQ(string_substitute_vars("${var1", vars()), "${var1");

  EXPECT_EQ(string_substitute_vars("$var1", vars()), "$var1");
  EXPECT_EQ(string_substitute_vars("$var1$foo$x${x}$bar", vars()), "$var1$foo$xbaz$bar");
}
