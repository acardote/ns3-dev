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

#include <algorithm>
#include "ns3/assert.h"
#include "ns3/debug.h"
#include "ns3/fatal-error.h"
#include "ns3/nstime.h"
#include "ns3/internet-node.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4.h"
#include "ns3/queue.h"

#include "csma-cd-channel.h"
#include "csma-cd-net-device.h"
#include "csma-cd-ipv4-topology.h"

namespace ns3 {


void
CsmaCdIpv4Topology::AddIpv4CsmaCdNode(Ptr<Node> n1,
                                      Ptr<CsmaCdChannel> ch,
                                      MacAddress addr)
{
  Ptr<Queue> q = Queue::CreateDefault ();

  // Use the first net device in the node to transmit
  Ptr<CsmaCdNetDevice> nd0 = Create<CsmaCdNetDevice> (n1, addr, 
                                                      ns3::CsmaCdNetDevice::IP_ARP,
                                                      true, false);
  nd0->AddQueue(q);
  nd0->Attach (ch);

  // Use the second net device in the node to transmit
  Ptr<CsmaCdNetDevice> nd1 = Create<CsmaCdNetDevice> (n1, addr, 
                                                      ns3::CsmaCdNetDevice::IP_ARP,
                                                      false, true);
  nd1->AddQueue(q);
  nd1->Attach (ch);
}


void
CsmaCdIpv4Topology::AddIpv4LlcCsmaCdNode(Ptr<Node> n1,
                                         Ptr<CsmaCdChannel> ch,
                                         MacAddress addr)
{
  Ptr<Queue> q = Queue::CreateDefault ();

  Ptr<CsmaCdNetDevice> nd0 = Create<CsmaCdNetDevice> (n1, addr,
                                                      ns3::CsmaCdNetDevice::LLC,
                                                      true, false);
  nd0->AddQueue(q);
  nd0->Attach (ch);

  Ptr<CsmaCdNetDevice> nd1 = Create<CsmaCdNetDevice> (n1, addr,
                                                      ns3::CsmaCdNetDevice::LLC,
                                                      false, true);
  nd1->AddQueue(q);
  nd1->Attach (ch);
}

void
CsmaCdIpv4Topology::AddIpv4RawCsmaCdNode(Ptr<Node> n1,
                                         Ptr<CsmaCdChannel> ch,
                                         MacAddress addr)
{
  Ptr<Queue> q = Queue::CreateDefault ();

  Ptr<CsmaCdNetDevice> nd0 = Create<CsmaCdNetDevice> (n1, addr,
                                                      ns3::CsmaCdNetDevice::RAW,
                                                      true, false);
  nd0->AddQueue(q);
  nd0->Attach (ch);

  Ptr<CsmaCdNetDevice> nd1 = Create<CsmaCdNetDevice> (n1, addr,
                                                      ns3::CsmaCdNetDevice::RAW,
                                                      false, true);
  nd1->AddQueue(q);
  nd1->Attach (ch);
}

void
CsmaCdIpv4Topology::AddIpv4Address(Ptr<Node> n1,
                                       int ndNum,
                                       const Ipv4Address& addr1,
                                       const Ipv4Mask& netmask1)
{

  // Duplex link is assumed to be subnetted as a /30
  // May run this unnumbered in the future?
  Ipv4Mask netmask(netmask1);

  Ptr<NetDevice> nd1 = n1->GetDevice(ndNum);

  Ptr<Ipv4> ip1 = n1->QueryInterface<Ipv4> (Ipv4::iid);
  uint32_t index1 = ip1->AddInterface (nd1);

  ip1->SetAddress (index1, addr1);
  ip1->SetNetworkMask (index1, netmask);
  ip1->SetUp (index1);

}

void
CsmaCdIpv4Topology::AddIpv4Routes (
  Ptr<NetDevice> nd1, Ptr<NetDevice> nd2)
{ 
  // Assert that both are Ipv4 nodes
  Ptr<Ipv4> ip1 = nd1->GetNode ()->QueryInterface<Ipv4> (Ipv4::iid);
  Ptr<Ipv4> ip2 = nd2->GetNode ()->QueryInterface<Ipv4> (Ipv4::iid);
  NS_ASSERT(ip1 != 0 && ip2 != 0);

  // Get interface indexes for both nodes corresponding to the right channel
  uint32_t index1 = 0;
  bool found = false;
  for (uint32_t i = 0; i < ip1->GetNInterfaces (); i++)
    {
      if (ip1 ->GetNetDevice (i) == nd1)
        {
          index1 = i;
          found = true;
        }
    }
  NS_ASSERT(found);

  uint32_t index2 = 0;
  found = false;
  for (uint32_t i = 0; i < ip2->GetNInterfaces (); i++)
    {
      if (ip2 ->GetNetDevice (i) == nd2)
        {
          index2 = i;
          found = true;
        }
    }
  NS_ASSERT(found);

  ip1->AddHostRouteTo (ip2-> GetAddress (index2), index1);
  ip2->AddHostRouteTo (ip1-> GetAddress (index1), index2); 
}

} // namespace ns3
 
