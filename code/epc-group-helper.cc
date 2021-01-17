#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"

#include "ns3/point-to-point-module.h"

#include "ns3/config-store-module.h"

#include "ns3/netanim-module.h"

#include "point-to-point-epc-helper-ext.h"

#include "ns3/internet-apps-module.h"

#include "epc-group-helper.h"

// #include "ns3/tap-bridge-module.h"

// #include "ns3/csma-module.h"
#include "ns3/flow-monitor-module.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EpcGroupHelper");

NS_OBJECT_ENSURE_REGISTERED (EpcGroupHelper);


TypeId
EpcGroupHelper::GetTypeId (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  static TypeId tid = TypeId ("ns3::EpcGroupHelper")
    .SetParent<Object> ()
    .SetGroupName ("EpcGroup")
    .AddTraceSource ("UeConnect",
                 "add description here",
                 MakeTraceSourceAccessor (&EpcGroupHelper::m_UeConnect),
                 "ns3::EpcGroupHelper::UeConnect")
    // .AddTraceSource ("StopRealServices",
    //              "add description here",
    //              MakeTraceSourceAccessor (&EpcGroupHelper::m_StopRealServices),
    //              "ns3::EpcGroupHelper::StopRealServices")
    .AddTraceSource ("SetupCsmaUe",
                 "add description here",
                 MakeTraceSourceAccessor (&EpcGroupHelper::m_setupCsmaUe),
                 "ns3::EpcGroupHelper::SetupCsmaUe")
    .AddTraceSource ("StopRunningApp",
             "add description here",
             MakeTraceSourceAccessor (&EpcGroupHelper::m_StopRunningApp),
             "ns3::EpcGroupHelper::StopRunningApp")
  ;
  return tid;
}

TypeId
EpcGroupHelper::GetInstanceTypeId () const
{
  return GetTypeId ();
}

EpcGroupHelper::EpcGroupHelper () 
{
  enbDistance = 30.0;
  epcHelper = CreateObject<PointToPointEpcHelperExt> ();

  lteHelper = CreateObject<LteHelper> ();

}

EpcGroupHelper::EpcGroupHelper (uint16_t index) 
{
  enbDistance = 30.0;
  epcHelper = CreateObject<PointToPointEpcHelperExt> ();
  epcIndex = index;
  epcHelper->setZone((epcIndex+1));

  lteHelper = CreateObject<LteHelper> ();

}

void
EpcGroupHelper::setup () {

  double enbTxPowerDbm = 46.0;

  lteHelper->setInitialValue((epcIndex+1)*100, (epcIndex+1)*10);
  // epcHelper = CreateObject<PointToPointEpcHelperExt> ();
  epcHelper->setup();

  // Ptr<ListPositionAllocator> epcPositionAlloc = CreateObject<ListPositionAllocator> ();
  // for (uint16_t i = 0; i < 3; i++)
  //   {
  //     Vector epcPosition (posX + (i%2)*5, posY * (i + 1), i+3);
  //     epcPositionAlloc->Add (epcPosition);
  //   }
  
  // MobilityHelper epcMobility;
  // epcMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  // epcMobility.SetPositionAllocator (epcPositionAlloc);
  // epcMobility.Install (epcHelper->getNodeContainerBackhaul());

  Ptr<Node> pgw = epcHelper->GetPgwNode();
  Ptr<Node> sgw = epcHelper->GetSgwNode();
  Ptr<Node> mme = epcHelper->GetMmeNode();
  Ptr<Node> hss = epcHelper->GetHssNode();


  // setNodePosition(pgw, posX, posY*2, 3);
  // setNodePosition(sgw, posX + 5, posY*3, 4);
  // setNodePosition(mme, posX + 30, posY*3, 5);
  // setNodePosition(hss, posX + 40, posY*3-20, 8);

  setNodePosition(pgw, posX, posY + enbDistance, 3);
  if (epcIndex==1) {
    // setNodePosition(sgw, posX + enbDistance*10000, posY + enbDistance*2, 4);
    setNodePosition(sgw, posX + enbDistance/5, posY + enbDistance*2, 4);
  } else {
    setNodePosition(sgw, posX + enbDistance/5, posY + enbDistance*2, 4);
  }
  setNodePosition(mme, posX + enbDistance*3/5, posY + enbDistance*2, 5);
  setNodePosition(hss, posX + enbDistance*4/5, posY + enbDistance*2 - enbDistance/4, 8);

  Names::Add ("PGW-" + std::to_string(epcIndex), pgw);
  Names::Add ("SGW-"+ std::to_string(epcIndex), sgw);
  Names::Add ("MME-"+ std::to_string(epcIndex), mme);
  Names::Add ("HSS-"+ std::to_string(epcIndex), hss);

  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");

  lteHelper->SetHandoverAlgorithmType ("ns3::A2A4RsrqHandoverAlgorithm");
  lteHelper->SetHandoverAlgorithmAttribute ("ServingCellThreshold",
                                            UintegerValue (30));
  lteHelper->SetHandoverAlgorithmAttribute ("NeighbourCellOffset",
                                            UintegerValue (1));

  //for radio link failure
  lteHelper->SetPathlossModelType (TypeId::LookupByName ("ns3::LogDistancePropagationLossModel"));
  lteHelper->SetPathlossModelAttribute ("Exponent", DoubleValue (3.9));
  lteHelper->SetPathlossModelAttribute ("ReferenceLoss", DoubleValue (38.57)); //ref. loss in dB at 1m for 2.025GHz
  lteHelper->SetPathlossModelAttribute ("ReferenceDistance", DoubleValue (1));

  lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (100)); //2120MHz
  lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (18100)); //1930MHz
  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (25)); //5MHz
  lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (25)); //5MHz


  // Create a single RemoteHost
  // NodeContainer remoteHostContainer;
  remoteHostContainer.Create (2);
  remoteHost = remoteHostContainer.Get (0);

  Names::Add ("RemoteHost-" + std::to_string(epcIndex), remoteHost);

  internet.Install (remoteHostContainer);

  Ptr<ListPositionAllocator> svrPositionAlloc = CreateObject<ListPositionAllocator> ();
  // for (uint16_t i = 0; i < 3; i++)
  //   {
  posX = posX + 1;
  posY = posY + 1;
  std::cout<<"Server:"<< epcIndex << " "<<posX <<" " <<posY << std::endl;
  Vector svrPosition (posX, posY, 9);
  svrPositionAlloc->Add (svrPosition);
    // }
  
  MobilityHelper svrMobility;
  svrMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  svrMobility.SetPositionAllocator (svrPositionAlloc);
  svrMobility.Install (remoteHostContainer);


  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("10Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0)));

  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase (svrIpv4Address.c_str(), "255.255.0.0");

  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  remoteHostAddr = internetIpIfaces.GetAddress (1);


  // Routing of the Internet Host (towards the LTE network)
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  // interface 0 is localhost, 1 is the p2p device

  // Ipv4Address gateway = epcHelper->GetUeDefaultGatewayAddress ();

  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address (ueIpv4Address.c_str()), Ipv4Mask ("255.255.0.0"), 1);
  // remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("10.5.0.0"), Ipv4Mask ("255.255.0.0"), 1);
  // remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("10.5.0.0"), Ipv4Mask ("255.255.0.0"), gateway, 2);
  // remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("10.0.5.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("10.0.5.5"), 1);

  // Install Mobility Model in eNB
  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  std::cout<<"eNodeB locations"<<std::endl;
  for (uint16_t i = 0; i < numberOfEnbs; i++)
    {
      Vector enbPosition (posX + enbDistance * i, posY+enbDistance*3, 6);
      std::cout<<"--"<<i<<" "<<(posX + enbDistance * i)<<" "<<(posY+enbDistance*3)<<std::endl;
      enbPositionAlloc->Add (enbPosition);
    }
  MobilityHelper enbMobility;
  enbMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbMobility.SetPositionAllocator (enbPositionAlloc);
  enbMobility.Install (enbNodes);

  
  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (enbTxPowerDbm));
  enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);

  // Add X2 interface
  lteHelper->AddX2Interface (enbNodes);

  // X2-based Handover
  //lteHelper->HandoverRequest (Seconds (0.100), ueLteDevs.Get (0), enbLteDevs.Get (0), enbLteDevs.Get (1));

  // Uncomment to enable PCAP tracing
  // p2ph.EnablePcapAll("lena-x2-handover-measures");

  lteHelper->EnablePhyTraces ();
  lteHelper->EnableMacTraces ();
  lteHelper->EnableRlcTraces ();
  lteHelper->EnablePdcpTraces ();
  // Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats ();
  // rlcStats->SetAttribute ("EpochDuration", TimeValue (Seconds (1.0)));
  // Ptr<RadioBearerStatsCalculator> pdcpStats = lteHelper->GetPdcpStats ();
  // pdcpStats->SetAttribute ("EpochDuration", TimeValue (Seconds (1.0)));

  std::string csmaBase = std::to_string(10+epcIndex)+".5.0.0";
  // ipv4Csma.SetBase (csmaBase.c_str(), "255.255.0.0");

  // if (tapEnable){
  //   setupCsmaServer();
  // }

  // Ipv4Address gateway = epcHelper->GetUeDefaultGatewayAddress (); 

  // remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("10.5.0.0"), Ipv4Mask ("255.255.0.0"), 1);
  // remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("10.5.0.0"), Ipv4Mask ("255.255.0.0"), gateway, 2);
  // remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("10.0.5.0"), Ipv4Mask ("255.255.255.0"), Ipv4Address ("10.0.5.5"), 1);
}


void
EpcGroupHelper::pingApp(Ptr<Node> fromNode, std::string toAddr)
{
  NS_LOG_INFO ("Create Applications.");
  uint32_t packetSize = 1024;
  Time interPI = Seconds (1.0);
  V4PingHelper ping (toAddr.c_str());

  ping.SetAttribute ("Interval", TimeValue (interPI));
  ping.SetAttribute ("Size", UintegerValue (packetSize));
  ping.SetAttribute ("Verbose", BooleanValue (true));

  ApplicationContainer apps = ping.Install (fromNode);
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (5.0));
};

void
EpcGroupHelper::printRoutingTable(Ptr<Ipv4StaticRouting> sr)
{
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);

  sr->PrintRoutingTable (routingStream, ns3::Time::Unit(5));
};

// Ptr<Node> 
// EpcGroupHelper::getTapNode(Ptr<Node> ue)
// {
//   std::map<Ptr<Node>, Ptr<Node> >::iterator it = ueTaps.find (ue);
//   NS_ASSERT_MSG (it != ueTaps.end (), "could not find any ue tap ");

//   return it->second;
// };


void EpcGroupHelper::setEnbDistance(double enbd)
{
  enbDistance = enbd;
};


EpcGroupHelper::~EpcGroupHelper ()
{

}


Ptr<LteHelper> 
EpcGroupHelper::getLteHelper()
{
  return lteHelper;
}

InternetStackHelper
EpcGroupHelper::getInternet()
{
  return internet;
}

Ptr<PointToPointEpcHelperExt> 
EpcGroupHelper::getEpcHelper()
{
  return epcHelper;
};


void 
EpcGroupHelper::setNodePosition(Ptr<Node> node, uint16_t posx, uint16_t posy,uint16_t posz)
{
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  Vector vposition (posx,posy,posz);

  positionAlloc->Add (vposition);
  
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (node);

};


NodeContainer
EpcGroupHelper::getRemoteHostContainer ()
{
  return remoteHostContainer;
}

// Ptr<PointToPointEpcHelperExt>
// EpcGroupHelper::getEpcHelper ()
// // Ptr<PointToPointEpcHelperExt> getEpcHelper
// {
//   return epcHelper;
// }

NodeContainer 
EpcGroupHelper::getEpcContainer()
{
  return epcHelper->getNodeContainerBackhaul();
}

Ipv4Address
EpcGroupHelper::getRemoteAddress ()
{
  return remoteHostAddr;
}

void EpcGroupHelper::setSvrIpv4Address(std::string addr)
{
  svrIpv4Address = addr;
};

void EpcGroupHelper::setUeIpv4Address(std::string addr)
{
  ueIpv4Address = addr;
};

void EpcGroupHelper::setNumberOfEnbs(uint16_t num) 
{
  numberOfEnbs = num;
  enbNodes.Create (numberOfEnbs);
};

void EpcGroupHelper::setPosition(uint16_t posx, uint16_t posy)
{
  posX = posx;
  posY = posy;
};

NetDeviceContainer 
EpcGroupHelper::getEnbLteDevs()
{
  return enbLteDevs;
};

NetDeviceContainer 
EpcGroupHelper::getUeLteDevs()
{
  return ueLteDevs;
};

NodeContainer
EpcGroupHelper::getEnbNodes()
{
  return enbNodes;
};

Ptr<Node> 
EpcGroupHelper::getRemoteHost()
{
  return remoteHost;
};


void 
EpcGroupHelper::attachUeNode(NodeContainer ueNodes)
{
  ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

  internet.Install(ueNodes);

  ueIpIfaces = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));

  // for (uint16_t i = 0; i < numberOfUes; i++)
  // {
  //   lteHelper->Attach (ueLteDevs.Get (i), enbLteDevs.Get (0));
  // }
  lteHelper->Attach (ueLteDevs);
};

void 
EpcGroupHelper::attachUeNode(Ptr<Node> ueNode)
{
  // ueLteDevs = lteHelper->InstallUeDevice (ueNode);
  NetDeviceContainer ueLte = lteHelper->InstallUeDevice(NodeContainer(ueNode));

  internet.Install(NodeContainer(ueNode));

  Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address (ueLte);

  lteHelper->Attach (ueLte.Get(0), enbLteDevs.Get(0));

  ueLteDevs.Add(ueLte);
  
  ueIpIfaces.Add(ueIpIface);

};

void 
EpcGroupHelper::detachUeNode(Ptr<Node> ueNode, uint16_t currentCellId, uint16_t currentRnti)
{
  // std::cout<<"EpcGroupHelper::detachUeNode::"<< ueLteDevs.GetN() <<std::endl;
  // Ptr<LteUeNetDevice> ptrUeLte = getUeLte(ueNode);

  // //epcHelper->RemoveIpv4Address (ueLte)
  // ueNode->GetObject<Ipv4> ()->RemoveAddress (ueNode->GetObject<Ipv4> ()->GetInterfaceForDevice (ptrUeLte), 0);

  // // lteHelper->RemoveUeDevice(ueNode);
  // ptrUeLte->GetNas()->Disconnect();
  // ptrUeLte->GetRrc()->LeaveConnectedMode();
  // ptrUeLte->GetRrc()->Disconnect();

  // // ptrUeLte->GetRrc()->Dispose();
  // // ptrUeLte->GetNas()->Dispose();

  // //lteHelper->Detach()
  // Ptr<LteEnbNetDevice> lteEnb = findEnbWithCellId(currentCellId);
  // lteEnb->GetRrc()->RemoveUe(currentRnti);

  // ptrUeLte->Dispose();

  // std::cout<<"EpcGroupHelper::detachUeNode:: out lteEnb::"<< ueLteDevs.GetN() <<std::endl;

  // uint32_t numOfDevs =  ueLteDevs.GetN();

  // for (uint32_t k = 0; k < numOfDevs; k++) { 
  //   Ptr<Node> ck = ueLteDevs.Get(k)->GetNode();

  //   // std::cout<<"pass3 : "<<ck->GetId()<<std::endl;
  //   if (ck->GetId() == ueNode->GetId()) {
  //     ueLteDevs.Get(k)->Dispose();
  //   }
  // }

  // ueLteDevs->Remove ueLTe

  //  ueIpIfaces->Remove ueIpIFace
}

uint16_t
EpcGroupHelper::getEpcIndex()
{
  return epcIndex;
};


void 
EpcGroupHelper::stopRunningApp(Ptr<Node> ueNode)
{
  // std::cout<<"EpcGroupHelper::stopRunningApp"<<std::endl;
  // if (!sendRequestViaRoaming && !tapEnable) {
  //   m_StopRunningApp(epcIndex,ueNode);
  // } else {
  //   Ipv4Address oldAddr = getCurrentIpv4Address(ueNode);
  //   epcHelper->getPgwApp()->AddAddrToBackwards8(oldAddr);
  // }
  if (sendRequestViaRoaming) {
    Ipv4Address oldAddr = getCurrentIpv4Address(ueNode);
    epcHelper->getPgwApp()->AddAddrToBackwards8(oldAddr);
  }

  m_StopRunningApp(epcIndex,ueNode);

}

void 
EpcGroupHelper::attachUeNode(Ptr<Node> ueNode, uint16_t enbIndex)
{
  // ueLteDevs = lteHelper->InstallUeDevice (ueNode);
  Ptr<LteUeNetDevice> ptrUeLte = getUeLte(ueNode);

  if(!ptrUeLte){
    NetDeviceContainer ueLte;

    ueLte = lteHelper->InstallUeDevice(NodeContainer(ueNode));
    internet.Install(NodeContainer(ueNode));
    Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address (ueLte);
    lteHelper->Attach (ueLte.Get(0), enbLteDevs.Get(enbIndex));
    ueLteDevs.Add(ueLte);
    ueIpIfaces.Add(ueIpIface);

    // std::cout<<"not ptrUeLte"<<std::endl;
    getCurrentIpv4Address(ueNode);

    if (tapEnable)
    {
      m_setupCsmaUe(ueNode);
    }

    m_UeConnect(epcIndex,ueNode);


  } else {
    // if (tapEnable) {
    //   m_StopRealServices(epcIndex, ueNode);
    // }
    epcHelper->getMmeApp()->setCallbackAfterCheckingRoaming(MakeCallback (&EpcGroupHelper::attachUeNodeAfterChecking,this), ptrUeLte, enbIndex);

    epcHelper->getMmeApp()->checkImsiViaRoaming(ptrUeLte->GetImsi());
  }

};

void 
EpcGroupHelper::attachUeNodeAfterChecking(Ptr<LteUeNetDevice> ptrUeLte, uint16_t enbIndex)
{
  // std::cout<<"refresh to roaming - epcIndex:"<< epcIndex <<" enbIndex:"<<enbIndex<<" rrc imsi:"<<ptrUeLte->GetRrc()->GetImsi()<<std::endl;

//TODO:
//  uint16_t rnti = AddUe (UeManager::HANDOVER_JOINING, CellToComponentCarrierId (enb cellID));
// or LteEnbRrc::DoAllocateTemporaryCellRnti (uint8_t componentCarrierId): check if they call this function
  // 
  // rnti = enb->getRRC->GetLteEnbCmacSapUser->AllocateTemporaryCellRnti();
  //
//Time deActivateTime (Seconds (1.5));
  // Simulator::Schedule (deActivateTime, &LteHelper::DeActivateDedicatedEpsBearer, lteHelper, ueDevice, enbDevice, 2);

  NetDeviceContainer ueLte;

  uint64_t m_imsi = ptrUeLte->GetImsi();

  // std::cout<<"before refresh ue with ip:"<<std::endl;
  Ipv4Address oldAddr = getCurrentIpv4Address(ptrUeLte->GetNode());

  // ///
  Ptr<Node> ueNode = ptrUeLte->GetNode();
  // // Ptr<NetDevice> netdevs = ueNode->GetDevice();

  // std::cout<<"Remove address:"<<std::endl;
  ueNode->GetObject<Ipv4> ()->RemoveAddress (ueNode->GetObject<Ipv4> ()->GetInterfaceForDevice (ptrUeLte), 0);

  // ptrUeLte->GetNas()->Disconnect();
  ptrUeLte->GetNas()->resetConnection();
  ptrUeLte->GetRrc()->LeaveCurrentEpc();
  // if (!sendRequestViaRoaming) {
  //   ptrUeLte->resetFrameNo();
  // }

  // ptrUeLte->GetRrc()->LeaveConnectedMode();
  // ptrUeLte->GetRrc()->Disconnect();

  // std::cout<<"after disconnect::rrc state:"<<ptrUeLte->GetRrc()->GetState()<<std::endl;

  // ueNode->GetObject<Ipv6> ()->RemoveAddress (ueNode->GetObject<Ipv6> ()->GetInterfaceForDevice (ptrUeLte), 0);

  ptrUeLte = lteHelper->refreshLteUeDevice(ptrUeLte, m_imsi);
  // ptrUeLte->Dispose();

  // epcHelper->AddUe (ptrUeLte, ptrUeLte->GetImsi ());


  // std::cout<<"check ptrUeLte:"<<getUeLte(ptrUeLte->GetNode())<<std::endl;

  ueLte.Add(ptrUeLte);
  
  // std::cout<<"EpcGroupHelper pass1"<<std::endl;

  Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address (ueLte);

  // std::cout<<"EpcGroupHelper pass2"<<std::endl;

  // Ptr<Node> enbNode = enbLteDevs.Get(enbIndex);
  Ptr<LteEnbNetDevice> lteEnb = enbLteDevs.Get(enbIndex)->GetObject<LteEnbNetDevice> ();

  ptrUeLte->SetTargetEnb (lteEnb);

  // epcHelper->AddUe (ptrUeLte, ptrUeLte->GetImsi ());


  // std::cout<<"EpcGroupHelper pass3"<<ueLte.Get(0)<<std::endl;

  ueLteDevs.Add(ueLte);

  ueIpIfaces.Add(ueIpIface);

  // ptrUeLte->GetNas()->Connect();


  // std::cout<<"attached ue with ip:"<<std::endl;
  Ipv4Address newAddr =  getCurrentIpv4Address(ptrUeLte->GetNode());
  if (sendRequestViaRoaming){
    epcHelper->getSgwApp()->AddAddrToForwards8(newAddr, oldAddr, m_imsi);
  }

  //rnti in : LteEnbRrc::AddUe (UeManager::State state, uint8_t componentCarrierId)

  // uint16_t m_rnti = ptrUeLte->GetRrc()->GetRnti();
  // uint16_t m_cellId = lteEnb->GetCellId();
  // uint16_t m_rnti = lteEnb->GetRrc()->DoAllocateTemporaryCellRnti(lteEnb->GetRrc()->CellToComponentCarrierId(m_cellId));
  // uint8_t m_componentCarrierId = lteEnb->GetRrc()->CellToComponentCarrierId(m_cellId);
  // uint16_t m_rnti = lteEnb->GetRrc()->AddUe(UeManager::INITIAL_RANDOM_ACCESS, m_componentCarrierId);

  // lteEnb->GetRrc()->GetUeManager(m_rnti)->SwitchToState(UeManager::ATTACH_REQUEST);

  // std::cout<<"->GetRnti"<< m_rnti <<" m_cellId:" << m_cellId <<std::endl;

  // Ptr<EpcEnbApplication> enbApp = enbNodes.Get(enbIndex)->GetApplication (0)->GetObject<EpcEnbApplication> ();

  lteHelper->Attach (ueLte.Get(0), enbLteDevs.Get(enbIndex));


  // ptrUeLte->GetRrc()->GetAsSapProvider ()->Connect();
  // ptrUeLte->GetRrc()->SynchronizeToStrongestCell();

  // // ptrUeLte->reInitialze ();
  // ptrUeLte->reInitialze ();
  // // enbApp->DoInitialUeMessage(m_imsi, m_rnti);

  // std::cout<<"EpcGroupHelper: after enbApp->DoInitialUeMessage"<<std::endl;

  // // epcHelper->getMmeApp()->DoInitialUeMessage (m_imsi, m_rnti, m_imsi, m_cellId);

  // std::cout<<"next:"<<checkUeExistInNetwork(ueNode)<<std::endl;
  // std::cout<<"enb Now:"<<ptrUeLte->GetTargetEnb()->GetCellId()<<std::endl;
  // // std::cout<<"Nas imsi:"<<ptrUeLte->GetNas()->GetImsi()<<std::endl;
  // std::cout<<"Nas rrc:"<<ptrUeLte->GetRrc()->GetImsi()<<std::endl;


  // ////////
  //   //rnti in : LteEnbRrc::AddUe (UeManager::State state, uint8_t componentCarrierId)

  // uint16_t m_rnti = ptrUeLte->GetRrc()->GetRnti();
  // uint16_t m_cellId = lteEnb->GetCellId();
  // // m_rnti = lteEnb->GetRrc()->DoAllocateTemporaryCellRnti(lteEnb->GetRrc()->CellToComponentCarrierId(m_cellId));
  // // uint16_t m_rnti = lteEnb->GetRrc()->DoAllocateTemporaryCellRnti(lteEnb->GetRrc()->CellToComponentCarrierId(m_cellId));
  // // uint8_t m_componentCarrierId = lteEnb->GetRrc()->CellToComponentCarrierId(m_cellId);
  // // uint16_t m_rnti = lteEnb->GetRrc()->AddUe(UeManager::INITIAL_RANDOM_ACCESS, m_componentCarrierId);

  // ptrUeLte->GetPhy()->GetLteUeCphySapProvider()->SynchronizeWithEnb(m_cellId);


  // std::cout<<"rnti :"<<m_rnti<<" cellId:"<<m_cellId<<std::endl;
  // std::cout<<"EpcGroupHelper::attachUeNodeAfterChecking"<<std::endl;

  if (!sendRequestViaRoaming) {
    //false =>Restart application
    m_UeConnect(epcIndex,ueNode);
  } else if (tapEnable) {
    m_UeConnect(epcIndex,ueNode);
  }
}

void
EpcGroupHelper::setServerTapIp(std::string addr)
{
  serverTapIp = addr;
};


Ptr<LteUeNetDevice> 
EpcGroupHelper::getUeLte(Ptr<Node> ue)
{
  int nDevs = ue->GetNDevices ();

  // std::cout<<"EpcGroupHelper::getUeLte : nDevs:"<<nDevs<<std::endl;
  
  for (int j = 0; j < nDevs; j++)
  {
    Ptr<LteUeNetDevice> uedev = ue->GetDevice (j)->GetObject <LteUeNetDevice> ();
    if (uedev){
      // std::cout<<">j "<< j <<" imsi " << uedev->GetImsi() <<std::endl;
      return uedev;
    }
  }
  return NULL;
};

Ipv4Address
EpcGroupHelper::getCurrentIpv4Address(Ptr<Node> node)
{
  int nDevs = node->GetNDevices ();

  // std::cout<<"nDevs:"<<nDevs<<std::endl;
  
  for (int j = 0; j < nDevs; j++)
  {
    Ptr<LteUeNetDevice> uedev = node->GetDevice (j)->GetObject <LteUeNetDevice> ();
    if (uedev){
      // std::cout<<">j "<< j <<" imsi " <<std::endl;
      // std::cout<< uedev->GetImsi() <<std::endl;
      // std::cout<<"->>"<<node->GetObject<Ipv4> ()->GetAddress (node->GetObject<Ipv4> ()->GetInterfaceForDevice (uedev), 0).GetLocal ()<<std::endl;
      return node->GetObject<Ipv4> ()->GetAddress (node->GetObject<Ipv4> ()->GetInterfaceForDevice (uedev), 0).GetLocal ();
    }
  }

  return 0;
  // Ptr<Ipv4> ueIpv4 = node->GetObject<Ipv4> ();
  // int32_t interface =  ueIpv4->GetInterfaceForDevice (getUeLte(node));
  // Ipv4Address ueAddr = ueIpv4->GetAddress (interface, 0).GetLocal ();
  // return ueAddr;
};


Ipv4InterfaceContainer 
EpcGroupHelper::getUeIpIfaces()
{
  return ueIpIfaces;
};


bool
EpcGroupHelper::checkExistEnbCellId(uint16_t cellId)
{
  for (uint16_t i = 0; i < numberOfEnbs; i++)
    {
      // Ptr<LteEnbNetDevice> uedev = enbLteDevs.Get(i)->GetObject<LteEnbNetDevice>();

      // std::cout<<"---"<<i<<" "<<uedev->GetCellId()<<"x"<<std::endl;
      // if (uedev->GetCellId() == cellId) {
      //   return true;
      // }
      Ptr<Node> node = enbNodes.Get(i);
      int devs = node->GetNDevices ();
      for (int j = 0; j < devs; j++)
        {
          Ptr<LteEnbNetDevice> enbdev = node->GetDevice (j)->GetObject <LteEnbNetDevice> ();
          if (enbdev){
            std::cout<<"---"<<i<<" "<<" "<<node->GetId()<<" "<<j<<" "<<enbdev->GetCellId()<<std::endl;
            if (enbdev->GetCellId() == cellId) {
              return true;
            }
          }
        }
    }
  return false;
};


uint16_t
EpcGroupHelper::checkExistEnbId(uint32_t id)
{
  for (uint16_t i = 0; i < numberOfEnbs; i++)
    {
      if (enbNodes.Get(i)->GetId() == id) {
        return i+1;
      };
      
    }
  return 0;
};

Ptr<LteEnbNetDevice> 
EpcGroupHelper::findEnbWithCellId(uint16_t cellId)
{
  // std::cout<<"EpcGroupHelper::findEnbWithCellId"<<std::endl;
  for (uint16_t i = 0; i < numberOfEnbs; i++)
    {
      Ptr<Node> node = enbNodes.Get(i);
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteEnbNetDevice> enbdev = node->GetDevice (j)->GetObject <LteEnbNetDevice> ();
          if (enbdev)
          {
            if (enbdev->GetCellId() == cellId) {
              // std::cout<<"EpcGroupHelper::findEnbWithCellId:return"<<std::endl;
              return enbdev;
            }
          }
        }
    }
  // std::cout<<"EpcGroupHelper::findEnbWithCellId:Nooo"<<std::endl;
  return 0;
};

void
EpcGroupHelper::showEnbNodes()
{
  std::cout<<"EpcGroupHelper::showEnbNodes"<<std::endl;

  for (uint16_t i = 0; i < numberOfEnbs; i++)
    {
      Ptr<Node> node = enbNodes.Get(i);
      std::cout<<"- "<<i<<" "<<node->GetId()<<std::endl;

      int devs = node->GetNDevices ();
      for (int j = 0; j < devs; j++)
        {
          Ptr<LteEnbNetDevice> enbdev = node->GetDevice (j)->GetObject <LteEnbNetDevice> ();
          if (enbdev){
            std::cout<<"+++"<<i<<" id:"<<node->GetId()<<" "<<j<<" cellId:"<<enbdev->GetCellId()<<" DlEarfcn:"<<  enbdev->GetDlEarfcn()<<std::endl;
            // <<" rnti:"<<enbdev->GetRrc()->GetRnti()
          }
        }
    }
};

bool
EpcGroupHelper::checkUeExistInNetwork(Ptr<Node> ueNode)
{
  uint32_t numOfDevs =  ueLteDevs.GetN();

  for (uint32_t k = 0; k < numOfDevs; k++) { 
    Ptr<Node> ck = ueLteDevs.Get(k)->GetNode();

    // std::cout<<"pass3 : "<<ck->GetId()<<std::endl;
    if (ck->GetId() == ueNode->GetId()) {
      // std::cout<<"pass3 : "<<ck->GetId()<<" choose epc "<< i <<std::endl;
      return true;
    }
  }
  return false;
}

Ipv4Address
EpcGroupHelper::getAddressOfUe(Ptr<Node> ueNode)
{
  uint32_t numOfDevs =  ueLteDevs.GetN();

  for (uint32_t k = 0; k < numOfDevs; k++) { 
    Ptr<Node> ck = ueLteDevs.Get(k)->GetNode();

    // std::cout<<"pass3 : "<<ck->GetId()<<std::endl;
    if (ck->GetId() == ueNode->GetId()) {
      // std::cout<<"pass3 : "<<ck->GetId()<<" choose epc "<< i <<std::endl;
      return ueIpIfaces.GetAddress (k);
    }
  }
  return Ipv4Address();
};


uint32_t
EpcGroupHelper::getRealIndexOfUe(Ptr<Node> ueNode)
{
  uint32_t numOfDevs =  ueLteDevs.GetN();

  for (uint32_t k = 0; k < numOfDevs; k++) { 
    Ptr<Node> ck = ueLteDevs.Get(k)->GetNode();

    // std::cout<<"pass3 : "<<ck->GetId()<<std::endl;
    if (ck->GetId() == ueNode->GetId()) {
      // std::cout<<"pass3 : "<<ck->GetId()<<" choose epc "<< i <<std::endl;
      return k;
    }
  }
  return -1;
};

void 
EpcGroupHelper::setIpWithIndex(uint16_t index)
{

  // svrIpv4Address = std::to_string(index)+std::string(".0.0.0");
  // ueIpv4Address = std::to_string((index+10))+std::string(".0.0.0");

  // std::string ue6 = std::to_string((index))+std::to_string(index)+std::to_string(index)+std::to_string(index)+std::string(":f00d::");
  // std::string x2 = std::to_string((index+20))+std::string(".0.0.0");

  // std::string s11 = std::to_string((index+30))+std::string(".0.0.0");
  // std::string s5 = std::to_string((index+40))+std::string(".0.0.0");

  // epcHelper->setIpBackhaul(
  //   x2,
  //   s11,
  //   s5,
  //   ueIpv4Address,
  //   ue6
  // );

  // std::string s1u = std::to_string((index+50))+std::string(".0.0.0");
  // std::string s1ap = std::to_string((index+60))+std::string(".0.0.0");

  // epcHelper->setIpP2P(
  //   s1u,
  //   s1ap
  // );

  // svrIpv4Address = std::to_string(index)+std::string(".0.0.0");
  // ueIpv4Address = std::to_string((index+10))+std::string(".0.0.0");

  // std::string ue6 = std::to_string((index))+std::to_string(index)+std::to_string(index)+std::to_string(index)+std::string(":f00d::");
  // std::string x2 = std::to_string((index+20))+std::string(".0.0.0");

  // std::string s11 = std::to_string((index+30))+std::string(".0.0.0");
  // std::string s5 = std::to_string((index+40))+std::string(".0.0.0");

  // epcHelper->setIpBackhaul(
  //   x2,
  //   s11,
  //   s5,
  //   ueIpv4Address,
  //   ue6
  // );

  // std::string s1u = std::to_string((index+50))+std::string(".0.0.0");
  // std::string s1ap = std::to_string((index+60))+std::string(".0.0.0");

  // epcHelper->setIpP2P(
  //   s1u,
  //   s1ap
  // );

  svrIpv4Address = std::string("10.")+std::to_string(index)+std::string(".0.0");
  ueIpv4Address = std::string("10.")+std::to_string(index+10)+std::string(".0.0");

  std::string ue6 = std::to_string((index))+std::to_string(index)+std::to_string(index)+std::to_string(index)+std::string(":f00d::");
  std::string x2 = std::string("10.")+std::to_string(index+20)+std::string(".0.0");

  std::string s11 = std::string("10.")+std::to_string(index+30)+std::string(".0.0");
  std::string s5 = std::string("10.")+std::to_string(index+40)+std::string(".0.0");

  std::string s6a = std::string("10.")+std::to_string(index+50)+std::string(".0.0");

  epcHelper->setIpBackhaul(
    x2,
    s11,
    s5,
    s6a,
    ueIpv4Address,
    ue6
  );

  std::string s1u = std::string("10.")+std::to_string(index*2+60)+std::string(".0.0");
  std::string s1ap = std::string("10.")+std::to_string(index+70)+std::string(".0.0");

  epcHelper->setIpP2P(
    s1u,
    s1ap
  );

  // tapSrvAddress = std::string("10.")+std::to_string(index+80)+std::string(".0.0");
  // tapUeAddress = std::string("10.")+std::to_string(index+90)+std::string(".0.0");

  // epcHelper->setIp(
  //   "12.0.0.0",
  //   "13.0.0.0",
  //   "14.0.0.0",
  //   "7.0.0.0",
  //   "7777:f00d::"
  // );
}

void
EpcGroupHelper::setTapEnable(bool enable)
{
  tapEnable = enable;
};

void
EpcGroupHelper::setSendRequestViaRoaming(bool m_ck)
{
  sendRequestViaRoaming = m_ck;
};

bool
EpcGroupHelper::getSendRequestViaRoaming()
{
  return sendRequestViaRoaming;
};

std::string 
EpcGroupHelper::getSrvIpv4AddressBase()
{
  return svrIpv4Address;
};

std::string 
EpcGroupHelper::getUeIpv4AddressBase()
{
  return ueIpv4Address;
};

} // namespace ns3