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
#ifndef SIMPLE_NET_DEVICE_H
#define SIMPLE_NET_DEVICE_H

#include <stdint.h>
#include <string>
#include "ns3/traced-callback.h"
#include "ns3/net-device.h"
#include "ns3/queue.h"
#include "ns3/data-rate.h"
#include "ns3/data-rate.h"
#include "ns3/event-id.h"
#include "ns3/lwsn-header.h"
#include "mac48-address.h"

namespace ns3 {

class SimpleChannel;
class Node;
class ErrorModel;

/**
 * \ingroup netdevice
 *
 * This device assumes 48-bit mac addressing; there is also the possibility to
 * add an ErrorModel if you want to force losses on the device.
 * 
 * The device can be installed on a node through the SimpleNetDeviceHelper.
 * In case of manual creation, the user is responsible for assigning an unique
 * address to the device.
 *
 * By default the device is in Broadcast mode, with infinite bandwidth.
 *
 * \brief simple net device for simple things and testing
 */
class SimpleNetDevice : public NetDevice
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  SimpleNetDevice ();

  /**
   * Receive a packet from a connected SimpleChannel.  The 
   * SimpleNetDevice receives packets from its connected channel
   * and then forwards them by calling its rx callback method
   *
   * \param packet Packet received on the channel
   * \param protocol protocol number
   * \param to address packet should be sent to
   * \param from address packet was sent from
   */
  void Receive (Ptr<Packet> packet, uint16_t protocol, Mac48Address to, Mac48Address from);
  
  /**
   * Attach a channel to this net device.  This will be the 
   * channel the net device sends on
   * 
   * \param channel channel to assign to this net device
   *
   */
  void SetChannel (Ptr<SimpleChannel> channel);

  /**
   * Attach a queue to the SimpleNetDevice.
   *
   * \param queue Ptr to the new queue.
   */
  void SetQueue (Ptr<Queue> queue);

  /**
   * Get a copy of the attached Queue.
   *
   * \returns Ptr to the queue.
   */
  Ptr<Queue> GetQueue (void) const;

  /**
   * Attach a receive ErrorModel to the SimpleNetDevice.
   *
   * The SimpleNetDevice may optionally include an ErrorModel in
   * the packet receive chain.
   *
   * \see ErrorModel
   * \param em Ptr to the ErrorModel.
   */
  void SetReceiveErrorModel (Ptr<ErrorModel> em);

  // inherited from NetDevice base class.
  virtual void SetIfIndex (const uint32_t index);
  virtual uint32_t GetIfIndex (void) const;
  virtual Ptr<Channel> GetChannel (void) const;
  virtual void SetAddress (Address address);
  virtual Address GetAddress (void) const;
  virtual bool SetMtu (const uint16_t mtu);
  virtual uint16_t GetMtu (void) const;
  virtual bool IsLinkUp (void) const;
  virtual void AddLinkChangeCallback (Callback<void> callback);
  virtual bool IsBroadcast (void) const;
  virtual Address GetBroadcast (void) const;
  virtual bool IsMulticast (void) const;
  virtual Address GetMulticast (Ipv4Address multicastGroup) const;
  virtual bool IsPointToPoint (void) const;
  virtual bool IsBridge (void) const;
  virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
  virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
  virtual Ptr<Node> GetNode (void) const;
  virtual void SetNode (Ptr<Node> node);
  virtual bool NeedsArp (void) const;
  virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);

  virtual Address GetMulticast (Ipv6Address addr) const;

  virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
  virtual bool SupportsSendFrom (void) const;
  
  virtual void SetSid(uint16_t sid);
  virtual uint16_t GetSid();
  virtual void SetSideAddress(Address laddress, Address raddress);
  virtual Mac48Address GetLaddress();
  virtual Mac48Address GetRaddress();
  virtual void AckReceive(bool x, Mac48Address from);
  virtual void AckSend(Ptr <Packet> p, uint16_t protocol, Mac48Address to);
  virtual void WaitSend();
  virtual void Forwarding(Ptr<Packet> p,uint16_t protocol,Mac48Address to);
  virtual int RandTime();
  virtual void WaitAck(Ptr<Packet> p, uint16_t protocol, Mac48Address to, Mac48Address from);
  virtual void ReSend(Ptr<Packet> p, uint16_t protocol, Mac48Address to,Mac48Address from);
  virtual void SetSleep();
  virtual void ChannelSend(Ptr<Packet> p, uint16_t protocol, Mac48Address to,Mac48Address from);
  virtual void AckCheck(Ptr<Packet> p, uint16_t protocol, Mac48Address to, Mac48Address from);
  virtual void OriginalWaitAck(Ptr<Packet> p, uint16_t protocol, Mac48Address from);
  virtual bool OriginalTransmission(Ptr<Packet> p, uint16_t protocolNumber,bool header);
  virtual void OriginalAckCheck(Ptr<Packet> p, uint16_t protocol);
  virtual void ReOriginalSend(Ptr<Packet> p, uint16_t protocol);  
  Ptr<Packet> GetTxPacket(void) const;
  void SetTxPacket(Ptr<Packet> p);
  void ReceiveStart(Ptr<Packet> packet, uint16_t protocol,Mac48Address to, Mac48Address from);
  void ReceiveCheck(Ptr<Packet> packet, uint16_t protocol,Mac48Address to, Mac48Address from);
  void SetRound(int x);
  void SetLastNode(bool x);
  void SetGid(int x);
  int GetGid();
  void Print();
  void QueueCheck(Ptr<Packet> p); 
  int m_count;
  int m_retrans_count;
  int m_collision;
  int g_receive;
protected:
  virtual void DoDispose (void);
private:
  Ptr<SimpleChannel> m_channel; //!< the channel the device is connected to
  NetDevice::ReceiveCallback m_rxCallback; //!< Receive callback
  NetDevice::PromiscReceiveCallback m_promiscCallback; //!< Promiscuous receive callback
  Ptr<Node> m_node; //!< Node this netDevice is associated to
  uint16_t m_mtu;   //!< MTU
  uint32_t m_ifIndex; //!< Interface index
  Mac48Address m_address; //!< MAC address
  Ptr<ErrorModel> m_receiveErrorModel; //!< Receive error model.

  /**
   * The trace source fired when the phy layer drops a packet it has received
   * due to the error model being active.  Although SimpleNetDevice doesn't 
   * really have a Phy model, we choose this trace source name for alignment
   * with other trace sources.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet> > m_phyRxDropTrace;

  /**
   * The TransmitComplete method is used internally to finish the process
   * of sending a packet out on the channel.
   */
  void TransmitComplete (void);

  bool m_linkUp; //!< Flag indicating whether or not the link is up

  /**
   * Flag indicating whether or not the NetDevice is a Point to Point model.
   * Enabling this will disable Broadcast and Arp.
   */
  bool m_pointToPointMode;

  Ptr<Queue> m_queue; //!< The Queue for outgoing packets.
  DataRate m_bps; //!< The device nominal Data rate. Zero means infinite
  EventId TransmitCompleteEvent; //!< the Tx Complete event

  /**
   * List of callbacks to fire if the link changes state (up or down).
   */
  TracedCallback<> m_linkChangeCallbacks;
  Mac48Address m_raddress;
  Mac48Address m_laddress;
  uint16_t m_sid;
  bool rack_flag;
  bool lack_flag;
  bool send_flag;
  uint16_t round;
  uint16_t k;
  Ptr<Packet> m_txPacket;
  int txarray[2];
  Ptr<Queue> temp_queue;
  bool wait_ack;
  bool receive_flag;
  bool receive_flag_1;
  bool last_node;
  int m_gid;
  int ndid;
  bool temp_flag;
  double minTime;
  double maxTime;
};

} // namespace ns3

#endif /* SIMPLE_NET_DEVICE_H */
