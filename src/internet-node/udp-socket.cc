/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INRIA
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

#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-route.h"
#include "ns3/ipv4.h"
#include "udp-socket.h"
#include "udp-l4-protocol.h"
#include "ipv4-end-point.h"
#include "ipv4-l4-demux.h"
#include "ns3/ipv4.h"

NS_LOG_COMPONENT_DEFINE ("UdpSocket");

namespace ns3 {

UdpSocket::UdpSocket (Ptr<Node> node, Ptr<UdpL4Protocol> udp)
  : m_endPoint (0),
    m_node (node),
    m_udp (udp),
    m_errno (ERROR_NOTERROR),
    m_shutdownSend (false),
    m_shutdownRecv (false),
    m_connected (false)
{
  NS_LOG_FUNCTION;
}

UdpSocket::~UdpSocket ()
{
  NS_LOG_FUNCTION;

  m_node = 0;
  if (m_endPoint != 0)
    {
      NS_ASSERT (m_udp != 0);
      /**
       * Note that this piece of code is a bit tricky:
       * when DeAllocate is called, it will call into
       * Ipv4EndPointDemux::Deallocate which triggers
       * a delete of the associated endPoint which triggers
       * in turn a call to the method ::Destroy below
       * will will zero the m_endPoint field.
       */
      NS_ASSERT (m_endPoint != 0);
      m_udp->DeAllocate (m_endPoint);
      NS_ASSERT (m_endPoint == 0);
    }
  m_udp = 0;
}

enum Socket::SocketErrno
UdpSocket::GetErrno (void) const
{
  NS_LOG_FUNCTION;
  return m_errno;
}

Ptr<Node>
UdpSocket::GetNode (void) const
{
  NS_LOG_FUNCTION;
  return m_node;
}

void 
UdpSocket::Destroy (void)
{
  NS_LOG_FUNCTION;
  m_node = 0;
  m_endPoint = 0;
  m_udp = 0;
}

int
UdpSocket::FinishBind (void)
{
  NS_LOG_FUNCTION;
  if (m_endPoint == 0)
    {
      return -1;
    }
  m_endPoint->SetRxCallback (MakeCallback (&UdpSocket::ForwardUp, this));
  m_endPoint->SetDestroyCallback (MakeCallback (&UdpSocket::Destroy, this));
  return 0;
}

int
UdpSocket::Bind (void)
{
  NS_LOG_FUNCTION;
  m_endPoint = m_udp->Allocate ();
  return FinishBind ();
}

int 
UdpSocket::Bind (const Address &address)
{
  NS_LOG_FUNCTION;
  NS_LOG_PARAM("(" << address << ")");

  if (!InetSocketAddress::IsMatchingType (address))
    {
      NS_LOG_ERROR ("Not IsMatchingType");
      return ERROR_INVAL;
    }
  InetSocketAddress transport = InetSocketAddress::ConvertFrom (address);
  Ipv4Address ipv4 = transport.GetIpv4 ();
  uint16_t port = transport.GetPort ();
  if (ipv4 == Ipv4Address::GetAny () && port == 0)
    {
      m_endPoint = m_udp->Allocate ();
    }
  else if (ipv4 == Ipv4Address::GetAny () && port != 0)
    {
      m_endPoint = m_udp->Allocate (port);
    }
  else if (ipv4 != Ipv4Address::GetAny () && port == 0)
    {
      m_endPoint = m_udp->Allocate (ipv4);
    }
  else if (ipv4 != Ipv4Address::GetAny () && port != 0)
    {
      m_endPoint = m_udp->Allocate (ipv4, port);
    }

  return FinishBind ();
}

int 
UdpSocket::ShutdownSend (void)
{
  NS_LOG_FUNCTION;
  m_shutdownSend = true;
  return 0;
}

int 
UdpSocket::ShutdownRecv (void)
{
  NS_LOG_FUNCTION;
  m_shutdownRecv = false;
  return 0;
}

int
UdpSocket::Close(void)
{
  NS_LOG_FUNCTION;
  NotifyCloseCompleted ();
  return 0;
}

int
UdpSocket::Connect(const Address & address)
{
  NS_LOG_FUNCTION;
  NS_LOG_PARAM ("(" << address << ")");
  Ipv4Route routeToDest;
  InetSocketAddress transport = InetSocketAddress::ConvertFrom (address);
  m_defaultAddress = transport.GetIpv4 ();
  m_defaultPort = transport.GetPort ();
  NotifyConnectionSucceeded ();
  m_connected = true;

  return 0;
}

int 
UdpSocket::Send (Ptr<Packet> p)
{
  NS_LOG_FUNCTION;
  NS_LOG_PARAM ("(" << p << ")");

  if (!m_connected)
    {
      m_errno = ERROR_NOTCONN;
      return -1;
    }
  return DoSend (p);
}

int 
UdpSocket::DoSend (Ptr<Packet> p)
{
  NS_LOG_FUNCTION;
  if (m_endPoint == 0)
    {
      if (Bind () == -1)
       {
          NS_ASSERT (m_endPoint == 0);
         return -1;
       }
      NS_ASSERT (m_endPoint != 0);
    }
  if (m_shutdownSend)
    {
      m_errno = ERROR_SHUTDOWN;
      return -1;
    } 
  
  return DoSendTo (p, m_defaultAddress, m_defaultPort);
}

int
UdpSocket::DoSendTo (Ptr<Packet> p, const Address &address)
{
  NS_LOG_FUNCTION;
  NS_LOG_PARAM ("(" << p << ", " << address << ")");

  if (!m_connected)
    {
      NS_LOG_LOGIC ("Not connected");
      InetSocketAddress transport = InetSocketAddress::ConvertFrom (address);
      Ipv4Address ipv4 = transport.GetIpv4 ();
      uint16_t port = transport.GetPort ();
      return DoSendTo (p, ipv4, port);
    }
  else
    {
      // connected UDP socket must use default addresses
      NS_LOG_LOGIC ("Connected");
      return DoSendTo (p, m_defaultAddress, m_defaultPort);
    }
}

int
UdpSocket::DoSendTo (Ptr<Packet> p, Ipv4Address dest, uint16_t port)
{
  NS_LOG_FUNCTION;
  NS_LOG_PARAM ("(" << p << ", " << dest << ", " << port << ")");

  Ipv4Route routeToDest;

  if (m_endPoint == 0)
    {
      if (Bind () == -1)
	{
          NS_ASSERT (m_endPoint == 0);
	  return -1;
	}
      NS_ASSERT (m_endPoint != 0);
    }
  if (m_shutdownSend)
    {
      m_errno = ERROR_SHUTDOWN;
      return -1;
    }

  uint32_t localIfIndex;
  Ptr<Ipv4> ipv4 = m_node->QueryInterface<Ipv4> (Ipv4::iid);

  //
  // If dest is sent to the limited broadcast address (all ones),
  // convert it to send a copy of the packet out of every interface
  //
  if (dest.IsBroadcast ())
    {
      NS_LOG_LOGIC ("Limited broadcast");
      for (uint32_t i = 0; i < ipv4->GetNInterfaces (); i++ )
        {
          Ipv4Address addri = ipv4->GetAddress (i);
          Ipv4Mask maski = ipv4->GetNetworkMask (i);
          m_udp->Send (p->Copy (), addri, addri.GetSubnetDirectedBroadcast (maski),
                       m_endPoint->GetLocalPort (), port);
          NotifyDataSent (p->GetSize ());
        }
    }
  else if (ipv4->GetIfIndexForDestination(dest, localIfIndex))
    {
      NS_LOG_LOGIC ("Route exists");
      m_udp->Send (p, ipv4->GetAddress (localIfIndex), dest,
		   m_endPoint->GetLocalPort (), port);
      NotifyDataSent (p->GetSize ());
      return 0;
    }
  else
   {
      NS_LOG_ERROR ("ERROR_NOROUTETOHOST");
      m_errno = ERROR_NOROUTETOHOST;
      return -1;
   }

  return 0;
}

int 
UdpSocket::SendTo(const Address &address, Ptr<Packet> p)
{
  NS_LOG_FUNCTION;
  NS_LOG_PARAM ("(" << address << ", " << p << ")");
  InetSocketAddress transport = InetSocketAddress::ConvertFrom (address);
  Ipv4Address ipv4 = transport.GetIpv4 ();
  uint16_t port = transport.GetPort ();
  return DoSendTo (p, ipv4, port);
}

void 
UdpSocket::ForwardUp (Ptr<Packet> packet, Ipv4Address ipv4, uint16_t port)
{
  NS_LOG_FUNCTION;
  NS_LOG_PARAM ("(" << packet << ", " << ipv4 << ", " << port << ")");

  if (m_shutdownRecv)
    {
      return;
    }
  
  Address address = InetSocketAddress (ipv4, port);
  NotifyDataReceived (packet, address);
}

} //namespace ns3


#ifdef RUN_SELF_TESTS

#include "ns3/test.h"
#include "ns3/internet-node.h"
#include "ns3/socket-factory.h"
#include "ns3/udp.h"
#include "ns3/simulator.h"
#include "ns3/point-to-point-channel.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/drop-tail-queue.h"
#include <string>

namespace ns3 {

class UdpSocketTest: public Test
{
  Ptr<Packet> m_receivedPacket;
  Ptr<Packet> m_receivedPacket2;

public:
  virtual bool RunTests (void);
  UdpSocketTest ();

  void ReceivePacket (Ptr<Socket> socket, Ptr<Packet> packet, const Address &from);
  void ReceivePacket2 (Ptr<Socket> socket, Ptr<Packet> packet, const Address &from);
};


UdpSocketTest::UdpSocketTest ()
  : Test ("UdpSocket") {}


void UdpSocketTest::ReceivePacket (Ptr<Socket> socket, Ptr<Packet> packet, const Address &from)
{
  m_receivedPacket = packet;
}

void UdpSocketTest::ReceivePacket2 (Ptr<Socket> socket, Ptr<Packet> packet, const Address &from)
{
  m_receivedPacket2 = packet;
}

bool
UdpSocketTest::RunTests (void)
{
  bool result = true;

  // Create topology
  
  // Receiver Node
  Ptr<Node> rxNode = Create<InternetNode> ();
  Ptr<PointToPointNetDevice> rxDev = Create<PointToPointNetDevice> (rxNode);
  rxDev->AddQueue(Create<DropTailQueue> ());
  Ptr<Ipv4> ipv4 = rxNode->QueryInterface<Ipv4> (Ipv4::iid);
  uint32_t netdev_idx = ipv4->AddInterface (rxDev);
  ipv4->SetAddress (netdev_idx, Ipv4Address ("10.0.0.1"));
  ipv4->SetNetworkMask (netdev_idx, Ipv4Mask (0xffff0000U));
  ipv4->SetUp (netdev_idx);

  // Sender Node
  Ptr<Node> txNode = Create<InternetNode> ();
  Ptr<PointToPointNetDevice> txDev = Create<PointToPointNetDevice> (txNode);
  txDev->AddQueue(Create<DropTailQueue> ());
  ipv4 = txNode->QueryInterface<Ipv4> (Ipv4::iid);
  netdev_idx = ipv4->AddInterface (txDev);
  ipv4->SetAddress (netdev_idx, Ipv4Address ("10.0.0.2"));
  ipv4->SetNetworkMask (netdev_idx, Ipv4Mask (0xffff0000U));
  ipv4->SetUp (netdev_idx);

  // link the two nodes
  Ptr<PointToPointChannel> channel = Create<PointToPointChannel> ();
  rxDev->Attach (channel);
  txDev->Attach (channel);


  // Create the UDP sockets
  Ptr<SocketFactory> rxSocketFactory = rxNode->QueryInterface<SocketFactory> (Udp::iid);
  Ptr<Socket> rxSocket = rxSocketFactory->CreateSocket ();
  NS_TEST_ASSERT_EQUAL (rxSocket->Bind (InetSocketAddress (Ipv4Address ("10.0.0.2"), 1234)), 0);
  rxSocket->SetRecvCallback (MakeCallback (&UdpSocketTest::ReceivePacket, this));

  Ptr<SocketFactory> txSocketFactory = txNode->QueryInterface<SocketFactory> (Udp::iid);
  Ptr<Socket> txSocket = txSocketFactory->CreateSocket ();

  // ------ Now the tests ------------

  // Unicast test
  m_receivedPacket = Create<Packet> ();
  NS_TEST_ASSERT_EQUAL (txSocket->SendTo (InetSocketAddress (Ipv4Address("10.0.0.1"), 1234),
                                          Create<Packet> (123)), 0);
  Simulator::Run ();
  NS_TEST_ASSERT_EQUAL (m_receivedPacket->GetSize (), 123);


  // Simple broadcast test

  m_receivedPacket = Create<Packet> ();
  NS_TEST_ASSERT_EQUAL (txSocket->SendTo (InetSocketAddress (Ipv4Address("255.255.255.255"), 1234),
                                          Create<Packet> (123)), 0);
  Simulator::Run ();
  NS_TEST_ASSERT_EQUAL (m_receivedPacket->GetSize (), 123);


  // Broadcast test with multiple receiving sockets

  // When receiving broadcast packets, all sockets sockets bound to
  // the address/port should receive a copy of the same packet.
  Ptr<Socket> rxSocket2 = rxSocketFactory->CreateSocket ();
  rxSocket2->SetRecvCallback (MakeCallback (&UdpSocketTest::ReceivePacket2, this));
  NS_TEST_ASSERT_EQUAL (rxSocket2->Bind (InetSocketAddress (Ipv4Address ("0.0.0.0"), 1234)), 0);

  m_receivedPacket = Create<Packet> ();
  m_receivedPacket2 = Create<Packet> ();
  NS_TEST_ASSERT_EQUAL (txSocket->SendTo (InetSocketAddress (Ipv4Address("255.255.255.255"), 1234),
                                          Create<Packet> (123)), 0);
  Simulator::Run ();
  NS_TEST_ASSERT_EQUAL (m_receivedPacket->GetSize (), 123);
  NS_TEST_ASSERT_EQUAL (m_receivedPacket2->GetSize (), 123);

  return result;
}


static UdpSocketTest gUdpSocketTest;

}; // namespace ns3

#endif /* RUN_SELF_TESTS */
