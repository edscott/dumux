#!/bin/bash
# Dune release
DUNE_RELEASE=releases/2.7
DUMUX_RELEASE=releases/3.1
DUNE_OPTS=./dune2.7.opts
### Get DUNE modules
git clone https://gitlab.dune-project.org/core/dune-common.git
cd dune-common
git checkout $DUNE_RELEASE
cd ..
git clone https://gitlab.dune-project.org/core/dune-geometry.git
cd dune-geometry
git checkout $DUNE_RELEASE
cd ..
git clone https://gitlab.dune-project.org/core/dune-grid.git
cd dune-grid
git checkout $DUNE_RELEASE
cd ..
git clone https://gitlab.dune-project.org/core/dune-istl.git
cd dune-istl
git checkout $DUNE_RELEASE
cd ..
git clone https://gitlab.dune-project.org/core/dune-localfunctions.git
cd dune-localfunctions
git checkout $DUNE_RELEASE
cd ..

#wget -q https://git.iws.uni-stuttgart.de/dumux-pub/beck2019a/raw/master/ResolveCompilerPaths.cmake

git clone https://gitlab.dune-project.org/extensions/dune-alugrid
cd dune-alugrid
git checkout $DUNE_RELEASE
cd ..

# fix this file in dune-pdelab
#mv ResolveCompilerPaths.cmake dune-pdelab/cmake/modules/

# Get Dumux
git clone https://git.iws.uni-stuttgart.de/dumux-repositories/dumux.git
cd dumux
git checkout $DUMUX_RELEASE
cd ..


# # run dunecontrol
#./dune-common/bin/dunecontrol --opts=./dumux/optim.opts all
./dune-common/bin/dunecontrol --opts=$DUNE_OPTS all

