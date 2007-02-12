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

// NS3 - Layer 4 Protocol base class
// George F. Riley, Georgia Tech, Spring 2007

#ifndef UDP_IPV4_L4_PROTOCOL_H
#define UDP_IPV4_L4_PROTOCOL_H

#include <stdint.h>
#include "ipv4-l4-protocol.h"

namespace ns3 {

class Node;
class Packet;
class Ipv4Address;
  
class UdpIpv4L4Protocol : Ipv4L4Protocol {
public:
  UdpIpv4L4Protocol(Node *node);
  virtual ~UdpIpv4L4Protocol ();

  virtual UdpIpv4L4Protocol* Copy(Node *node) const;
  /**
   * Called from lower-level layers to send the packet up
   * in the stack. 
   */
  virtual void Receive(Packet& p, 
                       Ipv4Address const &source,
                       Ipv4Address const &destination);

 private:
  Node *m_node;
  static const uint8_t UDP_PROTOCOL;
};

} // Namespace ns3

#endif /* UDP_IPV4_L4_PROTOCOL */
