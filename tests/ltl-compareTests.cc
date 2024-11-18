#include "ltl-compare.hh"
#include "gtest/gtest_pred_impl.h"
#include <gtest/gtest-message.h>
#include <gtest/gtest-test-part.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

TEST(ltlcompare, spotSemanticEquivalence) {

  std::string assFile = "../tests/input/ltl-compare-ass.txt";
  std::ifstream file (assFile);
  if(!file){
    std::cerr << "Errore nell aprire il file!" << std::endl;
  }
  float risultati[10] = {0,0,0.5,0.5,1,0,0.5,0,0.5,1};
  float res;
  int i = 0;
  std::string line1 , line2;
  while(std::getline(file , line1 )){
    if(std::getline(file, line2)){
      res = semanticComparison(line1,line2);
      ASSERT_EQ(res,risultati[i]);
      i++;
    }
  }
  

  //int res = semanticComparison("a U b", "a U b");
  //ASSERT_EQ(res, 1);
}

