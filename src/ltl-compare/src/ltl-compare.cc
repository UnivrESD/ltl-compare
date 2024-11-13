#include "ltl-compare.hh"
#include <filesystem>
#include <string>


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


//normal include 



int semanticComparison(std::string a1, std::string a2) { 

spot::formula p1 = spot::parse_formula(a1);
spot::formula p2 = spot::parse_formula(a2);

if(spot::are_equivalent(p1,p2)==0){
    std::cout << ((spot::contains(p1,p2))? return 0.5 : return 1);
  }else
  {
     return 1;
  }
  return 1; 
}
