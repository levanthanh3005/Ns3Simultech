#ifndef LTE_REPORTER_H
#define LTE_REPORTER_H

#include "ns3/core-module.h"

#include <iostream>

namespace ns3 {

class LteReporter : public Object
{
public:
  /**
   * Constructor
   */
  LteReporter ();

  virtual ~LteReporter ();

  static TypeId GetTypeId (void);

  TypeId GetInstanceTypeId () const;

  static uint16_t runCurl(std::string cmd);

  static uint16_t sshDocker(std::string serviceName, std::string path);

  virtual void setEnableReport(bool ck);
  
  static uint16_t routing(std::string serviceName, uint16_t index);

  static std::string exec(std::string command);
private:    


  uint16_t startingPoint = 20;
  bool enableReport = false;

};

} // namespace ns3

#endif // POINT_TO_POINT_EPC_HELPER_H