/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2004 Francisco J. Ros 
 * Copyright (c) 2007 INESC Porto
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
 * Authors: Francisco J. Ros  <fjrm@dif.um.es>
 *          Gustavo J. A. M. Carneiro <gjc@inescporto.pt>
 */

///
/// \file	olsr-routing-table.h
/// \brief	Header file for routing table's related stuff.
///

#ifndef __OLSR_RTABLE_H__
#define __OLSR_RTABLE_H__

#include "ns3/ipv4.h"
#include <map>


namespace ns3 { namespace olsr {

/// An %OLSR's routing table entry.
struct RoutingTableEntry
{
  Ipv4Address destAddr;	///< Address of the destination node.
  Ipv4Address nextAddr;	///< Address of the next hop.
  uint32_t interface; ///< Interface index
  uint32_t distance; ///< Distance in hops to the destination.

  RoutingTableEntry () : // default values
    destAddr (), nextAddr (),
    interface (0), distance (0) {};
};

///
/// \brief Defines rtable_t as a map of OLSR_rt_entry, whose key is the destination address.
///
/// The routing table is thus defined as pairs: [dest address, entry]. Each element
/// of the pair can be accesed via "first" and "second" members.
///
//typedef std::map<Ipv4Address, RoutingTableEntry> RoutingTable;

///
/// \brief This class is a representation of the OLSR's Routing Table.
///
class RoutingTable : public Ipv4RoutingProtocol
{
  std::map<Ipv4Address, RoutingTableEntry> m_table; ///< Data structure for the routing table.
  Ptr<Ipv4> m_ipv4;

  Ipv4Address m_mainAddress; // used only for printing debug messages

  void DoDispose ()
  {
    m_ipv4 = 0;
    Ipv4RoutingProtocol::DoDispose ();
  }
  
public:
	
  RoutingTable () {}

  void SetIpv4 (Ptr<Ipv4> ipv4);
  void SetMainAddress (Ipv4Address mainAddress);

  ~RoutingTable () {}

  void Clear ();
  uint32_t GetSize () const { return m_table.size (); }
  std::vector<RoutingTableEntry> GetEntries () const;
  void RemoveEntry (const Ipv4Address &dest);
  void AddEntry (const Ipv4Address &dest,
                 const Ipv4Address &next,
                 uint32_t interface,
                 uint32_t distance);
  void AddEntry (const Ipv4Address &dest,
                 const Ipv4Address &next,
                 const Ipv4Address &interfaceAddress,
                 uint32_t distance);
  bool Lookup (const Ipv4Address &dest,
               RoutingTableEntry &outEntry) const;
  bool FindSendEntry (const RoutingTableEntry &entry,
                      RoutingTableEntry &outEntry) const;

  // From Ipv4RoutingProtocol
  virtual bool RequestRoute (uint32_t ifIndex,
                             const Ipv4Header &ipHeader,
                             Ptr<Packet> packet,
                             RouteReplyCallback routeReply);
  virtual bool RequestIfIndex (Ipv4Address destination, 
                               uint32_t& ifIndex);

};

}}; // namespace ns3, olsr

#endif
