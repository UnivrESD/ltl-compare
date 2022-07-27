

#pragma once

#include <algorithm>
#include <cstdarg>
#include <deque>
#include <mutex>
#include <numeric>
#include <regex>
#include <spot/tl/formula.hh>
#include <string>
#include <thread>
#include <unordered_map>

#include "Automaton.hh"
#include "DTAnd.hh"
#include "DTNCReps.hh"
#include "DTNext.hh"
#include "DTNextAnd.hh"
#include "EdgeProposition.hh"
#include "Hstring.hh"
#include "Location.hh"
#include "PermGenerator.hh"
#include "Semaphore.hh"
#include "Trace.hh"
#include "Trinary.hh"
#include "exp.hh"

// spotLTL
#include "spot/tl/parse.hh"
#include "spot/tl/print.hh"
#include "spot/twa/bddprint.hh"
#include "spot/twaalgos/hoa.hh"
#include "spot/twaalgos/translate.hh"

namespace harm {

/*! \class Template
    \brief Class representing an assertion template, the template can be
   instantiated by loading boolean propositions
*/
class Template {

public:
  /** \brief Constructor
   * \param templateFormula representation of a template as a PSL formula
   * \param max_length length of the trace
   */
  Template(Hstring &templateFormula, harm::Trace *trace, DTLimits limits);

  /** \brief Copy Constructor
   */
  Template(const Template &original);

  ~Template();

  /** \brief loads propositions to instantiate the complete
   * \param props list of propositions to be used to instantiate the template
   * \param where location in which insert the propositions ( antecedent, consequent or both)
   */
  void loadPropositions(std::vector<Proposition *> &props,
                        harm::Location where);
  /** \brief evaluates the assertion generated by loading the propositions in the template
   * \param time trace instant on which to evaluate the assertion
   */
  Trinary evaluate(size_t time);
  /** \brief evaluates the offset assertion (ant->!con) generated by loading the propositions in the template
   * \param time trace instant on which to evaluate the assertion
   */
  Trinary evaluateOffset(size_t time);
  /** \brief evaluates only the antecedent of the assertion
   * \param time trace instant on which to evaluate 
   */
  Trinary evaluate_ant(size_t time);
  /** \brief evaluates only the antecedent ignoring the chaded values
   * \param time trace instant on which to evaluate 
   */
  Trinary evaluateAntNoChache(size_t time);
  /** \brief evaluates only the consequent of the assertion
   * \param time trace instant on which to evaluate 
   */
  Trinary evaluate_con(size_t time);
  /** \brief get the maximum depth from the root node
   * \param aut the automaton
   */
  int getDepth(Automaton *aut);

  /** \brief force the next evaluation of the antecedent to ignore cached values (and store new ones)
   */
  void setCacheAntFalse();
  /** \brief force the next evaluation of the consequent to ignore cached values (and store new ones)
   */
  void setCacheConFalse();

  /** \brief returns the template insantiated with the propositions
   */
  std::string getAssertion();
  /** \brief returns the template insantiated with the propositions (colored)
   */
  std::string getColoredAssertion();

  /** \brief returns the string representation of the template
   */
  std::string getTemplate();
  /** \brief returns the string representation of the template (colored)
   */
  std::string getColoredTemplate();

  /** \brief returns a typed string representation of the template
   */
  Hstring getTemplateFormula();

  /** \brief returns a spot-compatible string representation of the template
   */
  std::string getSpotFormula();

  /** \brief returns a pointer to the decision tree operator
   */
  DTOperator *getDT();

  std::map<std::string, Proposition **> &get_aphToProp();
  std::map<std::string, Proposition **> &get_cphToProp();
  std::map<std::string, Proposition **> &get_acphToProp();

  /** \brief 
   * \param where ant or con
   */
  size_t getNumPlaceholders(harm::Location where);
  size_t getNumPlaceholders();

  /** \brief  generates a sequence of permutations for the current template,
   * each permutation is a set of propositions to be used to instantiate the
   * template
   * \param antP propositions for the antecedent
   * \param conP propositions for the consequent
   * \param antConP propositions for placeholders that are both in the antecedent and the consequent
   */
  void genPermutations(const std::vector<Proposition *> &antP,
                       const std::vector<Proposition *> &conP,
                       const std::vector<Proposition *> &antConP);

  /** \brief loads the next permutation in the template. This method must be
   * called after genPermutations.
   */
  bool nextPerm();
  /** \brief loads the n_th permutation in the template. This method must be
   * called after genPermutations.
   * \param the index of the permutation to load
   */
  void loadPerm(size_t n);
  std::vector<Proposition *> getLoadedPropositions();
  /** \brief get the depth of the placeholder in the abtract syntax tree of the template formula
   * \param phToDepth a list that is filled by this method
   */
  void
  getPlaceholdersDepth(spot::formula f,
                       std::vector<std::pair<std::string, size_t>> &phToDepth);
  /** \brief get the depth of the propositions in the abtract syntax tree of the template formula
   */
  std::vector<std::pair<Proposition *, size_t>>
  getLoadedPropositionsWithDepth();
  std::vector<std::pair<Proposition *, size_t>>
  /** \brief get the depth of the propositions of the antecedent in the abtract syntax tree of the template formula
   */
  getLoadedPropositionsWithDepthAnt();
  std::vector<std::pair<Proposition *, size_t>>
  /** \brief get the depth of the propositions of the consequent in the abtract syntax tree of the template formula
   */
  getLoadedPropositionsWithDepthCon();
  /** \brief get the number of permutations generated */
  size_t nPermsGenerated() const;

  /** \brief returns true if the assertion holds on the input trace, false otherwise */
  bool assHoldsOnTrace(harm::Location update);
  /** \brief returns true if the assertion (offset) holds on the input trace, false otherwise */
  bool assHoldsOnTraceOffset(harm::Location update);
  /** \brief returns true if the assertion is vacuouse, false otherwise */
  bool isVacuous(harm::Location update);
  /** \brief returns true if the assertion is vacuouse (offset), false otherwise */
  bool isVacuousOffset(harm::Location update);

private:
  //technical methods, they implement the generation and the evaluation of the template

  /** \brief initialises the template and builds all the necessary parts. To be
   * called only in the constructor!
   */
  void build();

  /** \brief builds a custom automaton from a spotLTL automaton
   */
  Automaton *buildAutomaton(spot::twa_graph_ptr &automata);

  /** \brief converts a spot LTL formula to a custom proposition
   */
  EdgeProposition *edgeToProposition(const spot::formula &f);

  /** \brief evaluates the formula implemented by the custom automaton
   */
  Trinary evalAutomaton(size_t time, Automaton *root) const;
  Trinary evalAutomatonDyShift(size_t time, Automaton *root, size_t &dShift);

  /** \brief utility method of getDepth
   */
  void maxDepth(int &max, Automaton::Node *cn, size_t currDepth,
                std::unordered_set<size_t> &marked);

  /** \brief returns the 'cause' of the failing assertions
   */
  std::unordered_map<std::string, size_t> findCauseOfFailure();
  /** \brief returns the 'cause' of the failing assertions at time 'time'
   */
  std::string findCauseOfFailure(size_t time);
  /** \brief utility method to find the 'cause' of failure
   */
  std::string findCauseInEdgeProposition(EdgeProposition *ep, size_t time,
                                         bool goal, bool expPlaceholders);
  /** \brief utility method to find the 'cause' of failure
   */
  std::string findCauseInProposition(Proposition *ep, size_t time, bool goal);

  /** \brief generate a FSM from a spot formula
   */
  std::shared_ptr<spot::twa_graph>
  generateDeterministicSpotAutomaton(spot::formula &formula);

  /*! \structure Range
    \brief utility data structure for cuts and threads in the evaluation algorithm
*/
  struct Range {
    Range() {}
    Range(size_t start, size_t length) : _start(start), _length(length) {
      // not todo
    }
    size_t _start;
    size_t _length;
  };


  /** \brief evaluation: 1. this function handles the update of cached values and the dynamic or non-dynamic shift case 
   * \param what specifies if the function has to update the cached values of ant/con/both/none
   */
  void linearEval(harm::Location what);

  /** \brief evaluation: 2. this function handles the cuts in the trace (the traces may contain 'cuts' if it was generated by merging several input traces, the simples case only have one cut, that is, only one input trace)
   * \param aut is the automaton representation of the ant or con
   * \param aut is the automaton representation of the ant or con
   */
  template <bool Dynamic>
  inline void evalWithCut(Automaton *aut, Trinary *cachedValues);

  /** \brief evaluation: 3. this function handles l1 multithreading
   * \param aut is the automaton representation of the ant or con
   * \param cachedValues of the ant or con
   */
  template <bool Dynamic>
  void linearEval(Automaton *aut, Trinary *cachedValues, Range traceRange,
                  Range threadsRange, ProtectedQueueNotify<Range> &threadsPool);

  /** \brief evaluation: 4. actually runs the evaluation algorithm using thread nThread
   * \param aut is the automaton representation of the ant or con
   * \param cachedValues of the ant or con
   * \param traceRange corresponds the sub-trace to be evaluated
   * \param threadsRange contains information on the threads currently in use
   */
  template <bool Dynamic>
  inline void runLinearEval(Automaton *aut, Trinary *cachedValues,
                            Range &traceRange, const Range &threadsRange,
                            size_t nThread = 0);

  /** \brief evaluation: 5. join data concurrently generated by multithreading
   */
  template <bool Dynamic>
  void joinData(Trinary *cachedValues, Range traceRange,
                const Range &threadsRange);

public:

  /** \brief fill the contingency table with the cached values
   * \param ct is a pointer to a suitable matrix
   */
  void fillContingency(size_t (&ct)[3][3], bool offset);

  void setL1Threads(size_t n);
  size_t getL1Threads();

  void setDTLimits(const DTLimits &l);

  /** \brief returns true if the dt algo must also consider the offset assertions
   */
  bool saveOffset();
  bool isFullyInstantiated();
  void printContingency();

  /** \brief check if this (fully instantied) template holds on the input trace, it prints the reason if it does not hold
   */
  void check();

  Proposition *getPropByToken(const std::string &token);
  Automaton *buildDiamondAutomaton(bool conNegated = 0);
  Automaton *getAntecedentAutomaton();

  size_t gatherInterestingValue(size_t time, int depth, int width);

  void subPropInAssertion(Proposition *original, Proposition *newProp);


private:
  // printers
  std::string printAutomatons();
  std::string printAutomaton(Automaton *aut);

  ///mutex for level 1 changes of available threads (see setL1Threads)
  std::mutex _l1Guard;

  /// current employed templated
  Hstring _templateFormula;

  /// original templated with the dt operator uninstantiated
  Hstring _buildTemplateFormula;

  /// used to generate the permutations
  PermGenerator _pg;

  /*utility fields to link the placeholders with the instantiated propositions,
   * note that if the placeholder is not instantiated, then the proposition
   * points to a nullptr
   */
  /// link of all placeholders to all instantiated propositions
  std::unordered_map<std::string, Proposition **> _tokenToProp;
  /// link of all user instantiated placeholders to their respective
  /// propositions
  std::unordered_map<std::string, Proposition **> _iToProp;
  /// links all the dt operators to their respective propositions
  std::pair<std::string, DTOperator *> _dtOp = {"", nullptr};

  ///_aphToProp + _cphToProp + _acphToProp == _phToProp
  std::map<std::string, Proposition **> _aphToProp;
  std::map<std::string, Proposition **> _cphToProp;
  std::map<std::string, Proposition **> _acphToProp;

  /* current proposition domains:
    the generator of permutations will use the following propositions to
    generate the permutations
    */
  ///used by the generator of permutations to generate the permutations
  std::vector<Proposition *> _aProps;
  std::vector<Proposition *> _cProps;
  std::vector<Proposition *> _acProps;

  ///automaton representation of the antecedent
  Automaton *_ant;
  Automaton *_con;

public:
  /// length of the trace
  size_t _max_length;

private:
  /// dt operator configuration
  DTLimits _limits;

public:
  /// pointer to the input trace
  harm::Trace *_trace;

private:
  /// The constant temporal shift that must be applied to evaluate the consequent ex. If the template is Ant => Con the constant shift will be equal to 1. If the template is Ant -> Con the constant shift will be equal to 0
  size_t _constShift;

  /// if true, we must apply a dynamic shift before evaluating the consequent
  bool _applyDynamicShift;

  /// if true, the current values stored in the antecedent are valid
  bool _antInCache = false;
  bool _conInCache = false;

  ///dynamic shift for every truth value of the antecedent
  size_t *_dynamicShiftCachedValues = nullptr;

  /// cached values are used to avoid recalculating each time the truth values of the current assertion
  Trinary *_antCachedValues = nullptr;
  Trinary *_conCachedValues = nullptr;

  ///additional cached values to enable parallelism
  Trinary **_cachedValuesP = nullptr;

  ///additional cached values to enable parallelism
  size_t **_cachedDynShiftsP = nullptr;

public:
  /// index to the current permutatian, -1 if no permutations are generated
  int _permIndex = -1;
  /// the template is an assertion that must be checked on the trace
  bool _check = 0;

private:
  /// maximium non-cyclic reachable distance from the root of the automaton representation of the antecedent
  int _antDepth = 0;
  int _conDepth = 0;

  /// available threads to implement level1 parallelism
  size_t _availThreads = 1;

  friend DTAnd;
  friend DTNext;
  friend DTNextAnd;
  friend DTNCReps;
};
} // namespace harm
