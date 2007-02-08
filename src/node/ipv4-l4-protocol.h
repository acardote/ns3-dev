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

#ifndef IPV4_L4_PROTOCOL_H
#define IPV4_L4_PROTOCOL_H


namespace ns3 {

class Node;
class Packet;
class Ipv4Address;
  
class Ipv4L4Protocol {
public:
  Ipv4L4Protocol(int protocolNumber, int version, int layer);
  virtual ~Ipv4L4Protocol ();

  int GetProtocolNumber (void) const;
  int GetVersion() const;

  virtual Ipv4L4Protocol* Copy() const = 0;
  /**
   * Called from lower-level layers to send the packet up
   * in the stack. 
   */
  virtual void Receive(Packet& p, 
                       Ipv4Address const &source,
                       Ipv4Address const &destination) = 0;

 private:
  int m_protocolNumber;
  int m_version;
};

} // Namespace ns3

#endif
