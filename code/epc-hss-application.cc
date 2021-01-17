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
#include "epc-hss-application.h"
#include "simTag.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EpcHssApplication");

NS_OBJECT_ENSURE_REGISTERED (EpcHssApplication);


EpcHssApplication::EpcHssApplication ()
  : m_gtpcUdpPort (2123) //fixed by the standard
{
  NS_LOG_FUNCTION (this);
}
EpcHssApplication::~EpcHssApplication ()
{
  NS_LOG_FUNCTION (this);
}

void
EpcHssApplication::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
EpcHssApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EpcHssApplication")
    .SetParent<Object> ()
    .SetGroupName ("Lte")
    .AddConstructor<EpcHssApplication> ();
  return tid;
}

void
EpcHssApplication::showSimCard ()
{
  // std::cout<<"EpcHssApplication::showSimCard"<<std::endl;

  std::map<uint64_t, Ptr<SimCard> >::iterator it = m_SimCardMap.begin();

  while(it != m_SimCardMap.end()){
    // std::cout<<it->first<<" :: zone :"<<it->second->getZone()<<std::endl;
    it++;
  }

};

void
EpcHssApplication::syncUeInfor (uint64_t imsi, Ptr<EpcMmeApplicationExt::UeInfo> ueInfor)
{
  std::map<uint64_t, Ptr<SimCard> >::iterator it = m_SimCardMap.find (imsi);
  NS_ASSERT_MSG (it != m_SimCardMap.end (), "could not find any UE with IMSI " << imsi);

  // it->second->setUeInfor(ueInfor);
};

void
EpcHssApplication::registerSimCard(uint64_t m_imsi, uint16_t m_zone)
{

  Ptr<SimCard> sim = Create<SimCard> ();
  sim->setImsi(m_imsi);
  sim->setZone(m_zone);
  m_SimCardMap[m_imsi] = sim;
}

void
EpcHssApplication::AddMme (Ipv4Address mmeS6sAddr, Ipv4Address hssS6Addr, Ptr<Socket> s6aSocket)
{
  // std::cout<<"EpcHssApplication::AddMme"<<std::endl;
  m_s6aSocket = s6aSocket;
  m_mmeS6aAddr = mmeS6sAddr;
  m_hssS6aAddr = hssS6Addr;
  m_s6aSocket->SetRecvCallback (MakeCallback (&EpcHssApplication::RecvFromS6aSocket, this));
}

void
EpcHssApplication::AddMmeRoaming (Ipv4Address mmeS6dAddr, Ipv4Address hssS6dAddr, Ptr<Socket> s6dSocket)
{
  // std::cout<<"EpcHssApplication::AddMmeRoaming"<<std::endl;
  m_s6dSocket = s6dSocket;
  m_mmeS6dAddr = mmeS6dAddr;
  m_hssS6dAddr = hssS6dAddr;
  m_s6dSocket->SetRecvCallback (MakeCallback (&EpcHssApplication::RecvFromS6dSocket, this)); 
};


void
EpcHssApplication::RecvFromS6aSocket (Ptr<Socket> socket)
{
  // std::cout<<"EpcHssApplication::RecvFromS6aSocket"<<std::endl;
  NS_LOG_FUNCTION (this << socket);
  NS_ASSERT (socket == m_s6aSocket);
  Ptr<Packet> packet = socket->Recv ();
  
  // PacketTagIterator i = packet->GetPacketTagIterator();
  SimTagImsi simTagImsi;

  bool ckImis = packet->PeekPacketTag(simTagImsi);
  NS_ASSERT_MSG (ckImis, "imis has problen");

  SimTagZone simTagZone;
  bool ckZone = packet->PeekPacketTag(simTagZone);
  NS_ASSERT_MSG (ckZone, "zone has problen");

  // std::cout<<simTagImsi.getImsi()<<" vs "<<simTagImsi.getImsi()<<" " <<ckImis<<std::endl;

  registerSimCard(simTagImsi.getImsi(), simTagImsi.getImsi());
}

void
EpcHssApplication::RecvFromS6dSocket (Ptr<Socket> socket)
{
  // std::cout<<"EpcHssApplication::RecvFromS6dSocket"<<std::endl;
  NS_LOG_FUNCTION (this << socket);
  NS_ASSERT_MSG (socket == m_s6dSocket,"socket is not s6dsocker");
  Ptr<Packet> packet = socket->Recv ();
  // std::cout<<"pass1"<<std::endl;
  // PacketTagIterator i = packet->GetPacketTagIterator();
  SimTagImsi simTagImsi;

  bool ckImis = packet->PeekPacketTag(simTagImsi);
  NS_ASSERT_MSG (ckImis, "imis has problen");
  // std::cout<<"pass2"<<std::endl;

  // SimTagZone simTagZone;
  // bool ckZone = packet->PeekPacketTag(simTagZone);
  // NS_ASSERT_MSG (ckZone, "zone has problen");
  uint16_t imsi = simTagImsi.getImsi();
  std::map<uint64_t, Ptr<SimCard> >::iterator it = m_SimCardMap.find (imsi);
  NS_ASSERT_MSG (it != m_SimCardMap.end (), "could not find any UE with IMSI " << imsi);
  
  Ptr<Packet> packetS6d = Create <Packet> ();
  // std::cout<<"pass3"<<std::endl;

  simTagImsi.setImsi(imsi);

  SimTagZone simTagZone;
  simTagZone.setZone(it->second->getZone());
  // std::cout<<"pass4"<<std::endl;

  SimTagRoaming simTagRoaming;
  simTagRoaming.setRoaming(1);
  
  packetS6d->AddPacketTag (simTagImsi);
  packetS6d->AddPacketTag (simTagZone);
  packetS6d->AddPacketTag (simTagRoaming);

    // std::cout<<"pass5"<<std::endl;

  m_s6dSocket->SendTo (packetS6d, 0, InetSocketAddress (m_mmeS6dAddr, m_gtpcUdpPort));
  // std::cout<<"Sent:"<< simTagImsi.getImsi() <<" "<<simTagRoaming.getRoaming()<<std::endl;
}

} // namespace ns3
