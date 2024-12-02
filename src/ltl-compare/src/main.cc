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
#include <algorithm>
#include <unordered_set>
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
#include <spot/twa/acc.hh>
#include <spot/twaalgos/word.hh>
#include <spot/twa/twaproduct.hh>

static void parseCommandLineArguments(int argc, char *args[]);

/*
void dfs(spot::const_twa_graph_ptr aut){

  size_t statecount = 0;
  std::vector<std::string>word;
  std::unordered_map<size_t,size_t> hashToId;
  auto initState = aut->get_init_state();
  

  //auto bddDict = aut->get_dict;

  std::unordered_map<size_t,const spot::state *>visited;

  std::deque<const spot::state*>fringe;

  // visitamo l'automa partendo da lo stato iniziale
  fringe.push_front(initState);
  visited.insert({initState->hash(),initState});

  while (!fringe.empty())
  {
    const spot::state *currState = fringe.back();
    fringe.pop_back();
    //auto ris = aut->get_acceptance();
    
    auto it =  aut->succ_iter(currState);
    if(it->first() && (it->dst()->hash()==currState->hash()&&!it->next())){
      if(aut->state_is_accepting(currState))
      {
        //word.push_back(it->acc().as_string());
        std::cout << it->acc().as_string() << "\n";
      }
    }
}
  //delete it;


}*/

typedef std::unordered_set<const spot::state*,
                           spot::state_ptr_hash,
                           spot::state_ptr_equal> seen_t;

                        
void dfs(spot::const_twa_graph_ptr aut ,const  spot::state* state, seen_t& seen, auto bddDict , std::vector<std::string>&risultato,std::string temp,int& primavolta, size_t& statoaccetante){
  
  
  
  if(seen.insert(state).second == false){
    state->destroy();
    return;
  }
  
  for(auto i: aut->succ(state)){
    
    const spot::state* dst = i->dst();
    spot::formula edge = spot::parse_formula(spot::bdd_format_formula(bddDict,i->cond()));
    if(aut->state_is_accepting(state) && (state->hash() ==dst->hash()&&!aut->succ_iter(state)->next()) || ((dst->hash() == statoaccetante)==1))
    {
       //auto risultato = dst->hash() == state->hash();
      //std::cout << "risultato dentro " << risultato <<'\n'; 
      if(primavolta == 0)
      {
          statoaccetante = state->hash();
          
      }
      //std::cout << "dst dentro lo stato accetante " << dst->hash() << '\n';
      //std::cout << "state hash " << state->hash() << '\n';
      //std::cout << "prima volta " << primavolta << '\n';
      //std::cout << "stato accetante " << statoaccetante << '\n';
      //if(iteratore == aut->num_states()-1)
      //{
          //temp = temp.append(edge);
      //}
      //risultato.push_back(temp);
      if(primavolta == 1)
      {
        temp = temp.append(edge.ap_name());
      }
      //std::cout << "vaffanculo " << aut->format_state(state)<< edge <<'\n';
      //std::cout << temp << '\n';
      risultato.push_back(temp);
      //std::cout << risultato.back() << '\n';
      //std::cout << edge << '\n';
      primavolta = 1;
      temp=" ";
    }else{
      
      //std::cout << "dst " << dst->hash() << '\n';
      //std::cout << "state "<< state->hash() << '\n';
      //auto risultato = dst->hash() == state->hash();
      //std::cout << "risultato " << risultato <<'\n'; 
      //std::cout << "stato accentante" << statoaccetante << '\n';
      //std::cout << "primo volta" << primavolta << '\n';
      //iteratore++;
      //std::cout<<"num_states"<<aut->num_states()-1 << '\n';
      if(primavolta==1)
      {
      temp = temp.erase(0,1);
      temp = temp.append(edge.ap_name());
     
      }
      else
        temp=temp.append(edge.ap_name());
      //std::cout<< edge << '\n';
      //std::cout << temp << '\n';
      //std::cout << aut->format_state(state) << "->" << aut->format_state(dst) << edge << '\n';
    }
    dfs(aut, dst , seen,bddDict,risultato,temp,primavolta,statoaccetante);
  }



  
  }
  



std::vector<std::string> dfs_rec(spot::const_twa_graph_ptr aut,int& primavolta, size_t& statoaccetante){

 //auto s = aut->get_init_state();
 //std::vector<std::vector<std::string>>ris;
 //std::unordered_map<size_t,const spot::state*>visited;
 //std::vector<std::string>word;
 seen_t seen;
 //std::vector<bool>seen(aut->num_states());
 auto bdddict= aut->get_dict();
 std::vector<std::string> risultato;
 //int iteratore = 0;
 //std::cout<<"entrato"<<'\n';
 std::string temp=" ";
 dfs(aut,aut->get_init_state(),seen,bdddict,risultato,temp,primavolta,statoaccetante);
 for (auto s: seen)
    s->destroy();
//std::cout<<"entrato" << '\n';
 
 return risultato;
}

//domande da fare perche forse da solo non ne esco
/*
  1) input da dare esempio spotltl
  2) far vedere le scoperte
  3) capire come funziona twa_graph soprattuto per la funzione 
     accpting_run che ritorna una run accettata se esiste
     idem per accepting_word anche perche non ci sono esempi
     su spot e chatgpt poretto manco lui sa che pesci prendere
  



*/
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
  //test 3 p2 & Xp0 {p2[*1..3]}
  /*spot::parsed_formula pf = spot::parse_infix_psl("{p2[*1..3]}");
  if (pf.format_errors(std::cerr))
    return 1;
  spot::translator trans;
  trans.set_type(spot::postprocessor::Monitor);
  trans.set_pref(spot::postprocessor::Deterministic);
  spot::twa_graph_ptr aut = trans.run(pf.f);
  print_hoa(std::cout, aut) << '\n';


  spot::parsed_formula pf2 = spot::parse_infix_psl("p2 & Xp0");
  if(pf2.format_errors(std::cerr))
    return 1;
  spot::translator trans2;
  trans2.set_type(spot::postprocessor::Monitor);
  trans2.set_pref(spot::postprocessor::Deterministic);
  spot::twa_graph_ptr aut2 = trans.run(pf2.f);
  print_hoa(std::cout, aut2 ) << '\n';
*/
 //const std::vector <spot::formula>& spot::twa::ap(pf) prova const;

 //for(size_t i=0 ; i < prova.size(); ++i){
   // spot::print_latex_psl(std::cout , prova[i]);
 //}
 /*std::list<std::string> prova = spot::list_formula_props(&pf);

 for(std::string stringa : prova ){
    std::cout << stringa << " ";
 }*/

  // test che non centra L(E) L(M)

  // visitare l'automa usando bfs

    // todo transform in tgba



  
  
  /*spot::parsed_formula pf = spot::parse_infix_psl("{a##1b}|{x ##1 y ##1z}");
  if(pf.format_errors(std::cerr))
    return 1;
  //dfs_rec(spot::translator().run(pf.f));
  dfs_rec(spot::translator().run(pf.f));*/
  int primavolta = 0;
 size_t  statoaccetante = 0;
 std::vector<std::string> list1;
  spot::parsed_formula pf2 = spot::parse_infix_psl("{a##1b}|{x ##1 y ##1z}|{c ##1 d ##1e}");
  if(pf2.format_errors(std::cerr))
    return 1;
  list1 = dfs_rec(spot::translator().run(pf2.f),primavolta,statoaccetante);
 
 //prova 2 chatgpt
    // Impostare lo stato accettante

  primavolta = 0;
 statoaccetante = 0;
 std::vector<std::string> list2;
  std::cout << "seconda prova" << '\n';
  spot::parsed_formula pf = spot::parse_infix_psl("{a##1b}|{x ##1 y ##1z}");
  if(pf2.format_errors(std::cerr))
    return 1;
  list2 = dfs_rec(spot::translator().run(pf.f),primavolta,statoaccetante); 
   
  std::vector<std::string> intersezione; 
  std::vector<std::string> unione ; 

  std::sort(list1.begin(), list1.end());
  std::sort(list2.begin(), list2.end());

  std::set_intersection(list1.begin(), list1.end(), list2.begin(), list2.end(),
                        std::back_inserter(intersezione));

  std::set_union(list1.begin(), list1.end(), list2.begin(), list2.end(),
                 std::back_inserter(unione));

  double percentuale = (double)intersezione.size() / unione.size() * 100;

  std::cout << "Percentuale di intersezione: " << percentuale << "%" << std::endl;
  

  return 0;
}

