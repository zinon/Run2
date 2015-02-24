#include "xVariables/xVariables.h"

/*
  ZZ: the C++11 standard supports uniform initialization which makes life much simpler
  @ insert your set of variables with the appropriate enumerated parameter
  @ try to avoid duplications
  @ when indroducing a new set of variables, remember to attach a comma at the end of the previous closing curly bracket
*/

xVariables::VariablesMap_t xVariables::m_sVariablesMap = {

  {ELECTRON, 
   {"electron_0",
    "electron_0_pt",
    "electron_0_eta",
    "electron_0_phi"
   }
  },
  
  
  {TAU, 
   {"tau_n",
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
    "tau_0_JetBDT_eff_sf"
   }
  },

  {TAU_MET,
   {"tau_0_met_mt"
   }
  }/*,*/
  
};




