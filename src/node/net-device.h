/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
 * All rights reserved.
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
#ifndef NET_DEVICE_H
#define NET_DEVICE_H

#include <string>
#include <stdint.h>
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "mac-address.h"

namespace ns3 {

class Ipv4L4Demux;
class Node;
class TraceResolver;
class TraceContext;

/**
 * \brief Network layer to device interface
 *
 * This interface defines the API which the IP and ARP
 * layers need to access to manage an instance of a network device 
 * layer. It currently does not support MAC-level 
 * multicast but this should not be too hard to add by adding
 * extra methods to register MAC multicast addresses to
 * filter out unwanted packets before handing them to the
 * higher layers.
 *
 * In Linux, this interface is analogous to the interface
 * just above dev_queue_xmit() (i.e., IP packet is fully
 * constructed with destination MAC address already selected).
 * 
 * If you want to write a new MAC layer, you need to subclass
 * this base class and implement your own version of the
 * NetDevice::SendTo method.
 */
class NetDevice {
 public:
  /**
   * \param node base class node pointer of device's node 
   */
  NetDevice(Node* node, const MacAddress& addr);
  virtual ~NetDevice() {}

  TraceResolver *CreateTraceResolver (TraceContext const &context);

  /**
   * \return the current MacAddress of this interface.
   */
  MacAddress GetAddress (void) const;
  /**
   * \param mtu MTU value, in bytes, to set for the device
   * \return whether the MTU value was within legal bounds
   * 
   * Override for default MTU defined on a per-type basis.
   */
  bool SetMtu (const uint16_t mtu);
  /**
   * \return the link-level MTU in bytes for this interface.
   * 
   * This value is typically used by the IP layer to perform
   * IP fragmentation when needed.
   */
  uint16_t GetMtu (void) const;
  /**
   * \param name name of the device (e.g. "eth0")
   */
  void SetName(const std::string name); 
  /**
   * \return name name of the device (e.g. "eth0")
   */
  std::string GetName(void) const;
  /**
   * \return true if link is up; false otherwise
   */
  bool IsLinkUp (void) const;
  /**
   * \param callback the callback to invoke
   *
   * Register a callback invoked whenever the link 
   * status changes to UP. This callback is typically used
   * by the IP/ARP layer to flush the ARP cache 
   * whenever the link goes up.
   */
  void SetLinkChangeCallback (Callback<void> callback);
  /**
   * \return true if this interface supports a broadcast address,
   *         false otherwise.
   */
  bool IsBroadcast (void) const;
  /**
   * \return the broadcast address supported by
   *         this netdevice.
   *
   * Calling this method is invalid if IsBroadcast returns
   * not true.
   */
  MacAddress const &GetBroadcast (void) const;
  /**
   * \return value of m_isMulticast flag
   */
  bool IsMulticast (void) const;
  /**
   * \return value of m_isPointToPoint flag
   */
  bool IsPointToPoint (void) const;
  /**
   * \param p packet sent from above down to Network Device
   * \param dest mac address of the destination (already resolved)
   * \param protocolNumber identifies the type of payload contained in
   *        this packet. Used to call the right L3Protocol when the packet
   *        is received.
   * 
   *  Called from higher layer to send packet into Network Device
   *  to the specified destination MacAddress
   * 
   * \return whether the Send operation succeeded 
   */
  bool Send(Packet& p, const MacAddress& dest, uint16_t protocolNumber);

 protected:
  /**
   * Enable broadcast support. This method should be
   * called by subclasses from their constructor
   */
  void EnableBroadcast (MacAddress broadcast);
  /**
   * Set m_isBroadcast flag to false
   */
  void DisableBroadcast (void);
  /**
   * Set m_isMulticast flag to true
   */
  void EnableMulticast (void);
  /**
   * Set m_isMulticast flag to false
   */
  void DisableMulticast (void);
  /**
   * Set m_isPointToPoint flag to true
   */
  void EnablePointToPoint (void);
  /**
   * Set m_isPointToPoint flag to false
   */
  void DisablePointToPoint (void);
  /**
   * When a subclass notices that the link status has changed to up,
   * it notifies its parent class by calling this method. This method
   * is responsible for notifying any LinkUp callbacks. 
   */
  void NotifyLinkUp (void);
  /**
   * When a subclass notices that the link status has changed to 
   * down, it notifies its parent class by calling this method.
   */
  void NotifyLinkDown (void);
  /**
   * \returns the node base class which contains this network
   *          interface.
   *
   * When a subclass needs to get access to the underlying node
   * base class to print the nodeid for example, it can invoke
   * this method.
   */
  Node* GetNode (void) const;

  /**
   * \param p packet sent from below up to Network Device
   * \param from source mac address of the sender 
   * \returns true if the packet was forwarded successfully,
   *          false otherwise.
   *
   * When a subclass gets a packet from the channel, it 
   * forwards it to the higher layers by calling this method
   * which is responsible for passing it up to the Rx callback.
   */
  bool ForwardUp (Packet& p);

 private:
  /**
   * \param p packet to send
   * \param dest address of destination to which packet must be sent
   * \returns true if the packet could be sent successfully, false
   *          otherwise.
   *
   * This is the private virtual target function of the public Send()
   * method.  When the link is Up, this method is invoked to ask 
   * subclasses to forward packets. Subclasses MUST override this method.
   */
  virtual bool SendTo (Packet& p, const MacAddress& dest) = 0;

  virtual TraceResolver *DoCreateTraceResolver (TraceContext const &context) = 0;
  Node*         m_node;
  std::string   m_name;
  uint16_t      m_ifIndex;
  MacAddress    m_address;
  MacAddress    m_broadcast;
  uint16_t      m_mtu;
  bool          m_isUp;
  bool          m_isBroadcast;
  bool          m_isMulticast;
  bool          m_isPointToPoint;
  Callback<void> m_linkChangeCallback;
};

}; // namespace ns3
#endif
