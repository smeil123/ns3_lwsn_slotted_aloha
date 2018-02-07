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
#include <cstdlib>
#include <ns3/object.h>

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
    .AddAttribute ("temp_packet",
                   "A queue to use as the transmit queue in the device.",
                   StringValue ("ns3::DropTailQueue"),
                   MakePointerAccessor (&SimpleNetDevice::temp_queue),
                   MakePointerChecker<Queue> ())
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
  rack_flag = false;
  lack_flag = false;
  round = 3;
  k = 1;
  send_flag = false;
  m_txPacket = 0;
  wait_ack = false;
  receive_flag = false;
  receive_flag_1 = false;
  last_node = false;
  ndid = 1;
  m_count = 0;
  m_retrans_count = 0;
  m_collision = 0;
  temp_flag = true;
  txarray[0]=0;
  txarray[1]=0;
  minTime = 0;
  g_receive = 0;
}

void
SimpleNetDevice::SetTxPacket(Ptr<Packet> p){
  m_txPacket = p;
}

Ptr<Packet>
SimpleNetDevice::GetTxPacket(void) const{
  return m_txPacket;
}

void
SimpleNetDevice::SetRound(int x){
	round = x;
}

void
SimpleNetDevice::SetLastNode(bool x){
	last_node = x;
}

void
SimpleNetDevice::ReceiveStart(Ptr<Packet> packet, uint16_t protocol,
                          Mac48Address to, Mac48Address from)
{
        if (to == m_address){ 
	        if(receive_flag==true){
		        receive_flag_1 = true;
	        }else{
		        receive_flag = true;
                        Simulator::Schedule(Seconds(0.9),&SimpleNetDevice::ReceiveCheck,this,packet,protocol,to,from);
	        }
	}

}

void
SimpleNetDevice::ReceiveCheck(Ptr<Packet> packet, uint16_t protocol,
                          Mac48Address to, Mac48Address from)
{
	if(receive_flag_1==false){
		Simulator::ScheduleNow(&SimpleNetDevice::Receive,this,packet,protocol,to,from);
	}
  else{
    m_collision++;
    NS_LOG_FUNCTION("Sid : "<<m_sid<<"collison!");
  }

	receive_flag = false;
	receive_flag_1 = false;
}

void
SimpleNetDevice::Receive (Ptr<Packet> packet, uint16_t protocol,
                          Mac48Address to, Mac48Address from)
{
//  NS_LOG_FUNCTION (this << packet << protocol << to << from);
  NetDevice::PacketType packetType;

  if (m_receiveErrorModel && m_receiveErrorModel->IsCorrupt (packet) )
    {
      m_phyRxDropTrace (packet);
      return;
    }
  if (to == m_address && send_flag==false)
    {

      packetType = NetDevice::PACKET_HOST;
  	  
  	  Ptr<Packet> p = packet->Copy ();

      LwsnHeader receiveheader;
      p ->PeekHeader(receiveheader);

      NS_LOG_FUNCTION ("Sid -> " << this->GetSid() << ": from -> " <<receiveheader.GetPsid() << "Packet Type :" << receiveheader.GetType());
      NS_LOG_FUNCTION("Sid->" <<m_sid << "Did -> "<<receiveheader.GetDid());
      
      if(this->GetSid()==0){
        
        if(receiveheader.GetType() == LwsnHeader::ORIGINAL_TRANSMISSION || receiveheader.GetType() == LwsnHeader::FORWARDING){
          /*g_receive++;
          for(uint16_t i =0; i<m_queue->GetNPackets(); i++){
            Ptr<Packet> temp = temp_queue->Dequeue()->GetPacket ();
            LwsnHeader tempheader;
            temp-> PeekHeader(tempheader);
            if(tempheader.GetOsid() == receiveheader.GetOsid()){
              if(tempheader.GetDid() == receiveheader.GetDid() ){
                NS_LOG_UNCOND("GateWay" << m_gid<<" already receive");
                return;
              }
            }
          }
          m_queue->Enqueue(Create<QueueItem> (packet));*/
          g_receive++;
          int pre = m_queue->GetNPackets();
          QueueCheck(packet);
          int post = m_queue->GetNPackets();

          if(pre<post){

    	 	    //int time = Simulator::Now().GetSeconds();
          	NS_LOG_UNCOND("---------------------------------------");
          	NS_LOG_UNCOND("GateWay "<<m_gid<< " : Receive Sid ->"<<receiveheader.GetOsid() << " Did ->"<<receiveheader.GetDid());
            NS_LOG_UNCOND("Start Time : "<<receiveheader.GetStartTime());
            //NS_LOG_UNCOND("Total Time : "<<time-receiveheader.GetStartTime()+1);
          	NS_LOG_UNCOND("Total Time : "<<receiveheader.GetStartTime2());
          	NS_LOG_UNCOND("---------------------------------------");
          }
        }
 
      	return;
      }

      if(receiveheader.GetType() == LwsnHeader::ORIGINAL_TRANSMISSION){    

      	if(wait_ack == false){
      		send_flag = true;
      		
      		if(from == m_raddress){
	      		//ack send
	      		Simulator::Schedule(Seconds(0.1),&SimpleNetDevice::AckSend,this,packet,protocol,m_raddress);
		      	//forwarding
		      	//NS_LOG_UNCOND("Sid ->"<<this->GetSid()<<"Forwading to "<<m_laddress);
		      	Simulator::Schedule(Seconds(0.1),&SimpleNetDevice::Forwarding,this,packet,protocol,m_laddress); 		
	      	}

	      	else if(from == m_laddress){
	      		//ack send
	      		Simulator::Schedule(Seconds(1.1),&SimpleNetDevice::AckSend,this,packet,protocol,m_laddress);
  	   			//forwarding
  	   			//NS_LOG_UNCOND("Sid ->"<<this->GetSid()<<"Forwading to "<<m_raddress);
  	     		Simulator::Schedule(Seconds(1.1),&SimpleNetDevice::Forwarding,this,packet,protocol,m_raddress);
	      	}
		}
	 	else{
			NS_LOG_UNCOND("Sid ->" << this->GetSid() << " not receive ");
			 if(m_txPacket != 0){
	                Ptr<Packet> temp = m_txPacket -> Copy();
	                LwsnHeader tempheader;
	                temp -> PeekHeader(tempheader);
	                //temp->AddHeader(tempheader);

	                if(tempheader.GetDid() != receiveheader.GetDid() || tempheader.GetOsid()!= receiveheader.GetOsid()){
	                                          
	                    if(m_queue->GetNPackets () > 0){
			         		     //  Ptr<Packet> queue_packet = m_queue->Peek()->GetPacket ();
			           		   //  LwsnHeader tempheader_1;
			           		   //  queue_packet->PeekHeader(tempheader_1);
	                  //       //queue_packet->AddHeader(tempheader_1);

      			        		// if(tempheader_1.GetDid()!=receiveheader.GetDid() || tempheader_1.GetOsid()!=receiveheader.GetOsid()){
      			          //     	    m_queue->Enqueue (Create<QueueItem> (packet));
      			          //      	}

                        QueueCheck(packet);
      			         	}
      			           	else{
      			               	m_queue->Enqueue (Create<QueueItem> (packet));
      			            }       
      	              }
      	            }
	            //m_txPacket == 0 
	            else{
                QueueCheck(packet);
	              //   if(m_queue->GetNPackets () > 0){
			       		   //  Ptr<Packet> queue_packet = m_queue->Peek()->GetPacket ();
			            //   LwsnHeader tempheader_1;
			            //   queue_packet->PeekHeader(tempheader_1);
	              //       //queue_packet->AddHeader(tempheader_1);
			            //   if(tempheader_1.GetDid()!=receiveheader.GetDid() || tempheader_1.GetOsid()!=receiveheader.GetOsid()){
			            //     m_queue->Enqueue (Create<QueueItem> (packet));
			           	//   }
			       	    //  }
    			        // else{
    			        //    	m_queue->Enqueue (Create<QueueItem> (packet));
    			       	//}
	            }           
		     }
	  }
	
       else if(receiveheader.GetType() == LwsnHeader::IACK){
      	//본인이 원전송 혹은 재전송한 패킷을 나중에 다시 받을 수 있으니까 그때 큐를 확인하고 큐에서 제거 필요 
	        
	        //받아야하는 ACK가 있는 경우       
       		if(wait_ack){
       			Ptr<Packet> temp = m_txPacket->Copy();
	            LwsnHeader tempheader;
	            temp -> PeekHeader(tempheader);

	            if(tempheader.GetDid() == receiveheader.GetDid() && tempheader.GetOsid() == receiveheader.GetOsid()){
			         NS_LOG_UNCOND("Sid-> "<<this->GetSid()<<"  ACK RECEIVE");
			         AckReceive(true,from);
			         //Simulator::ScheduleNow(&SimpleNetDevice::AckReceive,this,true,from);
			         //Simulator::Schedule(Seconds(1.0),&SimpleNetDevice::SetSleep,this);
			    }
    			    else{
    			        NS_LOG_UNCOND("Sid ->" << this->GetSid() <<" not except packet,did"<<tempheader.GetDid());
    			    }
       		}
       		else{
       			if(m_queue->GetNPackets() > 0){
		        	Ptr<Packet> queue_packet = m_queue->Peek()->GetPacket ();
				      LwsnHeader tempheader_1;
				      queue_packet->PeekHeader(tempheader_1);
	                //queue_packet->AddHeader(tempheader_1);
  				    if(tempheader_1.GetDid()==receiveheader.GetDid() && tempheader_1.GetOsid()==receiveheader.GetOsid()){
  				        queue_packet = m_queue->Dequeue()->GetPacket();
  				    }
		        }
       		}
	  }

      else if(receiveheader.GetType() == LwsnHeader::FORWARDING){
      	if(wait_ack == false){
      		if(m_queue->GetNPackets () > 0){
		        Ptr<Packet> queue_packet = m_queue->Peek()->GetPacket ();
		        LwsnHeader tempheader_1;
		        queue_packet->PeekHeader(tempheader_1);

		        if(tempheader_1.GetDid()==receiveheader.GetDid()&&tempheader_1.GetOsid()==receiveheader.GetOsid()){
				      queue_packet = m_queue->Dequeue()->GetPacket();
		        }   
          }
      	send_flag = true;
		    if(from == m_raddress){
		  		//ack send
		      Simulator::Schedule(Seconds(0.1),&SimpleNetDevice::AckSend,this,packet,protocol,m_raddress);
		  		//forwarding
		   		Simulator::Schedule(Seconds(0.1),&SimpleNetDevice::Forwarding,this,packet,protocol,m_laddress);
		    }
		    else if(from == m_laddress){
		      //ack send
		      Simulator::Schedule(Seconds(0.1),&SimpleNetDevice::AckSend,this,packet,protocol,m_laddress);
		      //forwarding
		      Simulator::Schedule(Seconds(0.1),&SimpleNetDevice::Forwarding,this,packet,protocol,m_raddress);
		    }
		 }
		 // ack를 기다리는 중일 경우, 패킷을 받는다?
		 else{
		    if(m_txPacket != 0){
                Ptr<Packet> temp = m_txPacket -> Copy();
                LwsnHeader tempheader;
                temp -> PeekHeader(tempheader);
                //temp->AddHeader(tempheader);
                // 받아야 하는 패킷이면 큐에 넣는다 
                if(tempheader.GetDid() != receiveheader.GetDid() || tempheader.GetOsid()!= receiveheader.GetOsid()){
                                         
                    if(m_queue->GetNPackets () > 0){
		         		     //  Ptr<Packet> queue_packet = m_queue->Peek()->GetPacket ();
		               	//   LwsnHeader tempheader_1;
		               	//   queue_packet->PeekHeader(tempheader_1);
                  //       //p->AddHeader(tempheader_1);

		                // if(tempheader_1.GetDid()!=receiveheader.GetDid()||tempheader_1.GetOsid()!=receiveheader.GetOsid()){
                  //           m_queue->Enqueue (Create<QueueItem> (packet));                                
		                //  }
                      QueueCheck(packet);
		             }
    		            else{
    		                m_queue->Enqueue (Create<QueueItem> (packet));
    		            }       
                }
            }
            else{
          //       if(m_queue->GetNPackets () > 0){
  		      //     	Ptr<Packet> queue_packet = m_queue->Peek()->GetPacket ();
  		      //       LwsnHeader tempheader_1;
  		      //       queue_packet->PeekHeader(tempheader_1);
          //             //p->AddHeader(tempheader_1);
  		      //       if(tempheader_1.GetDid()!=receiveheader.GetDid()||tempheader_1.GetOsid()!=receiveheader.GetOsid()){
  		      //           m_queue->Enqueue (Create<QueueItem> (packet));
  		      //       }
		        // }
    		    //     else{
    		    //         m_queue->Enqueue (Create<QueueItem> (packet));
    		    //     }
              QueueCheck(packet);
            }           
		}
    }
      else {
      		NS_LOG_UNCOND("UNDEFINE HEADER TYPE");
      		}
   	 	
	}else if(to == m_address && send_flag){
    m_collision++;
  }


  else if (to.IsBroadcast ())
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
    }
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
  //NS_LOG_FUNCTION (this);
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
void
SimpleNetDevice::SetSid(uint16_t sid)
{
	m_sid = sid;
}

uint16_t
SimpleNetDevice::GetSid()
{
	return m_sid;
}

void
SimpleNetDevice::SetGid(int x)
{
	m_gid = x;
}

int
SimpleNetDevice::GetGid()
{
	return m_gid;
}

void
SimpleNetDevice::SetSideAddress(Address laddress,Address raddress)
{
	m_laddress = Mac48Address::ConvertFrom(laddress);
	m_raddress = Mac48Address::ConvertFrom(raddress);
}

Mac48Address
SimpleNetDevice::GetLaddress()
{
	return m_laddress;
}

Mac48Address
SimpleNetDevice::GetRaddress()
{
	return m_raddress;
}

void
SimpleNetDevice::AckReceive(bool x,Mac48Address from)
{
	if(from == m_raddress){
    rack_flag = x;
	}
	else if(from == m_laddress){
    lack_flag = x;
	}
	return ;
}
void
SimpleNetDevice::AckSend(Ptr <Packet> p, uint16_t protocol,Mac48Address to)
{
	NS_LOG_FUNCTION("Sid -> " << this->GetSid() <<"AckSend  to " << to);
	send_flag = true;

  Ptr<Packet> ackpacket =p->Copy();

	LwsnHeader tempheader;
	ackpacket->RemoveHeader(tempheader);        

	LwsnHeader ackheader;
	ackheader.SetOsid(tempheader.GetOsid());
	ackheader.SetPsid(this->GetSid());
	ackheader.SetE(0);
	ackheader.SetR(0);
  ackheader.SetDid(tempheader.GetDid());
	ackheader.SetType(LwsnHeader::IACK);
	ackheader.SetStartTime(tempheader.GetStartTime());

	ackpacket->AddHeader(ackheader);

	Mac48Address from = Mac48Address::ConvertFrom (this->GetAddress());

	ChannelSend(ackpacket,protocol,to,from);
}
void
SimpleNetDevice::WaitSend()
{
	if(send_flag == false){
		if(m_queue->GetNPackets()>0){

		  NS_LOG_UNCOND("Sid"<<this->GetSid()<<" WaitSend Queue Packet #  "<<m_queue -> GetNPackets());

			Ptr<Packet> packet;
			packet = m_queue->Dequeue()->GetPacket();

		  LwsnHeader tempheader;
		  packet->RemoveHeader(tempheader);

			LwsnHeader sendheader;
			sendheader.SetOsid(tempheader.GetOsid());
			sendheader.SetPsid(this->GetSid());
			sendheader.SetE(0);
			sendheader.SetR(0);
	    sendheader.SetDid(tempheader.GetDid());
      if(tempheader.GetType()==LwsnHeader::ORIGINAL_TRANSMISSION){
        if(tempheader.GetOsid()==m_sid){
          sendheader.SetType(tempheader.GetType());
        }
        else{
          sendheader.SetType(LwsnHeader::FORWARDING);
        }
      }
      else{
        sendheader.SetType(tempheader.GetType());
      }
			sendheader.SetStartTime(tempheader.GetStartTime());

			packet->AddHeader(sendheader);

			m_txPacket = packet ->Copy();

			if(sendheader.GetType()==LwsnHeader::ORIGINAL_TRANSMISSION){
				OriginalTransmission(packet,0,true);
			}
			else if(sendheader.GetType()==LwsnHeader::FORWARDING){
				if(sendheader.GetOsid() > this->GetSid()){
					Forwarding(packet,0,m_laddress);
					AckSend(packet,0,m_raddress);
				}
				else{
					Forwarding(packet,0,m_raddress);
					AckSend(packet,0,m_laddress);
				}
			}
		}
		else {
			NS_LOG_UNCOND("SID" << m_sid << "  NO WAITING PACKET");
    }
	}
} 

void
SimpleNetDevice::QueueCheck(Ptr<Packet> p){
  NS_LOG_FUNCTION("Sid =>"<<m_sid);
  Ptr<Packet> receivepacket = p -> Copy();
  LwsnHeader receiveheader;

  if(m_sid == 0){
    p->RemoveHeader(receiveheader);
    int time = Simulator::Now().GetSeconds();
    receiveheader.SetStartTime2(time-receiveheader.GetStartTime()+1);
    NS_LOG_UNCOND("time : "<<time);
    NS_LOG_UNCOND("starttime2 : " <<receiveheader.GetStartTime2());

    p -> AddHeader(receiveheader);
    Ptr<Packet> receivepacket = p -> Copy();

  }
  receivepacket -> PeekHeader(receiveheader);
  int t=0;
  int n=0;
    if(m_queue->GetNPackets () > 0){
      while(m_queue->GetNPackets() > 0){

        Ptr<Packet> queue_packet = m_queue->Dequeue()->GetPacket ();
        LwsnHeader tempheader;
        queue_packet->PeekHeader(tempheader);

        if(tempheader.GetDid()==receiveheader.GetDid() && tempheader.GetOsid()==receiveheader.GetOsid()){
          n = 1;
          temp_queue->Enqueue(Create<QueueItem> (queue_packet));
          NS_LOG_UNCOND("------------same packet -----Sid -> "<< receiveheader.GetOsid()<<"-----");
          break;       
        }
          else{          
          temp_queue->Enqueue(Create<QueueItem> (queue_packet));
        }
      }
      if(n != 1){
        m_queue -> Enqueue(Create<QueueItem> (p));
      }

      if(m_queue->GetNPackets()==0){
        t = temp_queue ->GetNPackets();
        for(int i =0; i < t; i++){        

          Ptr<Packet> temp = temp_queue->Dequeue()->GetPacket ();

          m_queue->Enqueue(Create<QueueItem> (temp));

        }
      }else{
        t = m_queue->GetNPackets();
        for(int i = 0; i < t;i++){                  
          Ptr<Packet> temp = m_queue->Dequeue()->GetPacket ();

          temp_queue->Enqueue(Create<QueueItem> (temp));
        }
        t = temp_queue->GetNPackets();
        for(int i = 0; i < t; i++){

          Ptr<Packet> temp1 = temp_queue->Dequeue()->GetPacket ();

          m_queue->Enqueue(Create<QueueItem> (temp1));
        }
      }
    }else{
      m_queue->Enqueue(Create<QueueItem>(p));
    }
}

int 
SimpleNetDevice::RandTime()
{
  
	if(k>round)
	{
		k = 1;
		return 100;
	}
	/*else if (k==0)
	{
		if(rand()%2 == 0)
		{	
			k++;
			NS_LOG_UNCOND("Sid : "<<m_sid<<" rand time -> 0");
			return 0;
		}
		else
		{	
			k++;
			NS_LOG_UNCOND("Sid : "<<m_sid<<"rand time -> 1");
			return 1;
		}
    // maxTime = 1;

    // r->SetAttribute ("Min", DoubleValue (minTime));
    // r->SetAttribute ("Max", DoubleValue (maxTime));
    // int x = r->GetInteger();
    // NS_LOG_UNCOND("Sid : " << m_sid <<"rand time -> " << x );
    // return x;

	}*/
	else
	{
    Ptr<UniformRandomVariable> r = CreateObject<UniformRandomVariable> ();
    maxTime = pow(2,k++)-1;
    r->SetAttribute ("Min", DoubleValue (minTime));
    r->SetAttribute ("Max", DoubleValue (maxTime));
    int x = r->GetInteger();
    // int x = (int)(rand()%(int)(pow(2,k)-1));
		NS_LOG_UNCOND("Sid : " << m_sid <<"rand time -> " << x );
		return x;
	}


}
void
SimpleNetDevice::AckCheck(Ptr<Packet> p, uint16_t protocol, Mac48Address to, Mac48Address from){

	if (to == m_raddress){
	    if(rack_flag){
				NS_LOG_FUNCTION("Sid -> " << this->GetSid() <<"AckReceive Success to " << to);
	    	rack_flag = false;
	      k = 1;
	      wait_ack = false;
        m_txPacket=0;
        WaitSend();

	    }
	    else{
	        //retransmission
	        //random time
          LwsnHeader temp;
          p->PeekHeader(temp);
	        uint16_t delay = RandTime();
	        if(delay == 100) {
	        	NS_LOG_FUNCTION("---------------------------");
	        	NS_LOG_FUNCTION("Sid -> " << this->GetSid()<< "Send Fail to" << to << "  Osid :" << temp.GetOsid());
	        	NS_LOG_FUNCTION("---------------------------");
	        	k = 1;
	        	wait_ack = false;
            m_txPacket=0;
            rack_flag = false;
            WaitSend();
	        }
	        else{
            NS_LOG_FUNCTION("Sid->" <<m_sid << "Did -> "<<temp.GetDid());
	        	Simulator::Schedule(Seconds(delay),&SimpleNetDevice::ReSend,this,p,protocol,to,from);
	    	}
	    }
	}
	else if (to == m_laddress){
		if(lack_flag){
			NS_LOG_FUNCTION("Sid -> " << this->GetSid() <<"AckReceive Success to " << to);
			lack_flag = false;
			k = 1;
	    wait_ack = false;
	    m_txPacket=0;
	    WaitSend();
	  }
	    else{
	        //retransmission
	        //random time
	        uint16_t delay = RandTime();
          LwsnHeader temp;
          p->PeekHeader(temp);
	        if(delay == 100) {
	        	NS_LOG_FUNCTION("---------------------------");	        	
	        	NS_LOG_FUNCTION("Sid -> " << this->GetSid()<< "Send Fail to" << to<< "  Osid :" << temp.GetOsid());
	        	NS_LOG_FUNCTION("---------------------------");
	        	k = 1;
	        	wait_ack = false;
            m_txPacket=0;
				    lack_flag = false;
            WaitSend();		
	        }
	        else{
        		LwsnHeader temp;
            p->PeekHeader(temp);
            NS_LOG_FUNCTION("Sid->" <<m_sid << "Did -> "<<temp.GetDid());
	        	Simulator::Schedule(Seconds(delay),&SimpleNetDevice::ReSend,this,p,protocol,to,from);
	        }
	    }
	}
}

void
SimpleNetDevice::OriginalAckCheck(Ptr<Packet> p, uint16_t protocol){

  NS_LOG_FUNCTION("sid -> " << m_sid);
	if((rack_flag==false)&&(lack_flag==false)){
		uint16_t delay = RandTime();
	    if(delay == 100) {
	        NS_LOG_FUNCTION("---------------------------");
	     	NS_LOG_FUNCTION("Sid -> " << this->GetSid()<< "Send Fail to" << m_raddress <<", "<<m_laddress);
	        NS_LOG_FUNCTION("---------------------------");
	     	k = 1;
	     	wait_ack = false;
            m_txPacket=0;
            WaitSend();
            rack_flag = false;
            lack_flag = false;
	    }
	    else{
	    	Simulator::Schedule(Seconds(delay),&SimpleNetDevice::ReOriginalSend,this,p,0);
	    }
	}
	else if((rack_flag)&&(lack_flag)){
		 NS_LOG_FUNCTION("Sid -> " << this->GetSid() <<"AckReceive Success to " << m_raddress);
		 NS_LOG_FUNCTION("Sid -> " << this->GetSid() <<"AckReceive Success to " << m_laddress);
		 rack_flag = false;	lack_flag = false;
	     wait_ack = false;
	     k = 1;
         m_txPacket=0;
         WaitSend();
	}
	else{
    LwsnHeader temp;
    p->PeekHeader(temp);
		if(!rack_flag){
			//retransmission
		    //random time
		    uint16_t delay = RandTime();
		    if(delay == 100) {
	        	NS_LOG_FUNCTION("---------------------------");
		     	  NS_LOG_FUNCTION("Sid -> " << this->GetSid()<< "Send Fail to" << m_raddress<< "  Osid :" << temp.GetOsid());
	        	NS_LOG_FUNCTION("---------------------------");
	        	wait_ack = false;	
	         	k = 1;
	         	m_txPacket=0;
	         	WaitSend();
		   	}
        else{
		   	  Simulator::Schedule(Seconds(delay),&SimpleNetDevice::ReSend,this,p,protocol,m_raddress,m_address);
        }
		}
		else if(!lack_flag){
		    //retransmission
		    //random time
		    uint16_t delay = RandTime();
	        if(delay == 100) {
	        	NS_LOG_FUNCTION("---------------------------");
	        	NS_LOG_FUNCTION("Sid -> " << this->GetSid()<< "Send Fail to" << m_laddress<< "  Osid :" << temp.GetOsid());
	        	NS_LOG_FUNCTION("---------------------------");
	        	wait_ack = false;
	        	k = 1;
	         	m_txPacket=0;
	         	WaitSend();
	        }
	        else{
	        	Simulator::Schedule(Seconds(delay),&SimpleNetDevice::ReSend,this,p,protocol,m_laddress,m_address);
	        }
		    
		}
	}
	
	
}

void
SimpleNetDevice::OriginalWaitAck(Ptr<Packet> p, uint16_t protocol, Mac48Address from){

	wait_ack = true;

	if(last_node){
		if(m_sid == 1){
			lack_flag = true;
			rack_flag = false;
		}
		else{
			rack_flag = true;
			lack_flag = false;
		}
	}
	else{
		lack_flag = false;
		rack_flag = false;
	}
	Simulator::Schedule(Seconds(3.0),&SimpleNetDevice::OriginalAckCheck,this,p,protocol);
}

void
SimpleNetDevice::WaitAck(Ptr<Packet> p, uint16_t protocol, Mac48Address to, Mac48Address from)
{
  NS_LOG_FUNCTION("Sid ->"<<m_sid);
	lack_flag = false;
	rack_flag = false;
	if(!last_node){
		wait_ack = true;
		Simulator::Schedule(Seconds(2.0),&SimpleNetDevice::AckCheck,this,p,protocol,to,from);
	}	
  else{
     if(m_sid == 1){
       if(to == m_raddress){
        wait_ack = true;
        Simulator::Schedule(Seconds(2.0),&SimpleNetDevice::AckCheck,this,p,protocol,to,from);
       }
       else{
        NS_LOG_FUNCTION("Sid : "<<m_sid<<"Wait Send");
        m_txPacket=0;
        Simulator::Schedule(Seconds(2.0),&SimpleNetDevice::WaitSend,this);
       }

    }
     else{
      if(to==m_laddress){
        wait_ack = true;
        Simulator::Schedule(Seconds(2.0),&SimpleNetDevice::AckCheck,this,p,protocol,to,from);
      }
      else{
        NS_LOG_FUNCTION("Sid : "<<m_sid<<"Wait Send");
        m_txPacket=0;
        Simulator::Schedule(Seconds(2.0),&SimpleNetDevice::WaitSend,this);

      }
     }
  }
	
}
void
SimpleNetDevice::ReSend(Ptr<Packet> p, uint16_t protocol, Mac48Address to, Mac48Address from)
{

  if(to == m_raddress){
    if(rack_flag){
      k = 1;
      wait_ack = false;
      m_txPacket=0;
      rack_flag = false;
      WaitSend();
      return ;
    }
  }
  else{
    if(lack_flag){
      k = 1;
      wait_ack = false;
      m_txPacket=0;
      lack_flag = false;
      WaitSend();
      return;
    }
  }

	send_flag = true;
  m_retrans_count++;


	NS_LOG_FUNCTION ("Sid -> "<<this->GetSid() << "  retransmission to :"<< to);

  Ptr<Packet> repacket = p->Copy();
  Ptr<Packet> ackpacket = p->Copy();

	LwsnHeader tempheader;
	repacket->RemoveHeader(tempheader);

  NS_LOG_FUNCTION("Sid->" <<m_sid << "Osid -> "<<tempheader.GetOsid());
    
	LwsnHeader retransheader;
	retransheader.SetOsid(tempheader.GetOsid());
	retransheader.SetPsid(this->GetSid());
	retransheader.SetE(0);
	retransheader.SetR(1);
  retransheader.SetDid(tempheader.GetDid());
	retransheader.SetType(LwsnHeader::FORWARDING);
	retransheader.SetStartTime(tempheader.GetStartTime());

  LwsnHeader ackheader;
	ackheader.SetOsid(tempheader.GetOsid());
	ackheader.SetPsid(this->GetSid());
	ackheader.SetE(0);
	ackheader.SetR(1);
  ackheader.SetDid(tempheader.GetDid());
	ackheader.SetType(LwsnHeader::IACK);
	ackheader.SetStartTime(tempheader.GetStartTime());

	repacket->AddHeader(retransheader);
  ackpacket->AddHeader(ackheader);

    if(to == m_laddress){
      ChannelSend(ackpacket,protocol,m_raddress,from);      
    }
    else{
      ChannelSend(ackpacket,protocol,m_laddress,from); 
    }
    ChannelSend(repacket,protocol,to,from); 
    WaitAck(repacket,0,to,from);
}

void
SimpleNetDevice::ReOriginalSend(Ptr<Packet> p, uint16_t protocol){
  if(lack_flag&&rack_flag){
    k = 1;
    wait_ack = false;
    m_txPacket=0;
    rack_flag = false;
    lack_flag = false;
    WaitSend();
    return;
  }
  else if(lack_flag){
    lack_flag = false;
    ReSend(p,protocol,m_raddress,m_address);
    return;
  }
  else if(rack_flag){
    rack_flag = false;
    ReSend(p,protocol,m_laddress,m_address);
    return;
  }

	send_flag = true;
  m_retrans_count++;
	
	NS_LOG_FUNCTION ("Sid -> "<<this->GetSid() << "  retransmission OriginalTransmission ");    

	Ptr<Packet> repacket = p->Copy();
	LwsnHeader tempheader;
	repacket->RemoveHeader(tempheader);

	LwsnHeader retransheader;
	retransheader.SetOsid(tempheader.GetOsid());
	retransheader.SetPsid(this->GetSid());
	retransheader.SetE(0);
	retransheader.SetR(1);
  retransheader.SetDid(tempheader.GetDid());
	retransheader.SetType(LwsnHeader::ORIGINAL_TRANSMISSION);
	retransheader.SetStartTime(tempheader.GetStartTime()); 

	repacket->AddHeader(retransheader);

	Mac48Address r_to = Mac48Address::ConvertFrom (m_raddress);
	Mac48Address from = Mac48Address::ConvertFrom (m_address);
	Mac48Address l_to = Mac48Address::ConvertFrom (m_laddress);

  ChannelSend(repacket,protocol,r_to,from);
  ChannelSend(repacket,protocol,l_to,from);

	OriginalWaitAck(repacket,0,from);
}

void
SimpleNetDevice::SetSleep()
{
  if(m_queue -> GetNPackets() >0)
  	NS_LOG_UNCOND("Sid ->"<<this->GetSid()<<"ONE MORE PACKET IS WAITING");
  send_flag = false;
}

bool
SimpleNetDevice::OriginalTransmission(Ptr<Packet> p, uint16_t protocolNumber,bool header){


	  Ptr<Packet> packet = p->Copy ();

	  Mac48Address r_to = Mac48Address::ConvertFrom (m_raddress);
	  Mac48Address l_to = Mac48Address::ConvertFrom (m_laddress);
	  Mac48Address from = Mac48Address::ConvertFrom (m_address);

	  int time = Simulator::Now().GetSeconds();

	  LwsnHeader sendheader;
    if(!header){   
      sendheader.SetOsid(m_sid);
      sendheader.SetPsid(m_sid);
      sendheader.SetR(0);
      sendheader.SetType(LwsnHeader::ORIGINAL_TRANSMISSION);
      sendheader.SetStartTime(time);
      sendheader.SetDid(ndid++);
    }
    else{   
      LwsnHeader header;
      p -> RemoveHeader(header);

      sendheader.SetOsid(m_sid);
      sendheader.SetPsid(m_sid);
      sendheader.SetR(0);
      sendheader.SetType(LwsnHeader::ORIGINAL_TRANSMISSION);
      sendheader.SetStartTime(header.GetStartTime());
      sendheader.SetDid(header.GetDid());
    }
	  packet->AddHeader(sendheader);

	  NS_LOG_UNCOND("Sid"<<this->GetSid()<<" OriginalTransmission Queue Packet #  "<<m_queue -> GetNPackets());

	  if( send_flag == true || wait_ack ){

	  	m_queue->Enqueue (Create<QueueItem> (packet));

	  	NS_LOG_UNCOND("Sid"<<this->GetSid()<<"  SendFrom Function m_queue packet # 1 more");
	  	
	  	return 0;
	  }

	  if (m_queue->Enqueue (Create<QueueItem> (packet)))
	    {
	       send_flag = true;
	       p = m_queue->Dequeue()->GetPacket ();

	       Time txTime = Time (0);
	       if (m_bps > DataRate (0))
	       {
	         txTime = m_bps.CalculateBytesTxTime (packet->GetSize ());
	       }
	       m_txPacket = p -> Copy();
         txarray[0] = m_sid;
         txarray[1] = ndid-1;
         ChannelSend(p,protocolNumber,r_to,from);
         ChannelSend(p,protocolNumber,l_to,from);

         OriginalWaitAck(p,0,from);
	        
	      return true;
	    }

	  return true;	

}

void
SimpleNetDevice::Forwarding(Ptr<Packet> p,uint16_t protocol, Mac48Address to)
{
	send_flag = true;

  Ptr<Packet> packet = p->Copy();
  LwsnHeader receiveheader;
  packet->RemoveHeader(receiveheader);       

  if(txarray[0] == receiveheader.GetOsid()){
    if(txarray[1] == receiveheader.GetDid()){
      m_retrans_count++;
      AckSend(p,0,to);
      return;
    }
  }
  NS_LOG_FUNCTION("Sid->" <<m_sid << "Osid -> "<<receiveheader.GetOsid());
	LwsnHeader forwardingheader;
	forwardingheader.SetOsid(receiveheader.GetOsid());
	forwardingheader.SetPsid(this->GetSid());
	forwardingheader.SetE(0);
	forwardingheader.SetR(0);        
  forwardingheader.SetDid(receiveheader.GetDid());
	forwardingheader.SetType(LwsnHeader::FORWARDING);
	forwardingheader.SetStartTime(receiveheader.GetStartTime());



  txarray[0] = forwardingheader.GetOsid();
  txarray[1] = forwardingheader.GetDid();

	packet->AddHeader(forwardingheader);

	Mac48Address from = Mac48Address::ConvertFrom (this->GetAddress());

	m_txPacket = packet -> Copy();

  ChannelSend(packet,protocol,to,from);        

	WaitAck(packet,protocol,to,from);

}

void
SimpleNetDevice::ChannelSend(Ptr<Packet> p, uint16_t protocol, Mac48Address to, Mac48Address from)
{
	NS_LOG_FUNCTION ("Sid ->"<<this->GetSid() <<"ChannelSend to  " << to << "m_count : "<<m_count<<"  m_retrans_count"<<m_retrans_count);
  m_count++;
	m_channel -> Send(p,protocol,to,from,this);
	Simulator::Schedule(Seconds(1.0),&SimpleNetDevice::SetSleep,this);
}

void
SimpleNetDevice::Print()
{
  if(m_sid == 0){
    NS_LOG_UNCOND("============================");
    NS_LOG_UNCOND("GateWay : "<<m_gid<<"receive packet : " << m_queue->GetNPackets());
    int j = m_queue->GetNPackets();
    for(int i =0; i<j;i++){
      LwsnHeader temp;
      Ptr<Packet> p = m_queue->Dequeue()->GetPacket();

      p->RemoveHeader(temp);
      NS_LOG_UNCOND(i+1 << " :: packet Osid : "<<temp.GetOsid()<<" Did : "<<temp.GetDid()<<" Start Time: "<<temp.GetStartTime()<<"  Total Time : " <<temp.GetStartTime2());
    }
    NS_LOG_UNCOND("============================");
  }
  else{
    NS_LOG_UNCOND("============================");
    NS_LOG_UNCOND("Sid : "<<m_sid<<"Send Count : " << m_count/2 << "  RESend Count : "<<m_retrans_count);
    NS_LOG_UNCOND("============================");
  }
}

bool 
SimpleNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  //NS_LOG_FUNCTION (this << packet << dest << protocolNumber);

  return SendFrom (packet, m_address, dest, protocolNumber);
}

bool
SimpleNetDevice::SendFrom (Ptr<Packet> p, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  //NS_LOG_FUNCTION (this << p << source << dest << protocolNumber);
  if (p->GetSize () > GetMtu ())
    {
      return false;
    }
  Ptr<Packet> packet = p->Copy ();

  Mac48Address to = Mac48Address::ConvertFrom (dest);
  Mac48Address from = Mac48Address::ConvertFrom (source);

  SimpleTag tag;
  tag.SetSrc (from);
  tag.SetDst (to);
  tag.SetProto (protocolNumber);

  p->AddPacketTag (tag);
  if(m_queue -> GetNPackets() > 0){
  	m_queue->Enqueue (Create<QueueItem> (p));
  	//
/*  	if(ack_flag == true){
  		Simulator::ScheduleNow(&SimpleNetDevice::WaitSend,this,p,source,dest,protocolNumber);
  	}
  	else{
  		Simulator::ScheduleNow(&SimpleNetDevice::WaitAck,this,p,source,dest,protocolNumber);
  	}*/
  	NS_LOG_UNCOND("Sid"<<this->GetSid()<<"  SendFrom Function m_queue packet # 1 more");
  	
  	return 0;
  }

  if (m_queue->Enqueue (Create<QueueItem> (p)))
    {
      if (m_queue->GetNPackets () == 1 && !TransmitCompleteEvent.IsRunning ())
        {
          p = m_queue->Dequeue ()->GetPacket ();
          p->RemovePacketTag (tag);
          Time txTime = Time (0);
          if (m_bps > DataRate (0))
            {
              txTime = m_bps.CalculateBytesTxTime (packet->GetSize ());
            }
          //m_channel->Send (p, protocolNumber, to, from, this);
          //TransmitCompleteEvent = Simulator::Schedule (txTime, &SimpleNetDevice::TransmitComplete, this);
          m_txPacket = p -> Copy();
          Simulator::ScheduleNow(&SimpleNetDevice::ChannelSend,this,p,protocolNumber,to,from);
          Simulator::Schedule(Seconds(1.0),&SimpleNetDevice::WaitAck,this,p,protocolNumber,to,from);

        }
      return true;
    }


  m_channel->Send (packet, protocolNumber, to, from, this);
  return true;
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

