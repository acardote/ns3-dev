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
#include "random-walk-2d-mobility-model.h"
#include "ns3/default-value.h"
#include "ns3/time-default-value.h"
#include "ns3/rectangle-default-value.h"
#include "ns3/random-variable-default-value.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <cmath>

NS_LOG_COMPONENT_DEFINE ("RandomWalk2d");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (RandomWalk2dMobilityModel);

static EnumDefaultValue<RandomWalk2dMobilityModelParameters::Mode> 
g_mode ("RandomWalk2dMode",
        "The mode indicates the condition used to "
        "change the current speed and direction",
        RandomWalk2dMobilityModelParameters::MODE_DISTANCE, "Distance",
        RandomWalk2dMobilityModelParameters::MODE_TIME, "Time",
        0, (void*)0);

static NumericDefaultValue<double>
g_modeDistance ("RandomWalk2dDistance",
                "Change current direction and speed after moving this distance.",
                2.0);

static TimeDefaultValue
g_modeTime ("RandomWalk2dTime",
             "Change current direction and speed after moving for this delay.",
             Seconds (1.0));

static RandomVariableDefaultValue
g_speed ("RandomWalk2dSpeed",
         "A random variable used to pick the speed.",
         "Uniform:2:4");
static RandomVariableDefaultValue
g_direction ("RandomWalk2dDirection",
             "A random variable used to pick the direction (gradients).",
             "Uniform:0.0:6.283184");

static RectangleDefaultValue
g_rectangle ("RandomWalk2dBounds",
             "Bounds of the area to cruise.",
             0.0, 0.0, 100.0, 100.0);

RandomWalk2dMobilityModelParameters::RandomWalk2dMobilityModelParameters ()
  : m_mode (g_mode.GetValue ()),
    m_modeDistance (g_modeDistance.GetValue ()),
    m_modeTime (g_modeTime.GetValue ()),
    m_speed (g_speed.Get ()),
    m_direction (g_direction.Get ()),
    m_bounds (g_rectangle.GetValue ())
{}

RandomWalk2dMobilityModelParameters::~RandomWalk2dMobilityModelParameters ()
{}

void 
RandomWalk2dMobilityModelParameters::SetSpeed (const RandomVariable &speed)
{
  m_speed = speed;
}
void 
RandomWalk2dMobilityModelParameters::SetDirection (const RandomVariable &direction)
{
  m_direction = direction;
}
void 
RandomWalk2dMobilityModelParameters::SetModeDistance (double distance)
{
  m_mode = RandomWalk2dMobilityModelParameters::MODE_DISTANCE;
  m_modeDistance = distance;
}
void 
RandomWalk2dMobilityModelParameters::SetModeTime (Time time)
{
  m_mode = RandomWalk2dMobilityModelParameters::MODE_TIME;
  m_modeTime = time;
}
void 
RandomWalk2dMobilityModelParameters::SetBounds (const Rectangle &bounds)
{
  m_bounds = bounds;
}

Ptr<RandomWalk2dMobilityModelParameters> 
RandomWalk2dMobilityModelParameters::GetCurrent (void)
{
  static Ptr<RandomWalk2dMobilityModelParameters> parameters = 0;
  if (parameters == 0 ||
      g_speed.IsDirty () ||
      g_direction.IsDirty () ||
      g_mode.IsDirty () ||
      g_modeDistance.IsDirty () ||
      g_modeTime.IsDirty () ||
      g_rectangle.IsDirty ())
    {
      parameters = CreateObject<RandomWalk2dMobilityModelParameters> ();
    }
  return parameters;
}

TypeId
RandomWalk2dMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("RandomWalkMobilityModel")
    .SetParent<MobilityModel> ()
    .AddConstructor<RandomWalk2dMobilityModel> ()
    .AddConstructor<RandomWalk2dMobilityModel,Ptr<RandomWalk2dMobilityModelParameters> > ();
  return tid;
}

RandomWalk2dMobilityModel::RandomWalk2dMobilityModel ()
  : m_parameters (RandomWalk2dMobilityModelParameters::GetCurrent ())
{
  m_event = Simulator::ScheduleNow (&RandomWalk2dMobilityModel::Start, this);
}

RandomWalk2dMobilityModel::RandomWalk2dMobilityModel (Ptr<RandomWalk2dMobilityModelParameters> parameters)
  : m_parameters (parameters)
{
  m_event = Simulator::ScheduleNow (&RandomWalk2dMobilityModel::Start, this);
}

void
RandomWalk2dMobilityModel::Start (void)
{
  double speed = m_parameters->m_speed.GetValue ();
  double direction = m_parameters->m_direction.GetValue ();
  Vector vector (std::cos (direction) * speed,
                 std::sin (direction) * speed,
                 0.0);
  m_helper.Reset (vector);

  Time delayLeft;
  if (m_parameters->m_mode == RandomWalk2dMobilityModelParameters::MODE_TIME)
    {
      delayLeft = m_parameters->m_modeTime;
    }
  else
    {
      delayLeft = Seconds (m_parameters->m_modeDistance / speed); 
    }
  DoWalk (delayLeft);
}

void
RandomWalk2dMobilityModel::DoWalk (Time delayLeft)
{
  Vector position = m_helper.GetCurrentPosition ();
  Vector speed = m_helper.GetVelocity ();
  Vector nextPosition = position;
  nextPosition.x += speed.x * delayLeft.GetSeconds ();
  nextPosition.y += speed.y * delayLeft.GetSeconds ();
  if (m_parameters->m_bounds.IsInside (nextPosition))
    {
      m_event = Simulator::Schedule (delayLeft, &RandomWalk2dMobilityModel::Start, this);
    }
  else
    {
      nextPosition = m_parameters->m_bounds.CalculateIntersection (position, speed);
      Time delay = Seconds ((nextPosition.x - position.x) / speed.x);
      m_event = Simulator::Schedule (delay, &RandomWalk2dMobilityModel::Rebound, this,
                                     delayLeft - delay);      
    }  
  NotifyCourseChange ();
}

void
RandomWalk2dMobilityModel::Rebound (Time delayLeft)
{
  Vector position = m_helper.GetCurrentPosition (m_parameters->m_bounds);
  Vector speed = m_helper.GetVelocity ();
  switch (m_parameters->m_bounds.GetClosestSide (position))
    {
    case Rectangle::RIGHT:
    case Rectangle::LEFT:
      speed.x = - speed.x;
      break;
    case Rectangle::TOP:
    case Rectangle::BOTTOM:
      speed.y = - speed.y;
      break;
    }
  m_helper.Reset (speed);
  DoWalk (delayLeft);
}

void
RandomWalk2dMobilityModel::DoDispose (void)
{
  m_parameters = 0;
  // chain up
  MobilityModel::DoDispose ();
}
Vector
RandomWalk2dMobilityModel::DoGetPosition (void) const
{
  return m_helper.GetCurrentPosition (m_parameters->m_bounds);
}
void
RandomWalk2dMobilityModel::DoSetPosition (const Vector &position)
{
  NS_ASSERT (m_parameters->m_bounds.IsInside (position));
  m_helper.InitializePosition (position);
  Simulator::Remove (m_event);
  m_event = Simulator::ScheduleNow (&RandomWalk2dMobilityModel::Start, this);
}
Vector
RandomWalk2dMobilityModel::DoGetVelocity (void) const
{
  return m_helper.GetVelocity ();
}



} // namespace ns3
