#include <filesystem>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdlib.h>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <fstream>
#include "CSVtraceReader.hh"
#include "Trace.hh"
#include "TraceReader.hh"
#include "VCDtraceReader.hh"
#include "VarDeclaration.hh"
#include "commandLineParser.hh"
#include "cxxopts.hpp"
#include "expUtils/ExpType.hh"
#include "globals.hh"
#include "harmIcon.hh"
#include "message.hh"
#include "misc.hh"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "ltl-compare.hh"
//spot includes
#include "spot/twa/bddprint.hh"
#include "spot/twa/twa.hh"
#include "spot/twa/twagraph.hh"
#include "spot/twaalgos/translate.hh"
#include <spot/tl/formula.hh>
#include <spot/tl/parse.hh>
#include <spot/tl/print.hh>
#include <spot/twaalgos/isdet.hh>
#include <spot/twaalgos/postproc.hh>
#include <spot/twaalgos/contains.hh>
#include <spot/twaalgos/hoa.hh>
#include <spot/tl/ltlf.hh>
#include <spot/twaalgos/remprop.hh>


static void parseCommandLineArguments(int argc, char *args[]);

int main(int arg, char *argv[]) {

  /*enforce deterministic rand
  srand(1);

  std::cout << spot::parse_formula("[]<>p0 || <>[]p1") << '\n';
  spot::formula f1 = spot::parse_formula("& & G p0 p1 p2");
  spot::print_latex_psl(std::cout, f1) << '\n';
  spot::print_lbt_ltl(std::cout, f1) << '\n';
  spot::print_spin_ltl(std::cout, f1, true) << '\n';
  
  //equivalenza
  spot::formula f2 = spot::parse_formula("(a U b) U a");
  spot::formula g = spot::parse_formula("b U a");
  std::cout << ((spot::are_equivalent(f2,g)) ? "Equivalent\n" : "Not equivalent\n");
 */
  /*automata 1
  spot::parsed_formula pf = spot::parse_infix_psl("!(p0 xor ((p0 R p2) U F(p0 <-> p1)))");
  if (pf.format_errors(std::cerr))
    return 1;
  spot::translator trans;
  trans.set_type(spot::postprocessor::Monitor);
  trans.set_pref(spot::postprocessor::Deterministic);
  spot::twa_graph_ptr aut = trans.run(pf.f);
  print_hoa(std::cout, aut) << '\n';
  */
  /*automata 2 

  spot::parsed_formula pf2 = spot::parse_infix_psl("((p0 -> Gp1) xor (Xp2 -> p2)) <-> !(p1 U Gp2)");
  if (pf.format_errors(std::cerr))
    return 1;
  spot::translator trans2;
  trans2.set_type(spot::postprocessor::Monitor);
  trans2.set_pref(spot::postprocessor::Deterministic);
  spot::twa_graph_ptr aut2 = trans.run(pf2.f);
  print_hoa(std::cout, aut2) << '\n';

  */

  /*test 1
  spot::formula p1 = spot::parse_formula("{p2[*1..3]}");
  spot::formula p2 = spot::parse_formula("p2 & Xp0");
 
  if(spot::are_equivalent(p1,p2)==0){
    std::cout << ((spot::contains(p1,p2))? "0.5" : "0");
  }else
  {
     std::cout << "1";
  }
  //std::cout << "1";
  */
  //test 2 
  /*std::string assFile = "/home/vboxuser/ltl-compare/src/ltl-compare/src/prova.txt";
  std::ifstream file (assFile);
  if(!file){
    std::cerr << "Errore nell aprire il file!" << std::endl;
    return 1;
  }
  float risultati[5] = {0,0,0.5,0.5,1};
  int i=0;
  float res;
  std::string line1 , line2;
  while(std::getline(file , line1 )){
    if(std::getline(file, line2)){
      std::cout << line1 << std::endl; 
      std::cout << line2 << std::endl;
      res = semanticComparison(line1,line2);
      std::cout << res <<std::endl; 
      //ASSERT_EQ(res,risultati[i]);
      if(res == risultati[i])
      {
        std::cout << "test" << i+1 << "passato" << "\n" << std::endl; 
      }
      i++;
    }
  }
*/
  return 0;
}

