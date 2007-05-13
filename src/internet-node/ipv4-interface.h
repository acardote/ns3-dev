/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006,2007 INRIA
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
 * Authors: 
 *  Mathieu Lacage <mathieu.lacage@sophia.inria.fr>,
 *  Tom Henderson <tomh@tomh.org>
 */
#ifndef IPV4_INTERFACE_H
#define IPV4_INTERFACE_H

#include <list>
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"

namespace ns3 {

class NetDevice;
class Packet;
class TraceResolver;
class TraceContext;

/**
 * \brief The IPv4 representation of a network interface
 *
 * This class roughly corresponds to the struct in_device
 * of Linux; the main purpose is to provide address-family
 * specific information (addresses) about an interface.
 *
 * This class defines two APIs:
 *  - the public API which is expected to be used by both 
 *    the IPv4 layer and the user during forwarding and 
 *    configuration.
 *  - the private API which is expected to be implemented
 *    by subclasses of this base class. One such subclass 
 *    will be a Loopback interface which loops every
 *    packet sent back to the ipv4 layer. Another such 
 *    subclass typically contains the Ipv4 <-> MAC address
 *    translation logic which will use most of the time the
 *    ARP/RARP protocols.
 *
 * By default, Ipv4 interface are created in the "down" state
 * with ip address 192.168.0.1 and a matching mask. Before
 * becoming useable, the user must invoke SetUp on them
 * once the final Ipv4 address and mask has been set.
 *
 * Subclasses must implement the two methods:
 *   - Ipv4Interface::SendTo
 *   - Ipv4Interface::DoCreateTraceResolver
 */
class Ipv4Interface 
{
public:
  /**
   * \param nd the NetDevice associated to this Ipv4Interface.
   *           This value can be zero in which case the MTU
   *           of this interface will be 2^(16-1).
   */
  Ipv4Interface (Ptr<NetDevice> nd);
  virtual ~Ipv4Interface();

  /**
   * \param context the trace context to use to construct the
   *        TraceResolver to return
   * \returns a TraceResolver which can resolve all traces
   *          performed in this object. The caller must
   *          delete the returned object.
   *
   * This method will delegate the work to the private DoCreateTraceResolver 
   * method which is supposed to be implemented by subclasses.
   */
  TraceResolver *CreateTraceResolver (TraceContext const &context);
  /**
   * \returns the underlying NetDevice. This method can return
   *          zero if this interface has no associated NetDevice.
   */
  Ptr<NetDevice> GetDevice (void) const;

  /**
   * \param a set the ipv4 address of this interface.
   */
  void SetAddress (Ipv4Address a);
  /**
   * \param mask set the ipv4 netmask of this interface.
   */
  void SetNetworkMask (Ipv4Mask mask);

  /**
   * \returns the broadcast ipv4 address associated to this interface
   */
  Ipv4Address GetBroadcast (void) const;
  /**
   * \returns the ipv4 netmask of this interface
   */
  Ipv4Mask GetNetworkMask (void) const;
  /**
   * \returns the ipv4 address of this interface
   */
  Ipv4Address GetAddress (void) const;

  /**
   * This function a pass-through to NetDevice GetMtu, modulo
   * the  LLC/SNAP header i.e., ipv4MTU = NetDeviceMtu - LLCSNAPSIZE
   * \returns the Maximum Transmission Unit associated to this interface.
   */
  uint16_t GetMtu (void) const;

  /**
   * These are IP interface states and may be distinct from 
   * NetDevice states, such as found in real implementations
   * (where the device may be down but IP interface state is still up).
   */
  /**
   * \returns true if this interface is enabled, false otherwise.
   */
  bool IsUp (void) const;
  /**
   * \returns true if this interface is disabled, false otherwise.
   */
  bool IsDown (void) const;
  /**
   * Enable this interface
   */
  void SetUp (void);
  /**
   * Disable this interface
   */
  void SetDown (void);

  /**
   * \param p packet to send
   * \param dest next hop address of packet.
   *
   * This method will eventually call the private
   * SendTo method which must be implemented by subclasses.
   */ 
  void Send(Packet p, Ipv4Address dest);


 private:
  virtual void SendTo (Packet p, Ipv4Address dest) = 0;
  virtual TraceResolver *DoCreateTraceResolver (TraceContext const &context) = 0;
  Ptr<NetDevice> m_netdevice;
  bool m_ifup;
  Ipv4Address m_address;
  Ipv4Mask m_netmask;
};

}; // namespace ns3

#endif
