/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2004,2005,2006 INRIA
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

#include "aarf-mac-stations.h"

#define Min(a,b) ((a<b)?a:b)
#define Max(a,b) ((a>b)?a:b)

namespace ns3 {

AarfMacStations::AarfMacStations (WifiMode defaultTxMode, 
                                  uint32_t minTimerThreshold,
                                  uint32_t minSuccessThreshold,
                                  double successK,
                                  uint32_t maxSuccessThreshold,
                                  double timerK)
  : ArfMacStations (defaultTxMode, 
                    minTimerThreshold,
                    minSuccessThreshold),
    m_minTimerThreshold (minTimerThreshold),
    m_minSuccessThreshold (minSuccessThreshold),
    m_successK (successK),
    m_maxSuccessThreshold (maxSuccessThreshold),
    m_timerK (timerK)
{}
AarfMacStations::~AarfMacStations ()
{}
MacStation *
AarfMacStations::CreateStation (void)
{
  return new AarfMacStation (this, m_minTimerThreshold,
                             m_minSuccessThreshold,
                             m_successK,
                             m_maxSuccessThreshold,
                             m_timerK);
}




AarfMacStation::AarfMacStation (AarfMacStations *stations,
                                uint32_t minTimerThreshold,
                                uint32_t minSuccessThreshold,
                                double successK,
                                uint32_t maxSuccessThreshold,
                                double timerK)
  : ArfMacStation (stations, minTimerThreshold, minSuccessThreshold),
    m_successK (successK),
    m_maxSuccessThreshold (maxSuccessThreshold),
    m_timerK (timerK)
{}


AarfMacStation::~AarfMacStation ()
{}

void 
AarfMacStation::ReportRecoveryFailure (void)
{
  SetSuccessThreshold ((int)(Min (GetSuccessThreshold () * m_successK,
                                  m_maxSuccessThreshold)));
  SetTimerTimeout ((int)(Max (GetMinTimerTimeout (),
                              GetSuccessThreshold () * m_timerK)));
}

void 
AarfMacStation::ReportFailure (void)
{
  SetTimerTimeout (GetMinTimerTimeout ());
  SetSuccessThreshold (GetMinSuccessThreshold ());
}

} // namespace ns3
