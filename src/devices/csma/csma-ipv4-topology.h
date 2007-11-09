/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//
// Copyright (c) 2007 Emmanuelle Laprise
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: Emmanuelle Laprise <emmanuelle.laprise@bluekazoo.ca>
//

#ifndef __CSMA_IPV4_TOPOLOGY_H__
#define __CSMA_IPV4_TOPOLOGY_H__

#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-route.h"
#include "ns3/internet-node.h"
#include "ns3/csma-net-device.h"

// The topology class consists of only static methods thar are used to
// create the topology and data flows for an ns3 simulation

namespace ns3 {

class CsmaIpv4Channel;
class Node;
class IPAddr;
class DataRate;
class Queue;

/**
 * \brief A helper class to create Topologies based on the
 * InternetNodes and CsmaChannels. Either the
 * SimpleCsmaNetDevice or the LLCCsmaNetDevice can be used
 * when constructing these topologies.
 */
class CsmaIpv4Topology {
public:

  /**
   * \param node Node to be attached to the Csma channel
   * \param channel CsmaChannel to which node n1 should be attached
   * \param addr Mac address of the node
   *
   * Add a Csma node to a Csma channel. This function adds
   * a EthernetCsmaNetDevice to the nodes so that they can
   * connect to a CsmaChannel. This means that Ethernet headers
   * and trailers will be added to the packet before sending out on
   * the net device.
   * 
   * \return ifIndex of the device
   */
  static uint32_t AddIpv4CsmaNetDevice(Ptr<Node> node,
                                       Ptr<CsmaChannel> channel,
                                       Mac48Address addr);

  /**
   * \param n1 Node to be attached to the Csma channel
   * \param ch CsmaChannel to which node n1 should be attached
   * \param addr Mac address of the node
   *
   * Add a Csma node to a Csma channel. This function adds
   * a RawCsmaNetDevice to the nodes so that they can connect
   * to a CsmaChannel.
   */
  static void AddIpv4RawCsmaNode( Ptr<Node> n1,
                                    Ptr<CsmaChannel> ch,
                                    Mac48Address addr);

  /**
   * \param n1 Node to be attached to the Csma channel
   * \param ch CsmaChannel to which node n1 should be attached
   * \param addr Mac address of the node
   *
   * Add a Csma node to a Csma channel. This function adds
   * a LlcCsmaNetDevice to the nodes so that they can connect
   * to a CsmaChannel.
   */
  static void AddIpv4LlcCsmaNode( Ptr<Node> n1,
                                    Ptr<CsmaChannel> ch,
                                    Mac48Address addr);



  /** 
   * \brief Create an Ipv4 interface for a net device and assign an 
   * Ipv4Address to that interface.
   *
   * \param node The node to which to add the new address and corresponding 
   *        interface.
   * \param netDeviceNumber The NetDevice index number with which to associate
   *        the address.
   * \param address The Ipv4 Address for the interface.
   * \param mask The network mask for the interface
   * \param metric (optional) metric (cost) to assign for routing calculations
   * 
   * Add an Ipv4Address to the Ipv4 interface associated with the
   * ndNum CsmaIpv4NetDevices on the provided CsmaIpv4Channel
   */
  static uint32_t AddIpv4Address(Ptr<Node> node,
                                 uint32_t netDeviceNumber, 
                                 const Ipv4Address address,
                                 const Ipv4Mask mask,
                                 uint16_t metric = 1);

  /**
   * \param nd1 Node
   * \param nd2 Node
   * 
   * Add an IPV4 host route between the two specified net devices
   */
  static void AddIpv4Routes (Ptr<NetDevice> nd1, Ptr<NetDevice> nd2);
};

} // namespace ns3

#endif

