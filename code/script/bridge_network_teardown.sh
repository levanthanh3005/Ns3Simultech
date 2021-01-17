#!/bin/bash
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

echo "tear-down ns3 bridge"

# Take both of the bridges down
ifconfig br-left down
ifconfig br0 down
# ifconfig lxcbr0 down

# Remove the taps from the bridges
brctl delif br-left tap-UE1
brctl delif br-left tap-UE2
brctl delif br-left tap-UE3
brctl delif br-left tap-UE4
brctl delif br-left tap-UE5
brctl delif br-left tap-UE6
brctl delif br-left eno2

brctl delif br0 tapNas
brctl delif br0 eno1

# Destroy the bridges
brctl delbr br-left
brctl delbr br0
# brctl delbr lxcbr0

# Bring down the taps
ifconfig tap-UE1 down
ifconfig tap-UE2 down
ifconfig tap-UE3 down
ifconfig tap-UE4 down
ifconfig tap-UE5 down
ifconfig tap-UE6 down
ifconfig tapNas down
ifconfig eno1 down
ifconfig eno1 up
ifconfig eno2 down
ifconfig eno2 up

# Delete the taps
tunctl -d tap-UE1
tunctl -d tap-UE2
tunctl -d tap-UE3
tunctl -d tap-UE4
tunctl -d tap-UE5
tunctl -d tap-UE6
tunctl -d tapNas

# what's left
echo "ifconfig output"
ifconfig
# echo "lxc-ls output"
# lxc-ls
