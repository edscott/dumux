# Docker file to build Dumux on archlinux base
# Build with
# docker build -t impmx/archlinux:20180823 -f archlinux/archlinux.dockerfile archlinux
FROM base/archlinux
MAINTAINER Edscott Wilson Garcia <edscott@imp.mx>
# Update system package database
# If building from behind a proxy, update proxy settings 
# or else comment out lines below
ENV ftp_proxy http://192.168.148.25:8082
ENV http_proxy http://192.168.148.25:8082
ENV https_proxy http://192.168.148.25:8082

# Set global C and CXX flags for compiling 
ENV CFLAGS -fPIC -Wno-deprecated
ENV CXXFLAGS -fPIC -Wno-deprecated
ENV CXX_FLAGS -fPIC -Wno-deprecated

# update package database
RUN echo "updating database and ArchLinux base 20180823"; \
    pacman -Syy; pacman -Syu --noconfirm 

# Barebones gnuplot from source needs cairo terminal

RUN pacman -S --needed --noconfirm --noprogressbar cmake gdb  pkgconfig make \
    fakeroot gcc gcc-fortran gcc-libs autoconf automake lapack vc metis \
    suitesparse openmpi tcsh python2 rsync  wget xterm git cairo pango hdf5 \
    fftw arpack boost python2-numpy swig gtest  netcdf libmatio arpack \
    texlive-core doxygen inkscape python-sphinx freeglut qt4 gnuplot

###########################################
# * Install packages from aur.archlinux.org
###########################################
RUN mkdir /usr/local/src/aur; useradd aur; chown -R aur /usr/local/src/aur;

USER aur
# zoltan requires scotch
# papi is broken today 20180802 (skipping)
# scotch is broken today 20180806 (skipping)
# This as non root user

RUN cd /usr/local/src/aur; \
    p=`pwd`;\
    src='papi superlu psurface parmetis petsc arpackpp scalapack scotch zoltan eigen32'; \
    for a in $src ; do \
        wget "https://aur.archlinux.org/cgit/aur.git/snapshot/$a.tar.gz";\
    done;  \
    for a in $src ; do  \
        tar -xzf `ls *.gz|grep $a`; \
    done;  \
cd /usr/local/src/aur; \
    p=`pwd`; \
    rm *.gz;\
    src='superlu psurface parmetis petsc arpackpp scalapack'; \
    for a in $src ; do \
        d=`ls |grep $a`; \
        cd $d;\
        makepkg --noconfirm;\
        mv *.xz ../ && \
        cd .. ; \
    done;

USER root
RUN userdel aur
#this as root
RUN cd /usr/local/src/aur; \
    src='superlu psurface parmetis petsc arpackpp scalapack '; \
    for a in $src ; do \
        echo "package=$a"; \
        d=`ls *.xz | grep $a`; \
        pacman -U --noconfirm $d ; \
    done;  
 


##############################
# Install directly from source
# Alberta
# http://www.mathematik.uni-stuttgart.de/fak8/ians/
# lehrstuhl/nmh/downloads/alberta/alberta-3.0.1.tar.xz
# sionlib http://apps.fz-juelich.de/jsc/sionlib/download.php?version=1.7.1
# 
##############################
#RUN cd /usr/local/src; \
#    wget \
#    http://www.mathematik.uni-stuttgart.de/fak8/ians/lehrstuhl/nmh/downloads/\
#alberta/alberta-3.0.1.tar.xz; \
#    tar -xJf alberta-3.0.1.tar.xz; \
#    rm /usr/local/src/alberta-3.0.1.tar.xz

#RUN cd /usr/local/src/alberta-3.0.1; \
#    ./configure --prefix=/usr && \
#    make -j6 && \
#    make install

# compile issue:
#RUN cd /usr/local/src; wget -O sionlib.tar.gz 
#http://apps.fz-juelich.de/jsc/sionlib/download.php?version=1.7.1; 
#tar -xzf sionlib.tar.gz

COPY set-user.sh /usr/local/src/
RUN chmod a+rx /usr/local/src/set-user.sh

# End of impmx/archlinux
CMD bash 

