#include "ltl-compare.hh"
#include "gtest/gtest_pred_impl.h"
#include <gtest/gtest-message.h>
#include <gtest/gtest-test-part.h>
#include <string>
#include <vector>

TEST(ltlcompare, spotSemanticEquivalence) {

  std::string assFile = "../tests/input/ltl-compare-ass.txt";

  int res = semanticComparison("a U b", "a U b");
  ASSERT_EQ(res, 1);
}

