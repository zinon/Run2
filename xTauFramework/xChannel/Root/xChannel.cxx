#include <xChannel/xChannel.h>

xChannel::xChannel()
{
  std::cout<<"xChannel::xChannel 0 "<<std::endl;
  m_syst_name_vec.clear();
  m_syst_set_vec.clear();
  m_tree_map.clear();
  m_tree_var_map.clear();
}

xChannel::xChannel(const TreeVarMap_t &arg) : //using C+11 ctor delegation (calling peer other ctors)
  xChannel()
{
  std::cout<<"xChannel::xChannel 1 "<<std::endl;
  SetVariables(arg);
}

xChannel::xChannel(const TreeVarVec_t &arg) :
  xChannel()
{
  std::cout<<"xChannel::xChannel 2 "<<std::endl;
  CreateTreeVarMap(arg);
}

xChannel::xChannel(const SystSetVec_t &arg) :
  xChannel()
{
  std::cout<<"xChannel::xChannel 3 "<<std::endl;
  SetSystematics(arg);
}

xChannel::xChannel(const TreeVarMap_t &arg1, const SystSetVec_t &arg2) :
  xChannel(arg1)
{
  std::cout<<"xChannel::xChannel 3 "<<std::endl;
  SetSystematics(arg2);
}

xChannel::xChannel(const TreeVarVec_t &arg1, const SystSetVec_t &arg2) :
  xChannel(arg1)
{
  std::cout<<"xChannel::xChannel 4 "<<std::endl;
  SetSystematics(arg2);
}

void xChannel::SetVariables(const TreeVarVec_t &arg)
{
  std::cout<<"xChannel::SetVariables 1 "<<std::endl;
  CreateTreeVarMap(arg);
}

void xChannel::SetVariables(const TreeVarMap_t &arg)
{
  std::cout<<"xChannel::SetVariables 2 "<<std::endl;
  m_tree_var_map = arg;
}

void xChannel::SetSystematics(const SystSetVec_t &arg)
{
  std::cout<<"xChannel::Setsystematics 1 "<<std::endl;
  m_syst_set_vec = arg;
}

void xChannel::CreateTreeVarMap(const TreeVarVec_t &arg_vec)
{
  std::cout<<"xChannel::CreateTreeVarMap 1 "<<std::endl;
  /*  using tree_var_map_type = decltype(arg);
      std::vector<tree_var_map_type::value_type> ini_vec;
  */

  std::vector<double> ini_vec(arg_vec.size(), m_tree_var_reset_value);
  std::transform( arg_vec.begin(), arg_vec.end(), ini_vec.begin(),
                  std::inserter(m_tree_var_map, m_tree_var_map.end()),
                  //              std::make_pair<arg::value_type const&, ini_vec::value_type const &>
                  std::make_pair<std::string const&, double const &>
                  );

}

void xChannel::AdjustSystematics(){
  std::cout<<"xChannel:: adjust systematics "<<std::endl;
  for( auto &syst : m_syst_set_vec ){
    if( syst.name().empty() )
      m_syst_name_vec.push_back( "NOMINAL" );
    else
      m_syst_name_vec.push_back( ReplaceSubstr( syst.name(), "_", "") );
  }

  //  Info("initialize()", "Number of systematic variations %lu", m_sysList.size() );
}

void xChannel::InitializeTrees() {
  std::cout<<"xChannel:: Ini Trees "<<std::endl;
  // tree initialization
  // TObject needs to be created on the heap,  otherwise the worker can not take ownership
  // do no call delete on the pointers
  for( const auto &syst : m_syst_name_vec){
    Info("initialize()", "systematic: %s", syst.c_str());
    m_tree_map.emplace( syst,
                        //CxxUtils::make_unique<TTree>(syst.c_str(), syst.c_str())
			new TTree(syst.c_str(), syst.c_str()) //
                        );
  }

}

void xChannel::AssociateOutputFile(TFile *outfile){
  std::cout<<"xChannel:: Associate out file "<<std::endl;
  for( auto &tree : m_tree_map )
    tree.second->SetDirectory(outfile);

}

void xChannel::LinkVariables() {
  std::cout<<"xChannel:: Link variables "<<std::endl;
  //link local variables to branches
  for( auto &tree : m_tree_map ){
    tree.second->SetMaxTreeSize();
    for( auto &var : m_tree_var_map){
      tree.second->Branch( var.first.c_str(), &var.second, (var.first+"/D").c_str() );
    }
  }

}

void xChannel::AddTreesWorkerOutput(EL::Worker *wk){
  std::cout<<"xChannel:: WK "<<std::endl;
  //add trees to the worker output
  for( const auto &tree : m_tree_map ){
    //wk->addOutput(tree.second.get());
    //wk->addOutput( std::move(tree.second.get()) );
    wk->addOutput( tree.second );
    std::cout<<"xChannel:: WK "<<tree.first<<" added "<<std::endl;
  }
}

void xChannel::SetupTrees(EL::Worker *wk, TFile *outfile){
  // setup the event
  //tree operations executed in the followig logical order - called automatically in ctor
  std::cout<<"xChannel::Setup in Channel"<<std::endl;
  /*1*/ AdjustSystematics();
  /*2*/ InitializeTrees();
  /*3*/ AssociateOutputFile(outfile);
  /*4*/ LinkVariables();
  /*5*/ AddTreesWorkerOutput(wk);
  /*6*/ //further checks?
}

void xChannel::ResetVariables() {

  //reset tree variables
  for( auto &var : m_tree_var_map)
    var.second = m_tree_var_reset_value;
}

void xChannel::FillVar(const std::string &varname, const double varvalue){

  //fill variables
  ASSERT_WITH_MESSAGE(m_tree_var_map.find(varname) != m_tree_var_map.end(), 
		      ("xChannel::FillVar - Cannot find variable '"+varname+"' ").c_str());
  m_tree_var_map[varname] = varvalue;

  /*//fill variables via iteration
  if(m_tree_var_map.find(varname) != m_tree_var_map.end() )
    m_tree_var_map[varname] = varvalue;
  else
    Warning("xChannel::FillVar()", "Cannot find variable named '%s'", varname.c_str());
  */
}

void xChannel::FillTree(const std::string &systName) {

  //fill tree - callable at the end each event
  ASSERT_WITH_MESSAGE(m_tree_map.find(systName) != m_tree_map.end(), 
		      ("xChannel::FillTree - Cannot fill tree with name '"+systName+"' ").c_str());
  m_tree_map[systName]->Fill();


  //fill tree - callable at the end each event
  /*  if ( m_tree_map.find(systName) != m_tree_map.end() )
    m_tree_map[systName]->Fill();
  else
    Error("xChannel::FillTree()", "Cannot fill tree with name '%s'", systName.c_str());
  */
}

auto xChannel::ReplaceSubstr(std::string subject,
                             const std::string& search,
                             const std::string& replace) -> decltype(subject)
{
  //replace substring patterns in a string
  size_t pos = 0;
  while((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }

  return subject;
}

