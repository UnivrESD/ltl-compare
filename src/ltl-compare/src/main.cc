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
#include <sstream>
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
        std::cout << it->acc().as_string() << "\n";cd 
      }
    }
}
  //delete it;


}*/
std::set<std::string> splitConditions(const std::string& str) {
    std::set<std::string> conditions;
    std::stringstream ss(str);
    std::string condition;
    while (std::getline(ss, condition, '&')) { // Separazione per '&' invece di ','
        condition.erase(0, condition.find_first_not_of(" ")); // Rimuove spazi iniziali
        condition.erase(condition.find_last_not_of(" ") + 1); // Rimuove spazi finali
        conditions.insert(condition);
    }
    return conditions;
}
std::set<std::string> flatten(const std::vector<std::vector<std::string>>& vec) {
   std::set<std::string> flatSet;
    for (const auto& subVec : vec) {
        for (const auto& str : subVec) {
            std::set<std::string> conditions = splitConditions(str);
            flatSet.insert(conditions.begin(), conditions.end());
        }
    }
    return flatSet;
}

std::string vectorToString(const std::vector<std::string> vec, const std::string& delimiter){
  std::ostringstream oss;
  for(size_t i = 0; i< vec.size(); i++){
    oss << vec[i];
    if(i<vec.size()-1){
      oss << delimiter;
    }
  }
  return oss.str();
}

std::string prova(const spot::formula &f, bool negato){

      if(f.is(spot::op::And)){
        std::vector<std::string> operandi;
        
        for(size_t i = 0; i < f.size(); i++){
          //std::cout<<f[i].ap_name()<<'\n';
          operandi.push_back(prova(f[i],negato));
        }
          return vectorToString(operandi," & ");
      }

      if(f.is(spot::op::Or)){
        std::vector<std::string> operandi;
        for(size_t i = 0 ; i < f.size(); i++){
           //std::cout<<f[i].ap_name()<<'\n';
          operandi.push_back(prova(f[i],negato));
        }
        return vectorToString(operandi, " | ");
      }

      if(f.is(spot::op::Not)){
        negato = true;
        return prova(f[0],negato);

      }

      if(f.is(spot::op::ap)){
        if(negato == false)
        {
          std:: string temp = f.ap_name();

          return temp.append("==1");
        }
        else{
          std:: string temp = f.ap_name();

          return temp.append("==0");
        }
      }


  }

void stampaVettore(const std::vector<std::vector<std::string>>& vec) {
    for (const auto& sottovettore : vec) {
        std::cout << "Percorso: { ";
        for (size_t i = 0 ; i < sottovettore.size(); ++i) {
           std::cout << sottovettore[i];
           if ( i != sottovettore.size()- 1){
            std::cout << ", ";
           }
        }
        std::cout << "}" << std::endl; // Chiudi le graffe
    }
}





typedef std::unordered_set<const spot::state*,
                           spot::state_ptr_hash,
                           spot::state_ptr_equal> seen_t;

                        
void dfs(spot::const_twa_graph_ptr aut ,const  spot::state* state, seen_t& seen, auto bddDict , std::vector<std::vector<std::string>>&risultato,std::vector<std::string>temp,int& primavolta, size_t& statoaccetante){
  
  
  
  if(seen.insert(state).second == false){
    state->destroy();
    return;
  }
  
  for(auto i: aut->succ(state)){
    
    const spot::state* dst = i->dst();
    spot::formula edge = spot::parse_formula(spot::bdd_format_formula(bddDict,i->cond()));
    std::string tempString;
    std::cout << aut->format_state(state) << "->" << aut->format_state(dst) << '\n';
    if(aut->state_is_accepting(state) && (state->hash() ==dst->hash()&&!aut->succ_iter(state)->next()) || ((dst->hash() == statoaccetante)==1))
    {
       //auto risultato = dst->hash() == state->hash();
      //std::cout << "risultato dentro " << risultato <<'\n'; 
      if(primavolta == 0)
      {
          statoaccetante = state->hash();
          std::cout<<"valore stato finale = " + edge <<'\n';
          
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
        std::cout<<"dentro stato finito:"<<'\n';
        std::cout << edge<< '\n';
        //temp = temp.append(spot::bdd_format_formula(bddDict,i->cond()));

          temp.push_back(prova(edge,false));
      }
      //std::cout << "vaffanculo " << aut->format_state(state)<< edge <<'\n';
      //std::cout << temp << '\n';
      risultato.push_back(temp);
      std::cout <<"codice finale: " + temp.back() << '\n';
      //std::cout << edge << '\n';
      primavolta = 1;
      temp.clear();
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
         std::cout<<"prima volta = 1 "<< '\n';
          std::cout<<edge<<'\n';
           temp.push_back(prova(edge,false));
             std::cout << "uscita di sta roba " + temp.back() << '\n';
      }else{
        std::cout<<"prima volta = 1 "<< '\n';
      std::cout<<edge << '\n';
      //temp.push_back(spot::bdd_format_formula(bddDict,i->cond()));
      //std::cout << temp.back() << '\n';
      temp.push_back(prova(edge,false));
       std::cout << "uscita di sta roba " + temp.back() << '\n';
      }
      //std::cout<< edge << '\n';
      //std::cout << temp << '\n';
      //std::cout << aut->format_state(state) << "->" << aut->format_state(dst) << edge << '\n';
    }
    dfs(aut, dst , seen,bddDict,risultato,temp,primavolta,statoaccetante);
  }



  
    }

 std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Funzione per elaborare i vettori di stringhe
std::vector<std::vector<std::string>> processVector(const std::vector<std::vector<std::string>>& input) {
    std::vector<std::vector<std::string>> output;

    for (const auto& vec : input) {
        std::vector<std::vector<std::string>> tempOutputs(1); // Per mantenere i risultati parziali

        for (const auto& str : vec) {
            if (str.find('|') != std::string::npos) {
                // Dividi la stringa in base a '|'
                std::vector<std::string> parts = split(str, '|');
                // Aggiungi ogni parte a tutte le combinazioni esistenti
                std::vector<std::vector<std::string>> newOutputs;
                for (const auto& part : parts) {
                    for (auto temp : tempOutputs) {
                        temp.push_back(part); // Aggiungi parte al risultato parziale
                        newOutputs.push_back(temp);
                    }
                }
                tempOutputs = std::move(newOutputs);
            } else {
                // Aggiungi la stringa invariata a tutti i risultati parziali
                for (auto& temp : tempOutputs) {
                    temp.push_back(str);
                }
            }
        }

        // Aggiungi i risultati finali
        for (const auto& temp : tempOutputs) {
            output.push_back(temp);
        }
    }

    return output;
}

std::vector<std::vector<std::string>> removeMatchingFirstElement(const std::vector<std::vector<std::string>>& input) {
    // Controlla se l'input è vuoto o ha un solo vettore
    if (input.empty() || input.size() == 1) {
        return input;
    }

    // Copia l'input per mantenere immutabile l'originale
    std::vector<std::vector<std::string>> result = input;

    // Itera a partire dal secondo vettore
    for (size_t i = 1; i < result.size(); ++i) {
        // Rimuovi i primi `i` elementi, se ci sono abbastanza elementi
        if (result[i].size() > i) {
            result[i].erase(result[i].begin(), result[i].begin() + i);
        } else {
            result[i].clear(); // Se ci sono meno di `i` elementi, svuota il vettore
        }
    }

    return result;
}
std::vector<std::vector<std::string>> beautify(const std::vector<std::vector<std::string>>& vec){
    std::vector<std::vector<std::string>> output1 = removeMatchingFirstElement(vec);
    //printVector(output1);
    std::vector<std::vector<std::string>> output2 = processVector(output1);
    return output2;
}  
std::vector<std::vector<std::string>> modificavettore(std::vector<std::vector<std::string>>vec){
  for(size_t i = 1; i < vec.size(); ++i){
    size_t numToRemove = i;
    if(vec[i].size()>= numToRemove ){
      vec[i].erase(vec[i].begin(),vec[i].begin() + numToRemove);
    }
  }
  return vec;
}


std::vector<std::vector<std::string>> dfs_rec(spot::const_twa_graph_ptr aut,int& primavolta, size_t& statoaccetante){

 //auto s = aut->get_init_state();
 //std::vector<std::vector<std::string>>ris;
 //std::unordered_map<size_t,const spot::state*>visited;
 //std::vector<std::string>word;
 seen_t seen;
 //std::vector<bool>seen(aut->num_states());
 auto bdddict= aut->get_dict();
 std::vector<std::vector<std::string>> risultato;
 //int iteratore = 0;
 //std::cout<<"entrato"<<'\n';
 std::vector<std::string> temp;
 std::vector<std::vector<std::string>> real;
 dfs(aut,aut->get_init_state(),seen,bdddict,risultato,temp,primavolta,statoaccetante);
 stampaVettore(risultato);
 std::cout<<"\n\n"<< std::endl;
 real = beautify(risultato);
 //stampaVettore(real);
 for (auto s: seen)
    s->destroy();
//std::cout<<"entrato" << '\n';
 return real;
}
//ennesima prova
void dfs_rec_prova(spot::const_twa_graph_ptr aut, unsigned state, 
std::vector<bool>&seen,
std::vector<unsigned>&path,
std::vector<spot::formula>&edges,
const spot::bdd_dict_ptr& bddDict,
std::vector<std::string>&temp,
std::vector<std::vector<std::string>>&risultato){
  seen[state] = true;       // Contrassegna lo stato come visitato
    path.push_back(state);    // Aggiungi lo stato al percorso corrente

    // Controlla se lo stato è accettante
    if (aut->state_is_accepting(state)) {
        //std::cout << "Route to accepting state: ";
        for(size_t i=0; i<path.size();++i){
          //std::cout<<"prova"<<'\n';
          //std::cout<<path[i];
          if(i<edges.size()){
            temp.push_back(prova(edges[i],false));
            //std::cout<< temp.back() << "," <<'\n';
          }
        }
        risultato.push_back(temp);
        temp.clear();
        //std::cout << '\n';
    }

    // Esplora tutti i vicini
    for (auto& e : aut->out(state)) {
      if(e.dst >= aut->num_states()){
        std::cerr << "Invalid destination state detected.\n";
        continue;
      }
        if (!seen[e.dst]) {
           //std::cout<<"prova2"<<'\n';
           spot::formula edge_label = spot::parse_formula(spot::bdd_format_formula(bddDict,e.cond));
           edges.push_back(edge_label);
            dfs_rec_prova(aut, e.dst, seen, path,edges,bddDict,temp,risultato);
            edges.pop_back();
        }
    }

    // Backtracking: rimuovi lo stato dal percorso e resetta il visto
    path.pop_back();
    seen[state] = false;
}

void samu_prova(spot::const_twa_graph_ptr aut){
  size_t stateCount = 0;
  auto initState = aut->get_init_state();
  std::unordered_map<size_t,const spot::state *> visited;
  std::deque<const spot::state *> fringe;


  fringe.push_front(initState);
  visited.insert({initState->hash(),initState});
  while(!fringe.empty()){
    const spot::state *currstate = fringe.back();
    fringe.pop_back();
    for(auto s: aut->succ(currstate)){
      if(visited.count(s->dst()->hash())==0){
        std::cout << aut->format_state(currstate) << "->" << aut->format_state(s->dst())<< "'\n";
        visited.insert({s->dst()->hash(),s->dst()});
        fringe.push_back(s->dst());
      }
    }
  }
}


std::vector<std::vector<std::string>> hybridCompare(spot::const_twa_graph_ptr aut){
  std::vector<bool>seen(aut->num_states(),false);
  std::vector<unsigned>path;
  //std::vector<spot::formula>edges;
  std::vector<spot::formula>edges;
  std::vector<std::string> temp;
  std::vector<std::vector<std::string>> risultato;
    std::vector<std::vector<std::string>> real;
  dfs_rec_prova(aut,aut->get_init_state_number(),seen,path,edges,aut->get_dict(),temp,risultato);
  stampaVettore(risultato);
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
  /*int primavolta = 0;
 size_t  statoaccetante = 0;
 std::vector<std::vector<std::string>> list1;
  spot::parsed_formula pf2 = spot::parse_infix_psl("{a##1b}|{x ##1 y ##1z}|{c ##1 d ##1e}");
  if(pf2.format_errors(std::cerr))
    return 1;
  list1 = dfs_rec(spot::translator().run(pf2.f),primavolta,statoaccetante);
 stampaVettore(list1);
 //prova 2 chatgpt
    // Impostare lo stato accettante*/

  int primavolta = 0;
 size_t statoaccetante = 0;
 std::vector<std::vector<std::string>> list1;

  spot::parsed_formula pf = spot::parse_infix_psl("{a##1b}|{x ##1 y ##1z}");
  if(pf.format_errors(std::cerr))
    return 1;
    spot::translator trans;
  trans.set_pref(spot::postprocessor::Deterministic);
  auto aut = trans.run(pf.f);
  //list2 = dfs_rec(aut,primavolta,statoaccetante);
   //stampaVettore(list2);
  std::cout << "\n\n" << std::endl;
  list1 = hybridCompare(aut);
  std::vector<std::vector<std::string>> list2;
  spot::parsed_formula pf2 = spot::parse_infix_psl("{a##1b}|{x ##1 y ##1z}");
  if(pf2.format_errors(std::cerr))
    return 1;
    spot::translator trans1;
  trans1.set_pref(spot::postprocessor::Deterministic);
  auto aut1 = trans.run(pf2.f);
  std::cout<<"\n\n"<<std::endl;
  list2 = hybridCompare(aut1);
  /*std::vector<bool>seen(aut->num_states(),false);
  std::vector<unsigned>path;
  //std::vector<spot::formula>edges;
  std::vector<spot::formula>edges;
  std::vector<std::string> temp;
  std::vector<std::vector<std::string>> risultato;
    std::vector<std::vector<std::string>> real;
  dfs_rec_prova(aut,aut->get_init_state_number(),seen,path,edges,aut->get_dict(),temp,risultato);
  stampaVettore(risultato);*/

  //real = beautify(risultato);
  //printf("siumm\n");
  //stampaVettore(real);
  //std::cout << "\n\n" << std::endl;
  //samu_prova(aut);
  //std::cout << prova(pf.f,false) << '\n';
  


   std::set<std::string> set1 = flatten(list1);
    std::set<std::string> set2 = flatten(list2);
    
    std::vector<std::string> intersezione;
    std::vector<std::string> unione;
    
    std::set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(),
                          std::back_inserter(intersezione));
    
    std::set_union(set1.begin(), set1.end(), set2.begin(), set2.end(),
                   std::back_inserter(unione));
    
    double percentuale = unione.empty() ? 0 : (double)intersezione.size() / unione.size() * 100;
    
    std::cout << "Percentuale di intersezione: " << percentuale << "%" << std::endl;
  
  /*primavolta = 0;
  statoaccetante = 0;
  spot::parsed_formula pf3 = spot::parse_infix_psl("{p2}");
  std::vector<std::vector<std::string>> list3;
  list3 = dfs_rec(spot::translator().run(pf3.f),primavolta,statoaccetante);


stampaVettore(list3);

 primavolta = 0;
 statoaccetante = 0;
 spot::parsed_formula pf4 = spot::parse_infix_psl("{(a |  b) ##1c}");
 std::vector<std::vector<std::string>> list4;
 list4 = dfs_rec(spot::translator().run(pf4.f),primavolta,statoaccetante);
 stampaVettore(list4);

  primavolta = 0;
 statoaccetante = 0;
 spot::parsed_formula pf5 = spot::parse_infix_psl("{(a |  b) ##1c} | {x ##1 y ##1z}");
 std::vector<std::vector<std::string>> list5;
 list5 = dfs_rec(spot::translator().run(pf5.f),primavolta,statoaccetante);
 stampaVettore(list5);*/
}

