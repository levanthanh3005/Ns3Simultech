#ifndef EPC_GLOBAL_HELPER_EXT_H
#define EPC_GLOBAL_HELPER_EXT_H

#include "point-to-point-epc-helper-ext.h"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"

#include <iostream>

#include "ns3/tap-bridge-module.h"

#include "ns3/csma-module.h"

#include "epc-group-helper.h"

// #include "app-controller.h"


namespace ns3 {

class EpcGlobalHelper : public Object
{
public:
  /**
   * Constructor
   */
  EpcGlobalHelper ();

  virtual ~EpcGlobalHelper ();

  virtual void setup();

  virtual void setBackground();

  static TypeId GetTypeId (void);

  TypeId GetInstanceTypeId () const;

  virtual std::vector<Ptr<EpcGroupHelper>> getEpcGroupHelpers();

  virtual Ptr<EpcGroupHelper> getEpc(uint16_t index);

  virtual void setNumOfEpc(uint16_t num);

  virtual void attachUeWithENodeB(Ptr<Node> ueNode, uint16_t currentCellId, uint16_t currentRnti, Ptr<Node> enbNode);

  // virtual void setup();

  virtual void setEpcGroupHelper(std::vector<Ptr<EpcGroupHelper>> ls);

  virtual Ptr<EpcGroupHelper> getEpcConnectedUe(Ptr<Node> ueNode);

  virtual void showEnbNodes();
  
  virtual void doRoaming(uint16_t epcHome, uint16_t epcVisit);

  virtual void SetPathlossModelType (TypeId type);

  virtual void SetPathlossModelAttribute (std::string n, const AttributeValue &v);

  virtual void SetSpectrumChannelType (std::string type);

  virtual void SetSpectrumChannelAttribute (std::string n, const AttributeValue &v);

  virtual void SetFadingModel (std::string type);

  virtual void SetFadingModelAttribute (std::string n, const AttributeValue &v);

  virtual Ptr<SpectrumChannel> GetUplinkSpectrumChannel();

  virtual Ptr<SpectrumChannel> GetDownlinkSpectrumChannel();

  virtual Ptr<Object> GetDownlinkPathlossModel();

  virtual Ptr<Object> GetUplinkPathlossModel();

  virtual Ptr<SpectrumPropagationLossModel> GetFadingModelPointer ();

  virtual void ChannelModelInitialization (void);

  virtual void setSendRequestViaRoaming(bool m_ck);
  // virtual void SetAppController (Ptr<AppController> aC);

// protected:
//   // inherited from Object
//   virtual void DoInitialize (void);

  virtual void setupCsmaServer(uint16_t epcIndex, std::string tapSrvAddressBase, std::string tapSrvRealAddress, std::string tapName, std::string serviceName, std::string controllerUrl);

  virtual void setupCsmaUe(Ptr<Node> ue);

  virtual void prepareCsmaUe(Ptr<Node> ue, std::string tapUeAddressBase, std::string tapUeRealAddress, std::string tapName, std::string serviceName, std::string controllerUrl);

  virtual void routingSrvUeTap(uint16_t epcIndex, Ptr<Node> ue);

  static std::string Ipv4AddressToString(Ipv4Address addr);

  virtual void printRoutingTable(Ptr<Ipv4StaticRouting> sr);

  virtual void pingApp(Ptr<Node> fromNode, std::string toAddr);

  virtual void setTapEnable(bool enable);

  struct NodeTap
  {
    Ptr<Node> tapNode;
    Ipv4Address ipTap;  
    Ipv4Address ipAddrBase;
    std::string tapName;
    std::string serviceName;
    std::string controllerUrl;
  };

  virtual EpcGlobalHelper::NodeTap getUeNodeTap(Ptr<Node> ue);
  virtual EpcGlobalHelper::NodeTap getSrvNodeTap(uint16_t epcIndex);

  virtual uint16_t findEpcByEnbId(uint32_t id);
  // virtual void setUniqueHostIp(std::string ip);

  static double propagationDelay(Ptr<Node> a, Ptr<Node> b);

private:

  uint16_t numOfEpc = 2;

  std::vector<Ptr<EpcGroupHelper>> epcGroupHelper;

  ObjectFactory m_channelFactory;

  std::string m_fadingModelType;
  /// Factory of fading model object for both the downlink and uplink channels.
  ObjectFactory m_fadingModelFactory;

  ObjectFactory m_pathlossModelFactory;


  /// The downlink LTE channel used in the simulation.
  Ptr<SpectrumChannel> m_downlinkChannel;
  /// The uplink LTE channel used in the simulation.
  Ptr<SpectrumChannel> m_uplinkChannel;

  Ptr<Object>  m_downlinkPathlossModel;
  /// The path loss model used in the uplink channel.
  Ptr<Object> m_uplinkPathlossModel;

  Ptr<SpectrumPropagationLossModel> m_fadingModel;

  // Ptr<AppController> appController;

  bool sendRequestViaRoaming = false;



  // struct Roaming
  // {
  //   uint16_t epcHome;
  //   uint16_t epcVisit;
  //   std::string s6dAddr;
  //   std::string s8Addr;
  //   Ipv4Address mmeVs6dAddress;
  //   Ipv4Address hssHs6dAddress;
  //   Ipv4Address pgwHS8Address;
  //   Ipv4Address sgwVS8Address;
  // };

  std::map<Ptr<Node>, NodeTap > ueTaps;

  std::map<uint16_t, NodeTap > srvTaps;

  bool tapEnable = false;


  // std::string uniqueHostIp = "";
  // CsmaHelper csmaUe;

  // std::string tapSrvAddress ="10.0.5.0";

  // std::string tapUeAddress ="10.0.5.0";
};

} // namespace ns3

#endif // POINT_TO_POINT_EPC_HELPER_H