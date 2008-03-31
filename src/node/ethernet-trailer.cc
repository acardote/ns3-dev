/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Emmanuelle Laprise <emmanuelle.laprise@bluekazoo.ca>
 */

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/trailer.h"
#include "ethernet-trailer.h"

NS_LOG_COMPONENT_DEFINE ("EthernetTrailer");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (EthernetTrailer);

bool EthernetTrailer::m_calcFcs = false;

EthernetTrailer::EthernetTrailer ()
{
  Init();
}

void EthernetTrailer::Init()
{
  m_fcs = 0;
}

void 
EthernetTrailer::EnableFcs (bool enable)
{
  m_calcFcs = enable;
}

bool
EthernetTrailer::CheckFcs (Ptr<Packet> p) const
{
  if (!m_calcFcs)
    {
      return true;
    } 
  else 
    {
      NS_LOG_WARN ("FCS calculation is not yet enabled");
      return false;
    }
}

void
EthernetTrailer::CalcFcs (Ptr<Packet> p)
{
  NS_LOG_WARN ("FCS calculation is not yet enabled");
}

void
EthernetTrailer::SetFcs (uint32_t fcs)
{
  m_fcs = fcs;
}

uint32_t
EthernetTrailer::GetFcs (void)
{
  return m_fcs;
}

uint32_t
EthernetTrailer::GetTrailerSize (void) const
{
  return GetSerializedSize();
}

TypeId 
EthernetTrailer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EthernetTrailer")
    .SetParent<Trailer> ()
    .AddConstructor<EthernetTrailer> ()
    ;
  return tid;
}
TypeId 
EthernetTrailer::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
void 
EthernetTrailer::Print (std::ostream &os) const
{
  os << "fcs=" << m_fcs;
}
uint32_t 
EthernetTrailer::GetSerializedSize (void) const
{
  return 4;
}

void
EthernetTrailer::Serialize (Buffer::Iterator end) const
{
  Buffer::Iterator i = end;
  i.Prev(GetSerializedSize());
  
  i.WriteU32 (m_fcs);
}
uint32_t
EthernetTrailer::Deserialize (Buffer::Iterator end)
{
  Buffer::Iterator i = end;
  uint32_t size = GetSerializedSize();
  i.Prev(size);

  m_fcs = i.ReadU32 ();

  return size;
}

}; // namespace ns3
