/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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

#include "mac-stations.h"
#include "wifi-default-parameters.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/tag.h"

NS_LOG_COMPONENT_DEFINE ("MacStations");

namespace ns3 {

/**
 * _all_ broadcast and multicast frames are transmitted
 * at the same constant default rate because since we don't
 * have any kind of feedback from their transmission,
 * we cannot adjust the rate, so, we pick one which ensures
 * that all frames reach destination.
 */
class NonUnicastMacStation : public MacStation
{
public:
  NonUnicastMacStation (MacStations *stations);
  virtual void ReportRxOk (double rxSnr, WifiMode txMode);
  virtual void ReportRtsFailed (void);
  virtual void ReportDataFailed (void);
  virtual void ReportRtsOk (double ctsSnr, WifiMode ctsMode, double rtsSnr);
  virtual void ReportDataOk (double ackSnr, WifiMode ackMode, double dataSnr);
  virtual void ReportFinalRtsFailed (void);
  virtual void ReportFinalDataFailed (void);

private:
  virtual MacStations *GetStations (void) const;
  virtual WifiMode DoGetDataMode (uint32_t size);
  virtual WifiMode DoGetRtsMode (void);
  MacStations *m_stations;
};

NonUnicastMacStation::NonUnicastMacStation (MacStations *stations)
  : m_stations (stations)
{
  RecordDisassociated ();
}
void 
NonUnicastMacStation::ReportRxOk (double rxSnr, WifiMode txMode)
{
  NS_ASSERT (false);
}
void 
NonUnicastMacStation::ReportRtsFailed (void)
{
  NS_ASSERT (false);
}
void 
NonUnicastMacStation::ReportDataFailed (void)
{
  NS_ASSERT (false);
}
void 
NonUnicastMacStation::ReportRtsOk (double ctsSnr, WifiMode ctsMode, double rtsSnr)
{
  NS_ASSERT (false);
}
void 
NonUnicastMacStation::ReportDataOk (double ackSnr, WifiMode ackMode, double dataSnr)
{
  NS_ASSERT (false);
}
void 
NonUnicastMacStation::ReportFinalRtsFailed (void)
{}
void 
NonUnicastMacStation::ReportFinalDataFailed (void)
{}

WifiMode 
NonUnicastMacStation::DoGetDataMode (uint32_t size)
{
  WifiMode mode = m_stations->GetBasicMode (0);
  NS_LOG_DEBUG ("non-unicast size="<<size<<", mode="<<mode);
  return mode;
}
WifiMode 
NonUnicastMacStation::DoGetRtsMode (void)
{
  NS_ASSERT (false);
  // theoretically, no rts for broadcast/multicast packets.
  return m_stations->GetBasicMode (0);
}
MacStations *
NonUnicastMacStation::GetStations (void) const
{
  return m_stations;
}


} // namespace ns3

namespace ns3 {

MacStations::MacStations (WifiMode defaultTxMode)
  : m_defaultTxMode (defaultTxMode),
    m_nonUnicast (new NonUnicastMacStation (this)),
    m_isLowLatency (WifiDefaultParameters::GetIsLowLatency ())
{
  Reset ();
}

MacStations::~MacStations ()
{
  for (Stations::const_iterator i = m_stations.begin (); i != m_stations.end (); i++) 
    {
      delete (*i).second;
    }
  m_stations.clear ();
  delete m_nonUnicast;
}

MacStation *
MacStations::Lookup (Mac48Address address)
{
  if (address.IsBroadcast () ||
      address.IsMulticast ())
    {
      return m_nonUnicast;
    }
  for (Stations::const_iterator i = m_stations.begin (); i != m_stations.end (); i++) 
    {
      if ((*i).first == address)
        {
          return (*i).second;
        }
    }
  MacStation *station = CreateStation ();
  station->Reset ();
  m_stations.push_back (std::make_pair (address, station));
  return station;
}

MacStation *
MacStations::LookupNonUnicast (void)
{
  return m_nonUnicast;
}

WifiMode 
MacStations::GetDefaultMode (void) const
{
  return m_defaultTxMode;
}
void
MacStations::Reset (void)
{
  for (Stations::const_iterator i = m_stations.begin (); i != m_stations.end (); i++)
    {
      delete i->second;
    }
  m_stations.clear ();
  m_basicModes.clear ();
  m_basicModes.push_back (m_defaultTxMode);
  NS_ASSERT (m_defaultTxMode.IsMandatory ());
}
void 
MacStations::AddBasicMode (WifiMode mode)
{
  for (uint32_t i = 0; i < GetNBasicModes (); i++)
    {
      if (GetBasicMode (i) == mode)
        {
          return;
        }
    }
  m_basicModes.push_back (mode);
}
uint32_t 
MacStations::GetNBasicModes (void) const
{
  return m_basicModes.size ();
}
WifiMode 
MacStations::GetBasicMode (uint32_t i) const
{
  NS_ASSERT (i < m_basicModes.size ());
  return m_basicModes[i];
}
MacStations::BasicModesIterator 
MacStations::BeginBasicModes (void) const
{
  return m_basicModes.begin ();
}
MacStations::BasicModesIterator 
MacStations::EndBasicModes (void) const
{
  return m_basicModes.end ();
}
bool
MacStations::IsLowLatency (void) const
{
  return m_isLowLatency;
}


} // namespace ns3

/***************************************************************
 *           Packet Mode Tagger
 ***************************************************************/ 

namespace ns3 {

class TxModeTag : public Tag
{
public:
  TxModeTag ();
  TxModeTag (WifiMode rtsMode, WifiMode dataMode);
  WifiMode GetRtsMode (void) const;
  WifiMode GetDataMode (void) const;

  static uint32_t GetUid (void);
  void Print (std::ostream &os) const;
  void Serialize (ns3::Buffer::Iterator start) const;
  uint32_t Deserialize (ns3::Buffer::Iterator start);
  uint32_t GetSerializedSize (void) const;
private:
  WifiMode m_rtsMode;
  WifiMode m_dataMode;
};

TxModeTag::TxModeTag ()
{}
TxModeTag::TxModeTag (WifiMode rtsMode, WifiMode dataMode)
  : m_rtsMode (rtsMode),
    m_dataMode (dataMode)
{}
WifiMode 
TxModeTag::GetRtsMode (void) const
{
  return m_rtsMode;
}
WifiMode 
TxModeTag::GetDataMode (void) const
{
  return m_dataMode;
}

uint32_t 
TxModeTag::GetUid (void)
{
  static uint32_t uid = Tag::AllocateUid<TxModeTag> ("ns3.wifi.TxModeTag");
  return uid;
}
void 
TxModeTag::Print (std::ostream &os) const
{
  os << "rts="<<m_rtsMode<<" data="<<m_dataMode;
}
void 
TxModeTag::Serialize (ns3::Buffer::Iterator start) const
{}
uint32_t 
TxModeTag::Deserialize (ns3::Buffer::Iterator start)
{
  return 0;
}
uint32_t 
TxModeTag::GetSerializedSize (void) const
{
  return 0;
}

} // namespace ns3


/***************************************************************
 *           MacStation below.
 ***************************************************************/ 

namespace ns3 {

MacStation::MacStation ()
  : m_state (BRAND_NEW)
{}
MacStation::~MacStation ()
{}

bool
MacStation::IsBrandNew (void) const
{
  return m_state == BRAND_NEW;
}

bool 
MacStation::IsAssociated (void) const
{
  return m_state == GOT_ASSOC_TX_OK;
}
bool 
MacStation::IsWaitAssocTxOk (void) const
{
  return m_state == WAIT_ASSOC_TX_OK;
}
void 
MacStation::RecordWaitAssocTxOk (void)
{
  m_state = WAIT_ASSOC_TX_OK;
}
void 
MacStation::RecordGotAssocTxOk (void)
{
  m_state = GOT_ASSOC_TX_OK;
}
void 
MacStation::RecordGotAssocTxFailed (void)
{
  m_state = DISASSOC;
}
void 
MacStation::RecordDisassociated (void)
{
  m_state = DISASSOC;
}

void 
MacStation::Reset (void)
{
  m_modes.clear ();
  AddSupportedMode (GetStations ()->GetDefaultMode ());
}
void 
MacStation::AddSupportedMode (WifiMode mode)
{
  if (IsIn (mode))
    {
      return;
    }
  m_modes.push_back (mode);
}

bool
MacStation::IsIn (WifiMode mode) const
{
  for (SupportedModes::const_iterator i = m_modes.begin (); i != m_modes.end (); i++)
    {
      if ((*i) == mode)
        {
          return true;
        }
    }
  return false;
}

WifiMode
MacStation::GetControlAnswerMode (WifiMode reqMode)
{
  /**
   * see ieee 802.11e, section 9.6:
   * 
   * To allow the transmitting STA to calculate the contents of 
   * the Duration/ID field, a STA responding to a received frame 
   * shall transmit its Control Response frame (either CTS or ACK) 
   * frames, other than the Block-Ack control frame, at the highest 
   * rate in the BSSBasicRateSet parameter that is less than or equal 
   * to the rate of the immediately previous frame in the frame 
   * exchange sequence (as defined in 9.79.12) and that is of the
   * same modulation type as the received frame. If no rate in the 
   * basic rate set meets these conditions, then the control frame 
   * sent in response to a received frame shall be transmitted at 
   * the highest mandatory rate of the PHY that is less than or equal 
   * to the rate of the received frame, and that is of the same 
   * modulation type as the received frame. In addition, the Control 
   * Response frame shall be sent using the same PHY options as the
   * received frame, unless they conflict with the requirement to use 
   * the BSSBasicRateSet parameter.
   */
  WifiMode mode = GetStations ()->GetDefaultMode ();
  bool found = false;

  // First, search the BSS Basic Rate set
  for (MacStations::BasicModesIterator i = GetStations ()->BeginBasicModes (); 
       i != GetStations ()->EndBasicModes (); i++)
    {
      if (i->GetPhyRate () > mode.GetPhyRate () &&
          i->GetPhyRate () <= reqMode.GetPhyRate () &&
          i->GetModulationType () == reqMode.GetModulationType ())
        {
          mode = *i;
          found = true;
        }
    }
  // no need to search Mandatory rate set because it is included
  // within the Basic rate set.
  return mode;
}

WifiMode 
MacStation::GetCtsMode (WifiMode rtsMode)
{
  return GetControlAnswerMode (rtsMode);
}
WifiMode 
MacStation::GetAckMode (WifiMode dataMode)
{
  return GetControlAnswerMode (dataMode);
}

uint32_t 
MacStation::GetNSupportedModes (void) const
{
  return m_modes.size ();
}
WifiMode 
MacStation::GetSupportedMode (uint32_t i) const
{
  NS_ASSERT (i < m_modes.size ());
  return m_modes[i];
}
void 
MacStation::PrepareForQueue (Ptr<const Packet> packet, uint32_t fullPacketSize)
{
  if (GetStations ()->IsLowLatency ())
    {
      return;
    }
  TxModeTag tag = TxModeTag (DoGetRtsMode (), DoGetDataMode (fullPacketSize));
  packet->AddTag (tag);
}
WifiMode 
MacStation::GetDataMode (Ptr<const Packet> packet, uint32_t fullPacketSize)
{
  if (GetStations ()->IsLowLatency ())
    {
      return DoGetDataMode (fullPacketSize);
    }
  TxModeTag tag;
  bool found = packet->PeekTag (tag);
  NS_ASSERT (found);
  return tag.GetDataMode ();
}
WifiMode 
MacStation::GetRtsMode (Ptr<const Packet> packet)
{
  if (GetStations ()->IsLowLatency ())
    {
      return DoGetRtsMode ();
    }
  TxModeTag tag;
  bool found = packet->PeekTag (tag);
  NS_ASSERT (found);
  return tag.GetRtsMode ();
}


} // namespace ns3

