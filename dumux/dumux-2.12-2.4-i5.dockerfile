# Docker file to build Dumux on archlinux base
FROM impmx/dune-2.4:pdelab-i5
MAINTAINER Edscott Wilson Garcia (IMP)
# Update system package database
# If building from behind a proxy, update proxy settings
# clone and configure dumux repository
RUN cd /usr/local/src/dune; \
    git clone -b releases/2.12 https://git.iws.uni-stuttgart.de/dumux-repositories/dumux.git dumux/;
 # compile 
RUN p='dumux'; \
    cd /usr/local/src/dune && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; \
    do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; \
    done &&  dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make \
    && dune-common/bin/dunecontrol  --opts=dune-2.4.opts --only=$p make install

# Install to /opt
# Some dumux files don't installed right (should be fixed by now, but just in case: rsync)

# and compositionfromfugacities2pncmin.hh not found
#RUN cd /usr/local/src/dune && touch dumux/dumux/material/constraintsolvers/compositionfromfugacities2pncmin.hh
#  "/usr/local/src/dune/dumux/dumux/material/constraintsolvers/computefromreferencephase2pnc.hh".
 
RUN rsync -av /usr/local/src/dune/dumux/dumux/common/ /opt/dune/include/dumux/common/
RUN mkdir /home/dumux
WORKDIR /home/dumux
# Hacks:
RUN cp /usr/local/src/dune/dumux/dumux/material/fluidmatrixinteractions/permeabilityrutqvisttsang.hh  /opt/dune/include/dumux/material/fluidmatrixinteractions/


# End of impmx/dumux-2.12:base
# Set up permissions for input/output directory
# There is no systemd nor sysvinit on this image...
CMD bash -c /usr/local/src/set-user.sh
#CMD bash 


