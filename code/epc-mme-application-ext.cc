/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017-2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Manuel Requena <manuel.requena@cttc.es>
 */

#include "ns3/log.h"
#include "ns3/epc-gtpc-header.h"
// #include "ns3/epc-mme-application.h"
#include "epc-mme-application-ext.h"
#include "simTag.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EpcMmeApplicationExt");

NS_OBJECT_ENSURE_REGISTERED (EpcMmeApplicationExt);

EpcMmeApplicationExt::EpcMmeApplicationExt ()
  : m_gtpcUdpPort (2123) //fixed by the standard
{
  NS_LOG_FUNCTION (this);
  m_s1apSapMme = new MemberEpcS1apSapMme<EpcMmeApplicationExt> (this);
}
EpcMmeApplicationExt::~EpcMmeApplicationExt ()
{
  NS_LOG_FUNCTION (this);
}

void
EpcMmeApplicationExt::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_s1apSapMme;
}

TypeId
EpcMmeApplicationExt::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EpcMmeApplicationExt")
    .SetParent<Object> ()
    .SetGroupName ("Lte")
    .AddConstructor<EpcMmeApplicationExt> ();
  return tid;
}

EpcS1apSapMme*
EpcMmeApplicationExt::GetS1apSapMme ()
{
  return m_s1apSapMme;
}

void
EpcMmeApplicationExt::AddSgw (Ipv4Address sgwS11Addr, Ipv4Address mmeS11Addr, Ptr<Socket> mmeS11Socket)
{
  NS_LOG_FUNCTION (this << sgwS11Addr << mmeS11Addr << mmeS11Socket);
  m_sgwS11Addr = sgwS11Addr;
  m_mmeS11Addr = mmeS11Addr;
  m_s11Socket = mmeS11Socket;
  m_s11Socket->SetRecvCallback (MakeCallback (&EpcMmeApplicationExt::RecvFromS11Socket, this));
}

void
EpcMmeApplicationExt::AddHss (Ipv4Address hssS6aAddress, Ptr<Socket> mmeS6aSocket)
{
  // std::cout<<"EpcMmeApplicationExt::AddHss"<<std::endl;
  m_s6aSocket = mmeS6aSocket;
  m_hssS6aAddr = hssS6aAddress;

  m_s6aSocket->SetRecvCallback (MakeCallback (&EpcMmeApplicationExt::RecvFromS6aSocket, this));
}

void
EpcMmeApplicationExt::AddHssRoaming (Ipv4Address hssS6dAddress, Ptr<Socket> mmeS6dSocket)
{
  // std::cout<<"EpcMmeApplicationExt::AddHssRoaming"<<std::endl;
  m_s6dSocket = mmeS6dSocket;
  m_hssS6dAddr = hssS6dAddress;

  m_s6dSocket->SetRecvCallback (MakeCallback (&EpcMmeApplicationExt::RecvFromS6dSocket, this));
}


void
EpcMmeApplicationExt::AddEnb (uint16_t gci, Ipv4Address enbS1uAddr, EpcS1apSapEnb* enbS1apSap)
{
  NS_LOG_FUNCTION (this << gci << enbS1uAddr << enbS1apSap);
  Ptr<EnbInfo> enbInfo = Create<EnbInfo> ();
  enbInfo->gci = gci;
  enbInfo->s1uAddr = enbS1uAddr;
  enbInfo->s1apSapEnb = enbS1apSap;
  m_enbInfoMap[gci] = enbInfo;
}

void
EpcMmeApplicationExt::AddUe (uint64_t imsi)
{
  NS_LOG_FUNCTION (this << imsi);
  Ptr<UeInfo> ueInfo = Create<UeInfo> ();
  ueInfo->imsi = imsi;
  ueInfo->mmeUeS1Id = imsi;
  ueInfo->bearerCounter = 0;
  m_ueInfoMap[imsi] = ueInfo;
  // syncWithHss();
  // std::cout<<"syncWithHss"<<std::endl;
  // Ptr<Packet> packet = Create <Packet> ();
  // SimTag simTag;
  // simTag.setImsi(imsi);
  // packet->AddPacketTag (simTag);
  // m_s6aSocket->SendTo (packet, 0, InetSocketAddress (m_hssS6aAddr, m_gtpcUdpPort));
}

uint8_t
EpcMmeApplicationExt::AddBearer (uint64_t imsi, Ptr<EpcTft> tft, EpsBearer bearer)
{
  NS_LOG_FUNCTION (this << imsi);
  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
  NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
  NS_ASSERT_MSG (it->second->bearerCounter < 11, "too many bearers already! " << it->second->bearerCounter);
  BearerInfo bearerInfo;
  bearerInfo.bearerId = ++(it->second->bearerCounter);
  bearerInfo.tft = tft;
  bearerInfo.bearer = bearer;
  it->second->bearersToBeActivated.push_back (bearerInfo);
  return bearerInfo.bearerId;
}


std::map<uint64_t, Ptr<EpcMmeApplicationExt::UeInfo> >
EpcMmeApplicationExt::getUeInfoMap(){
  return m_ueInfoMap;
}

Ptr<EpcMmeApplicationExt::UeInfo>
EpcMmeApplicationExt::GetUe (uint64_t imsi)
{
  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
  NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
  return it->second;
};


// S1-AP SAP MME forwarded methods

void
EpcMmeApplicationExt::DoInitialUeMessage (uint64_t mmeUeS1Id, uint16_t enbUeS1Id, uint64_t imsi, uint16_t gci)
{
  // std::cout<<"EpcMmeApplicationExt::DoInitialUeMessage:"<<mmeUeS1Id<<" "<<enbUeS1Id<<" "<< imsi <<" "<<gci<<std::endl;
  NS_LOG_FUNCTION (this << mmeUeS1Id << enbUeS1Id << imsi << gci);
  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
  NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
  it->second->cellId = gci;

  GtpcCreateSessionRequestMessage msg;
  msg.SetImsi (imsi);
  msg.SetUliEcgi (gci);

  GtpcHeader::Fteid_t mmeS11Fteid;
  mmeS11Fteid.interfaceType = GtpcHeader::S11_MME_GTPC;
  mmeS11Fteid.teid = imsi;
  mmeS11Fteid.addr = m_mmeS11Addr;
  msg.SetSenderCpFteid (mmeS11Fteid); // S11 MME GTP-C F-TEID

  std::list<GtpcCreateSessionRequestMessage::BearerContextToBeCreated> bearerContexts;
  for (std::list<BearerInfo>::iterator bit = it->second->bearersToBeActivated.begin ();
       bit != it->second->bearersToBeActivated.end ();
       ++bit)
    {
      GtpcCreateSessionRequestMessage::BearerContextToBeCreated bearerContext;
      bearerContext.epsBearerId =  bit->bearerId;
      bearerContext.tft = bit->tft;
      bearerContext.bearerLevelQos = bit->bearer;
      bearerContexts.push_back (bearerContext);
    }
  NS_LOG_DEBUG ("BearerContextToBeCreated size = " << bearerContexts.size ());
  msg.SetBearerContextsToBeCreated (bearerContexts);

  msg.SetTeid (0);
  msg.ComputeMessageLength ();

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (msg);
  NS_LOG_DEBUG ("Send CreateSessionRequest to SGW " << m_sgwS11Addr);
  m_s11Socket->SendTo (packet, 0, InetSocketAddress (m_sgwS11Addr, m_gtpcUdpPort));

  // syncWithHss();
  // std::cout<<"syncWithHss"<<std::endl;
  Ptr<Packet> packetS6a = Create <Packet> ();
  SimTagImsi simTagImsi;
  simTagImsi.setImsi(imsi);

  SimTagZone simTagZone;
  simTagZone.setZone(zone);
  
  packetS6a->AddPacketTag (simTagImsi);
  packetS6a->AddPacketTag (simTagZone);

  m_s6aSocket->SendTo (packetS6a, 0, InetSocketAddress (m_hssS6aAddr, m_gtpcUdpPort));
}

void
EpcMmeApplicationExt::DoInitialContextSetupResponse (uint64_t mmeUeS1Id, uint16_t enbUeS1Id, std::list<EpcS1apSapMme::ErabSetupItem> erabSetupList)
{
  NS_LOG_FUNCTION (this << mmeUeS1Id << enbUeS1Id);
  NS_FATAL_ERROR ("unimplemented");
}

void
EpcMmeApplicationExt::DoPathSwitchRequest (uint64_t enbUeS1Id, uint64_t mmeUeS1Id, uint16_t gci, std::list<EpcS1apSapMme::ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList)
{
  NS_LOG_FUNCTION (this << mmeUeS1Id << enbUeS1Id << gci);
  uint64_t imsi = mmeUeS1Id;
  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
  NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
  NS_LOG_INFO ("IMSI " << imsi << " old eNB: " << it->second->cellId << ", new eNB: " << gci);
  it->second->cellId = gci;
  it->second->enbUeS1Id = enbUeS1Id;

  GtpcModifyBearerRequestMessage msg;
  msg.SetImsi (imsi);
  msg.SetUliEcgi (gci);

  std::list<GtpcModifyBearerRequestMessage::BearerContextToBeModified> bearerContexts;
  for (auto &erab : erabToBeSwitchedInDownlinkList)
    {
      NS_LOG_DEBUG ("erabId " << erab.erabId <<
                    " eNB " << erab.enbTransportLayerAddress <<
                    " TEID " << erab.enbTeid);

      GtpcModifyBearerRequestMessage::BearerContextToBeModified bearerContext;
      bearerContext.epsBearerId = erab.erabId;
      bearerContext.fteid.interfaceType = GtpcHeader::S1U_ENB_GTPU;
      bearerContext.fteid.addr = erab.enbTransportLayerAddress;
      bearerContext.fteid.teid = erab.enbTeid;
      bearerContexts.push_back (bearerContext);
    }
  msg.SetBearerContextsToBeModified (bearerContexts);
  msg.SetTeid (imsi);
  msg.ComputeMessageLength ();

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (msg);
  NS_LOG_DEBUG ("Send ModifyBearerRequest to SGW " << m_sgwS11Addr);
  m_s11Socket->SendTo (packet, 0, InetSocketAddress (m_sgwS11Addr, m_gtpcUdpPort));

  // syncWithHss();
}

void
EpcMmeApplicationExt::DoErabReleaseIndication (uint64_t mmeUeS1Id, uint16_t enbUeS1Id, std::list<EpcS1apSapMme::ErabToBeReleasedIndication> erabToBeReleaseIndication)
{
  // std::cout<<"EpcMmeApplicationExt::DoErabReleaseIndication"<<std::endl;
  NS_LOG_FUNCTION (this << mmeUeS1Id << enbUeS1Id);
  uint64_t imsi = mmeUeS1Id;
  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
  NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);

  GtpcDeleteBearerCommandMessage msg;
  std::list<GtpcDeleteBearerCommandMessage::BearerContext> bearerContexts;
  for (auto &erab : erabToBeReleaseIndication)
    {
      NS_LOG_DEBUG ("erabId " << (uint16_t)erab.erabId);
      // std::cout<<"erabId " << (uint16_t)erab.erabId<<std::endl;

      GtpcDeleteBearerCommandMessage::BearerContext bearerContext;
      bearerContext.m_epsBearerId = erab.erabId;
      bearerContexts.push_back (bearerContext);
    }
  msg.SetBearerContexts (bearerContexts);
  msg.SetTeid (imsi);
  msg.ComputeMessageLength ();

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (msg);
  NS_LOG_DEBUG ("Send DeleteBearerCommand to SGW " << m_sgwS11Addr);
  // std::cout<<"Send DeleteBearerCommand to SGW "<<std::endl;
  m_s11Socket->SendTo (packet, 0, InetSocketAddress (m_sgwS11Addr, m_gtpcUdpPort));

  // syncWithHss();
}

void
EpcMmeApplicationExt::RemoveBearer (Ptr<UeInfo> ueInfo, uint8_t epsBearerId)
{
  NS_LOG_FUNCTION (this << epsBearerId);
  std::list<BearerInfo>::iterator bit = ueInfo->bearersToBeActivated.begin ();
  while (bit != ueInfo->bearersToBeActivated.end ())
    {
      if (bit->bearerId == epsBearerId)
        {
          ueInfo->bearersToBeActivated.erase (bit);
          ueInfo->bearerCounter = ueInfo->bearerCounter - 1;
          break;
        }
      ++bit;
    }
}

void
EpcMmeApplicationExt::RecvFromS6dSocket (Ptr<Socket> socket)
{
  // std::cout<<"EpcMmeApplicationExt::RecvFromS6dSocket"<<std::endl;

  NS_LOG_FUNCTION (this << socket);
  NS_ASSERT (socket == m_s6dSocket);
  Ptr<Packet> packet = socket->Recv ();
  
  // PacketTagIterator i = packet->GetPacketTagIterator();
  SimTagImsi simTagImsi;

  bool ckImis = packet->PeekPacketTag(simTagImsi);
  NS_ASSERT_MSG (ckImis, "imis has problen");

  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (simTagImsi.getImsi());
  if (it != m_ueInfoMap.end ()) {
    // std::cout<<"EpcMmeApplicationExt::RecvFromS6dSocket: Ue is added into epc"<<std::endl;
    return;
  }

  SimTagZone simTagZone;
  bool ckZone = packet->PeekPacketTag(simTagZone);
  NS_ASSERT_MSG (ckZone, "zone has problen");

  SimTagRoaming simTagRoaming;
  bool ckRoaming = packet->PeekPacketTag(simTagRoaming);
  NS_ASSERT_MSG (ckRoaming, "roaming has problen");

  // std::cout<<simTagImsi.getImsi()<<" vs "<<simTagZone.getZone()<<" " <<simTagRoaming.getRoaming()<<std::endl;

  AddUe(simTagImsi.getImsi());

  m_afterCheckingRoaming(tmpPtrUeLte,tmpEnbIndex);

}


void
EpcMmeApplicationExt::RecvFromS6aSocket (Ptr<Socket> socket)
{
  // std::cout<<"EpcMmeApplicationExt::RecvFromS6aSocket"<<std::endl;
}

void
EpcMmeApplicationExt::RecvFromS11Socket (Ptr<Socket> socket)
{
  // std::cout<<"EpcMmeApplicationExt::RecvFromS11Socket"<<std::endl;
  NS_LOG_FUNCTION (this << socket);
  NS_ASSERT (socket == m_s11Socket);
  Ptr<Packet> packet = socket->Recv ();
  GtpcHeader header;
  packet->PeekHeader (header);
  uint16_t msgType = header.GetMessageType ();

  switch (msgType)
    {
    case GtpcHeader::CreateSessionResponse:
      // std::cout<<"EpcMmeApplicationExt::CreateSessionResponse"<<std::endl;
      //start
      DoRecvCreateSessionResponse (header, packet);
      break;

    case GtpcHeader::ModifyBearerResponse:
      // std::cout<<"EpcMmeApplicationExt::ModifyBearerResponse"<<std::endl;
      //After handover
      DoRecvModifyBearerResponse (header, packet);
      break;

    case GtpcHeader::DeleteBearerRequest:
      // std::cout<<"EpcMmeApplicationExt::DeleteBearerRequest"<<std::endl;
      DoRecvDeleteBearerRequest (header, packet);
      break;

    default:
      // std::cout<<"EpcMmeApplicationExt::GTP-C message not supported"<<std::endl;
      NS_FATAL_ERROR ("GTP-C message not supported");
      break;
    }
}

void
EpcMmeApplicationExt::DoRecvCreateSessionResponse (GtpcHeader &header, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << header);
  uint64_t imsi = header.GetTeid ();
  NS_LOG_DEBUG ("TEID/IMSI " << imsi);
  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
  NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
  uint16_t cellId = it->second->cellId;
  uint16_t enbUeS1Id = it->second->enbUeS1Id;
  uint64_t mmeUeS1Id = it->second->mmeUeS1Id;
  NS_LOG_DEBUG ("cellId " << cellId << " mmeUeS1Id " << mmeUeS1Id << " enbUeS1Id " << enbUeS1Id);
  std::map<uint16_t, Ptr<EnbInfo> >::iterator jt = m_enbInfoMap.find (cellId);
  NS_ASSERT_MSG (jt != m_enbInfoMap.end (), "could not find any eNB with CellId " << cellId);

  GtpcCreateSessionResponseMessage msg;
  packet->RemoveHeader (msg);

  std::list<EpcS1apSapEnb::ErabToBeSetupItem> erabToBeSetupList;
  std::list<GtpcCreateSessionResponseMessage::BearerContextCreated> bearerContexts =
    msg.GetBearerContextsCreated ();
  NS_LOG_DEBUG ("BearerContextsCreated size = " << bearerContexts.size ());
  for (auto &bearerContext : bearerContexts)
    {
      EpcS1apSapEnb::ErabToBeSetupItem erab;
      erab.erabId = bearerContext.epsBearerId;
      erab.erabLevelQosParameters = bearerContext.bearerLevelQos;
      erab.transportLayerAddress = bearerContext.fteid.addr; // SGW S1U address
      erab.sgwTeid = bearerContext.fteid.teid;
      NS_LOG_DEBUG ("SGW " << erab.transportLayerAddress << " TEID " << erab.sgwTeid);
      erabToBeSetupList.push_back (erab);
    }

  NS_LOG_DEBUG ("Send InitialContextSetupRequest to eNB " << jt->second->s1apSapEnb);
  jt->second->s1apSapEnb->InitialContextSetupRequest (mmeUeS1Id, enbUeS1Id, erabToBeSetupList);

  // syncWithHss();
}

void
EpcMmeApplicationExt::DoRecvModifyBearerResponse (GtpcHeader &header, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << header);
  GtpcModifyBearerResponseMessage msg;
  packet->RemoveHeader (msg);
  NS_ASSERT (msg.GetCause () == GtpcModifyBearerResponseMessage::REQUEST_ACCEPTED);

  uint64_t imsi = header.GetTeid ();
  NS_LOG_DEBUG ("TEID/IMSI " << imsi);
  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
  NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
  uint16_t cellId = it->second->cellId;
  uint16_t enbUeS1Id = it->second->enbUeS1Id;
  uint64_t mmeUeS1Id = it->second->mmeUeS1Id;
  NS_LOG_DEBUG ("cellId " << cellId << " mmeUeS1Id " << mmeUeS1Id << " enbUeS1Id " << enbUeS1Id);
  std::list<EpcS1apSapEnb::ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList; // unused for now
  std::map<uint16_t, Ptr<EnbInfo> >::iterator jt = m_enbInfoMap.find (cellId);
  NS_ASSERT_MSG (jt != m_enbInfoMap.end (), "could not find any eNB with CellId " << cellId);

  NS_LOG_DEBUG ("Send PathSwitchRequestAcknowledge to eNB " << jt->second->s1apSapEnb);
  jt->second->s1apSapEnb->PathSwitchRequestAcknowledge (enbUeS1Id, mmeUeS1Id, cellId, erabToBeSwitchedInUplinkList);

  // syncWithHss();
}

void
EpcMmeApplicationExt::DoRecvDeleteBearerRequest (GtpcHeader &header, Ptr<Packet> packet)
{
  // std::cout<<"EpcMmeApplicationExt::DoRecvDeleteBearerRequest"<<std::endl;
  NS_LOG_FUNCTION (this << header);
  uint64_t imsi = header.GetTeid ();
  NS_LOG_DEBUG ("TEID/IMSI " << imsi);
  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
  NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);

  GtpcDeleteBearerRequestMessage msg;
  packet->RemoveHeader (msg);

  GtpcDeleteBearerResponseMessage msgOut;

  std::list<uint8_t> epsBearerIds;
  for (auto &ebid : msg.GetEpsBearerIds ())
    {
      epsBearerIds.push_back (ebid);
      /*
       * This condition is added to not remove bearer info at MME
       * when UE gets disconnected since the bearers are only added
       * at beginning of simulation at MME and if it is removed the
       * bearers cannot be activated again unless scheduled for
       * addition of the bearer during simulation
       *
       */
      if (it->second->cellId == 0)
        {
          RemoveBearer (it->second, ebid); //schedules function to erase, context of de-activated bearer
        }
    }
  msgOut.SetEpsBearerIds (epsBearerIds);
  msgOut.SetTeid (imsi);
  msgOut.ComputeMessageLength ();

  Ptr<Packet> packetOut = Create <Packet> ();
  packetOut->AddHeader (msgOut);
  NS_LOG_DEBUG ("Send DeleteBearerResponse to SGW " << m_sgwS11Addr);
  m_s11Socket->SendTo (packetOut, 0, InetSocketAddress (m_sgwS11Addr, m_gtpcUdpPort));

  // syncWithHss();
}

void
EpcMmeApplicationExt::setZone(uint16_t m_zone)
{
  zone = m_zone;
};

uint16_t 
EpcMmeApplicationExt::getZone()
{
  return zone;
};

void
EpcMmeApplicationExt::checkImsiViaRoaming(uint16_t imsi)
{
  // std::cout<<"EpcMmeApplicationExt:checkImsiViaRoaming"<<std::endl;
  Ptr<Packet> packetS6d = Create <Packet> ();
  SimTagImsi simTagImsi;
  simTagImsi.setImsi(imsi);
  
  packetS6d->AddPacketTag (simTagImsi);

  m_s6dSocket->SendTo (packetS6d, 0, InetSocketAddress (m_hssS6dAddr, m_gtpcUdpPort));
};

void 
EpcMmeApplicationExt::setCallbackAfterCheckingRoaming(
  Callback<void, Ptr<LteUeNetDevice>, uint16_t> afterCheckingRoaming, 
  Ptr<LteUeNetDevice> ptrUeLte, uint16_t enbIndex)
{
  m_afterCheckingRoaming = afterCheckingRoaming;
  tmpPtrUeLte = ptrUeLte;
  tmpEnbIndex = enbIndex;
};


} // namespace ns3
