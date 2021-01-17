#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include <iostream>
#include <vector>
#include <stdio.h>
#include <iomanip>

#include "point-to-point-epc-helper-ext.h"
#include "epc-group-helper.h"
#include "epc-global-helper.h"
#include "ue-controller.h"

#include "udp-client-ext.h"

#include "udp-client-server-helper-ext.h"

// #include "node-ext.h"

#include "ns3/internet-apps-module.h"

#include "app-controller.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AppController");

NS_OBJECT_ENSURE_REGISTERED (AppController);


TypeId
AppController::GetTypeId (void)
{
  // NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_FUNCTION ("AppController::GetTypeId");

  static TypeId tid = TypeId ("ns3::AppController")
    .SetParent<Object> ()
    .SetGroupName ("EpcGroup")
    .AddConstructor<AppController>()
  ;
  return tid;
}

TypeId
AppController::GetInstanceTypeId () const
{
  return GetTypeId ();
}

AppController::AppController () 
{

}

AppController::~AppController () 
{

}

void
AppController::runOnOffApp(
  Ptr<UeController> ueController,
  Ptr<EpcGlobalHelper> epcGlobalHelper
)
{
  // uint16_t port = 9;   // Discard port (RFC 863)
  // // OnOffHelper onoff ("ns3::UdpSocketFactory", 
  // //                    Address (InetSocketAddress (ueController->getUe(0)->GetObject<Ipv4>(), port)));
  // OnOffHelper onoff ("ns3::UdpSocketFactory", 
  //                    Address (InetSocketAddress (ueController->getUe(0)->GetObject<Ipv4>(), port)));

  // onoff.SetConstantRate (DataRate ("448kb/s"));
  // ApplicationContainer apps = onoff.Install (ueController->getUe(0));
  // apps.Start (Seconds (1.0));
  // apps.Stop (Seconds (10.0));
}

// void
// AppController::runSimpleApp()
// {
  // runSimpleApp(ueController, epcGlobalHelper);  
// }

void
AppController::runSimpleApp(Ptr<Node> ue,Ptr<EpcGroupHelper> epcHelper)
{
  // std::cout<<"AppController::runSimpleApp"<<std::endl;
  // NS_LOG_INFO ("Install and start applications on UEs and remote host");


  DataRateValue dataRateValue = DataRate ("18.6Mbps");

  uint64_t bitRate = dataRateValue.Get ().GetBitRate ();

  uint32_t packetSize = 1024; //bytes

  NS_LOG_DEBUG ("bit rate " << bitRate);

  double interPacketInterval = static_cast<double> (packetSize * 8) / bitRate;

  Time udpInterval = Seconds (interPacketInterval);

  NS_LOG_DEBUG ("UDP will use application interval " << udpInterval.GetSeconds () << " sec");

  uint16_t dlPort = 10000+epcHelper->getEpcIndex();
  uint16_t ulPort = 20000+epcHelper->getEpcIndex();

  uint16_t numBearersPerUe = 1;//How many bearers per UE there are in the simulation

  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  // uint16_t numberOfUes = ueController->getUeNodes().GetN();

  // epcGlobalHelper->showEnbNodes();
  // ueController->showUes();

  // for (uint32_t u = 0; u < numberOfUes; ++u)
  //   {
      // Ptr<Node> ue = ueNodes.Get (u);
  // Ptr<Node> ue = ueController->getUe(0);
  // Set the default gateway for the UE

  // Ptr<EpcGroupHelper> epcHelper = epcGlobalHelper->getEpcConnectedUe(ue);
  
  // Ptr<Node> ueTap = epcHelper->getTapNode(ue);

  // std::cout<<"Remote host:"<<epcHelper->getRemoteHost()<<std::endl;

  Ptr<LteHelper> lteHelper = epcHelper->getLteHelper();

  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
  ueStaticRouting->SetDefaultRoute (epcHelper->getEpcHelper()->GetUeDefaultGatewayAddress (), 1);

  // std::cout<<"app-controller pass4"<<std::endl;
  
  for (uint32_t b = 0; b < numBearersPerUe; ++b)
    {

      ApplicationContainer ulClientApps;
      ApplicationContainer ulServerApps;
      ApplicationContainer dlClientApps;
      ApplicationContainer dlServerApps;

      ++dlPort;
      ++ulPort;

      // NS_LOG_LOGIC ("installing UDP DL app for UE " << u + 1);

      // std::cout<<"app-controller pass5"<<std::endl;

      UdpClientHelperExt dlClientHelper (epcHelper->getAddressOfUe(ue), dlPort);

      // std::cout<<"app-controller pass6"<<std::endl;

      dlClientHelper.SetAttribute ("Interval", TimeValue (udpInterval));
      dlClientHelper.SetAttribute ("PacketSize", UintegerValue (packetSize));
      dlClientHelper.SetAttribute ("MaxPackets", UintegerValue (1000000));
      dlClientApps.Add (dlClientHelper.Install (epcHelper->getRemoteHost()));

      // PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
      // dlServerApps.Add (dlPacketSinkHelper.Install (ue));

      // NS_LOG_LOGIC ("installing UDP UL app for UE " << u + 1);
      UdpClientHelperExt ulClientHelper (epcHelper->getRemoteAddress(), ulPort);
      ulClientHelper.SetAttribute ("Interval", TimeValue (udpInterval));
      dlClientHelper.SetAttribute ("PacketSize", UintegerValue (packetSize));
      ulClientHelper.SetAttribute ("MaxPackets", UintegerValue (1000000));
      ulClientApps.Add (ulClientHelper.Install (ue));

      // PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
      // ulServerApps.Add (ulPacketSinkHelper.Install (epcHelper->getRemoteHost()));

      Ptr<EpcTft> tft = Create<EpcTft> ();
      EpcTft::PacketFilter dlpf;
      dlpf.localPortStart = dlPort;
      dlpf.localPortEnd = dlPort;
      tft->Add (dlpf);
      EpcTft::PacketFilter ulpf;
      ulpf.remotePortStart = ulPort;
      ulpf.remotePortEnd = ulPort;
      tft->Add (ulpf);
      EpsBearer bearer (EpsBearer::NGBR_IMS);

      // uint8_t bearerId =
      lteHelper->ActivateDedicatedEpsBearer (epcHelper->getUeLteDevs().Get (epcHelper->getRealIndexOfUe(ue)), bearer, tft);

      // dlServerApps.Start (Simulator::Now() + Seconds(0.27));
      // dlClientApps.Start (Simulator::Now() + Seconds(0.27));
      // ulServerApps.Start (Simulator::Now() + Seconds(0.27));
      // ulClientApps.Start (Simulator::Now() + Seconds(0.27));
      // dlServerApps.Start (Simulator::Now());
      // dlClientApps.Start (Simulator::Now());
      // ulServerApps.Start (Simulator::Now());
      // ulClientApps.Start (Simulator::Now());
      dlServerApps.Start (Seconds(0.27));
      dlClientApps.Start (Seconds(0.27));
      ulServerApps.Start (Seconds(0.27));
      ulClientApps.Start (Seconds(0.27));
    } // end for b
    // }  
    std::cout<<"AppController::runSimpleApp::finish"<<std::endl;

};


void
AppController::runSimpleApp(Ptr<UeController> ueController,Ptr<EpcGlobalHelper> epcGlobalHelper)
{
  // std::cout<<"AppController::runSimpleApp"<<std::endl;
  // NS_LOG_INFO ("Install and start applications on UEs and remote host");


  DataRateValue dataRateValue = DataRate ("18.6Mbps");

  uint64_t bitRate = dataRateValue.Get ().GetBitRate ();

  uint32_t packetSize = 1024; //bytes

  NS_LOG_DEBUG ("bit rate " << bitRate);

  double interPacketInterval = static_cast<double> (packetSize * 8) / bitRate;

  Time udpInterval = Seconds (interPacketInterval);

  NS_LOG_DEBUG ("UDP will use application interval " << udpInterval.GetSeconds () << " sec");

  uint16_t dlPort = 10000;
  uint16_t ulPort = 20000;

  uint16_t numBearersPerUe = 1;//How many bearers per UE there are in the simulation

  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  // uint16_t numberOfUes = ueController->getUeNodes().GetN();

  // epcGlobalHelper->showEnbNodes();
  // ueController->showUes();

  // for (uint32_t u = 0; u < numberOfUes; ++u)
  //   {
      // Ptr<Node> ue = ueNodes.Get (u);
  Ptr<Node> ue = ueController->getUe(0);
  // Set the default gateway for the UE

  Ptr<EpcGroupHelper> epcHelper = epcGlobalHelper->getEpcConnectedUe(ue);
  
  // Ptr<Node> ueTap = epcHelper->getTapNode(ue);

  // std::cout<<"Remote host:"<<epcHelper->getRemoteHost()<<std::endl;

  Ptr<LteHelper> lteHelper = epcHelper->getLteHelper();

  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
  ueStaticRouting->SetDefaultRoute (epcHelper->getEpcHelper()->GetUeDefaultGatewayAddress (), 1);

  // std::cout<<"app-controller pass4"<<std::endl;
  
  for (uint32_t b = 0; b < numBearersPerUe; ++b)
    {

      ApplicationContainer ulClientApps;
      ApplicationContainer ulServerApps;
      ApplicationContainer dlClientApps;
      ApplicationContainer dlServerApps;

      ++dlPort;
      ++ulPort;

      // NS_LOG_LOGIC ("installing UDP DL app for UE " << u + 1);

      // std::cout<<"app-controller pass5"<<std::endl;

      UdpClientHelperExt dlClientHelper (epcHelper->getAddressOfUe(ue), dlPort);

      // std::cout<<"app-controller pass6"<<std::endl;

      dlClientHelper.SetAttribute ("Interval", TimeValue (udpInterval));
      dlClientHelper.SetAttribute ("PacketSize", UintegerValue (packetSize));
      dlClientHelper.SetAttribute ("MaxPackets", UintegerValue (1000000));
      dlClientApps.Add (dlClientHelper.Install (epcHelper->getRemoteHost()));

      // PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
      // dlServerApps.Add (dlPacketSinkHelper.Install (ue));

      // NS_LOG_LOGIC ("installing UDP UL app for UE " << u + 1);
      UdpClientHelperExt ulClientHelper (epcHelper->getRemoteAddress(), ulPort);
      ulClientHelper.SetAttribute ("Interval", TimeValue (udpInterval));
      dlClientHelper.SetAttribute ("PacketSize", UintegerValue (packetSize));
      ulClientHelper.SetAttribute ("MaxPackets", UintegerValue (1000000));
      ulClientApps.Add (ulClientHelper.Install (ue));

      // PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
      // ulServerApps.Add (ulPacketSinkHelper.Install (epcHelper->getRemoteHost()));

      Ptr<EpcTft> tft = Create<EpcTft> ();
      EpcTft::PacketFilter dlpf;
      dlpf.localPortStart = dlPort;
      dlpf.localPortEnd = dlPort;
      tft->Add (dlpf);
      EpcTft::PacketFilter ulpf;
      ulpf.remotePortStart = ulPort;
      ulpf.remotePortEnd = ulPort;
      tft->Add (ulpf);
      EpsBearer bearer (EpsBearer::NGBR_IMS);

      // uint8_t bearerId =
      lteHelper->ActivateDedicatedEpsBearer (epcHelper->getUeLteDevs().Get (epcHelper->getRealIndexOfUe(ue)), bearer, tft);

      dlServerApps.Start (Simulator::Now() + Seconds(0.1));
      dlClientApps.Start (Simulator::Now() + Seconds(0.1));
      ulServerApps.Start (Simulator::Now() + Seconds(0.1));
      ulClientApps.Start (Simulator::Now() + Seconds(0.1));
    } // end for b
    // }
};

void
AppController::stopSimpleApp(Ptr<EpcGroupHelper> epcHelper,Ptr<Node> ueNode)
{
  // std::cout<<"AppController::stopSimpleApp2:"<< ueNode->GetNApplications()<<" "<<epcHelper->getRemoteHost()->GetNApplications()<<" "<<Simulator::Now() <<std::endl;
  // std::cout<<"AppController::stopSimpleApp2:"<< ueNode->GetNApplications()<<std::endl;//<<" "<<epcHelper->getRemoteHost()->GetNApplications()<<" "<< Simulator::Now() <<std::endl;

  ueNode->GetApplication(0)->GetObject<UdpClientExt>()->StopAppNow();
  epcHelper->getRemoteHost()->GetApplication(0)->GetObject<UdpClientExt>()->StopAppNow();

}


void
AppController::printRoutingTable(Ptr<Ipv4StaticRouting> sr)
{
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);

  sr->PrintRoutingTable (routingStream, ns3::Time::Unit(5));
};

void
AppController::pingApp(Ptr<Node> fromNode, std::string toAddr)
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

} // namespace ns3