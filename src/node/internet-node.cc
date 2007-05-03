// -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*-
//
// Copyright (c) 2006 Georgia Tech Research Corporation
// All rights reserved.
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
// Implementation of the InternetNode class for ns3.
// George F. Riley, Georgia Tech, Fall 2006

#include "ns3/composite-trace-resolver.h"

#include "application-list.h"
#include "l3-demux.h"
#include "ipv4-l4-demux.h"
#include "internet-node.h"
#include "udp.h"
#include "ipv4.h"
#include "arp.h"
#include "net-device.h"

namespace ns3 {


InternetNode::InternetNode()
{
  // Instantiate the capabilities
  ApplicationList *applicationList = new ApplicationList(this);
  L3Demux *l3Demux = new L3Demux(this);
  Ipv4L4Demux *ipv4L4Demux = new Ipv4L4Demux(this);

  NsUnknown::AddInterface (applicationList);
  NsUnknown::AddInterface (l3Demux);
  NsUnknown::AddInterface (ipv4L4Demux);

  Ipv4 *ipv4 = new Ipv4 (this);
  Arp *arp = new Arp (this);
  Udp *udp = new Udp (this);

  l3Demux->Insert (ipv4);
  l3Demux->Insert (arp);
  ipv4L4Demux->Insert (udp);

  applicationList->Unref ();
  l3Demux->Unref ();
  ipv4L4Demux->Unref ();
  ipv4->Unref ();
  arp->Unref ();
  udp->Unref ();
}

InternetNode::~InternetNode ()
{}

void
InternetNode::SetName (std::string name)
{
  m_name = name;
}

TraceResolver *
InternetNode::CreateTraceResolver (TraceContext const &context)
{
  CompositeTraceResolver *resolver = new CompositeTraceResolver (context);
  Ipv4 *ipv4 = GetIpv4 ();
  resolver->Add ("ipv4",
                 MakeCallback (&Ipv4::CreateTraceResolver, ipv4),
                 InternetNode::IPV4);
  ipv4->Unref ();

  Arp *arp = GetArp ();
  resolver->Add ("arp",
                 MakeCallback (&Arp::CreateTraceResolver, arp),
                 InternetNode::ARP);
  arp->Unref ();

  Udp *udp = GetUdp ();
  resolver->Add ("udp",
                 MakeCallback (&Udp::CreateTraceResolver, udp),
                 InternetNode::UDP);
  udp->Unref ();

  return resolver;
}

void 
InternetNode::DoDispose()
{
  Node::DoDispose ();
}

Ipv4 *
InternetNode::GetIpv4 (void) const
{
  L3Demux *l3Demux = QueryInterface<L3Demux> (L3Demux::iid);
  Ipv4 *ipv4 = static_cast<Ipv4*> (l3Demux->PeekProtocol (Ipv4::PROT_NUMBER));
  l3Demux->Unref ();
  ipv4->Ref ();
  return ipv4;
}
Udp *
InternetNode::GetUdp (void) const
{
  Ipv4L4Demux *demux = QueryInterface<Ipv4L4Demux> (Ipv4L4Demux::iid);
  Udp *udp = static_cast<Udp*> (demux->PeekProtocol (Udp::PROT_NUMBER));
  demux->Unref ();
  udp->Ref ();
  return udp;
}

Arp *
InternetNode::GetArp (void) const
{
  L3Demux *l3Demux = QueryInterface<L3Demux> (L3Demux::iid);
  Arp *arp = static_cast<Arp*> (l3Demux->PeekProtocol (Arp::PROT_NUMBER));
  l3Demux->Unref ();
  arp->Ref ();
  return arp;
}

void 
InternetNode::DoAddDevice (NetDevice *device) const
{
  device->SetReceiveCallback (MakeCallback (&InternetNode::ReceiveFromDevice, this));
}

bool
InternetNode::ReceiveFromDevice (NetDevice *device, const Packet &p, uint16_t protocolNumber) const
{
  L3Demux *demux = QueryInterface<L3Demux> (L3Demux::iid);
  L3Protocol *target = demux->PeekProtocol (protocolNumber);
  demux->Unref ();
  if (target != 0) 
    {
      Packet packet = p;
      target->Receive(packet, device);
      return true;
    }
  return false;
}


}//namespace ns3
