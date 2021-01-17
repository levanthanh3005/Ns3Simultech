#ifndef APPLICATION_CONTROLLER_H
#define APPLICATION_CONTROLLER_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include <iostream>
#include <vector>
#include <stdio.h>
#include <iomanip>

#include "point-to-point-epc-helper-ext.h"
#include "epc-group-helper.h"
#include "epc-global-helper.h"
#include "ue-controller.h"

namespace ns3 {

class AppController : public Object
{
public:
  /**
   * Constructor
   */
  AppController ();

  virtual ~AppController ();

  static TypeId GetTypeId (void);

  TypeId GetInstanceTypeId () const;

  // virtual void runSimpleApp();

  virtual void runSimpleApp(Ptr<UeController> ueController,Ptr<EpcGlobalHelper> epcGlobalHelper);

  virtual void runSimpleApp(Ptr<Node> ue,Ptr<EpcGroupHelper> epcHelper);

  virtual void runOnOffApp(Ptr<UeController> ueController,Ptr<EpcGlobalHelper> epcGlobalHelper);

  virtual void stopSimpleApp(Ptr<EpcGroupHelper> epcHelper,Ptr<Node> ueNode);
  
  virtual void printRoutingTable(Ptr<Ipv4StaticRouting> sr);

  virtual void pingApp(Ptr<Node> fromNode, std::string toAddr);

private:    
  uint16_t index = 0; 

};

} // namespace ns3

#endif // POINT_TO_POINT_EPC_HELPER_H