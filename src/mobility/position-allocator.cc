/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INRIA
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
#include "position-allocator.h"
#include "ns3/random-variable.h"
#include "ns3/fp-value.h"
#include "ns3/int-value.h"
#include "ns3/enum-value.h"
#include "ns3/log.h"
#include <cmath>

NS_LOG_COMPONENT_DEFINE ("PositionAllocator");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PositionAllocator);

TypeId 
PositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("PositionAllocator")
    .SetParent<Object> ();
  return tid;
}

PositionAllocator::PositionAllocator ()
{
}

PositionAllocator::~PositionAllocator ()
{}

TypeId 
GridPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("GridPositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Mobility")
    .AddConstructor<GridPositionAllocator> ()
    .AddParameter ("GridWidth", "The number of objects layed out on a line.",
                   Integer (10),
                   MakeIntegerAccessor (&GridPositionAllocator::m_n),
                   MakeIntegerChecker<uint32_t> ())
    .AddParameter ("MinX", "The x coordinate where the grid starts.",
                   FpValue (1.0),
                   MakeFpAccessor (&GridPositionAllocator::m_xMin),
                   MakeFpChecker<double> ())
    .AddParameter ("MinY", "The y coordinate where the grid starts.",
                   FpValue (0.0),
                   MakeFpAccessor (&GridPositionAllocator::m_yMin),
                   MakeFpChecker<double> ())
    .AddParameter ("DeltaX", "The x space between objects.",
                   FpValue (1.0),
                   MakeFpAccessor (&GridPositionAllocator::m_deltaX),
                   MakeFpChecker<double> ())
    .AddParameter ("DeltaY", "The y space between objects.",
                   FpValue (1.0),
                   MakeFpAccessor (&GridPositionAllocator::m_deltaY),
                   MakeFpChecker<double> ())
    .AddParameter ("LayoutType", "The type of layout.",
                   Enum (ROW_FIRST),
                   MakeEnumAccessor (&GridPositionAllocator::m_layoutType),
                   MakeEnumChecker (ROW_FIRST, "RowFirst",
                                    COLUMN_FIRST, "ColumnFirst"))
    ;
  return tid;
}
GridPositionAllocator::GridPositionAllocator ()
  : m_current (0)
{}

Vector 
GridPositionAllocator::GetNext (void) const
{
  double x, y;
  switch (m_layoutType) {
  case ROW_FIRST:
    x = m_xMin + m_deltaX * (m_current % m_n);
    y = m_yMin + m_deltaY * (m_current / m_n);
    break;
  case COLUMN_FIRST:
    x = m_xMin + m_deltaX * (m_current / m_n);
    y = m_yMin + m_deltaY * (m_current % m_n);
    break;
  }
  m_current++;
  return Vector (x, y, 0.0);
}


NS_OBJECT_ENSURE_REGISTERED (RandomRectanglePositionAllocator);

TypeId
RandomRectanglePositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("RandomRectanglePositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Mobility")
    .AddConstructor<RandomRectanglePositionAllocator> ()
    .AddParameter ("X",
                   "A random variable which represents the x coordinate of a position in a random rectangle.",
                   UniformVariable (0.0, 1.0),
                   MakeRandomVariableAccessor (&RandomRectanglePositionAllocator::m_x),
                   MakeRandomVariableChecker ())
    .AddParameter ("Y",
                   "A random variable which represents the y coordinate of a position in a random rectangle.",
                   UniformVariable (0.0, 1.0),
                   MakeRandomVariableAccessor (&RandomRectanglePositionAllocator::m_y),
                   MakeRandomVariableChecker ());
  return tid;
}

RandomRectanglePositionAllocator::RandomRectanglePositionAllocator ()
{}
RandomRectanglePositionAllocator::~RandomRectanglePositionAllocator ()
{}
Vector
RandomRectanglePositionAllocator::GetNext (void) const
{
  double x = m_x.GetValue ();
  double y = m_y.GetValue ();
  return Vector (x, y, 0.0);
}

NS_OBJECT_ENSURE_REGISTERED (RandomDiscPositionAllocator);

TypeId
RandomDiscPositionAllocator::GetTypeId (void)
{
  static TypeId tid = TypeId ("RandomDiscPositionAllocator")
    .SetParent<PositionAllocator> ()
    .SetGroupName ("Mobility")
    .AddConstructor<RandomDiscPositionAllocator> ()
    .AddParameter ("Theta",
                   "A random variable which represents the angle (gradients) of a position in a random disc.",
                   UniformVariable (0.0, 6.2830),
                   MakeRandomVariableAccessor (&RandomDiscPositionAllocator::m_theta),
                   MakeRandomVariableChecker ())
    .AddParameter ("Rho",
                   "A random variable which represents the radius of a position in a random disc.",
                   UniformVariable (0.0, 200.0),
                   MakeRandomVariableAccessor (&RandomDiscPositionAllocator::m_rho),
                   MakeRandomVariableChecker ())
    .AddParameter ("X",
                   "The x coordinate of the center of the random position disc.",
                   FpValue (0.0),
                   MakeFpAccessor (&RandomDiscPositionAllocator::m_x),
                   MakeFpChecker<double> ())
    .AddParameter ("Y",
                   "The y coordinate of the center of the random position disc.",
                   FpValue (0.0),
                   MakeFpAccessor (&RandomDiscPositionAllocator::m_y),
                   MakeFpChecker<double> ())
    ;
  return tid;
}   

RandomDiscPositionAllocator::RandomDiscPositionAllocator ()
{}
RandomDiscPositionAllocator::~RandomDiscPositionAllocator ()
{}
Vector
RandomDiscPositionAllocator::GetNext (void) const
{
  double theta = m_theta.GetValue ();
  double rho = m_rho.GetValue ();
  double x = m_x + std::cos (theta) * rho;
  double y = m_y + std::sin (theta) * rho;
  NS_LOG_DEBUG ("Disc position x=" << x << ", y=" << y);
  return Vector (x, y, 0.0);
}


} // namespace ns3 
