/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
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

#include "chunk.h"
#include <cassert>

namespace ns3 {

Chunk::Chunk ()
	: m_must_peek_before_remove (false) {}

void 
Chunk::print (std::ostream &os) const
{
	print (&os);
}
void 
Chunk::add (Buffer *buffer) const
{
	add_to (buffer);
}
void 
Chunk::peek (Buffer const *buffer)
{
	peek_from (buffer);
	m_must_peek_before_remove = true;
}
void 
Chunk::remove (Buffer *buffer)
{
	assert (m_must_peek_before_remove);
	remove_from (buffer);
	m_must_peek_before_remove = false;
}



Chunk::~Chunk ()
{}

std::ostream& operator<< (std::ostream& os, Chunk const& chunk)
{
	chunk.print (os);
	return os;
}

}; // namespace ns3
