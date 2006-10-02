/* -*-    Mode:C++; c-basic-offset:4; tab-width:4; indent-tabs-mode:nil -*- */
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

#include "event-impl.h"


namespace ns3 {


EventImpl::~EventImpl ()
{}

EventImpl::EventImpl ()
    : m_internalIterator (0),
      m_cancel (false)
{}
void 
EventImpl::invoke (void)
{
    if (!m_cancel) {
        notify ();
    }
}
void 
EventImpl::setInternalIterator (void *tag)
{
    m_internalIterator = tag;
}
void *
EventImpl::getInternalIterator (void) const
{
    return m_internalIterator;
}
void 
EventImpl::cancel (void)
{
    m_cancel = true;
}

}; // namespace ns3
