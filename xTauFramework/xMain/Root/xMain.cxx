#include <xMain/xMain.h>
#include <xMain/xMainIncludes.h>

#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h> //wk()
#include <EventLoop/OutputStream.h>


// this is needed to distribute the algorithm to the workers
ClassImp(xMain)

xMain :: xMain ()
{
  Info("ctor()", "call");
}

EL::StatusCode xMain :: setupJob (EL::Job& job)
{
  Info("setupJob()", "call");
  // Here you put code that sets up the job on the submission object
  // so that it is ready to work with your algorithm, e.g. you can
  // request the D3PDReader service or add output files.  Any code you
  // put here could instead also go into the submission script.  The
  // sole advantage of putting it here is that it gets automatically
  // activated/deactivated when you add/remove the algorithm from your
  // job, which may or may not be of value to you.

  job.useXAOD ();

  EL::OutputStream out( m_output_label );
  job.outputAdd( out ); 

  //timer
  m_timer = CxxUtils::make_unique<xTimer>();
  m_timer->start("setupJob");

  // xJobOptions
  m_jo = CxxUtils::make_unique<xJobOptions>(m_job_options_file);
  if( !m_jo->Configure() )
    Error("setupJob()", "Failed to configure the job options class instance with input file %s",  m_job_options_file.c_str());

  // add PileUpReweighting to file
  EL::OutputStream pu_output("PileUpReweighting");
  if(m_jo->get<bool>(xJobOptions::DO_PILEUP_FILE)){
    Info("setupJob()", "Will create PileUpReweighting output file" );
    job.outputAdd(pu_output);
  }

  // let's initialize the algorithm to use the xAODRootAccess package
  xAOD::Init( "xMain" ).ignore(); // call before opening first file

  m_timer->stop("setupJob");

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode xMain :: fileExecute()
{
  Info("fileExecute()", "call");
  m_timer->start("fileExecute");
  // Here you do everything that needs to be done exactly once for every
  // single     file, e.g. collect a list of all lumi-blocks processed
  m_timer->stop("fileExecute");
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode xMain :: changeInput (bool firstFile)
{
  Info("changeInput()", "call");
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.

  m_timer->start("changeInput");

  //retrieve sum of mc weights from MetaData if running on DxAOD
  if(m_isMC && m_jo->get<bool>(xJobOptions::DO_DERIVATION)){
    Info("changeInput()", "Retrieving Sum of weights");

    double finalSumOfWeightsInFile = 0.;
    double initialSumOfWeightsInFile = 0.;

    if(firstFile){
      //mc weights for DxAODs
      std::cout<<"Processing First file, setting mc_weight_sums to zero"<<std::endl;
      m_initialSumOfWeights  = 0;
      m_finalSumOfWeights = 0;
    }

    // get the MetaData tree once a new file is opened, with
    TTree *MetaData = dynamic_cast<TTree*>(wk()->inputFile()->Get("MetaData"));
    if (MetaData) {
      // extract the information from the EventBookkeepers branch
      TTreeFormula tf("tf","EventBookkeepers.m_nWeightedAcceptedEvents",MetaData);
      MetaData->LoadTree(0);
      tf.UpdateFormulaLeaves();
      tf.GetNdata();
      finalSumOfWeightsInFile = tf.EvalInstance(0);
      initialSumOfWeightsInFile = tf.EvalInstance(1);
    }

    Info("changeInput()", "In Original xAOD: %f  In Derivation: %f", initialSumOfWeightsInFile, finalSumOfWeightsInFile);

    m_finalSumOfWeights += finalSumOfWeightsInFile;
    m_initialSumOfWeights += initialSumOfWeightsInFile;
  }

  //TODO: release and delete MetaData ptr or make it unique_ptr?
  
  m_timer->stop("changeInput");

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode xMain :: initialize ()
{
  Info("initialize()", "call");

  m_timer->start("initialize");

  // create a local reference to the xAODRootAccess event object
  m_event = wk()->xaodEvent();

  // here we are defining the TStore this tool needs
  // Create a transient object store needed for METRebuilder
  m_store = CxxUtils::make_unique<xAOD::TStore>();

  //number of entries
  m_entries = static_cast< int >( m_event->getEntries() );
  Info("initialize()", "Number of events = %i", m_entries );

  // event info
  const xAOD::EventInfo* eventInfo = 0;
  if( !m_event->retrieve( eventInfo, "EventInfo").isSuccess() ){
    Error("initialize()", "Failed to retrieve EventInfo. Exiting." );
    return EL::StatusCode::FAILURE;
  }
  bool  isMC = false;
  if( eventInfo->eventType(xAOD::EventInfo::IS_SIMULATION) ) isMC = true;
  bool  isData = !isMC;

  m_jo->SetAuxInfo<bool>("isMC", isMC);
  m_jo->SetAuxInfo<bool>("isData", isData);

  if( m_jo->GetAuxInfo<bool>("isMC"))
    Info("initialize()", "Running on MC" );

  //View containers - deep copy containers
  m_ViewElemJetCont  = CxxUtils::make_unique<xAOD::JetContainer>(SG::VIEW_ELEMENTS);
  m_ViewElemTauCont  = CxxUtils::make_unique<xAOD::TauJetContainer>(SG::VIEW_ELEMENTS);
  m_ViewElemMuonCont = CxxUtils::make_unique<xAOD::MuonContainer>(SG::VIEW_ELEMENTS);
  m_ViewElemEleCont  = CxxUtils::make_unique<xAOD::ElectronContainer>(SG::VIEW_ELEMENTS);

  // count number of events - all these counters will go in one vector or a class one day
  m_eventCounter   = 0; m_numGrlEvents   = 0; m_numCleanEvents = 0; m_numWeightedEvents = 0.;

  //other counters
  m_numGoodJets = 0;  m_numGoodTaus = 0;

  //event information
  m_eventNumber = -999.;  m_runNumber = -999.; m_mcChannelNumber = -999.; m_rndRunNumber = -999.;

  //mc event weight safety init - only called when vars not supposed to be used (ie data/xAOD)
  if(!(m_isMC) || !(m_jo->get<bool>(xJobOptions::DO_DERIVATION))){
    m_initialSumOfWeights  = -999;     m_finalSumOfWeights = -999;
  }

  // xTools Helper class
  m_tools = CxxUtils::make_unique<xTools>();

  // xCPTools container class
  m_cp_tools = CxxUtils::make_unique<xCPTools>( m_jo.get() );
  m_cp_tools->Setup();//construct and initialize tools

  //====================
  // Systematics       ||
  //====================

  //clear syst list
  m_sysList.clear();

  // loop over systematics registry:
  const CP::SystematicRegistry &registry = CP::SystematicRegistry::getInstance();
  const CP::SystematicSet &recommendedSystematics = registry.recommendedSystematics();     // get list of recommended systematics

  // this is the nominal set, no systematic
  m_sysList.push_back(CP::SystematicSet());

  // using PAT method to derive +- 1sigma systematics
  if( m_jo->get<bool>(xJobOptions::DO_SYSTEMATICS) )
    m_sysList = CP::make_systematics_vector(recommendedSystematics);

  //list of strings of  systematics
  std::vector<std::string>  m_syst;
  for( auto &syst : m_sysList ){
    if( syst.name().empty() )
      m_syst.push_back( "NOMINAL" );
    else
      m_syst.push_back( m_tools->replace_substr( syst.name(), "_", "") );
  }
  Info("initialize()", "Number of systematic variations %lu", m_sysList.size() );


  //===============
  // Channels    ||
  //===============
  TFile* outputfile = wk()->getOutputFile( m_output_label ); 
  m_channel =  CxxUtils::make_unique<xChannelA>( wk(), outputfile, m_sysList );

  //===============
  // Tree booking ||
  //===============
  /*
  //tree initialization
  for( const auto &syst : m_syst){
    Info("initialize()", "systematic: %s", syst.c_str());
    m_Tree.insert(std::make_pair(syst,  new TTree(syst.c_str(), syst.c_str()) )  ); // to-do: this "new" will change soon
  }

  //link variables to tree branches - variables are defined in xMainVariables.h
  for( auto &tr : m_Tree ){
    tr.second->SetMaxTreeSize();
    for( auto &var : m_tree_var){
      tr.second->Branch( var.first.c_str(), &var.second, (var.first+"/D").c_str() );
    }
  }

  //add trees to wk output
  for( const auto &tr : m_Tree )
    wk()->addOutput(tr.second);
  */

  m_timer->stop("initialize");
  m_timer->start("execute");

  return EL::StatusCode::SUCCESS;
}


EL::StatusCode xMain :: execute ()
{
  if(m_eventCounter<1) Info("execute()", "call");
  // Event initializations
  m_evtw  = 1.; //total event weight
  m_puw = 1.; //pile up weight
  m_mcw = 1.; //mc event weight

  if( (m_eventCounter % 1000) ==0 ) Info("execute()", "Event   number = %i", m_eventCounter );
  m_eventCounter++;

  // Event information
  const xAOD::EventInfo *eventInfo = 0;
  if( ! m_event->retrieve( eventInfo, "EventInfo").isSuccess() ){
    Error("execute()", "Failed to retrieve event info. Exiting." );
    return EL::StatusCode::FAILURE;
  }

  m_isMC = eventInfo->eventType( xAOD::EventInfo::IS_SIMULATION );

  const xAOD::VertexContainer* vertices = NULL;
  if( !m_event->retrieve( vertices, "PrimaryVertices").isSuccess() ){
    Error("initialize()", "Failed to retrieve PrimaryVertices!" );
    return EL::StatusCode::FAILURE;
  }

  int m_npv = 0;
  xAOD::VertexContainer::const_iterator vitr;
  for (vitr = vertices->cbegin(); vitr != vertices->cend(); ++vitr){
    if ( (*vitr)->nTrackParticles() > 1) m_npv++;
  }

  //  std::cout<<"npv: "<<NPV<<std::endl;


  //retrieve the mc
  const std::vector<float> weights = m_isMC ? eventInfo->mcEventWeights() : std::vector<float>({});
  m_mcw = weights.size() > 0 ? weights[0] : 1.;
  m_evtw *= m_mcw; //update total event weight

  //retrieving event info
  m_eventNumber     = static_cast< int >(eventInfo->eventNumber() );
  m_runNumber       = static_cast< int >( eventInfo->runNumber() );
  m_mcChannelNumber = m_isMC ? (static_cast< int >( eventInfo->mcChannelNumber() )) : 0;

  //pileup reweighting : TODO move somewhere else?
  if(m_isMC && (m_jo->get<bool>(xJobOptions::DO_PILEUP_REWEIGHTING) || m_jo->get<bool>(xJobOptions::DO_PILEUP_FILE)) ){

    m_cp_tools->AccessTool<CP::PileupReweightingTool>()->apply(eventInfo);

    if(m_jo->get<bool>(xJobOptions::DO_PILEUP_REWEIGHTING)){
      m_puw = eventInfo->auxdata< double >( "PileupWeight" );
      m_evtw *= m_puw; //update total event weight
    }
  }
  m_numWeightedEvents +=m_evtw;

  //stop here if generating a pileup root file
  if(m_jo->get<bool>(xJobOptions::DO_PILEUP_FILE)) return EL::StatusCode::SUCCESS;

  //GRL:  if data check if event passes GRL
  if( IsData() ){
    if(! m_cp_tools->AccessTool<GoodRunsListSelectionTool>()->passRunLB(*eventInfo)){
      Info("execute()", "GRL - Discarding event  %i", m_eventNumber );
      return EL::StatusCode::SUCCESS;   // go to next event
    }
  }
  m_numGrlEvents++;

  // Apply event cleaning to remove events due to  problematic regions of the detector, and incomplete events: on data
  if( IsData() ){
    if( (eventInfo->eventFlags(xAOD::EventInfo::LAr)==2) || (eventInfo->eventFlags(xAOD::EventInfo::Tile) == 2) || (eventInfo->eventFlags(xAOD::EventInfo::Core)!=0) ){
      return EL::StatusCode::SUCCESS;   // go to the next event
    }
  }
  m_numCleanEvents++;

  //----------------------------
  // Retrieve xAOD containers  |
  //----------------------------

  // retrieve tau container
  const xAOD::TauJetContainer *TauJetCont = 0;
  if(!m_event->retrieve(TauJetCont, "TauRecContainer" ).isSuccess()){
    Error("execute()", "Failed to retrieve TauRecContainer container. Exiting." );
    return EL::StatusCode::FAILURE;
  }

  // Retrieve the jet container
  const xAOD::JetContainer*   JetCont = 0;
  if ( !m_event->retrieve( JetCont, (m_jo->get<std::string>(xJobOptions::JET_COLLECTION)+"Jets").c_str() ).isSuccess() ){
    Error("execute()", "Failed to retrieve %s container. Exiting...", m_jo->get<std::string>(xJobOptions::JET_COLLECTION).c_str() );
    return EL::StatusCode::FAILURE;
  }

  // Get the Muon container
  const xAOD::MuonContainer*  MuonCont = 0;
  if( m_event->contains<xAOD::MuonContainer>("Muons") ){
    if ( !m_event->retrieve( MuonCont, "Muons" ).isSuccess() ){
      Error("execute()", "Failed to retrieve Muons container. Exiting." );
      return EL::StatusCode::FAILURE;
    }
  }else{
    Error("execute()", "No Muons container in event %i, run %i. Exiting.", m_eventNumber, m_runNumber );
    return EL::StatusCode::FAILURE;
  }

  // Retrieve the Electrons container
  const xAOD::ElectronContainer*  EleCont = 0;
  if( m_event->contains<xAOD::ElectronContainer>("ElectronCollection") ){
    if ( !m_event->retrieve( EleCont, "ElectronCollection" ).isSuccess() ){
      Error("execute()", "Failed to retrieve Electrons container. Exiting." );
      return EL::StatusCode::FAILURE;
    }
  }else{
    Error("execute()", "No Electrons container in event %i, run %i. Exiting.", m_eventNumber, m_runNumber );
    return EL::StatusCode::FAILURE;
  }

  // Get the default MET container and terms
  const xAOD::MissingETContainer*     MissingETCont = 0;
  if( m_event->contains<xAOD::MissingETContainer>(m_jo->get<std::string>(xJobOptions::MET_CONTAINER)) ){
    if( !m_event->retrieve(MissingETCont, m_jo->get<std::string>(xJobOptions::MET_CONTAINER)) ){
      Error("execute()", "Failed to retrieve the %s container. Exiting.", m_jo->get<std::string>(xJobOptions::MET_CONTAINER).c_str());
      return EL::StatusCode::FAILURE;
    }
  }else{
    Warning("execute()", "Did not find %s container...", m_jo->get<std::string>(xJobOptions::MET_CONTAINER).c_str());
  }

  // Retrieve MET and subterms
  const xAOD::MissingET *MET = 0;
  const xAOD::MissingET *MET_SoftTerm = 0;
  const xAOD::MissingET *MET_JetTerm  = 0;
  const xAOD::MissingET *MET_TauTerm = 0;
  const xAOD::MissingET *MET_MuonTerm = 0;
  const xAOD::MissingET *MET_EleTerm = 0;
  if( MissingETCont ){
    MET  = (*MissingETCont)[m_jo->get<std::string>(xJobOptions::MET_OUTMETTERM)];
    if(!MET){
      Error("execute()", "Null pointer to %s out MET term", m_jo->get<std::string>(xJobOptions::MET_OUTMETTERM).c_str());
      return EL::StatusCode::FAILURE;
    }

    MET_SoftTerm = (*MissingETCont)[m_jo->get<std::string>(xJobOptions::MET_SOFTTERM)];
    if(!MET_SoftTerm){
      Error("execute()", "Null pointer to MET %s term", m_jo->get<std::string>(xJobOptions::MET_SOFTTERM).c_str());
      return EL::StatusCode::FAILURE;
    }

    MET_JetTerm = (*MissingETCont)[m_jo->get<std::string>(xJobOptions::MET_JETTERM)];
    if(!MET_JetTerm){
      Error("execute()", "Null pointer to MET %s term", m_jo->get<std::string>(xJobOptions::MET_JETTERM).c_str());
      return EL::StatusCode::FAILURE;
    }

    MET_TauTerm = (*MissingETCont)[m_jo->get<std::string>(xJobOptions::MET_TAUTERM)];
    if(!MET_TauTerm){
      Error("execute()", "Null pointer to MET %s term", m_jo->get<std::string>(xJobOptions::MET_TAUTERM).c_str());
      return EL::StatusCode::FAILURE;
    }

    MET_MuonTerm = (*MissingETCont)[m_jo->get<std::string>(xJobOptions::MET_MUONTERM)];
    if(!MET_MuonTerm){
      Error("execute()", "Null pointer to MET %s term", m_jo->get<std::string>(xJobOptions::MET_MUONTERM).c_str());
      return EL::StatusCode::FAILURE;
    }

    MET_EleTerm = (*MissingETCont)[m_jo->get<std::string>(xJobOptions::MET_ELETERM)];
    if(!MET_MuonTerm){
      Error("execute()", "Null pointer to MET %s term", m_jo->get<std::string>(xJobOptions::MET_ELETERM).c_str());
      return EL::StatusCode::FAILURE;
    }

  }else{
    Error("execute()", "Null pointer to MissingET container");
    return EL::StatusCode::FAILURE;
  }

  //-------------------
  // Define Goodness  |
  //-------------------

  //list of cuts on muons that define the good ones
  std::vector<std::string> vMuonGoodFlags { "accepted", "pt_cut", "iso_cut"};

  //list of cuts on electrons that define the good ones
  std::vector<std::string> vEleGoodFlags { "accepted"};//, "no_muon_overlap", "pt_cut", "eta_cut", "iso_cut"};

  //list of cuts on taus that define the good ones
  std::vector<std::string> vTauGoodFlags { "accepted", "no_muon_overlap" };

  //list of cuts on jets that define the good ones
  std::vector<std::string> vJetGoodFlags { "clean", "pt_cut", "eta_cut" , "no_tau_overlap", "jvf_cut"};

  //---------------
  // Systematics  |
  //---------------

  std::vector<CP::SystematicSet>::const_iterator sysListItr_end = m_jo->get<bool>(xJobOptions::DO_SYSTEMATICS) ? m_sysList.cend() : m_sysList.cbegin() + 1;
  for(std::vector<CP::SystematicSet>::const_iterator sysListItr = m_sysList.cbegin(); sysListItr != sysListItr_end; ++sysListItr){

    std::string systname = (*sysListItr).name(); //tmp - for histos
    std::string systName = (*sysListItr).name().empty() ? "NOMINAL" :  m_tools->replace_substr( (*sysListItr).name(), "_", "");//name for tree keyword


    // apply recommended systematic for CP Tools |
    if( ! m_cp_tools->ApplySystematicVariation(*sysListItr) ) return EL::StatusCode::FAILURE;

    // -----------------------
    // Transient Containers  |
    // -----------------------
    // Inside the syst loop: recreate shallow copies in every systematics iteration and apply smearing for each systematc independently

    //create a tau shallow copy
    m_TauJetContShallowCopy = xAOD::shallowCopyContainer( *TauJetCont );

    //jet  shallow copy
    m_JetContShallowCopy = xAOD::shallowCopyContainer( *JetCont );

    // create a shallow copy of the muons container
    m_MuonContShallowCopy = xAOD::shallowCopyContainer( *MuonCont );

    // create a shallow copy of the electrons container
    m_EleContShallowCopy = xAOD::shallowCopyContainer( *EleCont );

    //--------------
    // Muons       |
    //--------------
    unsigned short nGoodMuons(0);
    for(auto MuonShallowCopyItr = (m_MuonContShallowCopy.first)->begin(); MuonShallowCopyItr != (m_MuonContShallowCopy.first)->end(); MuonShallowCopyItr++) {

      //calibration and smearing
      if(m_jo->get<bool>(xJobOptions::MUON_CALIBRATION_SMEARING)){
        CP::CorrectionCode res = m_cp_tools->AccessTool<CP::MuonCalibrationAndSmearingTool>()->applyCorrection(**MuonShallowCopyItr);
        if( res == CP::CorrectionCode::Error ){
          Warning("execute()","Cannot apply shallow-copy muon calibration/smearing run %i event %i", m_runNumber, m_eventNumber);
          //return EL::StatusCode::FAILURE; // do what?
        }
      }

      //Efficiency SF
      float effSF = 1.0;
      if(IsMC()) CP::CorrectionCode mucorrcode = m_cp_tools->AccessTool<CP::MuonEfficiencyScaleFactors>()->getEfficiencyScaleFactor(**MuonShallowCopyItr, effSF);
      (*MuonShallowCopyItr)->auxdata< float >( "eff_sf" ) = static_cast< float >( effSF );

      //isolation
      float ptc20(0);
      (*MuonShallowCopyItr)->isolation(ptc20, xAOD::Iso::ptcone20);

      //muons for tau-muon overlap removal
      ( *MuonShallowCopyItr )->auxdata< int >( "overlap" ) =
        static_cast< int >( (*MuonShallowCopyItr)->pt() * MeV2GeV > m_jo->get<double>(xJobOptions::MUON_OVERLAP_MIN_PT) &&
                            std::fabs( (*MuonShallowCopyItr)->eta() )  < m_jo->get<double>(xJobOptions::MUON_OVERLAP_MAX_ABS_ETA)
                            //other?
                            ? 1:0);

      //apply selection cuts
      ( *MuonShallowCopyItr )->auxdata< int >( "accepted" ) =
        static_cast< int >( m_cp_tools->AccessTool<CP::MuonSelectionTool>()->accept( **MuonShallowCopyItr ) ? 1:0);
      ( *MuonShallowCopyItr )->auxdata< int >( "pt_cut" ) =
        static_cast< int >( (*MuonShallowCopyItr)->pt() * MeV2GeV > m_jo->get<double>(xJobOptions::MUON_MIN_PT) ? 1:0 );
      ( *MuonShallowCopyItr )->auxdata< int >( "iso_cut" ) =
        static_cast< int >( ptc20/(*MuonShallowCopyItr)->pt() < m_jo->get<double>(xJobOptions::MUON_MAX_ISO) ? 1:0 );

      //determine goodness
      if(static_cast< int >( m_tools->Goodness(*MuonShallowCopyItr, vMuonGoodFlags) ) ){
        ( *MuonShallowCopyItr )->auxdata< int >( "good" ) = 1;
        nGoodMuons++;
      }else{
        ( *MuonShallowCopyItr )->auxdata< int >( "good" ) = 0;
      }

    }// Muon loop

    // Link to original muons needed for METRebuilder
    if( ! xAOD::setOriginalObjectLink(*MuonCont, *m_MuonContShallowCopy.first)) {
      Error("execute()", "Failed to set original muons links -- MET rebuilding cannot proceed.");
      return StatusCode::FAILURE;
    }
    // Save corrected muons in TStore for METRebuilder : hence must NOT delete them at end
    if( ! m_store->record(m_MuonContShallowCopy.first, m_jo->get<std::string>(xJobOptions::MET_MUONCOL)) ){
      Error("execute()", "Failed to record %s to TStore. Exiting...", m_jo->get<std::string>(xJobOptions::MET_MUONCOL).c_str());
      return EL::StatusCode::FAILURE;
    }
    if( ! m_store->record(m_MuonContShallowCopy.second, m_jo->get<std::string>(xJobOptions::MET_MUONCOL)+"Aux.") ){
      Error("execute()", "Failed to record %sAux. to TStore. Exiting...", m_jo->get<std::string>(xJobOptions::MET_MUONCOL).c_str() );
      return EL::StatusCode::FAILURE;
    }

    //--------------
    // Electrons    |
    //---------------
    unsigned short nGoodElectrons(0);
 //   m_cp_tools->AccessTool<CP::EgammaCalibrationAndSmearingTool>()->setDefaultConfiguration(eventInfo); // for electron momentum correction
  // interface change with ABR2.0.25
    //Info("execute()", "Electron loop...");
    for(auto EleShallowCopyItr = (m_EleContShallowCopy.first)->begin(); EleShallowCopyItr != (m_EleContShallowCopy.first)->end(); EleShallowCopyItr++) {
      m_cp_tools->AccessTool<CP::EgammaCalibrationAndSmearingTool>()->setRandomSeed(eventInfo->eventNumber()+100*(EleShallowCopyItr-(m_EleContShallowCopy.first)->begin())); // Electron momentum corr: smearing
      xAOD::Electron* el = *EleShallowCopyItr;

      // Electron momentum corr: apply calibration
      //Info("execute()", "Applying ele correction");
      if(m_jo->get<bool>(xJobOptions::ELE_CALIBRATION_SMEARING)) {

  // interface change with ABR2.0.25
         //if( m_cp_tools->AccessTool<CP::EgammaCalibrationAndSmearingTool>()->applyCorrection(*el, eventInfo) != CP::CorrectionCode::Ok) {
         if( m_cp_tools->AccessTool<CP::EgammaCalibrationAndSmearingTool>()->applyCorrection(*el) != CP::CorrectionCode::Ok) {
            Error("execute()","Cannot apply electron calibration and smearing, eta = %.2f, pt = %.2f. Exiting...", el->eta(), el->pt());
            return EL::StatusCode::FAILURE;
            }
      }

      //apply selection cuts
      bool ele_id = false;
      // DAN: should this be done using tool? When the electron is recalibrated, its ID flag might change, mightn't it?
      if( !el->passSelection(ele_id, m_jo->get<std::string>(xJobOptions::ELE_ID)) ) {
        Error("execute()", "Failed to set the %s electron menu. Exiting...", m_jo->get<std::string>(xJobOptions::ELE_ID).c_str());
        return EL::StatusCode::FAILURE;
      }
      el->auxdata< int >( "accepted" ) = static_cast< int >(ele_id ? 1:0);
      el->auxdata< int >( "no_muon_overlap" ) = static_cast< int >(! (m_tools->EleOverlapsWithGoodMuon( *EleShallowCopyItr,
                                                                                                    m_MuonContShallowCopy.first,
                                                                                                    m_jo->get<double>(xJobOptions::ELE_MUON_OVERLAP_DR)) ? 1:0) );
      el->auxdata< int >( "pt_cut" )   = static_cast< int >( el->pt() * MeV2GeV > m_jo->get<double>(xJobOptions::ELE_PT) ? 1:0 );

      float ele_eta = TMath::Abs(el->caloCluster()->eta());
      el->auxdata< int >( "eta_cut" )  = static_cast< int >( ele_eta < m_jo->get<double>(xJobOptions::ELE_ETA_CRACK_LO) || (ele_eta > m_jo->get<double>(xJobOptions::ELE_ETA_CRACK_HI) && ele_eta < m_jo->get<double>(xJobOptions::ELE_ETA_MAX)) ? 1:0 );
      el->auxdata< int >( "iso_cut" )  = static_cast< int >( m_cp_tools->AccessTool<CP::ElectronIsolationSelectionTool>()->accept(*el) ? 1:0 );

      // should this electron be used to check overlap with jets?
      bool ele_id_overlap = false;
      if( !el->passSelection(ele_id_overlap, m_jo->get<std::string>(xJobOptions::ELE_ID_FOR_TAUOVERLAP)) ) {
        Error("execute()", "Chosen selection menu unavailable for electrons");
        return EL::StatusCode::FAILURE;
      }
      // Note that looser criteria are required for overlap removal with taus
      el->auxdata< int >( "overlap" ) = static_cast< int >(ele_id_overlap &&
                                                           el->auxdata< int >( "no_muon_overlap") &&
                                                           el->auxdata< int >( "pt_cut" ) &&
                                                           el->auxdata< int >( "eta_cut" ) ? 1:0);

      //determine goodness
      if(static_cast< int >( m_tools->Goodness(el, vEleGoodFlags) ) ){
	el->auxdata< int >( "good" ) = 1;
	nGoodElectrons++;
      }else{
	el->auxdata< int >( "good" ) = 0;
      }


    }
    // end for loop over electrons

    if( ! xAOD::setOriginalObjectLink(*EleCont, *m_EleContShallowCopy.first)) {
      Error("execute()", "Failed to set original electrons links -- MET rebuilding cannot proceed. Exiting...");
      return StatusCode::FAILURE;
    }
    // Save corrected electrons in TStore for METRebuilder : hence must NOT delete them at end
    if( ! m_store->record(m_EleContShallowCopy.first, m_jo->get<std::string>(xJobOptions::MET_ELECOL) ) ){
      Error("execute()", "Failed to record %s to TStore. Exiting...", m_jo->get<std::string>(xJobOptions::MET_ELECOL).c_str());
      return EL::StatusCode::FAILURE;
    }
    if( ! m_store->record(m_EleContShallowCopy.second, m_jo->get<std::string>(xJobOptions::MET_ELECOL)+"Aux.") ){
      Error("execute()", "Failed to record %sAux. to TStore. Exiting...", m_jo->get<std::string>(xJobOptions::MET_ELECOL).c_str() );
      return EL::StatusCode::FAILURE;
    }
    //---------------
    // Taus         |
    //---------------
    unsigned short nGoodTaus(0);
    for( auto TauJetShallowCopyItr = (m_TauJetContShallowCopy.first)->begin(); TauJetShallowCopyItr != (m_TauJetContShallowCopy.first)->end(); TauJetShallowCopyItr++) {

      //tes correction, only sys in MC, nominal shift in data
      if(m_jo->get<bool>(xJobOptions::TAU_CORRECTION)){
        if( m_cp_tools->AccessTool<TauAnalysisTools::TauSmearingTool>()->applyCorrection((**TauJetShallowCopyItr)) != CP::CorrectionCode::Ok){
          Error("execute()","Cannot apply tes correction on tau with eta = %.2f, pt = %.2f, run %i event %i. Exiting...",
                (*TauJetShallowCopyItr)->eta(), (*TauJetShallowCopyItr)->pt(), m_runNumber, m_eventNumber);
          return EL::StatusCode::FAILURE;
        }
      }

      //tau efficiency
      double effSF= 1.0;
      if(IsMC())
        m_cp_tools->AccessTool<TauAnalysisTools::TauEfficiencyCorrectionsTool>()->getEfficiencyScaleFactor((**TauJetShallowCopyItr), effSF);
      //decorate tau with eff SF
      (*TauJetShallowCopyItr)->auxdata< double >( "jet_id_eff_sf" ) = static_cast< double >( effSF );

      //tau selection
      (*TauJetShallowCopyItr)->auxdata< int >( "accepted" ) =
        static_cast< int >( (m_cp_tools->AccessTool<TauAnalysisTools::TauSelectionTool>()->accept((*TauJetShallowCopyItr)) ? 1:0) );

      //tau -muon overlap
      (*TauJetShallowCopyItr)->auxdata< int >( "no_muon_overlap" ) =
        static_cast< int >( ! (m_tools->TauOverlapsWithMuon( *TauJetShallowCopyItr,
                                                         m_MuonContShallowCopy.first,
                                                         m_jo->get<double>(xJobOptions::TAU_MUON_OVERLAP_DR)) ? 1:0) );

      // tau-electron overlap
      (*TauJetShallowCopyItr)->auxdata< int >( "no_ele_overlap" ) = static_cast< int >( ! (m_tools->TauOverlapsWithEle( *TauJetShallowCopyItr,
                                                                                                                    m_EleContShallowCopy.first,
                                                                                                                    m_jo->get<double>(xJobOptions::TAU_ELE_OVERLAP_DR)) ? 1:0) );


      //define goodness
      if( static_cast< int >( m_tools->Goodness(*TauJetShallowCopyItr, vTauGoodFlags) ) ){
        ( *TauJetShallowCopyItr )->auxdata< int >( "good" ) = 1;
        nGoodTaus++;
      }else{
        ( *TauJetShallowCopyItr )->auxdata< int >( "good" ) = 0;
      }
    }

    // Link to original taus needed for METRebuilder
    if( ! xAOD::setOriginalObjectLink(*TauJetCont, *m_TauJetContShallowCopy.first)) { //method is defined in the header file xAODBase/IParticleHelpers.h
      Error("execute()", "Failed to set original tau links -- MET rebuilding cannot proceed.");
      return StatusCode::FAILURE;
    }
    // Save smeared taus in TStore for METRebuilder : hence must NOT delete them at end
    if( ! m_store->record(m_TauJetContShallowCopy.first, m_jo->get<std::string>(xJobOptions::MET_TAUCOL) ) ){
      Error("execute()", "Failed to record %s to TStore. Exiting...", m_jo->get<std::string>(xJobOptions::MET_TAUCOL).c_str());
      return EL::StatusCode::FAILURE;
    }
    if( ! m_store->record(m_TauJetContShallowCopy.second, m_jo->get<std::string>(xJobOptions::MET_TAUCOL)+"Aux.") ){
      Error("execute()", "Failed to record %sAux. to TStore. Exiting..", m_jo->get<std::string>(xJobOptions::MET_TAUCOL).c_str() );
      return EL::StatusCode::FAILURE;
    }


    //--------------
    // Jets        |
    //--------------
    int ijet(0); double tmp_jpt(0);
    unsigned short nGoodJets(0);
    for(auto JetShallowCopyItr = (m_JetContShallowCopy.first)->begin(); JetShallowCopyItr != (m_JetContShallowCopy.first)->end(); JetShallowCopyItr++){

      tmp_jpt=(*JetShallowCopyItr)->pt();

      // apply jet recalibration only for jets with a min pt
      if(m_jo->get<bool>(xJobOptions::JET_RECALIBRATION))
        if(!m_jo->get<bool>(xJobOptions::JET_PRESELECT_SKIP_CALIB_LOW_PT) || tmp_jpt*MeV2GeV > m_jo->get<double>(xJobOptions::JET_PRESELECT_CALIB_MIN_PT))
          if( m_cp_tools->AccessTool<JetCalibrationTool>()->applyCalibration(**JetShallowCopyItr) == CP::CorrectionCode::Error)
            Error("execute()","Cannot apply shallow-copy jet recalibration: run %i event %i", m_runNumber, m_eventNumber);

      //decorate calibration sf
      ( *JetShallowCopyItr )->auxdata< double >( "jet_CalibSF" ) = static_cast< double >((*JetShallowCopyItr)->pt()/tmp_jpt );

      // apply corrections
      if( IsMC() ){
        //apply JES correction
        if(m_jo->get<bool>(xJobOptions::JET_JES_CORRECTION) && m_npv > 0){
          if( m_cp_tools->AccessTool<JetUncertaintiesTool>()->applyCorrection(**JetShallowCopyItr) == CP::CorrectionCode::Error ){
            Error("execute()","Cannot apply shallow-copy JES correction: run %i event %i", m_runNumber, m_eventNumber);
          }
        }

        //apply JER correction
        if(m_jo->get<bool>(xJobOptions::JET_JER_CORRECTION)){
          if( m_cp_tools->AccessTool<JERSmearingTool>()->applyCorrection(**JetShallowCopyItr) == CP::CorrectionCode::Error ){
            Error("execute()","Cannot apply shallow-copy JER correction: run %i event %i", m_runNumber, m_eventNumber);
          }
        }

      } // end if MC

      //apply jvf
      float jetvf  = ((*JetShallowCopyItr)->auxdata< std::vector<float> >( "JVF" )).at(0);
      ( *JetShallowCopyItr )->auxdata< int >( "jvf_cut" ) =
        static_cast< int >( (std::fabs(jetvf) > m_jo->get<double>(xJobOptions::JET_MIN_ABS_JVF) &&
                             std::fabs((*JetShallowCopyItr)->eta()) < m_jo->get<double>(xJobOptions::JET_JVF_CUT_MAX_ABS_ETA))
                            ? 1:0);

      //apply jet cleaning
      ( *JetShallowCopyItr )->auxdata< int >( "clean" ) =
        static_cast< int >( m_cp_tools->AccessTool<JetCleaningTool>()->accept( **JetShallowCopyItr ) ? 1:0);

      //apply jet cuts
      ( *JetShallowCopyItr )->auxdata< int >( "pt_cut" ) =
        static_cast< int >( (*JetShallowCopyItr)->pt() * MeV2GeV > m_jo->get<double>(xJobOptions::JET_MIN_PT) ? 1:0 );
      ( *JetShallowCopyItr )->auxdata< int >( "eta_cut" ) =
        static_cast< int >( std::fabs( (*JetShallowCopyItr)->eta() ) < m_jo->get<double>(xJobOptions::JET_MAX_ABS_ETA) );

      //DAN: jet-muon overlap
      ( *JetShallowCopyItr )->auxdata< int >( "no_muon_overlap" ) = static_cast< int >( ! (m_tools->JetOverlapsWithGoodMuon( *JetShallowCopyItr,
                                                                                                                         m_MuonContShallowCopy.first,
                                                                                                                         m_jo->get<double>(xJobOptions::MUON_JET_OVERLAP_DR)) ? 1:0) );

      //DAN: jet-electron overlap
      ( *JetShallowCopyItr )->auxdata< int >( "no_ele_overlap" ) = static_cast< int >( ! (m_tools->JetOverlapsWithGoodEle( *JetShallowCopyItr,
                                                                                                                       m_EleContShallowCopy.first,
                                                                                                                       m_jo->get<double>(xJobOptions::ELE_JET_OVERLAP_DR)) ? 1:0) );

      //jet -tau overlap
      ( *JetShallowCopyItr )->auxdata< int >( "no_tau_overlap" ) =
        static_cast< int >( ! (m_tools->JetOverlapsWithGoodTau( *JetShallowCopyItr,
                                                            m_TauJetContShallowCopy.first,
                                                            m_jo->get<double>(xJobOptions::TAU_JET_OVERLAP_DR)) ? 1:0) );
      //define goodness
      if( static_cast< int >( m_tools->Goodness(*JetShallowCopyItr, vJetGoodFlags) ) ){
        ( *JetShallowCopyItr )->auxdata< int >( "good" ) = 1;
        nGoodJets++;
      }else{
        ( *JetShallowCopyItr )->auxdata< int >( "good" ) = 0;
      }

      ijet++;
    }//end of loop over the jet cont shallow copy

    // Link to original jets needed for METRebuilder
    if( ! xAOD::setOriginalObjectLink(*JetCont, *m_JetContShallowCopy.first)) {
      Error("execute()", "Failed to set original jet links -- MET rebuilding cannot proceed.");
      return StatusCode::FAILURE;
    }
    // Save calibrated jets in TStore for METRebuilder : hence must NOT delete them at end
    if( ! m_store->record(m_JetContShallowCopy.first, m_jo->get<std::string>(xJobOptions::MET_JETCOL)) ){
      Error("execute()", "Failed to record %s to TStore. Exiting...", m_jo->get<std::string>(xJobOptions::MET_JETCOL).c_str());
      return EL::StatusCode::FAILURE;
    }
    if( ! m_store->record(m_JetContShallowCopy.second, m_jo->get<std::string>(xJobOptions::MET_JETCOL)+"Aux.") ){
      Error("execute()", "Failed to record %sAux. to TStore. Exiting...", m_jo->get<std::string>(xJobOptions::MET_JETCOL).c_str() );
      return EL::StatusCode::FAILURE;
    }

    //==============================
    // Deep Copy / View Containers ||
    //==============================

    //===========
    // Muons    ||
    //===========
    //select good muons
    m_ViewElemMuonCont->reserve(nGoodMuons);
    for(auto MuonShallowCopyItr = (m_MuonContShallowCopy.first)->begin(); MuonShallowCopyItr != (m_MuonContShallowCopy.first)->end(); MuonShallowCopyItr++) {
      if( (*MuonShallowCopyItr)->auxdata< int >("good") )
        m_ViewElemMuonCont->emplace_back(*MuonShallowCopyItr);
    }
    //pt sort of the "view elements" muon container
    m_tools->PtSort(m_ViewElemMuonCont.get());

    //=============
    // Electrons ||
    //=============
    //select good electrons
    m_ViewElemEleCont->reserve(nGoodElectrons);
    for(auto EleShallowCopyItr = (m_EleContShallowCopy.first)->begin(); EleShallowCopyItr != (m_EleContShallowCopy.first)->end(); EleShallowCopyItr++) {
      if( (*EleShallowCopyItr)->auxdata< int >("good") )
        m_ViewElemEleCont->emplace_back( *EleShallowCopyItr );
    }
    //pt sort of the "view elements" electron container
    m_tools->PtSort(m_ViewElemEleCont.get());

    //===========
    // Taus     ||
    //===========
    //select good taus
    m_ViewElemTauCont->reserve(nGoodTaus);
    for(auto TauJetShallowCopyItr = (m_TauJetContShallowCopy.first)->begin(); TauJetShallowCopyItr != (m_TauJetContShallowCopy.first)->end(); TauJetShallowCopyItr++) {
      if( (*TauJetShallowCopyItr)->auxdata< int >("good") ){
        m_ViewElemTauCont->emplace_back(*TauJetShallowCopyItr); //using emplace_back avoids the extra copy or move operation required when using push_back
        //push_back( std::move(*TauJetShallowCopyItr) ); //move: acts as push_back(T&&)  instead of push_back(const T&),
      }
    }
    //pt sort of the "view elements" jet container
    m_tools->PtSort(m_ViewElemTauCont.get());

    //===========
    // Jets     ||
    //===========
    //select good jets
    m_ViewElemJetCont->reserve(nGoodJets);
    for(auto JetShallowCopyItr = (m_JetContShallowCopy.first)->begin(); JetShallowCopyItr != (m_JetContShallowCopy.first)->end(); JetShallowCopyItr++){
      if( (*JetShallowCopyItr)->auxdata< int >("good") ){
        m_ViewElemJetCont->emplace_back(*JetShallowCopyItr);
      }
    }
    //pt sort of the "view elements" jet container
    m_tools->PtSort(m_ViewElemJetCont.get());

    //---------------
    // MET          |
    //---------------

    if(m_jo->get<bool>(xJobOptions::DO_TSTORE_CHECK)){
      //retrieve containers from the store - cross-check
      //jets
      const xAOD::JetContainer    *tmpJetCont = 0;
      if( ! m_store->retrieve(tmpJetCont, m_jo->get<std::string>(xJobOptions::MET_JETCOL)) ){
        Error("execute()", "Failed to retrieve the %s container. Exiting...", m_jo->get<std::string>(xJobOptions::MET_JETCOL).c_str());
        return EL::StatusCode::FAILURE;
      }
      //taus
      const xAOD::TauJetContainer    *tmpTauCont = 0;
      if( ! m_store->retrieve(tmpTauCont, m_jo->get<std::string>(xJobOptions::MET_TAUCOL)) ){
        Error("execute()", "Failed to retrieve the %s container. Exiting...", m_jo->get<std::string>(xJobOptions::MET_TAUCOL).c_str());
        return EL::StatusCode::FAILURE;
      }
      //muons
      const xAOD::MuonContainer    *tmpMuonCont = 0;
      if( ! m_store->retrieve(tmpMuonCont, m_jo->get<std::string>(xJobOptions::MET_MUONCOL)) ){
        Error("execute()", "Failed to retrieve the %s container. Exiting...", m_jo->get<std::string>(xJobOptions::MET_MUONCOL).c_str());
        return EL::StatusCode::FAILURE;
      }

      //electrons
      const xAOD::ElectronContainer    *tmpEleCont = 0;
      if( ! m_store->retrieve(tmpEleCont, m_jo->get<std::string>(xJobOptions::MET_ELECOL)) ){
        Error("execute()", "Failed to retrieve the %s container. Exiting...", m_jo->get<std::string>(xJobOptions::MET_ELECOL).c_str());
        return EL::StatusCode::FAILURE;
      }

    }

    //call METrebuilder
    if( ! m_cp_tools->AccessTool<met::METRebuilder>()->execute() ){
      Error("execute()", "Failed to execute METRebuilder. Exiting...");
      return EL::StatusCode::FAILURE;
    }

    // Retrieve new MET
    m_MissingETCalibCont = 0;
    if( ! m_store->retrieve(m_MissingETCalibCont, m_jo->get<std::string>(xJobOptions::MET_OUTMETCONT)) ){
      Error("execute()", "Failed to retrieve %s. Exiting...", m_jo->get<std::string>(xJobOptions::MET_OUTMETCONT).c_str());
      return EL::StatusCode::FAILURE;
    }

    const xAOD::MissingET *MET_Calib = 0;
    const xAOD::MissingET *MET_Calib_SoftTerm = 0;
    const xAOD::MissingET *MET_Calib_JetTerm = 0;
    const xAOD::MissingET *MET_Calib_TauTerm = 0;
    const xAOD::MissingET *MET_Calib_MuonTerm = 0;
    const xAOD::MissingET *MET_Calib_EleTerm = 0;

    if( m_MissingETCalibCont ){
      MET_Calib  = (*m_MissingETCalibCont)[m_jo->get<std::string>(xJobOptions::MET_OUTMETTERM)];
      if(!MET_Calib){
        Error("execute()", "Null pointer to %s out MET Calib term. Exiting...", m_jo->get<std::string>(xJobOptions::MET_OUTMETTERM).c_str());
        return EL::StatusCode::FAILURE;
      }

      MET_Calib_SoftTerm = (*m_MissingETCalibCont)[m_jo->get<std::string>(xJobOptions::MET_SOFTTERM)];
      if(!MET_Calib_SoftTerm){
        Error("execute()", "Null pointer to MET Calib %s term. Exiting...", m_jo->get<std::string>(xJobOptions::MET_SOFTTERM).c_str());
        return EL::StatusCode::FAILURE;
      }

      MET_Calib_JetTerm = (*m_MissingETCalibCont)[m_jo->get<std::string>(xJobOptions::MET_JETTERM)];
      if(!MET_Calib_JetTerm){
        Error("execute()", "Null pointer to MET Calib %s term. Exiting...", m_jo->get<std::string>(xJobOptions::MET_JETTERM).c_str());
        return EL::StatusCode::FAILURE;
      }

      MET_Calib_TauTerm = (*m_MissingETCalibCont)[m_jo->get<std::string>(xJobOptions::MET_TAUTERM)];
      if(!MET_Calib_TauTerm){
        Error("execute()", "Null pointer to MET Calib %s term. Exiting...", m_jo->get<std::string>(xJobOptions::MET_TAUTERM).c_str());
        return EL::StatusCode::FAILURE;
      }

      MET_Calib_MuonTerm = (*m_MissingETCalibCont)[m_jo->get<std::string>(xJobOptions::MET_MUONTERM)];
      if(!MET_Calib_MuonTerm){
        Error("execute()", "Null pointer to MET Calib %s term", m_jo->get<std::string>(xJobOptions::MET_MUONTERM).c_str());
        return EL::StatusCode::FAILURE;
      }

      MET_Calib_EleTerm = (*m_MissingETCalibCont)[m_jo->get<std::string>(xJobOptions::MET_ELETERM)];
      if(!MET_Calib_EleTerm){
        Error("execute()", "Null pointer to MET Calib %s term", m_jo->get<std::string>(xJobOptions::MET_ELETERM).c_str());
        return EL::StatusCode::FAILURE;
      }

    }else{
      Error("execute()", "Null pointer to MissingETCalib %s container. Exiting...", m_jo->get<std::string>(xJobOptions::MET_OUTMETCONT).c_str());
      return EL::StatusCode::FAILURE;
    }

    //Fill Trees
    //m_channel->FillBranches( { {m_ViewElemMuonCont.get(), m_ViewElemEleCont.get(), m_ViewElemTauCont.get(), m_ViewElemJetCont.get()}, 2, 3.14} );

    //m_channel->FillBranches( S_t({12,13}, 3, 2.14) );
    m_channel->ResetVariables();

    m_channel->FillBranches( m_ViewElemTauCont.get(), MET_Calib );

    m_channel->FillTree( systName );

    //clear copy VIEW containers:  clearing view container DOES NOT delete contents
    m_ViewElemJetCont->clear();
    m_ViewElemTauCont->clear();
    m_ViewElemMuonCont->clear();
    m_ViewElemEleCont->clear();

    //do not delete copy containers, like
    //delete JetContShallowCopy.first; delete JetContShallowCopy.second;

    // Clear transient store instead
    m_store->clear();

  } // end of systematics loop

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode xMain :: postExecute ()
{
  if(m_eventCounter<1) Info("postExecute()", "call");
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}

//## finalise method - delete pointers, finish run
EL::StatusCode xMain :: finalize ()
{
  Info("finilize()", "call");
  // This method is the mirror image of initialize(), meaning it gets
  // called after the last event has been processed on the worker node
  // and allows you to finish up any objects you created in
  // initialize() before they are written to disk.  This is actually
  // fairly rare, since this happens separately for each worker node.
  // Most of the time you want to do your post-processing on the
  // submission node after all your histogram outputs have been
  // merged.  This is different from histFinalize() in that it only
  // gets called on worker nodes that processed input events.
  m_timer->stop("execute");
  m_timer->start("finalize");

  Info( "finalize()","Finished processing %i events", m_eventCounter);
  Info("finalize()", "Number of clean events = %i", m_numCleanEvents);
  Info("finalize()", "Number of weighted events = %f", m_numWeightedEvents);

  // Write PU-configfiles
  // name of file defined in setupJob()
  if(m_jo->get<bool>(xJobOptions::DO_PILEUP_FILE))
    m_cp_tools->AccessTool<CP::PileupReweightingTool>()->WriteToFile(wk()->getOutputFile("PileUpReweighting"));

  //clear ptrs
  m_tools.reset(); m_tools = nullptr;
  m_cp_tools.reset(); m_cp_tools = nullptr;
  m_store.reset(); m_store = nullptr;

  m_ViewElemJetCont.reset(); m_ViewElemJetCont = nullptr;
  m_ViewElemTauCont.reset(); m_ViewElemTauCont = nullptr;
  m_ViewElemMuonCont.reset(); m_ViewElemMuonCont = nullptr;
  m_ViewElemEleCont.reset(); m_ViewElemEleCont= nullptr;

  m_timer->stop("finalize");
  return EL::StatusCode::SUCCESS;
}

//## Getter MC bool
bool xMain::IsMC() const { return m_isMC; }

//## Getter Data bool
bool xMain::IsData() const { return !m_isMC; }

//##finalise histograms
EL::StatusCode xMain :: histFinalize ()
{
  Info("histFinalize()", "call");
  // This method is the mirror image of histInitialize(), meaning it
  // gets called after the last event has been processed on the worker
  // node and allows you to finish up any objects you created in
  // histInitialize() before they are written to disk.  This is
  // actually fairly rare, since this happens separately for each
  // worker node.  Most of the time you want to do your
  // post-processing on the submission node after all your histogram
  // outputs have been merged.  This is different from finalize() in
  // thait gets called on all worker nodes regardless of whether
  // they processed input events.

  // m_tauSelTool->writeControlHistograms();

  //m_timer->start("histFinalize");
  //m_timer->stop("histFinalize");

  //print out the times
  m_timer->report();

  //last to be deleted/freed
  m_jo.reset(); m_jo = nullptr;
  m_channel.reset(); m_channel = nullptr;
  m_timer.reset(); m_timer = nullptr;

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode xMain :: histInitialize ()
{
  Info("histInitialize()", "call");
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.
  m_timer->start("histInitialize");

  //event information histograms
  if(m_runGrid){

    // m_job_options_file = "xMain/JobOptions/default";
    //re-instantiate this class on grid because its instantiation on local nodes is lost
    m_job_options_file = gSystem->GetFromPipe("echo $ROOTCOREBIN/data/xMain/JobOptions/default");
    m_jo = CxxUtils::make_unique<xJobOptions>(m_job_options_file);
    if( !m_jo->Configure() )
      Error("histInitialize()", "Failed to configure the job options class instance with input file %s",  m_job_options_file.c_str());
  }

  m_timer->stop("histInitialize");

  return EL::StatusCode::SUCCESS;
}

