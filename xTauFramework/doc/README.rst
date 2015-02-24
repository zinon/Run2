#Quick Demo

Copy-and-paste in a fresh terminal under your /tmp on lxplus :
LATEST_TAG=00-00-06

ATLASINST=svn+ssh://${USER}@svn.cern.ch/reps/atlasinst
GOETAGS=$ATLASINST/Institutes/Goettingen/GoeHiggsAnalysis/tags
svn co $GOETAGS/GoeHiggsAnalysis-${LATEST_TAG}/ GoeHiggsAnalysis-${LATEST_TAG}/
cd GoeHiggsAnalysis-00-00-06/
setupATLAS
rcSetup Base,2.0.23
rc clean
rc find_packages
rc compile
RunMainAnalysis --local-sample-directory /afs/cern.ch/user/z/zenon/public/xAODs --destination testTag



# Quick instructions:

. Check out the latest svn tag
1. setupATLAS
2. rcSetup Base,2.0.X (where X>=18) | e.g. 2.0.24 as for 04.02.2015
3. Create a local directory xAODs/<input dir> in the directory which contains RootCoreBin
3. rc clean
4. rc find_packages
5. rc compile
6. RunMainAnalysis -lsd xAODs/<input dir> -d test11 (picks the default job options files and executes the code)
7. results are stored in the test11/ output directory 

Notes:
If you'd like to do some PileUpReweighting you need to:
  - switch on the JobOption
  - cp -R /afs/cern.ch/work/e/edrechsl/public/share/PileUp MainAnalysis/share/.
