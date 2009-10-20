/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage.inria.fr>
 */

#include "system-wall-clock-ms.h"
#include <sys/time.h>

namespace ns3 {

class SystemWallClockMsPrivate {
public:
  void Start (void);
  unsigned long long End (void);
private:
  struct timeval m_startTv;
  struct timeval m_endTv;
};

void 
SystemWallClockMsPrivate::Start (void)
{
  struct timezone tz;
  gettimeofday (&m_startTv, &tz);
}

unsigned long long 
SystemWallClockMsPrivate::End (void)
{
  struct timezone tz;
  gettimeofday (&m_endTv, &tz);
  unsigned long long end = m_endTv.tv_sec *1000 + m_endTv.tv_usec / 1000;
  unsigned long long start = m_startTv.tv_sec *1000 + m_startTv.tv_usec / 1000;
  return end - start;
}

SystemWallClockMs::SystemWallClockMs ()
  : m_priv (new SystemWallClockMsPrivate ())
{}

SystemWallClockMs::~SystemWallClockMs ()
{
  delete m_priv;
  m_priv = 0;
}

void
SystemWallClockMs::Start (void)
{
  m_priv->Start ();
}
unsigned long long
SystemWallClockMs::End (void)
{
  return m_priv->End ();
}

}; // namespace ns3
