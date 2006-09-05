/* -*-    Mode:C++; c-basic-offset:4; tab-width:4; indent-tabs-mode:f -*- */
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

#include "chunk-constant-data.h"

namespace ns3 {

ChunkConstantData::ChunkConstantData (uint32_t len, uint8_t data)
    : m_len (len), m_data (data)
{}

ChunkConstantData::~ChunkConstantData ()
{}


void 
ChunkConstantData::print (std::ostream *os) const
{
    *os << "(constant data)"
        << " len=" << m_len
        << ", data=" << m_data;
}

void 
ChunkConstantData::addTo (Buffer *buffer) const
{
    buffer->addAtStart (m_len);
#ifndef NDEBUG
    buffer->begin ().writeU8 (m_data, m_len);
#endif
}
void 
ChunkConstantData::peekFrom (Buffer const *buffer)
{
    m_len = buffer->getSize ();
    m_data = buffer->begin ().readU8 ();
}
void 
ChunkConstantData::removeFrom (Buffer *buffer)
{
    buffer->removeAtStart (m_len);
}


}; // namespace ns3
