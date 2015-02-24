#include <xCPTools/xCPTools.h>
#include <xCPTools/xCPToolsSetup.h>


bool xCPTools::Setup(/*const xJobOptions *jo*/){

  if(m_jo == nullptr) {
    Error("xCPTools::Setup", "Not valid pointer to job options");
    return false;
  }

  if( ! Construct() ) return false;

  return true;
}

bool xCPTools::Construct(){

  // Construct, setup and initialize CP tools
  // Order is very important to get certain tools synchronized

  //Jet Tools
  m_JetCleaningTool = CxxUtils::make_unique<JetCleaningTool> ("JetCleaningTool") ;
  if( ! SetupTool(m_JetCleaningTool.get(), m_jo) ) return false;

  m_JERTool = CxxUtils::make_unique<JERTool> ("JERTool") ;
  if( ! SetupTool(m_JERTool.get(), m_jo) ) return false;

  m_JERSmearingTool = CxxUtils::make_unique<JERSmearingTool> ("JERSmearingTool") ;
  if( ! SetupTool(m_JERSmearingTool.get(), m_JERTool.get(), m_jo) ) return false;

  std::string JetCalibSeq =  m_jo->GetAuxInfo<bool>("isData") ? "EtaJES_Insitu" : "EtaJES";
  std::string JetCalibConf = "JES_Full2012dataset_Preliminary_MC14.config";
  m_JetCalibrationTool = CxxUtils::make_unique<JetCalibrationTool>( "JetCalibrationTool",
                                                                    m_jo->get<std::string>(xJobOptions::JET_COLLECTION),
                                                                    JetCalibConf,
                                                                    JetCalibSeq,
                                                                    m_jo->GetAuxInfo<bool>("isData") );
  if( ! SetupTool(m_JetCalibrationTool.get(), m_jo) ) return false;

  m_JetUncertaintiesTool = CxxUtils::make_unique<JetUncertaintiesTool> ("JetUncertaintiesTool") ;
  if( ! SetupTool(m_JetUncertaintiesTool.get(), m_jo) ) return false;


  //Muon Tools
  m_MuonCalibrationAndSmearingTool = CxxUtils::make_unique<CP::MuonCalibrationAndSmearingTool> ("MuonCalibrationAndSmearingTool") ;
  if( ! SetupTool(m_MuonCalibrationAndSmearingTool.get(), m_jo) ) return false;

  m_MuonEfficiencyScaleFactors = CxxUtils::make_unique<CP::MuonEfficiencyScaleFactors>("MuonEfficiencyScaleFactors");
  if( ! SetupTool(m_MuonEfficiencyScaleFactors.get(), m_jo) ) return false;

  m_MuonSelectionTool = CxxUtils::make_unique<CP::MuonSelectionTool>("MuonSelectionTool");
  if( ! SetupTool(m_MuonSelectionTool.get(), m_jo) ) return false;


  //Electron Tools
  m_EgammaCalibrationAndSmearingTool = CxxUtils::make_unique<CP::EgammaCalibrationAndSmearingTool> ("EgammaCalibrationAndSmearingTool") ;
  if( ! SetupTool(m_EgammaCalibrationAndSmearingTool.get(), m_jo) ) return false;
  
  m_EleIsoTool = CxxUtils::make_unique<CP::ElectronIsolationSelectionTool> ("ElectronIsolationSelectionTool") ;
  if( ! SetupTool(m_EleIsoTool.get(), m_jo) ) return false;

  //Tau tools
  m_TauSmearingTool = CxxUtils::make_unique<TauAnalysisTools::TauSmearingTool> ("TauSmearingTool") ;
  if( ! SetupTool(m_TauSmearingTool.get(), m_jo) ) return false;

  m_TauSelectionTool = CxxUtils::make_unique<TauAnalysisTools::TauSelectionTool> ("TauSelectionTool") ;
  if( ! SetupTool(m_TauSelectionTool.get(), m_jo) ) return false;

  m_TauEfficiencyCorrectionsTool = CxxUtils::make_unique<TauAnalysisTools::TauEfficiencyCorrectionsTool> ("TauEfficiencyCorrectionsTool") ;
  if( ! SetupTool(m_TauEfficiencyCorrectionsTool.get(), m_TauSelectionTool.get(), m_jo) ) return false;

  //MET tools
  m_METRebuilder = CxxUtils::make_unique<met::METRebuilder> ("METRebuilder") ;
  if( ! SetupTool(m_METRebuilder.get(), m_jo) ) return false;

  //common tools
  m_GoodRunsListSelectionTool = CxxUtils::make_unique<GoodRunsListSelectionTool> ("GoodRunsListSelectionTool") ;
  if( ! SetupTool(m_GoodRunsListSelectionTool.get(), m_jo) ) return false;

  m_PileupReweightingTool = CxxUtils::make_unique<CP::PileupReweightingTool> ("PileupReweightingTool") ;
  if( ! SetupTool(m_PileupReweightingTool.get(), m_jo) ) return false;

  return true;

}


/*auto xCPTools::ToolAccess(const std::string &name) -> asg::AsgTool * {

  if(name == "JetCleaningTool") return m_cp_registry["JetCleaningTool"].get();

  return m_cp_registry["MuonCalibrationAndSmearingTool"].get();

  }*/

bool xCPTools::ApplySystematicVariation(const CP::SystematicSet & syst){

  // CP tools which have to know about the systematic variation
  // Can be templated in the future

  //muon tools
  if( m_MuonCalibrationAndSmearingTool->applySystematicVariation( syst ) != CP::SystematicCode::Ok ){
    Error("xCPTools::ApplySystematicVariation()", "Cannot configure MuonCalibrationAndSmearingTool for systematic %s", syst.name().c_str());
    return false;
  }

  if( m_MuonEfficiencyScaleFactors->applySystematicVariation( syst ) != CP::SystematicCode::Ok ){
    Error("xCPTools::ApplySystematicVariation()", "Cannot configure MuonEfficiencyScaleFactorsTool for systematic %s", syst.name().c_str());
    return false;
  }

  //electron tools
  if( m_EgammaCalibrationAndSmearingTool->applySystematicVariation( syst ) != CP::SystematicCode::Ok ){
    Error("xCPTools::ApplySystematicVariation()", "Cannot configure EgammaCalibrationAndSmearingTool for systematic %s", syst.name().c_str());
    return false;
  }
 
  //jet tools
  if( m_JERSmearingTool->applySystematicVariation( syst ) != CP::SystematicCode::Ok ) {
    Error("xCPTools::ApplySystematicVariation()", "Cannot configure JER smearing tool for systematic" );

  }

  if( m_JetUncertaintiesTool->applySystematicVariation( syst ) != CP::SystematicCode::Ok ) {
    Error("xCPTools::ApplySystematicVariation()", "Cannot configure JES uncertainty tool for systematic" );

  }

  //tau tools
  if( m_TauSmearingTool->applySystematicVariation( syst ) != CP::SystematicCode::Ok ) {
    Error("xCPTools::ApplySystematicVariation()", "Cannot configure tau smearing tool for systematic" );

  }

  if( m_TauEfficiencyCorrectionsTool->applySystematicVariation( syst ) != CP::SystematicCode::Ok ) {
    Error("xCPTools::ApplySystematicVariation()", "Cannot configure tau efficincy tool for systematic" );

  }



  return true;

  //  return false;

}

template<typename T>
void xCPTools::func(T t){}
