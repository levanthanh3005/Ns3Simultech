/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 *         (based on epc-sgw-pgw-application.cc)
 */

#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/mac48-address.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv6.h"
#include "ns3/ipv6-header.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/inet-socket-address.h"
#include "ns3/epc-gtpu-header.h"
#include "epc-pgw-application-ext.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EpcPgwApplicationExt");


/////////////////////////
// UeInfo
/////////////////////////

EpcPgwApplicationExt::UeInfo::UeInfo ()
{
  NS_LOG_FUNCTION (this);
}

void
EpcPgwApplicationExt::UeInfo::AddBearer (uint8_t bearerId, uint32_t teid, Ptr<EpcTft> tft)
{
  NS_LOG_FUNCTION (this << (uint16_t) bearerId << teid << tft);
  m_teidByBearerIdMap[bearerId] = teid;
  return m_tftClassifier.Add (tft, teid);
}

void
EpcPgwApplicationExt::UeInfo::RemoveBearer (uint8_t bearerId)
{
  NS_LOG_FUNCTION (this << (uint16_t) bearerId);
  std::map<uint8_t, uint32_t >::iterator it = m_teidByBearerIdMap.find (bearerId);
  m_tftClassifier.Delete (it->second); //delete tft
  m_teidByBearerIdMap.erase (bearerId);
}

uint32_t
EpcPgwApplicationExt::UeInfo::Classify (Ptr<Packet> p, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << p);
  // we hardcode DOWNLINK direction since the PGW is expected to
  // classify only downlink packets (uplink packets will go to the
  // internet without any classification).
  return m_tftClassifier.Classify (p, EpcTft::DOWNLINK, protocolNumber);
}

Ipv4Address
EpcPgwApplicationExt::UeInfo::GetSgwAddr ()
{
  return m_sgwAddr;
}

void
EpcPgwApplicationExt::UeInfo::SetSgwAddr (Ipv4Address sgwAddr)
{
  m_sgwAddr = sgwAddr;
}

Ipv4Address 
EpcPgwApplicationExt::UeInfo::GetUeAddr ()
{
  return m_ueAddr;
}

void
EpcPgwApplicationExt::UeInfo::SetUeAddr (Ipv4Address ueAddr)
{
  m_ueAddr = ueAddr;
}

Ipv6Address 
EpcPgwApplicationExt::UeInfo::GetUeAddr6 ()
{
  return m_ueAddr6;
}

void
EpcPgwApplicationExt::UeInfo::SetUeAddr6 (Ipv6Address ueAddr)
{
  m_ueAddr6 = ueAddr;
}

/////////////////////////
// EpcPgwApplicationExt
/////////////////////////

TypeId
EpcPgwApplicationExt::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EpcPgwApplicationExt")
    .SetParent<Object> ()
    .SetGroupName ("Lte")
    .AddTraceSource ("RxFromTun",
                     "Receive data packets from internet in Tunnel NetDevice",
                     MakeTraceSourceAccessor (&EpcPgwApplicationExt::m_rxTunPktTrace),
                     "ns3::EpcPgwApplicationExt::RxTracedCallback")
    .AddTraceSource ("RxFromS1u",
                     "Receive data packets from S5 Socket",
                     MakeTraceSourceAccessor (&EpcPgwApplicationExt::m_rxS5PktTrace),
                     "ns3::EpcPgwApplicationExt::RxTracedCallback")
    ;
  return tid;
}

void
EpcPgwApplicationExt::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_s5uSocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  m_s5uSocket = 0;
  m_s5cSocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  m_s5cSocket = 0;
}

EpcPgwApplicationExt::EpcPgwApplicationExt (const Ptr<VirtualNetDevice> tunDevice, Ipv4Address s5Addr,
                                      const Ptr<Socket> s5uSocket, const Ptr<Socket> s5cSocket)
  : m_pgwS5Addr (s5Addr),
    m_s5uSocket (s5uSocket),
    m_s5cSocket (s5cSocket),
    m_tunDevice (tunDevice),
    m_gtpuUdpPort (2152), // fixed by the standard
    m_gtpcUdpPort (2123)  // fixed by the standard
{
  NS_LOG_FUNCTION (this << tunDevice << s5Addr << s5uSocket << s5cSocket);
  m_s5uSocket->SetRecvCallback (MakeCallback (&EpcPgwApplicationExt::RecvFromS5uSocket, this));
  m_s5cSocket->SetRecvCallback (MakeCallback (&EpcPgwApplicationExt::RecvFromS5cSocket, this));
}

EpcPgwApplicationExt::~EpcPgwApplicationExt ()
{
  NS_LOG_FUNCTION (this);
}

bool
EpcPgwApplicationExt::RecvFromTunDevice (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  // std::cout<<"EpcPgwApplicationExt::RecvFromTunDevice::"<<std::endl;

  NS_LOG_FUNCTION (this << source << dest << protocolNumber << packet << packet->GetSize ());
  m_rxTunPktTrace (packet->Copy ());

  // get IP address of UE
  if (protocolNumber == Ipv4L3Protocol::PROT_NUMBER)
    {
      Ipv4Header ipv4Header;
      packet->PeekHeader (ipv4Header);
      Ipv4Address ueAddr = ipv4Header.GetDestination ();
      NS_LOG_LOGIC ("packet addressed to UE " << ueAddr);
      // std::cout<<"packet addressed to UE " << ueAddr<<std::endl;
      // find corresponding UeInfo address
      std::map<Ipv4Address, Ptr<UeInfo> >::iterator it = m_ueInfoByAddrMap.find (ueAddr);
      if (it == m_ueInfoByAddrMap.end ())
        {
          NS_LOG_WARN ("unknown UE address " << ueAddr);
          // std::cout<<"unknown UE address " << ueAddr<<std::endl;
        }
      else
        {
          Ipv4Address sgwAddr = it->second->GetSgwAddr ();
          uint32_t teid = it->second->Classify (packet, protocolNumber);
          if (teid == 0)
            {
              NS_LOG_WARN ("no matching bearer for this packet");
              // std::cout<<"no matching bearer for this packet::"<<sgwAddr<<std::endl;
              // if (AddrNeedToBackwards8(ueAddr)) {
              //   std::cout<<"Send to S8"<<std::endl;
              //   SendToS8Socket (packet, m_sgwS8Address, teid);
              // }
            }
          else
            {

              // std::ostringstream oss;
              // ipv4Header.Print(oss);
              // std::cout<<oss.str()<<std::endl;
              // std::cout<<"Sent to s8 or s5"<<std::endl;
              if (AddrNeedToBackwards8(ueAddr)) {
                // std::cout<<"Send to S8 with paket"<<std::endl;
                // std::cout<<packet->ToString()<<std::endl;
                SendToS8Socket (packet, m_sgwS8Address, teid);
              } else {
                SendToS5uSocket (packet, sgwAddr, teid);
              }
            }
        }
    }
  else if (protocolNumber == Ipv6L3Protocol::PROT_NUMBER)
    {
      Ipv6Header ipv6Header;
      packet->PeekHeader (ipv6Header);
      Ipv6Address ueAddr =  ipv6Header.GetDestinationAddress ();
      NS_LOG_LOGIC ("packet addressed to UE " << ueAddr);

      // find corresponding UeInfo address
      std::map<Ipv6Address, Ptr<UeInfo> >::iterator it = m_ueInfoByAddrMap6.find (ueAddr);
      if (it == m_ueInfoByAddrMap6.end ())
        {
          NS_LOG_WARN ("unknown UE address " << ueAddr);
        }
      else
        {
          Ipv4Address sgwAddr = it->second->GetSgwAddr ();
          uint32_t teid = it->second->Classify (packet, protocolNumber);
          if (teid == 0)
            {
              NS_LOG_WARN ("no matching bearer for this packet");
            }
          else
            {
              SendToS5uSocket (packet, sgwAddr, teid);
            }
        }
    }
  else
    {
      NS_ABORT_MSG ("Unknown IP type");
    }

  // there is no reason why we should notify the TUN
  // VirtualNetDevice that he failed to send the packet: if we receive
  // any bogus packet, it will just be silently discarded.
  const bool succeeded = true;
  return succeeded;
}

void
EpcPgwApplicationExt::RecvFromS5uSocket (Ptr<Socket> socket)
{
  // std::cout<<"EpcPgwApplicationExt::RecvFromS5uSocket"<<std::endl;

  NS_LOG_FUNCTION (this << socket);
  NS_ASSERT (socket == m_s5uSocket);
  Ptr<Packet> packet = socket->Recv ();
  m_rxS5PktTrace (packet->Copy ());

  GtpuHeader gtpu;
  packet->RemoveHeader (gtpu);
  uint32_t teid = gtpu.GetTeid ();

  SendToTunDevice (packet, teid);
}

void
EpcPgwApplicationExt::RecvFromS8Socket (Ptr<Socket> socket)
{
  // std::cout<<"EpcPgwApplicationExt::RecvFromS8Socket"<<std::endl;
  NS_LOG_FUNCTION (this << socket);
  NS_ASSERT (socket == m_pgwS8Socket);
  Ptr<Packet> packet = socket->Recv ();
  m_rxS5PktTrace (packet->Copy ());

  GtpuHeader gtpu;
  packet->RemoveHeader (gtpu);
  uint32_t teid = gtpu.GetTeid ();

  Ipv4Header ipv4Header;
  packet->PeekHeader(ipv4Header);

  // std::cout<<"Source::"<<ipv4Header.GetSource()<<" Destination::"<< ipv4Header.GetDestination() <<std::endl;


  SendToTunDevice (packet, teid);
}

void
EpcPgwApplicationExt::RecvFromS5cSocket (Ptr<Socket> socket)
{
  // std::cout<<"EpcPgwApplicationExt::RecvFromS5cSocket"<<std::endl;
  // std::cout<<this<<std::endl;
  
  NS_LOG_FUNCTION (this << socket);
  NS_ASSERT (socket == m_s5cSocket);
  Ptr<Packet> packet = socket->Recv ();
  GtpcHeader header;
  packet->PeekHeader (header);
  uint16_t msgType = header.GetMessageType ();

  switch (msgType)
    {
    case GtpcHeader::CreateSessionRequest:
      DoRecvCreateSessionRequest (packet);
      break;

    case GtpcHeader::ModifyBearerRequest:
      DoRecvModifyBearerRequest (packet);
      break;

    case GtpcHeader::DeleteBearerCommand:
      DoRecvDeleteBearerCommand (packet);
      break;

    case GtpcHeader::DeleteBearerResponse:
      DoRecvDeleteBearerResponse (packet);
      break;

    default:
      NS_FATAL_ERROR ("GTP-C message not supported");
      break;
    }
}

void
EpcPgwApplicationExt::DoRecvCreateSessionRequest (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);

  GtpcCreateSessionRequestMessage msg;
  packet->RemoveHeader (msg);
  uint64_t imsi = msg.GetImsi ();
  uint16_t cellId = msg.GetUliEcgi ();
  NS_LOG_DEBUG ("cellId " << cellId << " imsi " << (uint16_t)imsi);

  std::map<uint64_t, Ptr<UeInfo> >::iterator ueit = m_ueInfoByImsiMap.find (imsi);
  NS_ASSERT_MSG (ueit != m_ueInfoByImsiMap.end (), "unknown IMSI " << imsi);
  ueit->second->SetSgwAddr (m_sgwS5Addr);

  GtpcHeader::Fteid_t sgwS5cFteid = msg.GetSenderCpFteid ();
  NS_ASSERT_MSG (sgwS5cFteid.interfaceType == GtpcHeader::S5_SGW_GTPC,
                 "Wrong interface type");

  GtpcCreateSessionResponseMessage msgOut;
  msgOut.SetTeid (sgwS5cFteid.teid);
  msgOut.SetCause (GtpcCreateSessionResponseMessage::REQUEST_ACCEPTED);

  GtpcHeader::Fteid_t pgwS5cFteid;
  pgwS5cFteid.interfaceType = GtpcHeader::S5_PGW_GTPC;
  pgwS5cFteid.teid = sgwS5cFteid.teid;
  pgwS5cFteid.addr = m_pgwS5Addr;
  msgOut.SetSenderCpFteid (pgwS5cFteid);

  std::list<GtpcCreateSessionRequestMessage::BearerContextToBeCreated> bearerContexts =
      msg.GetBearerContextsToBeCreated ();
  NS_LOG_DEBUG ("BearerContextsToBeCreated size = " << bearerContexts.size ());

  std::list<GtpcCreateSessionResponseMessage::BearerContextCreated> bearerContextsCreated;
  for (auto &bearerContext : bearerContexts)
    {
      uint32_t teid = bearerContext.sgwS5uFteid.teid;
      NS_LOG_DEBUG ("bearerId " << (uint16_t)bearerContext.epsBearerId <<
                    " SGW " << bearerContext.sgwS5uFteid.addr << " TEID " << teid);

      ueit->second->AddBearer (bearerContext.epsBearerId, teid, bearerContext.tft);

      GtpcCreateSessionResponseMessage::BearerContextCreated bearerContextOut;
      bearerContextOut.fteid.interfaceType = GtpcHeader::S5_PGW_GTPU;
      bearerContextOut.fteid.teid = teid;
      bearerContextOut.fteid.addr = m_pgwS5Addr;
      bearerContextOut.epsBearerId = bearerContext.epsBearerId;
      bearerContextOut.bearerLevelQos = bearerContext.bearerLevelQos;
      bearerContextOut.tft = bearerContext.tft;
      bearerContextsCreated.push_back (bearerContextOut);
    }

  NS_LOG_DEBUG ("BearerContextsCreated size = " << bearerContextsCreated.size ());
  msgOut.SetBearerContextsCreated (bearerContextsCreated);
  msgOut.SetTeid (sgwS5cFteid.teid);
  msgOut.ComputeMessageLength ();

  Ptr<Packet> packetOut = Create <Packet> ();
  packetOut->AddHeader (msgOut);
  NS_LOG_DEBUG ("Send CreateSessionResponse to SGW " << sgwS5cFteid.addr);
  m_s5cSocket->SendTo (packetOut, 0, InetSocketAddress (sgwS5cFteid.addr, m_gtpcUdpPort));
}

void
EpcPgwApplicationExt::DoRecvModifyBearerRequest (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);

  GtpcModifyBearerRequestMessage msg;
  packet->RemoveHeader (msg);
  uint8_t imsi = msg.GetImsi ();
  uint16_t cellId = msg.GetUliEcgi ();
  NS_LOG_DEBUG ("cellId " << cellId << "IMSI " << (uint16_t)imsi);

  std::map<uint64_t, Ptr<UeInfo> >::iterator ueit = m_ueInfoByImsiMap.find (imsi);
  NS_ASSERT_MSG (ueit != m_ueInfoByImsiMap.end (), "unknown IMSI " << imsi); 
  ueit->second->SetSgwAddr (m_sgwS5Addr);

  std::list<GtpcModifyBearerRequestMessage::BearerContextToBeModified> bearerContexts =
      msg.GetBearerContextsToBeModified ();
  NS_LOG_DEBUG ("BearerContextsToBeModified size = " << bearerContexts.size ());

  for (auto &bearerContext : bearerContexts)
    {
      Ipv4Address sgwAddr = bearerContext.fteid.addr;
      uint32_t teid = bearerContext.fteid.teid;
      NS_LOG_DEBUG ("bearerId " << (uint16_t)bearerContext.epsBearerId <<
                    " SGW " << sgwAddr << " TEID " << teid);
    }

  GtpcModifyBearerResponseMessage msgOut;
  msgOut.SetCause (GtpcIes::REQUEST_ACCEPTED);
  msgOut.SetTeid (imsi);
  msgOut.ComputeMessageLength ();

  Ptr<Packet> packetOut = Create <Packet> ();
  packetOut->AddHeader (msgOut);
  NS_LOG_DEBUG ("Send ModifyBearerResponse to SGW " << m_sgwS5Addr);
  m_s5cSocket->SendTo (packetOut, 0, InetSocketAddress (m_sgwS5Addr, m_gtpcUdpPort));
}

void
EpcPgwApplicationExt::DoRecvDeleteBearerCommand (Ptr<Packet> packet)
{
  // std::cout<<"EpcPgwApplicationExt::DoRecvDeleteBearerCommand"<<std::endl;
  NS_LOG_FUNCTION (this);

  GtpcDeleteBearerCommandMessage msg;
  packet->RemoveHeader (msg);

  std::list<uint8_t> epsBearerIds;
  for (auto &bearerContext : msg.GetBearerContexts ())
    {
      NS_LOG_DEBUG ("ebid " << (uint16_t) bearerContext.m_epsBearerId);
      epsBearerIds.push_back (bearerContext.m_epsBearerId);
    }

  GtpcDeleteBearerRequestMessage msgOut;
  msgOut.SetEpsBearerIds (epsBearerIds);
  msgOut.SetTeid (msg.GetTeid ());
  msgOut.ComputeMessageLength ();

  Ptr<Packet> packetOut = Create <Packet> ();
  packetOut->AddHeader (msgOut);
  NS_LOG_DEBUG ("Send DeleteBearerRequest to SGW " << m_sgwS5Addr);
  m_s5cSocket->SendTo (packetOut, 0, InetSocketAddress (m_sgwS5Addr, m_gtpcUdpPort));
}

void
EpcPgwApplicationExt::DoRecvDeleteBearerResponse (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);

  GtpcDeleteBearerResponseMessage msg;
  packet->RemoveHeader (msg);

  uint64_t imsi = msg.GetTeid ();
  std::map<uint64_t, Ptr<UeInfo> >::iterator ueit = m_ueInfoByImsiMap.find (imsi);
  NS_ASSERT_MSG (ueit != m_ueInfoByImsiMap.end (), "unknown IMSI " << imsi);

  for (auto &epsBearerId : msg.GetEpsBearerIds ())
    {
      // Remove de-activated bearer contexts from PGW side
      NS_LOG_INFO ("PGW removing bearer " << (uint16_t) epsBearerId << " of IMSI " << imsi);
      ueit->second->RemoveBearer (epsBearerId);
    }
}


void
EpcPgwApplicationExt::SendToTunDevice (Ptr<Packet> packet, uint32_t teid)
{
  // std::cout<<"EpcPgwApplicationExt::SendToTunDevice"<<std::endl;
  // std::cout<<packet->ToString()<<std::endl;
  NS_LOG_FUNCTION (this << packet << teid);
  NS_LOG_LOGIC ("packet size: " << packet->GetSize () << " bytes");

  uint8_t ipType;
  packet->CopyData (&ipType, 1);
  ipType = (ipType>>4) & 0x0f;

  uint16_t protocol = 0;
  if (ipType == 0x04)
    {
      protocol = 0x0800;
    }
  else if (ipType == 0x06)
    {
      protocol = 0x86DD;
    }
  else
    {
      NS_ABORT_MSG ("Unknown IP type");
    }

  // std::cout<<"EpcPgwApplicationExt::SendToTunDevice::Sent"<<std::endl;
  // std::cout<<protocol<<m_tunDevice->GetAddress ()<<m_tunDevice->GetAddress ()<<std::endl;
  m_tunDevice->Receive (packet, protocol, m_tunDevice->GetAddress (), m_tunDevice->GetAddress (), NetDevice::PACKET_HOST);
}

void
EpcPgwApplicationExt::SendToS8Socket (Ptr<Packet> packet, Ipv4Address sgwAddr, uint32_t teid)
{
  // std::cout<<"EpcPgwApplicationExt::SendToS8Socket"<<std::endl;

  NS_LOG_FUNCTION (this << packet << sgwAddr << teid);

  GtpuHeader gtpu;
  gtpu.SetTeid (teid);
  // From 3GPP TS 29.281 v10.0.0 Section 5.1
  // Length of the payload + the non obligatory GTP-U header
  gtpu.SetLength (packet->GetSize () + gtpu.GetSerializedSize () - 8);
  packet->AddHeader (gtpu);
  uint32_t flags = 0;
  m_pgwS8Socket->SendTo (packet, flags, InetSocketAddress (sgwAddr, m_gtpuUdpPort));
}

void
EpcPgwApplicationExt::SendToS5uSocket (Ptr<Packet> packet, Ipv4Address sgwAddr, uint32_t teid)
{
  // // std::cout<<"EpcPgwApplicationExt::SendToS5uSocket"<<std::endl;

  NS_LOG_FUNCTION (this << packet << sgwAddr << teid);

  GtpuHeader gtpu;
  gtpu.SetTeid (teid);
  // From 3GPP TS 29.281 v10.0.0 Section 5.1
  // Length of the payload + the non obligatory GTP-U header
  gtpu.SetLength (packet->GetSize () + gtpu.GetSerializedSize () - 8);
  packet->AddHeader (gtpu);
  uint32_t flags = 0;
  m_s5uSocket->SendTo (packet, flags, InetSocketAddress (sgwAddr, m_gtpuUdpPort));
}


void
EpcPgwApplicationExt::AddSgw (Ipv4Address sgwS5Addr)
{
    NS_LOG_FUNCTION (this << sgwS5Addr);
    m_sgwS5Addr = sgwS5Addr;
}

void
EpcPgwApplicationExt::AddUe (uint64_t imsi)
{
  NS_LOG_FUNCTION (this << imsi);
  Ptr<UeInfo> ueInfo = Create<UeInfo> ();
  m_ueInfoByImsiMap[imsi] = ueInfo;
}

void
EpcPgwApplicationExt::SetUeAddress (uint64_t imsi, Ipv4Address ueAddr)
{
  NS_LOG_FUNCTION (this << imsi << ueAddr);
  std::map<uint64_t, Ptr<UeInfo> >::iterator ueit = m_ueInfoByImsiMap.find (imsi);
  NS_ASSERT_MSG (ueit != m_ueInfoByImsiMap.end (), "unknown IMSI" << imsi); 
  ueit->second->SetUeAddr (ueAddr);
  m_ueInfoByAddrMap[ueAddr] = ueit->second;
}

void
EpcPgwApplicationExt::SetUeAddress6 (uint64_t imsi, Ipv6Address ueAddr)
{
  NS_LOG_FUNCTION (this << imsi << ueAddr);
  std::map<uint64_t, Ptr<UeInfo> >::iterator ueit = m_ueInfoByImsiMap.find (imsi);
  NS_ASSERT_MSG (ueit != m_ueInfoByImsiMap.end (), "unknown IMSI " << imsi); 
  m_ueInfoByAddrMap6[ueAddr] = ueit->second;
  ueit->second->SetUeAddr6 (ueAddr);
}

void
EpcPgwApplicationExt::AddSgwS8Roaming (Ipv4Address sgwS8Addr, Ipv4Address pgwS8Addr, Ptr<Socket> pgwS8Sk)
{
  m_sgwS8Address = sgwS8Addr;
  m_pgwS8Address = pgwS8Addr;
  m_pgwS8Socket = pgwS8Sk;

  m_pgwS8Socket->SetRecvCallback (MakeCallback (&EpcPgwApplicationExt::RecvFromS8Socket, this));

};

void
EpcPgwApplicationExt::AddAddrToBackwards8(Ipv4Address addr)
{
  s8BackwardList.push_back(addr);
};

bool 
EpcPgwApplicationExt::AddrNeedToBackwards8(Ipv4Address addr)
{
  for (Ipv4Address it : s8BackwardList) {
    if (it == addr) {
      return true;
    }
  }
  return false;
};


}  // namespace ns3
