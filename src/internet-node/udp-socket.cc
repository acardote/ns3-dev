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
#include "ns3/node.h"
#include "ns3/inet-socket-address.h"
#include "udp-socket.h"
#include "udp-l4-protocol.h"
#include "ipv4-end-point.h"
#include "ipv4-l4-demux.h"

namespace ns3 {

UdpSocket::UdpSocket (Ptr<Node> node, Ptr<UdpL4Protocol> udp)
  : m_endPoint (0),
    m_node (node),
    m_udp (udp),
    m_errno (ERROR_NOTERROR),
    m_shutdownSend (false),
    m_shutdownRecv (false),
    m_connected (false)
{}
UdpSocket::~UdpSocket ()
{
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

Ptr<Node>
UdpSocket::GetNode (void) const
{
  return m_node;
}

void 
UdpSocket::Destroy (void)
{
  m_node = 0;
  m_endPoint = 0;
  m_udp = 0;
}
int
UdpSocket::FinishBind (void)
{
  m_endPoint->SetRxCallback (MakeCallback (&UdpSocket::ForwardUp, this));
  m_endPoint->SetDestroyCallback (MakeCallback (&UdpSocket::Destroy, this));
  if (m_endPoint == 0)
    {
      return -1;
    }
  return 0;
}

int
UdpSocket::Bind (void)
{
  m_endPoint = m_udp->Allocate ();
  return FinishBind ();
}
int 
UdpSocket::Bind (const Address &address)
{
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

enum Socket::SocketErrno
UdpSocket::GetErrno (void) const
{
  return m_errno;
}
int 
UdpSocket::ShutdownSend (void)
{
  m_shutdownSend = true;
  return 0;
}
int 
UdpSocket::ShutdownRecv (void)
{
  m_shutdownRecv = false;
  return 0;
}

void 
UdpSocket::DoClose(ns3::Callback<void, Ptr<Socket> > closeCompleted)
{
  // XXX: we should set the close state and check it in all API methods.
  if (!closeCompleted.IsNull ())
    {
      closeCompleted (this);
    }
}
void 
UdpSocket::DoConnect(const Address & address,
                     ns3::Callback<void, Ptr<Socket> > connectionSucceeded,
                     ns3::Callback<void, Ptr<Socket> > connectionFailed,
                     ns3::Callback<void, Ptr<Socket> > halfClose)
{
  InetSocketAddress transport = InetSocketAddress::ConvertFrom (address);
  m_defaultAddress = transport.GetIpv4 ();
  m_defaultPort = transport.GetPort ();
  if (!connectionSucceeded.IsNull ())
    {
      connectionSucceeded (this);
    }
  m_connected = true;
}
int
UdpSocket::DoAccept(ns3::Callback<bool, Ptr<Socket>, const Address&> connectionRequest,
                    ns3::Callback<void, Ptr<Socket>, const Address&> newConnectionCreated,
                    ns3::Callback<void, Ptr<Socket> > closeRequested)
{
  // calling accept on a udp socket is a programming error.
  m_errno = ERROR_OPNOTSUPP;
  return -1;
}
int 
UdpSocket::DoSend (const uint8_t* buffer,
                   uint32_t size,
                   ns3::Callback<void, Ptr<Socket>, uint32_t> dataSent)
{
  if (!m_connected)
    {
      m_errno = ERROR_NOTCONN;
      return -1;
    }
  Packet p;
  if (buffer == 0)
    {
      p = Packet (size);
    }
  else
    {
      p = Packet (buffer, size);
    }
  return DoSendPacketTo (p, m_defaultAddress, m_defaultPort, dataSent);
}
int
UdpSocket::DoSendPacketTo (const Packet &p, const Address &address,
                           ns3::Callback<void, Ptr<Socket>, uint32_t> dataSent)
{
  InetSocketAddress transport = InetSocketAddress::ConvertFrom (address);
  Ipv4Address ipv4 = transport.GetIpv4 ();
  uint16_t port = transport.GetPort ();
  return DoSendPacketTo (p, ipv4, port, dataSent);
}
int
UdpSocket::DoSendPacketTo (const Packet &p, Ipv4Address ipv4, uint16_t port,
                           ns3::Callback<void, Ptr<Socket>, uint32_t> dataSent)
{
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
  m_udp->Send (p, m_endPoint->GetLocalAddress (), ipv4,
		   m_endPoint->GetLocalPort (), port);
  if (!dataSent.IsNull ())
    {
      dataSent (this, p.GetSize ());
    }
  return 0;
}
int 
UdpSocket::DoSendTo(const Address &address,
                    const uint8_t *buffer,
                    uint32_t size,
                    ns3::Callback<void, Ptr<Socket>, uint32_t> dataSent)
{
  if (m_connected)
    {
      m_errno = ERROR_ISCONN;
      return -1;
    }
  Packet p;
  if (buffer == 0)
    {
      p = Packet (size);
    }
  else
    {
      p = Packet (buffer, size);
    }
  InetSocketAddress transport = InetSocketAddress::ConvertFrom (address);
  Ipv4Address ipv4 = transport.GetIpv4 ();
  uint16_t port = transport.GetPort ();
  return DoSendPacketTo (p, ipv4, port, dataSent);
}
void 
UdpSocket::DoRecv(ns3::Callback<void, Ptr<Socket>, const uint8_t*, uint32_t,const Address&> callback)
{
  m_rxCallback = callback;
}
void 
UdpSocket::DoRecvDummy(ns3::Callback<void, Ptr<Socket>, uint32_t,const Address&> callback)
{
  m_dummyRxCallback = callback;
}

void 
UdpSocket::ForwardUp (const Packet &packet, Ipv4Address ipv4, uint16_t port)
{
  if (m_shutdownRecv)
    {
      return;
    }
  
  Address address = InetSocketAddress (ipv4, port).ConvertTo ();
  Packet p = packet;
  if (!m_dummyRxCallback.IsNull ())
    {
      m_dummyRxCallback (this, p.GetSize (), address);
    }
  if (!m_rxCallback.IsNull ())
    {
      m_rxCallback (this, p.PeekData (), p.GetSize (), address);
    }
}

}//namespace ns3
