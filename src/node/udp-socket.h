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
#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include <stdint.h>
#include "ns3/callback.h"
#include "socket.h"

namespace ns3 {

class Ipv4EndPoint;
class Node;
class Packet;
class Udp;

class UdpSocket : public Socket
{
public:
  /**
   * Create an unbound udp socket.
   */
  UdpSocket (Node *node);
  virtual ~UdpSocket ();

  virtual enum SocketErrno GetErrno (void) const;
  virtual Node *PeekNode (void) const;
  virtual int Bind (void);
  virtual int Bind (Ipv4Address address);
  virtual int Bind (uint16_t port); 
  virtual int Bind (Ipv4Address address, uint16_t port);
  virtual int ShutdownSend (void);
  virtual int ShutdownRecv (void);

private:
  virtual void DoClose(ns3::Callback<void, Socket*> closeCompleted);
  virtual void DoConnect(const Ipv4Address & address,
			 uint16_t portNumber,
			 ns3::Callback<void, Socket*> connectionSucceeded,
			 ns3::Callback<void, Socket*> connectionFailed,
			 ns3::Callback<void, Socket*> halfClose);
  virtual int DoAccept(ns3::Callback<bool, Socket*, const Ipv4Address&, uint16_t> connectionRequest,
		       ns3::Callback<void, Socket*, const Ipv4Address&, uint16_t> newConnectionCreated,
		       ns3::Callback<void, Socket*> closeRequested);
  virtual int DoSend (const uint8_t* buffer,
                    uint32_t size,
                    ns3::Callback<void, Socket*, uint32_t> dataSent);
  virtual int DoSendTo(const Ipv4Address &address,
                      uint16_t port,
                      const uint8_t *buffer,
                      uint32_t size,
                      ns3::Callback<void, Socket*, uint32_t> dataSent);
  virtual void DoRecv(ns3::Callback<void, Socket*, const uint8_t*, uint32_t,const Ipv4Address&, uint16_t>);
  virtual void DoRecvDummy(ns3::Callback<void, Socket*, uint32_t,const Ipv4Address&, uint16_t>);

private:
  friend class Udp;
  // invoked by Udp class
  int FinishBind (void);
  void ForwardUp (const Packet &p, Ipv4Address saddr, uint16_t sport);
  void Destroy (void);
  Udp *GetUdp (void) const;
  int DoSendPacketTo (const Packet &p, Ipv4Address daddr, uint16_t dport,
		      ns3::Callback<void, Socket*, uint32_t> dataSent);

  Ipv4EndPoint *m_endPoint;
  Node *m_node;
  Udp *m_udp;
  Ipv4Address m_defaultAddress;
  uint16_t m_defaultPort;
  Callback<void,Socket*,uint32_t,const Ipv4Address &,uint16_t> m_dummyRxCallback;
  Callback<void,Socket*,uint8_t const*,uint32_t,const Ipv4Address &,uint16_t> m_rxCallback;
  enum SocketErrno m_errno;
  bool m_shutdownSend;
  bool m_shutdownRecv;
  bool m_connected;
};

}//namespace ns3

#endif /* UDP_SOCKET_H */
