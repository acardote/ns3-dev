/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006,2007 INRIA
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

#ifndef UDP_H
#define UDP_H

#include <stdint.h>

#include "ns3/packet.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ipv4-end-point-demux.h"
#include "ipv4-l4-protocol.h"

namespace ns3 {

class INode;
class TraceResolver;
class TraceContext;
class Socket;

class Udp : public Ipv4L4Protocol {
public:
  static const uint8_t PROT_NUMBER;

  Udp (Ptr<INode> node);
  virtual ~Udp ();

  virtual TraceResolver *CreateTraceResolver (TraceContext const &context);

  Ptr<Socket> CreateSocket (void);

  Ipv4EndPoint *Allocate (void);
  Ipv4EndPoint *Allocate (Ipv4Address address);
  Ipv4EndPoint *Allocate (uint16_t port);
  Ipv4EndPoint *Allocate (Ipv4Address address, uint16_t port);
  Ipv4EndPoint *Allocate (Ipv4Address localAddress, uint16_t localPort,
                         Ipv4Address peerAddress, uint16_t peerPort);

  void DeAllocate (Ipv4EndPoint *endPoint);

  // called by UdpSocket.
  void Send (Packet packet,
             Ipv4Address saddr, Ipv4Address daddr, 
             uint16_t sport, uint16_t dport);
  // inherited from Ipv4L4Protocol
  virtual void Receive(Packet& p, 
                       Ipv4Address const &source,
                       Ipv4Address const &destination);
protected:
  virtual void DoDispose (void);
private:
  Ptr<INode> m_node;
  Ipv4EndPointDemux *m_endPoints;
};

}; // namespace ns3

#endif /* UDP_H */
