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


#include "ie-dot11s-rann.h"
#include "ns3/assert.h"
#include "ns3/address-utils.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/test.h"

namespace ns3 {
namespace dot11s {

IeRann::~IeRann ()
{
}
TypeId
IeRann::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::dot11s::IeRann").SetParent<Object> ();
  return tid;
}
TypeId
IeRann::GetInstanceTypeId () const
{
  return GetTypeId ();
}
IeRann::IeRann ():
    m_flags (0),
    m_hopcount (0),
    m_ttl (0),
    m_originatorAddress (Mac48Address::GetBroadcast()),
    m_destSeqNumber (0),
    m_metric (0)
{
}
void
IeRann::SetFlags (uint8_t flags)
{
  m_flags = flags;
}
void
IeRann::SetHopcount (uint8_t hopcount)
{
  m_hopcount = hopcount;
}
void
IeRann::SetTTL (uint8_t ttl)
{
  m_ttl = ttl;
}
void
IeRann::SetDestSeqNumber (uint32_t dest_seq_number)
{
  m_destSeqNumber = dest_seq_number;
}
void
IeRann::SetMetric (uint32_t metric)
{
  m_metric = metric;
}
void
IeRann::SetOriginatorAddress (Mac48Address originator_address)
{
  m_originatorAddress = originator_address;
}

uint8_t
IeRann::GetFlags ()
{
  return m_flags;
}
uint8_t
IeRann::GetHopcount ()
{
  return m_hopcount;
}
uint8_t
IeRann::GetTtl ()
{
  return m_ttl;
}
uint32_t
IeRann::GetDestSeqNumber ()
{
  return m_destSeqNumber;
}
uint32_t
IeRann::GetMetric ()
{
  return m_metric;
}
void
IeRann::DecrementTtl ()
{
  m_ttl --;
  m_hopcount ++;
}

void 
IeRann::IncrementMetric (uint32_t m)
{
  m_metric += m;
}

Mac48Address
IeRann::GetOriginatorAddress ()
{
  return m_originatorAddress;
}
void
IeRann::SerializeInformation (Buffer::Iterator i) const
{
  i.WriteU8 (m_flags);
  i.WriteU8 (m_hopcount);
  i.WriteU8 (m_ttl);
  WriteTo (i, m_originatorAddress);
  i.WriteHtolsbU32 (m_destSeqNumber);
  i.WriteHtolsbU32 (m_metric);
}
uint8_t
IeRann::DeserializeInformation (Buffer::Iterator start, uint8_t length)
{
  Buffer::Iterator i = start;
  m_flags = i.ReadU8 ();
  m_hopcount = i.ReadU8 ();
  m_ttl = i.ReadU8 ();
  ReadFrom (i, m_originatorAddress);
  m_destSeqNumber = i.ReadLsbtohU32 ();
  m_metric = i.ReadLsbtohU32 ();
  return i.GetDistanceFrom (start);
}
uint8_t
IeRann::GetInformationSize () const
{
  uint8_t retval =
     1 //Flags
    +1 //Hopcount
    +1 //TTL
    +6 //OriginatorAddress
    +4 //DestSeqNumber
    +4;//Metric
  return retval;
}

void 
IeRann::PrintInformation (std::ostream &os) const
{
  os << "  flags              = " << (int)m_flags       << "\n";
  os << "  hop count          = " << (int)m_hopcount    << "\n";
  os << "  TTL                = " << (int)m_ttl         << "\n";
  os << "  originator address = " << m_originatorAddress<< "\n";
  os << "  dst seq. number    = " << m_destSeqNumber    << "\n";
  os << "  metric             = " << m_metric           << "\n";
}

bool
operator== (const IeRann & a, const IeRann & b)
{
  return (a.m_flags == b.m_flags 
      &&  a.m_hopcount == b.m_hopcount 
      &&  a.m_ttl == b.m_ttl
      &&  a.m_originatorAddress == b.m_originatorAddress
      &&  a.m_destSeqNumber == b.m_destSeqNumber
      &&  a.m_metric == b.m_metric 
      );
}
  
#ifdef RUN_SELF_TESTS

/// Built-in self test for IeRann
struct IeRannBist : public IeTest
{
  IeRannBist () : IeTest ("Mesh/802.11s/IE/RANN") {}
  virtual bool RunTests(); 
};

/// Test instance
static IeRannBist g_IeRannBist;

bool IeRannBist::RunTests ()
{
  bool result(true);
  
  // create test information element
  IeRann a;
  
  a.SetFlags (1);
  a.SetHopcount (2);
  a.SetTTL (4);
  a.DecrementTtl ();
  NS_TEST_ASSERT_EQUAL (a.GetTtl(), 3);
  a.SetOriginatorAddress (Mac48Address ("11:22:33:44:55:66"));
  a.SetDestSeqNumber (5);
  a.SetMetric (6);
  a.IncrementMetric (2);
  NS_TEST_ASSERT_EQUAL (a.GetMetric(), 8);
  
  result = result && TestRoundtripSerialization (a);
  return result;
}
#endif // RUN_SELF_TESTS

}} // namespace ns3::dot11s


