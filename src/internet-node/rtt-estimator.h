/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
// Author: Rajib Bhattacharjea<raj.b@gatech.edu>
//

// Georgia Tech Network Simulator - Round Trip Time Estimation Class
// George F. Riley.  Georgia Tech, Spring 2002

// Implements several variations of round trip time estimators

#ifndef __rtt_estimator_h__
#define __rtt_estimator_h__

#include <deque>
#include "sequence-number.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/type-id-default-value.h"

namespace ns3 {

class RttHistory {
public:
  RttHistory (SequenceNumber s, uint32_t c, Time t);
  RttHistory (const RttHistory& h); // Copy constructor
public:
  SequenceNumber  seq;    // First sequence number in packet sent
  uint32_t        count;  // Number of bytes sent
  Time            time;   // Time this one was sent
  bool            retx;   // True if this has been retransmitted
};

typedef std::deque<RttHistory> RttHistory_t;

class RttEstimator : public Object {  //  Base class for all RTT Estimators
public:
  static TypeId GetTypeId (void);

  RttEstimator();
  RttEstimator(Time e);
  RttEstimator(const RttEstimator&); // Copy constructor
  virtual ~RttEstimator();

  virtual void SentSeq(SequenceNumber, uint32_t);
  virtual Time AckSeq(SequenceNumber);
  virtual void ClearSent();
  virtual void   Measurement(Time t) = 0;
  virtual Time Estimate() = 0;
  virtual Time RetransmitTimeout() = 0;
  void Init(SequenceNumber s) { next = s;}
  virtual Ptr<RttEstimator> Copy() const = 0;
  virtual void IncreaseMultiplier();
  virtual void ResetMultiplier();
  virtual void Reset();

private:
  SequenceNumber        next;    // Next expected sequence to be sent
  RttHistory_t history; // List of sent packet
public:
  Time       est;     // Current estimate
  uint32_t      nSamples;// Number of samples
  double       multiplier;   // RTO Multiplier
public:
  static void InitialEstimate(Time);
  static Ptr<RttEstimator> CreateDefault();      // Retrieve current default

  static TypeIdDefaultValue defaultTid;
  static NumericDefaultValue<double> defaultMaxMultiplier;

private:
  static Time initialEstimate;       // Default initial estimate
};

// The "Mean-Deviation" estimator, as discussed by Van Jacobson
// "Congestion Avoidance and Control", SIGCOMM 88, Appendix A

  //Doc:Class Class {\tt RttMeanDeviation} implements the "Mean--Deviation" estimator
  //Doc:Class as described by Van Jacobson 
  //Doc:Class "Congestion Avoidance and Control", SIGCOMM 88, Appendix A
class RttMeanDeviation : public RttEstimator {
public :
  static TypeId GetTypeId (void);

    //Doc:Desc Constructor for {\tt RttMeanDeviation} specifying the gain factor for the
    //Doc:Desc estimator.
    //Doc:Arg1 Gain factor.
  RttMeanDeviation (double g);

    //Doc:Desc Constructor for {\tt RttMeanDeviation} specifying the gain factor
    //Doc:Desc and the initial estimate.
    //Doc:Arg1 Gain factor.
    //Doc:Arg2 Initial estimate.
  RttMeanDeviation (double g, Time e);

  //Doc:Method
  RttMeanDeviation (const RttMeanDeviation&); // Copy constructor
    //Doc:Desc Copy constructor.
    //Doc:Arg1 {\tt RttMeanDeviation} object to copy.

  void Measurement (Time);
  Time Estimate () { return est;}
  Time Variance () { return variance;}
  Time RetransmitTimeout ();
  Ptr<RttEstimator> Copy () const;
  void Reset ();
  void Gain (double g) { gain = g;}

public:
  double       gain;       // Filter gain
  Time       variance;   // Current variance
};
}//namespace ns3
#endif



