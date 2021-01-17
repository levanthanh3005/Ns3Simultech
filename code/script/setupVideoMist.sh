#!/bin/sh
echo "Start docker"

docker run -d \
    --name server-1 \
    -p 4241:4242 \
    -p 1931:1931 \
    -p 8081:8081 \
    -v /Users/vanthanhle/Desktop/Tools/ns3/ImageTest/VideoConfig/server1:/config \
    -v /Users/vanthanhle/Documents/:/media \
    r0gger/mistserver

docker run -d \
    --name server-2 \
    -p 4242:4242 \
    -p 1932:1932 \
    -p 8082:8081 \
    -v /Users/vanthanhle/Desktop/Tools/ns3/ImageTest/VideoConfig/server2:/config \
    -v /Users/vanthanhle/Documents/:/media \
    r0gger/mistserver

docker run -d \
    -p 5800:5800 \
    --name car-1 \
    --shm-size 2g \
    jlesage/firefox

echo "Wait for docker running"
sleep 5
echo "Setup ip"

cd /usr/ns-allinone-3.30/ns-3.30/scratch/test/script
./singleSetup.sh server-1
./singleSetup.sh server-2
./singleSetup.sh car-1
./container.sh server-1 10.81.0.8/16
./container.sh server-2 10.82.0.8/16
./container.sh car-1 11.0.0.8/16
./routing.sh

echo "Done"