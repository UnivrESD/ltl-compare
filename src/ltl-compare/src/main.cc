#include <filesystem>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdlib.h>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

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

static void parseCommandLineArguments(int argc, char *args[]);

int main(int arg, char *argv[]) {

  //enforce deterministic rand
  srand(1);

  std::cout << spot::parse_formula("[]<>p0 || <>[]p1") << '\n';
  spot::formula f = spot::parse_formula("& & G p0 p1 p2");
  spot::print_latex_psl(std::cout, f) << '\n';
  print_lbt_ltl(std::cout, f) << '\n';
  print_spin_ltl(std::cout, f, true) << '\n';
  return 0;

  return 0;
}

