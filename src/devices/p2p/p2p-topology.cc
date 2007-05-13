/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
//
// Copyright (c) 2006 Georgia Tech Research Corporation
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
// Author: George F. Riley<riley@ece.gatech.edu>
//

//
// Topology helper for ns3.
// George F. Riley, Georgia Tech, Spring 2007

#include <algorithm>
#include "ns3/assert.h"
#include "ns3/nstime.h"
#include "ns3/internet-node.h"
#include "ns3/ipv4-address.h"
#include "ns3/i-ipv4.h"
#include "ns3/queue.h"

#include "p2p-channel.h"
#include "p2p-net-device.h"
#include "p2p-topology.h"

namespace ns3 {

Ptr<PointToPointChannel>
PointToPointTopology::AddPointToPointLink(
  Ptr<Node> n1,
  Ptr<Node> n2,
  const DataRate& bps,
  const Time& delay)
{
  Ptr<PointToPointChannel> channel = MakeNewObject<PointToPointChannel> (bps, delay);

  Ptr<PointToPointNetDevice> net1 = MakeNewObject<PointToPointNetDevice> (n1);

  Ptr<Queue> q = Queue::CreateDefault ();
  net1->AddQueue(q);
  n1->AddDevice (net1);
  net1->Attach (channel);
  
  Ptr<PointToPointNetDevice> net2 = MakeNewObject<PointToPointNetDevice> (n2);

  q = Queue::CreateDefault ();
  net2->AddQueue(q);
  n2->AddDevice (net2);
  net2->Attach (channel);

  return channel;
}

bool
PointToPointTopology::AddIpv4Addresses(
  Ptr<const PointToPointChannel> chan,
  Ptr<Node> n1, const Ipv4Address& addr1,
  Ptr<Node> n2, const Ipv4Address& addr2)
{

  // Duplex link is assumed to be subnetted as a /30
  // May run this unnumbered in the future?
  Ipv4Mask netmask("255.255.255.252");
  NS_ASSERT (netmask.IsMatch(addr1,addr2));

  // The PointToPoint channel is used to find the relevant NetDevices
  NS_ASSERT (chan->GetNDevices () == 2);
  Ptr<NetDevice> nd1 = chan->GetDevice (0);
  Ptr<NetDevice> nd2 = chan->GetDevice (1);
  // Make sure that nd1 belongs to n1 and nd2 to n2
  if ( (nd1->GetNode ()->GetId () == n2->GetId () ) && 
       (nd2->GetNode ()->GetId () == n1->GetId () ) )
    {
      std::swap(nd1, nd2);
    }
  NS_ASSERT (nd1->GetNode ()->GetId () == n1->GetId ());
  NS_ASSERT (nd2->GetNode ()->GetId () == n2->GetId ());
  
  Ptr<IIpv4> ip1 = n1->QueryInterface<IIpv4> (IIpv4::iid);
  uint32_t index1 = ip1->AddInterface (nd1);

  ip1->SetAddress (index1, addr1);
  ip1->SetNetworkMask (index1, netmask);
  ip1->SetUp (index1);

  Ptr<IIpv4> ip2 = n2->QueryInterface<IIpv4> (IIpv4::iid);
  uint32_t index2 = ip2->AddInterface (nd2);

  ip2->SetAddress (index2, addr2);
  ip2->SetNetworkMask (index2, netmask);
  ip2->SetUp (index2);
  
  ip1->AddHostRouteTo (addr2, index1);
  ip2->AddHostRouteTo (addr1, index2);
    
  return true;
}

#ifdef NOTYET

// Get the net device connecting node n1 to n2.  For topologies where
// there are possibly multiple devices connecting n1 and n2 (for example
// wireless with two devices on different channels) this will return
// the first one found.
Ptr<PointToPointNetDevice> PointToPointTopology::GetNetDevice(Ptr<Node> n1, Ptr<Node> n2)
{
  for (NetDeviceList::NetDevices_t::const_iterator i = dlist.Begin();
       i != dlist.End(); ++i)
    { // Check each device
      Ptr<NetDevice> nd = *i; // next device
      Ptr<Channel> c = nd->GetChannel();
      if (!c) continue; // No channel
      if (c->NodeIsPeer(n2)) return nd; // found it
    }
  return 0; // None found
}
  
// Get the channel connecting node n1 to node n2
Ptr<PointToPointChannel> PointToPointTopology::GetChannel(
  Ptr<Node> n1, 
  Ptr<Node> n2
)
{
  Ptr<NetDevice> nd = GetNetDevice(n1, n2);
  if (!nd) return 0; // No net device, so no channel
  return nd->GetChannel();
}

Ptr<Queue> PointToPointTopology::GetQueue(Ptr<Node> n1, Ptr<Node> n2)
{
  Ptr<NetDevice> nd = GetNetDevice(n1, n2);
  if (!nd) return 0; // No net device, so in queue
  return nd->GetQueue();
}

void PointToPointTopology::SetQueue(Ptr<Node> n1, Ptr<Node> n2, Ptr<Queue> q)
{
  Ptr<NetDevice> nd = GetNetDevice(n1, n2);
  if (!nd) return 0; // No net device, can't set queue
  // Add the specified queue to the netdevice
  return nd->SetQueue(q);
}

#endif

#ifdef GFR
P2PChannel Topology::AddDuplexLink(Ptr<Node> n1, const IPAddr& ip1, 
                                    Ptr<Node> n2, const IPAddr& ip2,
                                    const Rate& rate, const Time& delay)
{
  // Get the net devices
  P2PNetDevice* nd1 = ndl1->Add(P2PNetDevice(n1, rate, nil));
  P2PNetDevice* nd2 = ndl2->Add(P2PNetDevice(n1, rate, nd1->GetChannel()));
  // Not implemented yet.  Add appropriate layer 2 protocol for
  // the net devices.
  // Get the L3 proto for node 1 and configure it with this device
  Ptr<L3Demux>    l3demux1 = n1->GetL3Demux();
  Ptr<L3Protocol> l3proto1 = nil;
  // If the node 1 l3 demux exists, find the coresponding l3 protocol
  if (l3demux1) l3proto1 = l3demux1->Lookup(ip1.L3Proto());
  // If the l3 protocol exists, configure this net device.  Use a mask
  // of all ones, since there is only one device on the remote end
  // of this link
  if (l3proto1) l3proto1->AddNetDevice(nd1, ip1, ip1.GetMask(ip1.Size()*8));
  // Same for node 2
  Ptr<L3Demux>    l3demux2 = n2->GetL3Demux();
  Ptr<L3Protocol> l3proto2 = nil;
  // If the node 2 l3 demux exists, find the coresponding l3 protocol
  if (l3demux2) l3proto2 = l3demux2->Lookup(ip2.L3Proto());
  if (l3proto2) l3proto2->AddNetDevice(nd2, ip2, ip2.GetMask(ip2.Size()*8));
  return dynamic_cast<P2PChannel*>(nd1->GetChannel());  // Always succeeds
}

// Get the channel connecting node n1 to node n2
Ptr<Channel> Topology::GetChannel(Ptr<Node> n1, Ptr<Node> n2)
{
  Ptr<NetDevice> nd = GetNetDevice(n1, n2);
  if (!nd) return 0; // No net device, so no channel
  return nd->GetChannel();
}

Ptr<Queue> Topology::GetQueue(Ptr<Node> n1, Ptr<Node> n2)
{
  Ptr<NetDevice> nd = GetNetDevice(n1, n2);
  if (!nd) return 0; // No net device, so in queue
  return nd->GetQueue();
}

void Topology::SetQueue(Ptr<Node> n1, Ptr<Node> n2, Ptr<Queue> q)
{
  Ptr<NetDevice> nd = GetNetDevice(n1, n2);
  if (!nd) return 0; // No net device, can't set queue
  // Add the specified queue to the netdevice
  return nd->SetQueue(q);
}

#endif

} // namespace ns3
 
