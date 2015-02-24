/*
  An Abstract Base Class
  - defines the structure of the output root files for a given channel
  - manipulates trees
  - stores output

  Z. Zinonos

  zenon@cern.ch

  Feb 2015
*/

#ifndef xChannel_xChannel_H
#define xChannel_xChannel_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <iterator>
#include <algorithm>
#include <memory>
#include <initializer_list>

//implementation for C++14
#include "CxxUtils/make_unique.h"

#include "TTree.h"
#include "TBranch.h"
#include "TFile.h"

#include "AsgTools/AsgTool.h"
#include "PATInterfaces/SystematicRegistry.h"
#include "PATInterfaces/SystematicVariation.h" 

#include <EventLoop/Worker.h> 
#include <EventLoop/OutputStream.h>

#include "xAODBase/IParticleContainer.h"
#include "xAODBase/IParticle.h"

#include <xDefinitions/xDefinitions.h>


class xChannel {

 protected:
  
  typedef std::vector<std::string> TreeVarVec_t;
  typedef std::vector<CP::SystematicSet> SystSetVec_t;
  
  typedef std::map<std::string, double> TreeVarMap_t;
  //  typedef std::map<std::string, double> TreeVarMap_t;
  //typedef std::map<std::string, std::unique_ptr<TTree>> TreeMap_t;
  typedef std::map<std::string, TTree*> TreeMap_t;
  typedef std::vector<std::string> SystNameVec_t;


  xChannel(); // we don't want people to use an empty ctor    //dear compiler, provide a default constructor for this class as well

 public:

  //dtor
  virtual ~xChannel() = default;

  //our custom ctors
  xChannel(const TreeVarMap_t &); 
  xChannel(const TreeVarVec_t &); 
  xChannel(const SystSetVec_t &); 
  xChannel(const TreeVarVec_t &, const SystSetVec_t &); 
  xChannel(const TreeVarMap_t &, const SystSetVec_t &); 

  
  // an overloaded output operator
  friend std::ostream& operator<<(std::ostream& out, const xChannel &channel) {
    auto cmap = channel.GetTreeVarMap();
    std::for_each( cmap.begin(), cmap.end(), 
		   [&out](const std::pair<std::string, double>& p) { //using a C++ Lambda function
		     out << p.first << " : " << p.second << "\n";
		   });
    
    return out;
  }

  /* to be defined
  xChannel(const xChannel& other); //the default copy ctor
  xChannel(xChannel&& other) = default; // the copy ctor overloaded
  xChannel& operator= (const xChannel& other); // the assignment operator overloaded
  */
  //overload the << operator here
  /*std::for_each(map.begin(), map.end(), [](const std::pair<int,int>& p)
		{
		  std::cout << p.first << "," << p.second << "\n";
		});
  */

 private:

  TreeMap_t m_tree_map; //map of trees
  TreeVarMap_t m_tree_var_map; //map of tree variables

  SystNameVec_t m_syst_name_vec; //list of syst names
  SystSetVec_t m_syst_set_vec; //syst set

  //TODO: add this in xTools - maybe
  auto ReplaceSubstr(std::string subject, 
		     const std::string& search, 
		     const std::string& replace) -> decltype(subject);

  void CreateTreeVarMap(const TreeVarVec_t &);
  
  double m_tree_var_reset_value = 0;

  TreeVarMap_t GetTreeVarMap() const { return m_tree_var_map;}
  
 protected:

  // pure virtual functions / empty body
  virtual void SetDefaultVariables() = 0;  //derived classes must have default definitions

  virtual void SetDefaultSystematics() = 0; 

  //virtual final functions can't be overridden
  virtual void SetVariables(const TreeVarVec_t &) final; 

  virtual void SetVariables(const TreeVarMap_t &) final;

  virtual void SetSystematics(const SystSetVec_t &) final;

  virtual void AdjustSystematics() final; 

  virtual void InitializeTrees() final;

  virtual void AddTreesWorkerOutput(EL::Worker *) final;

  virtual void LinkVariables() final;

  virtual void AssociateOutputFile(TFile *) final;

  virtual void SetupTrees(EL::Worker *, TFile *) final;

  virtual void FillVar(const std::string &, const double) final;
  
 public:

  //pure virtual
  //  virtual void FillBranches(...) = 0;

  //final
  virtual void ResetVariables() final;

  virtual void FillTree(const std::string &) final;

};

#endif

