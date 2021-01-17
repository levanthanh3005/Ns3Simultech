/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Fraunhofer ESK
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Vignesh Babu <ns3-dev@esk.fraunhofer.de>
 */

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
#include "lte-reporter.h"
#include "app-controller.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LenaRadioLinkFailure");

//Global values to check the simulation
//behavior during and after the simulation.
uint16_t counterN310FirsteNB = 0;
Time t310StartTimeFirstEnb = Seconds (0);
uint32_t ByteCounter = 0;
uint32_t oldByteCounter = 0;

Ptr<EpcGlobalHelper> epcGlobalHelper;
Ptr<UeController> ueController;
Ptr<AppController> appController;

bool useRealDevice = true;
bool sendRequestViaRoaming = true;
bool needMigration = false;


void NotifyMibReceived(uint64_t m_imsi, uint16_t m_cellId, uint16_t m_rnti, uint16_t cellId){
  std::cout<<"NotifyMibReceived:"<<m_imsi<<" "<<m_cellId<<" "<<m_rnti<<" "<<cellId<<std::endl;
}

void
EpcGroupUeConnect (uint16_t epcIndex,Ptr<Node> ueNode)
{
  std::cout<<"EpcGroupUeConnect:"<<epcIndex<<" "<<ueNode->GetId()<<std::endl;
  if (!useRealDevice) {
    // if (epcIndex==0){
    appController->runSimpleApp(ueNode, epcGlobalHelper->getEpc(epcIndex));
    // }
    // if (!sendRequestViaRoaming) {
    //   appController->runSimpleApp(ueNode, epcGlobalHelper->getEpc(epcIndex));
    // }
    // appController->pingApp(ueNode, "10.1.0.2");
  } else {
    epcGlobalHelper->routingSrvUeTap(epcIndex, ueNode);

    EpcGlobalHelper::NodeTap ueTap = epcGlobalHelper->getUeNodeTap(ueNode);
    std::string ueTapServiceName = ueTap.serviceName;
    std::string ueTapController = ueTap.controllerUrl;

    EpcGlobalHelper::NodeTap epcTap = epcGlobalHelper->getSrvNodeTap(epcIndex);
    std::string epcServiceName = epcTap.serviceName;

    LteReporter::routing(epcServiceName, epcIndex);

    // LteReporter::sshDocker(ueTapServiceName,"http://localhost:3003/startservice");
    LteReporter::runCurl(ueTapController+"/resumeservice");

  }
}

// void
// EpcGroupStopRealService (uint16_t epcIndex,Ptr<Node> ueNode)
// {
//   // std::cout<<"EpcGroupUeConnect:"<<epcIndex<<" "<<ueNode->GetId()<<std::endl;
//   if (!useRealDevice) {

//   }
// }

void
EpcGroupSetupCsmaUe (Ptr<Node> ueNode)
{
  // std::cout<<"EpcGroupSetupCsmaUe:"<<ueNode->GetId()<<std::endl;
  // appController->runSimpleApp(ueNode, epcGlobalHelper->getEpc(epcIndex));
  // appController->pingApp(ueNode, "10.1.0.2");
  epcGlobalHelper->setupCsmaUe(ueNode);
}

void
EpcGroupStopRunningApp (uint16_t epcIndex,Ptr<Node> ueNode)
{
  // std::cout<<"EpcGroupStopRunningApp:"<<epcIndex<<" "<<ueNode->GetId()<<std::endl;
  if (!useRealDevice) {
    appController->stopSimpleApp(epcGlobalHelper->getEpc(epcIndex),ueNode);
  } else {
    if (!sendRequestViaRoaming) {
      EpcGlobalHelper::NodeTap ueTap = epcGlobalHelper->getUeNodeTap(ueNode);
      std::string ueTapServiceName = ueTap.serviceName;
      std::string ueTapController = ueTap.controllerUrl;

      EpcGlobalHelper::NodeTap srvTap = epcGlobalHelper->getSrvNodeTap(epcIndex);
      std::string srvTapController = srvTap.controllerUrl;

      // LteReporter::sshDocker(ueTapServiceName,"http://localhost:3003/stopservice");
      // LteReporter::runCurl(srvTapController+"/stopservice/car-1");
      LteReporter::runCurl(ueTapController+"/stopservice/local");
      //Do       LteReporter::runCurl(srvTapController+"/stopservice/car-1");
    }
  }

}

void
UeReconnect (std::string msg)
{
  // std::cout<<"Need to Reconnnect----------"<<std::endl;
}

void
NotifyAttachUeToEnb (Ptr<Node> ueNode, uint16_t currentCellId,uint16_t currentRnti,Ptr<Node> enbNode, double distance)
{
  // std::cout //<< context
  //           << " Attach UE " << ueNode->GetId()
  //           << " From Enb: "<<currentCellId
  //           << " With rnti: "<<currentRnti
  //           << " To eNodeB " << enbNode->GetId()
  //           << " with distance  " << distance
  //           << std::endl;
  epcGlobalHelper->attachUeWithENodeB(ueNode,currentCellId,currentRnti, enbNode);
}

 //imsi,    cellId,    rnti,    rsrq,     rsrp,   , distance, needMigration? 
void
ReportUeStatus (Ptr<Node> ue, Ptr<Node> enb, uint16_t rsrq, uint16_t rsrp, double distance, bool passingBorder, Ptr<Node> closestEnb, double rate)
{
  // std::cout //<< context
  //           << " Attach UE " << ueNode->GetId()
  //           << " From Enb: "<<currentCellId
  //           << " With rnti: "<<currentRnti
  //           << " To eNodeB " << enbNode->GetId()
  //           << " with distance  " << distance
  //           << std::endl;
  // epcGlobalHelper->attachUeWithENodeB(ueNode,currentCellId,currentRnti, enbNode);
  // std::cout<<"report"<<std::endl;
 
  uint16_t epcIndex = epcGlobalHelper->findEpcByEnbId(enb->GetId());
  
  EpcGlobalHelper::NodeTap ueTap = epcGlobalHelper->getUeNodeTap(ue);
  EpcGlobalHelper::NodeTap srvTap = epcGlobalHelper->getSrvNodeTap(epcIndex);

  std::string ueLocalIp = epcGlobalHelper->Ipv4AddressToString(epcGlobalHelper->getEpc(epcIndex)->getCurrentIpv4Address(ue));

  std::string ueTapServiceName = ueTap.serviceName;
  std::string ueTapController = ueTap.controllerUrl;

  std::string svrTapServiceName = srvTap.serviceName;
  std::string srvTapController = srvTap.controllerUrl;

  std::string strToUe;
  std::string srvTapIp = epcGlobalHelper->Ipv4AddressToString(srvTap.ipTap);

  // 0: run as normal: no passing border
  // 1: passing border and doRoaming = case 0
  // 2: passing border and no migration
  // 3: passing border and migration

  if (!passingBorder) {

    strToUe = ueLocalIp+"/"+Names::FindName(enb)+"/"+ svrTapServiceName+"/"+std::to_string(rsrq)+"/"+std::to_string(rsrp)+"/"+std::to_string(distance)+"/0/"+svrTapServiceName+"/"+srvTapIp;
  } else {
    if (sendRequestViaRoaming) {
      strToUe = ueLocalIp+"/"+Names::FindName(enb)+"/"+ svrTapServiceName+"/"+std::to_string(rsrq)+"/"+std::to_string(rsrp)+"/"+std::to_string(distance)+"/1/"+svrTapServiceName+"/"+srvTapIp;
    } else {
        
      uint16_t epcNewIndex = epcGlobalHelper->findEpcByEnbId(closestEnb->GetId());
      EpcGlobalHelper::NodeTap srvNewTap = epcGlobalHelper->getSrvNodeTap(epcNewIndex);
      std::string svrNewTapServiceName = srvNewTap.serviceName;
      std::string srvNewTapIp = epcGlobalHelper->Ipv4AddressToString(srvNewTap.ipTap);
      std::string srvNewTapController = srvNewTap.controllerUrl;

      // std::cout<<"Rate:"<<rate<<std::endl;

      // if (rate <-0.20) {
      //   LteReporter::runCurl(ueTapController+"/stopservice");
      // }

      if (needMigration) {
        strToUe = ueLocalIp+"/"+Names::FindName(enb)+"/"+ svrTapServiceName+"/"+std::to_string(rsrq)+"/"+std::to_string(rsrp)+"/"+std::to_string(distance)+"/3/"+svrNewTapServiceName+"/"+srvNewTapIp;

        LteReporter::runCurl(srvTapController+"/migration/"+"user1"+"/"+srvNewTapController);

      } else {
        strToUe = ueLocalIp+"/"+Names::FindName(enb)+"/"+ svrTapServiceName+"/"+std::to_string(rsrq)+"/"+std::to_string(rsrp)+"/"+std::to_string(distance)+"/2/"+svrNewTapServiceName+"/"+srvNewTapIp;       
        LteReporter::runCurl(srvTapController+"/stopallservice/"+"user1");
      }

    }
  }

  // LteReporter::sshDocker(ueTapServiceName,"http://localhost:3003/uereport/"+strToUe);
  LteReporter::runCurl(ueTapController+"/uereport/"+strToUe);

  // this is for Trust system
  // Vector ueVelocity = ueController->getUeVelocity(ue);
  // Vector uePosition = ueController->getUePosition(ue);

  // std::string ueVelocityStr = "/uevelocity/user1/"+std::to_string(ueVelocity.x)+"/"+std::to_string(ueVelocity.y)+"/"+std::to_string(ueVelocity.z)+"/"+std::to_string(uePosition.x)+"/"+std::to_string(uePosition.y)+"/"+std::to_string(uePosition.z);
  // std::string rsvl = LteReporter::exec("curl "+srvTapController + ueVelocityStr);
  // std::cout<<"after ping velocity:"<<rsvl<<std::endl;
  // if (rsvl.compare("local")==0) {
  //   sendRequestViaRoaming = false;
  //   epcGlobalHelper->setSendRequestViaRoaming(sendRequestViaRoaming);
  // } else {
  //   sendRequestViaRoaming = true;
  //   epcGlobalHelper->setSendRequestViaRoaming(sendRequestViaRoaming);
  // }
}


void
NotifyUeManagerFinishSetup ()
{
  // std::cout << "Ue Controller Finish Setup"
            // << std::endl;

  // appController->setUeController(ueController);
  // appController->setEpcGlobalHelper(epcGlobalHelper);
  // appController->runSimpleApp();
  // appController->runSimpleApp();
  // epcGlobalHelper->showEnbNodes();
  // ueController->showUes();
}


void
NotifyHandoverStartUe (std::string context,
                       uint64_t imsi,
                       uint16_t cellid,
                       uint16_t rnti,
                       uint16_t targetCellId)
{
  std::cout << context
            << " UE IMSI " << imsi
            << ": previously connected to CellId " << cellid
            << " with RNTI " << rnti
            << ", doing handover to CellId " << targetCellId
            << std::endl;
}

void
NotifyHandoverEndOkUe (std::string context,
                       uint64_t imsi,
                       uint16_t cellid,
                       uint16_t rnti)
{
  std::cout << context
            << " UE IMSI " << imsi
            << ": successful handover to CellId " << cellid
            << " with RNTI " << rnti
            << std::endl;
}

void
NotifyHandoverStartEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellid,
                        uint16_t rnti,
                        uint16_t targetCellId)
{
  std::cout << context
            << " eNB CellId " << cellid
            << ": start handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << " to CellId " << targetCellId
            << std::endl;
}

void
NotifyHandoverEndOkEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellid,
                        uint16_t rnti)
{
  std::cout << context
            << " eNB CellId " << cellid
            << ": completed handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << std::endl;
}

void
PrintUePosition (uint64_t imsi)
{

  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteUeNetDevice> uedev = node->GetDevice (j)->GetObject <LteUeNetDevice> ();
          if (uedev)
            {
              if (imsi == uedev->GetImsi ())
                {
                  Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
                  std::cout << "IMSI : " << uedev->GetImsi () << " at " << pos.x << "," << pos.y << std::endl;
                }
            }
        }
    }
}

void
NotifyRecvMeasurementReport (
                               uint64_t imsi,
                               uint16_t cellid,
                               uint16_t rnti,
                               ns3::LteRrcSap::MeasurementReport msg)
{
  // std::cout << Simulator::Now ().GetSeconds () << " " 
  //           << " UE IMSI " << imsi
  //           << ": connected to cell id " << cellid
  //           << " with RNTI " << rnti
  //           << " RSRP:"<<(uint16_t) msg.measResults.rsrpResult
  //           << " RSRQ:"<<(uint16_t) msg.measResults.rsrqResult
  //           << std::endl;
  ueController->refresh(imsi, cellid, rnti, (uint16_t) msg.measResults.rsrpResult, (uint16_t) msg.measResults.rsrqResult);
}

void
NotifyConnectionEstablishedUe (std::string context,
                               uint64_t imsi,
                               uint16_t cellid,
                               uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " UE IMSI " << imsi
            << ": connected to cell id " << cellid
            << " with RNTI " << rnti
            << std::endl;
}

void
NotifyConnectionEstablishedEnb (std::string context,
                                uint64_t imsi,
                                uint16_t cellId,
                                uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " eNB cell id " << cellId
            << ": successful connection of UE with IMSI " << imsi
            << " RNTI " << rnti
            << std::endl;
  //In this example, a UE should experience RLF at least one time in
  //cell 1. For the case, when there is only one eNB with ideal RRC,
  //a UE might reconnects to the eNB multiple times due to more than
  //one RLF. To handle this, we reset the counter here so, even if the UE
  //connects multiple time to cell 1 we count N310
  //indication correctly, i.e., for each RLF UE RRC should receive
  //configured number of N310 indications.
  if (cellId == 1)
    {
      counterN310FirsteNB = 0;
    }
}

/// Map each of UE RRC states to its string representation.
static const std::string g_ueRrcStateName[LteUeRrc::NUM_STATES] =
{
  "IDLE_START",
  "IDLE_CELL_SEARCH",
  "IDLE_WAIT_MIB_SIB1",
  "IDLE_WAIT_MIB",
  "IDLE_WAIT_SIB1",
  "IDLE_CAMPED_NORMALLY",
  "IDLE_WAIT_SIB2",
  "IDLE_RANDOM_ACCESS",
  "IDLE_CONNECTING",
  "CONNECTED_NORMALLY",
  "CONNECTED_HANDOVER",
  "CONNECTED_PHY_PROBLEM",
  "CONNECTED_REESTABLISHING"
};

/**
 * \param s The UE RRC state.
 * \return The string representation of the given state.
 */
static const std::string & ToString (LteUeRrc::State s)
{
  return g_ueRrcStateName[s];
}

void
UeStateTransition (uint64_t imsi, uint16_t cellId, uint16_t rnti, LteUeRrc::State oldState, LteUeRrc::State newState)
{
  std::cout << Simulator::Now ().GetSeconds ()
            << " UE with IMSI " << imsi << " RNTI " << rnti << " connected to cell " << cellId <<
  " transitions from " << ToString (oldState) << " to " << ToString (newState) << std::endl;
}

void
EnbRrcTimeout (uint64_t imsi, uint16_t rnti, uint16_t cellId, std::string cause)
{
  std::cout << Simulator::Now ().GetSeconds ()
            << " IMSI " << imsi << ", RNTI " << rnti << ", Cell id " << cellId
            << ", ENB RRC " << cause << std::endl;
}

void
NotifyConnectionReleaseAtEnodeB (uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  std::cout << Simulator::Now ()
            << " IMSI " << imsi << ", RNTI " << rnti << ", Cell id " << cellId
            << ", UE context destroyed at eNodeB" << std::endl;
}

void PhySyncDetection (uint16_t n310, uint64_t imsi, uint16_t rnti, uint16_t cellId, std::string type, uint8_t count)
{
  std::cout << Simulator::Now ().GetSeconds ()
            << " IMSI " << imsi << ", RNTI " << rnti
            << ", Cell id " << cellId << ", " << type << ", no of sync indications: " << +count
            << std::endl;

  if (type == "Notify out of sync" && cellId == 1)
    {
      ++counterN310FirsteNB;
      if (counterN310FirsteNB == n310)
        {
          t310StartTimeFirstEnb = Simulator::Now ();
        }
      NS_LOG_DEBUG ("counterN310FirsteNB = " << counterN310FirsteNB);
    }
}

void RadioLinkFailure (Time t310, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  std::cout << Simulator::Now ()
            << " IMSI " << imsi << ", RNTI " << rnti
            << ", Cell id " << cellId << ", radio link failure detected"
            << std::endl << std::endl;

  PrintUePosition (imsi);

//   for (uint16_t i = 0; i < numOfEpc; i++)
//   {
//     std::cout<<"Check epc :"<<i<<" "<<epcGlobalHelper->getEpc(i)->checkExistEnbCellId(cellId)<<std::endl;
//   }
// //////////////////
// for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
//     {
//       Ptr<Node> node = *it;
//       int nDevs = node->GetNDevices ();
//       for (int j = 0; j < nDevs; j++)
//         {
//           Ptr<LteEnbNetDevice> enbdev = node->GetDevice (j)->GetObject <LteEnbNetDevice> ();
//           if (enbdev)
//             {
//               std::cout<<"---"<<node->GetId()<<" "<<j<<" "<<enbdev->GetCellId()<<std::endl;
//             }
//         }
//     }
// ///////////////////

//   if (cellId == 1)
//     {
//       NS_ABORT_MSG_IF ((Simulator::Now () - t310StartTimeFirstEnb) != t310, "T310 timer expired at wrong time");
//     }
}

void
NotifyRandomAccessErrorUe (uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds ()
            << " IMSI " << imsi << ", RNTI " << rnti << ", Cell id " << cellId
            << ", UE RRC Random access Failed" << std::endl;
}

void
NotifyConnectionTimeoutUe (uint64_t imsi, uint16_t cellId, uint16_t rnti,
                           uint8_t connEstFailCount)
{
  std::cout << Simulator::Now ().GetSeconds ()
            << " IMSI " << imsi << ", RNTI " << rnti
            << ", Cell id " << cellId
            << ", T300 expiration counter " << (uint16_t) connEstFailCount
            << ", UE RRC Connection timeout" << std::endl;
}

void
NotifyRaResponseTimeoutUe (uint64_t imsi, bool contention,
                           uint8_t preambleTxCounter,
                           uint8_t maxPreambleTxLimit)
{
  std::cout << Simulator::Now ().GetSeconds ()
            << " IMSI " << imsi << ", Contention flag " << contention
            << ", preamble Tx Counter " << (uint16_t) preambleTxCounter
            << ", Max Preamble Tx Limit " << (uint16_t) maxPreambleTxLimit
            << ", UE RA response timeout" << std::endl;
  NS_FATAL_ERROR ("NotifyRaResponseTimeoutUe");
}

void
ReceivePacket (Ptr<const Packet> packet, const Address &)
{
  ByteCounter += packet->GetSize ();
  // std::cout<<"Packet"<<std::endl;
}

void
Throughput(bool firstWrite, Time binSize, std::string fileName)
{
  std::ofstream output;

  if (firstWrite == true)
    {
      output.open (fileName.c_str (), std::ofstream::out);
      firstWrite = false;
    }
  else
    {
      output.open (fileName.c_str (), std::ofstream::app);
    }

  //Instantaneous throughput every 200 ms
  double  throughput = (ByteCounter - oldByteCounter)*8/binSize.GetSeconds ()/1024/1024;
  output << Simulator::Now().GetSeconds() << " " << throughput << std::endl;
  oldByteCounter = ByteCounter;
  Simulator::Schedule (binSize, &Throughput, firstWrite, binSize, fileName);
}

/**
 * Sample simulation script for radio link failure.
 * By default, only one eNodeB and one UE is considered for verifying
 * radio link failure. The UE is initially in the coverage of
 * eNodeB and a RRC connection gets established.
 * As the UE moves away from the eNodeB, the signal degrades
 * and out-of-sync indications are counted. When the T310 timer
 * expires, radio link is considered to have failed and UE
 * leaves the CONNECTED_NORMALLY state and performs cell
 * selection again.
 *
 * The example can be run as follows:
 *
 * ./waf --run "lena-radio-link-failure --numberOfEnbs=1 --simTime=25"
 */
int
main (int argc, char *argv[])
{
  // Configurable parameters
  Time simTime = Seconds (300000);
  uint16_t numberOfEnbs = 1;
  double interSiteDistance = 1200;
  uint16_t n311 = 1;
  uint16_t n310 = 1;
  Time t310 = Seconds (1);
  bool useIdealRrc = true;
  bool enableCtrlErrorModel = true;
  bool enableDataErrorModel = true;
  bool enableNsLogs = false;

  CommandLine cmd;
  cmd.AddValue ("simTime", "Total duration of the simulation (in seconds)", simTime);
  cmd.AddValue ("numberOfEnbs", "Number of eNBs", numberOfEnbs);
  cmd.AddValue ("n311", "Number of in-synch indication", n311);
  cmd.AddValue ("n310", "Number of out-of-synch indication", n310);
  cmd.AddValue ("t310", "Timer for detecting the Radio link failure (in seconds)", t310);
  cmd.AddValue ("interSiteDistance", "Inter-site distance in meter", interSiteDistance);
  cmd.AddValue ("useIdealRrc", "Use ideal RRC protocol", useIdealRrc);
  cmd.AddValue ("enableCtrlErrorModel", "Enable control error model", enableCtrlErrorModel);
  cmd.AddValue ("enableDataErrorModel", "Enable data error model", enableDataErrorModel);
  cmd.AddValue ("enableNsLogs", "Enable ns-3 logging (debug builds)", enableNsLogs);
  cmd.Parse (argc, argv);

  // if (enableNsLogs)
  //   {
  //     LogLevel logLevel = (LogLevel) (LOG_PREFIX_FUNC | LOG_PREFIX_NODE | LOG_PREFIX_TIME | LOG_LEVEL_ALL);
  //     LogComponentEnable ("LteUeRrc", logLevel);
  //     LogComponentEnable ("LteUeMac", logLevel);
  //     LogComponentEnable ("LteUePhy", logLevel);

  //     LogComponentEnable ("LteEnbRrc", logLevel);
  //     LogComponentEnable ("LteEnbMac", logLevel);
  //     LogComponentEnable ("LteEnbPhy", logLevel);

  //     LogComponentEnable ("LenaRadioLinkFailure", logLevel);
  //   }

  Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (useIdealRrc));
  Config::SetDefault ("ns3::LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (enableCtrlErrorModel));
  Config::SetDefault ("ns3::LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (enableDataErrorModel));

  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (6000 * 1024));

  
  //----power related (equal for all base stations)----
  double eNodeB_txPower = 43;

  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (eNodeB_txPower));
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (23));
  Config::SetDefault ("ns3::LteUePhy::NoiseFigure", DoubleValue (0));
  Config::SetDefault ("ns3::LteEnbPhy::NoiseFigure", DoubleValue (0));
  Config::SetDefault ("ns3::LteUePhy::EnableUplinkPowerControl", BooleanValue (true));
  Config::SetDefault ("ns3::LteUePowerControl::ClosedLoop", BooleanValue (true));
  Config::SetDefault ("ns3::LteUePowerControl::AccumulationEnabled", BooleanValue (true));

 
  Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::PiroEW2010));
  Config::SetDefault ("ns3::LteAmc::Ber", DoubleValue (0.01));
  Config::SetDefault ("ns3::PfFfMacScheduler::HarqEnabled", BooleanValue (true));

  // Config::SetDefault ("ns3::FfMacScheduler::UlCqiFilter", EnumValue (FfMacScheduler::SRS_UL_CQI));

  //Radio link failure detection parameters
  Config::SetDefault ("ns3::LteUeRrc::N310", UintegerValue (n310));
  Config::SetDefault ("ns3::LteUeRrc::N311", UintegerValue (n311));
  Config::SetDefault ("ns3::LteUeRrc::T310", TimeValue (t310));

  // GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
  // GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

  NS_LOG_INFO ("Create the internet");

  // bool tapEnable = true; 
  sendRequestViaRoaming = true;  
  needMigration = false;
  useRealDevice = true;
  double ueSpeed = 0;
  uint16_t ueStartPoint = 0;
  bool ckReportUeStatus = useRealDevice;//needed when use real device

  uint16_t numOfEpc = 2;

  epcGlobalHelper = CreateObject<EpcGlobalHelper>();

  epcGlobalHelper->setNumOfEpc(numOfEpc);

  epcGlobalHelper->ChannelModelInitialization();
  epcGlobalHelper->setTapEnable(useRealDevice);
  // epcGlobalHelper->setUniqueHostIp("8.8.8.8");
  // epcGlobalHelper->setUniqueHostIp("8.8.8.8");

  uint16_t enbDis = 200;

  for (uint16_t i = 0; i < numOfEpc; i++)
  {
    epcGlobalHelper->getEpc(i)->setIpWithIndex(i+1);
    epcGlobalHelper->getEpc(i)->setTapEnable(useRealDevice);
    epcGlobalHelper->getEpc(i)->setPosition(i*enbDis*2.4,30);
    epcGlobalHelper->getEpc(i)->setNumberOfEnbs(i+2);
    epcGlobalHelper->getEpc(i)->setEnbDistance(enbDis);

    epcGlobalHelper->getEpc(i)->getLteHelper()->ChangeDefaulChannelModelInitialization(false);
    epcGlobalHelper->getEpc(i)->getLteHelper()->SetUplinkSpectrumChannel(epcGlobalHelper->GetUplinkSpectrumChannel());
    epcGlobalHelper->getEpc(i)->getLteHelper()->SetDownlinkSpectrumChannel(epcGlobalHelper->GetDownlinkSpectrumChannel());
    epcGlobalHelper->getEpc(i)->getLteHelper()->SetDownlinkPathlossModel(epcGlobalHelper->GetDownlinkPathlossModel());
    epcGlobalHelper->getEpc(i)->getLteHelper()->SetUplinkPathlossModel(epcGlobalHelper->GetUplinkPathlossModel());
    epcGlobalHelper->getEpc(i)->getLteHelper()->SetFadingModelPointer(epcGlobalHelper->GetFadingModelPointer());

    epcGlobalHelper->getEpc(i)->setup();
    epcGlobalHelper->getEpc(i)->TraceConnectWithoutContext("UeConnect",
                  MakeCallback (&EpcGroupUeConnect));
    // epcGlobalHelper->getEpc(i)->TraceConnectWithoutContext("StopRealServices",
    //               MakeCallback (&EpcGroupStopRealService));
    epcGlobalHelper->getEpc(i)->TraceConnectWithoutContext("SetupCsmaUe",
                  MakeCallback (&EpcGroupSetupCsmaUe));
    epcGlobalHelper->getEpc(i)->TraceConnectWithoutContext("StopRunningApp",
              MakeCallback (&EpcGroupStopRunningApp));
  }

  if (numOfEpc > 1) {
    // epcGlobalHelper->doRoaming(0,1);
    for (uint16_t i = 1; i < numOfEpc; i++) {
      epcGlobalHelper->doRoaming(i-1,i);
    }
    epcGlobalHelper->setSendRequestViaRoaming(sendRequestViaRoaming);
    Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/LteEnbRrc/RecvMeasurementReport",
                                   MakeCallback (&NotifyRecvMeasurementReport));
  }

  if (useRealDevice) {
    GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

    for (uint16_t i = 0; i < numOfEpc; i++){
      if (i%2 == 0) {
        epcGlobalHelper->setupCsmaServer(i, 
          std::string("10.8")+std::to_string((i+1))+std::string(".0.0"), 
          std::string("10.8")+std::to_string((i+1))+std::string(".0.8"), 
          std::string("tap-server-")+std::to_string((i+1)), 
          "server-1",
          "10.7.20.89:3007"
        );
      } else {
        epcGlobalHelper->setupCsmaServer(i, 
          std::string("10.8")+std::to_string((i+1))+std::string(".0.0"), 
          std::string("10.8")+std::to_string((i+1))+std::string(".0.8"), 
          std::string("tap-server-")+std::to_string((i+1)), 
          "server-2",
          "10.7.20.104:3009"
        );
      }
    }
    // epcGlobalHelper->setupCsmaServer(1, "10.82.0.0", "10.82.0.8", "tap-server-2", "server-1", "10.7.20.104:3009");
  }


  uint16_t numberOfUes = 1;

  appController = CreateObject<AppController>();

  ueController = CreateObject<UeController>(numberOfUes,10,enbDis*4);
  ueController->TraceConnectWithoutContext("AttachUeToEnb",
                  MakeCallback (&NotifyAttachUeToEnb));

  if (ckReportUeStatus) {
    ueController->TraceConnectWithoutContext("ReportUeStatus",
                  MakeCallback (&ReportUeStatus));
  }

  ueController->TraceConnectWithoutContext("FinishSetup",
                  MakeCallback (&NotifyUeManagerFinishSetup));

  ueController->TraceConnectWithoutContext("Reconnect",
                  MakeCallback (&UeReconnect));

  ueController->setSpeed(ueSpeed);
  ueController->setStartingPoint(ueStartPoint);

  ueController->createNode();

  if (useRealDevice) {
    epcGlobalHelper->prepareCsmaUe(ueController->getUe(0), "11.0.0.0", "11.0.0.8", "tap-car-1", "car-1","10.7.20.89:3005");
  }

  ueController->setup();

  epcGlobalHelper->setBackground();


  // LteReporter::runCmd("www.example.com");
  // LteReporter::sshDocker("car-1","http://localhost:3003/testconnection");

  NS_LOG_INFO ("Starting simulation...");

  Simulator::Stop (simTime);

  Simulator::Run ();

  // NS_ABORT_MSG_IF (counterN310FirsteNB != n310, "UE RRC should receive "
  //                  << n310 << " out-of-sync indications in Cell 1."
  //                  " Total received = " << counterN310FirsteNB);

  Simulator::Destroy ();

  return 0;
}
