#include "AutomataBasedEvaluator.hh"
#include "ltl-compare.hh"
#include "spot/twa/bddprint.hh"
#include "spot/twa/twa.hh"
#include "spot/twa/twagraph.hh"
#include "spot/twaalgos/translate.hh"
#include "gtest/gtest_pred_impl.h"
#include <fstream>
#include <gtest/gtest-message.h>
#include <gtest/gtest-test-part.h>
#include <iostream>
#include <spot/tl/formula.hh>
#include <spot/tl/parse.hh>
#include <spot/twaalgos/isdet.hh>
#include <spot/twaalgos/postproc.hh>
#include <spot/twaalgos/word.hh>

#include <string>
#include <vector>

TEST(ltlcompare, spotSemanticEquivalence) {

  std::string assFile = "../tests/input/ltl-compare-ass.txt";
  std::ifstream file(assFile);
  if (!file) {
    std::cerr << "Errore nell aprire il file!" << std::endl;
  }
  float risultati[10] = {0, 0, 0.5, 0.5, 1, 0, 0.5, 0, 0.5, 1};
  float res;
  int i = 0;
  std::string line1, line2;
  while (std::getline(file, line1)) {
    if (std::getline(file, line2)) {
      res = semanticComparison(line1, line2);
      ASSERT_EQ(res, risultati[i]);
      i++;
    }
  }

  //int res = semanticComparison("a U b", "a U b");
  //ASSERT_EQ(res, 1);
}

std::shared_ptr<spot::twa_graph> build(const spot::formula &formula) {

  spot::formula formula_to_use = formula;

  spot::translator trans;
  trans.set_pref(spot::postprocessor::Deterministic);
  auto aut = trans.run(formula_to_use);

  spot::postprocessor post;
  post.set_pref(spot::postprocessor::Complete);
  aut = post.run(aut);

  //  messageErrorIf(!spot::is_deterministic(aut),
  //                 "The formula_to_use '" + to_string(formula_to_use) +
  //                     "' generates a non-deterministic automaton");
  //  messageErrorIf(!spot::is_complete(aut),
  //                 "The formula_to_use '" + to_string(formula_to_use) +
  //                     "' generates an incomplete automaton");
  //  messageWarningIf(
  //      aut->num_states() == 1,
  //      "The formula '" + to_string(formula_to_use) +
  //          "' generates a trivial automaton with only one state");
  return aut;
}
TEST(ltlcompare, jaccard) {

  std::shared_ptr<spot::twa_graph> aut1 =
      //build(spot::parse_infix_psl("{a ##1 b ##1 c} & {w ##1 x ##1 y ##1 z}").f);
  //    build(spot::parse_infix_psl("a U b").f);
      build(spot::parse_infix_psl("{a ##1 b} | {x ##1 y ##1 z}").f);

    std::cout << *aut1->accepting_run()<< "\n";
   // std::cout << *aut1->accepting_run() << "\n";
  //
}
