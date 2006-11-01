/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include "buffer.h"
#include "header.h"
#include "trailer.h"
#include "tags.h"
#include "ns3/callback.h"

namespace ns3 {

/**
 * \brief network packets
 *
 * Each network packet contains a byte buffer and a list of tags.
 * - The byte buffer stores the serialized content of the headers and trailers 
 * added to a packet. The serialized representation of these headers is expected
 * to match that of real network packets bit for bit (although nothing
 * forces you to do this) which means that the content of a packet buffer
 * is expected to be that of a real packet.
 * - The list of tags stores an arbitrarily large set of arbitrary 
 * user-provided data structures in the packet: only one instance of
 * each type of data structure is allowed in a list of tags. 
 * These tags typically contain per-packet cross-layer information or 
 * flow identifiers. Each tag stored in the tag list can be at most
 * 16 bytes big. Trying to attach bigger data structures will trigger
 * crashes at runtime.
 *
 * Implementing a new type of Header for a new protocol is pretty easy
 * and is a matter of creating a subclass of the ns3::Header base class,
 * and implementing the 4 pure virtual methods defined in ns3::Header.
 * Sample code which shows how to create such a new Header, how to use
 * it, and how to manipulate tags is shown below:
 * \include samples/main-packet.cc
 *
 * The current implementation of the byte buffers and tag list is based
 * on COW (Copy On Write. An introduction to COW can be found in Scott 
 * Meyer's "More Effective C++", items 17 and 29). What this means is that
 * copying packets without modifying them is very cheap (in terms of cpu
 * and memory usage) and modifying them can be also very cheap. What is 
 * key for proper COW implementations is being
 * able to detect when a given modification of the state of a packet triggers
 * a full copy of the data prior to the modification: COW systems need
 * to detect when an operation is "dirty".
 *
 * Dirty operations:
 *   - ns3::Packet::RemoveTag
 *   - ns3::Packet::Add
 *   - both versions of ns3::Packet::AddAtEnd
 *
 * Non-dirty operations:
 *   - ns3::Packet::AddTag
 *   - ns3::Packet::RemoveAllTags
 *   - ns3::Packet::PeekTag
 *   - ns3::Packet::Peek
 *   - ns3::Packet::Remove
 *   - ns3::Packet::CreateFragment
 *   - ns3::Packet::RemoveAtStart
 *   - ns3::Packet::RemoveAtEnd
 *
 * Dirty operations will always be slower than non-dirty operations,
 * sometimes by several orders of magnitude. However, even the
 * dirty operations have been optimized for common use-cases which
 * means that most of the time, these operations will not trigger
 * data copies and will thus be still very fast.
 */
class Packet {
public:
  /**
   * Create an empty packet with a new uid (as returned
   * by getUid).
   */
  Packet ();
  /**
   * Create a packet with a zero-filled payload.
   * The memory necessary for the payload is not allocated:
   * it will be allocated at any later point if you attempt
   * to fragment this packet or to access the zero-filled
   * bytes. The packet is allocated with a new uid (as 
   * returned by getUid).
   * 
   * \param size the size of the zero-filled payload
   */
  Packet (uint32_t size);
  /**
   * Create a new packet which contains a fragment of the original
   * packet. The returned packet shares the same uid as this packet.
   *
   * \param start offset from start of packet to start of fragment to create
   * \param length length of fragment to create
   * \returns a fragment of the original packet
   */
  Packet CreateFragment (uint32_t start, uint32_t length) const;
  /**
   * \returns the size in bytes of the packet (including the zero-filled
   *          initial payload)
   */
  uint32_t GetSize (void) const;
  /**
   * Add header to this packet. This method invokes the
   * ns3::Header::serializeTo method to request the header to serialize
   * itself in the packet buffer.
   *
   * \param header a reference to the header to add to this packet.
   */
  void Add (Header const &header);
  /**
   * Deserialize header from this packet. This method invokes the
   * ns3::Header::deserializeFrom method to request the header to deserialize
   * itself from the packet buffer. This method does not remove
   * the data from the buffer. It merely reads it.
   *
   * \param header a reference to the header to deserialize from the buffer
   */
  void Peek (Header &header);
  /**
   * Remove a deserialized header from the internal buffer.
   * This method removes the bytes read by Packet::peek from
   * the packet buffer.
   *
   * \param header a reference to the header to remove from the internal buffer.
   */
  void Remove (Header const &header);
  /**
   * Add trailer to this packet. This method invokes the
   * ns3::Trailer::serializeTo method to request the trailer to serialize
   * itself in the packet buffer.
   *
   * \param trailer a reference to the trailer to add to this packet.
   */
  void Add (Trailer const &trailer);
  /**
   * Deserialize trailer from this packet. This method invokes the
   * ns3::Trailer::deserializeFrom method to request the trailer to deserialize
   * itself from the packet buffer. This method does not remove
   * the data from the buffer. It merely reads it.
   *
   * \param trailer a reference to the trailer to deserialize from the buffer
   */
  void Peek (Trailer &trailer);
  /**
   * Remove a deserialized trailer from the internal buffer.
   * This method removes the bytes read by Packet::peek from
   * the packet buffer.
   *
   * \param trailer a reference to the trailer to remove from the internal buffer.
   */
  void Remove (Trailer const &trailer);
  /**
   * Attach a tag to this packet. The tag is fully copied
   * in a packet-specific internal buffer. This operation 
   * is expected to be really fast.
   *
   * \param tag a pointer to the tag to attach to this packet.
   */
  template <typename T>
  void AddTag (T const &tag);
  /**
   * Remove a tag from this packet. The data stored internally
   * for this tag is copied in the input tag if an instance
   * of this tag type is present in the internal buffer. If this
   * tag type is not present, the input tag is not modified. 
   *
   * This operation can be potentially slow and might trigger
   * unexpectedly large memory allocations. It is thus
   * usually a better idea to create a copy of this packet,
   * and invoke removeAllTags on the copy to remove all 
   * tags rather than remove the tags one by one from a packet.
   *
   * \param tag a pointer to the tag to remove from this packet
   * \returns true if an instance of this tag type is stored
   *          in this packet, false otherwise.
   */
  template <typename T>
  bool RemoveTag (T &tag);
  /**
   * Copy a tag stored internally to the input tag. If no instance
   * of this tag is present internally, the input tag is not modified.
   *
   * \param tag a pointer to the tag to read from this packet
   * \returns true if an instance of this tag type is stored
   *          in this packet, false otherwise.
   */
  template <typename T>
  bool PeekTag (T &tag) const;
  /**
   * Remove all the tags stored in this packet. This operation is
   * much much faster than invoking removeTag n times.
   */
  void RemoveAllTags (void);
  /**
   * Concatenate the input packet at the end of the current
   * packet. This does not alter the uid of either packet.
   *
   * \param packet packet to concatenate
   */
  void AddAtEnd (Packet packet);
  /**
   * Concatenate the fragment of the input packet identified
   * by the offset and size parameters at the end of the current
   * packet. This does not alter the uid of either packet.
   *
   * \param packet to concatenate
   * \param offset offset of fragment to copy from the start of the input packet
   * \param size size of fragment of input packet to copy.
   */
  void AddAtEnd (Packet packet, uint32_t offset, uint32_t size);
  /** 
   * Remove size bytes from the end of the current packet
   * It is safe to remove more bytes that what is present in
   * the packet.
   *
   * \param size number of bytes from remove
   */
  void RemoveAtEnd (uint32_t size);
  /** 
   * Remove size bytes from the start of the current packet.
   * It is safe to remove more bytes that what is present in
   * the packet.
   *
   * \param size number of bytes from remove
   */
  void RemoveAtStart (uint32_t size);
  
  /**
   * If you try to change the content of the buffer
   * returned by this method, you will die.
   *
   * \returns a pointer to the internal buffer of the packet.
   */
  uint8_t const *PeekData (void) const;

  /**
   * A packet is allocated a new uid when it is created
   * empty or with zero-filled payload.
   *
   * \returns an integer identifier which uniquely
   *          identifies this packet.
   */
  uint32_t GetUid (void) const;
private:
  Packet (Buffer buffer, Tags tags, uint32_t uid);
  Buffer m_buffer;
  Tags m_tags;
  uint32_t m_uid;
  static uint32_t m_global_uid;
};

}; // namespace ns3


/**************************************************
  Start of implementation of templates defined
  above
 *************************************************/

namespace ns3 {

template <typename T>
void Packet::AddTag (T const& tag)
{
  m_tags.Add (tag);
}
template <typename T>
bool Packet::RemoveTag (T & tag)
{
  return m_tags.Remove (tag);
}
template <typename T>
bool Packet::PeekTag (T & tag) const
{
  return m_tags.Peek (tag);
}
}; // namespace ns3

#endif /* PACKET_H */
