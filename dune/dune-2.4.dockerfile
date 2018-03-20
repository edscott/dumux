# Docker file to build Dumux on archlinux base
FROM base/archlinux
MAINTAINER Edscott Wilson Garcia
# Update system package database
# If building from behind a proxy, update proxy settings

ENV http_proxy http://192.168.148.25:8082
ENV https_proxy http://192.168.148.25:8082
ENV ftp_proxy http://192.168.148.25:8082
#RUN echo "export http_proxy=$http_proxy" >> /etc/bash.bashrc && echo "export https_proxy=$https_proxy" >> /etc/bash.bashrc && echo "export ftp_proxy=$ftp_proxy" >> /etc/bash.bashrc
# Set C and CXX flags for compiling 
ENV CFLAGS -fPIC -Wno-deprecated
ENV CXXFLAGS -fPIC -Wno-deprecated
ENV CXX_FLAGS -fPIC -Wno-deprecated
#RUN echo "export CFLAGS=$CFLAGS" >> /etc/bash.bashrc && echo "export CXXFLAGS=$CXXFLAGS" >> /etc/bash.bashrc
# update package database
RUN echo "updating database and ArchLinux base"; pacman -Syy; pacman -Syu --noconfirm 
# Copy dune/dumux source from docker host
# (repositories previously set to release branch)
#
######################################################################
# * Install required packages:
# cmake gdb pkgconfig make fakeroot
# * Install build tools:
# gcc gcc-fortran gcc-libs autoconf automake python2 rsync
# * Install optional packages:
#   (not including stuff for documentation generation)
# lapack vc metis suitesparse openmpi tcsh
# (gnuplot-qt package brings in all hell... better compile from source)
#######################################################################
# Barebones gnuplot from source: cairo terminal
RUN pacman -S --needed --noconfirm --noprogressbar cmake gdb pkgconfig make fakeroot gcc gcc-fortran gcc-libs autoconf automake lapack vc metis suitesparse openmpi tcsh  python2 rsync  wget xterm git cairo pango 
# gnuplot (gnuplot-qt package brings in all hell... better compile from source)
#
###########################################
# * Install packages from aur.archlinux.org
# 1. psurface
# 2. parmetis
# 3. superlu
###########################################
RUN mkdir /usr/local/src/aur; useradd aur; chown -R aur /usr/local/src/aur;
USER aur
# download snapshots
RUN cd /usr/local/src/aur; p=`pwd`;\
    src='superlu psurface parmetis'; \
    for a in $src ; do wget "https://aur.archlinux.org/cgit/aur.git/snapshot/$a.tar.gz"; done
# untar
RUN cd /usr/local/src/aur; p=`pwd`;  \
    src='superlu psurface parmetis'; \
    for a in $src ; do  tar -xzf `ls *.gz|grep $a`; done
#build package    
RUN cd /usr/local/src/aur; p=`pwd`; rm *.gz; \
    src='superlu psurface parmetis'; \
    for a in $src ; do d=`ls |grep $a`; cd $d;\
    makepkg --noconfirm;\
    mv *.xz ../ && cd .. ; done
USER root
RUN userdel aur
#install
RUN cd /usr/local/src/aur; \
    src='superlu psurface parmetis'; \
    for a in $src ; do echo "package=$a"; \
    d=`ls *.xz | grep $a`; \
    pacman -U --noconfirm $d ; done



#https://aur.archlinux.org/cgit/aur.git/snapshot/psurface.tar.gz
#ENV AUR 'psurface parmetis superlu'
# Add necessary scripts:


#ADD makepackage /usr/local/src/
#RUN chmod a+rx /usr/local/src/makepackage 

#ADD AUR/psurface /home/psurface
#ADD AUR/parmetis /home/parmetis
#ADD AUR/superlu /home/superlu 
#  
#RUN a='superlu psurface parmetis'; for p in $a ; do useradd $p; chown -R $p /home/$p; runuser -l $p '/usr/local/src/makepackage'; pacman -U --noconfirm `ls /home/$p/*xz`; rm -rf /home/$p; done
##############################
##############################
# Install directly from source
# 1. PETSc
# 2. Alberta (pending)
# 3. arpackpp (pending)
##############################
#ADD src/petsc-lite-3.8.3.tar.gz /usr/local/src/
#FIXME:  dune-fem problem with petsc...
#RUN cd /usr/local/src; wget "http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-lite-3.8.3.tar.gz"; tar -xsf petsc-lite-3.8.3.tar.gz; cd petsc-3.8.3; python2 ./configure CXXFLAGS=$CXXFLAGS --prefix=/usr && make install
#Install gnuplot from source. Version 5.2
# This no longer downloads correctly...
#RUN cd /usr/local/src; wget "https://sourceforge.net/projects/gnuplot/files/gnuplot/5.2.2/gnuplot-5.2.2.tar.gz/download"; mv download gnuplot-5.2.2.tar.gz; tar -xzf gnuplot-5.2.2.tar.gz; cd gnuplot-5.2.2; ./configure --with-qt=no ; make; make install;
COPY gnuplot-5.2.2.tar.gz /usr/local/src/
RUN cd /usr/local/src; tar -xzf gnuplot-5.2.2.tar.gz; cd gnuplot-5.2.2; ./configure --with-qt=no ; make; make install;
RUN rm /usr/local/src/*.gz

############################################
# Configure and compile Dune
# For shortcut, first git clone repositories
# and set to the appropriate branch or tag
############################################
#
RUN mkdir /usr/local/src/dune
# clean start (not necessary with .dockerignore file with "build* CMakeFiles")
# RUN rm -rf `find /usr/local/src/dune -name build-cmake`
COPY dune-2.4.opts /usr/local/src/dune/

# clone dune dependencies
# alugrid releases/2.4 does not compile with gcc7, using master branch
# -D-pthread correction is necessary for bug with openmpi-3.0 configuration
# Dune core modules:
RUN modules='dune-common dune-geometry dune-grid dune-localfunctions dune-istl'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then rm -rf $module; fi; \
        git clone -b releases/2.4 https://gitlab.dune-project.org/core/$module.git $module; \
    done;
# Dune staging modules: requires releases/2.5
#RUN modules='dune-uggrid'; \
#    cd /usr/local/src/dune; \
#    for module in $modules; do \
#        if test -d $module; then rm -rf $module; fi; \
#        git clone -b releases/2.4 https://gitlab.dune-project.org/staging/$module.git $module; \
#    done;
# Dune extension modules:
RUN modules='dune-foamgrid dune-spgrid'; \
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
RUN p='dune-common'; cd /usr/local/src/dune && dune-common/bin/dunecontrol --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol --only=$p  --opts=dune-2.4.opts make
# OK
RUN p='dune-geometry'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make
# OK
#RUN p='dune-uggrid'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make
# OK
RUN p='dune-grid'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make
# OK
RUN p='dune-localfunctions'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make
# OK
RUN p='dune-istl'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make
# OK
RUN p='dune-spgrid'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make
# releases/2.4 does not compile with gcc7, for releases/.25 we are using branch master from jan-11-2018
# RUN p='dune-alugrid'; cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; files=`find /usr/local/src/dune -name flags.make`; for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make
# not OK make install fails with cannot find foamgrid/foamgridviews.hh
# Patch: touch foamgridviews.hh (empty file now)
RUN p='dune-foamgrid'; cd /usr/local/src/dune; \
    touch dune-foamgrid/dune/foamgrid/foamgrid/foamgridviews.hh \
    && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; \
    done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make


# Install all to /opt

RUN cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts make install
RUN mkdir /home/dune-2.4
WORKDIR /home/dune-2.4
COPY set-user.sh /usr/local/src/
RUN chmod a+rx /usr/local/src/set-user.sh
# End of impmx/dune-2.4
CMD bash 

