# Docker file to build Dumux on archlinux base
FROM impmx/dune-2.4:i5
MAINTAINER Edscott Wilson Garcia
# Update system package database
# If building from behind a proxy, update proxy settings

ENV http_proxy http://192.168.148.25:8082
ENV https_proxy http://192.168.148.25:8082
ENV ftp_proxy http://192.168.148.25:8082
RUN echo "export http_proxy=$http_proxy" >> /etc/bash.bashrc && echo "export https_proxy=$https_proxy" >> /etc/bash.bashrc && echo "export ftp_proxy=$ftp_proxy" >> /etc/bash.bashrc
# Set C and CXX flags for compiling 
ENV CFLAGS -fPIC -Wno-deprecated
ENV CXXFLAGS -fPIC -Wno-deprecated
ENV CXX_FLAGS -fPIC -Wno-deprecated
#RUN pacman -S --needed --noconfirm --noprogressbar doxygen 
# PDElab
RUN modules='dune-fem'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then rm -rf $module; fi; \
        git clone -b releases/2.4 https://gitlab.dune-project.org/dune-fem/$module.git $module; \
    done;
# Jed Brown workaround hack
COPY DuneFemMacros.cmake.dune-fem /usr/local/src/dune/dune-fem/cmake/modules/DuneFemMacros.cmake

RUN modules='dune-typetree'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then rm -rf $module; fi; \
        git clone -b releases/2.4 https://github.com/dune-project/$module.git $module; \
    done;

RUN modules='dune-functions'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then rm -rf $module; fi; \
        git clone -b releases/2.4-compatible https://gitlab.dune-project.org/staging/$module.git $module; \
    done;
#hack: Doxystyle
RUN cp /usr/local/src/dune/dune-common/doc/doxygen/Doxystyle /usr/local/src/dune/dune-fem/doc/doxygen/
RUN p='dune-fem'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make install

RUN modules='dune-pdelab'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then rm -rf $module; fi; \
        git clone -b releases/2.4 https://gitlab.dune-project.org/pdelab/$module.git $module; \
    done;

RUN p='dune-typetree'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make install

RUN p='dune-functions'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make install

RUN p='dune-pdelab'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make install

WORKDIR /home/dune-2.4
CMD bash
