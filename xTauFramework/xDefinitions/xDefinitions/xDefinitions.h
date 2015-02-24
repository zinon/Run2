#ifndef xDefinitions_xDefinitions_H
#define xDefinitions_xDefinitions_H

#define MeV2GeV 1e-3
#define GeV2MeV 1e+3

#define ASSERT_WITH_MESSAGE(condition, message) do {	\
    if (!(condition)) { printf((message)); }		\
    assert ((condition));				\
  } while(false)


#endif
