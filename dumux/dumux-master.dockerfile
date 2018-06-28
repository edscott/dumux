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
    && touch dumux/dumux/common/timesteppingscheme.hh \
    && touch dumux/dumux/discretization/cellcentered/mpfa/computetransmissibility \
    && touch dumux/dumux/discretization/staggered/connectivitymap.hh \
    && touch dumux/dumux/discretization/staggered/elementfluxvariables.hh \
    && touch dumux/dumux/discretization/staggered/globalfacevariables.hh \
    && touch dumux/dumux/discretization/staggered/gridfluxvariables.hh \
    && touch dumux/dumux/linear/linearsolverproperties.hh \
    && touch dumux/dumux/linear/pardisobackend.hh \
    && touch  dumux/dumux/porousmediumflow/2p/adaptionhelper.hh \
    && touch  dumux/dumux/porousmediumflow/2pnc/newtoncontroller.hh \
    && touch dumux/dumux/porousmediumflow/mpnc/vtkoutputfield.hh \
    && touch dumux/dumux/freeflow/rans/twoeq/komega/models.hh

RUN p='dumux';  \
    cd /usr/local/src/dune \
    && dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p configure; \
    files=`find /usr/local/src/dune -name flags.make`; \
    for f in $files; \
    do  sed 's/-D-pthread//' <$f >$f.new && mv $f.new $f ; \
    done &&  dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p make \
    && dune-common/bin/dunecontrol  --opts=dune-2.6.opts --only=$p make install

# Install to /opt
# Some dumux files don't installed right (should be fixed by now, but just in case: rsync)
 
# Hacks:
RUN rsync -av /usr/local/src/dune/dumux/dumux/common/ /opt/dune/include/dumux/common/
RUN mkdir /home/dumux
WORKDIR /home/dumux


# End of impmx/dumux-2.12:base
# Set up permissions for input/output directory
# There is no systemd nor sysvinit on this image...
CMD bash -c /usr/local/src/set-user.sh
#CMD bash 


