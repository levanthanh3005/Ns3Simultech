#!/bin/bash

cd /usr/ns-allinone-3.30/ns-3.30/scratch/test/script

NumOfServer=9

for (( i=1; i<=$NumOfServer; i++ ))
do
./singleDestroy.sh server-$i
done

./singleDestroy.sh car-1

docker rm server-1 car-1 -f