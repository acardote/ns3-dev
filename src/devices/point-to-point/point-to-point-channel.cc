/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 University of Washington
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
 * Author: Craig Dowell <craigdo@ee.washington.edu>
 */

#include "point-to-point-channel.h"
#include "point-to-point-net-device.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("PointToPointChannel");

namespace ns3 {

TypeId 
PointToPointChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("PointToPointChannel")
    .SetParent<Channel> ()
    .AddConstructor<PointToPointChannel> ()
    .AddAttribute ("BitRate", "The maximum bitrate of the channel",
                   DataRate (0xffffffff),
                   MakeDataRateAccessor (&PointToPointChannel::m_bps),
                   MakeDataRateChecker ())
    .AddAttribute ("Delay", "Transmission delay through the channel",
                   Seconds (0),
                   MakeTimeAccessor (&PointToPointChannel::m_delay),
                   MakeTimeChecker ())
    ;
  return tid;
}

//
// By default, you get a channel with the name "PointToPoint Channel" that 
// has an "infitely" fast transmission speed and zero delay.
PointToPointChannel::PointToPointChannel()
: 
  Channel ("PointToPoint Channel"), 
  m_nDevices(0)
{
  NS_LOG_FUNCTION;
}

void
PointToPointChannel::Attach(Ptr<PointToPointNetDevice> device)
{
  NS_LOG_FUNCTION;
  NS_LOG_PARAMS (this << device);
  NS_ASSERT(m_nDevices < N_DEVICES && "Only two devices permitted");
  NS_ASSERT(device != 0);

  m_link[m_nDevices++].m_src = device;
//
// If we have both devices connected to the channel, then finish introducing
// the two halves and set the links to IDLE.
//
  if (m_nDevices == N_DEVICES)
    {
      m_link[0].m_dst = m_link[1].m_src;
      m_link[1].m_dst = m_link[0].m_src;
      m_link[0].m_state = IDLE;
      m_link[1].m_state = IDLE;
    }
}

bool
PointToPointChannel::TransmitStart(Ptr<Packet> p,
                                   Ptr<PointToPointNetDevice> src,
                                   const Time& txTime)
{
  NS_LOG_FUNCTION;
  NS_LOG_PARAMS (this << p << src);
  NS_LOG_LOGIC ("UID is " << p->GetUid () << ")");

  NS_ASSERT(m_link[0].m_state != INITIALIZING);
  NS_ASSERT(m_link[1].m_state != INITIALIZING);

  uint32_t wire = src == m_link[0].m_src ? 0 : 1;

  // Here we schedule the packet receive event at the receiver,
  // which simplifies this model quite a bit.  The channel just
  // adds the propagation delay time
  Simulator::Schedule (txTime + m_delay,
                       &PointToPointNetDevice::Receive,
                       m_link[wire].m_dst, p);
  return true;
}

uint32_t 
PointToPointChannel::GetNDevices (void) const
{
  NS_LOG_FUNCTION;
  return m_nDevices;
}

Ptr<PointToPointNetDevice>
PointToPointChannel::GetDevice (uint32_t i) const
{
  NS_LOG_FUNCTION;
  NS_ASSERT(i < 2);
  return m_link[i].m_src;
}

const DataRate&
PointToPointChannel::GetDataRate (void)
{
  NS_LOG_FUNCTION;
  return m_bps;
}

const Time&
PointToPointChannel::GetDelay (void)
{
  NS_LOG_FUNCTION;
  return m_delay;
}

Ptr<NetDevice>
PointToPointChannel::DoGetDevice (uint32_t i) const
{
  NS_LOG_FUNCTION;
  return GetDevice (i);
}


} // namespace ns3
