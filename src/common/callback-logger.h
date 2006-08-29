/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
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

#ifndef CALLBACK_LOGGER_H
#define CALLBACK_LOGGER_H

#include "yans/callback.h"

namespace yans {

class CallbackLoggerBase {};

/**
 * \brief log arbitrary number of parameters to a matching yans:Callback
 *
 * Whenever operator () is invoked on this class, the call and its arguments
 * are forwarded to the internal matching yans::Callback.
 */
template<typename T1 = empty, typename T2 = empty, 
	 typename T3 = empty, typename T4 = empty,
	 typename T5 = empty>
class CallbackLogger : public CallbackLoggerBase{
public:
	CallbackLogger ()
		: m_callback () {}
	void set_callback (Callback<void,T1,T2,T3,T4,T5> callback) {
		m_callback = callback;
	}
	void operator() (void) {
		if (!m_callback.is_null ()) {
			m_callback ();
		}
	}
	void operator() (T1 a1) {
		if (!m_callback.is_null ()) {
			m_callback (a1);
		}
	}
	void operator() (T1 a1, T2 a2) {
		if (!m_callback.is_null ()) {
			m_callback (a1,a2);
		}
	}
	void operator() (T1 a1, T2 a2, T3 a3) {
		if (!m_callback.is_null ()) {
			m_callback (a1,a2,a3);
		}
	}
	void operator() (T1 a1, T2 a2, T3 a3, T4 a4) {
		if (!m_callback.is_null ()) {
			m_callback (a1,a2,a3,a4);
		}
	}
	void operator() (T1 a1, T2 a2, T3 a3, T4 a4,T5 a5) {
		if (!m_callback.is_null ()) {
			m_callback (a1,a2,a3,a4,a5);
		}
	}

private:
	Callback<void,T1,T2,T3,T4,T5> m_callback;
};

}; // namespace yans

#endif /* CALLBACK_LOGGER_H */
