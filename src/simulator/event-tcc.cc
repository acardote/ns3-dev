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
#include "event-impl.h"
#include "event.h"

namespace ns3 {

class EventFunctionImpl0 : public EventImpl {
public:
	typedef void (*F)(void);

	EventFunctionImpl0 (F function) 
		: m_function (function)
	{}
	virtual ~EventFunctionImpl0 () {}
private:
	virtual void notify (void) { 
		(*m_function) (); 
	}
private:
	F m_function;
};

Event make_event(void (*f) (void)) 
{
	return Event (new EventFunctionImpl0 (f));
}

}; // namespace ns3
