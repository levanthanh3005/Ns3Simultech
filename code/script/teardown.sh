#!/bin/sh

cd /usr/ns-allinone-3.30/ns-3.30/scratch/test/script
./singleDestroy.sh server-1
./singleDestroy.sh server-2
./singleDestroy.sh server-3
./singleDestroy.sh car-1

docker rm server-1 server-2 server-3 car-1 -f