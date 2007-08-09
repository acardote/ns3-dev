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
#ifndef TRACE_RESOLVER_H
#define TRACE_RESOLVER_H

#include <string>
#include <list>
#include "trace-context.h"

namespace ns3 {

class CallbackBase;

/**
 * \brief the base class which is used to incremental perform trace
 *        namespace resolution.
 * \ingroup lowleveltracing
 *
 * This class provides a public API to the ns3::TraceRoot object:
 *   - ns3::TraceResolver::Connect
 *   - ns3::TraceResolver::Disconnect
 *
 * It also provides an API for its subclasses. Each subclass should 
 * implement one of:
 *   - ns3::TraceResolver::DoLookup
 *   - ns3::TraceResolver::DoConnect and ns3::TraceResolver::DoDisconnect
 * Each subclass must also provide an ns3::TraceContext to the TraceResolver
 * constructor. Finally, each subclass can access the ns3::TraceContext 
 * associated to this  ns3::TraceResolver through the 
 * ns3::TraceResolver::GetContext method.
 */
class TraceResolver
{
public:
  virtual ~TraceResolver ();
  /**
   * \param path the namespace path to resolver
   * \param cb the callback to connect to the matching namespace
   *
   * This method is typically invoked by ns3::TraceRoot but advanced
   * users could also conceivably call it directly if they want to
   * skip the ns3::TraceRoot.
   */
  void Connect (std::string path, CallbackBase const &cb, const TraceContext &context);
  /**
   * \param path the namespace path to resolver
   * \param cb the callback to disconnect in the matching namespace
   *
   * This method is typically invoked by ns3::TraceRoot but advanced
   * users could also conceivably call it directly if they want to
   * skip the ns3::TraceRoot.
   */
  void Disconnect (std::string path, CallbackBase const &cb);
protected:
  typedef std::list<std::pair<TraceResolver *, TraceContext> > TraceResolverList;
private:
  /**
   * \param id the id to resolve. This is supposed to be
   * one element of the global tracing namespace.
   * \returns a list of reslvers which match the input namespace element
   *
   * A subclass which overrides this method should return a potentially
   * empty list of pointers to ns3::TraceResolver instances which match
   * the input namespace element. Each of these TraceResolver should be
   * instanciated with a TraceContext which holds enough context
   * information to identify the type of the TraceResolver.
   */
  virtual TraceResolverList DoLookup (std::string id) const;
  /**
   * \param cb callback to connect
   *
   * This method is invoked on leaf trace resolvers.
   */
  virtual void DoConnect (CallbackBase const &cb, const TraceContext &context);
  /**
   * \param cb callback to disconnect
   *
   * This method is invoked on leaf trace resolvers.
   */
  virtual void DoDisconnect (CallbackBase const &cb);
  TraceContext m_context;
};

}//namespace ns3

#endif /* TRACE_RESOLVER_H */
