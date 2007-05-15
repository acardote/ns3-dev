/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#ifndef HEADER_H
#define HEADER_H

#include "chunk.h"

namespace ns3 {

/**
 * \brief Protocol header serialization and deserialization.
 *
 * Every Protocol header which needs to be inserted or removed
 * from a Packet instance must derive from this abstract base class
 * and implement the private pure virtual methods listed below:
 *   - ns3::Header::SerializeTo
 *   - ns3::Header::DeserializeFrom
 *   - ns3::Header::GetSerializedSize
 *   - ns3::Header::PrintTo
 */
class Header : public Chunk {
public:
  virtual ~Header ();
private:
  /**
   * \param os the std output stream in which this 
   *       protocol header must print itself.
   */
  virtual void PrintTo (std::ostream &os) const = 0;

  /**
   * \returns the size of the serialized Header.
   */
  virtual uint32_t GetSerializedSize (void) const = 0;

  /**
   * \param i the buffer iterator in which the protocol header
   *    must serialize itself. This iterator identifies 
   *    the start of the buffer.
   */
  virtual void SerializeTo (Buffer::Iterator i) const = 0;
  /**
   * \param i the buffer iterator from which the protocol header must
   *    deserialize itself. This iterator identifies 
   *    the start of the buffer.
   * \returns the number of bytes read from the buffer
   */
  virtual uint32_t DeserializeFrom (Buffer::Iterator i) = 0;
};

}; // namespace ns3

#endif /* HEADER_H */
