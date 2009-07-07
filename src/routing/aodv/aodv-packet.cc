/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 1997, 1998 Carnegie Mellon University.
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
 * Authors: The AODV code developed by the CMU/MONARCH group was optimized and
 * tuned by Samir Das and Mahesh Marina, University of Cincinnati.
 * The work was partially done in Sun Microsystems.
 *
 * Ported to ns-3 by Elena Borovkova <borovkovaes@iitp.ru>
 */
#include "ns3/test.h"
#include "aodv-packet.h"
#include "ns3/address-utils.h"
#include "ns3/packet.h"

namespace ns3 {
namespace aodv {

//-----------------------------------------------------------------------------
// RREQ
//-----------------------------------------------------------------------------
RreqHeader::RreqHeader () : rq_flags(0), reserved(0), rq_hop_count(0), rq_bcast_id(0), rq_dst_seqno(0), rq_src_seqno(0)
{
  // TODO check defaults in AODV UU
  SetGratiousRrep (false);
  SetDestinationOnly (false);
  SetUnknownSeqno (false);
}

TypeId RreqHeader::GetInstanceTypeId() const
{
  return TypeId();
}

uint32_t 
RreqHeader::GetSerializedSize () const
{
  return 24;
}

void 
RreqHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 (type());
  i.WriteU8 (rq_flags);
  i.WriteU8 (reserved);
  i.WriteU8 (rq_hop_count);
  i.WriteHtonU32 (rq_bcast_id);
  WriteTo (i, rq_dst);
  i.WriteHtonU32 (rq_dst_seqno);
  WriteTo (i, rq_src);
  i.WriteHtonU32 (rq_src_seqno);
}

uint32_t 
RreqHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t t = i.ReadU8 ();
  NS_ASSERT (t == type());
  
  rq_flags = i.ReadU8 ();
  reserved = i.ReadU8 ();
  rq_hop_count = i.ReadU8 ();
  rq_bcast_id = i.ReadNtohU32 ();
  ReadFrom (i, rq_dst);
  rq_dst_seqno = i.ReadNtohU32 ();
  ReadFrom (i, rq_src);
  rq_src_seqno = i.ReadNtohU32 ();
  
  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void 
RreqHeader::Print (std::ostream &os) const
{
  // TODO
}

std::ostream & operator<<(std::ostream & os, RreqHeader const & h)
{
  h.Print (os);
  return os;
}

void 
RreqHeader::SetGratiousRrep (bool f)
{
  if (f) rq_flags |= (1 << 2);
  else   rq_flags &= ~(1 << 2);
}

bool 
RreqHeader::GetGratiousRrep () const
{
  return (rq_flags & (1 << 2));
}

void 
RreqHeader::SetDestinationOnly (bool f)
{
  if (f) rq_flags |= (1 << 3);
  else   rq_flags &= ~(1 << 3);
}

bool 
RreqHeader::GetDestinationOnly () const
{
  return (rq_flags & (1 << 3));
}

void 
RreqHeader::SetUnknownSeqno (bool f)
{
  if (f) rq_flags |= (1 << 4);
  else   rq_flags &= ~(1 << 4);
}

bool 
RreqHeader::GetUnknownSeqno () const
{
  return (rq_flags & (1 << 4));
}

bool
RreqHeader::operator==(RreqHeader const & o) const
{
  return (rq_flags == o.rq_flags && reserved == o.reserved &&
      rq_hop_count == o.rq_hop_count && rq_bcast_id == o.rq_bcast_id &&
      rq_dst == o.rq_dst && rq_dst_seqno == o.rq_dst_seqno && 
      rq_src == o.rq_src && rq_src_seqno == o.rq_src_seqno);
}

#ifdef RUN_SELF_TESTS
/// Unit test for RREQ
struct RreqHeaderTest : public Test
{
  RreqHeaderTest () : Test ("AODV/RREQ") {}
  virtual bool RunTests();
};

/// Test instance
static RreqHeaderTest g_RreqHeaderTest;

bool RreqHeaderTest::RunTests ()
{
  bool result(true);

  RreqHeader h;
  h.SetDst (Ipv4Address("1.2.3.4"));
  h.SetDstSeqno (123);
  h.SetSrc (Ipv4Address("4.3.2.1"));
  h.SetSrcSeqno (321);
  h.SetId (1);
  h.SetGratiousRrep (true);
  NS_TEST_ASSERT(h.GetGratiousRrep ());
  h.SetDestinationOnly (true);
  NS_TEST_ASSERT(h.GetDestinationOnly ());
  h.SetUnknownSeqno (true);
  NS_TEST_ASSERT(h.GetUnknownSeqno ());

  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (h);
  RreqHeader h2;
  uint32_t bytes = p->RemoveHeader(h2);
  NS_TEST_ASSERT_EQUAL (bytes, 24);
  NS_TEST_ASSERT_EQUAL (h, h2);
  return result;
}
#endif

//-----------------------------------------------------------------------------
// RREP
//-----------------------------------------------------------------------------

RrepHeader::RrepHeader() :rp_flags(0), prefixSize(0),  rp_hop_count(0), rp_dst_seqno(0)
{
	SetAckRequired(false);
}

TypeId
RrepHeader::GetInstanceTypeId() const
{
	return TypeId();
}

uint32_t
RrepHeader::GetSerializedSize () const
{
	return 20;
}

void
RrepHeader::Serialize (Buffer::Iterator i) const
{
	i.WriteU8(type());
	i.WriteU8(rp_flags);
	i.WriteU8(prefixSize);
  i.WriteU8 (rp_hop_count);
  WriteTo (i, rp_dst);
  i.WriteHtonU32 (rp_dst_seqno);
  WriteTo (i, rp_src);
  i.WriteHtonU32 (rp_lifetime);
}

uint32_t
RrepHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t t = i.ReadU8 ();
  NS_ASSERT (t == type());

  rp_flags = i.ReadU8 ();
  prefixSize = i.ReadU8 ();
  rp_hop_count = i.ReadU8 ();
  ReadFrom (i, rp_dst);
  rp_dst_seqno = i.ReadNtohU32 ();
  ReadFrom (i, rp_src);
  rp_lifetime = i.ReadNtohU32 ();

  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
RrepHeader::Print (std::ostream &os) const
{
	// TODO
}

void
RrepHeader::SetAckRequired (bool f)
{
	 if (f) rp_flags |= (1 << 1);
	 else   rp_flags &= ~(1 << 1);
}

bool
RrepHeader::GetAckRequired () const
{
	return (rp_flags & (1 << 1));
}

void
RrepHeader::SetPrefixSize(uint8_t sz)
{
	prefixSize = sz;
}

uint8_t
RrepHeader::GetPrefixSize() const
{
	return prefixSize;
}

bool
RrepHeader::operator==(RrepHeader const & o) const
{
	return (rp_flags == o.rp_flags && prefixSize == o.prefixSize &&
			rp_hop_count == o.rp_hop_count && rp_dst == o.rp_dst &&
			rp_dst_seqno == o.rp_dst_seqno && rp_src == o.rp_src &&
			rp_lifetime == o.rp_lifetime);
}


void
RrepHeader::SetHello(Ipv4Address src, uint32_t srcSeqNo)
{
	rp_flags = 0;
	prefixSize = 0;
	rp_hop_count = 0;
	rp_dst = src;
	rp_dst_seqno = srcSeqNo;
	rp_src = src;
	rp_lifetime = HELLO_INTERVAL * ALLOWED_HELLO_LOSS;
}

bool
RrepHeader::IsHello(Ipv4Address src, uint32_t srcSeqNo)
{
	return (rp_flags == 0 && prefixSize == 0 && rp_hop_count == 0 &&
			rp_dst == src && rp_dst_seqno == srcSeqNo && rp_src == src &&
			rp_lifetime ==  (HELLO_INTERVAL * ALLOWED_HELLO_LOSS));
}

std::ostream & operator<<(std::ostream & os, RrepHeader const & h)
{
  h.Print (os);
  return os;
}

#ifdef RUN_SELF_TESTS
/// Unit test for RREP
struct RrepHeaderTest : public Test
{
  RrepHeaderTest () : Test ("AODV/RREP") {}
  virtual bool RunTests();
};

/// Test instance
static RrepHeaderTest g_RrepHeaderTest;

bool RrepHeaderTest::RunTests ()
{
  bool result(true);

  RrepHeader h;
  h.SetDst (Ipv4Address("1.2.3.4"));
  h.SetDstSeqno (123);
  h.SetSrc (Ipv4Address("4.3.2.1"));
  h.SetLifeTime(12);
  h.SetAckRequired(true);
  NS_TEST_ASSERT(h.GetAckRequired ());
  h.SetAckRequired(false);
  NS_TEST_ASSERT(!h.GetAckRequired ());
  h.SetPrefixSize(2);
  uint8_t sz = h.GetPrefixSize();
  NS_TEST_ASSERT_EQUAL(2, sz);
  NS_TEST_ASSERT(!h.IsHello(Ipv4Address("1.2.3.4"),10));
  h.SetHello(Ipv4Address("1.2.3.4"),12);
  NS_TEST_ASSERT(h.IsHello(Ipv4Address("1.2.3.4"),12));

  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (h);
  RrepHeader h2;
  uint32_t bytes = p->RemoveHeader(h2);
  NS_TEST_ASSERT_EQUAL (bytes, 20);
  NS_TEST_ASSERT_EQUAL (h, h2);
  return result;
}
#endif

//-----------------------------------------------------------------------------
// RREP-ACK
//-----------------------------------------------------------------------------

RrepAckHeader::RrepAckHeader () : reserved(0)
{
}

TypeId
RrepAckHeader::GetInstanceTypeId() const
{
	return TypeId();
}

uint32_t
RrepAckHeader::GetSerializedSize () const
{
	return 2;
}

void
RrepAckHeader::Serialize (Buffer::Iterator i) const
{
	i.WriteU8(type());
	i.WriteU8(reserved);
}

uint32_t
RrepAckHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t t = i.ReadU8 ();
  NS_ASSERT (t == type());
  reserved = i.ReadU8 ();
  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
RrepAckHeader::Print (std::ostream &os) const
{
	// TODO
}


bool
RrepAckHeader::operator==(RrepAckHeader const & o) const
{
	return reserved == o.reserved;
}

std::ostream & operator<<(std::ostream & os, RrepAckHeader const & h)
{
	h.Print(os);
	return os;
}

#ifdef RUN_SELF_TESTS
/// Unit test for RREP-ACK
struct RrepAckHeaderTest : public Test
{
	RrepAckHeaderTest () : Test ("AODV/RREP-ACK") {}
  virtual bool RunTests();
};

/// Test instance
static RrepAckHeaderTest g_RrepAckHeaderTest;

bool RrepAckHeaderTest::RunTests ()
{
  bool result(true);

  RrepAckHeader h;
  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (h);
  RrepAckHeader h2;
  uint32_t bytes = p->RemoveHeader(h2);
  NS_TEST_ASSERT_EQUAL (bytes, 2);
  NS_TEST_ASSERT_EQUAL (h, h2);
  return result;
}
#endif

//-----------------------------------------------------------------------------
// RERR
//-----------------------------------------------------------------------------
RerrHeader::RerrHeader() : er_flag(0), reserved(0)
{
	destCount = unreachable_dst.size();
}

TypeId
RerrHeader::GetInstanceTypeId() const
{
	return TypeId();
}

uint32_t
RerrHeader::GetSerializedSize () const
{
	return ( 4 + 8*destCount);
}

void
RerrHeader::Serialize (Buffer::Iterator i) const
{
	i.WriteU8(type());
	i.WriteU8(er_flag);
	i.WriteU8(reserved);
	i.WriteU8(destCount);
	std::map<Ipv4Address, uint32_t>::const_iterator j;
	for(j = unreachable_dst.begin(); j != unreachable_dst.end(); ++j)
	{
	  WriteTo (i, (*j).first);
	  i.WriteHtonU32 ((*j).second);
	}
}

uint32_t
RerrHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t t = i.ReadU8 ();
  NS_ASSERT (t == type());

  er_flag = i.ReadU8 ();
  reserved = i.ReadU8 ();
  destCount = i.ReadU8 ();
  unreachable_dst.clear();
  Ipv4Address address;
  uint32_t seqNo;
	for(uint8_t k = 0; k < destCount; ++k)
	{
		ReadFrom (i, address);
		seqNo = i.ReadNtohU32 ();
		unreachable_dst[address] = seqNo;
	}

  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void
RerrHeader::Print (std::ostream &os) const
{
	// TODO
}

void
RerrHeader::SetNoDelete(bool f)
{
	 if (f) er_flag |= (1 << 0);
	 else   er_flag &= ~(1 << 0);
}

bool
RerrHeader::GetNoDelete()
{
	return (er_flag & (1 << 0));
}

bool
RerrHeader::AddUnDestination(Ipv4Address dst, uint32_t seqNo)
{
	if(unreachable_dst.find(dst) != unreachable_dst.end())
		return false;
	unreachable_dst[dst] = seqNo;
	destCount = unreachable_dst.size();
	return true;
}

bool
RerrHeader::operator==(RerrHeader const & o) const
{
	bool result = ( er_flag == o.er_flag && reserved == o.reserved &&
			destCount == o.destCount );
	if(!result)
		return false;
	std::map<Ipv4Address, uint32_t>::const_iterator j = unreachable_dst.begin();
	std::map<Ipv4Address, uint32_t>::const_iterator k = o.unreachable_dst.begin();
	for(uint8_t i = 0; i < destCount; ++i)
	{
		result = result && ( (*j).first == (*k).first ) && ( (*j).second == (*k).second );
		if(!result)
				return false;
		j++;
		k++;
	}
	return result;
}

std::ostream & operator<<(std::ostream & os, RerrHeader const & h)
{
	h.Print(os);
	return os;
}

#ifdef RUN_SELF_TESTS
/// Unit test for RERR
struct RerrHeaderTest : public Test
{
  RerrHeaderTest () : Test ("AODV/RERR") {}
  virtual bool RunTests();
};

/// Test instance
static RerrHeaderTest g_RerrHeaderTest;

bool RerrHeaderTest::RunTests ()
{
  bool result(true);

  RerrHeader h;
  h.SetNoDelete(true);
  NS_TEST_ASSERT(h.GetNoDelete());
  Ipv4Address dst = Ipv4Address("1.2.3.4");
  NS_TEST_ASSERT(h.AddUnDestination(dst, 12));
  NS_TEST_ASSERT_EQUAL(h.GetDestCount(),1);
  NS_TEST_ASSERT(!h.AddUnDestination(dst, 13));
  Ipv4Address dst2 = Ipv4Address("4.3.2.1");
  NS_TEST_ASSERT(h.AddUnDestination(dst2, 12));
  NS_TEST_ASSERT_EQUAL(h.GetDestCount(), 2);

  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (h);
  RerrHeader h2;
  uint32_t bytes = p->RemoveHeader(h2);
  NS_TEST_ASSERT_EQUAL (bytes, h.GetSerializedSize());
  NS_TEST_ASSERT_EQUAL (h, h2);
  return result;
}
#endif
}}
