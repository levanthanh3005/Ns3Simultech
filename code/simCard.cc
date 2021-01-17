#include "ns3/core-module.h"
#include "ns3/application.h"

#include <iostream>
#include <vector>
#include <stdio.h>
#include <iomanip>


#include "simCard.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SimCard");

NS_OBJECT_ENSURE_REGISTERED (SimCard);


TypeId
SimCard::GetTypeId (void)
{
  // NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_FUNCTION ("SimCard::GetTypeId");

  static TypeId tid = TypeId ("ns3::SimCard")
    .SetParent<Object> ()
    .SetGroupName ("EpcGroup")
    .AddConstructor<SimCard>()
  ;
  return tid;
}

TypeId
SimCard::GetInstanceTypeId () const
{
  return GetTypeId ();
}

SimCard::SimCard () 
{

}

SimCard::~SimCard () 
{

}

uint16_t 
SimCard::getZone()
{
  return zone;
};

void 
SimCard::setZone(uint16_t m_zone)
{
  zone = m_zone;
};

uint64_t 
SimCard::getImsi()
{
  return imsi;
};

void 
SimCard::setImsi(uint64_t m_imsi)
{
  imsi = m_imsi;
};


} // namespace ns3