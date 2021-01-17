#ifndef EPC_GROUP_HELPER_EXT_H
#define EPC_GROUP_HELPER_EXT_H

#include "point-to-point-epc-helper-ext.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"

#include "ns3/csma-module.h"

#include <iostream>

namespace ns3 {

class EpcGroupHelper : public Object
{
public:
  /**
   * Constructor
   */
  EpcGroupHelper ();

  EpcGroupHelper (uint16_t index);


  virtual ~EpcGroupHelper ();

  virtual void setup();

  static TypeId GetTypeId (void);

  TypeId GetInstanceTypeId () const;

  virtual NetDeviceContainer getEnbLteDevs();

  virtual NetDeviceContainer getUeLteDevs();

  virtual NodeContainer getEnbNodes();

  virtual NodeContainer getRemoteHostContainer();

  virtual NodeContainer getEpcContainer();

  virtual Ptr<PointToPointEpcHelperExt> getEpcHelper();

  virtual Ipv4Address getRemoteAddress();

  static void setNodePosition(Ptr<Node> node, uint16_t posx, uint16_t posy,uint16_t posz);

  virtual void setSvrIpv4Address(std::string addr);

  virtual void setUeIpv4Address(std::string addr);

  virtual void setServerTapIp(std::string addr);

  virtual void setNumberOfEnbs(uint16_t num);

  virtual void setPosition(uint16_t posx, uint16_t posy);

  virtual Ptr<LteHelper> getLteHelper();

  virtual InternetStackHelper getInternet();

  virtual Ptr<Node> getRemoteHost();

  virtual void setEnbDistance(double enbd);

  virtual void setIpWithIndex(uint16_t index);

  virtual void attachUeNode(NodeContainer ueNodes);

  virtual void detachUeNode(Ptr<Node> ueNode, uint16_t currentCellId, uint16_t currentRnti);

  virtual void stopRunningApp(Ptr<Node> ueNode);

  virtual void attachUeNode(Ptr<Node> ueNode);

  virtual void attachUeNode(Ptr<Node> ueNode, uint16_t enbIndex);

  virtual void attachUeNodeAfterChecking(Ptr<LteUeNetDevice> ptrUeLte, uint16_t enbIndex);

  virtual Ipv4InterfaceContainer getUeIpIfaces();

  virtual bool checkExistEnbCellId(uint16_t cellId);

  virtual uint16_t checkExistEnbId(uint32_t id);

  virtual Ptr<LteEnbNetDevice> findEnbWithCellId(uint16_t cellId);

  virtual void showEnbNodes();

  virtual Ipv4Address getAddressOfUe(Ptr<Node> ue);

  virtual uint32_t getRealIndexOfUe(Ptr<Node> ue);

  virtual Ptr<LteUeNetDevice> getUeLte(Ptr<Node> ue);

  // virtual void setupCsmaServer();

  // virtual void setupCsmaUe(Ptr<Node> ue);

  virtual Ipv4Address getCurrentIpv4Address(Ptr<Node> ue);

  virtual bool checkUeExistInNetwork(Ptr<Node> ue);
  
  // virtual Ptr<Node> getTapNode(Ptr<Node> ue);

  virtual void setTapEnable(bool enable);

  virtual uint16_t getEpcIndex();

  virtual void setSendRequestViaRoaming(bool m_ck);

  virtual bool getSendRequestViaRoaming();

  virtual void printRoutingTable(Ptr<Ipv4StaticRouting> sr);

  virtual void pingApp(Ptr<Node> fromNode, std::string toAddr);

  // virtual void makeForwardPacketInIpv4Routing();

  virtual std::string getSrvIpv4AddressBase();

  virtual std::string getUeIpv4AddressBase();

private:
  uint16_t epcIndex = 0;
  uint16_t numberOfEnbs = 5;
  uint16_t posX = 30;
  uint16_t posY = 30;
  std::string svrIpv4Address = "1.0.0.0";
  std::string ueIpv4Address ="7.0.0.0";


  NodeContainer remoteHostContainer;

  Ptr<PointToPointEpcHelperExt> epcHelper;// = CreateObject<PointToPointEpcHelperExt> ();

  Ptr<LteHelper> lteHelper;

  double enbDistance = 50.0; // m

  Ipv4Address remoteHostAddr;
  
  NetDeviceContainer enbLteDevs;
  NetDeviceContainer ueLteDevs;

  NodeContainer enbNodes;
  InternetStackHelper internet;

  Ptr<Node> remoteHost;

  Ptr<Node> remoteHostTap;


  Ipv4InterfaceContainer ueIpIfaces;

  Ptr<Ipv4StaticRouting> remoteHostStaticRouting;

  // CsmaHelper csmaServer;

  // Ipv4AddressHelper ipv4Csma;

  // struct UeTap
  // {
  //   Node tapNode;
  //   Ipv4Address ipTap;  
  // };

  // std::map<Ptr<Node>, Ptr<UeTap> > ueTaps;

  bool tapEnable = false;

  TracedCallback<Ptr<Node>> m_setupCsmaUe;

  // TracedCallback<uint16_t,Ptr<Node>> m_StopRealServices;


  TracedCallback<uint16_t,Ptr<Node>> m_UeConnect;

  TracedCallback<uint16_t,Ptr<Node>> m_StopRunningApp;

  bool sendRequestViaRoaming = false;

  std::string serverTapIp;
};

} // namespace ns3

#endif // POINT_TO_POINT_EPC_HELPER_H