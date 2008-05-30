/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
#include "packet-metadata.h"
#include "tag.h"
#include "tag-list.h"
#include "ns3/callback.h"
#include "ns3/assert.h"
#include "ns3/ptr.h"

namespace ns3 {

/**
 * \brief Iterator over the set of tags in a packet
 *
 * This is a java-style iterator.
 */
class TagIterator
{
public:
  /**
   * Identifies a set tag and a set of bytes within a packet
   * to which the tag applies.
   */
  class Item
  {
  public:
    /**
     * \returns the ns3::TypeId associated to this tag.
     */
    TypeId GetTypeId (void) const;
    /**
     * \returns the index of the first byte tagged by this tag.
     *
     * The index is an offset from the start of the packet.
     */
    uint32_t GetStart (void) const;
    /**
     * \returns the index of the last byte tagged by this tag.
     *
     * The index is an offset from the start of the packet.
     */
    uint32_t GetEnd (void) const;
    /**
     * \param tag the user tag to which the data should be copied.
     *
     * Read the requested tag and store it in the user-provided
     * tag instance. This method will crash if the type of the
     * tag provided by the user does not match the type of
     * the underlying tag.
     */
    void GetTag (Tag &tag) const;
  private:
    friend class TagIterator;
    Item (TypeId tid, uint32_t start, uint32_t end, TagBuffer buffer);
    TypeId m_tid;
    uint32_t m_start;
    uint32_t m_end;
    TagBuffer m_buffer;
  };
  /**
   * \returns true if calling Next is safe, false otherwise.
   */
  bool HasNext (void) const;
  /**
   * \returns the next item found and prepare for the next one.
   */
  Item Next (void);
private:
  friend class Packet;
  TagIterator (TagList::Iterator i);
  TagList::Iterator m_current;
};

/**
 * \ingroup common
 * \defgroup packet Packet
 *
 * \brief network packets
 *
 * Each network packet contains a byte buffer, a set of tags, and
 * metadata.
 *
 * - The byte buffer stores the serialized content of the headers and trailers 
 * added to a packet. The serialized representation of these headers is expected
 * to match that of real network packets bit for bit (although nothing
 * forces you to do this) which means that the content of a packet buffer
 * is expected to be that of a real packet.
 *
 * - Each tag tags a subset of the bytes in the packet byte buffer with the 
 * information stored in the tag. A classic example of a tag is a FlowIdTag 
 * which contains a flow id: the set of bytes tagged by this tag implicitely 
 * belong to the attached flow id.
 *
 * - The metadata describes the type of the headers and trailers which
 * were serialized in the byte buffer. The maintenance of metadata is
 * optional and disabled by default. To enable it, you must call
 * Packet::EnableMetadata and this will allow you to get non-empty
 * output from Packet::Print and Packet::Print.
 *
 * Implementing a new type of Header or Trailer for a new protocol is 
 * pretty easy and is a matter of creating a subclass of the ns3::Header 
 * or of the ns3::Trailer base class, and implementing the methods
 * described in their respective API documentation.
 *
 * Implementing a new type of Tag requires roughly the same amount of
 * work and this work is described in the ns3::Tag API documentation.
 *
 * The performance aspects of the Packet API are discussed in 
 * \ref packetperf
 */
class Packet 
{
public:
  void Ref (void) const;
  void Unref (void) const;

  Ptr<Packet> Copy (void) const;

  /**
   * Create an empty packet with a new uid (as returned
   * by getUid).
   */
  Packet ();
  Packet (const Packet &o);
  Packet &operator = (const Packet &o);  
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
   * Create a packet with payload filled with the content
   * of this buffer. The input data is copied: the input
   * buffer is untouched.
   *
   * \param buffer the data to store in the packet.
   * \param size the size of the input buffer.
   */
  Packet (uint8_t const*buffer, uint32_t size);
  /**
   * Create a new packet which contains a fragment of the original
   * packet. The returned packet shares the same uid as this packet.
   *
   * \param start offset from start of packet to start of fragment to create
   * \param length length of fragment to create
   * \returns a fragment of the original packet
   */
  Ptr<Packet> CreateFragment (uint32_t start, uint32_t length) const;
  /**
   * \returns the size in bytes of the packet (including the zero-filled
   *          initial payload)
   */
  uint32_t GetSize (void) const;
  /**
   * Add header to this packet. This method invokes the
   * GetSerializedSize and Serialize
   * methods to reserve space in the buffer and request the 
   * header to serialize itself in the packet buffer.
   *
   * \param header a reference to the header to add to this packet.
   */
  void AddHeader (const Header & header);
  /**
   * Deserialize and remove the header from the internal buffer.
   * This method invokes Deserialize.
   *
   * \param header a reference to the header to remove from the internal buffer.
   * \returns the number of bytes removed from the packet.
   */
  uint32_t RemoveHeader (Header &header);
  /**
   * Add trailer to this packet. This method invokes the
   * GetSerializedSize and Serialize
   * methods to reserve space in the buffer and request the trailer 
   * to serialize itself in the packet buffer.
   *
   * \param trailer a reference to the trailer to add to this packet.
   */
  void AddTrailer (const Trailer &trailer);
  /**
   * Remove a deserialized trailer from the internal buffer.
   * This method invokes the Deserialize method.
   *
   * \param trailer a reference to the trailer to remove from the internal buffer.
   * \returns the number of bytes removed from the end of the packet.
   */
  uint32_t RemoveTrailer (Trailer &trailer);
  /**
   * \param os output stream in which the data should be printed.
   *
   * Iterate over the tags present in this packet, and
   * invoke the Print method of each tag stored in the packet.
   */
  void PrintTags (std::ostream &os) const;

  /**
   * Concatenate the input packet at the end of the current
   * packet. This does not alter the uid of either packet.
   *
   * \param packet packet to concatenate
   */
  void AddAtEnd (Ptr<const Packet> packet);
  /**
   * \param size number of padding bytes to add.
   */
  void AddPaddingAtEnd (uint32_t size);
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
   * Note: This uid is an internal uid and cannot be counted on to
   * provide an accurate counter of how many "simulated packets" of a
   * particular protocol are in the system. It is not trivial to make
   * this uid into such a counter, because of questions such as what
   * should the uid be when the packet is sent over broadcast media, or
   * when fragmentation occurs. If a user wants to trace actual packet
   * counts, he or she should look at e.g. the IP ID field or transport
   * sequence numbers, or other packet or frame counters at other
   * protocol layers.
   *
   * \returns an integer identifier which uniquely
   *          identifies this packet.
   */
  uint32_t GetUid (void) const;

  /**
   * \param os output stream in which the data should be printed.
   *
   * Iterate over the headers and trailers present in this packet, 
   * from the first header to the last trailer and invoke, for
   * each of them, the user-provided method Header::DoPrint or 
   * Trailer::DoPrint methods.
   */
  void Print (std::ostream &os) const;

  PacketMetadata::ItemIterator BeginItem (void) const;

  /**
   * By default, packets do not keep around enough metadata to
   * perform the operations requested by the Print methods. If you
   * want to be able to invoke any of the two ::Print methods, 
   * you need to invoke this method at least once during the 
   * simulation setup and before any packet is created.
   *
   * The packet metadata is also used to perform extensive
   * sanity checks at runtime when performing operations on a 
   * Packet. For example, this metadata is used to verify that
   * when you remove a header from a packet, this same header
   * was actually present at the front of the packet. These
   * errors will be detected and will abort the program.
   */
  static void EnableMetadata (void);

  /**
   * \returns a byte buffer
   *
   * This method creates a serialized representation of a Packet object
   * ready to be transmitted over a network to another system. This
   * serialized representation contains a copy of the packet byte buffer,
   * the tag list, and the packet metadata (if there is one).
   *
   * This method will trigger calls to the Serialize and GetSerializedSize
   * methods of each tag stored in this packet.
   *
   * This method will typically be used by parallel simulations where
   * the simulated system is partitioned and each partition runs on
   * a different CPU.
   */
  Buffer Serialize (void) const;
  /**
   * \param buffer a byte buffer
   *
   * This method reads a byte buffer as created by Packet::Serialize
   * and restores the state of the Packet to what it was prior to
   * calling Serialize.
   *
   * This method will trigger calls to the Deserialize method
   * of each tag stored in this packet.
   *
   * This method will typically be used by parallel simulations where
   * the simulated system is partitioned and each partition runs on
   * a different CPU.
   */
  void Deserialize (Buffer buffer);

  /**
   * \param tag the new tag to add to this packet
   *
   * Tag each byte included in this packet with the
   * new tag.
   *
   * Note that adding a tag is a const operation which is pretty 
   * un-intuitive. The rationale is that the content and behavior of
   * a packet is _not_ changed when a tag is added to a packet: any
   * code which was not aware of the new tag is going to work just
   * the same if the new tag is added. The real reason why adding a
   * tag was made a const operation is to allow a trace sink which gets
   * a packet to tag the packet, even if the packet is const (and most
   * trace sources should use const packets because it would be
   * totally evil to allow a trace sink to modify the content of a
   * packet).
   */
  void AddTag (const Tag &tag) const;
  /**
   * \returns an iterator over the set of tags included in this packet.
   */
  TagIterator GetTagIterator (void) const;
  /**
   * \param tag the tag to search in this packet
   * \returns true if the requested tag type was found, false otherwise.
   *
   * If the requested tag type is found, it is copied in the user's 
   * provided tag instance.
   */
  bool FindFirstMatchingTag (Tag &tag) const;

  /**
   * Remove all the tags stored in this packet.
   */
  void RemoveAllTags (void);

private:
  Packet (const Buffer &buffer, const TagList &tagList, const PacketMetadata &metadata);
  Buffer m_buffer;
  TagList m_tagList;
  PacketMetadata m_metadata;
  mutable uint32_t m_refCount;
  static uint32_t m_globalUid;
};

std::ostream& operator<< (std::ostream& os, const Packet &packet);

/**
 * \ingroup common
 * \defgroup packetperf Packet Performance
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
 *   - ns3::Packet::AddHeader
 *   - ns3::Packet::AddTrailer
 *   - both versions of ns3::Packet::AddAtEnd
 *
 * Non-dirty operations:
 *   - ns3::Packet::AddTag
 *   - ns3::Packet::RemoveAllTags
 *   - ns3::Packet::PeekTag
 *   - ns3::Packet::RemoveHeader
 *   - ns3::Packet::RemoveTrailer
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

} // namespace ns3

#endif /* PACKET_H */
