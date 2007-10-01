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
#include "trace-resolver.h"
#include "log.h"

NS_LOG_COMPONENT_DEFINE ("TraceResolver");

namespace ns3 {

TraceResolver::TraceResolver ()
  : m_count (1)
{}

TraceResolver::~TraceResolver ()
{}

void 
TraceResolver::Ref (void)
{
  m_count++;
}
void 
TraceResolver::Unref (void)
{
  m_count--;
  if (m_count == 0)
    {
      NS_LOG_LOGIC ("delete "<<this);
      delete this;
    }
}

std::string 
TraceResolver::GetElement (std::string path)
{
  std::string::size_type cur = 1;
  // check that first char is "/"
  std::string::size_type next = path.find ("/", cur);
  std::string id = std::string (path, cur, next-1);
  return id;
}
std::string 
TraceResolver::GetSubpath (std::string path)
{
  std::string::size_type cur = 1;
  // check that first char is "/"
  std::string::size_type next = path.find ("/", cur);
  std::string subpath;
  if (next != std::string::npos)
    {
      subpath = std::string (path, next, std::string::npos);
    }
  else
    {
      subpath = "";
    }
  return subpath;
}

void 
TraceResolver::SourceCollection::AddUnique (std::string path, 
                                            const TraceContext &context,
                                            const TraceDoc &doc)
{
  for (SourceVector::const_iterator i = m_sources.begin (); i != m_sources.end (); i++)
    {
      if (i->path == path &&
          context.IsSimilar (i->context))
        {
          return;
        }
    }
  struct Source source;
  source.path = path;
  source.context = context;
  source.doc = doc;
  m_sources.push_back (source);
}
TraceResolver::SourceCollection::Iterator
TraceResolver::SourceCollection::Begin (void) const
{
  return m_sources.begin ();
}
TraceResolver::SourceCollection::Iterator
TraceResolver::SourceCollection::End (void) const
{
  return m_sources.end ();
}
}//namespace ns3
