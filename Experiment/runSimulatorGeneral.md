# Guideline to run Ns3 simulator with PyViz

open terminal and then run these commands in each OS in the location of Ns3 code:

## MacOS

`open -a XQuartz`
`ip=$(ifconfig en0 | grep inet | awk '$1=="inet" {print $2}')`
`xhost + $ip`
```
docker run -it --rm \
    -w /usr/ns-allinone-3.30/ns-3.30 \
    -v $(pwd)/code:/usr/ns-allinone-3.30/ns-3.30/scratch/test:rw \
    -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
    -e DISPLAY=$(ifconfig en0 | grep inet | awk '$1=="inet" {print $2}'):0 \
    -v /var/run/docker.sock:/var/run/docker.sock \
    --device /dev/dri \
    --privileged \
    --cap-add=NET_ADMIN \
    --device /dev/net/tun:/dev/net/tun \
    --net host \
    --name ns3 \
    --pid=host \
    levanthanh3005/ns3:change9`
```
## Ubuntu

`xhost +`
```
sudo docker run -it --rm \
    -w /usr/ns-allinone-3.30/ns-3.30 \
    -v $(pwd)/code:/usr/ns-allinone-3.30/ns-3.30/scratch/test:rw \
    -e DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -v /var/run/docker.sock:/var/run/docker.sock \
    --device /dev/dri \
    --privileged \
    --cap-add=NET_ADMIN \
    --cap-add=SYS_ADMIN \
    --device /dev/net/tun:/dev/net/tun \
    --net host \
    --name ns3 \
    --pid=host \
    levanthanh3005/ns3:change9
```
From now, we call the tab that run the command is **Ns3-Main-Command-Tab**