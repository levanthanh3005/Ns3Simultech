#!/bin/sh

docker run --net none \
    -d -t --name server-1 \
    levanthanh3005/ns3:change9

docker run --net none \
    -d -t --name server-2 \
    levanthanh3005/ns3:change9

docker run --net none \
    -d -t --name car-1 \
    levanthanh3005/ns3:change9


cd /usr/ns-allinone-3.30/ns-3.30/scratch/test/script
./singleSetup.sh server-1
./singleSetup.sh server-2
./singleSetup.sh car-1
./container.sh server-1 10.81.0.8/16
./container.sh server-2 10.82.0.8/16
./container.sh car-1 11.0.0.8/16
./routing.sh

#ping -D -n -O -i1 -W1 10.81.0.8 => To show timestamp