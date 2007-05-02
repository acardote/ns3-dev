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

// Define the layer 4 demultiplexer object for ns3.
// George F. Riley, Georgia Tech, Fall 2006

#ifndef IPV4_L4_DEMUX_H
#define IPV4_L4_DEMUX_H

#include <list>
#include "ns3/object.h"

namespace ns3 {

class Ipv4L4Protocol;
class Node;
class TraceResolver;
class TraceContext;

/**
 * \brief L4 Ipv4 Demux
 */
class Ipv4L4Demux : public Object
{
public:
  typedef int Ipv4L4ProtocolTraceType;
  Ipv4L4Demux (Node *node);
  virtual ~Ipv4L4Demux();

  void Dispose (void);

  /**
   * \param node the node on which the returned copy will run.
   * \returns a deep copy of this L4 Demux.
   */
  Ipv4L4Demux* Copy(Node *node) const;

  /**
   * \param context the trace context to use to construct the
   *        TraceResolver to return
   * \returns a TraceResolver which can resolve all traces
   *          performed in this object. The caller must
   *          delete the returned object.
   */
  TraceResolver *CreateTraceResolver (TraceContext const &context);
  /**
   * \param protocol a template for the protocol to add to this L4 Demux.
   * \returns the L4Protocol effectively added.
   *
   * Invoke Copy on the input template to get a copy of the input
   * protocol which can be used on the Node on which this L4 Demux 
   * is running. The new L4Protocol is registered internally as
   * a working L4 Protocol and returned from this method.
   * The caller does not get ownership of the returned pointer.
   */
  Ipv4L4Protocol* Insert(const Ipv4L4Protocol&protocol);
  /**
   * \param protocolNumber number of protocol to lookup
   *        in this L4 Demux
   * \returns a matching L4 Protocol
   *
   * This method is typically called by lower layers
   * to forward packets up the stack to the right protocol.
   * It is also called from InternetNode::GetUdp for example.
   */
  Ipv4L4Protocol* PeekProtocol(int protocolNumber);
  /**
   * \param protocol protocol to remove from this demux.
   *
   * The input value to this method should be the value
   * returned from the Ipv4L4Protocol::Insert method.
   */
  void Erase(Ipv4L4Protocol*protocol);
private:
  typedef std::list<Ipv4L4Protocol*> L4List_t;
  L4List_t m_protocols;
  Node *m_node;
};

} //namespace ns3
#endif
