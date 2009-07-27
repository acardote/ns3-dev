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

#include "ns3/object.h"
#include "ns3/assert.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/log.h"

#include "hwmp-rtable.h"

namespace ns3 {
namespace dot11s {

NS_LOG_COMPONENT_DEFINE ("HwmpRtable");

NS_OBJECT_ENSURE_REGISTERED (HwmpRtable);

TypeId
HwmpRtable::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::dot11s::HwmpRtable")
    .SetParent<Object> ()
    .AddConstructor<HwmpRtable> ();
  return tid;
}
HwmpRtable::HwmpRtable ()
{
  DeleteProactivePath ();
}
HwmpRtable::~HwmpRtable ()
{
}
void
HwmpRtable::DoDispose ()
{
  m_routes.clear ();
}
void
HwmpRtable::AddReactivePath (Mac48Address destination, Mac48Address retransmitter, uint32_t interface,
    uint32_t metric, Time lifetime, uint32_t seqnum)
{
  std::map<Mac48Address, ReactiveRoute>::iterator i = m_routes.find (destination);
  if (i == m_routes.end ())
    {
      ReactiveRoute newroute;
      m_routes[destination] = newroute;
    }
  i = m_routes.find (destination);
  NS_ASSERT (i != m_routes.end ());
  i->second.retransmitter = retransmitter;
  i->second.interface = interface;
  i->second.metric = metric;
  i->second.whenExpire = Simulator::Now () + lifetime;
  i->second.seqnum = seqnum;
}
void
HwmpRtable::AddProactivePath (uint32_t metric, Mac48Address root, Mac48Address retransmitter,
    uint32_t interface, Time lifetime, uint32_t seqnum)
{
  m_root.root = root;
  m_root.retransmitter = retransmitter;
  m_root.metric = metric;
  m_root.whenExpire = Simulator::Now () + lifetime;
  m_root.seqnum = seqnum;
  m_root.interface = interface;
}
void
HwmpRtable::AddPrecursor (Mac48Address destination, uint32_t precursorInterface,
    Mac48Address precursorAddress)
{
  std::pair<uint32_t, Mac48Address> precursor;
  precursor.first = precursorInterface;
  precursor.second = precursorAddress;
  std::map<Mac48Address, ReactiveRoute>::iterator i = m_routes.find (destination);
  if (i != m_routes.end ())
    {
      bool should_add = true;
      for (unsigned int j = 0; j < i->second.precursors.size (); j++)
        {
          //NB: Only one active route may exist, so do not check
          //interface ID, just address
          if (i->second.precursors[j].second == precursorAddress)
            {
              should_add = false;
              break;
            }
        }
      if (should_add)
        {
          i->second.precursors.push_back (precursor);
        }
    }
  if (m_root.root == destination)
    {
      for (unsigned int j = 0; j < m_root.precursors.size (); j++)
        {
          if (m_root.precursors[j].second == precursorAddress)
            {
              return;
            }
        }
    }
  m_root.precursors.push_back (precursor);
}
void
HwmpRtable::DeleteProactivePath ()
{
  m_root.precursors.clear ();
  m_root.interface = INTERFACE_ANY;
  m_root.metric = MAX_METRIC;
  m_root.retransmitter = Mac48Address::GetBroadcast ();
  m_root.seqnum = 0;
  m_root.whenExpire = Simulator::Now ();
}
void
HwmpRtable::DeleteProactivePath (Mac48Address root)
{
  if (m_root.root == root)
    {
      DeleteProactivePath ();
    }
}
void
HwmpRtable::DeleteReactivePath (Mac48Address destination)
{
  std::map<Mac48Address, ReactiveRoute>::iterator i = m_routes.find (destination);
  if (i != m_routes.end ())
    {
      m_routes.erase (i);
    }
}
HwmpRtable::LookupResult
HwmpRtable::LookupReactive (Mac48Address destination)
{
  std::map<Mac48Address, ReactiveRoute>::iterator i = m_routes.find (destination);
  if (i == m_routes.end ())
    {
      return LookupResult ();
    }
  if ((i->second.whenExpire < Simulator::Now ()) && (i->second.whenExpire != Seconds (0)))
    {
      NS_LOG_DEBUG ("Reactive route has expired, sorry.");
      return LookupResult ();
    }
  return LookupReactiveExpired (destination);
}
HwmpRtable::LookupResult
HwmpRtable::LookupReactiveExpired (Mac48Address destination)
{
  std::map<Mac48Address, ReactiveRoute>::iterator i = m_routes.find (destination);
  if (i == m_routes.end ())
    {
      return LookupResult ();
    }
  return LookupResult (i->second.retransmitter, i->second.interface, i->second.metric, i->second.seqnum,
      i->second.whenExpire - Simulator::Now ());
}
HwmpRtable::LookupResult
HwmpRtable::LookupProactive ()
{
  if (m_root.whenExpire < Simulator::Now ())
    {
      NS_LOG_DEBUG ("Proactive route has expired and will be deleted, sorry.");
      DeleteProactivePath ();
    }
  return LookupProactiveExpired ();
}
HwmpRtable::LookupResult
HwmpRtable::LookupProactiveExpired ()
{
  return LookupResult (m_root.retransmitter, m_root.interface, m_root.metric, m_root.seqnum,
      m_root.whenExpire - Simulator::Now ());
}
std::vector<IePerr::FailedDestination>
HwmpRtable::GetUnreachableDestinations (Mac48Address peerAddress)
{
  IePerr::FailedDestination dst;
  std::vector<IePerr::FailedDestination> retval;
  for (std::map<Mac48Address, ReactiveRoute>::iterator i = m_routes.begin (); i != m_routes.end (); i++)
  {
      if (i->second.retransmitter == peerAddress)
        {
          dst.destination = i->first;
          i->second.seqnum++;
          dst.seqnum = i->second.seqnum;
          retval.push_back (dst);
        }
  }
  //Lookup a path to root
  if (m_root.retransmitter == peerAddress)
    {
      dst.destination = m_root.root;
      dst.seqnum = m_root.seqnum;
      retval.push_back (dst);
    }
  return retval;
}
HwmpRtable::PrecursorList
HwmpRtable::GetPrecursors (Mac48Address destination)
{
  //We suppose that no duplicates here can be
  PrecursorList retval;
  std::map<Mac48Address, ReactiveRoute>::iterator route = m_routes.find (destination);
  if (route != m_routes.end ())
    {
      for (unsigned int i = 0; i < route->second.precursors.size (); i++)
        {
          retval.push_back (route->second.precursors[i]);
        }
    }
  if (m_root.root == destination)
    {
      for (unsigned int i = 0; i < m_root.precursors.size (); i++)
        {
          bool should_add = true;
          for (unsigned int j = 0; j < retval.size (); j++)
            {
              if (retval[j].second == m_root.precursors[i].second)
                {
                  should_add = false;
                  break;
                }
            }
          if (should_add)
            {
              retval.push_back (m_root.precursors[i]);
            }
        }
    }
  return retval;
}
bool
HwmpRtable::LookupResult::operator== (const HwmpRtable::LookupResult & o) const
{
  return (retransmitter == o.retransmitter && ifIndex == o.ifIndex && metric == o.metric && seqnum
      == o.seqnum);
}
HwmpRtable::LookupResult::LookupResult (Mac48Address r, uint32_t i, uint32_t m, uint32_t s, Time l) :
  retransmitter (r), ifIndex (i), metric (m), seqnum (s), lifetime (l)
{
}
bool
HwmpRtable::LookupResult::IsValid () const
{
  return !(retransmitter == Mac48Address::GetBroadcast () && ifIndex == INTERFACE_ANY && metric == MAX_METRIC
      && seqnum == 0);
}
#ifdef RUN_SELF_TESTS
/// Unit test for HwmpRtable
class HwmpRtableTest : public Test
{
public:
  HwmpRtableTest ();
  virtual bool
  RunTests ();

private:
  /// Test Add apth and lookup path;
  void
  TestLookup ();
  /**
   * \name Test add path and try to lookup after entry has expired
   * \{
   */
  void
  TestAddPath ();
  void
  TestExpire ();
  ///\}
  /**
   * \name Test add precursors and find precursor list in rtable
   * \{
   */
  void
  TestPrecursorAdd ();
  void
  TestPrecursorFind ();
  ///\}
private:
  bool result;

  Mac48Address dst;
  Mac48Address hop;
  uint32_t iface;
  uint32_t metric;
  uint32_t seqnum;
  Time expire;
  Ptr<HwmpRtable> table;
  std::vector<Mac48Address> precursors;
};
/// Test instance
static HwmpRtableTest g_HwmpRtableTest;

HwmpRtableTest::HwmpRtableTest () :
  Test ("Mesh/802.11s/HwmpRtable"), result (true), dst ("01:00:00:01:00:01"), hop ("01:00:00:01:00:03"),
      iface (8010), metric (10), seqnum (1), expire (Seconds (10))
{
  precursors.push_back (Mac48Address ("00:10:20:30:40:50"));
  precursors.push_back (Mac48Address ("00:11:22:33:44:55"));
  precursors.push_back (Mac48Address ("00:01:02:03:04:05"));
}
void
HwmpRtableTest::TestLookup ()
{
  HwmpRtable::LookupResult correct (hop, iface, metric, seqnum);

  // Reactive path
  table->AddReactivePath (dst, hop, iface, metric, expire, seqnum);
  NS_TEST_ASSERT (table->LookupReactive (dst) == correct);
  table->DeleteReactivePath (dst);
  NS_TEST_ASSERT (!table->LookupReactive (dst).IsValid ());

  // Proactive
  table->AddProactivePath (metric, dst, hop, iface, expire, seqnum);
  NS_TEST_ASSERT (table->LookupProactive () == correct);
  table->DeleteProactivePath (dst);
  NS_TEST_ASSERT (!table->LookupProactive ().IsValid ());
}
void
HwmpRtableTest::TestAddPath ()
{
  table->AddReactivePath (dst, hop, iface, metric, expire, seqnum);
  table->AddProactivePath (metric, dst, hop, iface, expire, seqnum);
}
void
HwmpRtableTest::TestExpire ()
{
  // this is assumed to be called when path records are already expired
  HwmpRtable::LookupResult correct (hop, iface, metric, seqnum);
  NS_TEST_ASSERT (table->LookupReactiveExpired (dst) == correct);
  NS_TEST_ASSERT (table->LookupProactiveExpired () == correct);

  NS_TEST_ASSERT (!table->LookupReactive (dst).IsValid ());
  NS_TEST_ASSERT (!table->LookupProactive ().IsValid ());
}
void
HwmpRtableTest::TestPrecursorAdd ()
{
  for (std::vector<Mac48Address>::const_iterator i = precursors.begin (); i != precursors.end (); i++)
    {
      table->AddPrecursor (dst, iface, *i);
      // Check that duplicates are filtered
      table->AddPrecursor (dst, iface, *i);
    }
}
void
HwmpRtableTest::TestPrecursorFind ()
{
  HwmpRtable::PrecursorList precursorList = table->GetPrecursors (dst);
  NS_TEST_ASSERT (precursors.size () == precursorList.size ());
  for (unsigned int i = 0; i < precursors.size (); i++)
    {
      NS_TEST_ASSERT (precursorList[i].first == iface);
      NS_TEST_ASSERT (precursorList[i].second == precursors[i]);
    }
}
bool
HwmpRtableTest::RunTests ()
{
  table = CreateObject<HwmpRtable> ();

  Simulator::Schedule (Seconds (0), &HwmpRtableTest::TestLookup, this);
  Simulator::Schedule (Seconds (1), &HwmpRtableTest::TestAddPath, this);
  Simulator::Schedule (Seconds (2), &HwmpRtableTest::TestPrecursorAdd, this);
  Simulator::Schedule (expire + Seconds (2), &HwmpRtableTest::TestExpire, this);
  Simulator::Schedule (expire + Seconds (3), &HwmpRtableTest::TestPrecursorFind, this);

  Simulator::Run ();
  Simulator::Destroy ();

  return result;
}
#endif // RUN_SELF_TESTS
} //namespace dot11s
} //namespace ns3
