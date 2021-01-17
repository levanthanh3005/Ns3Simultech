#!/bin/sh

export WORKDIR=$(pwd)
cd /proc/sys/net/bridge
for f in bridge-nf-*; do echo 0 > $f; done

cd ${WORKDIR}

#Ref:https://github.com/chepeftw/NS3DockerEmulator/blob/master/net/setup.sh