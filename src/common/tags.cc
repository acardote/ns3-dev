/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
#include "tags.h"
#include <string.h>
#include "ns3/fatal-error.h"

namespace ns3 {

Tag::TagInfoVector *
Tag::GetInfo (void)
{
  static Tag::TagInfoVector vector;
  return &vector;
}

std::string
Tag::GetUidString (uint32_t uid)
{
  TagInfo info = (*GetInfo ())[uid - 1];
  return info.uidString;
}
uint32_t 
Tag::GetUidFromUidString (std::string uidString)
{
  TagInfoVector *vec = GetInfo ();
  uint32_t uid = 1;
  for (TagInfoVector::iterator i = vec->begin (); i != vec->end (); i++)
    {
      if (i->uidString == uidString)
        {
          return uid;
        }
      uid++;
    }
  NS_FATAL_ERROR ("We are trying to deserialize an un-registered type. This can't work.");
  return 0;
}

void 
Tag::Destruct (uint32_t uid, uint8_t data[Tags::SIZE])
{
  TagInfo info = (*GetInfo ())[uid - 1];
  info.destruct (data);
}
void 
Tag::Print (uint32_t uid, uint8_t data[Tags::SIZE], std::ostream &os)
{
  TagInfo info = (*GetInfo ())[uid - 1];
  info.print (data, os);
}
uint32_t
Tag::GetSerializedSize (uint32_t uid, uint8_t data[Tags::SIZE])
{
  TagInfo info = (*GetInfo ())[uid - 1];
  return info.getSerializedSize (data);
}
void 
Tag::Serialize (uint32_t uid, uint8_t data[Tags::SIZE], Buffer::Iterator start)
{
  TagInfo info = (*GetInfo ())[uid - 1];
  info.serialize (data, start);
}
uint32_t 
Tag::Deserialize (uint32_t uid, uint8_t data[Tags::SIZE], Buffer::Iterator start)
{
  TagInfo info = (*GetInfo ())[uid - 1];
  return info.deserialize (data, start);
}

#ifdef USE_FREE_LIST

struct Tags::TagData *Tags::gFree = 0;
uint32_t Tags::gN_free = 0;

struct Tags::TagData *
Tags::AllocData (void)
{
  struct Tags::TagData *retval;
  if (gFree != 0) 
    {
      retval = gFree;
      gFree = gFree->m_next;
      gN_free--;
    } 
  else 
    {
      retval = new struct Tags::TagData ();
    }
  return retval;
}

void
Tags::FreeData (struct TagData *data)
{
  if (gN_free > 1000) 
    {
      delete data;
      return;
    }
  gN_free++;
  data->m_next = gFree;
  data->m_id = 0;
  gFree = data;
}
#else
struct Tags::TagData *
Tags::AllocData (void)
{
  struct Tags::TagData *retval;
  retval = new struct Tags::TagData ();
  return retval;
}

void
Tags::FreeData (struct TagData *data)
{
  delete data;
}
#endif

bool
Tags::Remove (uint32_t id)
{
  bool found = false;
  for (struct TagData *cur = m_next; cur != 0; cur = cur->m_next) 
    {
      if (cur->m_id == id) 
        {
          found = true;
        }
    }
  if (!found) 
    {
      return false;
    }
  struct TagData *start = 0;
  struct TagData **prevNext = &start;
  for (struct TagData *cur = m_next; cur != 0; cur = cur->m_next) 
    {
      if (cur->m_id == id) 
        {
          /**
           * XXX
           * Note: I believe that we could optimize this to
           * avoid copying each TagData located after the target id
           * and just link the already-copied list to the next tag.
           */
          continue;
        }
      struct TagData *copy = AllocData ();
      copy->m_id = cur->m_id;
      copy->m_count = 1;
      copy->m_next = 0;
      memcpy (copy->m_data, cur->m_data, Tags::SIZE);
      *prevNext = copy;
      prevNext = &copy->m_next;
    }
  *prevNext = 0;
  RemoveAll ();
  m_next = start;
  return true;
}

void 
Tags::Print (std::ostream &os) const
{
  for (struct TagData *cur = m_next; cur != 0; cur = cur->m_next) 
    {
      Tag::Print (cur->m_id, cur->m_data, os);
    }
}

uint32_t
Tags::GetSerializedSize (void) const
{
  uint32_t totalSize = 4; // reserve space for the size of the tag data.
  for (struct TagData *cur = m_next; cur != 0; cur = cur->m_next) 
    {
      uint32_t size = Tag::GetSerializedSize (cur->m_id, cur->m_data);
      if (size != 0)
        {
          std::string uidString = Tag::GetUidString (cur->m_id);
          totalSize += 4; // for the size of the string itself.
          totalSize += uidString.size ();
          totalSize += size;
        }
    }
  return totalSize;
}

void 
Tags::Serialize (Buffer::Iterator i, uint32_t totalSize) const
{
  i.WriteU32 (totalSize);
  for (struct TagData *cur = m_next; cur != 0; cur = cur->m_next) 
    {
      uint32_t size = Tag::GetSerializedSize (cur->m_id, cur->m_data);
      if (size != 0)
        {
          std::string uidString = Tag::GetUidString (cur->m_id);
          i.WriteU32 (uidString.size ());
          uint8_t *buf = (uint8_t *)uidString.c_str ();
          i.Write (buf, uidString.size ());
          Tag::Serialize (cur->m_id, cur->m_data, i);
        }
    }
}
uint32_t
Tags::Deserialize (Buffer::Iterator i)
{
  uint32_t totalSize = i.ReadU32 ();
  uint32_t bytesRead = 4;
  while (bytesRead < totalSize)
    {
      uint32_t uidStringSize = i.ReadU32 ();
      bytesRead += 4;
      std::string uidString;
      uidString.reserve (uidStringSize);
      for (uint32_t j = 0; j < uidStringSize; j++)
        {
          uint32_t c = i.ReadU8 ();
          uidString.push_back (c);
        }
      bytesRead += uidStringSize;
      uint32_t uid = Tag::GetUidFromUidString (uidString);
      struct TagData *newStart = AllocData ();
      newStart->m_count = 1;
      newStart->m_next = 0;
      newStart->m_id = uid;
      bytesRead += Tag::Deserialize (uid, newStart->m_data, i);
      newStart->m_next = m_next;
      m_next = newStart;
    }
  NS_ASSERT (bytesRead == totalSize);
  /**
   * The process of serialization/deserialization 
   * results in an inverted linked-list after
   * deserialization so, we invert the linked-list
   * in-place here.
   * Note: the algorithm below is pretty classic
   * but whenever I get to code it, it makes my
   * brain hurt :)
   */
  struct TagData *prev = 0;
  struct TagData *cur = m_next;
  while (cur != 0)
    {
      struct TagData *next = cur->m_next;
      cur->m_next = prev;
      prev = cur;
      cur = next;
    }
  m_next = prev;
  return totalSize;
}



}; // namespace ns3

#ifdef RUN_SELF_TESTS

#include "ns3/test.h"
#include <iomanip>
#include <iostream>

namespace ns3 {

static bool g_a;
static bool g_b;
static bool g_c;
static bool g_z;

class TagsTest : Test {
public:
  TagsTest ();
  virtual ~TagsTest ();
  virtual bool RunTests (void);
};

class myTagA 
{
public:
  static const char *GetUid (void) {return "myTagA.test.nsnam.org";}
  void Print (std::ostream &os) const {g_a = true;}
  uint32_t GetSerializedSize (void) const {return 0;}
  void Serialize (Buffer::Iterator i) const {}
  uint32_t Deserialize (Buffer::Iterator i) {return 0;}

  uint8_t a;
};
class myTagB 
{
public:
  static const char *GetUid (void) {return "myTagB.test.nsnam.org";}
  void Print (std::ostream &os) const {g_b = true;}
  uint32_t GetSerializedSize (void) const {return 0;}
  void Serialize (Buffer::Iterator i) const {}
  uint32_t Deserialize (Buffer::Iterator i) {return 0;}

  uint32_t b;
};
class myTagC 
{
public:
  static const char *GetUid (void) {return "myTagC.test.nsnam.org";}
  void Print (std::ostream &os) const {g_c = true;}
  uint32_t GetSerializedSize (void) const {return 0;}
  void Serialize (Buffer::Iterator i) const {}
  uint32_t Deserialize (Buffer::Iterator i) {return 0;}
  uint8_t c [Tags::SIZE];
};
class myInvalidTag 
{
public:
  static const char *GetUid (void) {return "myInvalidTag.test.nsnam.org";}
  void Print (std::ostream &os) const {}
  uint32_t GetSerializedSize (void) const {return 0;}
  void Serialize (Buffer::Iterator i) const {}
  uint32_t Deserialize (Buffer::Iterator i) {return 0;}

  uint8_t invalid [Tags::SIZE+1];
};
class myTagZ 
{
public:
  static const char *GetUid (void) {return "myTagZ.test.nsnam.org";}
  void Print (std::ostream &os) const {g_z = true;}
  uint32_t GetSerializedSize (void) const {return 0;}
  void Serialize (Buffer::Iterator i) const {}
  uint32_t Deserialize (Buffer::Iterator i) {return 0;}

  uint8_t z;
};

class MySmartTag 
{
public:
  static const char *GetUid (void) {return "mySmartTag.test.nsnam.org";}
  MySmartTag ()
  {
    //std::cout << "construct" << std::endl;
  }
  MySmartTag (const MySmartTag &o)
  {
    //std::cout << "copy" << std::endl;
  }
  ~MySmartTag ()
  {
    //std::cout << "destruct" << std::endl;
  }
  MySmartTag &operator = (const MySmartTag &o)
  {
    //std::cout << "assign" << std::endl;
    return *this;
  }
  void Print (std::ostream &os) const {}
  uint32_t GetSerializedSize (void) const {return 0;}
  void Serialize (Buffer::Iterator i) const {}
  uint32_t Deserialize (Buffer::Iterator i) {return 0;}
};


TagsTest::TagsTest ()
  : Test ("Tags")
{}
TagsTest::~TagsTest ()
{}

bool 
TagsTest::RunTests (void)
{
  bool ok = true;

  // build initial tag.
  Tags tags;
  myTagA a;
  a.a = 10;
  tags.Add (a);
  a.a = 0;
  tags.Peek (a);
  if (a.a != 10) 
    {
      ok = false;
    }
  g_a = false;
  tags.Print (std::cout);
  if (!g_a)
    {
      ok = false;
    }
  myTagB b;
  b.b = 0xff;
  tags.Add (b);
  b.b = 0;
  tags.Peek (b);
  if (b.b != 0xff) 
    {
      ok = false;
    }
  g_b = false;
  g_a = false;
  tags.Print (std::cout);
  if (!g_a || !g_b)
    {
      ok = false;
    }
  // make sure copy contains copy.
  Tags other = tags;
  g_b = false;
  g_a = false;
  other.Print (std::cout);
  if (!g_a || !g_b)
    {
      ok = false;
    }
  g_b = false;
  g_a = false;
  tags.Print (std::cout);
  if (!g_a || !g_b)
    {
      ok = false;
    }
  myTagA oA;
  oA.a = 0;
  other.Peek (oA);
  if (oA.a != 10) 
    {
      ok = false;
    }
  myTagB oB;
  oB.b = 1;
  other.Peek (oB);
  if (oB.b != 0xff) 
    {
      ok = false;
    }
  // remove data.
  other.Remove (oA);
  if (other.Peek (oA)) 
    {
      ok = false;
    }
  g_b = false;
  g_a = false;
  other.Print (std::cout);
  if (g_a || !g_b)
    {
      ok = false;
    }
  if (!tags.Peek (oA)) 
    {
      ok = false;
    }
  other.Remove (oB);
  if (other.Peek (oB)) 
    {
      ok = false;
    }
  if (!tags.Peek (oB)) 
    {
      ok = false;
    }

  other = tags;
  Tags another = other;
  myTagC c;
  memset (c.c, 0x66, 16);
  another.Add (c);
  c.c[0] = 0;
  another.Peek (c);
  if (!another.Peek (c)) 
    {
      ok = false;
    }
  if (tags.Peek (c)) 
    {
      ok = false;
    }

  other = other;
  //other.prettyPrint (std::cout);

  //struct myInvalidTag invalid;
  //tags.add (&invalid);

  myTagZ tagZ;
  Tags testLastTag;
  tagZ.z = 0;
  testLastTag.Add (tagZ);
  g_z = false;
  testLastTag.Print (std::cout);
  if (!g_z)
    {
      ok = false;
    }

  MySmartTag smartTag;
  {
    Tags tmp;
    tmp.Add (smartTag);
    tmp.Peek (smartTag);
    tmp.Remove (smartTag);
  }

  return ok;
}

static TagsTest gTagsTest;


}; // namespace ns3

#endif /* RUN_SELF_TESTS */

