#include "ltl-compare.hh"
#include <filesystem>
#include <string>

//spot includes
#include "spot/twa/bddprint.hh"
#include "spot/twa/twa.hh"
#include "spot/twa/twagraph.hh"
#include "spot/twaalgos/translate.hh"
#include <spot/tl/formula.hh>
#include <spot/tl/ltlf.hh>
#include <spot/tl/parse.hh>
#include <spot/tl/print.hh>
#include <spot/twaalgos/contains.hh>
#include <spot/twaalgos/hoa.hh>
#include <spot/twaalgos/isdet.hh>
#include <spot/twaalgos/postproc.hh>
#include <spot/twaalgos/remprop.hh>

//normal include
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
std::vector<std::vector<std::string>> hybridCompare(spot::const_twa_graph_ptr aut){
  std::vector<bool>seen(aut->num_states(),false);
  std::vector<unsigned>path;
  //std::vector<spot::formula>edges;
  std::vector<spot::formula>edges;
  std::vector<std::string> temp;
  std::vector<std::vector<std::string>> risultato;
    std::vector<std::vector<std::string>> real;
  dfs_rec_prova(aut,aut->get_init_state_number(),seen,path,edges,aut->get_dict(),temp,risultato);
  //stampaVettore(risultato);
  return risultato;

}

float semanticComparison(std::string a1, std::string a2) {

  spot::formula p1 = spot::parse_formula(a1);
  spot::formula p2 = spot::parse_formula(a2);

  if (spot::are_equivalent(p1, p2) == 0) {
    if (spot::contains(p1, p2)==1) {
      return 0.5;
    } else {
      return 0;
    }
  } else {
    return 1;
  }
}

double hybridComparison(std::string a1, std::string a2){
  std::vector<std::vector<std::string>> list1;
   spot::parsed_formula pf = spot::parse_infix_psl(a1);
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
  spot::parsed_formula pf2 = spot::parse_infix_psl(a2);
  if(pf2.format_errors(std::cerr))
    return 1;
    spot::translator trans1;
  trans1.set_pref(spot::postprocessor::Deterministic);
  auto aut1 = trans.run(pf2.f);
  std::cout<<"\n\n"<<std::endl;
  list2 = hybridCompare(aut1);


  std::set<std::string> set1 = flatten(list1);
    std::set<std::string> set2 = flatten(list2);
    
    std::vector<std::string> intersezione;
    std::vector<std::string> unione;
    
    std::set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(),
                          std::back_inserter(intersezione));
    
    std::set_union(set1.begin(), set1.end(), set2.begin(), set2.end(),
                   std::back_inserter(unione));
    
    double percentuale = unione.empty() ? 0 : (double)intersezione.size() / unione.size() * 100;
    //std::cout<<percentuale<<std::endl;
    //std::cout << "Percentuale di intersezione: " << percentuale << "%" << std::endl;

    return percentuale;

}
