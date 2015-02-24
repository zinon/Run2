#ifndef MainAnalysis_MainAnalysisCPToolsConf_H
#define MainAnalysis_MainAnalysisCPToolsConf_H

/*
std::unique_ptr<Foo> make_foo(int arg) {
  // Args passed to Foo constructor.
  auto ptr = CxxUtils::make_unique<Foo> (arg);
  return;
  //here
}
*/


template <typename T, typename std::enable_if< std::is_class<T>::value && std::is_base_of<asg::AsgTool, T>::value, int >::type = 0>
  std::unique_ptr<T> MakeCPTool(const std::string &arg){
    
  Info("check - ", "make tool %s", typeid(T).name() );
  return CxxUtils::make_unique<T> (arg);

}

//check CP initialization
template<typename T, typename std::enable_if<
  std::is_class<T>::value && 
  std::is_base_of<asg::AsgTool, T>::value, int>::type = 0> 
  inline bool INIT_CHECK( T *t) 
{
  //ZZ note: C++ compiler (>ggc-4.7) no longer performs some extra unqualified lookups it had performed in the past, 
  // namely dependent base class scope lookups and unqualified template function lookups.
  // Therefore, code must be declared before its first use.

  Info("check - ", "ini tool %s", typeid(T).name() );
  if ( ! t->initialize().isSuccess() ){
    Error("initialize()", "Failed to properly initialize %s. Exiting.", typeid(T).name() );
    return false;
  }
  Info("check - ", "ini tool DONE %s", typeid(T).name() );
  return true;
}

//check CP initialization & synchronization
template<typename T, typename U, typename std::enable_if< 
  std::is_class<T>::value && 
  std::is_class<U>::value &&
  std::is_base_of<asg::AsgTool, T>::value &&
  std::is_base_of<asg::AsgTool, U>::value, int>::type = 0>
  bool INIT_CHECK( T *t, U *u)
{
  
  if (! t->initialize().isSuccess() || t == NULL || u == NULL){
    Error("initialize()", "Failed to properly initialize %s and get it synchronized to %s. Exiting.", typeid(T).name(), typeid(U).name() );
    return false;
  }

  return true;
}

//GRL
template<typename T, typename std::enable_if<std::is_same<GoodRunsListSelectionTool, T>::value, int>::type = 0>
  bool SetupCPTool( T *t, std::unique_ptr<xJobOptions> const & jo ) //pass unique_ptr by value
{
  if(jo == nullptr){/*remove this line when jo is used below*/}
  Info("check - ", "set tool %s", typeid(T).name() );
  //t = CxxUtils::make_unique<GoodRunsListSelectionTool>("GoodRunsListSelectionTool");
  std::vector<std::string>      vecStringGRL;
  vecStringGRL.push_back("$ROOTCOREBIN/data/MainAnalysis/GRL/data12_8TeV.periodAllYear_DetStatus-v61-pro14-02_DQDefects-00-01-00_PHYS_StandardGRL_All_Good.xml");
  t->setProperty( "GoodRunsListVec", vecStringGRL);
  t->setProperty("PassThrough", false);     // if true (default) will ignore result of GRL and will just pass all events

  return INIT_CHECK(t);
}


//Pile Up
template<typename T, typename std::enable_if<std::is_same<CP::PileupReweightingTool, T>::value, int>::type = 0>
  bool SetupCPTool( std::shared_ptr<T> & t, std::unique_ptr<xJobOptions> const &jo)
  {

    //t = std::make_shared<CP::PileupReweightingTool>("PileupReweighting");

    std::vector<std::string>   confFiles;
    std::vector<std::string>   lcalcFiles;
    if( jo->get<bool>(xJobOptions::DO_PILEUP_REWEIGHTING) ){
      confFiles.push_back("MainAnalysis/PileUp/PileUpReweighting.141027.root");
      lcalcFiles.push_back("MainAnalysis/PileUp/ilumicalc_histograms_EF_mu18_tight_mu8_EFFS_202798.root");
      t->setProperty( "ConfigFiles", confFiles);
      t->setProperty( "LumiCalcFiles", lcalcFiles);
      t->setProperty("UnrepresentedDataAction",2);
      t->setProperty("DefaultChannel", 110101);
    }

    return 1;//INIT_CHECK(t);
  }


//Jet cleaning
template<typename T, typename std::enable_if<std::is_same<JetCleaningTool, T>::value, int>::type = 0>
  bool SetupCPTool( std::shared_ptr<T> & t, std::unique_ptr<xJobOptions> const & jo)
  {

    //t = std::make_shared<JetCleaningTool>("JetCleaning");
    t->msg().setLevel( MSG::ERROR );
    t->setProperty( "CutLevel", jo->get<std::string>(xJobOptions::JET_CLEANING));

    return 1;//INIT_CHECK(t);
  }

//Jet Energy Resolution
template<typename T, typename std::enable_if<std::is_same<JERTool, T>::value, int>::type = 0>
  bool SetupCPTool( std::shared_ptr<T> & t, std::unique_ptr<xJobOptions> const & jo)
{

  if(jo == nullptr){/*remove this line when jo is used below*/}
  
  //t = std::make_shared<JERTool>("JERTool");
  t->msg().setLevel( MSG::WARNING );
  t->setProperty("PlotFileName", "JetResolution/JERProviderPlots_2012.root");
  t->setProperty("CollectionName", "AntiKt4LCTopoJets") ;
  t->setProperty("BeamEnergy", "8TeV") ;
  t->setProperty("SimulationType", "FullSim");

  return 1;//INIT_CHECK(t);
}

//Jet Energy Resolution Smearing
template<typename T, typename U, typename std::enable_if<std::is_same<JERSmearingTool, T>::value && std::is_same<JERTool, U>::value, int>::type = 0>
  bool SetupCPTool( std::shared_ptr<T> & t, std::shared_ptr<U> & u, std::unique_ptr<xJobOptions> const & jo)
{

  if(jo == nullptr){/*remove this line when jo is used below*/}

  //t = std::make_shared<JERSmearingTool>("JERSmearingTool"); //instantiate the first tool
  t->msg().setLevel( MSG::WARNING );
  t->setJERTool( u.get() ); //connect and synchronize the two tools

  return 1;//INIT_CHECK(t, u);
}

//Jet calibration
template<typename T, typename std::enable_if<std::is_same<JetCalibrationTool, T>::value, int>::type = 0>
  bool SetupCPTool( std::shared_ptr<T> & t, std::unique_ptr<xJobOptions> const & jo)
{
    std::string JetCalibSeq =  jo->GetAuxInfo<bool>("isData") ? "EtaJES_Insitu" : "EtaJES";
    std::string JetCalibConf = "JES_Full2012dataset_Preliminary_MC14.config";

    t = std::make_shared<JetCalibrationTool>( "JetCalibrationTool",
							   jo->get<std::string>(xJobOptions::JET_COLLECTION),
							   JetCalibConf,
							   JetCalibSeq,
							   jo->GetAuxInfo<bool>("isData") );
    t->msg().setLevel( MSG::ERROR );
    t->initializeTool("JetCalibrationTool");

    return 1;//INIT_CHECK(t);
}

//Jet uncertainties
template<typename T, typename std::enable_if<std::is_same<JetUncertaintiesTool, T>::value, int>::type = 0>
  bool SetupCPTool( std::shared_ptr<T> & t, std::unique_ptr<xJobOptions> const & jo)
{

  t = std::make_shared<JetUncertaintiesTool>("JESUncProvider");
  t->msg().setLevel( MSG::WARNING );
  t->setProperty("JetDefinition", jo->get<std::string>(xJobOptions::JET_COLLECTION)); 
  t->setProperty("MCType","MC12"); 
  t->setProperty( "ConfigFile","JES_2012/Final/InsituJES2012_14NP.config");

  return 1;//INIT_CHECK(t);

}

//Tau smearing
template<typename T, typename std::enable_if<std::is_same<TauAnalysisTools::TauSmearingTool, T>::value, int>::type = 0>
  bool SetupCPTool( std::shared_ptr<T> & t, std::unique_ptr<xJobOptions> const & jo)
{
 
  if(jo == nullptr){/*remove this line when jo is used below*/}

  t = std::make_shared<TauAnalysisTools::TauSmearingTool>("TauSmearingTool");
  t->msg().setLevel( MSG::ERROR );

  return 1;//INIT_CHECK(t);

}

//Tau selection
template<typename T, typename std::enable_if<std::is_same<TauAnalysisTools::TauSelectionTool, T>::value, int>::type = 0>
  bool SetupCPTool( std::shared_ptr<T> & t, std::unique_ptr<xJobOptions> const & jo)
{
 
  t =  CxxUtils::make_unique<TauAnalysisTools::TauSelectionTool>("MainAnalysisTauSelectionTool");
  t->msg().setLevel( MSG::INFO );

  //use predefined cuts
  if( jo->get<bool>(xJobOptions::TAU_SELECTION_RECOMMENDED) )
    t->setRecommendedProperties();
  else{
    //define cuts by hand for now  - make this real job options
    std::vector<double> jobOptions_tau_abs_eta_region = {0, 1.37, 1.52, 2.5};   // define eta regions, excluding crack region
    double jobOptions_tau_abs_charge     = 1 ;      // define absolute charge requirement, in general should be set to 1
    std::vector<size_t> jobOptions_tau_n_tracks = {1,3};        // define number of tracks required, most analysis use 1 and 3 track taus

    //set properties
    t->setProperty("AbsEtaRegion", jobOptions_tau_abs_eta_region);
    t->setProperty("PtMin", jo->get<double>(xJobOptions::TAU_MIN_PT));
    t->setProperty("AbsCharge", jobOptions_tau_abs_charge);
    t->setProperty("NTracks", jobOptions_tau_n_tracks);
    t->setProperty("JetIDWP", int(jo->TauJetBDTindex(jo->get<std::string>(xJobOptions::TAU_JET_BDT))) );
    t->setProperty("EleBDTWP", int(jo->TauEleBDTindex(jo->get<std::string>(xJobOptions::TAU_ELE_BDT))) );

    //cuts to be executed:
    t->setProperty( "SelectionCuts",
		    int(
			TauAnalysisTools::CutPt
			| TauAnalysisTools::CutAbsEta
			| TauAnalysisTools::CutAbsCharge
			| TauAnalysisTools::CutNTrack
			| TauAnalysisTools::CutJetIDWP
			| TauAnalysisTools::CutEleBDTWP
			) );
  }

  return 1;//INIT_CHECK(t);

}

//Tau efficiency correction
template<typename T, typename U, typename std::enable_if<std::is_same<TauAnalysisTools::TauEfficiencyCorrectionsTool, T>::value && 
  std::is_same<TauAnalysisTools::TauSelectionTool, U>::value, int>::type = 0>
  bool SetupCPTool( std::shared_ptr<T> & t, std::shared_ptr<U> & u, std::unique_ptr<xJobOptions> const & jo)
{
  if(jo == nullptr){/*remove this line when jo is used below*/}
 
  t =  std::make_shared<TauAnalysisTools::TauEfficiencyCorrectionsTool>("TauEfficiencyCorrectionsTool", u.get());//connect & synchronize the two tau tools
  t->msg().setLevel( MSG::ERROR );
  t->setProperty("EfficiencyCorrectionType", (int) TauAnalysisTools::SFJetID); // set this as job option ...
  //t->setProperty("IDLevel", (int) TauAnalysisTools::JETIDBDTLOOSE);
  //t->setProperty("SharePath","RootCoreBin/data/TauAnalysisTools/EfficiencyCorrections/");

  return 1;//INIT_CHECK(t, u);
}

//muon calibration and smearing
template<typename T, typename std::enable_if<std::is_same<CP::MuonCalibrationAndSmearingTool, T>::value, int>::type = 0>
  bool SetupCPTool( std::shared_ptr<T> & t, std::unique_ptr<xJobOptions> const & jo)
{
  if(jo == nullptr){/*remove this line when jo is used below*/}
 
  t = std::make_shared<CP::MuonCalibrationAndSmearingTool>( "MuonCorrectionTool" );
  t->msg().setLevel( MSG::WARNING );

  return 1;//INIT_CHECK(t);
}

//muon efficiency SFs
template<typename T, typename std::enable_if<std::is_same<CP::MuonEfficiencyScaleFactors, T>::value, int>::type = 0>
  bool SetupCPTool( std::shared_ptr<T> & t, std::unique_ptr<xJobOptions> const & jo)
{
  if(jo == nullptr){/*remove this line when jo is used below*/}
 
  t = std::make_shared<CP::MuonEfficiencyScaleFactors>("MuonEfficiencyScaleFactors");
  t->setProperty("WorkingPoint","CBandST");
  t->setProperty("DataPeriod","2012");

  return 1;//INIT_CHECK(t);
}

//muon selection
template<typename T, typename std::enable_if<std::is_same<CP::MuonSelectionTool, T>::value, int>::type = 0>
  bool SetupCPTool( std::shared_ptr<T> & t, std::unique_ptr<xJobOptions> const & jo)
{
  if(jo == nullptr){/*remove this line when jo is used below*/}
 
  t = CxxUtils::make_unique<CP::MuonSelectionTool>("MuonSelectionTool");
  t->msg().setLevel( MSG::WARNING );

  return 1;//INIT_CHECK(t);
}

//MET recalculator
template<typename T, typename std::enable_if<std::is_same<met::METRebuilder, T>::value, int>::type = 0>
  bool SetupCPTool( std::shared_ptr<T> & t, std::unique_ptr<xJobOptions> const & jo)
{
  //  t =  std::make_shared<met::METRebuilder>("METRebuilderTool");

  t->setProperty("JetColl", jo->get<std::string>(xJobOptions::MET_JETCOL)); // key to save the shallowcopyjets.first
  t->setProperty("MuonColl", jo->get<std::string>(xJobOptions::MET_MUONCOL)); // for muons
  t->setProperty("TauColl", jo->get<std::string>(xJobOptions::MET_TAUCOL)); // for taus
  t->setProperty("EleColl", jo->get<std::string>(xJobOptions::MET_ELECOL)); // for electrons
  t->setProperty("GammaColl","");//need to switch it off explicitely, otherwise it is recalculated from the Photon Container
  t->setProperty("EleTerm", jo->get<std::string>(xJobOptions::MET_ELETERM));
  t->setProperty("GammaTerm", jo->get<std::string>(xJobOptions::MET_GAMMATERM));
  t->setProperty("TauTerm", jo->get<std::string>(xJobOptions::MET_TAUTERM));
  t->setProperty("JetTerm", jo->get<std::string>(xJobOptions::MET_JETTERM));
  t->setProperty("MuonTerm", jo->get<std::string>(xJobOptions::MET_MUONTERM));
  t->setProperty("SoftTerm", jo->get<std::string>(xJobOptions::MET_SOFTTERM));
  t->setProperty("CalibJetPtCut", jo->get<double>(xJobOptions::MET_JETPTCUT) * GeV2MeV);
  t->setProperty("DoJetJVFCut", jo->get<bool>(xJobOptions::MET_JETDOJVF) );
  t->setProperty("OutputContainer", jo->get<std::string>(xJobOptions::MET_OUTMETCONT) );
  t->setProperty("OutputTotal", jo->get<std::string>(xJobOptions::MET_OUTMETTERM));

  t->msg().setLevel( MSG::INFO);

  return 1;//INIT_CHECK(t);
}



#endif
