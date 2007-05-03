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
#ifndef INTERFACE_H
#define INTERFACE_H

#include <string>


namespace ns3 {

class NsUnknownImpl;

/**
 * \brief COM-like IUnknown
 *
 * This class should be used as a base class for every object which
 * wishes to provide a COM-like QueryInterface API. Multiple 
 * inheritance where this base class is at the top of the dreaded 
 * "diamond" shape is not allowed.
 */
class NsUnknown
{
public:
  virtual ~NsUnknown ();
  void Ref (void);
  void Unref (void);

  /**
   * \param iid the NsUnknown id of the requested interface
   */
  template <typename T>
  T *QueryInterface (uint32_t iid);

  /**
   * \param interface another interface
   * 
   * Aggregate together the two interfaces. After this call,
   * the two interface objects are tied together: each of them
   * will be able to perform QI on each other and their lifetimes
   * will be found by the same reference count.
   */
  void AddInterface (NsUnknown *interface);

  virtual void Dispose (void);
protected:
  /**
   * \param iid the Interface Id of the interface defined by a direct subclass
   * of this base class
   *
   * If you are a direct subclass of this class, you _must_ register
   * the name of your interface with this constructor.
   */
  NsUnknown (uint32_t iid);
  /**
   * \param iid the Interface id of the interface
   * \param a pointer to the interface object
   *
   * If you are not a direct subclass of the ns3::NsUnknown base class,
   * and if you want to register yourself as another accessible interface
   * (typically, your subclass has added API), you need to call
   * this method to associate an interface id to your interface.
   */
  void AddSelfInterface (uint32_t iid, NsUnknown *interface);
private:
  friend class NsUnknownImpl;
  NsUnknown ();
  NsUnknown *DoQueryInterface (uint32_t iid);
  void RefInternal (void);
  void UnrefInternal (void);
  NsUnknownImpl *m_impl;
  uint32_t m_ref;
};

}//namespace ns3

namespace ns3 {

template <typename T>
T *
NsUnknown::QueryInterface (uint32_t iid)
{
  NsUnknown *found = DoQueryInterface (iid);
  if (found != 0)
    {
      return dynamic_cast<T *> (found);
    }
  return 0;
}


}//namespace ns3

#endif /* INTERFACE_H */
