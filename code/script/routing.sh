#!/bin/bash

## First found this … http://blog.oddbit.com/2014/08/11/four-ways-to-connect-a-docker/
## then found https://docs.docker.com/v1.7/articles/networking/

echo "Routing"

SERVER1=$(docker inspect --format '{{ .State.Pid }}' server-1)
SERVER2=$(docker inspect --format '{{ .State.Pid }}' server-2)
CAR1=$(docker inspect --format '{{ .State.Pid }}' car-1)


ip netns exec $SERVER1 route add -net 11.0.0.0 netmask 255.255.0.0 dev eth1 gw 10.81.0.1

ip netns exec $SERVER2 route add -net 11.0.0.0 netmask 255.255.0.0 dev eth1 gw 10.82.0.1

ip netns exec $CAR1 route add -net 10.81.0.0 netmask 255.255.255.0 dev eth1 gw 11.0.0.1
ip netns exec $CAR1 route add -net 10.82.0.0 netmask 255.255.255.0 dev eth1 gw 11.0.0.1

#ip netns exec $CAR1 ip route change to default dev eth1 via 11.0.0.1

#ip netns exec $CAR1 route add -net 8.8.8.0 netmask 255.255.255.0 dev eth1 gw 11.0.0.1

#echo "nameserver 8.8.8.8" >> /etc/resolv.conf
#echo "search mydomain.bz" >> /etc/resolv.conf
#echo "domain 8.8.8.8" >> /etc/resolv.conf
