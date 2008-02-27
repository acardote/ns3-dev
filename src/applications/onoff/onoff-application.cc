/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
//
// Copyright (c) 2006 Georgia Tech Research Corporation
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: George F. Riley<riley@ece.gatech.edu>
//

// ns3 - On/Off Data Source Application class
// George F. Riley, Georgia Tech, Spring 2007
// Adapted from ApplicationOnOff in GTNetS.

#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/random-variable.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "onoff-application.h"
#include "ns3/udp.h"

NS_LOG_COMPONENT_DEFINE ("OnOffApplication");

using namespace std;

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (OnOffApplication);

TypeId
OnOffApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("OnOffApplication")
    .SetParent<Application> ()
    .AddConstructor<OnOffApplication> ()
    .AddAttribute ("DataRate", "The data rate in on state.",
                   DataRate ("500kb/s"),
                   MakeDataRateAccessor (&OnOffApplication::m_cbrRate),
                   MakeDataRateChecker ())
    .AddAttribute ("PacketSize", "The size of packets sent in on state",
                   Uinteger (512),
                   MakeUintegerAccessor (&OnOffApplication::m_pktSize),
                   MakeUintegerChecker<uint32_t> (1))
    .AddAttribute ("Remote", "The address of the destination",
                   Address (),
                   MakeAddressAccessor (&OnOffApplication::m_peer),
                   MakeAddressChecker ())
    .AddAttribute ("OnTime", "A RandomVariable used to pick the duration of the 'On' state.",
                   ConstantVariable (1.0),
                   MakeRandomVariableAccessor (&OnOffApplication::m_onTime),
                   MakeRandomVariableChecker ())
    .AddAttribute ("OffTime", "A RandomVariable used to pick the duration of the 'Off' state.",
                   ConstantVariable (1.0),
                   MakeRandomVariableAccessor (&OnOffApplication::m_offTime),
                   MakeRandomVariableChecker ())
    .AddAttribute ("Protocol", "The type of protocol to use.",
                   Udp::GetTypeId (),
                   MakeTypeIdAccessor (&OnOffApplication::m_tid),
                   MakeTypeIdChecker ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&OnOffApplication::m_txTrace))
    ;
  return tid;
}


OnOffApplication::OnOffApplication ()
{
  NS_LOG_FUNCTION;
  m_socket = 0;
  m_connected = false;
  m_residualBits = 0;
  m_lastStartTime = Seconds (0);
  m_maxBytes = 0;
  m_totBytes = 0;
}

OnOffApplication::~OnOffApplication()
{
  NS_LOG_FUNCTION;
}

void 
OnOffApplication::SetMaxBytes(uint32_t maxBytes)
{
  NS_LOG_FUNCTION;
  NS_LOG_PARAMS (this << maxBytes);
  m_maxBytes = maxBytes;
}


void
OnOffApplication::DoDispose (void)
{
  NS_LOG_FUNCTION;

  m_socket = 0;
  // chain up
  Application::DoDispose ();
}

// Application Methods
void OnOffApplication::StartApplication() // Called at time specified by Start
{
  NS_LOG_FUNCTION;

  // Create the socket if not already
  if (!m_socket)
    {
      Ptr<SocketFactory> socketFactory = GetNode ()->GetObject<SocketFactory> (m_tid);
      m_socket = socketFactory->CreateSocket ();
      m_socket->Bind ();
      m_socket->Connect (m_peer);
    }
  // Insure no pending event
  StopApplication();
  // If we are not yet connected, there is nothing to do here
  // The ConnectionComplete upcall will start timers at that time
  //if (!m_connected) return;
  ScheduleStartEvent();
}

void OnOffApplication::StopApplication() // Called at time specified by Stop
{
  NS_LOG_FUNCTION;

  if (m_sendEvent.IsRunning ())
    { // Cancel the pending send packet event
      // Calculate residual bits since last packet sent
      Time delta(Simulator::Now() - m_lastStartTime);
      m_residualBits += (uint32_t)(m_cbrRate.GetBitRate() * delta.GetSeconds());
    }
  Simulator::Cancel(m_sendEvent);
  Simulator::Cancel(m_startStopEvent);
}

// Event handlers
void OnOffApplication::StartSending()
{
  NS_LOG_FUNCTION;

  ScheduleNextTx();  // Schedule the send packet event
}

void OnOffApplication::StopSending()
{
  NS_LOG_FUNCTION;

  Simulator::Cancel(m_sendEvent);
}

// Private helpers
void OnOffApplication::ScheduleNextTx()
{
  NS_LOG_FUNCTION;

  if (m_maxBytes == 0 || m_totBytes < m_maxBytes)
    {
      uint32_t bits = m_pktSize * 8 - m_residualBits;
      NS_LOG_LOGIC ("bits = " << bits);
      Time nextTime(Seconds (bits / 
        static_cast<double>(m_cbrRate.GetBitRate()))); // Time till next packet
      NS_LOG_LOGIC ("nextTime = " << nextTime);
      m_sendEvent = Simulator::Schedule(nextTime, 
                                        &OnOffApplication::SendPacket, this);
    }
  else
    { // All done, cancel any pending events
      StopApplication();
    }
}

void OnOffApplication::ScheduleStartEvent()
{  // Schedules the event to start sending data (switch to the "On" state)
  NS_LOG_FUNCTION;

  Time offInterval = Seconds(m_offTime.GetValue());
  NS_LOG_LOGIC ("start at " << offInterval);
  m_startStopEvent = Simulator::Schedule(offInterval, &OnOffApplication::StartSending, this);
}

void OnOffApplication::ScheduleStopEvent()
{  // Schedules the event to stop sending data (switch to "Off" state)
  NS_LOG_FUNCTION;

  Time onInterval = Seconds(m_onTime.GetValue());
  Simulator::Schedule(onInterval, &OnOffApplication::StopSending, this);
}

  
void OnOffApplication::SendPacket()
{
  NS_LOG_FUNCTION;

  NS_ASSERT (m_sendEvent.IsExpired ());
  Ptr<Packet> packet = Create<Packet> (m_pktSize);
  m_txTrace (packet);
  m_socket->Send (packet);
  m_totBytes += m_pktSize;
  m_lastStartTime = Simulator::Now();
  m_residualBits = 0;
  ScheduleNextTx();
}

void OnOffApplication::ConnectionSucceeded(Ptr<Socket>)
{
  NS_LOG_FUNCTION;

  m_connected = true;
  ScheduleStartEvent();
}
  
void OnOffApplication::ConnectionFailed(Ptr<Socket>)
{
  NS_LOG_FUNCTION;
  cout << "OnOffApplication, Connection Failed" << endl;
}

} // Namespace ns3
