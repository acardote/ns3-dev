/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006,2007 INRIA
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
#include "ns3/ipv4-interface-address.h"
#include "ns3/ptr.h"
#include "ns3/object.h"

namespace ns3 {

class NetDevice;
class Packet;

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
 *   - Ipv4Interface::GetDevice
 */
class Ipv4Interface  : public Object
{
public:
  static TypeId GetTypeId (void);

  Ipv4Interface ();
  virtual ~Ipv4Interface();

  /**
   * \returns the underlying NetDevice. This method can return
   *          zero if this interface has no associated NetDevice.
   */
  virtual Ptr<NetDevice> GetDevice (void) const = 0;

  /**
   * \param metric configured routing metric (cost) of this interface
   */
  void SetMetric (uint16_t metric);
  /**
   * \returns configured routing metric (cost) of this interface
   */
  uint16_t GetMetric (void) const;

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
  void Send(Ptr<Packet> p, Ipv4Address dest);

  /**
   * \param address The Ipv4InterfaceAddress to add to the interface
   * \returns The index of the newly-added Ipv4InterfaceAddress
   */
  uint32_t AddAddress (Ipv4InterfaceAddress address);

  /**
   * \param i Index of Ipv4InterfaceAddress to return
   * \returns The Ipv4InterfaceAddress address whose index is i
   */
  Ipv4InterfaceAddress GetAddress (uint32_t index) const;

  /**
   * \returns the number of Ipv4InterfaceAddresss stored on this interface
   */
  uint32_t GetNAddresses (void) const;

  /**
   * \param i index of Ipv4InterfaceAddress to remove from address list.
   */
  void RemoveAddress (uint32_t index);

protected:
  virtual void DoDispose (void);
private:
  virtual void SendTo (Ptr<Packet> p, Ipv4Address dest) = 0;
  bool m_ifup;
  uint16_t m_metric;

  typedef std::list<Ipv4InterfaceAddress> Ipv4InterfaceAddressList;
  typedef std::list<Ipv4InterfaceAddress>::const_iterator Ipv4InterfaceAddressListCI;
  typedef std::list<Ipv4InterfaceAddress>::iterator Ipv4InterfaceAddressListI;
  Ipv4InterfaceAddressList m_ifaddrs;
};

}; // namespace ns3

#endif
