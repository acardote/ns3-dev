/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006,2007 INRIA
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

#ifndef TRACED_CALLBACK_H
#define TRACED_CALLBACK_H

#include <list>
#include "callback.h"

namespace ns3 {

/**
 * \brief log arbitrary number of parameters to a matching ns3::Callback
 * \ingroup tracing
 *
 * Whenever operator () is invoked on this class, the call and its arguments
 * are forwarded to the internal matching ns3::Callback.
 */
template<typename T1 = empty, typename T2 = empty, 
         typename T3 = empty, typename T4 = empty>
class TracedCallback 
{
public:
  TracedCallback ();
  void Connect (const CallbackBase & callback);
  void ConnectWithContext (const CallbackBase & callback, std::string path);
  void Disconnect (const CallbackBase & callback);
  void DisconnectWithContext (const CallbackBase & callback, std::string path);
  void operator() (void) const;
  void operator() (T1 a1) const;
  void operator() (T1 a1, T2 a2) const;
  void operator() (T1 a1, T2 a2, T3 a3) const;
  void operator() (T1 a1, T2 a2, T3 a3, T4 a4) const;

private:  
  typedef std::list<Callback<void,T1,T2,T3,T4> > CallbackList;
  CallbackList m_callbackList;
};

} // namespace ns3

// implementation below.

namespace ns3 {

template<typename T1, typename T2, 
         typename T3, typename T4>
TracedCallback<T1,T2,T3,T4>::TracedCallback ()
  : m_callbackList () 
{}
template<typename T1, typename T2, 
         typename T3, typename T4>
void 
TracedCallback<T1,T2,T3,T4>::Connect (const CallbackBase & callback)
{
  Callback<void,T1,T2,T3,T4> cb;
  cb.Assign (callback);
  m_callbackList.push_back (cb);
}
template<typename T1, typename T2, 
         typename T3, typename T4>
void 
TracedCallback<T1,T2,T3,T4>::ConnectWithContext (const CallbackBase & callback, std::string path)
{
  Callback<void,std::string,T1,T2,T3,T4> cb;
  cb.Assign (callback);
  Callback<void,T1,T2,T3,T4> realCb = cb.Bind (path);
  m_callbackList.push_back (realCb);
}
template<typename T1, typename T2, 
         typename T3, typename T4>
void 
TracedCallback<T1,T2,T3,T4>::Disconnect (const CallbackBase & callback)
{
  for (typename CallbackList::iterator i = m_callbackList.begin ();
       i != m_callbackList.end (); /* empty */)
    {
      if ((*i).IsEqual (callback))
	{
	  i = m_callbackList.erase (i);
	}
      else
	{
	  i++;
	}
    }
}
template<typename T1, typename T2, 
         typename T3, typename T4>
void 
TracedCallback<T1,T2,T3,T4>::DisconnectWithContext (const CallbackBase & callback, std::string path)
{
  Callback<void,std::string,T1,T2,T3,T4> cb;
  cb.Assign (callback);
  Callback<void,T1,T2,T3,T4> realCb = cb.Bind (path);
  Disconnect (realCb);
}
template<typename T1, typename T2, 
         typename T3, typename T4>
void 
TracedCallback<T1,T2,T3,T4>::operator() (void) const
{
  for (typename CallbackList::const_iterator i = m_callbackList.begin ();
       i != m_callbackList.end (); i++)
    {
      (*i) ();
    }
}
template<typename T1, typename T2, 
         typename T3, typename T4>
void 
TracedCallback<T1,T2,T3,T4>::operator() (T1 a1) const
{
  for (typename CallbackList::const_iterator i = m_callbackList.begin ();
       i != m_callbackList.end (); i++)
    {
      (*i) (a1);
    }
}
template<typename T1, typename T2, 
         typename T3, typename T4>
void 
TracedCallback<T1,T2,T3,T4>::operator() (T1 a1, T2 a2) const
{
  for (typename CallbackList::const_iterator i = m_callbackList.begin ();
       i != m_callbackList.end (); i++)
    {
      (*i) (a1, a2);
    }
}
template<typename T1, typename T2, 
         typename T3, typename T4>
void 
TracedCallback<T1,T2,T3,T4>::operator() (T1 a1, T2 a2, T3 a3) const
{
  for (typename CallbackList::const_iterator i = m_callbackList.begin ();
       i != m_callbackList.end (); i++)
    {
      (*i) (a1, a2, a3);
    }
}
template<typename T1, typename T2, 
         typename T3, typename T4>
void 
TracedCallback<T1,T2,T3,T4>::operator() (T1 a1, T2 a2, T3 a3, T4 a4) const
{
  for (typename CallbackList::const_iterator i = m_callbackList.begin ();
       i != m_callbackList.end (); i++)
    {
      (*i) (a1, a2, a3, a4);
    }
}

}//namespace ns3

#endif /* TRACED_CALLBACK_H */
