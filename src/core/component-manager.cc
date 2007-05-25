/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INRIA
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
#include "component-manager.h"
#include "uid-manager.h"
#include "singleton.h"

namespace ns3 {

// we redefine a UidManager type for the class id singleton below.
// otherwise, we would have to share the same id singleton instance
// with the Iids.
class CidManager : public UidManager
{};

ClassId::ClassId (std::string name)
  : m_classId (Singleton<CidManager>::Get ()->Allocate (name))
{}

ClassId::ClassId (uint32_t classId)
  : m_classId (classId)
{}

std::string 
ClassId::GetName (void)
{
  return Singleton<CidManager>::Get ()->LookupByUid (m_classId);
}

bool operator == (const ClassId &a, const ClassId &b)
{
  return a.m_classId == b.m_classId;
}

ComponentManager::ClassIdEntry::ClassIdEntry (ClassId classId)
  : m_classId (classId)
{}

Ptr<Object>
ComponentManager::Create (ClassId classId)
{
  Callback<Ptr<Object> > callback = DoGetCallback<empty,empty,empty,empty,empty> (classId);
  return callback ();
}

CallbackBase *
ComponentManager::Lookup (ClassId classId)
{
  List *list = Singleton<List>::Get ();
  for (List::const_iterator i = list->begin (); i != list->end (); i++)
    {
      if (i->m_classId == classId)
	{
	  return i->m_callback;
	}
    }
  return 0;
}

ClassId 
ComponentManager::LookupByName (std::string name)
{
  return ClassId (Singleton<CidManager>::Get ()->LookupByName (name));
}

void
ComponentManager::Register (ClassId classId, CallbackBase *callback, 
                            std::vector<InterfaceId> supportedInterfaces)
{
  List *list = Singleton<List>::Get ();
  struct ClassIdEntry entry = ClassIdEntry (classId);
  entry.m_callback = callback;
  bool foundObject = false;
  for (std::vector<InterfaceId>::iterator i = supportedInterfaces.begin ();
       i != supportedInterfaces.end (); i++)
    {
      if (*i == Object::iid)
        {
          foundObject = true;
        }
    }
  if (!foundObject)
    {
      supportedInterfaces.push_back (Object::iid);
    }
  entry.m_supportedInterfaces = supportedInterfaces;
  list->push_back (entry);
}

void
RegisterCallback (ClassId classId, CallbackBase *callback, std::vector<InterfaceId> supportedInterfaces)
{
  return ComponentManager::Register (classId, callback, supportedInterfaces);
}



} // namespace ns3

#ifdef RUN_SELF_TESTS

#include "test.h"
#include "object.h"

namespace {


class B : public ns3::Object
{
public:
  static const ns3::InterfaceId iid;
  B ();
};

const ns3::InterfaceId B::iid = MakeInterfaceId ("B", Object::iid);

B::B ()
{
  SetInterfaceId (B::iid);
}


class A : public ns3::Object
{
public:
  static const ns3::ClassId cidZero;
  static const ns3::ClassId cidOneBool;
  static const ns3::ClassId cidOneUi32;
  static const ns3::ClassId cidOther;
  static const ns3::InterfaceId iid;

  A ();
  A (bool);
  A (uint32_t);

  bool m_zeroInvoked;
  bool m_oneBoolInvoked;
  bool m_oneUi32Invoked;

  bool m_bool;
  int m_ui32;
};

const ns3::ClassId A::cidZero = ns3::MakeClassId<A> ("A", A::iid);
const ns3::ClassId A::cidOneBool = ns3::MakeClassId <A,bool> ("ABool", A::iid);
const ns3::ClassId A::cidOneUi32 = ns3::MakeClassId <A,uint32_t> ("AUi32", A::iid);
const ns3::InterfaceId A::iid = MakeInterfaceId ("A", Object::iid);

A::A ()
  : m_zeroInvoked (true),
    m_oneBoolInvoked (false),
    m_oneUi32Invoked (false)
{
  SetInterfaceId (A::iid);
  ns3::Ptr<B> b = ns3::MakeNewObject<B> ();
  AddInterface (b);
}

A::A (bool bo)
  : m_zeroInvoked (false),
    m_oneBoolInvoked (true),
    m_oneUi32Invoked (false),
    m_bool (bo)
{
  SetInterfaceId (A::iid);
  ns3::Ptr<B> b = ns3::MakeNewObject<B> ();
  AddInterface (b);
}

A::A (uint32_t i)
  : m_zeroInvoked (false),
    m_oneBoolInvoked (false),
    m_oneUi32Invoked (true),
    m_ui32 (i)
{
  SetInterfaceId (A::iid);
  ns3::Ptr<B> b = ns3::MakeNewObject<B> ();
  AddInterface (b);
}

}

namespace ns3 {

class ComponentManagerTest : public Test
{
public:
  ComponentManagerTest ();
  virtual bool RunTests (void);
};

ComponentManagerTest::ComponentManagerTest ()
  : Test ("ComponentManager")
{}
bool 
ComponentManagerTest::RunTests (void)
{
  bool ok = true;

  Ptr<A> a = 0;
  a = ComponentManager::Create<A> (A::cidZero, A::iid);
  if (a == 0 ||
      !a->m_zeroInvoked)
    {
      ok = false;
    }

  a = ComponentManager::Create<A,bool> (A::cidOneBool, A::iid, true);
  if (a == 0 ||
      !a->m_oneBoolInvoked ||
      !a->m_bool)
    {
      ok = false;
    }

  a = ComponentManager::Create<A,bool> (A::cidOneBool, A::iid, false);
  if (a == 0 ||
      !a->m_oneBoolInvoked ||
      a->m_bool)
    {
      ok = false;
    }

  a = ComponentManager::Create<A,uint32_t> (A::cidOneUi32, A::iid, 10);
  if (a == 0 ||
      !a->m_oneUi32Invoked ||
      a->m_ui32 != 10)
    {
      ok = false;
    }

  a = ComponentManager::Create<A> (A::cidOneUi32, A::iid, (uint32_t)10);
  if (a == 0 ||
      !a->m_oneUi32Invoked ||
      a->m_ui32 != 10)
    {
      ok = false;
    }

  Ptr<B> b = ComponentManager::Create<B,uint32_t> (A::cidOneUi32, B::iid, 10);
  if (b == 0)
    {
      ok = false;
    }

  return ok;
}


static ComponentManagerTest g_unknownManagerTest;

} // namespace ns3

#endif /* RUN_SELF_TESTS */
