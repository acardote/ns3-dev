/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
#ifndef TAG_H
#define TAG_H

#include "ns3/object-base.h"
#include "tag-buffer.h"
#include <stdint.h>

namespace ns3 {

/**
 * \brief tag a set of bytes in a packet
 *
 * New kinds of tags can be created by subclassing this base class.
 */
class Tag : public ObjectBase
{
public:
  static TypeId GetTypeId (void);

  /**
   * \returns the number of bytes required to serialize the data of the tag.
   *
   * This method is typically invoked by Packet::AddTag just prior to calling
   * Tag::Serialize.
   */
  virtual uint32_t GetSerializedSize (void) const = 0;
  /**
   * \param i the buffer to write data into.
   *
   * Write the content of the tag in the provided tag buffer.
   */
  virtual void Serialize (TagBuffer i) const = 0;
  /**
   * \param i the buffer to read data from.
   *
   * Read the content of the tag from the provided tag buffer.
   */
  virtual void Deserialize (TagBuffer i) = 0;
};

} // namespace ns3

#endif /* TAG_H */
