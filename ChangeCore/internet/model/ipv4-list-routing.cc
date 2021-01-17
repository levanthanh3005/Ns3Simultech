/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 University of Washington
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
 */

#include "ns3/log.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-route.h"
#include "ns3/node.h"
#include "ns3/ipv4-static-routing.h"

#include "ns3/core-module.h"

#include "ipv4-list-routing.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Ipv4ListRouting");

NS_OBJECT_ENSURE_REGISTERED (Ipv4ListRouting);

static std::map<std::string, Ipv4ListRouting::FwdPk> fwdPkMap;    

TypeId
Ipv4ListRouting::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Ipv4ListRouting")
    .SetParent<Ipv4RoutingProtocol> ()
    .SetGroupName ("Internet")
    .AddConstructor<Ipv4ListRouting> ()
  ;
  return tid;
}


Ipv4ListRouting::Ipv4ListRouting () 
  : m_ipv4 (0)
{
  NS_LOG_FUNCTION (this);
}

Ipv4ListRouting::~Ipv4ListRouting () 
{
  NS_LOG_FUNCTION (this);
}

void
Ipv4ListRouting::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  for (Ipv4RoutingProtocolList::iterator rprotoIter = m_routingProtocols.begin ();
       rprotoIter != m_routingProtocols.end (); rprotoIter++)
    {
      // Note:  Calling dispose on these protocols causes memory leak
      //        The routing protocols should not maintain a pointer to
      //        this object, so Dispose() shouldn't be necessary.
      (*rprotoIter).second = 0;
    }
  m_routingProtocols.clear ();
  m_ipv4 = 0;
}

void
Ipv4ListRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit) const
{
  NS_LOG_FUNCTION (this << stream);
  *stream->GetStream () << "Node: " << m_ipv4->GetObject<Node> ()->GetId () 
                        << ", Time: " << Now().As (unit)
                        << ", Local time: " << GetObject<Node> ()->GetLocalTime ().As (unit)
                        << ", Ipv4ListRouting table" << std::endl;
  for (Ipv4RoutingProtocolList::const_iterator i = m_routingProtocols.begin ();
       i != m_routingProtocols.end (); i++)
    {
      *stream->GetStream () << "  Priority: " << (*i).first << " Protocol: " << (*i).second->GetInstanceTypeId () << std::endl;
      (*i).second->PrintRoutingTable (stream, unit);
    }
}

void
Ipv4ListRouting::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  for (Ipv4RoutingProtocolList::iterator rprotoIter = m_routingProtocols.begin ();
       rprotoIter != m_routingProtocols.end (); rprotoIter++)
    {
      Ptr<Ipv4RoutingProtocol> protocol = (*rprotoIter).second;
      protocol->Initialize ();
    }
  Ipv4RoutingProtocol::DoInitialize ();
}


Ptr<Ipv4Route>
Ipv4ListRouting::RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, enum Socket::SocketErrno &sockerr)
{
  // std::cout<<"Ipv4ListRouting::RouteOutput::"<<Names::FindName(m_ipv4->GetObject<Node> ())<<std::endl;
  // std::cout<<this <<" "<< p <<" "<< header.GetDestination () <<" "<< header.GetSource () <<" "<< oif <<" "<< sockerr <<std::endl;

  NS_LOG_FUNCTION (this << p << header.GetDestination () << header.GetSource () << oif << sockerr);
  Ptr<Ipv4Route> route;

  for (Ipv4RoutingProtocolList::const_iterator i = m_routingProtocols.begin ();
       i != m_routingProtocols.end (); i++)
    {
      NS_LOG_LOGIC ("Checking protocol " << (*i).second->GetInstanceTypeId () << " with priority " << (*i).first);
      NS_LOG_LOGIC ("Requesting source address for destination " << header.GetDestination ());
      route = (*i).second->RouteOutput (p, header, oif, sockerr);
      if (route)
        {
          NS_LOG_LOGIC ("Found route " << route);

          // std::cout<< "Found route " << route <<std::endl;
          sockerr = Socket::ERROR_NOTERROR;
          return route;
        }
    }
  NS_LOG_LOGIC ("Done checking " << GetTypeId ());
  NS_LOG_LOGIC ("");
  // std::cout<<"Socket::ERROR_NOROUTETOHOST"<<std::endl;
  sockerr = Socket::ERROR_NOROUTETOHOST;
  return 0;
}

// Patterned after Linux ip_route_input and ip_route_input_slow
bool 
Ipv4ListRouting::RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev, 
                             UnicastForwardCallback ucb, MulticastForwardCallback mcb, 
                             LocalDeliverCallback lcb, ErrorCallback ecb)
{
  // std::cout<<"Ipv4ListRouting::RouteInput::"<<Names::FindName(m_ipv4->GetObject<Node> ())<<std::endl;
  // std::cout<<p->ToString()<<std::endl;
  // std::cout<<header.GetSource ()<<"::"<<header.GetDestination ()<<std::endl;
  
  NS_LOG_FUNCTION (this << p << header << idev << &ucb << &mcb << &lcb << &ecb);
  bool retVal = false;
  NS_LOG_LOGIC ("RouteInput logic for node: " << m_ipv4->GetObject<Node> ()->GetId ());

  NS_ASSERT (m_ipv4 != 0);
  // Check if input device supports IP 
  NS_ASSERT (m_ipv4->GetInterfaceForDevice (idev) >= 0);
  uint32_t iif = m_ipv4->GetInterfaceForDevice (idev); 

  bool forwardCheck;

  forwardCheck = m_ipv4->IsDestinationAddress (header.GetDestination (), iif);
  

  // if (forwardCheck == false) {
  Ipv4ListRouting::FwdPk fwd = Ipv4ListRouting::FindFwdPk(header.GetSource (), header.GetDestination ());
  // std::cout<<"fwd::exist::"<<fwd.exist<<"  newDes:"<<fwd.newDestination<<" fwd.forwardCheck::"<< fwd.forwardCheck<<" forwardCheck::"<<forwardCheck<<" iif::"<<iif <<std::endl;

  Ipv4Header fHeader = header;

  if (fwd.exist == true && fwd.forwardCheck == forwardCheck) {
    // std::cout<<"route to::"<<fwd.newDestination<<" with source::"<< fwd.newSource <<" gate :"<< fwd.index <<std::endl;

    fHeader.SetDestination (Ipv4Address(fwd.newDestination.c_str()));
    fHeader.SetSource (Ipv4Address(fwd.newSource.c_str()));

    if (fwd.changeRoute) {
      // std::cout<<"send to ucb"<<std::endl;
      Ptr<Ipv4Route> route = Create<Ipv4Route> ();
      route->SetDestination(fHeader.GetDestination());
      route->SetSource(fHeader.GetSource());
      route->SetGateway(fHeader.GetDestination());
      route->SetOutputDevice(m_ipv4->GetNetDevice(fwd.index));
      ucb(route, p, fHeader);
      return true;
    } 
    // else {
    //   std::cout<<"send to lcb"<<std::endl;
    //   lcb (p, fHeader, fwd.index);
    //   return true;
    // }
  }

  // std::cout<<"After change:"<<fHeader.GetSource ()<<"::"<<fHeader.GetDestination ()<<std::endl;

  // }


  // if ((header.GetSource ().IsEqual(Ipv4Address("10.0.5.8")))&&
  //         (header.GetDestination ().IsEqual(Ipv4Address ("10.0.4.8")))&&
  //         (forwardCheck == false))
  //   {
  //         Ipv4Header fHeader = header;
  //         fHeader.SetDestination (Ipv4Address("7.0.0.2"));

  //         Ptr<Ipv4Route> route = Create<Ipv4Route> ();
  //         route->SetDestination(fHeader.GetDestination());
  //         route->SetSource(fHeader.GetSource());
  //         route->SetGateway(fHeader.GetDestination());
  //         route->SetOutputDevice(m_ipv4->GetNetDevice(2));
  //         ucb(route, p, fHeader);
  //         return true;
  //     }


  // if ((header.GetSource ().IsEqual(Ipv4Address("10.0.5.8")))&&
  //         (header.GetDestination ().IsEqual(Ipv4Address ("7.0.0.2")))&&
  //         (forwardCheck == true))
  //   {
  //         Ipv4Header fHeader = header;
  //         fHeader.SetDestination (Ipv4Address("10.0.4.8"));

  //         Ptr<Ipv4Route> route = Create<Ipv4Route> ();
  //         route->SetDestination(fHeader.GetDestination());
  //         route->SetSource(fHeader.GetSource());
  //         route->SetGateway(fHeader.GetDestination());
  //         route->SetOutputDevice(m_ipv4->GetNetDevice(2));
  //         ucb(route, p, fHeader);
  //         return true;
  //     }


  // if ((header.GetSource ().IsEqual(Ipv4Address("10.0.4.8")))
  //         &&
  //         (header.GetDestination ().IsEqual(Ipv4Address ("10.0.5.8")))&&
  //         (forwardCheck == false))
  //   {
  //         Ipv4Header fHeader = header;
  //         fHeader.SetDestination (Ipv4Address("1.0.0.1"));

  //         Ptr<Ipv4Route> route = Create<Ipv4Route> ();
  //         route->SetDestination(fHeader.GetDestination());
  //         route->SetSource(fHeader.GetSource());
  //         route->SetGateway(fHeader.GetDestination());
  //         route->SetOutputDevice(m_ipv4->GetNetDevice(1));
  //         ucb(route, p, fHeader);
  //         return true;
  //     }

  // if ((header.GetSource ().IsEqual(Ipv4Address("10.0.4.8")))
  //         &&
  //         (header.GetDestination ().IsEqual(Ipv4Address ("1.0.0.1")))&&
  //         (forwardCheck == true))
  //   {
  //         Ipv4Header fHeader = header;
  //         fHeader.SetDestination (Ipv4Address("10.0.5.8"));
  //         //fHeader.SetSource (Ipv4Address("10.0.4.3"));

  //         Ptr<Ipv4Route> route = Create<Ipv4Route> ();
  //         route->SetDestination(fHeader.GetDestination());
  //         route->SetSource(fHeader.GetSource());          
  //         route->SetGateway(fHeader.GetDestination());
  //         route->SetOutputDevice(m_ipv4->GetNetDevice(1));
  //         ucb(route, p, fHeader);
  //         return true;
  //     }

  retVal = m_ipv4->IsDestinationAddress (fHeader.GetDestination (), iif);
  if (retVal == true)
    {
      NS_LOG_LOGIC ("Address "<< fHeader.GetDestination () << " is a match for local delivery");

      // std::cout<<"Address "<< fHeader.GetDestination () << " is a match for local delivery"<<std::endl;
      if (fHeader.GetDestination ().IsMulticast ())
        {
          Ptr<Packet> packetCopy = p->Copy ();
          lcb (packetCopy, fHeader, iif);
          retVal = true;
          // Fall through
        }
      else
        {
          // std::cout<<"Send via lcb :: line268"<<std::endl;
          lcb (p, fHeader, iif);
          return true;
        }
    }
  // Check if input device supports IP forwarding
  if (m_ipv4->IsForwarding (iif) == false)
    {
      // std::cout<<("Forwarding disabled for this interface")<<std::endl;

      NS_LOG_LOGIC ("Forwarding disabled for this interface");
      ecb (p, fHeader, Socket::ERROR_NOROUTETOHOST);
      return true;
    }
  // Next, try to find a route
  // If we have already delivered a packet locally (e.g. multicast)
  // we suppress further downstream local delivery by nulling the callback
  LocalDeliverCallback downstreamLcb = lcb;
  if (retVal == true)
    {
      downstreamLcb = MakeNullCallback<void, Ptr<const Packet>, const Ipv4Header &, uint32_t > ();
    }
  for (Ipv4RoutingProtocolList::const_iterator rprotoIter =
         m_routingProtocols.begin ();
       rprotoIter != m_routingProtocols.end ();
       rprotoIter++)
    {
      if ((*rprotoIter).second->RouteInput (p, fHeader, idev, ucb, mcb, downstreamLcb, ecb))
        {
          // std::cout<< "Route found to forward packet in protocol " << (*rprotoIter).second->GetInstanceTypeId ().GetName () <<std::endl; 

          NS_LOG_LOGIC ("Route found to forward packet in protocol " << (*rprotoIter).second->GetInstanceTypeId ().GetName ()); 
          return true;
        }
    }
  // No routing protocol has found a route.
  // std::cout<<"No routing protocol has found a route"<<std::endl;
  return retVal;
}

void
Ipv4ListRouting::AddFwdPk(std::string m_source, std::string m_destination, std::string m_newSource ,std::string m_newDestination , uint32_t m_index, std::string m_name, bool m_forwardCheck, bool m_changeRoute)
{
  FwdPk fwd;
  fwd.source = m_source;
  fwd.destination = m_destination;
  fwd.newSource = m_newSource;
  fwd.newDestination = m_newDestination;
  fwd.index = m_index;
  fwd.name = m_name;
  fwd.exist = true;
  fwd.forwardCheck = m_forwardCheck;
  fwd.changeRoute = m_changeRoute;
  AddFwdPk(fwd);
};

void
Ipv4ListRouting::AddFwdPk(std::string m_source, std::string m_destination, std::string m_newDestination , uint32_t m_index, std::string m_name, bool m_forwardCheck)
{
  FwdPk fwd;
  fwd.source = m_source;
  fwd.destination = m_destination;
  fwd.newSource = m_source;
  fwd.newDestination = m_newDestination;
  fwd.index = m_index;
  fwd.name = m_name;
  fwd.exist = true;
  fwd.forwardCheck = m_forwardCheck;
  fwd.changeRoute = true;
  AddFwdPk(fwd);
};


void
Ipv4ListRouting::AddFwdPk(Ipv4ListRouting::FwdPk fwd)
{
  fwd.exist = true;
  fwdPkMap[fwd.source+std::string("=>")+fwd.destination] = fwd;
};

void
Ipv4ListRouting::ModifyFwdPk(std::string name, Ipv4ListRouting::FwdPk fwd)
{
  // std::cout<<"Ipv4ListRouting::ModifyFwdPk"<<std::endl;
  // std::map<std::string, FwdPk >::iterator it = fwdPkMap.find (name);

  // if (it != fwdPkMap.end ()) {
  //   std::cout<<"Ipv4ListRouting::ModifyFwdPk::modified"<<std::endl;
  //   fwdPkMap[it->first] = fwd;
  // }

  for (std::map<std::string, FwdPk >::iterator it=fwdPkMap.begin(); it!=fwdPkMap.end(); ++it)
  {
    if(it->second.name == name) {
      // std::cout<<"Ipv4ListRouting::ModifyFwdPk::modified"<<std::endl;
      fwdPkMap[it->first] = fwd;
    }
  }

};

Ipv4ListRouting::FwdPk
Ipv4ListRouting::FindFwdPk(std::string source, std::string destination)
{

  std::map<std::string, FwdPk >::iterator it = fwdPkMap.find (std::string(source+std::string("=>")+destination));

  if (it != fwdPkMap.end ()) {
    return it->second;
  }

  FwdPk fwd;
  fwd.exist = false;
  return fwd;
};

Ipv4ListRouting::FwdPk
Ipv4ListRouting::FindFwdPk(Ipv4Address ipv4source, Ipv4Address ipv4destination)
{
  for (std::map<std::string, FwdPk >::iterator it=fwdPkMap.begin(); it!=fwdPkMap.end(); ++it)
  {
    if (ipv4source.IsEqual(Ipv4Address(it->second.source.c_str()))&&
        ipv4destination.IsEqual(Ipv4Address (it->second.destination.c_str()))) {
          return it->second;
        }
  }
  FwdPk fwd;
  fwd.exist = false;
  return fwd;
};

Ipv4ListRouting::FwdPk
Ipv4ListRouting::FindFwdPk(std::string key)
{
  std::map<std::string, FwdPk >::iterator it = fwdPkMap.find (key);

  if (it != fwdPkMap.end ()) {
    return it->second;
  }

  FwdPk fwd;
  fwd.exist = false;
  return fwd;
};

Ipv4ListRouting::FwdPk
Ipv4ListRouting::FindFwdPkByName(std::string name)
{
  for (std::map<std::string, FwdPk >::iterator it=fwdPkMap.begin(); it!=fwdPkMap.end(); ++it)
  {
    // std::cout<<it->first<<" "<<it->second.OldAddr<<" "<< it->second.teid <<std::endl;
    if(it->second.name == name) {
      return it->second;
    }
  }
  FwdPk fwd;
  fwd.exist = false;
  return fwd;
};

void
Ipv4ListRouting::showFwdPkMap()
{
  std::cout<<"Ipv4ListRouting::showFwdPkMap()"<<std::endl;

  for (std::map<std::string, FwdPk >::iterator it=fwdPkMap.begin(); it!=fwdPkMap.end(); ++it)
  {
    std::cout<<it->first<<std::endl;
    std::cout<<it->second.source<<std::endl;    
    std::cout<<it->second.destination<<std::endl;    
    std::cout<<it->second.newDestination<<std::endl;
    std::cout<<it->second.index<<std::endl;    
    std::cout<<it->second.forwardCheck<<std::endl;    
    std::cout<<it->second.exist<<std::endl;    
  }
}

void 
Ipv4ListRouting::NotifyInterfaceUp (uint32_t interface)
{
  NS_LOG_FUNCTION (this << interface);
  for (Ipv4RoutingProtocolList::const_iterator rprotoIter =
         m_routingProtocols.begin ();
       rprotoIter != m_routingProtocols.end ();
       rprotoIter++)
    {
      (*rprotoIter).second->NotifyInterfaceUp (interface);
    }
}
void 
Ipv4ListRouting::NotifyInterfaceDown (uint32_t interface)
{
  NS_LOG_FUNCTION (this << interface);
  for (Ipv4RoutingProtocolList::const_iterator rprotoIter =
         m_routingProtocols.begin ();
       rprotoIter != m_routingProtocols.end ();
       rprotoIter++)
    {
      (*rprotoIter).second->NotifyInterfaceDown (interface);
    }
}
void 
Ipv4ListRouting::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION (this << interface << address);
  for (Ipv4RoutingProtocolList::const_iterator rprotoIter =
         m_routingProtocols.begin ();
       rprotoIter != m_routingProtocols.end ();
       rprotoIter++)
    {
      (*rprotoIter).second->NotifyAddAddress (interface, address);
    }
}
void 
Ipv4ListRouting::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION (this << interface << address);
  for (Ipv4RoutingProtocolList::const_iterator rprotoIter =
         m_routingProtocols.begin ();
       rprotoIter != m_routingProtocols.end ();
       rprotoIter++)
    {
      (*rprotoIter).second->NotifyRemoveAddress (interface, address);
    }
}
void 
Ipv4ListRouting::SetIpv4 (Ptr<Ipv4> ipv4)
{
  NS_LOG_FUNCTION (this << ipv4);
  NS_ASSERT (m_ipv4 == 0);
  for (Ipv4RoutingProtocolList::const_iterator rprotoIter =
         m_routingProtocols.begin ();
       rprotoIter != m_routingProtocols.end ();
       rprotoIter++)
    {
      (*rprotoIter).second->SetIpv4 (ipv4);
    }
  m_ipv4 = ipv4;
}

void
Ipv4ListRouting::AddRoutingProtocol (Ptr<Ipv4RoutingProtocol> routingProtocol, int16_t priority)
{
  NS_LOG_FUNCTION (this << routingProtocol->GetInstanceTypeId () << priority);
  m_routingProtocols.push_back (std::make_pair (priority, routingProtocol));
  m_routingProtocols.sort ( Compare );
  if (m_ipv4 != 0)
    {
      routingProtocol->SetIpv4 (m_ipv4);
    }
}

uint32_t 
Ipv4ListRouting::GetNRoutingProtocols (void) const
{
  NS_LOG_FUNCTION (this);
  return m_routingProtocols.size (); 
}

Ptr<Ipv4RoutingProtocol> 
Ipv4ListRouting::GetRoutingProtocol (uint32_t index, int16_t& priority) const
{
  NS_LOG_FUNCTION (this << index << priority);
  if (index > m_routingProtocols.size ())
    {
      NS_FATAL_ERROR ("Ipv4ListRouting::GetRoutingProtocol():  index " << index << " out of range");
    }
  uint32_t i = 0;
  for (Ipv4RoutingProtocolList::const_iterator rprotoIter = m_routingProtocols.begin ();
       rprotoIter != m_routingProtocols.end (); rprotoIter++, i++)
    {
      if (i == index)
        {
          priority = (*rprotoIter).first;
          return (*rprotoIter).second;
        }
    }
  return 0;
}

bool 
Ipv4ListRouting::Compare (const Ipv4RoutingProtocolEntry& a, const Ipv4RoutingProtocolEntry& b)
{
  NS_LOG_FUNCTION_NOARGS ();
  return a.first > b.first;
}


} // namespace ns3

