/* -*-    Mode:C++; c-basic-offset:4; tab-width:4; indent-tabs-mode:f -*- */
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
#include "packet.h"
#include <cassert>

namespace ns3 {

uint32_t Packet::m_global_uid = 0;

Packet::Packet ()
    : m_buffer (),
	  m_uid (m_global_uid)
{
	m_global_uid++;
}

Packet::Packet (uint32_t size)
    : m_buffer (size),
	  m_uid (m_global_uid)
{
	m_global_uid++;
}
Packet::Packet (Buffer buffer, Tags tags, uint32_t uid)
    : m_buffer (buffer),
      m_tags (tags),
	  m_uid (uid)
{}

Packet 
Packet::createFragment (uint32_t start, uint32_t length) const
{
    Buffer tmp = m_buffer.createFragment (start, length);
    return Packet (tmp, m_tags, m_uid);
}

uint32_t 
Packet::getSize (void) const
{
    return m_buffer.getSize ();
}

void 
Packet::add (Header const &header)
{
	m_buffer.addAtStart (header.getSize ());
	header.serialize (m_buffer.begin ());
}
void 
Packet::peek (Header &header)
{
	header.deserialize (m_buffer.begin ());
}
void 
Packet::remove (Header const &header)
{
	assert (header.isDeserialized ());
	m_buffer.removeAtStart (header.getSize ());
}


void 
Packet::addAtEnd (Packet packet)
{
    Buffer src = packet.m_buffer;
    m_buffer.addAtEnd (src.getSize ());
    Buffer::Iterator destStart = m_buffer.end ();
    destStart.prev (src.getSize ());
    destStart.write (src.begin (), src.end ());
    /**
     * XXX: we might need to merge the tag list of the
     * other packet into the current packet.
     */
}
void 
Packet::addAtEnd (Packet packet, uint32_t start, uint32_t size)
{
    assert (packet.getSize () <= start + size);
    Buffer src = packet.m_buffer;
    m_buffer.addAtEnd (src.getSize ());
    Buffer::Iterator destStart = m_buffer.end ();
    destStart.prev (size);
    Buffer::Iterator srcStart = src.begin ();
    srcStart.next (start);
    Buffer::Iterator srcEnd = srcStart;
    srcEnd.next (size);
    destStart.write (srcStart, srcEnd);
    /**
     * XXX: we might need to merge the tag list of the
     * other packet into the current packet.
     */
}
void 
Packet::removeAtEnd (uint32_t size)
{
    m_buffer.removeAtEnd (size);
}
void 
Packet::removeAtStart (uint32_t size)
{
    m_buffer.removeAtStart (size);
}

void 
Packet::removeAllTags (void)
{
    m_tags.removeAll ();
}

uint8_t const *
Packet::peekData (void) const
{
	return m_buffer.peekData ();
}

uint32_t 
Packet::getUid (void) const
{
	return m_uid;
}

}; // namespace ns3
