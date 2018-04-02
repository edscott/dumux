# Docker file to build Dumux on archlinux base
FROM impmx/archlinux
MAINTAINER Edscott Wilson Garcia

############################################
# Configure and compile Dune
# For shortcut, first git clone repositories
# and set to the appropriate branch or tag
############################################
#
RUN mkdir /usr/local/src/dune
# clean start (not necessary with .dockerignore file with "build* CMakeFiles")
# RUN rm -rf `find /usr/local/src/dune -name build-cmake`
COPY dune-2.6.opts /usr/local/src/dune/

# clone dune dependencies
# alugrid releases/2.6 does not compile with gcc7, using master branch
# -D-pthread correction is necessary for bug with openmpi-3.0 configuration
# Dune core modules:
RUN modules='dune-common dune-geometry dune-grid dune-localfunctions dune-istl'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then rm -rf $module; fi; \
        git clone -b master https://gitlab.dune-project.org/core/$module.git $module; \
    done;
# Dune staging modules:
RUN modules='dune-uggrid'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then rm -rf $module; fi; \
        git clone -b master https://gitlab.dune-project.org/staging/$module.git $module; \
    done;
# Dune extension modules:
RUN modules='dune-foamgrid dune-spgrid'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then rm -rf $module; fi; \
        git clone -b master https://gitlab.dune-project.org/extensions/$module.git $module; \
    done;
# Dune extension modules: (part 2, master branch):
RUN modules='dune-alugrid'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then rm -rf $module; fi; \
        git clone -b master https://gitlab.dune-project.org/extensions/$module.git $module; \
    done;
# Configure
# Step by step for build tests:
# OK
RUN p='dune-common'; cd /usr/local/src/dune && dune-common/bin/dunecontrol --opts=dune-2.6.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol --only=$p  --opts=dune-2.6.opts make
# OK
RUN p='dune-geometry'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p make
# OK
RUN p='dune-uggrid'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p make
# OK
RUN p='dune-grid'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p make
# OK
RUN p='dune-localfunctions'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p make
# OK
RUN p='dune-istl'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p make
# OK
RUN p='dune-spgrid'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p make
# releases/2.6 does not compile with gcc7, we are using branch master from jan-11-2018
RUN p='dune-alugrid'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p make
# not OK make install fails with cannot find foamgrid/foamgridviews.hh
# Patch: touch foamgridviews.hh (empty file now)
RUN p='dune-foamgrid'; cd /usr/local/src/dune; \
    touch dune-foamgrid/dune/foamgrid/foamgrid/foamgridviews.hh \
    && dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; \
    done &&  dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p make


# Install all to /opt

RUN cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.6.opts make install
RUN mkdir /home/dune-2.6

WORKDIR /home/dune-2.6
ENV PATH /usr/local/sbin:/usr/local/bin:/usr/bin:/usr/bin/site_perl:/usr/bin/vendor_perl:/usr/bin/core_perl:/opt/dune/bin

# End of impmx/dune-2.6
CMD bash 

