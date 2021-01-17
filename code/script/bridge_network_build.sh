#!/bin/bash
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# very important that all files in "/proc/sys/net/bridge/" are set to 0, otherwise packet forwardig will not work across the bridge :(

# updated with new client sub-net - 18.1.0.0
# updated with new code to reset bridge

echo "build ns3 bridge"

#  *********** create the bridges ***********
brctl addbr br-left
brctl addbr br0
# brctl stp br0 off
# brctl setfd br0 0
#brctl setpathcost br0 8002 4

#  *********** create tap devices ***********
tunctl -t tap-UE1
tunctl -t tap-UE2
tunctl -t tap-UE3
tunctl -t tap-UE4
tunctl -t tap-UE5
tunctl -t tap-UE6
tunctl -t tapNas

#ifconfig tapNas hw ether 08:00:00:00:00:07

#  *********** release IP from the real interfaces ***********
ifconfig eno1 down
ifconfig eno2 down


# ********** create tap bridges ***********
brctl addif br0 tapNas
brctl addif br0 eno1

brctl addif br-left tap-UE1
brctl addif br-left tap-UE2
brctl addif br-left tap-UE3
brctl addif br-left tap-UE4
brctl addif br-left tap-UE5
brctl addif br-left tap-UE6
brctl addif br-left eno2


#  *********** set ip ***********
ifconfig tap-UE1 0.0.0.0 up
ifconfig tap-UE2 0.0.0.0 up
ifconfig tap-UE3 0.0.0.0 up
ifconfig tap-UE4 0.0.0.0 up
ifconfig tap-UE5 0.0.0.0 up
ifconfig tap-UE6 0.0.0.0 up
ifconfig eno2 0.0.0.0 up
# ifconfig br-left 12.0.0.120 up
ifconfig br-left 0.0.0.0 up

ifconfig tapNas 0.0.0.0 up
ifconfig eno1 0.0.0.0 up
ifconfig br0 11.0.0.120 netmask 255.255.255.0 up
ifconfig br-left 18.2.0.120 netmask 255.255.255.0 up
# ifconfig br0 11.0.0.120 up
# ifconfig br0 0.0.0.0 up


#  *********** update forwarding ***********
for file in br0 br-left eno1 eno2 tapNas tap-UE1 tap-UE2 tap-UE3 tap-UE4 tap-UE5 tap-UE6;
do
   echo "1" > /proc/sys/net/ipv4/conf/${file}/proxy_arp;
   echo "1" > /proc/sys/net/ipv4/conf/${file}/forwarding;
done;

# this seems to be needed for the beast -do not need this for other machines
for f in /proc/sys/net/bridge/bridge-nf-*; 
do 
    echo 0 > $f;
done


#  *********** show some details ***********
brctl showmacs br0
brctl showmacs br-left

#brctl showstp br0

# show bridges
brctl show

echo "ifconfig output"
ifconfig

