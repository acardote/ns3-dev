/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INESC Porto
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
 * Author: Gustavo J. A. M. Carneiro  <gjc@inescporto.pt>
 */
#ifndef EVENT_COLLECTOR_H
#define EVENT_COLLECTOR_H

#include <list>
#include "event-id.h"
#include "simulator.h"

namespace ns3 {

/**
 * \brief An object that tracks scheduled events and automatically
 * cancels them when it is destroyed.
 */
class EventCollector
{
public:

  EventCollector () :
    m_nextCleanupSize (CLEANUP_CHUNK_MIN_SIZE)
  {}

  /**
   * \brief Tracks a new event
   */
  void Track (EventId event)
  {
    m_events.push_back (event);
    if (m_events.size () >= m_nextCleanupSize)
      Cleanup ();
  }

  ~EventCollector ();

private:
  enum {
    CLEANUP_CHUNK_MIN_SIZE = 8,
    CLEANUP_CHUNK_MAX_SIZE = 1024
  };

  std::list<EventId>::size_type m_nextCleanupSize;
  std::list<EventId> m_events;

  void Cleanup ();
};

}; // namespace ns3

#endif /* EVENT_COLLECTOR_H */
