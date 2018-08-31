#!/bin/bash
# The user can pass the user and group id by passing
# --env HOST_UID=\$(id -u \$USER) --env HOST_GID=\$(id -g \$USER)
# with the UID on the host to the container. Should work for Linux, Mac and Windows.
# Allows to manage the writes for shared volumes.
printenv
if  test x$USER = x; then
    echo "USER environment variable is not set. Please run docker image with start_docker script"
    exec bash
else
    useradd -u $HOST_UID  $USER
    export PATH=$PATH:/opt/dune/bin
    export CFLAGS="-I/opt/dune/include"
    export CXXFLAGS="$CFLAGS -std=c++17"
    if test x$ROOT = xyes; then
        su $USER;
    else
        exec su $USER;
    fi
fi


