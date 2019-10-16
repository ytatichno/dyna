#include "gtest/gtest.h"
#include "Context.h"

using namespace dyna;

TEST(AddrInfoTest, Constructor)
{
  //test cases:
  //1) AddrInfo(AT_READ, iter);
  //2) AddrInfo(AT_WRITE, iter);
  const int iter = 13;
  AddrInfo ai1(AT_READ, iter);
  EXPECT_FALSE(ai1.is_private);
  EXPECT_EQ(ai1.first_read_iter, iter);
  EXPECT_NE(ai1.first_write_iter, iter);
  EXPECT_EQ(ai1.last_read_iter, iter);
  EXPECT_NE(ai1.last_write_iter, iter);
  EXPECT_LE(ai1.max_fwd_dep, 0);
  EXPECT_LE(ai1.max_rev_dep, 0);
  EXPECT_GT(ai1.min_fwd_dep, 1000);
  EXPECT_GT(ai1.min_rev_dep, 1000);

  AddrInfo ai2(AT_WRITE, iter);
  EXPECT_TRUE(ai2.is_private);
  EXPECT_NE(ai2.first_read_iter, iter);
  EXPECT_EQ(ai2.first_write_iter, iter);
  EXPECT_NE(ai2.last_read_iter, iter);
  EXPECT_EQ(ai2.last_write_iter, iter);
  EXPECT_LE(ai2.max_fwd_dep, 0);
  EXPECT_LE(ai2.max_rev_dep, 0);
  EXPECT_GT(ai2.min_fwd_dep, 1000);
  EXPECT_GT(ai2.min_rev_dep, 1000);
}

TEST(AddrInfoTest, CopyConstructor)
{
  //test cases:
  //1) AddrInfo ai = AddrInfo(AT_READ, iter);
  //2) AddrInfo ai = AddrInfo(AT_WRITE, iter);
  const int iter = 13;
  AddrInfo ai1 = AddrInfo(AT_READ, iter);
  EXPECT_FALSE(ai1.is_private);
  EXPECT_EQ(ai1.first_read_iter, iter);
  EXPECT_NE(ai1.first_write_iter, iter);
  EXPECT_EQ(ai1.last_read_iter, iter);
  EXPECT_NE(ai1.last_write_iter, iter);
  EXPECT_LE(ai1.max_fwd_dep, 0);
  EXPECT_LE(ai1.max_rev_dep, 0);
  EXPECT_GT(ai1.min_fwd_dep, 1000);
  EXPECT_GT(ai1.min_rev_dep, 1000);

  AddrInfo ai2 = AddrInfo(AT_WRITE, iter);
  EXPECT_TRUE(ai2.is_private);
  EXPECT_NE(ai2.first_read_iter, iter);
  EXPECT_EQ(ai2.first_write_iter, iter);
  EXPECT_NE(ai2.last_read_iter, iter);
  EXPECT_EQ(ai2.last_write_iter, iter);
  EXPECT_LE(ai2.max_fwd_dep, 0);
  EXPECT_LE(ai2.max_rev_dep, 0);
  EXPECT_GT(ai2.min_fwd_dep, 1000);
  EXPECT_GT(ai2.min_rev_dep, 1000);
}

TEST(AddrInfoTest, Assignment)
{
  //test cases:
  //1) ai = AddrInfo(AT_READ, iter);
  //2) ai = AddrInfo(AT_WRITE, iter);
  const int iter = 13;
  AddrInfo ai1(AT_WRITE, iter + 100);
  ai1 = AddrInfo(AT_READ, iter);
  EXPECT_FALSE(ai1.is_private);
  EXPECT_EQ(ai1.first_read_iter, iter);
  EXPECT_NE(ai1.first_write_iter, iter);
  EXPECT_EQ(ai1.last_read_iter, iter);
  EXPECT_NE(ai1.last_write_iter, iter);
  EXPECT_LE(ai1.max_fwd_dep, 0);
  EXPECT_LE(ai1.max_rev_dep, 0);
  EXPECT_GT(ai1.min_fwd_dep, 1000);
  EXPECT_GT(ai1.min_rev_dep, 1000);

  AddrInfo ai2 = AddrInfo(AT_READ, iter + 3);
  ai2 = AddrInfo(AT_WRITE, iter);
  EXPECT_TRUE(ai2.is_private);
  EXPECT_NE(ai2.first_read_iter, iter);
  EXPECT_EQ(ai2.first_write_iter, iter);
  EXPECT_NE(ai2.last_read_iter, iter);
  EXPECT_EQ(ai2.last_write_iter, iter);
  EXPECT_LE(ai2.max_fwd_dep, 0);
  EXPECT_LE(ai2.max_rev_dep, 0);
  EXPECT_GT(ai2.min_fwd_dep, 1000);
  EXPECT_GT(ai2.min_rev_dep, 1000);
}

TEST(AddrInfoTest, ConstructorFromExistingObject)
{
  const int iter = 42;
  AddrInfo ai1(AT_READ, 13);
  AddrInfo ai2(AT_WRITE, 13);

  AddrInfo ai3(ai1, iter);
  EXPECT_FALSE(ai3.is_private);
  EXPECT_EQ(ai3.first_read_iter, iter);
  EXPECT_NE(ai3.first_write_iter, iter);
  EXPECT_EQ(ai3.last_read_iter, iter);
  EXPECT_NE(ai3.last_write_iter, iter);
  EXPECT_LE(ai3.max_fwd_dep, 0);
  EXPECT_LE(ai3.max_rev_dep, 0);
  EXPECT_GT(ai3.min_fwd_dep, 1000);
  EXPECT_GT(ai3.min_rev_dep, 1000);

  AddrInfo ai4(ai2, iter);
  EXPECT_TRUE(ai4.is_private);
  EXPECT_NE(ai4.first_read_iter, iter);
  EXPECT_EQ(ai4.first_write_iter, iter);
  EXPECT_NE(ai4.last_read_iter, iter);
  EXPECT_EQ(ai4.last_write_iter, iter);
  EXPECT_LE(ai4.max_fwd_dep, 0);
  EXPECT_LE(ai4.max_rev_dep, 0);
  EXPECT_GT(ai4.min_fwd_dep, 1000);
  EXPECT_GT(ai4.min_rev_dep, 1000);
}

TEST(AddrInfoTest, DISABLED_ConstructorUnknownAccess)
{
  const int iter = 13;
  AddrInfo ai1(AT_UNKNOWN, iter);
  EXPECT_FALSE(ai1.is_private);
  //EXPECT_EQ(ai1.first_read_iter, iter);
  //EXPECT_EQ(ai1.first_write_iter, iter);
  //EXPECT_EQ(ai1.last_read_iter, iter);
  //EXPECT_EQ(ai1.last_write_iter, iter);

  AddrInfo ai2(ai1, 42);
  //...
}

TEST(AddrInfoTest, AddAccess1RevDep)
{
  //test cases:
  // AddrInfo ai(AT_READ, iter);
  // ai.add_access(AT_WRITE, iter+2);
  // ai.add_access(AT_WRITE, iter+3);
  const int iter = 13;
  AddrInfo ai(AT_READ, iter);

  ai.add_access(AT_WRITE, iter);
  EXPECT_FALSE(ai.is_private);
  EXPECT_EQ(ai.first_write_iter, iter);
  EXPECT_EQ(ai.last_write_iter, iter);

  int i = iter + 2;
  ai.add_access(AT_WRITE, i);
  EXPECT_FALSE(ai.is_private);
  EXPECT_EQ(ai.first_write_iter, iter);
  EXPECT_EQ(ai.last_write_iter, i);
  EXPECT_EQ(ai.first_read_iter, iter);
  EXPECT_EQ(ai.last_read_iter, iter);
  EXPECT_EQ(ai.min_rev_dep, 2);
  EXPECT_EQ(ai.max_rev_dep, 2);
  //"WRITE after WRITE" dependency is not considered by the analyser.
  EXPECT_GT(ai.min_fwd_dep, 1000);
  EXPECT_LE(ai.max_fwd_dep, 0);

  ai.add_access(AT_WRITE, ++i);
  EXPECT_EQ(ai.first_write_iter, iter);
  EXPECT_EQ(ai.last_write_iter, i);
  EXPECT_EQ(ai.min_rev_dep, 2);
  EXPECT_EQ(ai.max_rev_dep, 3);
}

TEST(AddrInfoTest, AddAccess1FwdDep)
{
  //test cases:
  // AddrInfo ai(AT_WRITE, iter);
  // ai.add_access(AT_READ, iter+2);
  // ai.add_access(AT_READ, iter+3);
  const int iter = 13;
  AddrInfo ai(AT_WRITE, iter);

  ai.add_access(AT_READ, iter);
  EXPECT_TRUE(ai.is_private); //< read after write on the same iteration.
  EXPECT_EQ(ai.first_read_iter, iter);
  EXPECT_EQ(ai.last_read_iter, iter);

  int i = iter + 2;
  ai.add_access(AT_READ, i);
  EXPECT_FALSE(ai.is_private); //< only read access on this iteration.
  EXPECT_EQ(ai.first_read_iter, iter);
  EXPECT_EQ(ai.last_read_iter, i);
  EXPECT_EQ(ai.first_write_iter, iter);
  EXPECT_EQ(ai.last_write_iter, iter);
  EXPECT_EQ(ai.min_fwd_dep, 2);
  EXPECT_EQ(ai.max_fwd_dep, 2);
  EXPECT_GT(ai.min_rev_dep, 1000);
  EXPECT_LE(ai.max_rev_dep, 0);

  ai.add_access(AT_READ, ++i);
  EXPECT_EQ(ai.first_read_iter, iter);
  EXPECT_EQ(ai.last_read_iter, i);
  EXPECT_EQ(ai.min_fwd_dep, 2);
  EXPECT_EQ(ai.max_fwd_dep, 3);
}

TEST(AddrInfoTest, AddAccess2)
{
  //TODO: Write test for: void dyna::AddrInfo::add_access(const dyna::AddrInfo &ai, long iter)
}
