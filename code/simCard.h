#ifndef SIMCARD_H
#define SIMCARD_H

#include "ns3/epc-gtpc-header.h"
#include "ns3/core-module.h"
#include "ns3/application.h"
#include "ns3/tag.h"

#include <iostream>
#include <vector>
#include <stdio.h>
#include <iomanip>

#include "epc-mme-application-ext.h"


namespace ns3 {

class SimCard : public Object
{
public:
  /**
   * Constructor
   */
  SimCard ();

  virtual ~SimCard ();

  static TypeId GetTypeId (void);

  TypeId GetInstanceTypeId () const;

  virtual uint16_t getZone();

  virtual void setZone(uint16_t zone);

  virtual uint64_t getImsi();

  virtual void setImsi(uint64_t imsi);

private:    
  uint16_t zone;
  uint64_t imsi;
};

} // namespace ns3

#endif // POINT_TO_POINT_EPC_HELPER_H