setupATLAS
localSetupDQ2Client --quiet
voms-proxy-init --voms atlas
localSetupPandaClient currentJedi --noAthenaCheck
rcSetup Base,2.0.24
rc clean
rc find_packages
rc compile
