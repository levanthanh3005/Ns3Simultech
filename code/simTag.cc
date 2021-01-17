#include "ns3/core-module.h"
#include "ns3/application.h"

#include <iostream>
#include <vector>
#include <stdio.h>
#include <iomanip>


#include "simTag.h"

namespace ns3 {

TypeId
SimTagZone::GetTypeId (void)
{
  // NS_LOG_FUNCTION_NOARGS ();
  // NS_LOG_FUNCTION ("SimTagZone::GetTypeId");

  static TypeId tid = TypeId ("ns3::SimTagZone")
    .SetParent<Object> ()
    .SetGroupName ("EpcGroup")
    .AddConstructor<SimTagZone>()
  ;
  return tid;
}

TypeId
SimTagZone::GetInstanceTypeId () const
{
  return GetTypeId ();
}

SimTagZone::SimTagZone () 
{

}

uint32_t
SimTagZone::GetSerializedSize (void) const
{
  return sizeof (uint16_t);
}

void
SimTagZone::Serialize (TagBuffer i) const
{
  i.WriteU16 (zone);
}

void
SimTagZone::Deserialize (TagBuffer i)
{
  zone = i.ReadU16();
}
void
SimTagZone::Print (std::ostream &os) const
{
  // os << "IPV6_TCLASS = " << m_ipv6Tclass;
}

uint16_t 
SimTagZone::getZone()
{
  return zone;
};

void 
SimTagZone::setZone(uint16_t m_zone)
{
  zone = m_zone;
};

///////////////

TypeId
SimTagImsi::GetTypeId (void)
{
  // NS_LOG_FUNCTION_NOARGS ();
  // NS_LOG_FUNCTION ("SimTagImsi::GetTypeId");

  static TypeId tid = TypeId ("ns3::SimTagImsi")
    .SetParent<Object> ()
    .SetGroupName ("EpcGroup")
    .AddConstructor<SimTagImsi>()
  ;
  return tid;
}

TypeId
SimTagImsi::GetInstanceTypeId () const
{
  return GetTypeId ();
}

SimTagImsi::SimTagImsi () 
{

}

uint32_t
SimTagImsi::GetSerializedSize (void) const
{
  return sizeof (uint64_t);
}

void
SimTagImsi::Serialize (TagBuffer i) const
{
  i.WriteU64 (imsi);
}

void
SimTagImsi::Deserialize (TagBuffer i)
{
  imsi = i.ReadU64();
}
void
SimTagImsi::Print (std::ostream &os) const
{
  // os << "IPV6_TCLASS = " << m_ipv6Tclass;
}

uint64_t 
SimTagImsi::getImsi()
{
  return imsi;
};

void 
SimTagImsi::setImsi(uint64_t m_imsi)
{
  imsi = m_imsi;
};

///////////////////

TypeId
SimTagRoaming::GetTypeId (void)
{
  // NS_LOG_FUNCTION_NOARGS ();
  // NS_LOG_FUNCTION ("SimTagRoaming::GetTypeId");

  static TypeId tid = TypeId ("ns3::SimTagRoaming")
    .SetParent<Object> ()
    .SetGroupName ("EpcGroup")
    .AddConstructor<SimTagRoaming>()
  ;
  return tid;
}

TypeId
SimTagRoaming::GetInstanceTypeId () const
{
  return GetTypeId ();
}

SimTagRoaming::SimTagRoaming () 
{

}

uint32_t
SimTagRoaming::GetSerializedSize (void) const
{
  return sizeof (uint16_t);
}

void
SimTagRoaming::Serialize (TagBuffer i) const
{
  i.WriteU16 (roaming);
}

void
SimTagRoaming::Deserialize (TagBuffer i)
{
  roaming = i.ReadU16();
}
void
SimTagRoaming::Print (std::ostream &os) const
{
  // os << "IPV6_TCLASS = " << m_ipv6Tclass;
}

uint16_t 
SimTagRoaming::getRoaming()
{
  return roaming;
};

void 
SimTagRoaming::setRoaming(uint16_t m_Roaming)
{
  roaming = m_Roaming;
};

} // namespace ns3