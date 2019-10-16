#include "gtest/gtest.h"
#include "Context.h"

using namespace dyna;

class ContextTest : public ::testing::Test
{
  struct CSStorage
  {
    FunctionString* cs_func1;
    FunctionString* cs_func2;
    LoopString* cs_loop1;
    LoopString* cs_loop2;
    VariableString* cs_var1;
    VariableString* cs_var2;
    VariableString* cs_arr1;
    VariableString* cs_arr2;
    CSStorage()
    {
      m_css.AddString((void*&)cs_func1, "type=function*file=jac.fdv*line1=2*line2=4*name1=main*vtype=42*rank=0**");
      m_css.AddString((void*&)cs_func2, "type=function*file=jac.fdv*line1=100*line2=200*name1=bar*vtype=42*rank=0**");
      m_css.AddString((void*&)cs_loop1, "46*type=seqloop*file=jac.fdv*line1=32*line2=35**");
      m_css.AddString((void*&)cs_loop2, "46*type=seqloop*file=jac.fdv*line1=36*line2=37**");
      m_css.AddString((void*&)cs_var1, "90*type=var_name*file=jac.fdv*line1=3*name1=eps*vtype=3*isindata=0*isincommon=0*isinsave=0**");
      m_css.AddString((void*&)cs_var2, "90*type=var_name*file=jac.fdv*line1=3*name1=lol*vtype=3*isindata=0*isincommon=0*isinsave=0**");
      m_css.AddString((void*&)cs_arr1, "95*type=arr_name*file=jac.fdv*line1=3*name1=a*vtype=3*rank=2*isindata=0*isincommon=0*isinsave=0**");
      m_css.AddString((void*&)cs_arr2, "95*type=arr_name*file=jac.fdv*line1=3*name1=b*vtype=3*rank=2*isindata=0*isincommon=0*isinsave=0**");
    }
  private:
    ContextStringsStore m_css;
  };

protected:
  static const CSStorage& css() { static CSStorage ret; return ret; }
  void SetUp()
  {
    ASSERT_NE(css().cs_func1, nullptr);
    ASSERT_NE(css().cs_loop1, nullptr);
    ASSERT_NE(css().cs_var1, nullptr);
    ASSERT_NE(css().cs_arr1, nullptr);
  }
};

TEST_F(ContextTest, Constructor)
{
  // test cases:
  // 1) Context c1(descr, 0)
  // 2) c1.description()
  // 3) Context c2(descr2, c1)
  Context cnt(css().cs_func1, 0);
  ASSERT_NE(cnt.description(), nullptr);
  const FunctionString* descr = dynamic_cast<const FunctionString*>(cnt.description());
  EXPECT_EQ(descr, css().cs_func1);

  Context cnt2(css().cs_loop1, &cnt);
  EXPECT_EQ(dynamic_cast<const LoopString*>(cnt2.description()), css().cs_loop1);
}

TEST_F(ContextTest, RegisterAccessFunc)
{
  // test cases:
  // 1) c.set_current_iteration(iter);
  // 2) c.register_access(addr1, AT_WRITE, var1);
  // 3) c.begin();
  // 4) c.end();
  // 5) c.register_access(addr2, AT_READ, arr1);
  // 6) c.get_var_descr(...)
  long iter = 13;
  const long addr1 = 42;
  const long addr2 = 100;
  Context func1(css().cs_func1, 0);
  func1.set_current_iteration(iter);
  func1.register_access(addr1, AT_WRITE, css().cs_var1);
  Context::AccessMap am(func1.begin(), func1.end());
  ASSERT_EQ(am.size(), 1);
  auto it = am.find(addr1);
  ASSERT_TRUE(it != am.end());
  EXPECT_EQ(it->second->last_write_iter, iter);

  iter++;
  func1.set_current_iteration(iter);
  func1.register_access(addr1, AT_WRITE, css().cs_var1);
  for (int i = 0; i < 3; i++)
    func1.register_access(addr2 + i, AT_READ, css().cs_arr1);
  am = Context::AccessMap(func1.begin(), func1.end());
  ASSERT_EQ(am.size(), 4);
  it = am.find(addr1);
  ASSERT_TRUE(it != am.end());
  EXPECT_EQ(it->second->last_write_iter, iter);
  it = am.find(addr2);
  ASSERT_TRUE(it != am.end());
  EXPECT_EQ(it->second->last_read_iter, iter);

  EXPECT_EQ(func1.get_var_descr(addr1  ), css().cs_var1);
  EXPECT_EQ(func1.get_var_descr(addr2  ), css().cs_arr1);
  EXPECT_EQ(func1.get_var_descr(addr2+1), css().cs_arr1);
  EXPECT_EQ(func1.get_var_descr(addr2+2), css().cs_arr1);
}

TEST_F(ContextTest, RegisterAccessLoop)
{
  // test cases:
  // 1) c.set_current_iteration(iter);
  // 2) c.register_access(addr1, AT_WRITE, var1);
  // 3) c.begin();
  // 4) c.end();
  // 5) c.register_access(addr2, AT_READ, arr1);
  // 6) c.get_var_descr(...)
  long iter = 13;
  const long addr1 = 42;
  const long addr2 = 100;
  Context func1(css().cs_func1, 0);
  Context loop1(css().cs_loop1, &func1);
  loop1.set_current_iteration(iter);
  loop1.register_access(addr1, AT_WRITE, css().cs_var1);
  Context::AccessMap am(loop1.begin(), loop1.end());
  ASSERT_EQ(am.size(), 1);
  auto it = am.find(addr1);
  ASSERT_TRUE(it != am.end());
  EXPECT_EQ(it->second->last_write_iter, iter);

  iter++;
  loop1.set_current_iteration(iter);
  loop1.register_access(addr1, AT_WRITE, css().cs_var1);
  for (int i = 0; i < 3; i++)
    loop1.register_access(addr2 + i, AT_READ, css().cs_arr1);
  am = Context::AccessMap(loop1.begin(), loop1.end());
  ASSERT_EQ(am.size(), 4);
  it = am.find(addr1);
  ASSERT_TRUE(it != am.end());
  EXPECT_EQ(it->second->last_write_iter, iter);
  it = am.find(addr2);
  ASSERT_TRUE(it != am.end());
  EXPECT_EQ(it->second->last_read_iter, iter);

  EXPECT_EQ(loop1.get_var_descr(addr1), css().cs_var1);
  EXPECT_EQ(loop1.get_var_descr(addr2), css().cs_arr1);
  EXPECT_EQ(loop1.get_var_descr(addr2+1), css().cs_arr1);
  EXPECT_EQ(loop1.get_var_descr(addr2+2), css().cs_arr1);
}

TEST_F(ContextTest, MergeLoopIntoFunction)
{
  // test cases:
  // 1) //loop1 contains {[addr1, var1], [addr2, arr1], [addr2 + 1, arr1], [addr2 + 2, arr1]}
  //    loop1.merge_into(func1);
  long iter = 13;
  const long addr1 = 42;
  const long addr2 = 100;
  Context func1(css().cs_func1, 0);
  Context loop1(css().cs_loop1, &func1);
  loop1.set_current_iteration(iter);
  loop1.register_access(addr1, AT_WRITE, css().cs_var1);
  loop1.set_current_iteration(++iter);
  loop1.register_access(addr1, AT_WRITE, css().cs_var1);
  for (int i = 0; i < 3; i++)
    loop1.register_access(addr2 + i, AT_READ, css().cs_arr1);
  loop1.merge_into(&func1);

  AddrInfo ai_read (AT_READ , 1);
  AddrInfo ai_write(AT_WRITE, 1);
  Context::AccessMap am(func1.begin(), func1.end());
  ASSERT_EQ(am.size(), 4);
  auto it = am.find(addr1);
  ASSERT_TRUE(it != am.end());
  EXPECT_EQ(it->second->last_read_iter , ai_write.last_read_iter);
  EXPECT_NE(it->second->last_write_iter, ai_read.last_write_iter);
  it = am.find(addr2+1);
  ASSERT_TRUE(it != am.end());
  EXPECT_NE(it->second->last_read_iter , ai_write.last_read_iter);
  EXPECT_EQ(it->second->last_write_iter, ai_read.last_write_iter);

  EXPECT_EQ(func1.get_var_descr(addr1  ), css().cs_var1);
  EXPECT_EQ(func1.get_var_descr(addr2  ), css().cs_arr1);
  EXPECT_EQ(func1.get_var_descr(addr2+1), css().cs_arr1);
  EXPECT_EQ(func1.get_var_descr(addr2+2), css().cs_arr1);
}

TEST_F(ContextTest, MergeLoopIntoLoop)
{
  // test cases:
  // 1) //loop1 contains {[addr1, var1], [addr2, arr1], [addr2 + 1, arr1], [addr2 + 2, arr1]}
  //    //loop2 contains {[addr1, var1]}
  //    loop1.merge_into(loop2);
  long iter = 13;
  const long addr1 = 42;
  const long addr2 = 100;
  Context func1(css().cs_func1, 0);
  Context loop1(css().cs_loop1, &func1);
  Context loop2(css().cs_loop2, &loop1);
  loop1.set_current_iteration(iter);
  loop1.register_access(addr1, AT_WRITE, css().cs_var1);
  loop1.set_current_iteration(++iter);

  loop2.set_current_iteration(iter+100);
  loop2.register_access(addr1, AT_WRITE, css().cs_var1);
  loop2.register_access(addr1, AT_READ , css().cs_var1);
  for (int i = 0; i < 3; i++)
    loop2.register_access(addr2 + i, AT_READ, css().cs_arr1);
  loop2.merge_into(&loop1);

  AddrInfo ai_read (AT_READ , 1);
  Context::AccessMap am(loop1.begin(), loop1.end());
  ASSERT_EQ(am.size(), 4);
  auto it = am.find(addr1);
  ASSERT_TRUE(it != am.end());
  EXPECT_EQ(it->second->last_read_iter , iter);
  EXPECT_EQ(it->second->last_write_iter, iter);
  EXPECT_EQ(it->second->first_write_iter, iter-1);
  it = am.find(addr2+1);
  ASSERT_TRUE(it != am.end());
  EXPECT_EQ(it->second->last_read_iter , iter);
  EXPECT_EQ(it->second->last_write_iter, ai_read.last_write_iter);

  EXPECT_EQ(loop1.get_var_descr(addr1  ), css().cs_var1);
  EXPECT_EQ(loop1.get_var_descr(addr2  ), css().cs_arr1);
  EXPECT_EQ(loop1.get_var_descr(addr2+1), css().cs_arr1);
  EXPECT_EQ(loop1.get_var_descr(addr2+2), css().cs_arr1);
}

TEST_F(ContextTest, MergeFunctionIntoLoop)
{
  // test cases:
  // 1) //loop1 contains {[addr1, var1]}
  //    //func2 contains {[addr1, var2], [addr2, arr1], [addr2 + 1, arr1], [addr2 + 2, arr1]}
  //    func2.merge_into(loop1);
  long iter = 13;
  const long addr1 = 42;
  const long addr2 = 100;
  Context func1(css().cs_func1, 0);
  Context loop1(css().cs_loop1, &func1);
  Context func2(css().cs_func2, 0);
  loop1.set_current_iteration(iter);
  loop1.register_access(addr1, AT_WRITE, css().cs_var1);
  loop1.set_current_iteration(++iter);

  func2.register_access(addr1, AT_WRITE, css().cs_var2);
  func2.register_access(addr1, AT_READ , css().cs_var2);
  for (int i = 0; i < 3; i++)
    func2.register_access(addr2 + i, AT_WRITE, css().cs_arr1);
  func2.merge_into(&loop1);

  AddrInfo ai_read (AT_READ , 1);
  AddrInfo ai_write(AT_WRITE, 1);
  Context::AccessMap am(loop1.begin(), loop1.end());
  ASSERT_EQ(am.size(), 4);
  auto it = am.find(addr1);
  ASSERT_TRUE(it != am.end());
  EXPECT_EQ(it->second->last_read_iter , iter);
  EXPECT_EQ(it->second->last_write_iter, iter);
  EXPECT_EQ(it->second->first_write_iter, iter-1);
  it = am.find(addr2+1);
  ASSERT_TRUE(it != am.end());
  EXPECT_EQ(it->second->last_read_iter , ai_write.last_read_iter);
  EXPECT_EQ(it->second->last_write_iter, iter);

  EXPECT_EQ(loop1.get_var_descr(addr1  ), css().cs_var1);
#if CONTEXT_FUNCTION_PROPAGATE_DESCRIPTIONS
  EXPECT_EQ(loop1.get_var_descr(addr2  ), css().cs_arr1);
  EXPECT_EQ(loop1.get_var_descr(addr2+1), css().cs_arr1);
  EXPECT_EQ(loop1.get_var_descr(addr2+2), css().cs_arr1);
#else
  EXPECT_EQ(loop1.get_var_descr(addr2  ), Context::unknown_var());
  EXPECT_EQ(loop1.get_var_descr(addr2+1), Context::unknown_var());
  EXPECT_EQ(loop1.get_var_descr(addr2+2), Context::unknown_var());
#endif
}

TEST_F(ContextTest, RegisterVariable)
{
  // test cases:
  // 1) func1.register_variable(addr1, var1);
  //    //loop1(&func1)
  //    //func2 contains {[addr1, var2]}
  //    func2.merge_into(loop1);
  long iter = 13;
  const long addr1 = 42;
  Context func1(css().cs_func1, 0);
  Context loop1(css().cs_loop1, &func1);
  Context func2(css().cs_func2, 0);
  func1.register_variable(addr1, css().cs_var1);
  loop1.set_current_iteration(iter);

  func2.register_access(addr1, AT_WRITE, css().cs_var2);
  func2.register_access(addr1, AT_READ , css().cs_var2);
  func2.merge_into(&loop1);

  Context::AccessMap am(loop1.begin(), loop1.end());
  ASSERT_EQ(am.size(), 1);
  auto it = am.find(addr1);
  ASSERT_TRUE(it != am.end());
  EXPECT_EQ(it->second->last_read_iter , iter);
  EXPECT_EQ(it->second->last_write_iter, iter);
  EXPECT_EQ(loop1.get_var_descr(addr1), css().cs_var1);
}

TEST_F(ContextTest, RegisterArray)
{
  // test cases:
  // 1) func1.register_array(AddrRange(addr1,addr1+1), arr1);
  //    //loop1(&func1)
  //    //func2 contains {[addr1, arr2], [addr1+1, arr2], [addr1+2], arr2]}
  //    func2.merge_into(loop1);
  long iter = 13;
  const long addr1 = 100;
  Context func1(css().cs_func1, 0);
  Context loop1(css().cs_loop1, &func1);
  Context func2(css().cs_func2, 0);
  func1.register_array(AddrRange(addr1+1, addr1+1), css().cs_arr1);
  loop1.set_current_iteration(iter);

  func2.register_array(AddrRange(addr1, addr1+2), css().cs_arr2);
  for (int i = 0; i < 3; i++) {
    func2.register_access(addr1+i, AT_WRITE, css().cs_arr2);
    func2.register_access(addr1+i, AT_READ , css().cs_arr2);
  }
  func2.merge_into(&loop1);

  Context::AccessMap am(loop1.begin(), loop1.end());
  ASSERT_EQ(am.size(), 3);
  auto it = am.find(addr1+1);
  ASSERT_TRUE(it != am.end());
  EXPECT_EQ(it->second->last_read_iter , iter);
  EXPECT_EQ(it->second->last_write_iter, iter);
  it = am.find(addr1+2);
  ASSERT_TRUE(it != am.end());
  EXPECT_EQ(it->second->last_read_iter , iter);
  EXPECT_EQ(it->second->last_write_iter, iter);

  EXPECT_EQ(func2.get_var_descr(addr1  ), css().cs_arr2);
  EXPECT_EQ(func2.get_var_descr(addr1+1), css().cs_arr2);
  EXPECT_EQ(func2.get_var_descr(addr1+2), css().cs_arr2);
#if CONTEXT_FUNCTION_PROPAGATE_DESCRIPTIONS
  EXPECT_EQ(loop1.get_var_descr(addr1  ), css().cs_arr2);
  EXPECT_EQ(loop1.get_var_descr(addr1+1), css().cs_arr1);
  EXPECT_EQ(loop1.get_var_descr(addr1+2), css().cs_arr2);
#else
  EXPECT_EQ(loop1.get_var_descr(addr1  ), Context::unknown_var());
  EXPECT_EQ(loop1.get_var_descr(addr1+1), css().cs_arr1);
  EXPECT_EQ(loop1.get_var_descr(addr1+2), Context::unknown_var());
#endif
}
