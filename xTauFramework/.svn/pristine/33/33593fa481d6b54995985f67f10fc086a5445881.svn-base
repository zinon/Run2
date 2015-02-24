#ifndef xMain_xMainIncludes_H
#define xMain_xMainIncludes_H

//event info
#include "xAODEventInfo/EventInfo.h"

//============
//STD includes
//============
#include <iostream>
#include <string>
#include <utility> //std::move
#include <memory>
#include <stdio.h> //fseek
#include <ctime>

//============
//STL includes
//============
#include <vector>
#include <map>
#include <iterator>
#include <algorithm>

//implementation for C++14
#include "CxxUtils/make_unique.h"

//============
//EDM includes
//============
//#include "AsgTools/AsgTool.h" needed?
#include "xAODBase/IParticleHelpers.h"

#include "xAODJet/JetContainer.h"
#include "xAODJet/JetAuxContainer.h"
#include "xAODJet/Jet.h"
#include "xAODJet/JetAttributes.h"

#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/ElectronAuxContainer.h"
#include "xAODEgamma/Electron.h"

#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauJetAuxContainer.h"
#include "xAODTau/TauJet.h"

#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/MuonAuxContainer.h"
#include "xAODMuon/Muon.h"

#include "xAODMissingET/MissingETContainer.h"
#include "xAODMissingET/MissingET.h"

#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/VertexContainer.h"

// header for systematics:
#include "PATInterfaces/SystematicRegistry.h"
#include "PATInterfaces/SystematicVariation.h" 
#include "PATInterfaces/SystematicsUtil.h"

//TStore for CP tools
#include "xAODRootAccess/TStore.h"

// shallow copy of containers
#include "xAODCore/ShallowCopy.h"

//handmade classes
#include "xTools/xTools.h"
#include "xCPTools/xCPTools.h"
#include "xJobOptions/xJobOptions.h"
#include "xTimer/xTimer.h"

//definitions
#include "xDefinitions/xDefinitions.h"

//#include "xChannel/xChannel.h"
#include "xChannel/xChannelA.h"

//=====================
// object declarations
//=====================

//EDM containers - deep copies
std::unique_ptr<xAOD::JetContainer> m_ViewElemJetCont;
std::unique_ptr<xAOD::TauJetContainer> m_ViewElemTauCont;
std::unique_ptr<xAOD::MuonContainer> m_ViewElemMuonCont;
std::unique_ptr<xAOD::ElectronContainer> m_ViewElemEleCont;

//EDM containers -shallow copies
std::pair< xAOD::JetContainer*, xAOD::ShallowAuxContainer* > m_JetContShallowCopy;
std::pair< xAOD::TauJetContainer*, xAOD::ShallowAuxContainer* > m_TauJetContShallowCopy;
std::pair< xAOD::MuonContainer*, xAOD::ShallowAuxContainer* > m_MuonContShallowCopy;
std::pair< xAOD::ElectronContainer*, xAOD::ShallowAuxContainer* > m_EleContShallowCopy;

// Containers for rebuilt MET, these get initialized by METRebuilder
const xAOD::MissingETContainer* m_MissingETCalibCont;
const xAOD::MissingETAuxContainer* m_MissingETAuxCalibCont;

// list of systematics
std::vector<CP::SystematicSet> m_sysList;

// Needed for METRebuilder constituent map
SG::AuxElement::Accessor< ElementLink<xAOD::IParticleContainer> > m_objLinkAcc("originalObjectLink");

// Transient object store. Needed for the CP tools and MET
std::unique_ptr<xAOD::TStore> m_store;

//helper class
std::unique_ptr<xTools> m_tools;

//CP service class
std::unique_ptr<xCPTools> m_cp_tools;

//job options class
std::unique_ptr<xJobOptions> m_jo;

//time measurement class
std::unique_ptr<xTimer> m_timer;

//channel class
std::unique_ptr<xChannelA> m_channel;

#endif // xMain_xMainIncludes_H

