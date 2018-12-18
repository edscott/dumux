# Docker file to build Dumux on archlinux base
FROM impmx/dune-2.6:pdelab
MAINTAINER Edscott Wilson Garcia <edscott@imp.mx>
# Update system package database
# If building from behind a proxy, update proxy settings
# clone and configure dumux repository
RUN cd /usr/local/src/dune; \
    git clone https://git.iws.uni-stuttgart.de/dumux-repositories/dumux.git dumux/; 
#    && cd dumux && git checkout 3.0.0-alpha
# hacks:

 # compile 
RUN p='dumux';  \
    cd /usr/local/src/dune \
    && dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; \
    do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; \
    done &&  dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p make \
    && dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p make install

# Install to /opt
# Some dumux files don't installed right
ENV CXXFLAGS -I/opt/dune/include -std=c++17 -I/usr/local/src/dune/dumux

RUN rsync -av /usr/local/src/dune/dumux/dumux/common/ /opt/dune/include/dumux/common/
RUN mkdir /home/dumux
WORKDIR /home/dumux


# End of impmx/dumux-2.12:base
# Set up permissions for input/output directory
# There is no systemd nor sysvinit on this image...
# CXXFLAGS is also set in set-user
COPY set-user.sh /usr/local/src/
RUN chmod a+rx /usr/local/src/set-user.sh
CMD bash -c /usr/local/src/set-user.sh
#CMD bash 


