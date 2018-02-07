/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "simple-net-device.h"
#include "simple-channel.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/error-model.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/boolean.h"
#include "ns3/string.h"
#include "ns3/tag.h"
#include "ns3/simulator.h"
#include "ns3/drop-tail-queue.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SimpleNetDevice");

/**
 * \brief SimpleNetDevice tag to store source, destination and protocol of each packet.
 */
class SimpleTag : public Tag {
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);

  /**
   * Set the source address
   * \param src source address
   */
  void SetSrc (Mac48Address src);
  /**
   * Get the source address
   * \return the source address
   */
  Mac48Address GetSrc (void) const;

  /**
   * Set the destination address
   * \param dst destination address
   */
  void SetDst (Mac48Address dst);
  /**
   * Get the destination address
   * \return the destination address
   */
  Mac48Address GetDst (void) const;

  /**
   * Set the protocol number
   * \param proto protocol number
   */
  void SetProto (uint16_t proto);
  /**
   * Get the protocol number
   * \return the protocol number
   */
  uint16_t GetProto (void) const;

  void Print (std::ostream &os) const;

private:
  Mac48Address m_src; //!< source address
  Mac48Address m_dst; //!< destination address
  uint16_t m_protocolNumber; //!< protocol number
};


NS_OBJECT_ENSURE_REGISTERED (SimpleTag);

TypeId
SimpleTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SimpleTag")
    .SetParent<Tag> ()
    .SetGroupName("Network")
    .AddConstructor<SimpleTag> ()
  ;
  return tid;
}
TypeId
SimpleTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
SimpleTag::GetSerializedSize (void) const
{
  return 8+8+2;
}
void
SimpleTag::Serialize (TagBuffer i) const
{
  uint8_t mac[6];
  m_src.CopyTo (mac);
  i.Write (mac, 6);
  m_dst.CopyTo (mac);
  i.Write (mac, 6);
  i.WriteU16 (m_protocolNumber);
}
void
SimpleTag::Deserialize (TagBuffer i)
{
  uint8_t mac[6];
  i.Read (mac, 6);
  m_src.CopyFrom (mac);
  i.Read (mac, 6);
  m_dst.CopyFrom (mac);
  m_protocolNumber = i.ReadU16 ();
}

void
SimpleTag::SetSrc (Mac48Address src)
{
  m_src = src;
}

Mac48Address
SimpleTag::GetSrc (void) const
{
  return m_src;
}

void
SimpleTag::SetDst (Mac48Address dst)
{
  m_dst = dst;
}

Mac48Address
SimpleTag::GetDst (void) const
{
  return m_dst;
}

void
SimpleTag::SetProto (uint16_t proto)
{
  m_protocolNumber = proto;
}

uint16_t
SimpleTag::GetProto (void) const
{
  return m_protocolNumber;
}

void
SimpleTag::Print (std::ostream &os) const
{
  os << "src=" << m_src << " dst=" << m_dst << " proto=" << m_protocolNumber;
}



NS_OBJECT_ENSURE_REGISTERED (SimpleNetDevice);

TypeId 
SimpleNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SimpleNetDevice")
    .SetParent<NetDevice> ()
    .SetGroupName("Network") 
    .AddConstructor<SimpleNetDevice> ()
    .AddAttribute ("ReceiveErrorModel",
                   "The receiver error model used to simulate packet loss",
                   PointerValue (),
                   MakePointerAccessor (&SimpleNetDevice::m_receiveErrorModel),
                   MakePointerChecker<ErrorModel> ())
    .AddAttribute ("PointToPointMode",
                   "The device is configured in Point to Point mode",
                   BooleanValue (false),
                   MakeBooleanAccessor (&SimpleNetDevice::m_pointToPointMode),
                   MakeBooleanChecker ())
    .AddAttribute ("TxQueue",
                   "A queue to use as the transmit queue in the device.",
                   StringValue ("ns3::DropTailQueue"),
                   MakePointerAccessor (&SimpleNetDevice::m_queue),
                   MakePointerChecker<Queue> ())
    .AddAttribute ("DataRate",
                   "The default data rate for point to point links. Zero means infinite",
                   DataRateValue (DataRate ("0b/s")),
                   MakeDataRateAccessor (&SimpleNetDevice::m_bps),
                   MakeDataRateChecker ())
    .AddTraceSource ("PhyRxDrop",
                     "Trace source indicating a packet has been dropped "
                     "by the device during reception",
                     MakeTraceSourceAccessor (&SimpleNetDevice::m_phyRxDropTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}

SimpleNetDevice::SimpleNetDevice ()
  : m_channel (0),
    m_node (0),
    m_mtu (0xffff),
    m_ifIndex (0),
    m_linkUp (false)
{
  NS_LOG_FUNCTION (this);
  ndid = 0;
  send_flag = false;
  sch_flag = false;
  receive_flag = false;
  receive_flag_1 = false;
  n_count = 0;
  n_ncCount = 0;  
}

void
SimpleNetDevice::SetN_node(uint16_t x){
  n_node = x;
  
  for(int i =0;i<n_node;i++){
    m_rxArray[i] = 0;
    m_rxArrayPacket[i] = 0;
  }

  if(x%3 == 0){
    timeslot = (x/3)*9;
  }
  else{
    timeslot = (x/3)*9+1;
  }
  int delay=Simulator::Now().GetSeconds();
  int slot=0;
  if(delay==0){
    slot=0;
  }
  else{
    slot=delay%timeslot;
  }
  end_flag = true;    
  Simulator::Schedule(Seconds(timeslot-slot-0.1),&SimpleNetDevice::EndSchedule,this);
}

void
SimpleNetDevice::SetSid(uint16_t sid){
	m_sid=sid;
}

uint16_t
SimpleNetDevice::GetSid(){
	return m_sid;
}
void
SimpleNetDevice::SetGid(uint16_t gid){
  m_gid=gid;
}

uint16_t
SimpleNetDevice::GetGid(){
  return m_gid;
}
void
SimpleNetDevice::SetSideAddress(Address laddress, Address raddress){
  l_address=Mac48Address::ConvertFrom (laddress);
  r_address=Mac48Address::ConvertFrom (raddress);
}                  

/*void
SimpleNetDevice::ReceiveStart(Ptr<Packet> packet, uint16_t protocol,Mac48Address to, Mac48Address from)
{
  if(!end_flag){
    int delay=Simulator::Now().GetSeconds();
    int slot=0;
    if(delay==0){
      slot=0;
    }
    else{
      slot=delay%timeslot;
    }     
    Simulator::Schedule(Seconds(timeslot-slot),&SimpleNetDevice::EndSchedule,this);
    end_flag = true;
  }

  Simulator::Schedule(Second(1.0),&SimpleNetDevice::Receive,this,packet,protocol,to,from);
}*/

void
SimpleNetDevice::Receive (Ptr<Packet> packet, uint16_t protocol,
                          Mac48Address to, Mac48Address from)
{

  if (m_receiveErrorModel && m_receiveErrorModel->IsCorrupt (packet) )
    {
      m_phyRxDropTrace (packet);
      return;
    }

  if (to == m_address)   
    { 


      /*
        m_rxArray[] == 0 : no receive packet
        m_rxArray[] == 1 : receive packet
        m_rxArray[] == 2 : receive & send packet
      */
      LwsnHeader receiveHeader;

      if(m_sid==0){
        packet->RemoveHeader(receiveHeader);
        receiveHeader.SetStartTime2(Simulator::Now().GetSeconds()-receiveHeader.GetStartTime()+1);

        NS_LOG_UNCOND("");
        NS_LOG_UNCOND("---------------------------");
        NS_LOG_UNCOND("Gid"<<m_gid<<"  Osid : "<<receiveHeader.GetOsid());
        NS_LOG_UNCOND(Simulator::Now().GetSeconds()+1<< "s : "<<"Start time : " << receiveHeader.GetStartTime() <<"  Total Time : " << Simulator::Now().GetSeconds()-receiveHeader.GetStartTime()+1); 
        NS_LOG_UNCOND("---------------------------");
        NS_LOG_UNCOND("");

        packet->AddHeader(receiveHeader);
        m_queue->Enqueue(Create<QueueItem> (packet));
        return;   
      }

      packet->PeekHeader(receiveHeader);

      if(!end_flag){
        int temp = 0;
        /*int temp = Simulator::Now().GetSeconds();
        slot=temp%timeslot;*/
        temp = Simulator::Now().GetSeconds()/timeslot;
        NS_LOG_FUNCTION("Sid " <<m_sid << "delay "<<timeslot-(Simulator::Now().GetSeconds()-temp*timeslot));
        Simulator::Schedule(Seconds(timeslot-(Simulator::Now().GetSeconds()-temp*timeslot)-0.1),&SimpleNetDevice::EndSchedule,this);
          
        end_flag = true;
      }

      if(receiveHeader.GetType() == LwsnHeader::NETWORK_CODING){
        Ptr<Packet> p = decoding(packet);
        p->PeekHeader(receiveHeader);

        if(m_rxArray[receiveHeader.GetOsid()-1] == 0 ){
          m_rxArray[receiveHeader.GetOsid()-1] = 1;
          m_rxArrayPacket[receiveHeader.GetOsid()-1] = p->Copy();

          NS_LOG_FUNCTION ("-----------Networkcoding-------------"); 
          NS_LOG_FUNCTION ("SID  : " <<this->GetSid() << "from->"  <<from << "   Osid->"<<receiveHeader.GetOsid());
          NS_LOG_FUNCTION("");
          SendSchedule(p,from);
        }
      }
      else{

        if(m_rxArray[receiveHeader.GetOsid()-1] == 0 ){
          m_rxArray[receiveHeader.GetOsid()-1] = 1;
          m_rxArrayPacket[receiveHeader.GetOsid()-1] = packet->Copy();

          if(receiveHeader.GetType() == LwsnHeader::ORIGINAL_TRANSMISSION){
            NS_LOG_FUNCTION ("-----------OriginalTransmission-------------"); 
            NS_LOG_FUNCTION ("SID  : " <<this->GetSid() << "from->"  <<from << "   Osid->"<<receiveHeader.GetOsid());
        
            SendSchedule(packet,from);
          }
          else if(receiveHeader.GetType() == LwsnHeader::FORWARDING){
            NS_LOG_FUNCTION ("-----------Forwarding-------------"); 
            NS_LOG_FUNCTION ("SID  : " <<this->GetSid() << "from->"  <<from << "   Osid->"<<receiveHeader.GetOsid());
            SendSchedule(packet,from);
          }
          else{
            NS_LOG_FUNCTION("SID : "<<receiveHeader.GetType());
          
          }

        }

        
      }
    }
  /*else if (to.IsBroadcast ())
    {
      packetType = NetDevice::PACKET_BROADCAST;
    }
  else if (to.IsGroup ())
    {
      packetType = NetDevice::PACKET_MULTICAST;
    }
  else 
    {
      packetType = NetDevice::PACKET_OTHERHOST;
    }

  if (packetType != NetDevice::PACKET_OTHERHOST)
    {
      m_rxCallback (this, packet, protocol, from);
    }

  if (!m_promiscCallback.IsNull ())
    {
      m_promiscCallback (this, packet, protocol, from, to, packetType);
    }*/
}

void 
SimpleNetDevice::SetChannel (Ptr<SimpleChannel> channel)
{
  NS_LOG_FUNCTION (this << channel);
  m_channel = channel;
  m_channel->Add (this);
  m_linkUp = true;
  m_linkChangeCallbacks ();
}

Ptr<Queue>
SimpleNetDevice::GetQueue () const
{
  NS_LOG_FUNCTION (this);
  return m_queue;
}

void
SimpleNetDevice::SetQueue (Ptr<Queue> q)
{
  NS_LOG_FUNCTION (this << q);
  m_queue = q;
}

void
SimpleNetDevice::SetReceiveErrorModel (Ptr<ErrorModel> em)
{
  NS_LOG_FUNCTION (this << em);
  m_receiveErrorModel = em;
}

void 
SimpleNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
  m_ifIndex = index;
}
uint32_t 
SimpleNetDevice::GetIfIndex (void) const
{
  NS_LOG_FUNCTION (this);
  return m_ifIndex;
}
Ptr<Channel> 
SimpleNetDevice::GetChannel (void) const
{
  NS_LOG_FUNCTION (this);
  return m_channel;
}
void
SimpleNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION (this << address);
  m_address = Mac48Address::ConvertFrom (address);
}
Address 
SimpleNetDevice::GetAddress (void) const
{
  //
  // Implicit conversion from Mac48Address to Address
  //
  NS_LOG_FUNCTION (this);
  return m_address;
}
bool 
SimpleNetDevice::SetMtu (const uint16_t mtu)
{
  NS_LOG_FUNCTION (this << mtu);
  m_mtu = mtu;
  return true;
}
uint16_t 
SimpleNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mtu;
}
bool 
SimpleNetDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION (this);
  return m_linkUp;
}
void 
SimpleNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
 NS_LOG_FUNCTION (this << &callback);
 m_linkChangeCallbacks.ConnectWithoutContext (callback);
}
bool 
SimpleNetDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  if (m_pointToPointMode)
    {
      return false;
    }
  return true;
}
Address
SimpleNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return Mac48Address ("ff:ff:ff:ff:ff:ff");
}
bool 
SimpleNetDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION (this);
  if (m_pointToPointMode)
    {
      return false;
    }
  return true;
}
Address 
SimpleNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION (this << multicastGroup);
  return Mac48Address::GetMulticast (multicastGroup);
}

Address SimpleNetDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (this << addr);
  return Mac48Address::GetMulticast (addr);
}

bool 
SimpleNetDevice::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION (this);
  if (m_pointToPointMode)
    {
      return true;
    }
  return false;
}

bool 
SimpleNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

Ptr<Packet>
SimpleNetDevice::encoding(Ptr<Packet> p1,Ptr<Packet> p2)
{
  LwsnHeader temp1;
  p1 -> RemoveHeader(temp1);

  LwsnHeader temp2;
  p2 -> RemoveHeader(temp2);

  Ptr<Packet> ncpacket = Create<Packet> (100);
  LwsnHeader ncHeader;

  ncHeader.SetType(LwsnHeader::NETWORK_CODING);
  ncHeader.SetOsid(temp1.GetOsid());
  ncHeader.SetDid(temp1.GetDid());
  ncHeader.SetPsid(m_sid);
  ncHeader.SetStartTime(temp1.GetStartTime());
  ncHeader.SetE(1);

  ncHeader.SetOsid2(temp2.GetOsid());
  ncHeader.SetDid2(temp2.GetDid());
  ncHeader.SetStartTime2(temp2.GetStartTime());

  ncpacket->AddHeader(ncHeader);

  NS_LOG_FUNCTION("Sid : "<<this->GetSid()<< "  packet encoding, get Osid_1 -> "<<ncHeader.GetOsid() << "Osid_2 ->"<<ncHeader.GetOsid2());

  return ncpacket;
}

Ptr<Packet>
SimpleNetDevice::decoding(Ptr<Packet> p)
{
  LwsnHeader tempheader;
  p -> RemoveHeader(tempheader);

  /*
    if(m_rxArray[]==0) -> no receive packet
  */
NS_LOG_FUNCTION("Sid : "<<m_sid<<"Osid_1" << m_rxArray[tempheader.GetOsid()-1] <<"   Osid_2" <<m_rxArray[tempheader.GetOsid2()-1]);
  if(m_rxArray[tempheader.GetOsid()-1]>0){
    Ptr<Packet> packet = Create<Packet> (100);
    LwsnHeader sendHeader;

    sendHeader.SetType(LwsnHeader::FORWARDING);
    sendHeader.SetOsid(tempheader.GetOsid2());
    sendHeader.SetDid(tempheader.GetDid2());
    sendHeader.SetPsid(tempheader.GetPsid());
    sendHeader.SetStartTime(tempheader.GetStartTime2());
    sendHeader.SetE(1);

  NS_LOG_FUNCTION("GetStartTime"<<tempheader.GetStartTime());

    packet->AddHeader(sendHeader);

    NS_LOG_FUNCTION("Sid : "<<this->GetSid()<< "  packet decoding, get Osid -> "<<sendHeader.GetOsid());

    return packet;
  }
  else if(m_rxArray[tempheader.GetOsid2()-1]>0){
    Ptr<Packet> packet = Create<Packet> (100);
    LwsnHeader sendHeader;

    sendHeader.SetType(LwsnHeader::FORWARDING);
    sendHeader.SetOsid(tempheader.GetOsid());
    sendHeader.SetDid(tempheader.GetDid());
    sendHeader.SetPsid(tempheader.GetPsid());
    sendHeader.SetStartTime(tempheader.GetStartTime());
    sendHeader.SetE(1);
  NS_LOG_FUNCTION("GetStartTime"<<tempheader.GetStartTime());

    packet->AddHeader(sendHeader);
    NS_LOG_FUNCTION("Sid : "<<this->GetSid()<< "  packet decoding, get Osid -> "<<sendHeader.GetOsid());

    return packet;
  }
  return p;
}

void
SimpleNetDevice::NetworkCoding(int x, int y)
{
  n_ncCount++;
  Ptr<Packet> ncpacket;

  Ptr<Packet> p1 = m_rxArrayPacket[x]->Copy();
  Ptr<Packet> p2 = m_rxArrayPacket[y]->Copy();

  ncpacket = encoding(p1,p2);

  NS_LOG_FUNCTION("");
  NS_LOG_FUNCTION ("SID  : " <<m_sid);
  NS_LOG_FUNCTION("");

  ChannelSend(ncpacket,0,l_address,m_address);
  ChannelSend(ncpacket,0,r_address,m_address);

  m_rxArray[x] = 2;
  m_rxArray[y] = 2;
}

void
SimpleNetDevice::SendCheck(Ptr<Packet> p,int n, bool flag,int x, int y){

  int number = 0;
  double a = 0;

  /*
    if receive_flag == receive_flag_1 == flag == true -> network coding
    else -> forwarding
  */
  if(receive_flag){
    if(receive_flag_1){
      if(flag){
        number = n;
        receive_flag_1 = receive_flag = false;
      }
    }
    else{
      if(!flag){
        number = n;
        receive_flag_1 = receive_flag = false;
        a = 0.1;
      }
    }
  }
  else{
    if(flag){
      number = n;
      receive_flag_1 = receive_flag = false;
    }
  }

  if(x != 100){
    m_rxArray[x] = 2;
  }
  if(y != 100){
    m_rxArray[y] = 2;
  }

  if(number==2){
    if(m_sid %3 == 2){
      Simulator::Schedule(Seconds(2.0-a),&SimpleNetDevice::NetworkCoding,this,x,y);
    }
    else if(m_sid%3 == 1){
      Simulator::Schedule(Seconds(1.0-a),&SimpleNetDevice::NetworkCoding,this,x,y);
    }
    else{
      Simulator::Schedule(Seconds(4.0-a),&SimpleNetDevice::NetworkCoding,this,x,y);
    }
  }
  else if(number==1){

    LwsnHeader tmpHeader;
    p->PeekHeader(tmpHeader);

    if(tmpHeader.GetOsid() > m_sid){
      if(m_sid %3 == 2){
        Simulator::Schedule(Seconds(2.0-a),&SimpleNetDevice::Forwarding,this,p,l_address);
      }
      else if(m_sid%3 == 1){
        Simulator::Schedule(Seconds(1.0-a),&SimpleNetDevice::Forwarding,this,p,l_address);
      }
      else{
        Simulator::Schedule(Seconds(4.0-a),&SimpleNetDevice::Forwarding,this,p,l_address);
      }
    }
    else{
      if(m_sid%3 == 2){
        Simulator::Schedule(Seconds(2.0-a),&SimpleNetDevice::Forwarding,this,p,r_address);
      }
      else if(m_sid%3 == 1){
        Simulator::Schedule(Seconds(1.0-a),&SimpleNetDevice::Forwarding,this,p,r_address);
      }
      else{
        Simulator::Schedule(Seconds(4.0-a),&SimpleNetDevice::Forwarding,this,p,r_address);
      }
    }
  }
}

void
SimpleNetDevice::Forwarding(Ptr<Packet> p,Mac48Address to){

  Ptr<Packet> packet = p->Copy();

  LwsnHeader tempHeader;
  packet->RemoveHeader(tempHeader);

  m_rxArray[tempHeader.GetOsid()-1] = 2;

  Ptr<Packet> ackpacket = packet->Copy();

  NS_LOG_FUNCTION("");
  NS_LOG_FUNCTION ("SID  : " <<m_sid << "to : "<<to);
  NS_LOG_FUNCTION("GetStartTime"<<tempHeader.GetStartTime());
  NS_LOG_FUNCTION("");

  LwsnHeader sendHeader;
  sendHeader.SetType(LwsnHeader::FORWARDING);
  sendHeader.SetPsid(m_sid);
  sendHeader.SetOsid(tempHeader.GetOsid());
  sendHeader.SetDid(tempHeader.GetDid());
  sendHeader.SetStartTime(tempHeader.GetStartTime());
  sendHeader.SetE(0);

  LwsnHeader ackHeader;
  ackHeader.SetType(LwsnHeader::IACK);
  ackHeader.SetPsid(m_sid);
  ackHeader.SetOsid(tempHeader.GetOsid());
  ackHeader.SetDid(tempHeader.GetDid());
  ackHeader.SetStartTime(tempHeader.GetStartTime());
  ackHeader.SetE(0);

  packet->AddHeader(sendHeader);
  ackpacket->AddHeader(ackHeader);



  ChannelSend(packet,0,to,m_address);
  if(to == l_address){
    ChannelSend(ackpacket,0,r_address,m_address);
  }
  else{
    ChannelSend(ackpacket,0,l_address,m_address);
  }
}

void 
SimpleNetDevice::SendSchedule(Ptr<Packet> p,Mac48Address from)
{
  int temp = 0,x = 100,y =100 ;
  for(int i =0; i<n_node; i++){
    if(m_rxArray[i]==1) {
      temp++;
      if(temp == 1){
        x = i;
      }
      else if(temp == 2){
        y = i;
      }
    }
  }

  if(m_sid%3 == 2){
    if(from == l_address){
      receive_flag = true;
      Simulator::Schedule(Seconds(2.1),&SimpleNetDevice::SendCheck,this,p,temp,false,x,y);
    }else{
      Simulator::ScheduleNow(&SimpleNetDevice::SendCheck,this,p,temp,true,x,y);
      receive_flag_1 = true;
    }
  }
  else if(m_sid%3 == 1){
    if(from == l_address){
      Simulator::ScheduleNow(&SimpleNetDevice::SendCheck,this,p,temp,true,x,y);
      receive_flag_1 = true;
    }else{
      receive_flag = true;
      Simulator::Schedule(Seconds(1.1),&SimpleNetDevice::SendCheck,this,p,temp,false,x,y);
    }
  }
  else{
    if(from == l_address){
      Simulator::ScheduleNow(&SimpleNetDevice::SendCheck,this,p,temp,true,x,y);
      receive_flag_1 = true;
    }else{
      receive_flag = true;
      Simulator::Schedule(Seconds(1.1),&SimpleNetDevice::SendCheck,this,p,temp,false,x,y);
    }
  }

}

void
SimpleNetDevice::EndSchedule(){
  NS_LOG_FUNCTION("Sid : "<<m_sid);
  if(sch_flag == true){
    sch_flag = false;
  }
  else{
    send_flag = false;
  }
  for(int i =0;i<n_node; i++){
    m_rxArray[i]=0;
    m_rxArrayPacket[i]=0;
  }

  receive_flag = receive_flag_1 = false;
  end_flag = false;

}

void
SimpleNetDevice::SetSleep(){
  if(m_queue->GetNPackets()>0)
    Ptr<Packet> packet = m_queue->Dequeue()->GetPacket ();
}
void 
SimpleNetDevice::ChannelSend(Ptr<Packet> p, uint16_t protocol, Mac48Address to, Mac48Address from){\
  n_count++;
  NS_LOG_FUNCTION ("Sid"<<this->GetSid() <<"n_count" <<n_count );
  m_channel->Send(p, protocol, to, from, this);
  Simulator::Schedule(Seconds(1.0),&SimpleNetDevice::SetSleep,this);

/*  if(!end_flag){
    int slot=0;
    if(Simulator::Now().GetSeconds()==0){
      slot=0;
    }
    else{
      int temp = Simulator::Now().GetSeconds();
      slot=temp%timeslot;
    }     
    Simulator::Schedule(Seconds(timeslot-slot),&SimpleNetDevice::EndSchedule,this);
    end_flag = true;
  }*/
  if(!end_flag){
    int temp = 0;
    temp = Simulator::Now().GetSeconds()/timeslot;
    NS_LOG_FUNCTION("now time"<<Simulator::Now().GetSeconds()<<"timeslot " << timeslot);
    NS_LOG_FUNCTION("temp"<<temp);
    Simulator::Schedule(Seconds(timeslot-(Simulator::Now().GetSeconds()-temp*timeslot)-0.1),&SimpleNetDevice::EndSchedule,this);
    NS_LOG_FUNCTION("Sid " <<m_sid << "delay "<<timeslot-(Simulator::Now().GetSeconds()-temp*timeslot));
            
    end_flag = true;
    }
}

void
SimpleNetDevice::Print(){

  NS_LOG_UNCOND("====================");

  if(m_sid == 0){
    int j = m_queue->GetNPackets();
    for(int i=0;i<j;i++){
      Ptr<Packet> packet = m_queue->Dequeue()->GetPacket();
      LwsnHeader temp;
      packet->RemoveHeader(temp);

      NS_LOG_UNCOND(i+1 << " :: packet Osid : "<<temp.GetOsid()<<" Did : "<<temp.GetDid()<<" Start Time: "<<temp.GetStartTime()<<"  Total Time : " <<temp.GetStartTime2());

    }
  }
  NS_LOG_UNCOND("Sid -> " << m_sid << "send : " << n_count/2-n_ncCount <<"  networkcoding : " << n_ncCount);
  NS_LOG_UNCOND("====================");
}

bool 
SimpleNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  //NS_LOG_FUNCTION (this << packet << dest << protocolNumber);
  //std::cout<<"send"<<std::endl;
  return SendFrom (packet, m_address, dest, protocolNumber);
}
bool
SimpleNetDevice::SendFrom(Ptr<Packet> p, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  if(p->GetSize()>GetMtu())
  {
    return false;
  }
  Ptr<Packet> packet = p->Copy();

  Mac48Address to = Mac48Address::ConvertFrom(dest);
  Mac48Address from = Mac48Address::ConvertFrom(source);

  SimpleTag tag;
  tag.SetSrc(from);
  tag.SetDst(to);
  tag.SetProto(protocolNumber);

  p->AddPacketTag(tag);

  if(m_queue->Enqueue(Create<QueueItem> (p)))
  {
    if(m_queue->GetNPackets()==1 && !TransmitCompleteEvent.IsRunning())
    {
      p = m_queue->Dequeue()->GetPacket ();
      p->RemovePacketTag(tag);
      Time txTime = Time(0);
      if(m_bps > DataRate(0))
      {
        txTime = m_bps.CalculateBytesTxTime(packet->GetSize());
      }
      m_channel -> Send(p,protocolNumber,to,from,this);
      TransmitCompleteEvent = Simulator::Schedule(txTime,&SimpleNetDevice::TransmitComplete,this);
    }
    return true;
  }
  m_channel->Send(packet,protocolNumber,to,from,this);
  return true;
}
bool
SimpleNetDevice::OriginalTransmission (Ptr<Packet> p,bool header)
{ 
  //std::cout<<this->GetSid()<<"sendFrom"<<std::endl;	
  NS_LOG_FUNCTION ("Sid" << m_sid );  
  if (p->GetSize () > GetMtu ())
    {
      return false;
    }
  Ptr<Packet> packet = p->Copy ();

  Mac48Address rto = Mac48Address::ConvertFrom (r_address);
  Mac48Address lto = Mac48Address::ConvertFrom (l_address);
  Mac48Address from = Mac48Address::ConvertFrom (m_address);

  int protocolNumber = 0;
  int time = Simulator::Now().GetSeconds();
  LwsnHeader sendheader;
  if(!header){
    sendheader.SetOsid(m_sid);
    sendheader.SetPsid(m_sid);
    sendheader.SetType(LwsnHeader::ORIGINAL_TRANSMISSION);
    sendheader.SetDid(++ndid);
    sendheader.SetE(0);
    sendheader.SetStartTime(Simulator::Now().GetSeconds());
  }
  else
  {
    LwsnHeader header;
    packet->RemoveHeader(header);
    NS_LOG_FUNCTION("GetStartTime"<<header.GetStartTime());

    sendheader.SetOsid(m_sid);
    sendheader.SetPsid(m_sid);
    sendheader.SetE(0);
    sendheader.SetType(LwsnHeader::ORIGINAL_TRANSMISSION);
    sendheader.SetStartTime(header.GetStartTime());
    sendheader.SetDid(header.GetDid());
  }
  packet->AddHeader(sendheader);

  if(send_flag == true){

    int delay = time%timeslot;
    NS_LOG_UNCOND("Sid : "<<m_sid<<"delay" << timeslot-delay);
    Simulator::Schedule(Seconds(timeslot-delay), &SimpleNetDevice::OriginalTransmission, this,packet,true);
      
    return 0;
  }

  if (m_queue->Enqueue (Create<QueueItem> (packet)))
    {
      if (m_queue->GetNPackets () == 1 && !TransmitCompleteEvent.IsRunning ())
        {
          send_flag = true;

          p = m_queue->Dequeue ()->GetPacket ();

          int delay=Simulator::Now().GetSeconds();
          int slot=0;

          Time txTime = Time (0);
          if (m_bps > DataRate (0))
            {
              txTime = m_bps.CalculateBytesTxTime (packet->GetSize ());
            }
            /*schedule*/
        
    	  if(delay==0){
    	  	slot=0;
    	  }
    	  else{
    	  	slot=delay%timeslot;
    	  }
        
    	  if(m_sid%3 == 1){
    	  		if(slot==0){
              ChannelSend(p,protocolNumber,rto,from);
              ChannelSend(p,protocolNumber,lto,from);
    	  		}
    	  		else {
    	  			delay=timeslot-slot;
              sch_flag = true;
              Simulator::Schedule(Seconds(delay),&SimpleNetDevice::ChannelSend,this,p,protocolNumber,rto,from);
    	  			Simulator::Schedule(Seconds(delay),&SimpleNetDevice::ChannelSend,this,p,protocolNumber,lto,from);
              Simulator::Schedule(Seconds(delay),&SimpleNetDevice::SetArray,this,m_sid,p);
              return true;
    	  		}
    	  }
    	  else if(m_sid%3 == 2){
    	  		if(slot==1){
              ChannelSend(p,protocolNumber,rto,from);
              ChannelSend(p,protocolNumber,lto,from);
    	  		}
            else if(slot==0){
              Simulator::Schedule(Seconds(1.0),&SimpleNetDevice::ChannelSend,this,p,protocolNumber,rto,from);
              Simulator::Schedule(Seconds(1.0),&SimpleNetDevice::ChannelSend,this,p,protocolNumber,lto,from);
            }
    	  		else{
              sch_flag = true;
    	  			delay=timeslot-slot+1;
              Simulator::Schedule(Seconds(delay),&SimpleNetDevice::ChannelSend,this,p,protocolNumber,rto,from);
    	  			Simulator::Schedule(Seconds(delay),&SimpleNetDevice::ChannelSend,this,p,protocolNumber,lto,from);
              Simulator::Schedule(Seconds(delay),&SimpleNetDevice::SetArray,this,m_sid,p);
              return true;
    	  		}
    	  }
    	  else {
    	  		if(slot==2){
              ChannelSend(p,protocolNumber,rto,from);
              ChannelSend(p,protocolNumber,lto,from);
    	  		}
            else if(slot==0){
              Simulator::Schedule(Seconds(2.0),&SimpleNetDevice::ChannelSend,this,p,protocolNumber,rto,from);
              Simulator::Schedule(Seconds(2.0),&SimpleNetDevice::ChannelSend,this,p,protocolNumber,lto,from);
            }
            else if(slot==1){
              Simulator::Schedule(Seconds(1.0),&SimpleNetDevice::ChannelSend,this,p,protocolNumber,rto,from);
              Simulator::Schedule(Seconds(1.0),&SimpleNetDevice::ChannelSend,this,p,protocolNumber,lto,from);
            }
    	  		else{
              sch_flag = true;
    	  			delay=timeslot-slot+2;
              Simulator::Schedule(Seconds(delay),&SimpleNetDevice::ChannelSend,this,p,protocolNumber,rto,from);
    	  			Simulator::Schedule(Seconds(delay),&SimpleNetDevice::ChannelSend,this,p,protocolNumber,lto,from);
              Simulator::Schedule(Seconds(delay),&SimpleNetDevice::SetArray,this,m_sid,p);

              return true;
    	  		}
    	  }

        m_rxArray[m_sid-1] = 2;
        m_rxArrayPacket[m_sid-1] = p -> Copy();
      }
      return true;
    }
  return true;
}

void
SimpleNetDevice::SetArray(int x,Ptr<Packet> p){
  m_rxArray[x-1] = 2;
  m_rxArrayPacket[x-1] = p ->Copy();
}

void
SimpleNetDevice::TransmitComplete ()
{
  NS_LOG_FUNCTION (this);

  if (m_queue->GetNPackets () == 0)
    {
      return;
    }

  Ptr<Packet> packet = m_queue->Dequeue ()->GetPacket ();

  SimpleTag tag;
  packet->RemovePacketTag (tag);

  Mac48Address src = tag.GetSrc ();
  Mac48Address dst = tag.GetDst ();
  uint16_t proto = tag.GetProto ();

  m_channel->Send (packet, proto, dst, src, this);

  if (m_queue->GetNPackets ())
    {
      Time txTime = Time (0);
      if (m_bps > DataRate (0))
        {
          txTime = m_bps.CalculateBytesTxTime (packet->GetSize ());
        }
      TransmitCompleteEvent = Simulator::Schedule (txTime, &SimpleNetDevice::TransmitComplete, this);
    }

  return;
}

Ptr<Node> 
SimpleNetDevice::GetNode (void) const
{
 // NS_LOG_FUNCTION (this);
  return m_node;
}
void 
SimpleNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}
bool 
SimpleNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION (this);
  if (m_pointToPointMode)
    {
      return false;
    }
  return true;
}
void 
SimpleNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_rxCallback = cb;
}

void
SimpleNetDevice::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_channel = 0;
  m_node = 0;
  m_receiveErrorModel = 0;
  m_queue->DequeueAll ();
  if (TransmitCompleteEvent.IsRunning ())
    {
      TransmitCompleteEvent.Cancel ();
    }
  NetDevice::DoDispose ();
}


void
SimpleNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION (this << &cb);
  m_promiscCallback = cb;
}

bool
SimpleNetDevice::SupportsSendFrom (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

} // namespace ns3
