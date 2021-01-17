FROM levanthanh3005/ns3:v0.2-py3

RUN apt-get install -y docker.io

COPY /ChangeCore/visualizer2/visualizer/core.py /usr/ns-allinone-3.30/ns-3.30/src/visualizer/visualizer/core.py 
COPY /ChangeCore/visualizer2/visualizer/svgitem.py /usr/ns-allinone-3.30/ns-3.30/src/visualizer/visualizer/svgitem.py 
COPY /ChangeCore/visualizer2/visualizer/resource /usr/ns-allinone-3.30/ns-3.30/src/visualizer/visualizer/resource 

COPY /ChangeCore/internet/helper /usr/ns-allinone-3.30/ns-3.30/src/internet/helper
COPY /ChangeCore/internet/model /usr/ns-allinone-3.30/ns-3.30/src/internet/model


COPY /ChangeCore/lteFull/helper /usr/ns-allinone-3.30/ns-3.30/src/lte/helper 
COPY /ChangeCore/lteFull/model /usr/ns-allinone-3.30/ns-3.30/src/lte/model 

COPY /ChangeCore/csma/helper /usr/ns-allinone-3.30/ns-3.30/src/csma/helper 
COPY /ChangeCore/csma/model /usr/ns-allinone-3.30/ns-3.30/src/csma/model 

COPY /ChangeCore/point-to-point/helper /usr/ns-allinone-3.30/ns-3.30/src/point-to-point/helper 
COPY /ChangeCore/point-to-point/model /usr/ns-allinone-3.30/ns-3.30/src/point-to-point/model 

RUN apt-get update
RUN apt-get install sudo -y
RUN apt-get install curl -y

RUN python3 ./waf configure --enable-sudo --enable-example
RUN ./waf
RUN python3 ./waf