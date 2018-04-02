# Docker file to build Dumux on archlinux base
FROM base/archlinux
MAINTAINER Edscott Wilson Garcia
# Update system package database
# If building from behind a proxy, update proxy settings or else comment out lines below
ENV ftp_proxy http://192.168.148.25:8082
ENV http_proxy http://192.168.148.25:8082
ENV https_proxy http://192.168.148.25:8082

# Set global C and CXX flags for compiling 
ENV CFLAGS -fPIC -Wno-deprecated
ENV CXXFLAGS -fPIC -Wno-deprecated
ENV CXX_FLAGS -fPIC -Wno-deprecated

# update package database
RUN echo "updating database and ArchLinux base 28-03-2018"; pacman -Syy; pacman -Syu --noconfirm 

# Barebones gnuplot from source needs cairo terminal

RUN pacman -S --needed --noconfirm --noprogressbar cmake gdb pkgconfig make fakeroot gcc gcc-fortran gcc-libs autoconf automake lapack vc metis suitesparse openmpi tcsh  python2 rsync  wget xterm git cairo pango hdf5 fftw arpack boost python2-numpy swig gtest  netcdf libmatio arpack texlive-core doxygen inkscape python-sphinx freeglut qt4 gnuplot

# gnuplot (gnuplot-qt package brings in all hell... better compile from source)
###########################################
# * Install packages from aur.archlinux.org
###########################################
RUN mkdir /usr/local/src/aur; useradd aur; chown -R aur /usr/local/src/aur;
USER aur
# download snapshots Block 1
# untar
#build package    
RUN cd /usr/local/src/aur; p=`pwd`;\
    src='papi superlu psurface parmetis petsc arpackpp scalapack scotch'; \
    for a in $src ; do wget "https://aur.archlinux.org/cgit/aur.git/snapshot/$a.tar.gz";\
    done;  \
    for a in $src ; do  tar -xzf `ls *.gz|grep $a`; \
    done; 

RUN cd /usr/local/src/aur; p=`pwd`; rm *.gz;\
    src='papi superlu psurface parmetis petsc arpackpp scalapack scotch'; \
    for a in $src ; do d=`ls |grep $a`; cd $d;\
    makepkg --noconfirm;\
    mv *.xz ../ && cd .. ; \
    done;

USER root
#install
RUN cd /usr/local/src/aur; \
    src='papi superlu psurface parmetis petsc arpackpp scalapack scotch'; \
    for a in $src ; do echo "package=$a"; \
    d=`ls *.xz | grep $a`; \
    pacman -U --noconfirm $d ; \
    done;

USER aur
# download snapshots Block 2
RUN cd /usr/local/src/aur; p=`pwd`;\
    src='zoltan eigen32'; \
    for a in $src ; do wget "https://aur.archlinux.org/cgit/aur.git/snapshot/$a.tar.gz"; \
    done;

# untar
RUN cd /usr/local/src/aur; p=`pwd`;  \
    src='zoltan eigen32'; \
    for a in $src ; do  tar -xzf `ls *.gz|grep $a`; \
    done;

#build package    
RUN cd /usr/local/src/aur; p=`pwd`; rm *.gz; \
    src='zoltan eigen32'; \
    for a in $src ; do d=`ls |grep $a`; cd $d;\
    makepkg --noconfirm;\
    mv *.xz ../ && cd .. ; \
    done;

USER root
#install
RUN cd /usr/local/src/aur; \
    src='zoltan eigen32'; \
    for a in $src ; do echo "package=$a"; \
    d=`ls *.xz | grep $a`; \
    pacman -U --noconfirm $d ; done
#  pastix is pending.
USER root
RUN userdel aur

##############################
# Install directly from source
# Alberta http://www.mathematik.uni-stuttgart.de/fak8/ians/lehrstuhl/nmh/downloads/alberta/alberta-3.0.1.tar.xz
# sionlib http://apps.fz-juelich.de/jsc/sionlib/download.php?version=1.7.1
# 
##############################
RUN cd /usr/local/src; wget http://www.mathematik.uni-stuttgart.de/fak8/ians/lehrstuhl/nmh/downloads/alberta/alberta-3.0.1.tar.xz; tar -xJf alberta-3.0.1.tar.xz; rm /usr/local/src/alberta-3.0.1.tar.xz
RUN cd /usr/local/src/alberta-3.0.1; ./configure --prefix=/usr && make -j6 && make install
# compile issue:
#RUN cd /usr/local/src; wget -O sionlib.tar.gz http://apps.fz-juelich.de/jsc/sionlib/download.php?version=1.7.1; tar -xzf sionlib.tar.gz

#Install gnuplot from source. Version 5.2
# This no longer downloads correctly...
#RUN cd /usr/local/src; wget "https://sourceforge.net/projects/gnuplot/files/gnuplot/5.2.2/gnuplot-5.2.2.tar.gz/download"; mv download gnuplot-5.2.2.tar.gz; tar -xzf gnuplot-5.2.2.tar.gz; cd gnuplot-5.2.2; ./configure --with-qt=no ; make; make install;
#COPY gnuplot-5.2.2.tar.gz /usr/local/src/
#RUN cd /usr/local/src; tar -xzf gnuplot-5.2.2.tar.gz; cd gnuplot-5.2.2; ./configure --with-qt=no ; make; make install; rm /usr/local/src/gnuplot-5.2.2.tar.gz

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
        git clone -b releases/2.6 https://gitlab.dune-project.org/core/$module.git $module; \
    done;
# Dune staging modules:
RUN modules='dune-uggrid'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then rm -rf $module; fi; \
        git clone -b releases/2.6 https://gitlab.dune-project.org/staging/$module.git $module; \
    done;
# Dune extension modules:
RUN modules='dune-foamgrid dune-spgrid'; \
    cd /usr/local/src/dune; \
    for module in $modules; do \
        if test -d $module; then rm -rf $module; fi; \
        git clone -b releases/2.6 https://gitlab.dune-project.org/extensions/$module.git $module; \
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
COPY set-user.sh /usr/local/src/
RUN chmod a+rx /usr/local/src/set-user.sh
ENV PATH /usr/local/sbin:/usr/local/bin:/usr/bin:/usr/bin/site_perl:/usr/bin/vendor_perl:/usr/bin/core_perl:/opt/dune/bin

# End of impmx/dune-2.6
CMD bash 

