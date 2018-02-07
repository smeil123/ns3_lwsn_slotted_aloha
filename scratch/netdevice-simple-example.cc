#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mac48-address.h"
#include <cstdlib>
#include <iostream>

using namespace ns3;

int main (int argc, char *argv[])
{
  NS_LOG_UNCOND("Start");

  LogComponentEnableAll(LOG_PREFIX_TIME);
  LogComponentEnableAll(LOG_PREFIX_FUNC);
  LogComponentEnable("SimpleNetDevice",LOG_LEVEL_ALL);
  NodeContainer nodes;
  nodes.Create (8);

  Ptr<SimpleNetDevice> dev1;
  dev1 = CreateObject<SimpleNetDevice> ();
  dev1->SetAddress(Mac48Address("00:00:00:00:00:01"));
  nodes.Get(0)->AddDevice (dev1);

  Ptr<SimpleNetDevice> dev2;
  dev2 = CreateObject<SimpleNetDevice> ();
  dev2->SetAddress(Mac48Address("00:00:00:00:00:02"));
  nodes.Get(1)->AddDevice (dev2);

  Ptr<SimpleNetDevice> dev3;
  dev3 = CreateObject<SimpleNetDevice> ();
  dev3->SetAddress(Mac48Address("00:00:00:00:00:03"));
  nodes.Get(2)->AddDevice (dev3);

  Ptr<SimpleNetDevice> dev4;
  dev4 = CreateObject<SimpleNetDevice> ();
  dev4->SetAddress(Mac48Address("00:00:00:00:00:04"));
  nodes.Get(3)->AddDevice (dev4);

  Ptr<SimpleNetDevice> dev5;
  dev5 = CreateObject<SimpleNetDevice> ();
  dev5->SetAddress(Mac48Address("00:00:00:00:00:05"));
  nodes.Get(4)->AddDevice (dev5);

  Ptr<SimpleNetDevice> dev6;
  dev6 = CreateObject<SimpleNetDevice> ();
  dev6->SetAddress(Mac48Address("00:00:00:00:00:06"));
  nodes.Get(5)->AddDevice (dev6);

  Ptr<SimpleNetDevice> gate1;
  gate1 = CreateObject<SimpleNetDevice> ();
  gate1->SetAddress(Mac48Address("00:00:00:00:00:07"));
  nodes.Get(6)->AddDevice (gate1);

  Ptr<SimpleNetDevice> gate2;
  gate2 = CreateObject<SimpleNetDevice> ();
  gate2->SetAddress(Mac48Address("00:00:00:00:00:08"));
  nodes.Get(7)->AddDevice (gate2);


  Ptr<SimpleChannel> channel = CreateObject<SimpleChannel> ();
  dev1->SetChannel (channel);
  dev2->SetChannel (channel);
  dev3->SetChannel (channel);
  dev4->SetChannel (channel);
  dev5->SetChannel (channel);
  dev6->SetChannel (channel);
  gate1->SetChannel (channel);
  gate2->SetChannel (channel);

  dev1->SetNode (nodes.Get (0));
  dev2->SetNode (nodes.Get (1));
  dev3->SetNode (nodes.Get (2));
  dev4->SetNode (nodes.Get (3));
  dev5->SetNode (nodes.Get (4));
  dev6->SetNode (nodes.Get (5));
  gate1->SetNode (nodes.Get (6));
  gate2->SetNode (nodes.Get (7));

  dev1->SetSid(1);
  dev2->SetSid(2);
  dev3->SetSid(3);  
  dev4->SetSid(4);
  dev5->SetSid(5);
  dev6->SetSid(6);
  gate1->SetSid(0);
  gate2->SetSid(0);

  gate1->SetGid(1);
  gate2->SetGid(2);

  dev1->SetSideAddress(gate1->GetAddress(),dev2->GetAddress());
  dev2->SetSideAddress(dev1->GetAddress(),dev3->GetAddress());
  dev3->SetSideAddress(dev2->GetAddress(),dev4->GetAddress());
  dev4->SetSideAddress(dev3->GetAddress(),dev5->GetAddress());
  dev5->SetSideAddress(dev4->GetAddress(),dev6->GetAddress());
  dev6->SetSideAddress(dev5->GetAddress(),gate2->GetAddress());

  Ptr<Packet> p = Create<Packet> (100);
  Ptr<Packet> p1 = Create<Packet> (100);
  Ptr<Packet> p2 = Create<Packet> (100);
  Ptr<Packet> p3 = Create<Packet> (100);
  Ptr<Packet> p4 = Create<Packet> (100);

  dev1 ->SetN_node(6); 
  dev2 ->SetN_node(6); 
  dev3 ->SetN_node(6); 
  dev4 ->SetN_node(6); 
  dev5 ->SetN_node(6); 
  dev6 ->SetN_node(6); 


  // 1

  // Simulator::Schedule(Seconds(0.0),&SimpleNetDevice::OriginalTransmission,dev1,p,false);
  // Simulator::Schedule(Seconds(1.0),&SimpleNetDevice::OriginalTransmission,dev5,p1,false);

  // 2

  // Simulator::Schedule(Seconds(0.0),&SimpleNetDevice::OriginalTransmission,dev2,p,false);
  // Simulator::Schedule(Seconds(0.0),&SimpleNetDevice::OriginalTransmission,dev3,p1,false);

  // // 3

  Simulator::Schedule(Seconds(0.0),&SimpleNetDevice::OriginalTransmission,dev1,p,false);
  Simulator::Schedule(Seconds(1.0),&SimpleNetDevice::OriginalTransmission,dev4,p1,false);

  // // 4

  // Simulator::Schedule(Seconds(0.0),&SimpleNetDevice::OriginalTransmission,dev1,p,false);
  // Simulator::Schedule(Seconds(2.0),&SimpleNetDevice::OriginalTransmission,dev3,p1,false);

  // // 5

  // Simulator::Schedule(Seconds(0.0),&SimpleNetDevice::OriginalTransmission,dev4,p,false);
  // Simulator::Schedule(Seconds(1.0),&SimpleNetDevice::OriginalTransmission,dev2,p1,false);

  // // 6

  // Simulator::Schedule(Seconds(0.0),&SimpleNetDevice::OriginalTransmission,dev4,p1,false);
  // Simulator::Schedule(Seconds(0.0),&SimpleNetDevice::OriginalTransmission,dev2,p,false);
  // Simulator::Schedule(Seconds(0.0),&SimpleNetDevice::OriginalTransmission,dev3,p2,false);

  // 7

  // Simulator::Schedule(Seconds(0.0),&SimpleNetDevice::OriginalTransmission,dev4,p1,false);
  // Simulator::Schedule(Seconds(0.0),&SimpleNetDevice::OriginalTransmission,dev2,p,false);
  // Simulator::Schedule(Seconds(0.0),&SimpleNetDevice::OriginalTransmission,dev3,p2,false);
  // Simulator::Schedule(Seconds(0.0),&SimpleNetDevice::OriginalTransmission,dev5,p3,false);

  // 8
  // Simulator::Schedule(Seconds(0.0),&SimpleNetDevice::OriginalTransmission,dev4,p1,false);
  // Simulator::Schedule(Seconds(0.0),&SimpleNetDevice::OriginalTransmission,dev2,p,false);
  //9
 // Simulator::Schedule(Seconds(0.0),&SimpleNetDevice::OriginalTransmission,dev2,p1,false);
 //  Simulator::Schedule(Seconds(1.0),&SimpleNetDevice::OriginalTransmission,dev2,p,false);
 
 //  //10
 // Simulator::Schedule(Seconds(0.0),&SimpleNetDevice::OriginalTransmission,dev2,p1,false);
 //  Simulator::Schedule(Seconds(2.0),&SimpleNetDevice::OriginalTransmission,dev2,p,false);
 
  Simulator::Schedule(Seconds(100.0),&SimpleNetDevice::Print,dev1);
  Simulator::Schedule(Seconds(100.0),&SimpleNetDevice::Print,dev2);
  Simulator::Schedule(Seconds(100.0),&SimpleNetDevice::Print,dev3);
  Simulator::Schedule(Seconds(100.0),&SimpleNetDevice::Print,dev4);
  Simulator::Schedule(Seconds(100.0),&SimpleNetDevice::Print,dev5);
  Simulator::Schedule(Seconds(100.0),&SimpleNetDevice::Print,dev6);

  Simulator::Run();
  Simulator::Destroy ();
  std::cout << " " <<(dev1->n_count+dev2->n_count+dev3->n_count+dev4->n_count+dev5->n_count+dev6->n_count)/2<<std::endl;

  return 0;
}
