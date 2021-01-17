#!/bin/bash

## First found this … http://blog.oddbit.com/2014/08/11/four-ways-to-connect-a-docker/
## then found https://docs.docker.com/v1.7/articles/networking/

echo "Routing"

CONTAINERNAME=$1
CONTAINERINDEX=$2

SERVER=$(docker inspect --format '{{ .State.Pid }}' ${CONTAINERNAME})
CAR1=$(docker inspect --format '{{ .State.Pid }}' car-1)


ip netns exec $SERVER route add -net 11.0.0.0 netmask 255.255.0.0 dev eth${CONTAINERINDEX} gw 10.8${CONTAINERINDEX}.0.1

ip netns exec $CAR1 route add -net 10.8${CONTAINERINDEX}.0.0 netmask 255.255.255.0 dev eth1 gw 11.0.0.1
