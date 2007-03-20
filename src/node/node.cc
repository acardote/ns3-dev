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

// Implement the basic Node object for ns3.
// George F. Riley, Georgia Tech, Fall 2006

#include "node.h"
#include "internet-node.h"
#include "node-list.h"

namespace ns3{

uint32_t Node::g_nextId = 0;
Node::SmartNodeVec_t Node::g_prototypes;  // The node prototypes stack
Node::SmartNodeVec_t Node::g_nodes;       // All nodes

Node::Node()
  : m_id(g_nextId), m_sid(0)
{
  g_nextId++;
}

Node::Node(uint32_t sid)
  : m_id(g_nextId), m_sid(sid)
{ 
  g_nextId++;
}
  
Node::~Node ()
{}


uint32_t 
Node::GetId (void) const
{
  return m_id;
}
uint32_t 
Node::GetSystemId (void) const
{
  return m_sid;
}

void   
Node::SetSystemId(uint32_t s )
{
  m_sid = s;
}

// Node stack creation and management routines.
Node* Node::Create()
{
  if (g_prototypes.Empty()) CreateDefaultPrototype();
  Node* n = g_prototypes.Back()->Copy(); // Copy the top of the stack
  n->m_id = g_nextId++;                  // Set unique node id
  g_nodes.Add(n);                        // Add to smart vector (mem mgmt)
  NodeList::Add (n);                     // Add to global list of nodes
  return n;
}

Node* Node::Create(uint32_t sid)
{ // Create with distributed simulation systemid
  // ! Need to check if sid matches DistributedSimulator system id,
  // and create an empty (ghost) node if so.  Code this later
  Node* n = Create(sid);
  return n;
}

Node* Node::GetNodePrototype()
{ // Get node* to top of prototypes stack
  if (g_prototypes.Empty()) CreateDefaultPrototype();
  return g_prototypes.Back();
}

Node* Node::PushNodePrototype(const Node& n)
{ // Add a new node to the top of the prototypes stack
  g_prototypes.Add(n.Copy());
  return g_prototypes.Back();
}

Node* Node::PushNodePrototype() 
{ // Replicate the top of the prototype stack
  if (g_prototypes.Empty()) CreateDefaultPrototype();
  g_prototypes.Add(GetNodePrototype()->Copy());
  return g_prototypes.Back();
}

void Node::PopNodePrototype()
{ 
  if (!g_prototypes.Empty()) g_prototypes.Remove();
}

void Node::ClearAll()
{ // Delete all nodes for memory leak checking, including prototypes
  g_nodes.Clear();
  g_prototypes.Clear();
}

// Private method to ceate a reasonable default if stack is empty
void Node::CreateDefaultPrototype()
{
  Node* n = new InternetNode();
  g_prototypes.Add(n);
}

L3Demux*
Node::GetL3Demux() const
{
  return 0;
}
Ipv4L4Demux*
Node::GetIpv4L4Demux() const
{
  return 0;
}

NetDeviceList*
Node::GetNetDeviceList() const
{
  return 0;
}

Ipv4 *
Node::GetIpv4 (void) const
{
  return 0;
}
Udp *
Node::GetUdp (void) const
{
  return 0;
}

Arp *
Node::GetArp (void) const
{
  return 0;
}

}//namespace ns3
