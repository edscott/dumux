# Docker file to build Dumux on archlinux base
FROM impmx/dumux-2.12:base
MAINTAINER Edscott Wilson Garcia
# 
RUN pacman -Syy
RUN pacman -S --needed --noconfirm --noprogressbar python2-numpy boost boost-libs 
#
#Reaktoro base:
RUN git clone https://bitbucket.org/reaktoro/reaktoro.git /usr/local/src/Reaktoro
RUN cd /usr/local/src/Reaktoro; mkdir build; cd build; cmake ..; make && make install
ADD base-pyreaktoro.dockerfile /usr/src/local/


CMD bash 


