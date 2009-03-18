/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008,2009 IITP RAS
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
 * Author: Kirill Andreev <andreev@iitp.ru>
 */


#include "ns3/ie-dot11s-perr.h"
#include "ns3/address-utils.h"
#include "ns3/node.h"

namespace ns3 {
IeDot11sPerr::~IeDot11sPerr ()
{
}

TypeId
IeDot11sPerr::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::IeDot11sPerr")
                      .SetParent<Object> ();
  return tid;
}
void
IeDot11sPerr::PrintInformation (std::ostream &os) const
{
    // FILL
}
TypeId
IeDot11sPerr::GetInstanceTypeId () const
{
    return GetTypeId ();
}
IeDot11sPerr::IeDot11sPerr ():
    m_numOfDest (0)
{
}
uint8_t
IeDot11sPerr::GetNumOfDest ()
{
  return m_numOfDest;
}
void
IeDot11sPerr::SerializeInformation (Buffer::Iterator i) const
{
  i.WriteU8 (0);
  i.WriteU8 (m_numOfDest);
  NS_ASSERT (m_numOfDest == m_addressUnits.size ());
  for (unsigned int j = 0; j < m_numOfDest; j++)
    {
      WriteTo (i, m_addressUnits[j].destination);
      i.WriteHtonU32 (m_addressUnits[j].seqnum);
    }
}
uint8_t
IeDot11sPerr::DeserializeInformation (Buffer::Iterator start, uint8_t length)
{
  Buffer::Iterator i = start;
  i.Next (1); //Mode flags is not used now
  m_numOfDest = i.ReadU8 ();
  NS_ASSERT ((2+10*m_numOfDest) == length);
  length = 0; //to avoid compiler warning in optimized builds
  for (unsigned int j = 0; j < m_numOfDest; j++)
    {
      HwmpRtable::FailedDestination unit;
      ReadFrom (i,unit.destination);
      unit.seqnum = i.ReadNtohU32 ();
      m_addressUnits.push_back (unit);
    }
  return i.GetDistanceFrom (start);
}

uint8_t
IeDot11sPerr::GetInformationSize () const
{
  uint8_t retval =
     1 //ModeFlags
    +1 //NumOfDests
    +6*m_numOfDest
    +4*m_numOfDest;
  return retval;
}

void
IeDot11sPerr::AddAddressUnit (HwmpRtable::FailedDestination unit)
{
  for (unsigned int i = 0; i < m_addressUnits.size (); i ++)
    if (m_addressUnits[i].destination == unit.destination)
      return;
  m_addressUnits.push_back (unit);
  m_numOfDest++;
}

std::vector<HwmpRtable::FailedDestination>
IeDot11sPerr::GetAddressUnitVector ()
{
  return m_addressUnits;
}
void
IeDot11sPerr::DeleteAddressUnit (Mac48Address address)
{
  for (std::vector<HwmpRtable::FailedDestination>::iterator i = m_addressUnits.begin (); i != m_addressUnits.end(); i ++)
    if ((*i).destination == address)
      {
        m_numOfDest --;
        m_addressUnits.erase (i);
        break;
      }
}

void
IeDot11sPerr::ResetPerr ()
{
  m_numOfDest = 0;
  m_addressUnits.clear ();
}
}//namespace ns3

