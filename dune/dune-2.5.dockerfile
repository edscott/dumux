# Docker file to build Dumux on archlinux base
FROM impmx/archlinux:20180823
MAINTAINER Edscott Wilson Garcia <edscott@imp.mx>
# Hacks for obsolete headers:
RUN  ln -s /usr/include/tirpc/netconfig.h /usr/include/netconfig.h; \
     rm -rf /usr/include/rpc; ln -s /usr/include/tirpc/rpc /usr/include/rpc;

############################################
# Configure and compile Dune
# For shortcut, first git clone repositories
# and set to the appropriate branch or tag
############################################
#
RUN mkdir /usr/local/src/dune
# clean start (not necessary with .dockerignore file with "build* CMakeFiles")
# RUN rm -rf `find /usr/local/src/dune -name build-cmake`
ADD dune-2.5.opts /usr/local/src/dune/

RUN echo "# Dune core modules:"; \
    modules='dune-common dune-geometry dune-grid dune-localfunctions dune-istl'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then \
            rm -rf $module; \
        fi; \
        git clone -b releases/2.5 \
        https://gitlab.dune-project.org/core/$module.git $module; \
    done; \
    echo "# Dune staging modules:"; \
    modules='dune-uggrid'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then \
            rm -rf $module; \
        fi; \
        git clone -b releases/2.5 \
        https://gitlab.dune-project.org/staging/$module.git $module; \
    done; \
    echo "# Dune extension modules: "; \
    modules='dune-alugrid'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then \
            rm -rf $module; \
        fi; \
        git clone -b releases/2.5 \
        https://gitlab.dune-project.org/extensions/$module.git $module; \
    done; \
    echo "# Configure"; \
    echo "# Step by step for build tests:"; \
    p='dune-common'; \
    cd /usr/local/src/dune && \
    dune-common/bin/dunecontrol --opts=dune-2.5.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; do  \
        sed 's/-D-pthread//' <$f >$f.new && \
        mv $f.new $f ; \
    done && dune-common/bin/dunecontrol --only=$p  --opts=dune-2.5.opts make; \
    p='dune-geometry'; \
    cd /usr/local/src/dune && \
    dune-common/bin/dunecontrol  --opts=dune-2.5.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; do  \
        sed 's/-D-pthread//' <$f >$f.new && \
        mv $f.new $f ; \
    done && dune-common/bin/dunecontrol  --opts=dune-2.5.opts --only=$p make; \
    p='dune-uggrid'; \
    cd /usr/local/src/dune && \
    dune-common/bin/dunecontrol  --opts=dune-2.5.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; do  \
        sed 's/-D-pthread//' <$f >$f.new && \
        mv $f.new $f ; \
    done && dune-common/bin/dunecontrol  --opts=dune-2.5.opts --only=$p make; \
    p='dune-grid'; \
    cd /usr/local/src/dune && \
    dune-common/bin/dunecontrol  --opts=dune-2.5.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; do  \
        sed 's/-D-pthread//' <$f >$f.new && \
        mv $f.new $f ; \
    done && dune-common/bin/dunecontrol  --opts=dune-2.5.opts --only=$p make; \
    p='dune-localfunctions'; \
    cd /usr/local/src/dune && \
    dune-common/bin/dunecontrol  --opts=dune-2.5.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; do  \
        sed 's/-D-pthread//' <$f >$f.new && \
        mv $f.new $f ; \
    done && dune-common/bin/dunecontrol  --opts=dune-2.5.opts --only=$p make; \
    p='dune-istl'; \
    cd /usr/local/src/dune && \
    dune-common/bin/dunecontrol  --opts=dune-2.5.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; do  \
        sed 's/-D-pthread//' <$f >$f.new && \
        mv $f.new $f ; \
    done && dune-common/bin/dunecontrol  --opts=dune-2.5.opts --only=$p make; \
    echo "# releases/2.5 does not compile with gcc7, we are using branch master from jan-11-2018"; \
    p='dune-alugrid'; \
    cd /usr/local/src/dune && \
    dune-common/bin/dunecontrol  --opts=dune-2.5.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; do  \
        sed 's/-D-pthread//' <$f >$f.new && \
        mv $f.new $f ; \
    done && dune-common/bin/dunecontrol  --opts=dune-2.5.opts --only=$p make;
RUN cd /usr/local/src/dune && \
    touch dune-geometry/doc/refinement/refinement_safepdf ; \
    touch /usr/local/src/dune/dune-geometry/build-cmake/doc/refinement/refinement.pdf ;\
    dune-common/bin/dunecontrol  --opts=dune-2.5.opts make install

#    echo "# Dune extension modules:"; \
#    modules='dune-foamgrid dune-spgrid'; \
#    cd /usr/local/src/dune; \
#    for module in $modules; do \
#        if test -d $module; then \
#            rm -rf $module; \
#        fi; \
#        git clone -b releases/2.5 \
#        https://gitlab.dune-project.org/extensions/$module.git $module; \
#    done; \
#    echo "# not OK make install fails with cannot find foamgrid/foamgridviews.hh"; \ 
#    echo "# Patch: touch foamgridviews.hh (empty file now)"; \
#    p='dune-foamgrid'; \
#    cd /usr/local/src/dune; \
#    touch dune-foamgrid/dune/foamgrid/foamgrid/foamgridviews.hh && \
#    dune-common/bin/dunecontrol  --opts=dune-2.5.opts --only=$p configure; \
#    files=`find /usr/local/src/dune -name flags.make`; \
#    for f in $files; do  \
#        sed 's/-D-pthread//' <$f >$f.new && \
#        mv $f.new $f ; \
#    done && dune-common/bin/dunecontrol  --opts=dune-2.5.opts --only=$p make; \
#    echo "# Install all to /opt:"; \
#    cd /usr/local/src/dune && dune-common/bin/dunecontrol  \
#    --opts=dune-2.5.opts make install

# dune-spgrid requires dune-grid >= 2.6 but only dune-grid = 2.5.3-git is available.
#    p='dune-spgrid'; \
#    cd /usr/local/src/dune && \
#    dune-common/bin/dunecontrol  --opts=dune-2.5.opts --only=$p configure; \
#    files=`find /usr/local/src/dune -name flags.make`; \
#    for f in $files; do  \
#        sed 's/-D-pthread//' <$f >$f.new && \
#        mv $f.new $f ; \
#    done && dune-common/bin/dunecontrol  --opts=dune-2.5.opts --only=$p make; \

RUN mkdir /home/dune-2.5
WORKDIR /home/dune-2.5
ENV PATH /usr/local/sbin:/usr/local/bin:/usr/bin:/opt/dune/bin:\
/usr/bin/site_perl:/usr/bin/vendor_perl:/usr/bin/core_perl

# End of impmx/dune-2.5
CMD bash 

