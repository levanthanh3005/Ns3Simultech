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

#include "epc-global-helper.h"

#include "epc-hss-application.h"
#include "epc-mme-application-ext.h"
#include "epc-sgw-application-ext.h"
#include "epc-pgw-application-ext.h"

#include <ns3/multi-model-spectrum-channel.h>




namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EpcGlobalHelper");

NS_OBJECT_ENSURE_REGISTERED (EpcGlobalHelper);


TypeId
EpcGlobalHelper::GetTypeId (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  static TypeId tid = TypeId ("ns3::EpcGlobalHelper")
    .SetParent<Object> ()
    .SetGroupName ("EpcGroup")
    .AddAttribute ("PathlossModel",
               "The type of pathloss model to be used. "
               "The allowed values for this attributes are the type names "
               "of any class inheriting from ns3::PropagationLossModel.",
               TypeIdValue (FriisPropagationLossModel::GetTypeId ()),
               MakeTypeIdAccessor (&EpcGlobalHelper::SetPathlossModelType),
               MakeTypeIdChecker ())
  ;
  return tid;
}

// void
// EpcGlobalHelper::DoInitialize(void) 
// {
//   ChannelModelInitialization();
// }

TypeId
EpcGlobalHelper::GetInstanceTypeId () const
{
  return GetTypeId ();
}

EpcGlobalHelper::EpcGlobalHelper () 
{
  m_channelFactory.SetTypeId (MultiModelSpectrumChannel::GetTypeId ());
  // ChannelModelInitialization();
}

void 
EpcGlobalHelper::SetFadingModel (std::string type) 
{
  NS_LOG_FUNCTION (this << type);
  m_fadingModelType = type;
  if (!type.empty ())
    {
      m_fadingModelFactory = ObjectFactory ();
      m_fadingModelFactory.SetTypeId (type);
    }
}

void 
EpcGlobalHelper::SetFadingModelAttribute (std::string n, const AttributeValue &v)
{
  m_fadingModelFactory.Set (n, v);
}

void 
EpcGlobalHelper::SetSpectrumChannelType (std::string type) 
{
  NS_LOG_FUNCTION (this << type);
  m_channelFactory.SetTypeId (type);
}

void 
EpcGlobalHelper::SetSpectrumChannelAttribute (std::string n, const AttributeValue &v)
{
  m_channelFactory.Set (n, v);
}

void
EpcGlobalHelper::SetPathlossModelType (TypeId type)
{
  NS_LOG_FUNCTION (this << type);
  m_pathlossModelFactory = ObjectFactory ();
  m_pathlossModelFactory.SetTypeId (type);
}

void 
EpcGlobalHelper::SetPathlossModelAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this << n);
  m_pathlossModelFactory.Set (n, v);
}

void
EpcGlobalHelper::ChannelModelInitialization (void)
{
  // std::cout<<"EpcGlobalHelper::ChannelModelInitialization (void)"<<std::endl;
  // Channel Object (i.e. Ptr<SpectrumChannel>) are within a vector
  // PathLossModel Objects are vectors --> in InstallSingleEnb we will set the frequency
  // NS_LOG_FUNCTION (this << m_noOfCcs);

  m_downlinkChannel = m_channelFactory.Create<SpectrumChannel> ();
  m_uplinkChannel = m_channelFactory.Create<SpectrumChannel> ();

  m_downlinkPathlossModel = m_pathlossModelFactory.Create ();
  Ptr<SpectrumPropagationLossModel> dlSplm = m_downlinkPathlossModel->GetObject<SpectrumPropagationLossModel> ();
  if (dlSplm != 0)
    {
      NS_LOG_LOGIC (this << " using a SpectrumPropagationLossModel in DL");
      m_downlinkChannel->AddSpectrumPropagationLossModel (dlSplm);
    }
  else
    {
      NS_LOG_LOGIC (this << " using a PropagationLossModel in DL");
      Ptr<PropagationLossModel> dlPlm = m_downlinkPathlossModel->GetObject<PropagationLossModel> ();
      NS_ASSERT_MSG (dlPlm != 0, " " << m_downlinkPathlossModel << " is neither PropagationLossModel nor SpectrumPropagationLossModel");
      m_downlinkChannel->AddPropagationLossModel (dlPlm);
    }

  m_uplinkPathlossModel = m_pathlossModelFactory.Create ();
  Ptr<SpectrumPropagationLossModel> ulSplm = m_uplinkPathlossModel->GetObject<SpectrumPropagationLossModel> ();
  if (ulSplm != 0)
    {
      NS_LOG_LOGIC (this << " using a SpectrumPropagationLossModel in UL");
      m_uplinkChannel->AddSpectrumPropagationLossModel (ulSplm);
    }
  else
    {
      NS_LOG_LOGIC (this << " using a PropagationLossModel in UL");
      Ptr<PropagationLossModel> ulPlm = m_uplinkPathlossModel->GetObject<PropagationLossModel> ();
      NS_ASSERT_MSG (ulPlm != 0, " " << m_uplinkPathlossModel << " is neither PropagationLossModel nor SpectrumPropagationLossModel");
      m_uplinkChannel->AddPropagationLossModel (ulPlm);
    }
  if (!m_fadingModelType.empty ())
    {
      m_fadingModel = m_fadingModelFactory.Create<SpectrumPropagationLossModel> ();
      m_fadingModel->Initialize ();
      m_downlinkChannel->AddSpectrumPropagationLossModel (m_fadingModel);
      m_uplinkChannel->AddSpectrumPropagationLossModel (m_fadingModel);
    }

  // ObjectFactory m_DelayModelFactory;
  // Ptr<Object> m_downlinkDelayModel = m_DelayModelFactory.Create<ConstantSpeedPropagationDelayModel>();
  // m_downlinkDelayModel->Initialize();
  // Ptr<ConstantSpeedPropagationDelayModel> delayUpModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  // Ptr<ConstantSpeedPropagationDelayModel> m_delayModel = m_DelayModelFactory.Create<ConstantSpeedPropagationDelayModel> ();
  // m_delayModel->Initialize ();
  // Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
  // m_downlinkChannel->SetPropagationDelayModel (delayDownModel);
  // m_downlinkChannel->SetPropagationDelayModel (CreateObject<ConstantSpeedPropagationDelayModel> ());

}

Ptr<SpectrumChannel>
EpcGlobalHelper::GetUplinkSpectrumChannel () 
{
  // std::cout<<"EpcGlobalHelper::GetUplinkSpectrumChannel"<<std::endl;
  return m_uplinkChannel;
}

Ptr<SpectrumChannel>
EpcGlobalHelper::GetDownlinkSpectrumChannel () 
{
  return m_downlinkChannel;
}

Ptr<Object>
EpcGlobalHelper::GetDownlinkPathlossModel ()
{
  return m_downlinkPathlossModel;
};

Ptr<Object>
EpcGlobalHelper::GetUplinkPathlossModel ()
{
  return m_uplinkPathlossModel;
};

Ptr<SpectrumPropagationLossModel>
EpcGlobalHelper::GetFadingModelPointer ()
{
  return m_fadingModel;  
};

EpcGlobalHelper::~EpcGlobalHelper ()
{

}

void
EpcGlobalHelper::setup () {
}

void
EpcGlobalHelper::setBackground () {
  Ptr<Node> background = CreateObject<Node>();
  EpcGroupHelper::setNodePosition(background,100,0,10);
}

void
EpcGlobalHelper::setNumOfEpc (uint16_t num) {
  numOfEpc = num;
  for (uint16_t i = 0; i < numOfEpc; i++)
  {
    epcGroupHelper.push_back(CreateObject<EpcGroupHelper>(i));
  }
}

void
EpcGlobalHelper::setEpcGroupHelper(std::vector<Ptr<EpcGroupHelper>> ls)
{
  epcGroupHelper = ls;
};

// void
// EpcGlobalHelper::SetAppController (Ptr<AppController> aC)
// {
//   appController = aC;
// };


void
EpcGlobalHelper::attachUeWithENodeB(Ptr<Node> ueNode, uint16_t currentCellId, uint16_t currentRnti, Ptr<Node> enbNode)
{
  // std::cout<<"EpcGlobalHelper::attachUeWithENodeB:"<<currentCellId<<" "<<currentRnti<<" enbNode Id:"<< enbNode->GetId() <<std::endl;
  if (currentCellId != 0 || currentRnti !=0 ) {
    for (uint16_t i = 0; i < numOfEpc; i++) { 
      Ptr<LteEnbNetDevice> lteEnb = getEpc(i)->findEnbWithCellId(currentCellId);
      if (lteEnb){
        // getEpc(i)->detachUeNode(ueNode, currentCellId, currentRnti);
        // std::cout<<"Leave current enb:"<<lteEnb->GetCellId()<<std::endl;
        // lteEnb->GetRrc()->RemoveUe(currentRnti);
        // lteEnb->GetRrc()->GetLteEnbRrcSapProvider()->RecvIdealUeContextRemoveRequest(currentRnti);

        // Ptr<LteEnbNetDevice> lteEnb0 = getEpc(i)->getEnbLteDevs().Get(0)->GetObject<LteEnbNetDevice> ();

        //Deadactive other bearers
        // Ptr<LteUeNetDevice> ptrUeLte = getEpc(i)->getUeLte(ueNode);
        // Ptr<EpcMmeApplicationExt::UeInfo> ueInfo = getEpc(i)->getEpcHelper()->getMmeApp()->GetUe(ptrUeLte->GetImsi());
        // uint16_t bearerCounter = ueInfo->bearerCounter;
        // std::cout<<"EpcGlobalHelper::"<<bearerCounter<<std::endl;
        // for (uint16_t b = 1 ; b<bearerCounter ; b++) {
        //   lteEnb->GetRrc()->DoSendReleaseDataRadioBearer (ptrUeLte->GetImsi(),currentRnti,b);
        // }
        // std::cout<<"EpcGlobalHelper::after Remove bearers"<<bearerCounter<<std::endl;

        if (!getEpc(i)->getSendRequestViaRoaming()){
          lteEnb->GetRrc()->GetLteEnbRrcSapProvider()->RecvIdealUeContextRemoveRequest(currentRnti);
        }
        // if (sendRequestViaRoaming) {
        //   oldAddr = getEpc(i)->getCurrentIpv4Address(ueNode);
        // }

        getEpc(i)->stopRunningApp(ueNode);

        if (tapEnable) {
            std::map<Ptr<Node>, NodeTap >::iterator itue = ueTaps.find (ueNode);
            NS_ASSERT_MSG (itue != ueTaps.end (), "could not find any ue tap ");
            getEpc(i)->getEpcHelper()->getPgwApp()->AddAddrToBackwards8(itue->second.ipTap);
        }

      }
    }
    //Remove everything below
    // ueLte = lteHelper->InstallUeDevice(NodeContainer(ueNode));
    // internet.Install(NodeContainer(ueNode));
    // Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address (ueLte);
    // lteHelper->Attach (ueLte.Get(0), enbLteDevs.Get(enbIndex));
    // ueLteDevs.Add(ueLte);
    // ueIpIfaces.Add(ueIpIface);
    //=> Make ueNode totally clear
  }

  for (uint16_t i = 0; i < numOfEpc; i++) { 
    uint16_t enbIndex = getEpc(i)->checkExistEnbId(enbNode->GetId());
    if (enbIndex > 0){
      enbIndex = enbIndex - 1;//Fix it, thats stupid
      // std::cout<<"epc:"<<i<<" "<<enbIndex<<std::endl;
      getEpc(i)->attachUeNode(ueNode, enbIndex);
    }
  }
}

std::vector<Ptr<EpcGroupHelper>> 
EpcGlobalHelper::getEpcGroupHelpers()
{
  return epcGroupHelper;
}

Ptr<EpcGroupHelper>
EpcGlobalHelper::getEpc(uint16_t index)
{
  return epcGroupHelper[index];
}

Ptr<EpcGroupHelper> 
EpcGlobalHelper::getEpcConnectedUe(Ptr<Node> ueNode)
{
  // std::cout<<"EpcGlobalHelper::getEpcConnectedUe"<<std::endl;
  
  // std::cout<<ueNode->GetId()<<std::endl;
  
  // std::cout<<ueNode->GetNDevices()<<std::endl;

  int nDevs = ueNode->GetNDevices ();

  for (int j = 0; j < nDevs; j++)
  {
    Ptr<LteUeNetDevice> uedev = ueNode->GetDevice (j)->GetObject <LteUeNetDevice> ();
    // std::cout<<">j "<< j <<" imsi " << uedev->GetImsi() <<std::endl;


    for (uint16_t i = 0; i < numOfEpc; i++) { 
      Ptr<EpcGroupHelper> currentEpc = getEpc(i);

      NetDeviceContainer ueLteDevs = currentEpc->getUeLteDevs();

      uint32_t numOfDevs =  ueLteDevs.GetN();

      for (uint32_t k = 0; k < numOfDevs; k++) { 
        Ptr<Node> ck = ueLteDevs.Get(k)->GetNode();

        // std::cout<<"pass3 : "<<ck->GetId()<<std::endl;
        if (ck->GetId() == ueNode->GetId()) {
          // std::cout<<"pass3 : "<<ck->GetId()<<" choose epc "<< i <<std::endl;
          return currentEpc;
        }
      }
    }
  }


  // int nDevs = ueNode->GetNDevices ();
  // std::cout<<"> "+nDevs<<std::endl;
  // for (int j = 0; j < nDevs; j++)
  //   {
  //     std::cout<<">j "<< j <<std::endl;

  //     Ptr<LteUeNetDevice> uedev = ueNode->GetDevice (j)->GetObject <LteUeNetDevice> ();
  //     if (uedev) {
  //       std::cout<<"pass"<<std::endl;
 
  //       Ptr<LteEnbNetDevice> targetEnb = uedev->GetTargetEnb();

  //       std::cout<<"pass2"<< targetEnb->GetCellId() <<std::endl;
 
  //       uint16_t cellId = uedev->GetTargetEnb()->GetCellId();
        
  //       std::cout<<" > "<<j<<" "<<cellId<<" - "<<std::endl;

  //       for (uint16_t i = 0; i < numOfEpc; i++) { 
  //         Ptr<EpcGroupHelper> currentEpc = getEpc(i);

  //         if (currentEpc->checkExistEnbCellId(cellId)){
  //           std::cout<<"choose epc:"<<std::endl;
  //           return currentEpc;
  //         }
  //       }
  //     }
  //   }
  // std::cout<<"Error at EpcGlobalHelper::getEpcConnectedUe"<<std::endl;
  return getEpc(0);
}

void
EpcGlobalHelper::showEnbNodes()
{
  // std::cout<<"EpcGlobalHelper::showEnbNodes"<<std::endl;
  for (uint16_t i = 0; i < numOfEpc; i++) 
  { 
    std::cout<<"Epc : "<<i<<std::endl;
    getEpc(i)->showEnbNodes();
  }
};

void 
EpcGlobalHelper::doRoaming(uint16_t epcHome, uint16_t epcVisit)
{
//Roaming from epcGroupHelper[epcVisit] to epcGroupHelper[epcHome]
  uint16_t m_gtpcUdpPort = 2123;
  Ipv4AddressHelper m_s6dIpv4AddressHelper;

  std::string s6dAddr = std::to_string((epcVisit+1))+std::string(".")+std::to_string((epcHome+1))+std::string(".0.0");
  // std::cout<<"roaming:"<<epcHome<<" "<<epcVisit <<" "<<s6dAddr<<std::endl;
  m_s6dIpv4AddressHelper.SetBase (s6dAddr.c_str(), "255.255.0.0");


  Ptr<EpcMmeApplicationExt> m_mmeApp = epcGroupHelper[epcVisit]->getEpcHelper()->getMmeApp();

  Ptr<EpcHssApplication> m_hssApp = epcGroupHelper[epcHome]->getEpcHelper()->getHssApp();

  Ptr<Node> m_mme = epcGroupHelper[epcVisit]->getEpcHelper()->getMme();

  Ptr<Node> m_hss = epcGroupHelper[epcHome]->getEpcHelper()->getHss();

  DataRate m_s11LinkDataRate = DataRate ("10Gb/s");
  Time m_s6dLinkDelay = Seconds (0);
  uint16_t m_s11LinkMtu = 3000;
  PointToPointHelper s6dP2ph;
  s6dP2ph.SetDeviceAttribute ("DataRate", DataRateValue (m_s11LinkDataRate));
  s6dP2ph.SetDeviceAttribute ("Mtu", UintegerValue (m_s11LinkMtu));
  s6dP2ph.SetChannelAttribute ("Delay", TimeValue (m_s6dLinkDelay));
  NetDeviceContainer mmeHssDevices = s6dP2ph.Install (m_mme, m_hss);

  Ptr<NetDevice> mmes6dDev = mmeHssDevices.Get (0);
  Ptr<NetDevice> hsss6dDev = mmeHssDevices.Get (1);
  // m_s6dIpv4AddressHelper.NewNetwork ();
  Ipv4InterfaceContainer mmeHssIpIfaces = m_s6dIpv4AddressHelper.Assign (mmeHssDevices);

  Ipv4Address mmes6dAddress = mmeHssIpIfaces.GetAddress (0);
  Ipv4Address hsss6dAddress = mmeHssIpIfaces.GetAddress (1);

  int retval;
  // Create s6d socket in the MME
  Ptr<Socket> mmes6dSocket = Socket::CreateSocket (m_mme, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  retval = mmes6dSocket->Bind (InetSocketAddress (mmes6dAddress, m_gtpcUdpPort));
  NS_ASSERT (retval == 0);

  // Create s6d socket in the HSS
  Ptr<Socket> hsss6dSocket = Socket::CreateSocket (m_hss, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  retval = hsss6dSocket->Bind (InetSocketAddress (hsss6dAddress, m_gtpcUdpPort));
  NS_ASSERT (retval == 0);

  m_hssApp->AddMmeRoaming (mmes6dAddress, hsss6dAddress, hsss6dSocket);
  m_mmeApp->AddHssRoaming (hsss6dAddress, mmes6dSocket);

  /////////////


  // Create S8 link between PGW and SGW
  uint16_t m_gtpuUdpPort = 2152;

  Ipv4AddressHelper m_s8Ipv4AddressHelper;

  std::string s8Addr = std::string("10.") + std::to_string((epcVisit+90))+std::string(".0.0");
  // std::cout<<"roaming s8:"<<epcHome<<" "<<epcVisit <<" "<<s8Addr<<std::endl;

  m_s8Ipv4AddressHelper.SetBase (s8Addr.c_str(), "255.255.0.0");

  // m_s8Ipv4AddressHelper.NewNetwork();


  Ptr<Node> m_pgw = epcGroupHelper[epcHome]->getEpcHelper()->getPgw();

  Ptr<Node> m_sgw = epcGroupHelper[epcVisit]->getEpcHelper()->getSgw();

  PointToPointHelper s8p2ph;
  s8p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("10Gb/s")));
  s8p2ph.SetDeviceAttribute ("Mtu", UintegerValue (m_s11LinkMtu));

  // Time m_s8LinkDelay = Seconds (propagationDelay(m_pgw, m_sgw));
  Time m_s8LinkDelay = Seconds (0.02);
  std::cout<<"delay:"<<propagationDelay(m_pgw, m_sgw)<<std::endl;

  s8p2ph.SetChannelAttribute ("Delay", TimeValue (m_s8LinkDelay));
  NetDeviceContainer pgwSgwDevices = s8p2ph.Install (m_pgw, m_sgw);

  Ptr<NetDevice> pgwDev = pgwSgwDevices.Get (0);
  Ptr<NetDevice> sgwDev = pgwSgwDevices.Get (1);

  Ipv4InterfaceContainer pgwSgwIpIfaces = m_s8Ipv4AddressHelper.Assign (pgwSgwDevices);


  Ipv4Address pgwS8Address = pgwSgwIpIfaces.GetAddress (0);
  Ipv4Address sgwS8Address = pgwSgwIpIfaces.GetAddress (1);

  // Create S8 socket in the PGW
  Ptr<Socket> pgwS8Socket = Socket::CreateSocket (m_pgw, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  retval = pgwS8Socket->Bind (InetSocketAddress (pgwS8Address, m_gtpuUdpPort));
  NS_ASSERT (retval == 0);

  // Create S8 socket in the SGW
  Ptr<Socket> sgwS8Socket = Socket::CreateSocket (m_sgw, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  retval = sgwS8Socket->Bind (InetSocketAddress (sgwS8Address, m_gtpuUdpPort));
  NS_ASSERT (retval == 0);

  // Create EpcPgwApplicationExt
  Ptr<EpcPgwApplicationExt> m_pgwApp = epcGroupHelper[epcHome]->getEpcHelper()->getPgwApp();
  Ptr<EpcSgwApplicationExt> m_sgwApp = epcGroupHelper[epcVisit]->getEpcHelper()->getSgwApp();

  // m_hssApp->AddMmeRoaming (mmes6dAddress, hsss6dAddress, hsss6dSocket);
  // m_mmeApp->AddHssRoaming (hsss6dAddress, mmes6dSocket);

  m_pgwApp->AddSgwS8Roaming (sgwS8Address, pgwS8Address, pgwS8Socket);
  m_sgwApp->AddPgwS8Roaming (pgwS8Address, sgwS8Socket);
};

double 
EpcGlobalHelper::propagationDelay(Ptr<Node> a, Ptr<Node> b)
{
  Vector aPos = a->GetObject<MobilityModel> ()->GetPosition ();
  Vector bPos = b->GetObject<MobilityModel> ()->GetPosition ();
  return CalculateDistance (aPos, bPos) / (2997924);
};

void
EpcGlobalHelper::setSendRequestViaRoaming(bool m_ck)
{
  sendRequestViaRoaming = m_ck;
  for (uint16_t i = 0; i < numOfEpc; i++) 
  { 
    getEpc(i)->setSendRequestViaRoaming(m_ck);
  }
};

void
EpcGlobalHelper::setupCsmaServer(uint16_t epcIndex, std::string tapSrvAddressBase, std::string tapSrvRealAddress, std::string tapName, std::string serviceName, std::string controllerUrl)
{
  CsmaHelper csmaServer;
  csmaServer.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  csmaServer.SetChannelAttribute ("Delay", TimeValue (Seconds (0)));

  Ptr<Node> remoteHostTap = getEpc(epcIndex)->getRemoteHostContainer().Get (1);

  Names::Add ("RemoteHostTap-" + std::to_string(epcIndex), remoteHostTap);

  NetDeviceContainer internetServer = csmaServer.Install (getEpc(epcIndex)->getRemoteHostContainer());  

  Ipv4AddressHelper ipv4Server;
  // std::string csmaBase = std::to_string(10+epcIndex)+".4.0.0";
  ipv4Server.SetBase (tapSrvAddressBase.c_str(), "255.255.0.0");
  Ipv4InterfaceContainer interfaceServer = ipv4Server.Assign (internetServer);

  TapBridgeHelper tapBridgeServer;
  tapBridgeServer.SetAttribute ("Mode", StringValue ("UseBridge"));

  // std::string tapServerStr = "tap-server-"+std::to_string(epcIndex+1);

  tapBridgeServer.SetAttribute ("DeviceName", StringValue (tapName));
  tapBridgeServer.Install (remoteHostTap, internetServer.Get (1));

  // MobilityHelper tapMobility;
  // tapMobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  // tapMobility.Install (rightNodes);
  Ptr<Node> remoteHost = getEpc(epcIndex)->getRemoteHost();
  Vector remoteVec = remoteHost->GetObject<MobilityModel> ()->GetPosition();

  remoteHostTap->GetObject<MobilityModel> ()->SetPosition( Vector(remoteVec.x + 4.7, remoteVec.y - 2.0 , 0));

  NodeTap srvTap;
  srvTap.tapNode = remoteHostTap;
  srvTap.ipTap = Ipv4Address(tapSrvRealAddress.c_str());
  srvTap.ipAddrBase = Ipv4Address(tapSrvAddressBase.c_str());
  srvTap.tapName = tapName;
  srvTap.serviceName = serviceName;
  srvTap.controllerUrl = controllerUrl;
  srvTaps[epcIndex] = srvTap;

};

void
EpcGlobalHelper::prepareCsmaUe(Ptr<Node> ue, std::string tapUeAddressBase, std::string tapUeRealAddress, std::string tapName, std::string serviceName, std::string controllerUrl)
{
  // std::cout<<"EpcGlobalHelper::prepareCsmaUe"<<std::endl;
  NodeTap ueTap;

  NodeContainer ueTapNodes;
  ueTapNodes.Create (1);

  ueTap.tapNode = ueTapNodes.Get(0);

  Names::Add ("Tap" + Names::FindName(ue), ueTap.tapNode);

  ueTap.ipTap = Ipv4Address(tapUeRealAddress.c_str());
  ueTap.ipAddrBase = Ipv4Address(tapUeAddressBase.c_str());
  ueTap.tapName = tapName;
  ueTap.serviceName = serviceName;
  ueTap.controllerUrl = controllerUrl;
  ueTaps[ue] = ueTap;

  // std::cout<<"EpcGlobalHelper::prepareCsmaUe::finish"<<std::endl;

}

void
EpcGlobalHelper::setupCsmaUe(Ptr<Node> ue)
{
  // std::cout<<"EpcGlobalHelper::setupCsmaUe"<<std::endl;

  std::map<Ptr<Node>, NodeTap >::iterator itue = ueTaps.find (ue);
  NS_ASSERT_MSG (itue != ueTaps.end (), "could not find any ue tap ");

  std::string tapUeAddressBase = Ipv4AddressToString(itue->second.ipAddrBase);
  std::string tapUeRealAddress = Ipv4AddressToString(itue->second.ipTap);
  std::string tapName = itue->second.tapName;

  NodeContainer ueTapNodes = NodeContainer(itue->second.tapNode);

  InternetStackHelper internet;
  internet.Install (ueTapNodes);

  NodeContainer csmaUeTapNodes (ue,ueTapNodes.Get (0));

  CsmaHelper csmaUeTap;
  csmaUeTap.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("10Gb/s")));
  csmaUeTap.SetChannelAttribute ("Delay", TimeValue (Seconds (0)));

  NetDeviceContainer internetDevicesUeTap = csmaUeTap.Install (csmaUeTapNodes);
  
  Ipv4AddressHelper ipv4TapUe;

  ipv4TapUe.SetBase (tapUeAddressBase.c_str(), "255.255.0.0");
  
  Ipv4InterfaceContainer interfacesRight = ipv4TapUe.Assign (internetDevicesUeTap);

  TapBridgeHelper tapBridgeRight;
  tapBridgeRight.SetAttribute ("Mode", StringValue ("UseBridge"));

  // std::string tapCarStr = "tap-car-"+std::to_string(epcIndex+1);

  tapBridgeRight.SetAttribute ("DeviceName", StringValue (tapName));
  tapBridgeRight.Install (ueTapNodes.Get (0), internetDevicesUeTap.Get (1));

  MobilityHelper ueMobility;
  ueMobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  ueMobility.Install (ueTapNodes);

  Vector ueVec = ue->GetObject<MobilityModel> ()->GetPosition();
  Vector ueVelocity = ue->GetObject<ConstantVelocityMobilityModel> ()->GetVelocity();

  ueTapNodes.Get(0)->GetObject<MobilityModel> ()->SetPosition( Vector(ueVec.x + 14.5, ueVec.y, 0));
  // ueTapNodes.Get(0)->GetObject<MobilityModel> ()->SetPosition( Vector(ueVec.x + 10, ueVec.y, 0));
  ueTapNodes.Get(0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (ueVelocity);

  // std::cout<<"EpcGlobalHelper::setupCsmaUe::finish"<<std::endl;

};

void
EpcGlobalHelper::routingSrvUeTap(uint16_t epcIndex, Ptr<Node> ue)
{
  if (!tapEnable) {
    return;
  }
  
  std::map<Ptr<Node>, NodeTap >::iterator itue = ueTaps.find (ue);
  NS_ASSERT_MSG (itue != ueTaps.end (), "could not find any ue tap ");
  NodeTap ueNodeTap = itue->second;
  Ptr<Node> ueTapNode = ueNodeTap.tapNode;

  std::map<uint16_t, NodeTap >::iterator itsvr = srvTaps.find (epcIndex);
  NS_ASSERT_MSG (itsvr != srvTaps.end (), "could not find any srv tap ");
  NodeTap svrNodeTap = itsvr->second;
  Ptr<Node> remoteHostTap = svrNodeTap.tapNode;

  if (getEpc(epcIndex)->getSendRequestViaRoaming() && epcIndex > 0){

    // std::cout<<"EpcGlobalHelper::routingSrvUeTap::"<<std::endl;
    
    // Ipv4ListRouting::AddFwdPk(Ipv4AddressToString(svrNodeTap.ipTap), Ipv4AddressToString(ue->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal()) ,Ipv4AddressToString(ueNodeTap.ipTap) ,2, "PgwToUser", true);
    Ipv4ListRouting::FwdPk fwdPk = Ipv4ListRouting::FindFwdPkByName("ServerToUserViaUeDevice_"+std::to_string(ue->GetId()));
    // std::cout<<"FindFwdPkByName::"<<fwdPk.exist<<std::endl;
    
    fwdPk.destination = Ipv4AddressToString(ue->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal());

    // std::cout<<"fwdPk.destination::"<<fwdPk.destination<<std::endl;
    // std::cout<<"fwdPk.newDestination::"<<fwdPk.newDestination<<std::endl;

    if (fwdPk.exist) {
      Ipv4ListRouting::ModifyFwdPk("ServerToUserViaUeDevice_"+std::to_string(ue->GetId()), fwdPk);
    }

    getEpc(epcIndex)->getEpcHelper()->getSgwApp()->AddAddrToForwards8ForTap(ueNodeTap.ipTap);
    
    // getEpc(epcIndex)->getEpcHelper()->getSgwApp()->AddAddrToForwards8(ueNodeTap.ipTap,"11.0.0.8",0);

    //epcGroupHelper->getEpcHelper()->GetPgwNode()->GetObject<Ipv4> ()->GetAddress (3, 0).GetLocal()

    // Ipv4ListRouting::AddFwdPk("10.65.0.2", "10.65.0.1" ,"10.82.0.8","11.12.0.2" ,1, "testing", true, false);
    // Ipv4ListRouting::AddFwdPk("3.2.0.2", "3.2.0.1" ,"10.42.0.2","10.42.0.1" ,4, "testing", true, false);

    return;
  }

  /////////////////////////////////Routing for server/////

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  // remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  // interface 0 is localhost, 1 is the p2p device
  // Ipv4Address gateway = epcHelper->GetUeDefaultGatewayAddress (); 
  // remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("10.5.0.0"), Ipv4Mask ("255.255.0.0"),2);
  // remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("10.5.0.0"), Ipv4Mask ("255.255.0.0"),gateway, 2);
  // remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("10.0.5.0"), Ipv4Mask ("255.255.0.0"), 2);
  // remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("10.0.4.0"), Ipv4Mask ("255.255.0.0"), 2);

  Ptr<EpcGroupHelper> epcGroupHelper = getEpc(epcIndex);

  Ptr<Node> remoteHost = getEpc(epcIndex)->getRemoteHost();

  Ipv4Address gateway = epcGroupHelper->getEpcHelper()->GetUeDefaultGatewayAddress ();
  
  // std::cout<<"gateway::"<<gateway<<std::endl;

  Ptr<Ipv4StaticRouting> remoteHostTapStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHostTap->GetObject<Ipv4> ());
  remoteHostTapStaticRouting->AddNetworkRouteTo (Ipv4Address (epcGroupHelper->getSrvIpv4AddressBase().c_str()), Ipv4Mask ("255.255.0.0"),Ipv4Address (remoteHost->GetObject<Ipv4> ()->GetAddress (2, 0).GetLocal()), 1);
  remoteHostTapStaticRouting->AddNetworkRouteTo (Ipv4Address (epcGroupHelper->getUeIpv4AddressBase().c_str()), Ipv4Mask ("255.255.0.0"),Ipv4Address (remoteHost->GetObject<Ipv4> ()->GetAddress (2, 0).GetLocal()), 1);
  remoteHostTapStaticRouting->AddNetworkRouteTo (ueNodeTap.ipAddrBase, Ipv4Mask ("255.255.0.0"),Ipv4Address (remoteHost->GetObject<Ipv4> ()->GetAddress (2, 0).GetLocal()), 1);

  // serverTapStaticRouting->AddNetworkRouteTo (Ipv4Address (server_pgw_base), Ipv4Mask ("255.255.255.0"),Ipv4Address (server_servertap_serverIp), 1);
  // serverTapStaticRouting->AddNetworkRouteTo (Ipv4Address (pgw_ue_base), Ipv4Mask ("255.255.255.0"),Ipv4Address (server_servertap_serverIp), 1);
  // serverTapStaticRouting->AddNetworkRouteTo (Ipv4Address (ue_ueTap_base), Ipv4Mask ("255.255.255.0"),Ipv4Address (pgw_ue_ueIp), 1);

  Ptr<Ipv4StaticRouting> pgwStatic = ipv4RoutingHelper.GetStaticRouting (epcGroupHelper->getEpcHelper()->GetPgwNode()->GetObject<Ipv4> ());
  // remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.255.255.0"), 2);
  pgwStatic->AddNetworkRouteTo (svrNodeTap.ipAddrBase, Ipv4Mask ("255.255.0.0"),remoteHost->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal(), 3);
  pgwStatic->AddNetworkRouteTo (ueNodeTap.ipAddrBase, Ipv4Mask ("255.255.0.0"),ue->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal(), 1);

  // pgwStatic->AddNetworkRouteTo (Ipv4Address (tapUeAddress.c_str()), Ipv4Mask ("255.255.0.0"),Ipv4Address ("10.11.0.2"), 1);

  // pgwStatic->AddNetworkRouteTo (Ipv4Address (server_serverTap_base), Ipv4Mask ("255.255.255.0"),Ipv4Address (server_pgw_serverIp), 3);
  // pgwStatic->AddNetworkRouteTo (Ipv4Address (ue_ueTap_base), Ipv4Mask ("255.255.255.0"),Ipv4Address (pgw_ue_ueIp), 1);


  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  // remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address (epcGroupHelper->getUeIpv4AddressBase().c_str()), Ipv4Mask ("255.255.0.0"), 2);
  // remoteHostStaticRouting->AddNetworkRouteTo (svrNodeTap.ipAddrBase, Ipv4Mask ("255.255.0.0"), 2);
  remoteHostStaticRouting->AddNetworkRouteTo (ueNodeTap.ipAddrBase, Ipv4Mask ("255.255.0.0"), ue->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal(), 1);

  // pingApp(remoteHost, "10.11.0.2");

  // printRoutingTable(pgwStatic);

  // remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address (pgw_ue_base), Ipv4Mask ("255.255.255.0"), 2);
  // remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address (server_serverTap_base), Ipv4Mask ("255.255.255.0"), 1);
  // remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address (ue_ueTap_base), Ipv4Mask ("255.255.255.0"), gateway, 2);

  /////////////////////////////////Routing for ue/////

  Ptr<Ipv4StaticRouting> UeStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());

  // std::cout<<"UeStaticRouting::"<< UeStaticRouting->GetNRoutes() <<std::endl;
  // printRoutingTable(UeStaticRouting);

  // if (UeStaticRouting->GetNRoutes() == 6) {
  //   std::cout<<"Remove route";
  //   UeStaticRouting->RemoveRoute(2);
  //   UeStaticRouting->RemoveRoute(2);
  // }
  // printRoutingTable(UeStaticRouting);

  UeStaticRouting->AddNetworkRouteTo (Ipv4Address (epcGroupHelper->getSrvIpv4AddressBase().c_str()), Ipv4Mask ("255.255.0.0"),gateway, 1);
  UeStaticRouting->AddNetworkRouteTo (svrNodeTap.ipAddrBase, Ipv4Mask ("255.255.0.0"),gateway, 1);
  // UeStaticRouting->AddNetworkRouteTo (Ipv4Address("3.2.0.0"), Ipv4Mask ("255.255.0.0"),gateway, 1);
  // UeStaticRouting->AddNetworkRouteTo (Ipv4Address("10.1.0.0"), Ipv4Mask ("255.255.0.0"),gateway, 1);
  // UeStaticRouting->AddNetworkRouteTo (Ipv4Address("10.81.0.0"), Ipv4Mask ("255.255.0.0"),gateway, 1);


  // pingApp(ue, "3.2.0.1");

  // UeStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

  // UeStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

  Ptr<Ipv4StaticRouting> UeTapStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueTapNode->GetObject<Ipv4> ());
  
  // if (UeTapStaticRouting->GetNRoutes() == 3) {
  //   UeTapStaticRouting->RemoveRoute(2);
  // }

  UeTapStaticRouting->AddNetworkRouteTo (svrNodeTap.ipAddrBase, Ipv4Mask ("255.255.0.0"),ue->GetObject<Ipv4> ()->GetAddress (2, 0).GetLocal(), 1);

  // std::cout<<"UeStaticRouting::"<< UeStaticRouting->GetNRoutes() <<std::endl;
  // printRoutingTable(UeTapStaticRouting);

  /////////////////////////////////IP routing////////

  Ipv4ListRouting::AddFwdPk(Ipv4AddressToString(svrNodeTap.ipTap), Ipv4AddressToString(ueNodeTap.ipTap) , Ipv4AddressToString(ue->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal()),1, "ServerToUserViaPgw_"+std::to_string(ue->GetId()), false);

  Ipv4ListRouting::AddFwdPk(Ipv4AddressToString(svrNodeTap.ipTap), Ipv4AddressToString(ue->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal()) ,Ipv4AddressToString(ueNodeTap.ipTap) ,2, "ServerToUserViaUeDevice_"+std::to_string(ue->GetId()), true);

  ////For Proxy
  // if (uniqueHostIp != "") {
  //   if (!getEpc(epcIndex)->getSendRequestViaRoaming()) {
  //     Ipv4ListRouting::FwdPk fwdPk = Ipv4ListRouting::FindFwdPkByName("Proxy_"+std::to_string(ue->GetId()));
        
  //     fwdPk.newDestination = Ipv4AddressToString(svrNodeTap.ipTap);

  //     // std::cout<<"fwdPk.destination::"<<fwdPk.destination<<std::endl;
  //     // std::cout<<"fwdPk.newDestination::"<<fwdPk.newDestination<<std::endl;

  //     if (fwdPk.exist) {
  //       Ipv4ListRouting::ModifyFwdPk("Proxy_"+std::to_string(ue->GetId()), fwdPk);
  //     } else {
  //       Ipv4ListRouting::AddFwdPk(Ipv4AddressToString(ueNodeTap.ipTap),uniqueHostIp,Ipv4AddressToString(ueNodeTap.ipTap),Ipv4AddressToString(svrNodeTap.ipTap) ,0, "Proxy_"+std::to_string(ue->GetId()), false, false);
  //     }
  //   } else {
  //     if (epcIndex==0) {
  //       Ipv4ListRouting::AddFwdPk(Ipv4AddressToString(ueNodeTap.ipTap),uniqueHostIp,Ipv4AddressToString(ueNodeTap.ipTap),Ipv4AddressToString(svrNodeTap.ipTap) ,0, "Proxy_"+std::to_string(ue->GetId()), false, false);
  //     }
  //   }

  //   // Ipv4ListRouting::AddFwdPk(Ipv4AddressToString(svrNodeTap.ipTap), Ipv4AddressToString(ueNodeTap.ipTap) , uniqueHostIp, Ipv4AddressToString(ueNodeTap.ipTap) ,1, "ServerBackUeProxy_"+std::to_string(ue->GetId()), true, false);
  //   Ipv4ListRouting::AddFwdPk(Ipv4AddressToString(svrNodeTap.ipTap), Ipv4AddressToString(ue->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal()) , uniqueHostIp ,Ipv4AddressToString(ueNodeTap.ipTap) ,2, "ServerToUserViaUeDevice_"+std::to_string(ue->GetId()), true, true);
  //   // Ipv4ListRouting::AddFwdPk(Ipv4AddressToString(svrNodeTap.ipTap), Ipv4AddressToString(ue->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal()) ,Ipv4AddressToString(ueNodeTap.ipTap) ,2, "ServerToUserViaUeDevice_"+std::to_string(ue->GetId()), true);

  // } else {
  //   Ipv4ListRouting::AddFwdPk(Ipv4AddressToString(svrNodeTap.ipTap), Ipv4AddressToString(ue->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal()) ,Ipv4AddressToString(ueNodeTap.ipTap) ,2, "ServerToUserViaUeDevice_"+std::to_string(ue->GetId()), true);
  // }

};

std::string 
EpcGlobalHelper::Ipv4AddressToString(Ipv4Address addr)
{
  std::ostringstream oss;
  addr.Print (oss);
  return oss.str();
};


void
EpcGlobalHelper::pingApp(Ptr<Node> fromNode, std::string toAddr)
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
EpcGlobalHelper::printRoutingTable(Ptr<Ipv4StaticRouting> sr)
{
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);

  sr->PrintRoutingTable (routingStream, ns3::Time::Unit(5));
};

void
EpcGlobalHelper::setTapEnable(bool enable)
{
  tapEnable = enable;
};

// void
// EpcGlobalHelper::setUniqueHostIp(std::string ip)
// {
//   uniqueHostIp = ip;
// };

EpcGlobalHelper::NodeTap
EpcGlobalHelper::getUeNodeTap(Ptr<Node> ue)
{
  std::map<Ptr<Node>, NodeTap >::iterator itue = ueTaps.find (ue);
  NS_ASSERT_MSG (itue != ueTaps.end (), "could not find any ue tap ");
  return itue->second;
};
EpcGlobalHelper::NodeTap 
EpcGlobalHelper::getSrvNodeTap(uint16_t epcIndex)
{
  std::map<uint16_t, NodeTap >::iterator itsvr = srvTaps.find (epcIndex);
  NS_ASSERT_MSG (itsvr != srvTaps.end (), "could not find any srv tap ");
  return itsvr->second;
};

uint16_t 
EpcGlobalHelper::findEpcByEnbId(uint32_t id)
{
  for (uint16_t i = 0; i < numOfEpc; i++) { 
    uint16_t enbIndex = getEpc(i)->checkExistEnbId(id);
    if (enbIndex > 0){
      return i;
    }
  }
  
  NS_ASSERT_MSG (true, "could not find Epc By Enb Id");

  return 0;
};


} // namespace ns3