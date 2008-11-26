/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 Georgia Tech Research Corporation
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
 * Author: George F. Riley<riley@ece.gatech.edu>
 */

// Implementation for ns3 Application base class.
// George F. Riley, Georgia Tech, Fall 2006

#include "application.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/random-variable.h"
#include "ns3/simulator.h"

using namespace std;

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (Application);

// Application Methods

TypeId 
Application::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Application")
    .SetParent<Object> ()
    ;
  return tid;
}

// \brief Application Constructor
Application::Application()
{}
  
// \brief Application Destructor
Application::~Application()
{}

void
Application::DoDispose (void)
{
  m_node = 0;
  Simulator::Cancel(m_startEvent);
  Simulator::Cancel(m_stopEvent);
  Object::DoDispose ();
}  
   
void Application::Start(const Time& startTime)
{
  ScheduleStart (startTime);
}

void Application::Start(const RandomVariable& startVar)
{
  RandomVariable v = startVar;
  ScheduleStart (Seconds (v.GetValue ()));
}

   
void Application::Stop(const Time& stopTime)
{
  ScheduleStop (stopTime);
}

void Application::Stop(const RandomVariable& stopVar)
{
  RandomVariable v = stopVar;
  ScheduleStop (Seconds (v.GetValue ()));
}
  
Ptr<Node> Application::GetNode() const
{
  return m_node;
}

void 
Application::SetNode (Ptr<Node> node)
{
  m_node = node;
}

// Protected methods
// StartApp and StopApp will likely be overridden by application subclasses
void Application::StartApplication()
{ // Provide null functionality in case subclass is not interested
}

void Application::StopApplication()
{ // Provide null functionality in case subclass is not interested
}


// Private helpers
void Application::ScheduleStart (const Time &startTime)
{
  m_startEvent = Simulator::Schedule (startTime,
                                      &Application::StartApplication, this);
}

void Application::ScheduleStop (const Time &stopTime)
{
  m_stopEvent = Simulator::Schedule (stopTime,
                                    &Application::StopApplication, this);
}

} //namespace ns3
      
  
