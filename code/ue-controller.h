#ifndef UE_CONTROLLER_H
#define UE_CONTROLLER_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"

#include "ns3/traced-callback.h"

#include <iostream>

namespace ns3 {

class UeController : public Object
{
public:
  /**
   * Constructor
   */
  UeController ();

  UeController (uint16_t num, double s, double y);


  virtual ~UeController ();

  static TypeId GetTypeId (void);

  TypeId GetInstanceTypeId () const;

  virtual NetDeviceContainer getUeLteDevs();

  virtual void attachToClosestEnb();

  virtual void attachToClosestEnb(Ptr<Node> ueNode);

  virtual void refreshNodeAtPosition(Ptr<Node> ueNode, double x, double y);

  virtual void setNumOfUes(uint16_t num);

  virtual void setSpeed(double s);

  virtual void setYForUe(double y);

  virtual void createNode();

  virtual void setup();

  virtual NodeContainer getUeNodes();

  virtual Ptr<Node> getUe(uint16_t index);

  virtual void refresh();

  // virtual void refresh(uint64_t imsi, uint16_t cellId, uint16_t rnti);


  virtual void refresh(uint64_t imsi, uint16_t cellId, uint16_t rnti,uint16_t rsrq, uint16_t rsrp);

  virtual void calculateDistanceToEnbs(Ptr<Node> ueNode);

  virtual Ptr<Node> findUeWithImsi(uint64_t imsi);

  virtual Ptr<Node> findEnbWithCellId(uint16_t cellId);

  virtual void showUes();
  virtual void setStartingPoint(uint16_t stp);

  virtual Vector getUeVelocity(Ptr<Node> ueNode);
  virtual Vector getUePosition(Ptr<Node> ueNode);

private:    
  NetDeviceContainer ueLteDevs;

  uint16_t numberOfUes = 1;

  double speed = 10;       // m/s
  double yForUe = 140.0;   // m
  double xForUe = 100.0;

  NodeContainer ueNodes;

  TracedCallback<Ptr<Node>,uint16_t, uint16_t, Ptr<Node>, double> m_AttachUeToEnb;

                //ue,       enb,     ,    cellId,    rnti,  rsrq,   rsrp, distance, needMigration? 
  TracedCallback<Ptr<Node>,Ptr<Node>,uint16_t,uint16_t,double,bool, Ptr<Node>, double> m_reportUeStatus;

  TracedCallback<> m_finishSetup;

  TracedCallback<std::string> m_Reconnect;

  uint16_t startingPoint = 20;

};

} // namespace ns3

#endif // POINT_TO_POINT_EPC_HELPER_H