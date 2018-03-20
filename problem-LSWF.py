#!/usr/bin/python3
import sys
import os
wd=os.getcwd()
sys.path.insert(0,wd+"/bin")
import project

def lswf():
    available_directives = {"AMG":"Solve with AMG iterations", "UMF":"Solve direct with Umfpack backend", "DIFFUSION":"Use MilliganQuirk molecular diffusion", "ALPHA_DIFFUSION":"Use alpha diffusion (implies DIFFUSION)", "USE_BC":"BrooksCorey", "BCM":"BrooksCorey modified", "BCMV":"Salinity variable BrooksCorey modified non-coupled", "BCMVC":"Salinity variable BrooksCorey modified coupled", "SH2O":"Use simple water", "TH2O":"Use tabulated water", "NTH2O":"Use non-tabulated water"}
#available_directives = ["AMG", "UMF", "DIFFUSION", "ALPHA_DIFFUSION", "USE_BC", "BCM", "BCMV", "BCMVC", "SH2O", "TH2O", "NTH2O"]
    defaults = ["BCMVC", "UMF", "SH2O"]
    argv = sys.argv
    project.main(argv, available_directives, defaults)

lswf()


