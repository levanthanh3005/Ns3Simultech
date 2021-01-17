## Guideline to run Ping application

First, run the simulator as common, please refer runSimulatorGeneral.md, after that, we have **Ns3-Main-Command-Tab**

Open another new tab, from now, we call the new tab is **Ns3-Script-Command-Tab**, and then run:

`docker exec -it ns3  bash`

In side the bash, run:

`cd scratch/test/script/`
`./setupPing.sh`

With *Ubuntu*, please run also ./setupUbuntu.sh, not needed for MacOS 

Now, go to directory *Ns3SimultechRelease/code*
Open *ue-controller.cc*
Find this function:

`void UeController::refresh(uint64_t imsi, uint16_t cellId, uint16_t rnti, uint16_t rsrq, uint16_t rsrp)`

Then uncommend the lines: 

```
//If want to prind distance:----
// struct timeval tp;
// gettimeofday(&tp, NULL);
// long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;

// std::cout<<ms/1000<<"."<<ms%1000<<","<<currDistance<<","<<rsrq<<","<<rsrp<<","<<rate<<","<<Simulator::Now ().GetSeconds ()<<std::endl;   
```

The will-be uncommend code lines will enable us to how the current location of UE, RSRP and RSRQ

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

`python3 ./waf --run test --vis --verbose > scratch/test/log/log.txt`

Open another tab called **Ns3-Car-Tab**:

`docker exec -it car-1 bash`

Inside bash, run:
`ping -D -n -O -i1 -W1 10.81.0.8`
it will print the timestamp for ping responses,

With the ping response and log file, we can have all data about the experiment with ping