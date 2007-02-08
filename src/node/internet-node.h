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
// Define a basic "Internet" node, with a protocol stack (l3 and l4),
// network device list, process list, and routing.

#ifndef INTERNET_NODE_H
#define INTERNET_NODE_H

#include <list>

#include "node.h"

namespace ns3 {


class InternetNode : public Node 
{
public:
  InternetNode();
  InternetNode(const InternetNode&); // Copy constructor 
  virtual InternetNode* Copy() const;// Make a copy of this node
  // Capability access
  virtual NetDeviceList*   GetNetDevices() const;
  virtual L3Demux*         GetL3Demux() const;
  virtual Ipv4L4Demux*     GetIpv4L4Demux() const;

private:
  // Capabilities
  NetDeviceList*   m_netDevices;
  L3Demux*         m_l3Demux;
  Ipv4L4Demux*     m_ipv4L4Demux;
 };

}//namespace ns3

#endif /* INTERNET_NODE_H */
