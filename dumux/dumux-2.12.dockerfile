# Docker file to build Dumux on archlinux base
FROM impmx/dune-2.5:pdelab
MAINTAINER Edscott Wilson Garcia
# Update system package database
# If building from behind a proxy, update proxy settings
# clone and configure dumux repository
RUN cd /usr/local/src/dune; \
    git clone -b releases/2.12 \
    https://git.iws.uni-stuttgart.de/dumux-repositories/dumux.git dumux/;

# compile 

RUN p='dumux';  \
    cd /usr/local/src/dune && \
    dune-common/bin/dunecontrol  --opts=dune-2.5.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; do  \
        sed 's/-D-pthread//' <$f >$f.new && \
        mv $f.new $f ; \
    done && \
    dune-common/bin/dunecontrol  --opts=dune-2.5.opts --only=$p make \
    && dune-common/bin/dunecontrol  --opts=dune-2.5.opts --only=$p make install

 
RUN rsync -av /usr/local/src/dune/dumux/dumux/common/ /opt/dune/include/dumux/common/
RUN mkdir /home/dumux
WORKDIR /home/dumux


# Set up permissions for input/output directory
# There is no systemd nor sysvinit on this image...

RUN cp /usr/local/src/dune/dumux/dumux/material/fluidmatrixinteractions/\
permeabilityrutqvisttsang.hh  \
    /opt/dune/include/dumux/material/fluidmatrixinteractions/
CMD bash -c /usr/local/src/set-user.sh
# End of impmx/dumux-2.12:base
#CMD bash 


