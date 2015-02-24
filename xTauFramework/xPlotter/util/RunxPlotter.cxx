#include "xPlotter/xPlotter.h"

//#include <memory>
//#include <iostream>
//#include <cstdlib>

#include <initializer_list>

#include "TColor.h"


using namespace std;

int main( int argc, char* argv[] ) {


  if(argc || argv[0]){/*just remove this line when argc & argv are used below*/}

  unique_ptr<xPlotter> p(new xPlotter());

  /* register samples:  type - category - wildcard - number/id - format */

  p->Register("data", "data", "tmpdata/Out_Data/*root*", "periodB", xPlotter::DATA);

  p->Register("signal", "Ztautau", "tmpdata/Out_Ztt/out*root*", 147808);

  p->Register("background", "Wjets", "tmpdata/Out_Wenu/out*167740*root*", 167740);
  p->Register("background", "Wjets", "tmpdata/Out_Wenu/out*167741*root*", 167741);
  p->Register("background", "Wjets", "tmpdata/Out_Wenu/out*167742*root*", 167742);

  p->Register("background", "Wjets", "tmpdata/Out_Wmunu/out*167743*root*", 167743);
  p->Register("background", "Wjets", "tmpdata/Out_Wmunu/out*167744*root*", 167744);
  p->Register("background", "Wjets", "tmpdata/Out_Wmunu/out*167745*root*", 167745);

  p->Register("background", "Wjets", "tmpdata/Out_Wtaunu/out*167746*root*", 167746);
  p->Register("background", "Wjets", "tmpdata/Out_Wtaunu/out*167747*root*", 167747);
  p->Register("background", "Wjets", "tmpdata/Out_Wtaunu/out*167748*root*", 167748);

  p->Register("background", "Zellell", "tmpdata/Out_Zee/out*167749*root*", 167749);
  p->Register("background", "Zellell", "tmpdata/Out_Zee/out*167750*root*", 167750);
  p->Register("background", "Zellell", "tmpdata/Out_Zee/out*167751*root*", 167751);

  p->Register("background", "Zellell", "tmpdata/Out_Zmm/out*167752*root*", 167752);
  p->Register("background", "Zellell", "tmpdata/Out_Zmm/out*167753*root*", 167753);
  p->Register("background", "Zellell", "tmpdata/Out_Zmm/out*167754*root*", 167754);

  p->Register("background", "top", "tmpdata/Out_Stop/out*110101*root*", 110101);
  p->Register("background", "top", "tmpdata/Out_Stop/out*110119*root*", 110119);
  p->Register("background", "top", "tmpdata/Out_Stop/out*110140*root*", 110140);

  p->Register("background", "top", "tmpdata/Out_Ttbar/out*117050*root*", 117050);

  p->Register("background", "QCDjets"); //booked as empty

  p->TemplateMethod("QCDjets", //and filled via the QCD template method
		    "data", // source sample for background estimation
		    {"Wjets", "top", "Zellell", "Ztautau"} // samples to be subtracted in template regions
		    //{}
		    );

  p->StackOrder( {"QCDjets", "Wjets", "top", "Zellell", "Ztautau"} );

  p->StackColors( {kRed, kGreen, kViolet, kOrange, kAzure-2} );

  p->StackNames( {"QCD jets", "W+jets", "t#bar{t}/t", "Z #rightarrow #it{l}#it{l}", "Z #rightarrow #tau #tau"} );


  p->Luminosity(5.093); //fb


  p->Draw();


  cout<<"Done!"<<endl;

  return 0;

}
