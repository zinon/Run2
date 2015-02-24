/*
  xVariables class

  - a handy class to define the TTree structure per channel
  - to be used in conjuction with the xChannel multiclass
  - can define sets of branch variables
  - can merge sets using the overloaded ctor, the copy ctor, the assignment and + operators

  Z. Zinonos - zenon@cern.ch

  Feb 2015

*/

#ifndef xVariables_xVariables_H
#define xVariables_xVariables_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <iterator>
#include <cassert>
#include <algorithm>
#include <iomanip>

#include "AsgTools/AsgTool.h"

class xVariables {

 public:
  //enumerate your set and then give a definition of it 
  // inside the map static member found in th source file
  typedef enum VariablesGroup_t {
    GLOBAL,

    ELECTRON,
    TAU,
    MUON,
    JET,
    MET,

    ELECTRON_ELECTRON,
    MUON_MUON,
    TAU_TAU,
    JET_JET,

    TAU_MUON,
    TAU_ELECTRON,
    TAU_MET,

    DUMMY
    /*
      more like trigger variables, cp angle variables, exotic stuff etc
    */

  } VariablesGroup_t;


  //-------------------------------------------------------------
  // everything below should be in place   ><(((('>
  //-------------------------------------------------------------
  typedef std::vector<std::string> VariablesList_t;
  typedef std::vector<VariablesGroup_t> VariablesGroupList_t;
  typedef std::map<VariablesGroup_t, VariablesList_t> VariablesMap_t;
  typedef std::pair<VariablesGroup_t, VariablesList_t> VariablesPair_t;
  typedef std::initializer_list<VariablesGroup_t> InitializerList_t;

 public:

  //our default dtor
  ~xVariables() = default;

  //the compile will not provide a default constructor for this class as well
  //xVariables() = default; 

  //ctor with no arguments
  xVariables(){
    ConstructVarVector();
  } 

  // our custom ctor 0 - note: pass single arg by value
  xVariables(VariablesGroup_t item)  //providing  unexisting enum params the code will simply not compile!
    try
    : m_groups ({item}) 
  {
    ConstructVarVector();
  }
  catch (std::exception& e)
    {
      std::cerr << "xVariables::xVariables - exception caught on initialization: " << e.what() << '\n';
    }


  // our custom ctor 1 - note: pass a list of variables by reference
  xVariables(InitializerList_t &l) 
    try
    : m_groups(l)
  {
    ConstructVarVector();
  }
  catch (std::exception& e)
    {
      std::cerr << "xVariables::xVariables - exception caught on initialization: " << e.what() << '\n';
    }

  // our custom ctor 2 - used mainly for overloadding the + operator
  xVariables(VariablesGroupList_t g) //note: argument passed by value 
    try
    : m_groups(g)
  {
    ConstructVarVector();
  }
  catch (std::exception& e)
    {
      std::cerr << "xVariables::xVariables - exception caught on initialization: " << e.what() << '\n';
    }

  //overloads
  xVariables(const xVariables& other); //the default copy ctor

  //xVariables(xVariables&& other) = default; // the copy ctor overloaded

  xVariables& operator= (const xVariables& other); // the assignment operator overloaded

  // the plus arithmetic operator overloaded - declared as member function
  xVariables operator+(const xVariables& other){//, const xVariables& other2){  //add xVariables + xVariables
    
    std::cout<< "plus arithmetic operator overloaded "<<std::endl;
  
    xVariables var;
    var.AppendGroupList(this->m_groups);
    var.AppendGroupList(other.m_groups);
    var.ConstructVarVector();
    return var;
  }

  //Compound assignment operators
  //+= (addition-assignment)
  xVariables& operator+=(const xVariables& right) {

    this->AppendGroupList(right.m_groups);
    this->ConstructVarVector();

    return *this;
  }

  // overloading the comparison operators
  //to do

  // a fancy overloaded output operator
  friend std::ostream& operator<<(std::ostream& stream, const xVariables &variables) {
    //variables.write(stream);  //prints out the complete list
    variables.print(stream);  //shows only the selected variables
    return stream;
  }

  //public function used to append new sets of variables to the existing list
  void AppendGroup(const InitializerList_t &l) {
    m_groups.insert(m_groups.end(), l.begin(), l.end());
  }
  
  //public function used to retrieve the selected list of variables
  auto GetVariables() const -> VariablesList_t {
    return m_variables;
  }

 private:

  //private function used to append new sets of variables to the existing list 
  void AppendGroupList(const VariablesGroupList_t &l) {
    std::cout << " appending "<< l.size() << " eleme" << std::endl;
    //m_groups.insert(m_groups.end(), l.begin(), l.end());
    ContainerInsert(m_groups, l);
  }

  //private self-writing function - static map
  std::ostream &write(std::ostream &os) const { 
    // write stuff to stream
    VariablesMap_t cmap = this->GetVariablesMap();

    std::for_each( cmap.begin(), cmap.end(), 
		   // [&os](const std::pair<int, int> &cpair) 
		   [&os](const VariablesPair_t &cpair) //using a C++11 Lambda function
		     {      
		       os << cpair.first << "\n";
		       for (const auto &el : cpair.second)
			 os<<"\t" << el << "\n";
		   } );
    
    return os;
  }

  //private self-writing function - selected vector
  std::ostream &print(std::ostream &os) const { 
    // write stuff to stream
    VariablesList_t cvec = GetVariables();
    unsigned short ivar(0);
    std::for_each( cvec.begin(), cvec.end(), 
		   [&os, &ivar](const std::string &element) //using a C++11 Lambda function
		     {      
		       os << std::left << std::setw(3) << ivar++ << " " << element << "\n";
		     } );
    
    return os;
  }

  //class local members - can get initialized
  VariablesList_t m_variables;
  VariablesGroupList_t m_groups;

  //static members
  static VariablesMap_t m_sVariablesMap;

  //static functions
  static VariablesList_t GetVariablesSet(const VariablesGroup_t &index) {
    assert( m_sVariablesMap.find(index) != m_sVariablesMap.end() );
    return m_sVariablesMap[index];
  }

  static VariablesMap_t GetVariablesMap() {
    return m_sVariablesMap;
  }

  //append container to another existing one
  template <typename T1, typename T2>
    void ContainerInsert(T1 &t1, const T2 &t2)
  {
    t1.reserve( t1.size() + t2.size() );
    t1.insert(t1.end(), t2.begin(), t2.end());
  }

  //create vector of selected variable sets
  void ConstructVarVector(){
    m_variables.clear();

    for(const auto& group : m_groups){
      if( m_sVariablesMap.find(group) != m_sVariablesMap.end() ){ // static member has this defined
	ContainerInsert( m_variables, GetVariablesSet(group) );
      }
      else{
	//Warning("xVariables::ConstructMap - variables group %llu not defined", group);
      }
    }
    
  }

};

//the default copy ctor
xVariables::xVariables(const xVariables& cSource){
  // no need to check for self-assignment in a copy-constructor
  // the copy constructor is only called when new objects are being constructed,
  // so, there is no way to assign a newly created object to itself in a way that calls to copy constructor.

  //copy members
  m_variables = cSource.m_variables;
  m_groups = cSource.m_groups;

}

// the assignment operator = overloaded
xVariables& xVariables::operator= (const xVariables& cSource){
  // check for self-assignment at the top of the overloaded assignment operator
  // by comparing the address of the implicit object and the parameter
  if (this == &cSource)
  return *this;

  // do the copy
  m_variables = cSource.m_variables;
  m_groups = cSource.m_groups;

  // return the existing object
  return *this;
}

// the plus operator + overloaded 
// note: this function is not a member function
//  xVariables operator+ (const xVariables& other){// 1, const xVariables& other2) {

    //use the xVariables constructor and operator+(list, list)
    
//    xVariables var;
//  var.AppendGroup(this->m_groups);
    //    var.m_groups = this->m_groups + other.m_groups;
      
//  return var;
//}




#endif

