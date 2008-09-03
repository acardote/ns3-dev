/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005 INRIA
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

#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/boolean.h"

#include "udp-l4-protocol.h"
#include "udp-header.h"
#include "ipv4-end-point-demux.h"
#include "ipv4-end-point.h"
#include "ipv4-l3-protocol.h"
#include "udp-socket-impl.h"

NS_LOG_COMPONENT_DEFINE ("UdpL4Protocol");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (UdpL4Protocol);

/* see http://www.iana.org/assignments/protocol-numbers */
const uint8_t UdpL4Protocol::PROT_NUMBER = 17;

TypeId 
UdpL4Protocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UdpL4Protocol")
    .SetParent<Ipv4L4Protocol> ()
    .AddConstructor<UdpL4Protocol> ()
    .AddAttribute ("CalcChecksum", "If true, we calculate the checksum of outgoing packets"
                   " and verify the checksum of incoming packets.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&UdpL4Protocol::m_calcChecksum),
                   MakeBooleanChecker ())
    ;
  return tid;
}

UdpL4Protocol::UdpL4Protocol ()
  : m_endPoints (new Ipv4EndPointDemux ())
{
  NS_LOG_FUNCTION_NOARGS ();
}

UdpL4Protocol::~UdpL4Protocol ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void 
UdpL4Protocol::SetNode (Ptr<Node> node)
{
  m_node = node;
}

int 
UdpL4Protocol::GetProtocolNumber (void) const
{
  return PROT_NUMBER;
}


void
UdpL4Protocol::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (m_endPoints != 0)
    {
      delete m_endPoints;
      m_endPoints = 0;
    }
  m_node = 0;
  Ipv4L4Protocol::DoDispose ();
}

Ptr<Socket>
UdpL4Protocol::CreateSocket (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<UdpSocketImpl> socket = CreateObject<UdpSocketImpl> ();
  socket->SetNode (m_node);
  socket->SetUdp (this);
  return socket;
}

Ipv4EndPoint *
UdpL4Protocol::Allocate (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_endPoints->Allocate ();
}

Ipv4EndPoint *
UdpL4Protocol::Allocate (Ipv4Address address)
{
  NS_LOG_FUNCTION (this << address);
  return m_endPoints->Allocate (address);
}

Ipv4EndPoint *
UdpL4Protocol::Allocate (uint16_t port)
{
  NS_LOG_FUNCTION (this << port);
  return m_endPoints->Allocate (port);
}

Ipv4EndPoint *
UdpL4Protocol::Allocate (Ipv4Address address, uint16_t port)
{
  NS_LOG_FUNCTION (this << address << port);
  return m_endPoints->Allocate (address, port);
}
Ipv4EndPoint *
UdpL4Protocol::Allocate (Ipv4Address localAddress, uint16_t localPort,
                         Ipv4Address peerAddress, uint16_t peerPort)
{
  NS_LOG_FUNCTION (this << localAddress << localPort << peerAddress << peerPort);
  return m_endPoints->Allocate (localAddress, localPort,
                                peerAddress, peerPort);
}

void 
UdpL4Protocol::DeAllocate (Ipv4EndPoint *endPoint)
{
  NS_LOG_FUNCTION (this << endPoint);
  m_endPoints->DeAllocate (endPoint);
}

void 
UdpL4Protocol::Receive(Ptr<Packet> packet, 
                       Ipv4Address const &source,
                       Ipv4Address const &destination,
                       Ptr<Ipv4Interface> interface)
{
  NS_LOG_FUNCTION (this << packet << source << destination);
  UdpHeader udpHeader;
  if(m_calcChecksum)
  {
    udpHeader.EnableChecksums();
  }

  udpHeader.InitializeChecksum (source, destination, PROT_NUMBER);

  packet->RemoveHeader (udpHeader);

  if(!udpHeader.IsChecksumOk ())
  {
    NS_LOG_INFO("Bad checksum : dropping packet!");
    return;
  }

  Ipv4EndPointDemux::EndPoints endPoints =
    m_endPoints->Lookup (destination, udpHeader.GetDestinationPort (),
                         source, udpHeader.GetSourcePort (), interface);
  for (Ipv4EndPointDemux::EndPointsI endPoint = endPoints.begin ();
       endPoint != endPoints.end (); endPoint++)
    {
      (*endPoint)->ForwardUp (packet->Copy (), source, udpHeader.GetSourcePort ());
    }
}

void
UdpL4Protocol::Send (Ptr<Packet> packet, 
                     Ipv4Address saddr, Ipv4Address daddr, 
                     uint16_t sport, uint16_t dport)
{
  NS_LOG_FUNCTION (this << packet << saddr << daddr << sport << dport);

  UdpHeader udpHeader;
  if(m_calcChecksum)
  {
    udpHeader.EnableChecksums();
    udpHeader.InitializeChecksum (saddr,
                                  daddr,
                                  PROT_NUMBER);
  }
  udpHeader.SetDestinationPort (dport);
  udpHeader.SetSourcePort (sport);

  packet->AddHeader (udpHeader);

  Ptr<Ipv4L3Protocol> ipv4 = m_node->GetObject<Ipv4L3Protocol> ();
  if (ipv4 != 0)
    {
      NS_LOG_LOGIC ("Sending to IP");
      ipv4->Send (packet, saddr, daddr, PROT_NUMBER);
    }
}


}; // namespace ns3

