# Docker file to build Dumux on archlinux base
FROM impmx/dumux-2.12:base-reaktoro
MAINTAINER Edscott Wilson Garcia
# 
RUN pacman -Syy; pacman -Syu --noconfirm
RUN pacman -S --needed --noconfirm --noprogressbar  python2-pip python-setuptools python2-setuptools
RUN mkdir /usr/local/src/aur; useradd aur; chown -R aur /usr/local/src/aur;
USER aur
# download snapshots
RUN cd /usr/local/src/aur; p=`pwd`;\
    src='gccxml-git pygccxml py++ pyinstaller pyinstaller2 python-tabulate'; \
    for a in $src ; do wget "https://aur.archlinux.org/cgit/aur.git/snapshot/$a.tar.gz"; done
# untar
RUN cd /usr/local/src/aur; p=`pwd`;  \
    src='gccxml-git pygccxml py++ pyinstaller pyinstaller2 python-tabulate'; \
    for a in $src ; do  tar -xzf `ls *.gz|grep $a`; done
#build package gccxml-git (serialized)   
RUN cd /usr/local/src/aur; p=`pwd`; rm *.gz; \
    src='gccxml-git'; \
    for a in $src ; do d=`ls |grep $a`; cd $d;\
    makepkg --noconfirm;\
    mv *.xz ../ && cd .. ; done
USER root
#install gccxml-git 
RUN cd /usr/local/src/aur; \
    src='gccxml-git'; \
    for a in $src ; do echo "package=$a"; \
    d=`ls *.xz | grep $a`; \
    pacman -U --noconfirm $d ; done
#build package pygccxml (serialized)   
USER aur
RUN cd /usr/local/src/aur; p=`pwd`; rm *.gz; \
    src='pygccxml'; \
    for a in $src ; do d=`ls |grep $a`; cd $d;\
    makepkg --noconfirm;\
    mv *.xz ../ && cd .. ; done
USER root
#install pygccxml
RUN cd /usr/local/src/aur; \
    src='pygccxml'; \
    for a in $src ; do echo "package=$a"; \
    d=`ls *.xz | grep $a`; \
    pacman -U --noconfirm $d ; done

##build other packages
USER aur
RUN cd /usr/local/src/aur;  \
    src='py++ pyinstaller pyinstaller2 python-tabulate'; \
    for a in $src ; do d=`ls |grep $a`; cd $d;\
    makepkg --noconfirm;\
    mv *.xz ../ && cd .. ; done
USER root
##install
RUN cd /usr/local/src/aur; \
    src='py++ pyinstaller pyinstaller2 python-tabulate'; \
    for a in $src ; do echo "package=$a"; \
    d=`ls *.xz | grep $a`; \
    pacman -U --noconfirm $d ; done


RUN userdel aur
# rebuild reactoro...
RUN cd /usr/local/src/Reaktoro; mkdir build; cd build; cmake ..; make && make install
ADD base-pyreaktoro.dockerfile /usr/src/local/

CMD bash 


