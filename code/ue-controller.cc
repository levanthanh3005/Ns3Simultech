#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"

#include "ns3/point-to-point-module.h"

#include "ns3/config-store-module.h"

#include "ns3/netanim-module.h"

#include "ue-controller.h"

#include <sys/time.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UeController");

NS_OBJECT_ENSURE_REGISTERED (UeController);


TypeId
UeController::GetTypeId (void)
{
  // NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_FUNCTION ("UeController::GetTypeId");

  static TypeId tid = TypeId ("ns3::UeController")
    .SetParent<Object> ()
    .SetGroupName ("EpcGroup")
    .AddConstructor<UeController>()
    .AddTraceSource ("AttachUeToEnb",
                     "add description here",
                     MakeTraceSourceAccessor (&UeController::m_AttachUeToEnb),
                     "ns3::UeController::AttachUeToEnbCallback")
    
    .AddTraceSource ("ReportUeStatus",
                     "add description here",
                     MakeTraceSourceAccessor (&UeController::m_reportUeStatus),
                     "ns3::UeController::ReportUeStatusCallback")

    .AddTraceSource ("Reconnect",
                     "add description here",
                     MakeTraceSourceAccessor (&UeController::m_Reconnect),
                     "ns3::UeController::Reconnect")
    .AddTraceSource ("FinishSetup",
                     "add description here",
                     MakeTraceSourceAccessor (&UeController::m_finishSetup),
                     "ns3::UeController::FinishSetup")
  ;
  return tid;
}

TypeId
UeController::GetInstanceTypeId () const
{
  return GetTypeId ();
}

UeController::UeController () 
{

}

UeController::UeController (uint16_t num, double s, double y) 
{
  numberOfUes = num;
  speed = s;
  yForUe = y;
}

UeController::~UeController ()
{

}

NetDeviceContainer 
UeController::getUeLteDevs()
{
  return ueLteDevs;
};

void
UeController::attachToClosestEnb()
{
  for (uint16_t i = 0; i < numberOfUes; i++)
    {
      // std::cout<<"attach :"<<i<<std::endl;
      attachToClosestEnb(ueNodes.Get(i));
    }
}

void
UeController::refresh()
{
  // m_Reconnect("abc");
  // calculateDistanceToEnbs(ueNodes.Get(0));
}

void
UeController::refresh(uint64_t imsi, uint16_t cellId, uint16_t rnti, uint16_t rsrq, uint16_t rsrp)
{
  // Ptr<Node> ue = findUeWithImsi(imsi);
  // Ptr<Node> enb = findEnbWithCellId(cellId);
  // Vector currEnbpos = enb->GetObject<MobilityModel> ()->GetPosition ();
  // Vector uepos = ue->GetObject<MobilityModel> ()->GetPosition ();
  // double currDistance = CalculateDistance (uepos, currEnbpos);
  // std::cout<<Simulator::Now ().GetSeconds ()<<","<<currDistance<<","<<rsrq<<","<<rsrp<<std::endl;
 
//   refresh(imsi, cellId, rnti);
// }

// void
// UeController::refresh(uint64_t imsi, uint16_t cellId, uint16_t rnti){;

  // calculateDistanceToEnbs(ueNodes.Get(0));
  //FIX THAT PART !!!!!!!!!!!!!!!!!!! THATS STUPID
  //CHECK: LteUeRrc::SynchronizeToStrongestCell () to fix

for (uint16_t i = 0; i < numberOfUes; i++) {
    Ptr<Node> ue = ueNodes.Get(i);
    int nDevs = ue->GetNDevices ();

    for (int j = 0; j < nDevs; j++) {
      Ptr<LteUeNetDevice> uedev = ue->GetDevice (j)->GetObject <LteUeNetDevice> ();
      // std::cout<<">j "<< j <<" imsi " << uedev->GetImsi() <<std::endl;
      if (uedev){
        if (uedev->GetImsi() == imsi) 
        {
          //Ptr<Node> ue = findUeWithImsi(imsi);
          //The code above replace for the function find UE with IMSI
          //The code below copy from SynchronizeToStrongestCell
          
          LteUeRrc::MaxRsrpCell maxRsrpCell = uedev->GetRrc()->getMaxRsrpCell();

          double rate = (maxRsrpCell.rsrp - rsrp)/rsrp;

          // std::cout<<"maxRsrpCell:cellId"<<maxRsrpCell.cellId<<" rsrp:"<<maxRsrpCell.rsrp<<" rate:"<<rate<<std::endl;


          //The code below is the default code

          Ptr<Node> enb = findEnbWithCellId(cellId);

          Vector currEnbpos = enb->GetObject<MobilityModel> ()->GetPosition ();
          Vector uepos = ue->GetObject<MobilityModel> ()->GetPosition ();

          double currDistance = CalculateDistance (uepos, currEnbpos);

          //If want to prind distance:----
          // struct timeval tp;
          // gettimeofday(&tp, NULL);
          // long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;

          // std::cout<<ms/1000<<"."<<ms%1000<<","<<currDistance<<","<<rsrq<<","<<rsrp<<","<<rate<<","<<Simulator::Now ().GetSeconds ()<<std::endl;   
          
          // 50m: -2.98
          // 200m: -3.43
            if (rate < -3.7 && cellId != maxRsrpCell.cellId) 
            {
              std::cout<<"change"<<std::endl;
              Ptr<Node> closestEnbNode = findEnbWithCellId(maxRsrpCell.cellId);

              m_AttachUeToEnb(ue,cellId,rnti,closestEnbNode,rate);
              
              m_reportUeStatus(ue,enb,rsrq,rsrp,currDistance,true, closestEnbNode, rate);
            } else {
              m_reportUeStatus(ue,enb,rsrq,rsrp,currDistance,false, enb, rate);
            }
          }
      }
    }

  }
}


Ptr<Node> 
UeController::findUeWithImsi(uint64_t imsi)
{
  for (uint16_t i = 0; i < numberOfUes; i++)
  {
    Ptr<Node> ue = ueNodes.Get(i);
    int nDevs = ue->GetNDevices ();

    for (int j = 0; j < nDevs; j++)
    {
      Ptr<LteUeNetDevice> uedev = ue->GetDevice (j)->GetObject <LteUeNetDevice> ();
      // std::cout<<">j "<< j <<" imsi " << uedev->GetImsi() <<std::endl;
      if (uedev){
        if (uedev->GetImsi() == imsi) 
        {
          return ue;
        }
      }
    }

  }
  return CreateObject<Node>();  
};

Ptr<Node> 
UeController::findEnbWithCellId(uint16_t cellId)
{
  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteEnbNetDevice> enbdev = node->GetDevice (j)->GetObject <LteEnbNetDevice> ();
          if (enbdev)
          {
            if (enbdev->GetCellId() == cellId) {
              return node;
            }
          }
        }
    }
  return CreateObject<Node>();
};

void
UeController::calculateDistanceToEnbs(Ptr<Node> ueNode)
{
  // int nDevs = ueNode->GetNDevices ();
  // std::cout<<"> "+nDevs<<std::endl;
  // for (int j = 0; j < nDevs; j++)
  //   {
  //     std::cout<<">j "<< j <<std::endl;

  //     Ptr<LteUeNetDevice> uedev = ueNode->GetDevice (j)->GetObject <LteUeNetDevice> ();
  //     if (uedev) {
  //       std::cout<<"pass"<<std::endl;
 
  //       Ptr<LteEnbNetDevice> targetEnb = uedev->GetTargetEnb();

  //       std::cout<<"pass2"<< targetEnb->GetCellId() <<std::endl;
 
  //       uint16_t cellId = uedev->GetTargetEnb()->GetCellId();
        
  //       std::cout<<" > "<<j<<" "<<cellId<<" - "<<std::endl;

  //       // for (uint16_t i = 0; i < numOfEpc; i++) { 
  //       //   Ptr<EpcGroupHelper> currentEpc = getEpc(i);

  //       //   if (currentEpc->checkExistEnbCellId(cellId)){
  //       //     std::cout<<"choose epc:"<<std::endl;
  //       //     return currentEpc;
  //       //   }
  //       // }
  //     }
  //   }
};


void
UeController::attachToClosestEnb(Ptr<Node> ueNode)
{
  double minDistance = std::numeric_limits<double>::infinity ();
  Vector uepos = ueNode->GetObject<MobilityModel> ()->GetPosition ();
  Ptr<Node> closestEnbNode;

  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteEnbNetDevice> enbdev = node->GetDevice (j)->GetObject <LteEnbNetDevice> ();
          if (enbdev)
            {
              // std::cout<<"---"<<node->GetId()<<" "<<j<<" "<<enbdev->GetCellId()<<std::endl;
              Vector enbpos = node->GetObject<MobilityModel> ()->GetPosition ();
              double distance = CalculateDistance (uepos, enbpos);
              if (distance < minDistance)
                {
                  minDistance = distance;
                  closestEnbNode = node;
                }

            }
        }
    }
    // std::cout<<"find closest:"<<ueNode->GetId()<<" "<<closestEnbNode->GetId()<<std::endl;
    m_AttachUeToEnb(ueNode,0,0,closestEnbNode,minDistance);
}

void 
UeController::setNumOfUes(uint16_t num)
{
  numberOfUes = num;
}

void 
UeController::setSpeed(double s)
{
  speed = s;
};

void 
UeController::setYForUe(double y)
{
  yForUe = y;
};

NodeContainer 
UeController::getUeNodes()
{
  return ueNodes;
};

Ptr<Node>
UeController::getUe(uint16_t index)
{
  return ueNodes.Get(index);
};

void
UeController::setStartingPoint(uint16_t stp)
{
  startingPoint = stp;
};


void
UeController::createNode()
{
// std::cout<<"setup:"<<numberOfUes<<" "<<yForUe<<std::endl;

  ueNodes.Create (numberOfUes);

  MobilityHelper ueMobility;
  ueMobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  ueMobility.Install (ueNodes);


  for (uint16_t i = 0; i < numberOfUes; i++)
    {
      //60 is ok
      ueNodes.Get(i)->GetObject<MobilityModel> ()->SetPosition( Vector(startingPoint+i, yForUe, 7));
      ueNodes.Get (i)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (speed, 0, 0));

      Names::Add ("UE-" + std::to_string(i), ueNodes.Get(i));

    }

}

Vector
UeController::getUeVelocity(Ptr<Node> ueNode)
{
  Vector rs = ueNode->GetObject<ConstantVelocityMobilityModel> ()->GetVelocity ();
  return rs;
}

Vector
UeController::getUePosition(Ptr<Node> ueNode)
{
  Vector rs = ueNode->GetObject<MobilityModel> ()->GetPosition ();
  return rs;
}

void 
UeController::setup()
{
  m_finishSetup();
  
  attachToClosestEnb();
}

void 
UeController::refreshNodeAtPosition(Ptr<Node> ueNode, double x, double y)
{
  ueNode = CreateObject<Node>();
  ueNode->GetObject<MobilityModel> ()->SetPosition( Vector(x, y, 7));
  ueNode->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (speed, 0, 0));
};


void 
UeController::showUes()
{
  std::cout<<"showUes:"<<numberOfUes<<std::endl;

  for (uint16_t i = 0; i < numberOfUes; i++)
  {
    Ptr<Node> ue = ueNodes.Get(i);
    int nDevs = ue->GetNDevices ();
    std::cout<<"--i "<<i<<" "<<ue->GetId()<<std::endl;
    for (int j = 0; j < nDevs; j++)
    {
      Ptr<LteUeNetDevice> uedev = ue->GetDevice (j)->GetObject <LteUeNetDevice> ();
      if (uedev)
      {
        std::cout<<"+++"<<i<<" "<<ue->GetId()<<" j "<< j <<" imsi " << uedev->GetImsi() <<" rnti:"<<uedev->GetRrc()->GetRnti() <<" nas csgid::"<<uedev->GetNas ()->GetCsgId()<<std::endl;
      }
    }
    // std::cout<<"--i "<<i<<" "<<ue->GetId()<<std::endl;
  }
};

} // namespace ns3