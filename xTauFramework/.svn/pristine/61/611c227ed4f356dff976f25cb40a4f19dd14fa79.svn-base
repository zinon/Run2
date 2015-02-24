#ifndef xCPTools_xCPTools_H
#define xCPTools_xCPTools_H

//to-do : add all headers in one big header file

//=============
//STD includes
//=============
#include <iostream>
#include <string>

#include <memory>

//=============
//STL includes
//=============
#include <vector>
#include <iterator>
#include <map>

//============
//ASG includes
//============
#include "AsgTools/AsgTool.h"

// GRL
#include "GoodRunsLists/GoodRunsListSelectionTool.h"

// PileUpReweighting
#include "PileupReweighting/PileupReweightingTool.h"

//Jet Tools
#include "JetSelectorTools/JetCleaningTool.h"
#include "JetResolution/JERTool.h"
#include "JetResolution/JERSmearingTool.h"
#include "JetCalibTools/JetCalibrationTool.h"
#include "JetUncertainties/JetUncertaintiesTool.h"

//TauAnalysisTools
#include "TauAnalysisTools/Enums.h"
#include "TauAnalysisTools/TauSelectionTool.h"
#include "TauAnalysisTools/TauSmearingTool.h"
#include "TauAnalysisTools/TauEfficiencyCorrectionsTool.h"

//MuonTools
#include "MuonMomentumCorrections/MuonCalibrationAndSmearingTool.h"
#include "MuonEfficiencyCorrections/MuonEfficiencyScaleFactors.h"
#include "MuonSelectorTools/MuonSelectionTool.h"

//ElectronTools
#include "ElectronPhotonFourMomentumCorrection/EgammaCalibrationAndSmearingTool.h"
#include "ElectronIsolationSelection/ElectronIsolationSelectionTool.h"

//Systematics tools
#include "PATInterfaces/SystematicRegistry.h"
#include "PATInterfaces/SystematicVariation.h"


//MET Tools
#include "METUtilities/METRebuilder.h"

//implementation for C++14
#include "CxxUtils/make_unique.h"

#include "xJobOptions/xJobOptions.h"

class xCPTools /*: public xJobOptions*/ {

 private:

  // This is a standard constructor
  // We don't want people to create xCPTools without job options
  // So our default constructor is private
  xCPTools ();

 public:

  //Specific ctor
  xCPTools (const xJobOptions *jo)  {

    std::cout<<"in CTOR"<<std::endl;

    m_jo = CxxUtils::make_unique<xJobOptions> (*jo); //aggregation

  }


  virtual ~xCPTools () = default; //auto generated

  bool Setup(/*const xJobOptions *t*/);

  //auto ToolAccess(const std::string &) -> asg::AsgTool *;

  //function to inform CP tools to get updated for systematic variations
  bool ApplySystematicVariation(const CP::SystematicSet &);

  // pointer return functions for tool accessing
  //muons
  template<typename T, typename std::enable_if<std::is_same<T, CP::MuonCalibrationAndSmearingTool>::value, int>::type = 0>
    T * AccessTool()
    {
      return m_MuonCalibrationAndSmearingTool.get();
    }

  template<typename T, typename std::enable_if<std::is_same<T, CP::MuonEfficiencyScaleFactors>::value, int>::type = 0>
    T * AccessTool()
    {
      return m_MuonEfficiencyScaleFactors.get();
    }

  template<typename T, typename std::enable_if<std::is_same<T, CP::MuonSelectionTool>::value, int>::type = 0>
    T * AccessTool()
    {
      return m_MuonSelectionTool.get();
    }
  //electrons

  template<typename T, typename std::enable_if<std::is_same<T, CP::EgammaCalibrationAndSmearingTool>::value, int>::type = 0>
    T * AccessTool()
    {
      return m_EgammaCalibrationAndSmearingTool.get();
    }

  template<typename T, typename std::enable_if<std::is_same<T, CP::ElectronIsolationSelectionTool>::value, int>::type = 0>
    T * AccessTool()
    {
      return m_EleIsoTool.get();
    }

  //jets
  template<typename T, typename std::enable_if<std::is_same<T, JetCleaningTool>::value, int>::type = 0>
    T * AccessTool()
    {
      return m_JetCleaningTool.get();
    }
  template<typename T, typename std::enable_if<std::is_same<T, JERTool>::value, int>::type = 0>
    T * AccessTool()
    {
      return m_JERTool.get();
    }
  template<typename T, typename std::enable_if<std::is_same<T, JERSmearingTool>::value, int>::type = 0>
    T * AccessTool()
    {
      return m_JERSmearingTool.get();
    }
  template<typename T, typename std::enable_if<std::is_same<T, JetCalibrationTool>::value, int>::type = 0>
    T * AccessTool()
    {
      return m_JetCalibrationTool.get();
    }
  template<typename T, typename std::enable_if<std::is_same<T, JetUncertaintiesTool>::value, int>::type = 0>
    T * AccessTool()
    {
      return m_JetUncertaintiesTool.get();
    }

  //taus
  template<typename T, typename std::enable_if<std::is_same<T, TauAnalysisTools::TauSmearingTool>::value, int>::type = 0>
    T * AccessTool()
    {
      return m_TauSmearingTool.get();
    }

  template<typename T, typename std::enable_if<std::is_same<T, TauAnalysisTools::TauSelectionTool>::value, int>::type = 0>
    T * AccessTool()
    {
      return m_TauSelectionTool.get();
    }

  template<typename T, typename std::enable_if<std::is_same<T, TauAnalysisTools::TauEfficiencyCorrectionsTool>::value, int>::type = 0>
    T * AccessTool()
    {
      return m_TauEfficiencyCorrectionsTool.get();
    }

  //met
  template<typename T, typename std::enable_if<std::is_same<T, met::METRebuilder>::value, int>::type = 0>
    T * AccessTool()
    {
      return m_METRebuilder.get();
    }

  //common tools
  template<typename T, typename std::enable_if<std::is_same<T, GoodRunsListSelectionTool>::value, int>::type = 0>
    T * AccessTool()
    {
      return m_GoodRunsListSelectionTool.get();
    }

  template<typename T, typename std::enable_if<std::is_same<T, CP::PileupReweightingTool>::value, int>::type = 0>
    T * AccessTool()
    {
      return m_PileupReweightingTool.get();
    }

 private:

  bool Construct();

  //jet tools
  std::unique_ptr<JetCleaningTool> m_JetCleaningTool;
  std::unique_ptr<JERTool> m_JERTool;
  std::unique_ptr<JERSmearingTool> m_JERSmearingTool;
  std::unique_ptr<JetCalibrationTool> m_JetCalibrationTool;
  std::unique_ptr<JetUncertaintiesTool> m_JetUncertaintiesTool;

  //muon tools
  std::unique_ptr<CP::MuonCalibrationAndSmearingTool> m_MuonCalibrationAndSmearingTool;
  std::unique_ptr<CP::MuonEfficiencyScaleFactors> m_MuonEfficiencyScaleFactors;
  std::unique_ptr<CP::MuonSelectionTool> m_MuonSelectionTool;

  //electrons
  std::unique_ptr<CP::EgammaCalibrationAndSmearingTool> m_EgammaCalibrationAndSmearingTool;
  std::unique_ptr<CP::ElectronIsolationSelectionTool> m_EleIsoTool;

  //tau tools
  std::unique_ptr<TauAnalysisTools::TauSmearingTool> m_TauSmearingTool;
  std::unique_ptr<TauAnalysisTools::TauSelectionTool> m_TauSelectionTool;
  std::unique_ptr<TauAnalysisTools::TauEfficiencyCorrectionsTool> m_TauEfficiencyCorrectionsTool;

  //MET tools
  std::unique_ptr<met::METRebuilder> m_METRebuilder;

  //Common tools
  std::unique_ptr<GoodRunsListSelectionTool> m_GoodRunsListSelectionTool;
  std::unique_ptr<CP::PileupReweightingTool> m_PileupReweightingTool;

  //
  //  std::map<const std::string, std::unique_ptr<asg::AsgTool> > m_cp_registry; {

  std::unique_ptr<xJobOptions> m_jo;

  template<typename T> void func(T t);
};

#endif
