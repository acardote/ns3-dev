/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005 INRIA
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

#include "ns3/assert.h"
#include "ns3/address-utils.h"
#include "arp-header.h"

namespace ns3 {

const char *
ArpHeader::GetUid (void)
{
  return "ArpHeader.ns3";
}

ArpHeader::~ArpHeader ()
{}

void 
ArpHeader::SetRequest (MacAddress sourceHardwareAddress,
                       Ipv4Address sourceProtocolAddress,
                       MacAddress destinationHardwareAddress,
                       Ipv4Address destinationProtocolAddress)
{
  m_type = ARP_TYPE_REQUEST;
  m_macSource = sourceHardwareAddress;
  m_macDest = destinationHardwareAddress;
  m_ipv4Source = sourceProtocolAddress;
  m_ipv4Dest = destinationProtocolAddress;
}
void 
ArpHeader::SetReply (MacAddress sourceHardwareAddress,
                     Ipv4Address sourceProtocolAddress,
                     MacAddress destinationHardwareAddress,
                     Ipv4Address destinationProtocolAddress)
{
  m_type = ARP_TYPE_REPLY;
  m_macSource = sourceHardwareAddress;
  m_macDest = destinationHardwareAddress;
  m_ipv4Source = sourceProtocolAddress;
  m_ipv4Dest = destinationProtocolAddress;
}
bool 
ArpHeader::IsRequest (void) const
{
  return (m_type == ARP_TYPE_REQUEST)?true:false;
}
bool 
ArpHeader::IsReply (void) const
{
  return (m_type == ARP_TYPE_REPLY)?true:false;
}
MacAddress 
ArpHeader::GetSourceHardwareAddress (void)
{
  return m_macSource;
}
MacAddress 
ArpHeader::GetDestinationHardwareAddress (void)
{
  return m_macDest;
}
Ipv4Address 
ArpHeader::GetSourceIpv4Address (void)
{
  return m_ipv4Source;
}
Ipv4Address 
ArpHeader::GetDestinationIpv4Address (void)
{
  return m_ipv4Dest;
}

std::string 
ArpHeader::DoGetName (void) const
{
  return "ARP";
}

void 
ArpHeader::PrintTo (std::ostream &os) const
{
  if (IsRequest ()) 
    {
      os << "("
         << "request "
         << "source mac: " << m_macSource << " "
         << "source ipv4: " << m_ipv4Source << " "
         << "dest ipv4: " << m_ipv4Dest
         << ")"
        ;
    } 
  else 
    {
      NS_ASSERT (IsReply ());
      os << "("
         << "reply " 
         << "source mac: " << m_macSource << " "
         << "source ipv4: " << m_ipv4Source << " "
         << "dest mac: " << m_macDest << " "
         << "dest ipv4: " <<m_ipv4Dest
         << ")"
        ;
    }
}
uint32_t 
ArpHeader::GetSerializedSize (void) const
{
  /* this is the size of an ARP payload. */
  return 28;
}

void
ArpHeader::SerializeTo (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  NS_ASSERT (m_macSource.GetLength () == m_macDest.GetLength ());

  /* ethernet */
  i.WriteHtonU16 (0x0001);
  /* ipv4 */
  i.WriteHtonU16 (0x0800);
  i.WriteU8 (m_macSource.GetLength ());
  i.WriteU8 (4);
  i.WriteHtonU16 (m_type);
  WriteTo (i, m_macSource);
  WriteTo (i, m_ipv4Source);
  WriteTo (i, m_macDest);
  WriteTo (i, m_ipv4Dest);
}
uint32_t
ArpHeader::DeserializeFrom (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  i.Next (2+2);
  uint32_t hardwareAddressLen = i.ReadU8 ();
  i.Next (1);
  m_type = i.ReadNtohU16 ();
  ReadFrom (i, m_macSource, hardwareAddressLen);
  ReadFrom (i, m_ipv4Source);
  ReadFrom (i, m_macDest, hardwareAddressLen);
  ReadFrom (i, m_ipv4Dest);
  return GetSerializedSize ();
}

}; // namespace ns3
