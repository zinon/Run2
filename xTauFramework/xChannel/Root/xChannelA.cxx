#include <xChannel/xChannelA.h>


void xChannelA::SetDefaultVariables() {

  std::cout<<" void xChannelA::SetDefaultVariables() "<<std::endl;

  xVariables var1(xVariables::TAU);
  std::cout<<var1;

  xVariables var2(xVariables::TAU_MET);
  std::cout<<var2;

  xVariables var3( {xVariables::TAU, xVariables::TAU_MET} );
  std::cout<<var3;

  xVariables var4 = var1 + var2; 
  std::cout<<var4;

  xVariables var5; 
  var5 += var1;
  var5 += var2;
  std::cout<<var5;
  
  m_var_vec = var5.GetVariables();

  //a super manual way
  /*  m_var_vec = {
    "tau_n",
    "tau_0",
    "tau_0_pt",
    "tau_0_eta",
    "tau_0_phi",
    "tau_0_q",
    "tau_0_ntrks",
    "tau_0_JetBDT_score",
    "tau_0_JetBDT_loose",
    "tau_0_JetBDT_medium",
    "tau_0_JetBDT_tight",
    "tau_0_JetBDT_eff_sf",
    "tau_0_met_mt"
  };
  */

  /*
    for(int i = 0; i < 5000; i++){
    m_var_vec.push_back("test_"+std::to_string(i) );
    }
  */



  std::cout<<"xChannelA::SetDefaultVariables() size "<<m_var_vec.size()<<std::endl;
}


void xChannelA::SetDefaultSystematics() {

  //default is nominal (empty name)
  m_syst_vec.push_back(CP::SystematicSet());

  std::cout<<"xChannelA::SetDefaultSystemaics() size "<<m_var_vec.size()<<std::endl;
}


void xChannelA::FillBranches( const xAOD::TauJetContainer *ptaus, const xAOD::MissingET *pmet)
{

  FillVar("tau_n", ptaus->size());

  FillTau(0, ptaus, pmet);
  //FillTau(1);

  //FillCrashTest();

}

void xChannelA::FillTau(const unsigned short i, const xAOD::TauJetContainer *ptaus, const xAOD::MissingET *pmet){

  if(ptaus->size() > i){
    const std::string prefix("tau_"+std::to_string(i));
    FillVar(prefix, 1);
    FillVar(prefix+"_pt", ptaus->at(i)->pt() * MeV2GeV );
    FillVar(prefix+"_eta", ptaus->at(i)->eta() );
    FillVar(prefix+"_phi", ptaus->at(i)->phi() );
    FillVar(prefix+"_q", ptaus->at(i)->charge() );
    FillVar(prefix+"_ntrks", ptaus->at(i)->nTracks() );
    FillVar(prefix+"_JetBDT_score", ptaus->at(i)->discriminant( xAOD::TauJetParameters::BDTJetScore ) );
    FillVar(prefix+"_JetBDT_loose", ptaus->at(i)->isTau( xAOD::TauJetParameters::JetBDTSigLoose ) );
    FillVar(prefix+"_JetBDT_medium", ptaus->at(i)->isTau( xAOD::TauJetParameters::JetBDTSigMedium ) );
    FillVar(prefix+"_JetBDT_tight", ptaus->at(i)->isTau( xAOD::TauJetParameters::JetBDTSigTight ) );
    FillVar(prefix+"_JetBDT_eff_sf", ptaus->at(i)->auxdata< double >( "jet_id_eff_sf" )  );
    FillVar(prefix+"_met_mt", m_tools->MT( ptaus->at(i), pmet) * MeV2GeV );
  }

}

void xChannelA::FillCrashTest(){

  //crash test
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(1, 100);
  for(int i = 0; i < 5000; i++){
    FillVar("test_"+std::to_string(i), dis(gen) );
  }

}

/*


  template<typename T>
  void xChannelA::FillBranches( const std::initializer_list<T *> &){



  }
*/
/*
  void xChannelA::FillBranches( const Input_t &){

  }

  void xChannelA::FillBranches( const S_t &){

  }
*/
/*void xChannelA::FillBranches( const std::initializer_list<xAOD::IParticleContainer *> &){

//  IParticleContainer *c1;
//  Input_t input = { {c1}, 10, 3.14 };

FillBranches( ({12,13}, 3, 2.14) );

}
*/
/*
  template <typename T>
  void xChannel::do_stuff(typename enable_if<std::is_integral<T>::value, T>::type &t) { //implementation for taus

  }
*/
/*
  template<typename T,
  typename std::enable_if< std::is_class<T>::value &&
  std::is_base_of<IParticleContainer, T>::value &&
  std::is_same<xAOD::TauJetContainer, T>::value,
  int>::type = 0>
  inline void xChannel::FillCase( T *t)
  {



  }
*/
