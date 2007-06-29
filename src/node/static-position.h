/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006,2007 INRIA
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
#ifndef STATIC_POSITION_H
#define STATIC_POSITION_H

#include "position.h"

namespace ns3 {

/**
 * \brief a position model for which the current position does not
 *        change once it has been set and until it is set again 
 *        explicitely to a new value.
 */
class StaticPosition : public Position 
{
public:
  static const InterfaceId iid;
  /**
   * Create a position located at coordinates (0,0,0)
   */
  StaticPosition ();
  /**
   * \param x x coordinate
   * \param y y coordinate
   * \param z z coordinate
   *
   * Create a position located at coordinates (x,y,z).
   * Unit is meters
   */
  StaticPosition (double x, double y, double z);
  virtual ~StaticPosition ();

  /**
   * \param x x coordinate
   * \param y y coordinate
   * \param z z coordinate
   *
   * Set all 3 coordinates at the same time.
   * Unit is meters
   */
  void Set (double x, double y, double z);
  /**
   * \param x x coordinate
   *
   * Set x coordinate. Unit is meters
   */
  void SetX (double x);
  /**
   * \param y y coordinate
   *
   * Set y coordinate. Unit is meters
   */
  void SetY (double y);
  /**
   * \param z z coordinate
   *
   * Set z coordinate. Unit is meters
   */
  void SetZ (double z);
private:
  /**
   * Subclasses must override this virtual method to be notified
   * of a call to one of the Set methods which changes the current
   * position. The default implementation does nothing. This method
   * is invoked _after_ the current position has been updated.
   */
  virtual void NotifyPositionChange (void) const;
  virtual void DoGet (double &x, double &y, double &z) const;
  double m_x;
  double m_y;
  double m_z;
};

}; // namespace ns3

#endif /* STATIC_POSITION_H */
