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

#ifndef EPC_HSS_APPLICATION_H
#define EPC_HSS_APPLICATION_H

#include "ns3/log.h"

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/epc-s1ap-sap.h"

#include "simCard.h"
// #include "simTag.h"

// #include "epc-mme-application-ext.h"

namespace ns3 {

// class SimTagImsi;
// class SimTagZone;


/**
 * \ingroup lte
 *
 * This application implements the Mobility Management Entity (MME) according to
 * the 3GPP TS 23.401 document.
 *
 * This Application implements the MME side of the S1-MME interface between
 * the MME node and the eNB nodes and the MME side of the S11 interface between
 * the MME node and the SGW node. It supports the following functions and messages:
 *
 *  - Bearer management functions including dedicated bearer establishment
 *  - NAS signalling
 *  - Tunnel Management messages
 *
 * Others functions enumerated in section 4.4.2 of 3GPP TS 23.401 are not supported.
 */
class EpcHssApplication : public Application
{

public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  /** Constructor */
  EpcHssApplication ();

  /** Destructor */
  virtual ~EpcHssApplication ();

  virtual void showSimCard ();

  virtual void syncUeInfor (uint64_t imsi, Ptr<EpcMmeApplicationExt::UeInfo> ueInfor);

  virtual void registerSimCard(uint64_t imsi, uint16_t zone);

  virtual void RecvFromS6aSocket (Ptr<Socket> socket);

  virtual void RecvFromS6dSocket (Ptr<Socket> socket);

  virtual void AddMme (Ipv4Address mmeS6aAddr, Ipv4Address hssS6aAddr, Ptr<Socket> s6aSocket);

  virtual void AddMmeRoaming (Ipv4Address mmeS6dAddr, Ipv4Address hssS6dAddr, Ptr<Socket> s6dSocket);


private:

  std::map<uint64_t, Ptr<SimCard> > m_SimCardMap;    

  uint16_t m_gtpcUdpPort; ///< UDP port for GTP-C protocol. Fixed by the standard to port 2123

  Ptr<Socket> m_s6aSocket; ///< Socket to send/receive messages in the S6a interface

  Ipv4Address m_mmeS6aAddr; ///< IPv4 address of the MME S11 interface
  Ipv4Address m_hssS6aAddr; ///< IPv4 address of the SGW S11 interface

  Ptr<Socket> m_s6dSocket; ///< Socket to send/receive messages in the S6a interface

  Ipv4Address m_mmeS6dAddr; ///< IPv4 address of the MME S11 interface
  Ipv4Address m_hssS6dAddr;
};

} // namespace ns3


#endif // EPC_MME_APPLICATION_H
