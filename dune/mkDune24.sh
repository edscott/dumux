#!/bin/bash
TARGET=/opt/dune-2.4
cp dune-2.4.opts /usr/local/src/dune
modules='dune-common dune-geometry dune-grid dune-localfunctions dune-istl'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then rm -rf $module; fi; \
        git clone -b releases/2.4 https://gitlab.dune-project.org/core/$module.git $module; \
    done;
modules='dune-foamgrid dune-spgrid'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then rm -rf $module; fi; \
        git clone -b releases/2.4 https://gitlab.dune-project.org/extensions/$module.git $module; \
    done;
# Dune extension modules: (part 2, master branch): 
#RUN modules='dune-alugrid'; \
#    cd /usr/local/src/dune; \
#    for module in $modules; do \
#        if test -d $module; then rm -rf $module; fi; \
#        git clone -b master https://gitlab.dune-project.org/extensions/$module.git $module; \
#    done;
# Configure
# Step by step for build tests:
# OK
p='dune-common'; cd /usr/local/src/dune && dune-common/bin/dunecontrol --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol --only=$p  --opts=dune-2.4.opts make
# OK
p='dune-geometry'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make
# OK
#RUN p='dune-uggrid'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make
# OK
p='dune-grid'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make
# OK
p='dune-localfunctions'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make
# OK
p='dune-istl'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make
# OK
p='dune-spgrid'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make
# releases/2.4 does not compile with gcc7, for releases/.25 we are using branch master from jan-11-2018
# RUN p='dune-alugrid'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make
# not OK make install fails with cannot find foamgrid/foamgridviews.hh
# Patch: touch foamgridviews.hh (empty file now)
 p='dune-foamgrid'; cd /usr/local/src/dune; \
    touch dune-foamgrid/dune/foamgrid/foamgrid/foamgridviews.hh \
    && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; \
    done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make


cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts make install
######################################################################################

 modules='dune-fem'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then rm -rf $module; fi; \
        git clone -b releases/2.4 https://gitlab.dune-project.org/dune-fem/$module.git $module; \
    done;
# Jed Brown workaround hack
cp DuneFemMacros.cmake.dune-fem /usr/local/src/dune/dune-fem/cmake/modules/DuneFemMacros.cmake

 modules='dune-typetree'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then rm -rf $module; fi; \
        git clone -b releases/2.4 https://github.com/dune-project/$module.git $module; \
    done;

 modules='dune-functions'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then rm -rf $module; fi; \
        git clone -b releases/2.4-compatible https://gitlab.dune-project.org/staging/$module.git $module; \
    done;
#hack: Doxystyle
 cp /usr/local/src/dune/dune-common/doc/doxygen/Doxystyle /usr/local/src/dune/dune-fem/doc/doxygen/
 p='dune-fem'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make install

 modules='dune-pdelab'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then rm -rf $module; fi; \
        git clone -b releases/2.4 https://gitlab.dune-project.org/pdelab/$module.git $module; \
    done;

 p='dune-typetree'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make install

 p='dune-functions'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make install

 p='dune-pdelab'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make install

######################################################################################################
 
cd /usr/local/src/dune; \
    git clone -b releases/2.12 https://git.iws.uni-stuttgart.de/dumux-repositories/dumux.git dumux/;
 # compile 
 p='dumux'; \
    cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; \
    do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; \
    done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make \
    && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make install

# Install to /opt
# Some dumux files don't installed right (should be fixed by now, but just in case: rsync)

# and compositionfromfugacities2pncmin.hh not found
#RUN cd /usr/local/src/dune && touch dumux/dumux/material/constraintsolvers/compositionfromfugacities2pncmin.hh
#  "/usr/local/src/dune/dumux/dumux/material/constraintsolvers/computefromreferencephase2pnc.hh".
 
 rsync -av /usr/local/src/dune/dumux/dumux/common/ /opt/dune/include/dumux/common/
# Hacks:
 cp /usr/local/src/dune/dumux/dumux/material/fluidmatrixinteractions/permeabilityrutqvisttsang.hh  /opt/dune/include/dumux/material/fluidmatrixinteractions/


