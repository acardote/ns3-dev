/* -*-  Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 INRIA
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
#include "wifi-mac-trailer.h"
#include <cassert>

namespace ns3 {

WifiMacTrailer::WifiMacTrailer ()
{}

WifiMacTrailer::~WifiMacTrailer ()
{}

uint32_t
WifiMacTrailer::GetUid (void)
{
  static uint32_t uid = AllocateUid<WifiMacTrailer> ("WifiMacTrailer.ns3.inria.fr");
  return uid;
}

std::string
WifiMacTrailer::GetName (void) const
{
  return "802.11 FCS";
}

void 
WifiMacTrailer::Print (std::ostream &os) const
{}
uint32_t 
WifiMacTrailer::GetSerializedSize (void) const
{
  return 4;
}
void 
WifiMacTrailer::Serialize (Buffer::Iterator start) const
{
  start.WriteU32 (0);
}
uint32_t
WifiMacTrailer::Deserialize (Buffer::Iterator start)
{
  return 4;
}

}; // namespace ns3