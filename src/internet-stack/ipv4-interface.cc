/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006,2007 INRIA
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

#include "ipv4-interface.h"
#include "ns3/ipv4-address.h"
#include "ns3/net-device.h"
#include "ns3/log.h"
#include "ns3/packet.h"

NS_LOG_COMPONENT_DEFINE ("Ipv4Interface");

namespace ns3 {

TypeId 
Ipv4Interface::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Ipv4Interface")
    .SetParent<Object> ()
    ;
  return tid;
}

  /**
   * By default, Ipv4 interface are created in the "down" state
   * with ip address 192.168.0.1 and a matching mask. Before
   * becoming useable, the user must invoke SetUp on them
   * once the final Ipv4 address and mask has been set.
   */
Ipv4Interface::Ipv4Interface () 
  : m_ifup(false),
    m_metric(1)
{
  NS_LOG_FUNCTION (this);
}

Ipv4Interface::~Ipv4Interface ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
Ipv4Interface::DoDispose (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Object::DoDispose ();
}

void
Ipv4Interface::SetMetric (uint16_t metric)
{
  NS_LOG_FUNCTION (metric);
  m_metric = metric;
}

uint16_t
Ipv4Interface::GetMetric (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_metric;
}

uint16_t 
Ipv4Interface::GetMtu (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  if (GetDevice () == 0)
    {
      uint32_t mtu = (1<<16) - 1;
      return mtu;
    }
  return GetDevice ()->GetMtu ();
}

/**
 * These are IP interface states and may be distinct from 
 * NetDevice states, such as found in real implementations
 * (where the device may be down but IP interface state is still up).
 */
bool 
Ipv4Interface::IsUp (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_ifup;
}

bool 
Ipv4Interface::IsDown (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return !m_ifup;
}

void 
Ipv4Interface::SetUp (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_ifup = true;
}

void 
Ipv4Interface::SetDown (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_ifup = false;
}

// public wrapper on private virtual function
void 
Ipv4Interface::Send(Ptr<Packet> p, Ipv4Address dest)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (IsUp()) {
    NS_LOG_LOGIC ("SendTo");
    SendTo(p, dest);
  }
}

uint32_t
Ipv4Interface::GetNAddresses (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_ifaddrs.size();
}

uint32_t
Ipv4Interface::AddAddress (Ipv4InterfaceAddress addr)
{
  NS_LOG_FUNCTION_NOARGS ();
  uint32_t index = m_ifaddrs.size ();
  m_ifaddrs.push_back (addr);
  return index;
}

Ipv4InterfaceAddress
Ipv4Interface::GetAddress (uint32_t index) const
{
  NS_LOG_FUNCTION_NOARGS ();
  if (index < m_ifaddrs.size ())
    {
      uint32_t tmp = 0;
      for (Ipv4InterfaceAddressListCI i = m_ifaddrs.begin (); i!= m_ifaddrs.end (); i++)
        {
          if (tmp  == index)
            {
              return *i;
            }
          ++tmp;
        }
    }
  NS_ASSERT (false);  // Assert if not found
  Ipv4InterfaceAddress addr;
  return (addr);  // quiet compiler
}

void
Ipv4Interface::RemoveAddress (uint32_t index)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (index >= m_ifaddrs.size ())
    {
      NS_ASSERT_MSG (false, "Bug in Ipv4Interface::RemoveAddress");
    }
  Ipv4InterfaceAddressListI i = m_ifaddrs.begin ();
  uint32_t tmp = 0;
  while (i != m_ifaddrs.end ())
    {
      if (tmp  == index)
        {
          m_ifaddrs.erase (i);
          return;
        }
       ++tmp;
    }
  NS_ASSERT_MSG (false, "Address " << index << " not found");
}

}; // namespace ns3

