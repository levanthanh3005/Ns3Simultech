## Guideline to run Mist Video Streaming application

First, run the simulator as common, please refer runSimulatorGeneral.md, after that, we have **Ns3-Main-Command-Tab**

Open another new tab, from now, we call the new tab is **Ns3-Script-Command-Tab**, and then run:

`docker exec -it ns3  bash`

In folder *code/script*, `./setupVideoMist.sh` is the script to setup the video streaming environment, you have to configure the volume mounting to corespond with your system:

```
docker run -d \
    --name server-1 \
    -p 4241:4242 \
    -p 1931:1931 \
    -p 8081:8081 \
    -v /Users/vanthanhle/Desktop/Tools/ns3/ImageTest/VideoConfig/server1:/config \
    -v /Users/vanthanhle/Documents/:/media \
    r0gger/mistserver
```

In side the bash, run:

`cd scratch/test/script/`
`./setupVideoMist.sh`

With *Ubuntu*, please run also ./setupUbuntu.sh, not needed for MacOS 

Now, go to directory *Ns3SimultechRelease/code*

Open *lena-radio-link-failure-6.cc*, find these paramenters and set as following:

```
sendRequestViaRoaming = true;  
needMigration = false;
useRealDevice = true;
double ueSpeed = 3;
uint16_t ueStartPoint = 1;
bool ckReportUeStatus = false;
```

Now, go to **Ns3-Main-Command-Tab**:

`python3 ./waf --run test --vis --verbose`

Now, lets watch video, open your browser and then access: `localhost:5800`, it will open firefox browser inside docker, in firefox, lets access:
`10.81.0.8/yourvideofile`