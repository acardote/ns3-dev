/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 University of Washington
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
 */

// The queue base class does not have any limit based on the number
// of packets or number of bytes. It is, conceptually, infinite 
// by default. Only subclasses define limitations.
// The base class implements tracing and basic statistics calculations.

#ifndef QUEUE_H
#define QUEUE_H

#include <string>
#include <list>
#include "ns3/packet.h"
#include "ns3/object.h"
#include "ns3/callback-trace-source.h"
#include "ns3/trace-resolver.h"
#include "ns3/trace-context-element.h"

namespace ns3 {

class StringEnumDefaultValue;

/**
 * \brief hold in a TraceContext the type of a trace source 
 */
class QueueTraceType : public TraceContextElement
{
public:
  enum Type {
    ENQUEUE,
    DEQUEUE,
    DROP
  };
  static uint16_t GetUid (void);
  QueueTraceType ();
  QueueTraceType (enum Type type);
  /**
   * \returns true if this is an enqueue event, false otherwise.
   */
  bool IsEnqueue (void) const;
  /**
   * \returns true if this is a dequeue event, false otherwise.
   */
  bool IsDequeue (void) const;
  /**
   * \returns true if this is a drop event, false otherwise.
   */
  bool IsDrop (void) const;
  void Print (std::ostream &os) const;
  std::string GetTypeName (void) const;
private:
  enum Type m_type;
};


/**
 * \brief Abstract base class for packet Queues
 * 
 * This class defines the base APIs for packet queues in the ns-3 system
 */
class Queue : public Object
{
public:
  static const InterfaceId iid;

  Queue ();
  virtual ~Queue ();
  
  /**
   * \return true if the queue is empty; false otherwise
   */
  bool IsEmpty (void) const;
  /**
   * Place a packet into the rear of the Queue
   * \return True if the operation was successful; false otherwise
   */
  bool Enqueue (Ptr<Packet> p);
  /**
   * Remove a packet from the front of the Queue
   * \return 0 if the operation was not successful; the packet otherwise.
   */
  Ptr<Packet> Dequeue (void);
  /**
   * Get a copy of the item at the front of the queue without removing it
   * \return 0 if the operation was not successful; the packet otherwise.
   */
  Ptr<Packet> Peek (void) const;

  /**
   * XXX Doesn't do anything right now, think its supposed to flush the queue
   */
  void DequeueAll (void);
  /**
   * \return The number of packets currently stored in the Queue
   */
  uint32_t GetNPackets (void) const;
  /**
   * \return The number of bytes currently occupied by the packets in the Queue
   */
  uint32_t GetNBytes (void) const;

  /**
   * \return The total number of bytes recieved by this Queue since the
   * simulation began, or since ResetStatistics was called, according to 
   * whichever happened more recently
   * 
   */
  uint32_t GetTotalReceivedBytes (void) const;
  /**
   * \return The total number of packets recieved by this Queue since the
   * simulation began, or since ResetStatistics was called, according to 
   * whichever happened more recently
   */
  uint32_t GetTotalReceivedPackets (void) const;
  /**
   * \return The total number of bytes dropped by this Queue since the
   * simulation began, or since ResetStatistics was called, according to 
   * whichever happened more recently
   */
  uint32_t GetTotalDroppedBytes (void) const;
  /**
   * \return The total number of bytes dropped by this Queue since the
   * simulation began, or since ResetStatistics was called, according to 
   * whichever happened more recently
   */
  uint32_t GetTotalDroppedPackets (void) const;
  /**
   * Resets the counts for dropped packets, dropped bytes, recieved packets, and
   * recieved bytes.
   */
  void ResetStatistics (void);

#if 0
  // average calculation requires keeping around
  // a buffer with the date of arrival of past received packets
  // which are within the average window
  // so, it is quite costly to do it all the time.
  // Hence, it is disabled by default and must be explicitely
  // enabled with this method which specifies the size
  // of the average window in time units.
  void EnableRunningAverage (Time averageWindow);
  void DisableRunningAverage (void);
  // average
  double GetQueueSizeAverage (void);
  double GetReceivedBytesPerSecondAverage (void);
  double GetReceivedPacketsPerSecondAverage (void);
  double GetDroppedBytesPerSecondAverage (void);
  double GetDroppedPacketsPerSecondAverage (void);
  // variance
  double GetQueueSizeVariance (void);
  double GetReceivedBytesPerSecondVariance (void);
  double GetReceivedPacketsPerSecondVariance (void);
  double GetDroppedBytesPerSecondVariance (void);
  double GetDroppedPacketsPerSecondVariance (void);
#endif

private:

  virtual bool DoEnqueue (Ptr<Packet> p) = 0;
  virtual Ptr<Packet> DoDequeue (void) = 0;
  virtual Ptr<Packet> DoPeek (void) const = 0;

protected:
  Ptr<TraceResolver> GetTraceResolver (void) const;
  // called by subclasses to notify parent of packet drops.
  void Drop (Ptr<Packet> packet);

private:
  CallbackTraceSource<Ptr<const Packet> > m_traceEnqueue;
  CallbackTraceSource<Ptr<const Packet> > m_traceDequeue;
  CallbackTraceSource<Ptr<const Packet> > m_traceDrop;

  uint32_t m_nBytes;
  uint32_t m_nTotalReceivedBytes;
  uint32_t m_nPackets;
  uint32_t m_nTotalReceivedPackets;
  uint32_t m_nTotalDroppedBytes;
  uint32_t m_nTotalDroppedPackets;

public:
  /**
   * A factory method to generate a preconfigured default Queue for use
   * \return a Queue smart pointer that is the default Queue type defined
   */
  static Ptr<Queue> CreateDefault (void);
};

}; // namespace ns3

#endif /* QUEUE_H */
