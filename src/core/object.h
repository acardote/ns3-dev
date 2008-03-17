/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INRIA, Gustavo Carneiro
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
 * Authors: Gustavo Carneiro <gjcarneiro@gmail.com>,
 *          Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>
#include <string>
#include <vector>
#include "ptr.h"
#include "callback.h"
#include "attribute.h"
#include "object-base.h"
#include "attribute-helper.h"
#include "attribute-list.h"


namespace ns3 {

class CallbackBase;
class Object;
class AttributeAccessor;
class AttributeValue;
class AttributeList;
class TraceSourceAccessor;

/**
 * \brief a base class which provides memory management and object aggregation
 *
 */
class Object : public ObjectBase
{
public:
  static TypeId GetTypeId (void);

  Object ();
  virtual ~Object ();

  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Increment the reference count. This method should not be called
   * by user code. Object instances are expected to be used in conjunction
   * of the Ptr template which would make calling Ref unecessary and 
   * dangerous.
   */
  inline void Ref (void) const;
  /**
   * Decrement the reference count. This method should not be called
   * by user code. Object instances are expected to be used in conjunction
   * of the Ptr template which would make calling Ref unecessary and 
   * dangerous.
   */
  inline void Unref (void) const;
  /**
   * \returns a pointer to the requested interface or zero if it could not be found.
   */
  template <typename T>
  Ptr<T> GetObject (void) const;
  /**
   * \param tid the interface id of the requested interface
   * \returns a pointer to the requested interface or zero if it could not be found.
   */
  template <typename T>
  Ptr<T> GetObject (TypeId tid) const;
  /**
   * Run the DoDispose methods of this object and all the
   * objects aggregated to it.
   * After calling this method, the object is expected to be
   * totally unusable except for the Ref and Unref methods.
   * It is an error to call Dispose twice on the same object 
   * instance
   */
  void Dispose (void);
  /**
   * \param other another object pointer
   *
   * This method aggregates the two objects together: after this
   * method returns, it becomes possible to call GetObject
   * on one to get the other, and vice-versa. 
   */
  void AggregateObject (Ptr<Object> other);

protected:
  /**
   * This method is called by Object::Dispose.
   * Subclasses are expected to override this method and chain
   * up to their parent's implementation once they are done.
   */
  virtual void DoDispose (void);
private:
  template <typename T>
  friend Ptr<T> CreateObject (const AttributeList &attributes);
  template <typename T>
  friend Ptr<T> CopyObject (Ptr<T> object);

  friend class ObjectFactory;

  Ptr<Object> DoGetObject (TypeId tid) const;
  bool Check (void) const;
  bool CheckLoose (void) const;
  /**
   * Attempt to delete this object. This method iterates
   * over all aggregated objects to check if they all 
   * have a zero refcount. If yes, the object and all
   * its aggregates are deleted. If not, nothing is done.
   */
  void MaybeDelete (void) const;
  /**
   * \param tid an TypeId
   *
   * Invoked from ns3::CreateObject only.
   * Initialize the m_tid member variable to
   * keep track of the type of this object instance.
   */
  void SetTypeId (TypeId tid);
   /**
   * \param attributes the attribute values used to initialize 
   *        the member variables of this object's instance.
   *
   * Invoked from ns3::ObjectFactory::Create and ns3::CreateObject only.
   * Initialize all the member variables which were
   * registered with the associated TypeId.
   */
  void Construct (const AttributeList &attributes);

  /**
   * The reference count for this object. Each aggregate
   * has an individual reference count. When the global
   * reference count (the sum of all reference counts) 
   * reaches zero, the object and all its aggregates is 
   * deleted.
   */
  mutable uint32_t m_count;
  /**
   * Identifies the type of this object instance.
   */
  TypeId m_tid;
  /**
   * Set to true when the DoDispose method of the object
   * has run, false otherwise.
   */
  bool m_disposed;
  /**
   * A pointer to the next aggregate object. This is a circular
   * linked list of aggregated objects: the last one points
   * back to the first one. If an object is not aggregated to
   * any other object, the value of this field is equal to the
   * value of the 'this' pointer.
   */
  Object *m_next;
};

} // namespace ns3

namespace ns3 {

/*************************************************************************
 *   The TypeId implementation which depends on templates
 *************************************************************************/

template <typename T>
TypeId 
TypeId::SetParent (void)
{
  return SetParent (T::GetTypeId ());
}

template <typename T>
TypeId 
TypeId::AddConstructor (void)
{
  struct Maker {
    static ObjectBase * Create () {
      ObjectBase * base = new T ();
      return base;
    }
  };
  Callback<ObjectBase *> cb = MakeCallback (&Maker::Create);
  DoAddConstructor (cb);
  return *this;
}

/*************************************************************************
 *   The Object implementation which depends on templates
 *************************************************************************/

void
Object::Ref (void) const
{
  m_count++;
}
void
Object::Unref (void) const
{
  NS_ASSERT (Check ());
  m_count--;
  if (m_count == 0)
    {
      MaybeDelete ();
    }
}

template <typename T>
Ptr<T> 
Object::GetObject () const
{
  Ptr<Object> found = DoGetObject (T::GetTypeId ());
  if (found != 0)
    {
      return Ptr<T> (dynamic_cast<T *> (PeekPointer (found)));
    }
  return 0;
}

template <typename T>
Ptr<T> 
Object::GetObject (TypeId tid) const
{
  Ptr<Object> found = DoGetObject (tid);
  if (found != 0)
    {
      return Ptr<T> (dynamic_cast<T *> (PeekPointer (found)));
    }
  return 0;
}

/*************************************************************************
 *   The helper functions which need templates.
 *************************************************************************/

template <typename T>
Ptr<T> CopyObject (Ptr<T> o)
{
  Ptr<T> p = Ptr<T> (new T (*PeekPointer (o)), false);
  p->SetTypeId (T::GetTypeId ());
  return p;
}


template <typename T>
Ptr<T> CreateObject (const AttributeList &attributes)
{
  Ptr<T> p = Ptr<T> (new T (), false);
  p->SetTypeId (T::GetTypeId ());
  p->Object::Construct (attributes);
  return p;  
}

template <typename T>
Ptr<T> 
CreateObject (std::string n1 = "", Attribute v1 = Attribute (),
              std::string n2 = "", Attribute v2 = Attribute (),
              std::string n3 = "", Attribute v3 = Attribute (),
              std::string n4 = "", Attribute v4 = Attribute (),
              std::string n5 = "", Attribute v5 = Attribute (),
              std::string n6 = "", Attribute v6 = Attribute (),
              std::string n7 = "", Attribute v7 = Attribute (),
              std::string n8 = "", Attribute v8 = Attribute (),
              std::string n9 = "", Attribute v9 = Attribute ())
  
{
  AttributeList attributes;
  if (n1 == "")
    {
      goto end;
    }
  attributes.SetWithTid (T::GetTypeId (), n1, v1);
  if (n2 == "")
    {
      goto end;
    }
  attributes.SetWithTid (T::GetTypeId (), n2, v2);
  if (n3 == "")
    {
      goto end;
    }
  attributes.SetWithTid (T::GetTypeId (), n3, v3);
  if (n4 == "")
    {
      goto end;
    }
  attributes.SetWithTid (T::GetTypeId (), n4, v4);
  if (n5 == "")
    {
      goto end;
    }
  attributes.SetWithTid (T::GetTypeId (), n5, v5);
  if (n6 == "")
    {
      goto end;
    }
  attributes.SetWithTid (T::GetTypeId (), n6, v6);
  if (n7 == "")
    {
      goto end;
    }
  attributes.SetWithTid (T::GetTypeId (), n7, v7);
  if (n8 == "")
    {
      goto end;
    }
  attributes.SetWithTid (T::GetTypeId (), n8, v8);
  if (n9 == "")
    {
      goto end;
    }
  attributes.SetWithTid (T::GetTypeId (), n9, v9);
 end:
  return CreateObject<T> (attributes);
}

} // namespace ns3

#endif /* OBJECT_H */

