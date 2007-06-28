/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006,2007 INRIA
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
#ifndef PACKET_METADATA_H
#define PACKET_METADATA_H

#include <stdint.h>
#include <vector>
#include "ns3/callback.h"
#include "ns3/assert.h"
#include "packet-printer.h"

namespace ns3 {

class Chunk;
class Buffer;

class PacketMetadata {
public:
  static void Enable (void);
  static void SetOptOne (bool optOne);

  inline PacketMetadata (uint32_t uid, uint32_t size);
  inline PacketMetadata (PacketMetadata const &o);
  inline PacketMetadata &operator = (PacketMetadata const& o);
  inline ~PacketMetadata ();

  template <typename T>
  void AddHeader (T const &header, uint32_t size);
  template <typename T>
  void RemoveHeader (T const &header, uint32_t size);

  template <typename T>
  void AddTrailer (T const &trailer, uint32_t size);
  template <typename T>
  void RemoveTrailer (T const &trailer, uint32_t size);

  PacketMetadata CreateFragment (uint32_t start, uint32_t end) const;
  void AddAtEnd (PacketMetadata const&o);
  void AddPaddingAtEnd (uint32_t end);
  void RemoveAtStart (uint32_t start);
  void RemoveAtEnd (uint32_t end);

  uint32_t GetUid (void) const;

  void PrintDefault (std::ostream &os, Buffer buffer) const;
  void Print (std::ostream &os, Buffer buffer, PacketPrinter const &printer) const;

  static void PrintStats (void);

private:
  struct Data {
    /* number of references to this struct Data instance. */
    uint16_t m_count;
    /* size (in bytes) of m_data buffer below */
    uint16_t m_size;
    /* max of the m_used field over all objects which 
     * reference this struct Data instance */
    uint16_t m_dirtyEnd;
    /* variable-sized buffer of bytes */
    uint8_t m_data[10];
  };
  /* Note that since the next and prev fields are 16 bit integers
     and since the value 0xffff is reserved to identify the 
     fact that the end or the start of the list is reached,
     only a limited number of elements can be stored in 
     a m_data byte buffer.
   */
  struct SmallItem {
    /* offset (in bytes) from start of m_data buffer 
       to next element in linked list. value is 0xffff 
       if next element does not exist.
    */
    uint16_t next;
    /* offset (in bytes) from start of m_data buffer 
       to previous element in linked list. value is 0xffff 
       if previous element does not exist.
     */
    uint16_t prev;
    /* the high 31 bits of this field identify the 
       type of the header or trailer represented by 
       this item: the value zero represents payload.
       If the low bit of this uid is one, an ExtraItem
       structure follows this SmallItem structure.
     */
    uint32_t typeUid;
    /* the size (in bytes) of the header or trailer represented
       by this element.
     */
    uint32_t size;
    /* this field tries to uniquely identify each header or 
       trailer _instance_ while the typeUid field uniquely
       identifies each header or trailer _type_. This field
       is used to test whether two items are equal in the sense 
       that they represent the same header or trailer instance.
       That equality test is based on the typeUid and chunkUid
       fields so, the likelyhood that two header instances 
       share the same chunkUid _and_ typeUid is very small 
       unless they are really representations of the same header
       instance.
     */
    uint16_t chunkUid;
  };
  struct ExtraItem {
    /* offset (in bytes) from start of original header to 
       the start of the fragment still present.
     */
    uint32_t fragmentStart;
    /* offset (in bytes) from start of original header to 
       the end of the fragment still present.
     */
    uint32_t fragmentEnd;
    /* the packetUid of the packet in which this header or trailer
       was first added. It could be different from the m_packetUid
       field if the user has aggregated multiple packets into one.
     */
    uint32_t packetUid;
  };

  typedef std::vector<struct Data *> DataFreeList;
  
  PacketMetadata ();
  void DoAddHeader (uint32_t uid, uint32_t size);
  void DoRemoveHeader (uint32_t uid, uint32_t size);
  void DoAddTrailer (uint32_t uid, uint32_t size);
  void DoRemoveTrailer (uint32_t uid, uint32_t size);

  inline uint16_t AddSmall (const PacketMetadata::SmallItem *item);
  uint16_t AddBig (uint32_t head, uint32_t tail,
                   const PacketMetadata::SmallItem *item, 
                   const PacketMetadata::ExtraItem *extraItem);
  void ReplaceTail (PacketMetadata::SmallItem *item, 
                    PacketMetadata::ExtraItem *extraItem,
                    uint32_t available);
  inline void UpdateHead (uint16_t written);
  inline void UpdateTail (uint16_t written);
  uint32_t GetUleb128Size (uint32_t value) const;
  uint32_t ReadUleb128 (const uint8_t **pBuffer) const;
  inline void Append16 (uint16_t value, uint8_t *buffer);
  inline bool TryToAppend (uint32_t value, uint8_t **pBuffer, uint8_t *end);
  inline bool TryToAppendFast (uint32_t value, uint8_t **pBuffer, uint8_t *end);
  inline bool TryToAppend32 (uint32_t value, uint8_t **pBuffer, uint8_t *end);
  inline bool TryToAppend16 (uint16_t value, uint8_t **pBuffer, uint8_t *end);
  void AppendValue (uint32_t value, uint8_t *buffer);
  void AppendValueExtra (uint32_t value, uint8_t *buffer);
  inline void Reserve (uint32_t n);
  void ReserveCopy (uint32_t n);
  uint32_t DoPrint (struct PacketMetadata::SmallItem *item, uint32_t current,
                    Buffer data, uint32_t offset, const PacketPrinter &printer,
                    std::ostream &os) const;
  uint32_t GetTotalSize (void) const;
  uint32_t ReadItems (uint16_t current, 
                      struct PacketMetadata::SmallItem *item,
                      struct PacketMetadata::ExtraItem *extraItem) const;


  static struct PacketMetadata::Data *Create (uint32_t size);
  static void Recycle (struct PacketMetadata::Data *data);
  static struct PacketMetadata::Data *Allocate (uint32_t n);
  static void Deallocate (struct PacketMetadata::Data *data);
  
  static DataFreeList m_freeList;
  static bool m_enable;
  static uint32_t m_maxSize;
  static uint16_t m_chunkUid;
  
  struct Data *m_data;
  /**
     head -(next)-> tail
       ^             |
        \---(prev)---|
   */
  uint16_t m_head;
  uint16_t m_tail;
  uint16_t m_used;
  uint32_t m_packetUid;
};

}; // namespace ns3

namespace ns3 {

template <typename T>
void 
PacketMetadata::AddHeader (T const &header, uint32_t size)
{
  DoAddHeader (PacketPrinter::GetHeaderUid<T> (), size);
}

template <typename T>
void 
PacketMetadata::RemoveHeader (T const &header, uint32_t size)
{
  DoRemoveHeader (PacketPrinter::GetHeaderUid<T> (), size);
}
template <typename T>
void 
PacketMetadata::AddTrailer (T const &trailer, uint32_t size)
{
  DoAddTrailer (PacketPrinter::GetTrailerUid<T> (), size);
}
template <typename T>
void 
PacketMetadata::RemoveTrailer (T const &trailer, uint32_t size)
{
  DoRemoveTrailer (PacketPrinter::GetTrailerUid<T> (), size);
}


PacketMetadata::PacketMetadata (uint32_t uid, uint32_t size)
  : m_data (m_data = PacketMetadata::Create (10)),
    m_head (0xffff),
    m_tail (0xffff),
    m_used (0),
    m_packetUid (uid)
{
  memset (m_data->m_data, 0xff, 4);
  if (size > 0)
    {
      DoAddHeader (0, size);
    }
}
PacketMetadata::PacketMetadata (PacketMetadata const &o)
  : m_data (o.m_data),
    m_head (o.m_head),
    m_tail (o.m_tail),
    m_used (o.m_used),
    m_packetUid (o.m_packetUid)
{
  NS_ASSERT (m_data != 0);
  m_data->m_count++;
}
PacketMetadata &
PacketMetadata::operator = (PacketMetadata const& o)
{
  if (m_data == o.m_data) 
    {
      // self assignment
      return *this;
    }
  NS_ASSERT (m_data != 0);
  m_data->m_count--;
  if (m_data->m_count == 0) 
    {
      PacketMetadata::Recycle (m_data);
    }
  m_data = o.m_data;
  m_head = o.m_head;
  m_tail = o.m_tail;
  m_used = o.m_used;
  m_packetUid = o.m_packetUid;
  NS_ASSERT (m_data != 0);
  m_data->m_count++;
  return *this;
}
PacketMetadata::~PacketMetadata ()
{
  NS_ASSERT (m_data != 0);
  m_data->m_count--;
  if (m_data->m_count == 0) 
    {
      PacketMetadata::Recycle (m_data);
    }
}

}; // namespace ns3


#endif /* PACKET_METADATA_H */
