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

#ifndef I_NODE_IMPL_H
#define I_NODE_IMPL_H

#include <list>
#include <string>

#include "ns3/i-node.h"

namespace ns3 {

class Packet;

class INodeImpl : public INode 
{
public:
  enum TraceType {
    IPV4,
  };
  INodeImpl();
  INodeImpl(uint32_t systemId);
  virtual ~INodeImpl ();

  void SetName(std::string name);
protected:
  virtual void DoDispose(void);
private:
  virtual void DoAddDevice (Ptr<NetDevice> device) const;
  virtual TraceResolver *DoCreateTraceResolver (TraceContext const &context);
  bool ReceiveFromDevice (Ptr<NetDevice> device, const Packet &p, uint16_t protocolNumber) const;
  void Construct (void);
  std::string      m_name;
};

}//namespace ns3

#endif /* I_NODE_IMPL_H */
