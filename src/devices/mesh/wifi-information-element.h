/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS
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
 * Author: Pavel Boyko <boyko@iitp.ru>
 */

#ifndef WIFIINFORMATIONELEMENT_H_
#define WIFIINFORMATIONELEMENT_H_

#include "ns3/header.h"
#include "ns3/ref-count-base.h"
#include "ns3/test.h"
#include "ns3/packet.h"

namespace ns3 {
class Packet;
/**
 * \ingroup mesh
 *
 * \brief Enum of all known information element id (aka tags).
 *
 * For now only 802.11s (mesh) related elements are supported here (so 11S prefix),
 * but this can change in future.
 *
 * Note that 802.11s element ids are not yet officially assigned, we use ones
 * compatible with open80211s (http://o11s.org/) implementation.
 */
enum WifiElementId {
  /* begin of open80211s-compatible IDs */
  IE11S_MESH_CONFIGURATION              = 51,
  IE11S_MESH_ID                         = 52,
  /* end of open80211s-compatible IDs */
  IE11S_LINK_METRIC_REPORT              = 20,
  IE11S_CONGESTION_NOTIFICATION,
  /* begin of open80211s-compatible IDs */
  IE11S_PEERING_MANAGEMENT              = 55,
  /* end of open80211s-compatible IDs */
  IE11S_SUPP_MBSS_REG_CLASSES_CHANNELS  = 23,
  IE11S_MESH_CHANNEL_SWITCH_ANNOUNCEMENT,
  IE11S_MESH_TIM,
  IE11S_AWAKE_WINDOW,
  IE11S_BEACON_TIMING,
  IE11S_MCCAOP_SETUP_REQUEST,
  IE11S_MCCAOP_SETUP_REPLY,
  IE11S_MCCAOP_ADVERTISEMENT,
  IE11S_MCCAOP_RESERVATION_TEARDOWN,
  IE11S_PORTAL_ANNOUNCEMENT,
  IE11S_RANN                            = 67,
  /* begin of open80211s-compatible IDs */
  IE11S_PREQ                            = 68,
  IE11S_PREP                            = 69,
  IE11S_PERR                            = 70,
  /* end of open80211s-compatible IDs */
  IE11S_PROXY_UPDATE                    = 37,
  IE11S_PROXY_UPDATE_CONFIRMATION,
  IE11S_ABBREVIATED_HANDSHAKE,
  IE11S_MESH_PEERING_PROTOCOL_VERSION   = 74,
};

/**
 * \ingroup mesh
 *
 * \brief Information element, as defined in 802.11-2007 standard
 *
 * Elements are defined to have a common general format consisting of a 1 octet Element ID field, a 1 octet
 * length field, and a variable-length element-specific information field. Each element is assigned a unique
 * Element ID as defined in this standard. The Length field specifies the number of octets in the Information
 * field.
 */
class WifiInformationElement : public RefCountBase
{
public:
  virtual void Print (std::ostream &os) const;
  ///\name Each subclass must implement
  //\{
  /// Own unique Element ID
  virtual WifiElementId ElementId () const = 0;
  /// Length of serialized information
  virtual uint8_t GetInformationSize () const = 0;
  /// Serialize information
  virtual void SerializeInformation (Buffer::Iterator start) const = 0;
  /// Deserialize information
  virtual uint8_t DeserializeInformation (Buffer::Iterator start, uint8_t length) = 0;
  /// Print information
  virtual void PrintInformation (std::ostream &os) const = 0;
  //\}

  /// Compare information elements using Element ID
  friend bool operator< (WifiInformationElement const & a, WifiInformationElement const & b);
};

/// Compare information elements using Element ID
bool operator< (WifiInformationElement const & a, WifiInformationElement const & b);

#ifdef RUN_SELF_TESTS
/// Generic test of information element
class IeTest : public Test
{
public:
  IeTest (const char * name) : Test (name) {}
  /// Test round-trip serialization
  template <typename IE> bool TestRoundtripSerialization (IE a);
};

template <typename IE> bool
IeTest::TestRoundtripSerialization (IE a)
{
  bool result (true);
#if 0
  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (a);
  IE b;
  packet->RemoveHeader (b);
  NS_TEST_ASSERT_EQUAL (a, b);
  packet->AddHeader (a);
  IE c;
  bool ok = packet->RemoveHeader (c);
  NS_TEST_ASSERT (ok);
  NS_TEST_ASSERT_EQUAL (a, c);
#endif
  return result;
}
#endif

}  // namespace ns3
#endif /* WIFIINFORMATIONELEMENT_H_ */
