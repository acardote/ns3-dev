/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 Georgia Tech Research Corporation
 *               2007 INRIA
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
 * Authors: George F. Riley<riley@ece.gatech.edu>
 *          Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "ns3/log.h"
#include "ns3/packet.h"
#include "socket.h"

NS_LOG_COMPONENT_DEFINE ("Socket");

namespace ns3 {

Socket::~Socket ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void 
Socket::SetCloseCallback (Callback<void,Ptr<Socket> > closeCompleted)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_closeCompleted = closeCompleted;
}

void 
Socket::SetConnectCallback (
  Callback<void, Ptr<Socket> > connectionSucceeded,
  Callback<void, Ptr<Socket> > connectionFailed,
  Callback<void, Ptr<Socket> > halfClose)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_connectionSucceeded = connectionSucceeded;
  m_connectionFailed = connectionFailed;
  m_halfClose = halfClose;
}

void 
Socket::SetAcceptCallback (
  Callback<bool, Ptr<Socket>, const Address &> connectionRequest,
  Callback<void, Ptr<Socket>, const Address&> newConnectionCreated,
  Callback<void, Ptr<Socket> > closeRequested)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_connectionRequest = connectionRequest;
  m_newConnectionCreated = newConnectionCreated;
  m_closeRequested = closeRequested;
}

bool 
Socket::SetDataSentCallback (Callback<void, Ptr<Socket>, uint32_t> dataSent)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_dataSent = dataSent;
  return true;
}

void
Socket::SetSendCallback (Callback<void, Ptr<Socket>, uint32_t> sendCb)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_sendCb = sendCb;
}

void 
Socket::SetRecvCallback (Callback<void, Ptr<Socket>, Ptr<Packet>,const Address&> receivedData)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_receivedData = receivedData;
}

int Socket::Send (const uint8_t* buf, uint32_t size)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<Packet> p;
  if (buf)
    {
      p = Create<Packet> (buf, size);
    }
  else
    {
      p = Create<Packet> (size);
    }
  return Send (p);
}

int Socket::SendTo (const Address &address, const uint8_t* buf, uint32_t size)
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<Packet> p;
  if(buf)
    {
      p = Create<Packet> (buf, size);
    }
  else
    {
      p = Create<Packet> (size);
    }
  return SendTo (address,p);
}

int Socket::Listen(uint32_t queueLimit)
{
  return 0; //XXX the base class version does nothing
}


void 
Socket::NotifyCloseCompleted (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (!m_closeCompleted.IsNull ())
    {
      m_closeCompleted (this);
    }
}

void 
Socket::NotifyConnectionSucceeded (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (!m_connectionSucceeded.IsNull ())
    {
      m_connectionSucceeded (this);
    }
}

void 
Socket::NotifyConnectionFailed (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (!m_connectionFailed.IsNull ())
    {
      m_connectionFailed (this);
    }
}

void 
Socket::NotifyHalfClose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (!m_halfClose.IsNull ())
    {
      m_halfClose (this);
    }
}

bool 
Socket::NotifyConnectionRequest (const Address &from)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (!m_connectionRequest.IsNull ())
    {
      return m_connectionRequest (this, from);
    }
  else
    {
      // accept all incoming connections by default.
      // this way people writing code don't have to do anything
      // special like register a callback that returns true
      // just to get incoming connections
      return true;
    }
}

void 
Socket::NotifyNewConnectionCreated (Ptr<Socket> socket, const Address &from)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (!m_newConnectionCreated.IsNull ())
    {
      m_newConnectionCreated (socket, from);
    }
}

void 
Socket::NotifyCloseRequested (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (!m_closeRequested.IsNull ())
    {
      m_closeRequested (this);
    }
}

void 
Socket::NotifyDataSent (uint32_t size)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (!m_dataSent.IsNull ())
    {
      m_dataSent (this, size);
    }
}

void 
Socket::NotifySend (uint32_t spaceAvailable)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (!m_sendCb.IsNull ())
    {
      m_sendCb (this, spaceAvailable);
    }
}

void 
Socket::NotifyDataReceived (Ptr<Packet> p, const Address &from)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (!m_receivedData.IsNull ())
    {
      m_receivedData (this, p, from);
    }
}

}//namespace ns3
