/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 Emmanuelle Laprise
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
 * Author: Emmanuelle Laprise <emmanuelle.laprise@bluekazoo.ca>
 */

#ifndef ETHERNET_HEADER_H
#define ETHERNET_HEADER_H

#include "ns3/header.h"
#include "ns3/mac-address.h"

namespace ns3 {

  /**
   * Types of ethernet packets. Indicates the type of the current
   * header.
   */
  enum ethernet_header_t {
    LENGTH,   /**< Basic ethernet packet, no tags, type/length field
                 indicates packet length or IP/ARP packet */
    VLAN,     /**< Single tagged packet. Header includes VLAN tag */
    QINQ      /**< Double tagged packet. Header includes two VLAN tags */
  };
/**
 * \brief Packet header for Ethernet
 *
 * This class can be used to add a header to an ethernet packet that
 * will specify the source and destination addresses and the length of
 * the packet. Eventually the class will be improved to also support
 * VLAN tags in packet headers.
 */
class EthernetHeader : public Header {
public:
  static const int PREAMBLE_SIZE = 8; /// size of the preamble_sfd header field
  static const int LENGTH_SIZE = 2;   /// size of the length_type header field
  static const int MAC_ADDR_SIZE = 6; /// size of src/dest addr header fields

  /**
   * \brief Construct a null ethernet header
   */
  EthernetHeader ();
  virtual ~EthernetHeader ();
  /**
   * \brief Enable or disabled the serialisation of the preamble and
   * Sfd header fields
   */
  static void EnablePreambleSfd (bool enable);
  /**
   * \param size The size of the payload in bytes
   */
  void SetLengthType (uint16_t size);
  /**
   * \param source The source address of this packet
   */
  void SetSource (MacAddress source);
  /**
   * \param destination The destination address of this packet.
   */
  void SetDestination (MacAddress destination);
  /**
   * \param preambleSfd The value that the preambleSfd field should take
   */
  void SetPreambleSfd (uint64_t preambleSfd);
  /**
   * \return The size of the payload in bytes
   */
  uint16_t GetLengthType (void) const;
  /**
   * \return The type of packet (only basic Ethernet is currently supported)
   */
  ethernet_header_t GetPacketType (void) const;
  /**
   * \return The source address of this packet
   */
  MacAddress GetSource (void) const;
  /**
   * \return The destination address of this packet
   */
  MacAddress GetDestination (void) const;  
  /**
   * \return The value of the PreambleSfd field
   */
  uint64_t GetPreambleSfd () const;
  /**
   * \return The size of the header
   */
  uint32_t GetHeaderSize() const;

private:
  virtual std::string DoGetName (void) const;
  virtual void PrintTo (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void SerializeTo (Buffer::Iterator start) const;
  virtual uint32_t DeserializeFrom (Buffer::Iterator start);

  void Init (void);

  /**
   * If false, the preamble/sfd are not serialised/deserialised.
   */
  static bool m_enPreambleSfd;

  uint64_t m_preambleSfd;     /// Value of the Preamble/SFD fields
  uint16_t m_lengthType : 16; /// Length or type of the packet
  MacAddress m_source;        /// Source address
  MacAddress m_destination;   /// Destination address
};

}; // namespace ns3


#endif /* ETHERNET_HEADER_H */
