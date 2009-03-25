/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008,2009 IITP RAS
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
 * Authors: Kirill Andreev <andreev@iitp.ru>
 *          Aleksey Kovalenko <kovalenko@iitp.ru>
 */


#include "ie-dot11s-configuration.h"

namespace ns3 {
namespace dot11s {

dot11sMeshCapability::dot11sMeshCapability ():
    acceptPeerLinks (true),
    MDAEnabled (false),
    forwarding (true),
    beaconTimingReport (false),
    TBTTAdjustment (false),
    powerSaveLevel (false)
{}

uint8_t dot11sMeshCapability::GetSerializedSize () const
{
  return 2;
}

Buffer::Iterator dot11sMeshCapability::Serialize (Buffer::Iterator i) const
{
  uint16_t result = 0;
  if (acceptPeerLinks)
    result |= 1 << 0;
  if (MDAEnabled)
    result |= 1 << 1;
  if (forwarding)
    result |= 1 << 2;
  if (beaconTimingReport)
    result |= 1 << 3;
  if (TBTTAdjustment)
    result |= 1 << 4;
  if (powerSaveLevel)
    result |= 1 << 5;
  i.WriteHtonU16 (result);
  return i;
}

Buffer::Iterator dot11sMeshCapability::Deserialize (Buffer::Iterator i)
{
  uint16_t  cap = i.ReadNtohU16 ();
  acceptPeerLinks = Is (cap, 0);
  MDAEnabled = Is (cap, 1);
  forwarding = Is (cap, 2);
  beaconTimingReport = Is (cap, 3);
  TBTTAdjustment = Is (cap, 4);
  powerSaveLevel = Is (cap, 5);
  return i;
}

bool dot11sMeshCapability::Is (uint16_t cap, uint8_t n) const
{
  uint16_t mask = 1<<n;
  return (cap & mask) == mask;
}

IeConfiguration::IeConfiguration ():
    m_APSId (PROTOCOL_HWMP),
    m_APSMId (METRIC_AIRTIME),
    m_CCMId (CONGESTION_DEFAULT),
    m_CP (CHANNEL_PRECEDENCE_OFF)
{}

TypeId
IeConfiguration::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::IeConfiguration")
    .SetParent<WifiInformationElement> ();
  return tid;
}

TypeId
IeConfiguration::GetInstanceTypeId () const
{
  return GetTypeId ();
}
uint8_t
IeConfiguration::GetInformationSize () const
{
  return 1 // Version
    + 4 // APSPId
    + 4 // APSMId
    + 4 // CCMId
    + 4 // CP
    + m_meshCap.GetSerializedSize ();
}

void
IeConfiguration::SerializeInformation (Buffer::Iterator i) const
{
  i.WriteU8 (1); //Version
  // Active Path Selection Protocol ID:
  i.WriteHtonU32 (m_APSId);
  // Active Path Metric ID:
  i.WriteHtonU32 (m_APSMId);
  // Congestion Control Mode ID:
  i.WriteU32 (m_CCMId);
  // Channel Precedence:
  i.WriteU32 (m_CP);
  m_meshCap.Serialize (i);
}

uint8_t
IeConfiguration::DeserializeInformation (Buffer::Iterator i, uint8_t length)
{
  Buffer::Iterator start = i;
  uint8_t version;
  version  = i.ReadU8 ();
  // Active Path Selection Protocol ID:
  m_APSId  = (dot11sPathSelectionProtocol)i.ReadNtohU32 ();
  // Active Path Metric ID:
  m_APSMId = (dot11sPathSelectionMetric)i.ReadNtohU32 ();
  // Congestion Control Mode ID:
  m_CCMId  = (dot11sCongestionControlMode)i.ReadU32 ();
  // Channel Precedence:
  m_CP     = (dot11sChannelPrecedence)i.ReadU32 ();
  i = m_meshCap.Deserialize (i);
  return i.GetDistanceFrom (start);
}
void
IeConfiguration::PrintInformation (std::ostream& os) const
{
  //TODO: print
}
void
IeConfiguration::SetRouting (dot11sPathSelectionProtocol routingId)
{
  m_APSId  =  routingId;
}

void
IeConfiguration::SetMetric (dot11sPathSelectionMetric metricId)
{
  m_APSMId =  metricId;
}

bool
IeConfiguration::IsHWMP ()
{
  return (m_APSId == PROTOCOL_HWMP);
}

bool
IeConfiguration::IsAirtime ()
{
  return (m_APSMId  == METRIC_AIRTIME);
}

dot11sMeshCapability const& IeConfiguration::MeshCapability ()
{
  return m_meshCap;
}
  
} // namespace dot11s
} //namespace ns3

