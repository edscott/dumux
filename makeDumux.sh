#!/bin/bash
DUNE_RELEASE=2.4
DUMUX_RELEASE=2.12
cd /usr/local/src
export CXXFLAGS="$CXXFLAGS -I/usr/include/tirpc"
mkdir dune
    cp dune/dune-2.4.opts /usr/local/src/dune/ && \
echo "# Dune core modules:"; \
    modules='dune-common dune-geometry dune-grid dune-localfunctions dune-istl'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then \
            rm -rf $module; \
        fi; \
        git clone -b releases/$DUNE_RELEASE \
        https://gitlab.dune-project.org/core/$module.git $module; \
    done; \
    echo "# Dune staging modules:"; 
    modules='dune-uggrid'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then \
            rm -rf $module; \
        fi; \
        git clone -b releases/$DUNE_RELEASE \
        https://gitlab.dune-project.org/staging/$module.git $module; \
    done; \
    echo "# Dune extension modules: "; \
    modules='dune-alugrid'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then \
            rm -rf $module; \
        fi; \
        git clone -b releases/$DUNE_RELEASE \
        https://gitlab.dune-project.org/extensions/$module.git $module; \
    done; \
    echo "# Configure"; \
    echo "# Step by step for build tests:"; \
    p='dune-common'; \
    cd /usr/local/src/dune && \
    dune-common/bin/dunecontrol --opts=dune-$DUNE_RELEASE.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; do  \
        sed 's/-D-pthread//' <$f >$f.new && \
        mv $f.new $f ; \
    done && dune-common/bin/dunecontrol --only=$p  --opts=dune-$DUNE_RELEASE.opts make; \
    p='dune-geometry'; \
    cd /usr/local/src/dune && \
    dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; do  \
        sed 's/-D-pthread//' <$f >$f.new && \
        mv $f.new $f ; \
    done && dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p make; 
#    p='dune-uggrid'; \
#    cd /usr/local/src/dune && \
#    dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p configure; \
#    files=`find /usr/local/src/dune -name flags.make`; \
#    for f in $files; do  \
#        sed 's/-D-pthread//' <$f >$f.new && \
#        mv $f.new $f ; \
#    done && dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p make; \
    p='dune-grid'; \
    cd /usr/local/src/dune && \
    dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; do  \
        sed 's/-D-pthread//' <$f >$f.new && \
        mv $f.new $f ; \
    done && dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p make; \
    p='dune-localfunctions'; \
    cd /usr/local/src/dune && \
    dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; do  \
        sed 's/-D-pthread//' <$f >$f.new && \
        mv $f.new $f ; \
    done && dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p make; \
    p='dune-istl'; \
    cd /usr/local/src/dune && \
    dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; do  \
        sed 's/-D-pthread//' <$f >$f.new && \
        mv $f.new $f ; \
    done && dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p make; \
    echo "# releases/$DUNE_RELEASE does not compile with gcc7, we are using branch master from jan-11-2018"; 
#    p='dune-alugrid'; \
#    cd /usr/local/src/dune && \
#    dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p configure; \
#    files=`find /usr/local/src/dune -name flags.make`; \
#    for f in $files; do  \
#        sed 's/-D-pthread//' <$f >$f.new && \
#        mv $f.new $f ; \
#    done && dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p make;

cd /usr/local/src/dune && \
    touch dune-geometry/doc/refinement/refinement_safepdf ; \
    touch /usr/local/src/dune/dune-geometry/build-cmake/doc/refinement/refinement.pdf ;\
    dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts make install
modules='dune-fem'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then \
            rm -rf $module; \
        fi; \
        git clone -b releases/$DUNE_RELEASE \
        https://gitlab.dune-project.org/dune-fem/$module.git $module; \
    done;\
echo "# Jed Brown workaround hack"; 
cp DuneFemMacros.cmake.dune-fem \
    /usr/local/src/dune/dune-fem/cmake/modules/DuneFemMacros.cmake
p='dune-fem'; \
    cd /usr/local/src/dune && \
    dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; do  \
        sed 's/-D-pthread//' <$f >$f.new && \
        mv $f.new $f ; \
    done &&  \
    dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p make && \
    dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p make install
modules='dune-typetree'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then \
            rm -rf $module; \
        fi; \
        git clone -b releases/$DUNE_RELEASE \
        https://github.com/dune-project/$module.git $module; \
    done;
p='dune-typetree'; \
    cd /usr/local/src/dune && \
    dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; do  \
        sed 's/-D-pthread//' <$f >$f.new && \
        mv $f.new $f ; \
    done &&  \
    dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p make && \
    dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p make install
modules='dune-functions'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then \
            rm -rf $module; \
        fi; \
        git clone -b releases/$DUNE_RELEASE \
        https://gitlab.dune-project.org/staging/$module.git $module; \
    done;
modules='dune-pdelab'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then rm -rf $module; fi; \
        git clone -b releases/$DUNE_RELEASE \
        https://gitlab.dune-project.org/pdelab/$module.git $module; \
    done;
p='dune-functions'; \
    cd /usr/local/src/dune && \
    dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; do  \
        sed 's/-D-pthread//' <$f >$f.new && \
        mv $f.new $f ; \
    done && \
    dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p make && \
    dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p make install
cd /usr/local/src/dune; \
    git clone -b releases/$DUMUX_RELEASE \
    https://git.iws.uni-stuttgart.de/dumux-repositories/dumux.git dumux/;
p='dumux';  \
    cd /usr/local/src/dune && \
    dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; do  \
        sed 's/-D-pthread//' <$f >$f.new && \
        mv $f.new $f ; \
    done && \
    dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p make \
    && dune-common/bin/dunecontrol  --opts=dune-$DUNE_RELEASE.opts --only=$p make install
rsync -av /usr/local/src/dune/dumux/dumux/common/ /opt/dune/include/dumux/common/    

