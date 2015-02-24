/*
  A Derived xChannel Class
  - defines the tree variables
  - defines the systematic variations
  - communicates with the xVariables container class
  - uses Base for all TTree core operations
  
  Z. Zinonos - zenon@cern.ch
  Feb 2015
*/

#ifndef xChannelA_xChannelA_H
#define xChannelA_xChannelA_H


#include "xChannel/xChannel.h"

#include "xAODTau/TauJetContainer.h"
//#include "xAODTau/TauJetAuxContainer.h"
#include "xAODTau/TauJet.h"

#include "xAODMissingET/MissingETContainer.h"


#include "xTools/xTools.h"
#include "xDefinitions/xDefinitions.h"
#include "xVariables/xVariables.h"


#include <random> //tmp - for crash tests

/*
struct Input {

  //ctor
Input(const std::initializer_list<xAOD::IParticleContainer *> &ilist, const int ix, const double iy) :
  containers(ilist), x(ix), y(iy)
  {

  }

  ~Input()//delete something?
  {
  }

  void append(const std::initializer_list<xAOD::IParticleContainer *> &ilist) {
    containers.insert(containers.end(), ilist.begin(), ilist.end());
  }
  std::vector<xAOD::IParticleContainer *> containers;
  int x;
  double y;


};


struct S {

  //ctor
S(const std::initializer_list<int> &ilist, const int ix, const double iy) :
  vec(ilist), x(ix), y(iy)
  {

  }

  ~S()//delete something?
  {
  }

  std::vector<int> vec;
  int x;
  double y;


};
*/

class xChannelA : public xChannel {

 public:

  //typedef Input Input_t;
  // typedef S S_t;


  // Dear compiler, disallow the  following ctors for this class
  xChannelA() = delete; //default ctor

  xChannelA(EL::Worker *) = delete;

  xChannelA(TFile *) = delete;

  xChannelA(TFile *, EL::Worker *) = delete;

  // variables: default, systematics: default
  xChannelA(EL::Worker *wk, TFile *fout)
    try //function-try block begins before the function body, which includes init list
    : xChannel() {
    std::cout<<"xChannel::xChannelA 1 "<<std::endl;

    SetTools();

    SetDefaultVariables();
    SetVariables(m_var_vec);

    SetDefaultSystematics();
    SetSystematics(m_syst_vec);

    SetupTrees(wk, fout);
  }
  catch (std::exception& e)
    {
      std::cerr << "exception caught on initialization: " << e.what() << '\n';
    }

  // variables: argument, systematics: argument
 xChannelA(EL::Worker *wk, TFile *fout, const TreeVarMap_t &arg1, const SystSetVec_t &arg2)
   : xChannel(arg1, arg2) {
    std::cout<<"xChannel::xChannelA 2 "<<std::endl;

    SetTools();

    SetDefaultVariables(arg1);
    SetDefaultSystematics(arg2);

    SetupTrees(wk, fout);
  }

  // variables: argument, systematics: argument
 xChannelA(EL::Worker *wk, TFile *fout, const TreeVarVec_t &arg1, const SystSetVec_t &arg2)
   : xChannel(arg1, arg2) {
    std::cout<<"xChannel::xChannelA 3 "<<std::endl;

    SetTools();

    SetDefaultVariables(arg1);
    SetDefaultSystematics(arg2);

    SetupTrees(wk, fout);
  }

  // variables: default, systematics: argument
 xChannelA(EL::Worker *wk, TFile *fout, const SystSetVec_t &arg)
   : xChannel(arg) //delegate
  {
    std::cout<<"xChannel::xChannelA 4 "<<std::endl;

    SetTools();

    SetDefaultVariables();
    SetVariables(m_var_vec);

    SetDefaultSystematics(arg);

    SetupTrees(wk, fout);
  }

  //variables: argument, systematics: default
 xChannelA(EL::Worker *wk, TFile *fout, const TreeVarMap_t &arg)
   : xChannel(arg)
  {
    std::cout<<"xChannel::xChannelA 5 "<<std::endl;

    SetTools();

    SetDefaultVariables(arg);
    SetDefaultSystematics();

    SetupTrees(wk, fout);
  }

  //variables: argument, systematics: default
 xChannelA(EL::Worker *wk, TFile *fout, const TreeVarVec_t &arg)
   : xChannel(arg)
  {
    std::cout<<"xChannel::xChannelA 6 "<<std::endl;

    SetTools();

    SetDefaultVariables(arg);
    SetDefaultSystematics();

    SetupTrees(wk, fout);
  }

  //overloads
  //xChannelA(const xChannelA& other); //the default copy ctor

  //xChannelA(xChannelA&& other) = default; // the copy ctor overloaded

  //xChannelA& operator= (const xChannelA& other); // the assignment operator overloaded

 private:

  //xTools class
  std::unique_ptr<xTools> m_tools;

  void SetTools(){
    m_tools = CxxUtils::make_unique<xTools>();
  }

  //lists
  TreeVarVec_t m_var_vec;
  TreeVarMap_t m_var_map;
  SystSetVec_t m_syst_vec;

  //functions to override derived default parameters
  void SetDefaultVariables(const TreeVarVec_t &arg){
    m_var_vec = arg;
  }

  void SetDefaultVariables(const TreeVarMap_t &arg){
    m_var_map = arg;
  }

  void SetDefaultSystematics(const SystSetVec_t &arg){
    m_syst_vec = arg;
  }

  // must-have virtual methods to override base functions
  // since Base doesn't know about variables and systematics
  virtual void SetDefaultVariables();
  virtual void SetDefaultSystematics();

 public:
  // channel specific functions for public access
  void FillBranches( const xAOD::TauJetContainer *, 
		     const xAOD::MissingET *);

 private:
  //channel specific functions used internally
  void FillTau(const unsigned short i, const xAOD::TauJetContainer *ptaus, const xAOD::MissingET *pmet);

  void FillCrashTest();

  //  virtual void FillBranches( initializer_list<int> &);

  //   template<typename T>
  //  void FillBranches(const std::initializer_list<T *> &);

  //  void FillBranches( const std::initializer_list<xAOD::IParticleContainer *> &);


  //void FillBranches( const Input_t &);

  //void FillBranches( const S_t &);

  //void FillBranches( const std::initializer_list<xAOD::IParticleContainer *> &);

  /*  Foo( const std::initializer_list<xAOD::IParticleContainer *> &clist, )
      : x(a), y(c) {

      assert(b.size() >= 24, "err");
      std::copy(b.begin(), b.begin() + 24, array);
      }

      ~Foo() { //delete something?
      }

      int x;
      char array[24];
      short* y;
  */




};

/*
  xChannelA& xChannelA::operator= (const xChannel &cSource)
  {
  // check for self-assignment at the top of the overloaded assignment operator
  // by comparing the address of the implicit object and the parameter
  if (this == &cSource)
  return *this;

  // do the copy
  m_var_vec = cSource.m_var_vec;
  m_var_map = cSource.m_var_map;
  m_syst_vec = cSource.m_syst_vec;

  // return the existing object
  return *this;
  };
*/
/*
  xChannelA::xChannelA(const xChannel &cSource)
  {
  // no need to check for self-assignment in a copy-constructor
  // the copy constructor is only called when new objects are being constructed,
  // so, there is no way to assign a newly created object to itself in a way that calls to copy constructor.

  m_var_vec = cSource.m_var_vec;
  m_var_map = cSource.m_var_map;
  m_syst_vec = cSource.m_syst_vec;

  }
*/
#endif

