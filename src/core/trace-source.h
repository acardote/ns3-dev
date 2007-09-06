/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INRIA
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
#ifndef TRACE_SOURCE_H
#define TRACE_SOURCE_H

#include <ostream>

namespace ns3 {

class CallbackBase;
class TraceContext;

/**
 * \brief the base class for all trace sources
 *
 * Every trace source which wishes to be connectable and disconnectable with
 * the TraceResolver system should derive from this base class and implement
 * all three methods below.
 */
class TraceSource
{
public:
  virtual ~TraceSource () {}
  /**
   * \param callback the callback to connect to this trace source
   * \param context the context associated to the input callback which should be passed
   *        back to the user.
   */
  virtual void AddCallback (CallbackBase const & callback, TraceContext const & context) = 0;
  /**
   * \param callback the callback to connect to this trace source
   */
  void AddCallback (CallbackBase const & callback);
  /**
   * \param callback the callback to disconnect from this trace source
   */
  virtual void RemoveCallback (CallbackBase const & callback) = 0;
  virtual void ConnectPrinter (std::ostream &os, TraceContext const &context) = 0;
};

} // namespace ns3

#endif /* TRACE_SOURCE_H */
