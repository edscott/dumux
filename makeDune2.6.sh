#!/bin/bash
### Get DUNE modules
git clone https://gitlab.dune-project.org/core/dune-common.git
cd dune-common
git checkout releases/2.6
cd ..
git clone https://gitlab.dune-project.org/core/dune-geometry.git
cd dune-geometry
git checkout releases/2.6
cd ..
git clone https://gitlab.dune-project.org/core/dune-grid.git
cd dune-grid
git checkout releases/2.6
cd ..
git clone https://gitlab.dune-project.org/core/dune-istl.git
cd dune-istl
git checkout releases/2.6
cd ..
git clone https://gitlab.dune-project.org/core/dune-localfunctions.git
cd dune-localfunctions
git checkout releases/2.6
cd ..

#wget -q https://git.iws.uni-stuttgart.de/dumux-pub/beck2019a/raw/master/ResolveCompilerPaths.cmake

git clone https://gitlab.dune-project.org/extensions/dune-alugrid
cd dune-alugrid
git checkout releases/2.6
cd ..

# fix this file in dune-pdelab
#mv ResolveCompilerPaths.cmake dune-pdelab/cmake/modules/

# Get Dumux
git clone https://git.iws.uni-stuttgart.de/dumux-repositories/dumux.git
cd dumux
git checkout releases/3.0
cd ..


# # run dunecontrol
#./dune-common/bin/dunecontrol --opts=./dumux/optim.opts all
./dune-common/bin/dunecontrol --opts=./dune2.6.opts all

