#ifndef SIMTAG_H
#define SIMTAG_H

#include "ns3/epc-gtpc-header.h"
#include "ns3/core-module.h"
#include "ns3/application.h"
#include "ns3/tag.h"

#include <iostream>
#include <vector>
#include <stdio.h>
#include <iomanip>

// #include "epc-mme-application-ext.h"


namespace ns3 {

class SimTagZone : public Tag
{
public:
  SimTagZone ();
 
  static TypeId GetTypeId (void);

  // inherited function, no need to doc.
  virtual TypeId GetInstanceTypeId (void) const;

  // inherited function, no need to doc.
  virtual uint32_t GetSerializedSize (void) const;

  // inherited function, no need to doc.
  virtual void Serialize (TagBuffer i) const;

  // inherited function, no need to doc.
  virtual void Deserialize (TagBuffer i);

  // inherited function, no need to doc.
  virtual void Print (std::ostream &os) const;

  virtual uint16_t getZone();

  virtual void setZone(uint16_t zone);

private:
  uint16_t zone;
};

class SimTagImsi : public Tag
{
public:
  SimTagImsi ();

  static TypeId GetTypeId (void);

  // inherited function, no need to doc.
  virtual TypeId GetInstanceTypeId (void) const;

  // inherited function, no need to doc.
  virtual uint32_t GetSerializedSize (void) const;

  // inherited function, no need to doc.
  virtual void Serialize (TagBuffer i) const;

  // inherited function, no need to doc.
  virtual void Deserialize (TagBuffer i);

  // inherited function, no need to doc.
  virtual void Print (std::ostream &os) const;

  virtual uint64_t getImsi();

  virtual void setImsi(uint64_t imsi);


private:
  uint64_t imsi;
};

class SimTagRoaming : public Tag
{
public:
  SimTagRoaming ();
 
  static TypeId GetTypeId (void);

  // inherited function, no need to doc.
  virtual TypeId GetInstanceTypeId (void) const;

  // inherited function, no need to doc.
  virtual uint32_t GetSerializedSize (void) const;

  // inherited function, no need to doc.
  virtual void Serialize (TagBuffer i) const;

  // inherited function, no need to doc.
  virtual void Deserialize (TagBuffer i);

  // inherited function, no need to doc.
  virtual void Print (std::ostream &os) const;

  virtual uint16_t getRoaming();

  virtual void setRoaming(uint16_t roaming);

private:
  uint16_t roaming = 0;
};

} // namespace ns3

#endif // POINT_TO_POINT_EPC_HELPER_H