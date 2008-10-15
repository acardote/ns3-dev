/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 University of Washington
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

#ifndef REALTIME_SIMULATOR_H
#define REALTIME_SIMULATOR_H

#include "simulator.h"
#include "make-event.h"

namespace ns3 {

/**
 * \ingroup simulator
 *
 * \brief Extension class to control the scheduling of real-time simulation 
 * events.  Intended to be used by threads driven by "external" system
 * events and will schedule events using the current real-time instead of 
 * the current simulation time.
 */
class RealtimeSimulator 
{
public:
  /**
   * Schedule an event to expire at the relative real-time "time"
   * is reached.  This can be thought of as scheduling an event
   * for the current real-time plus the Time passed as a parameter
   *
   * When the event expires (when it becomes due to be run), the 
   * input method will be invoked on the input object.  
   *
   * @param time the relative expiration time of the event.
   * @param mem_ptr member method pointer to invoke
   * @param obj the object on which to invoke the member method
   * @returns an id for the scheduled event.
   */
  template <typename MEM, typename OBJ>
  static void ScheduleRealtime (Time const &time, MEM mem_ptr, OBJ obj);

  /**
   * @param time the relative expiration time of the event.
   * @param mem_ptr member method pointer to invoke
   * @param obj the object on which to invoke the member method
   * @param a1 the first argument to pass to the invoked method
   * @returns an id for the scheduled event.
   */
  template <typename MEM, typename OBJ, typename T1>
  static void ScheduleRealtime (Time const &time, MEM mem_ptr, OBJ obj, T1 a1);

  /**
   * @param time the relative expiration time of the event.
   * @param mem_ptr member method pointer to invoke
   * @param obj the object on which to invoke the member method
   * @param a1 the first argument to pass to the invoked method
   * @param a2 the second argument to pass to the invoked method
   * @returns an id for the scheduled event.
   */
  template <typename MEM, typename OBJ, typename T1, typename T2>
  static void ScheduleRealtime (Time const &time, MEM mem_ptr, OBJ obj, T1 a1, T2 a2);

  /**
   * @param time the relative expiration time of the event.
   * @param mem_ptr member method pointer to invoke
   * @param obj the object on which to invoke the member method
   * @param a1 the first argument to pass to the invoked method
   * @param a2 the second argument to pass to the invoked method
   * @param a3 the third argument to pass to the invoked method
   * @returns an id for the scheduled event.
   */
  template <typename MEM, typename OBJ, 
            typename T1, typename T2, typename T3>
  static void ScheduleRealtime (Time const &time, MEM mem_ptr, OBJ obj, T1 a1, T2 a2, T3 a3);

  /**
   * @param time the relative expiration time of the event.
   * @param mem_ptr member method pointer to invoke
   * @param obj the object on which to invoke the member method
   * @param a1 the first argument to pass to the invoked method
   * @param a2 the second argument to pass to the invoked method
   * @param a3 the third argument to pass to the invoked method
   * @param a4 the fourth argument to pass to the invoked method
   * @returns an id for the scheduled event.
   */
  template <typename MEM, typename OBJ, 
            typename T1, typename T2, typename T3, typename T4>
  static void ScheduleRealtime (Time const &time, MEM mem_ptr, OBJ obj, T1 a1, T2 a2, T3 a3, T4 a4);

  /**
   * @param time the relative expiration time of the event.
   * @param mem_ptr member method pointer to invoke
   * @param obj the object on which to invoke the member method
   * @param a1 the first argument to pass to the invoked method
   * @param a2 the second argument to pass to the invoked method
   * @param a3 the third argument to pass to the invoked method
   * @param a4 the fourth argument to pass to the invoked method
   * @param a5 the fifth argument to pass to the invoked method
   * @returns an id for the scheduled event.
   */
  template <typename MEM, typename OBJ, 
            typename T1, typename T2, typename T3, typename T4, typename T5>
  static void ScheduleRealtime (Time const &time, MEM mem_ptr, OBJ obj, 
                           T1 a1, T2 a2, T3 a3, T4 a4, T5 a5);
  /**
   * @param time the relative expiration time of the event.
   * @param f the function to invoke
   * @returns an id for the scheduled event.
   */
  static void ScheduleRealtime (Time const &time, void (*f) (void));

  /**
   * @param time the relative expiration time of the event.
   * @param f the function to invoke
   * @param a1 the first argument to pass to the function to invoke
   * @returns an id for the scheduled event.
   */
  template <typename U1, typename T1>
  static void ScheduleRealtime (Time const &time, void (*f) (U1), T1 a1);

  /**
   * @param time the relative expiration time of the event.
   * @param f the function to invoke
   * @param a1 the first argument to pass to the function to invoke
   * @param a2 the second argument to pass to the function to invoke
   * @returns an id for the scheduled event.
   */
  template <typename U1, typename U2, typename T1, typename T2>
  static void ScheduleRealtime (Time const &time, void (*f) (U1,U2), T1 a1, T2 a2);

  /**
   * @param time the relative expiration time of the event.
   * @param f the function to invoke
   * @param a1 the first argument to pass to the function to invoke
   * @param a2 the second argument to pass to the function to invoke
   * @param a3 the third argument to pass to the function to invoke
   * @returns an id for the scheduled event.
   */
  template <typename U1, typename U2, typename U3, typename T1, typename T2, typename T3>
  static void ScheduleRealtime (Time const &time, void (*f) (U1,U2,U3), T1 a1, T2 a2, T3 a3);

  /**
   * @param time the relative expiration time of the event.
   * @param f the function to invoke
   * @param a1 the first argument to pass to the function to invoke
   * @param a2 the second argument to pass to the function to invoke
   * @param a3 the third argument to pass to the function to invoke
   * @param a4 the fourth argument to pass to the function to invoke
   * @returns an id for the scheduled event.
   */
  template <typename U1, typename U2, typename U3, typename U4, 
            typename T1, typename T2, typename T3, typename T4>
  static void ScheduleRealtime (Time const &time, void (*f) (U1,U2,U3,U4), T1 a1, T2 a2, T3 a3, T4 a4);

  /**
   * @param time the relative expiration time of the event.
   * @param f the function to invoke
   * @param a1 the first argument to pass to the function to invoke
   * @param a2 the second argument to pass to the function to invoke
   * @param a3 the third argument to pass to the function to invoke
   * @param a4 the fourth argument to pass to the function to invoke
   * @param a5 the fifth argument to pass to the function to invoke
   * @returns an id for the scheduled event.
   */
  template <typename U1, typename U2, typename U3, typename U4, typename U5,
            typename T1, typename T2, typename T3, typename T4, typename T5>
  static void ScheduleRealtime (Time const &time, void (*f) (U1,U2,U3,U4,U5), T1 a1, T2 a2, T3 a3, T4 a4, T5 a5);

  /**
   * Schedule an event to expire Now. All events scheduled to
   * to expire "Now" are scheduled FIFO, after all normal events
   * have expired. 
   *
   * @param mem_ptr member method pointer to invoke
   * @param obj the object on which to invoke the member method
   */
  template <typename MEM, typename OBJ>
  static void ScheduleRealtimeNow (MEM mem_ptr, OBJ obj);

  /**
   * @param mem_ptr member method pointer to invoke
   * @param obj the object on which to invoke the member method
   * @param a1 the first argument to pass to the invoked method
   */
  template <typename MEM, typename OBJ, 
            typename T1>
  static void ScheduleRealtimeNow (MEM mem_ptr, OBJ obj, T1 a1);

  /**
   * @param mem_ptr member method pointer to invoke
   * @param obj the object on which to invoke the member method
   * @param a1 the first argument to pass to the invoked method
   * @param a2 the second argument to pass to the invoked method
   */
  template <typename MEM, typename OBJ, 
            typename T1, typename T2>
  static void ScheduleRealtimeNow (MEM mem_ptr, OBJ obj, T1 a1, T2 a2);

  /**
   * @param mem_ptr member method pointer to invoke
   * @param obj the object on which to invoke the member method
   * @param a1 the first argument to pass to the invoked method
   * @param a2 the second argument to pass to the invoked method
   * @param a3 the third argument to pass to the invoked method
   */
  template <typename MEM, typename OBJ, 
            typename T1, typename T2, typename T3>
  static void ScheduleRealtimeNow (MEM mem_ptr, OBJ obj, T1 a1, T2 a2, T3 a3);

  /**
   * @param mem_ptr member method pointer to invoke
   * @param obj the object on which to invoke the member method
   * @param a1 the first argument to pass to the invoked method
   * @param a2 the second argument to pass to the invoked method
   * @param a3 the third argument to pass to the invoked method
   * @param a4 the fourth argument to pass to the invoked method
   */
  template <typename MEM, typename OBJ, 
            typename T1, typename T2, typename T3, typename T4>
  static void ScheduleRealtimeNow (MEM mem_ptr, OBJ obj, 
                              T1 a1, T2 a2, T3 a3, T4 a4);
  /**
   * @param mem_ptr member method pointer to invoke
   * @param obj the object on which to invoke the member method
   * @param a1 the first argument to pass to the invoked method
   * @param a2 the second argument to pass to the invoked method
   * @param a3 the third argument to pass to the invoked method
   * @param a4 the fourth argument to pass to the invoked method
   * @param a5 the fifth argument to pass to the invoked method
   */
  template <typename MEM, typename OBJ, 
            typename T1, typename T2, typename T3, typename T4, typename T5>
  static void ScheduleRealtimeNow (MEM mem_ptr, OBJ obj, 
                              T1 a1, T2 a2, T3 a3, T4 a4, T5 a5);
  /**
   * @param f the function to invoke
   */
  static void ScheduleRealtimeNow (void (*f) (void));

  /**
   * @param f the function to invoke
   * @param a1 the first argument to pass to the function to invoke
   */
  template <typename U1,
            typename T1>
  static void ScheduleRealtimeNow (void (*f) (U1), T1 a1);

  /**
   * @param f the function to invoke
   * @param a1 the first argument to pass to the function to invoke
   * @param a2 the second argument to pass to the function to invoke
   */
  template <typename U1, typename U2,
            typename T1, typename T2>
  static void ScheduleRealtimeNow (void (*f) (U1,U2), T1 a1, T2 a2);

  /**
   * @param f the function to invoke
   * @param a1 the first argument to pass to the function to invoke
   * @param a2 the second argument to pass to the function to invoke
   * @param a3 the third argument to pass to the function to invoke
   */
  template <typename U1, typename U2, typename U3,
            typename T1, typename T2, typename T3>
  static void ScheduleRealtimeNow (void (*f) (U1,U2,U3), T1 a1, T2 a2, T3 a3);

  /**
   * @param f the function to invoke
   * @param a1 the first argument to pass to the function to invoke
   * @param a2 the second argument to pass to the function to invoke
   * @param a3 the third argument to pass to the function to invoke
   * @param a4 the fourth argument to pass to the function to invoke
   */
  template <typename U1, typename U2, typename U3, typename U4,
            typename T1, typename T2, typename T3, typename T4>
  static void ScheduleRealtimeNow (void (*f) (U1,U2,U3,U4), T1 a1, T2 a2, T3 a3, T4 a4);

  /**
   * @param f the function to invoke
   * @param a1 the first argument to pass to the function to invoke
   * @param a2 the second argument to pass to the function to invoke
   * @param a3 the third argument to pass to the function to invoke
   * @param a4 the fourth argument to pass to the function to invoke
   * @param a5 the fifth argument to pass to the function to invoke
   */
  template <typename U1, typename U2, typename U3, typename U4, typename U5,
            typename T1, typename T2, typename T3, typename T4, typename T5>
  static void ScheduleRealtimeNow (void (*f) (U1,U2,U3,U4,U5), T1 a1, T2 a2, T3 a3, T4 a4, T5 a5);

private:
  RealtimeSimulator ();
  ~RealtimeSimulator ();
  static void ScheduleRealtime (const Time &delay, EventImpl *impl);
  static void ScheduleRealtimeNow (EventImpl *impl);
};

template <typename MEM, typename OBJ>
void RealtimeSimulator::ScheduleRealtime (Time const &time, MEM mem_ptr, OBJ obj) 
{
  return ScheduleRealtime (time, MakeEvent (mem_ptr, obj));
}

template <typename MEM, typename OBJ,
          typename T1>
void RealtimeScheduleRealtime (Time const &time, MEM mem_ptr, OBJ obj, T1 a1) 
{
  return ScheduleRealtime (time, MakeEvent (mem_ptr, obj, a1));
}

template <typename MEM, typename OBJ, 
          typename T1, typename T2>
void RealtimeScheduleRealtime (Time const &time, MEM mem_ptr, OBJ obj, T1 a1, T2 a2)
{
  return ScheduleRealtime (time, MakeEvent (mem_ptr, obj, a1, a2));
}

template <typename MEM, typename OBJ,
          typename T1, typename T2, typename T3>
void RealtimeScheduleRealtime (Time const &time, MEM mem_ptr, OBJ obj, T1 a1, T2 a2, T3 a3) 
{
  return ScheduleRealtime (time, MakeEvent (mem_ptr, obj, a1, a2, a3));
}

template <typename MEM, typename OBJ, 
          typename T1, typename T2, typename T3, typename T4>
void RealtimeScheduleRealtime (Time const &time, MEM mem_ptr, OBJ obj, T1 a1, T2 a2, T3 a3, T4 a4) 
{
  return ScheduleRealtime (time, MakeEvent (mem_ptr, obj, a1, a2, a3, a4));
}

template <typename MEM, typename OBJ, 
          typename T1, typename T2, typename T3, typename T4, typename T5>
void RealtimeScheduleRealtime (Time const &time, MEM mem_ptr, OBJ obj, 
                             T1 a1, T2 a2, T3 a3, T4 a4, T5 a5) 
{
  return ScheduleRealtime (time, MakeEvent (mem_ptr, obj, a1, a2, a3, a4, a5));
}

template <typename U1, typename T1>
void RealtimeScheduleRealtime (Time const &time, void (*f) (U1), T1 a1) 
{
  return ScheduleRealtime (time, MakeEvent (f, a1));
}

template <typename U1, typename U2, 
          typename T1, typename T2>
void RealtimeScheduleRealtime (Time const &time, void (*f) (U1,U2), T1 a1, T2 a2) 
{
  return ScheduleRealtime (time, MakeEvent (f, a1, a2));
}

template <typename U1, typename U2, typename U3,
          typename T1, typename T2, typename T3>
void RealtimeScheduleRealtime (Time const &time, void (*f) (U1,U2,U3), T1 a1, T2 a2, T3 a3)
{
  return ScheduleRealtime (time, MakeEvent (f, a1, a2, a3));
}

template <typename U1, typename U2, typename U3, typename U4,
          typename T1, typename T2, typename T3, typename T4>
void RealtimeScheduleRealtime (Time const &time, void (*f) (U1,U2,U3,U4), T1 a1, T2 a2, T3 a3, T4 a4) 
{
  return ScheduleRealtime (time, MakeEvent (f, a1, a2, a3, a4));
}

template <typename U1, typename U2, typename U3, typename U4, typename U5,
          typename T1, typename T2, typename T3, typename T4, typename T5>
void RealtimeScheduleRealtime (Time const &time, void (*f) (U1,U2,U3,U4,U5), T1 a1, T2 a2, T3 a3, T4 a4, T5 a5) 
{
  return ScheduleRealtime (time, MakeEvent (f, a1, a2, a3, a4, a5));
}

template <typename MEM, typename OBJ>
void
RealtimeScheduleRealtimeNow (MEM mem_ptr, OBJ obj) 
{
  return ScheduleRealtimeNow (MakeEvent (mem_ptr, obj));
}

template <typename MEM, typename OBJ, 
          typename T1>
void
RealtimeScheduleRealtimeNow (MEM mem_ptr, OBJ obj, T1 a1) 
{
  return ScheduleRealtimeNow (MakeEvent (mem_ptr, obj, a1));
}

template <typename MEM, typename OBJ, 
          typename T1, typename T2>
void
RealtimeScheduleRealtimeNow (MEM mem_ptr, OBJ obj, T1 a1, T2 a2) 
{
  return ScheduleRealtimeNow (MakeEvent (mem_ptr, obj, a1, a2));
}

template <typename MEM, typename OBJ, 
          typename T1, typename T2, typename T3>
void
RealtimeScheduleRealtimeNow (MEM mem_ptr, OBJ obj, T1 a1, T2 a2, T3 a3) 
{
  return ScheduleRealtimeNow (MakeEvent (mem_ptr, obj, a1, a2, a3));
}

template <typename MEM, typename OBJ, 
          typename T1, typename T2, typename T3, typename T4>
void
RealtimeScheduleRealtimeNow (MEM mem_ptr, OBJ obj, T1 a1, T2 a2, T3 a3, T4 a4) 
{
  return ScheduleRealtimeNow (MakeEvent (mem_ptr, obj, a1, a2, a3, a4));
}

template <typename MEM, typename OBJ, 
          typename T1, typename T2, typename T3, typename T4, typename T5>
void
RealtimeScheduleRealtimeNow (MEM mem_ptr, OBJ obj, 
                        T1 a1, T2 a2, T3 a3, T4 a4, T5 a5) 
{
  return ScheduleRealtimeNow (MakeEvent (mem_ptr, obj, a1, a2, a3, a4, a5));
}

template <typename U1,
          typename T1>
void
RealtimeScheduleRealtimeNow (void (*f) (U1), T1 a1) 
{
  return ScheduleRealtimeNow (MakeEvent (f, a1));
}

template <typename U1, typename U2,
          typename T1, typename T2>
void
RealtimeScheduleRealtimeNow (void (*f) (U1,U2), T1 a1, T2 a2) 
{
  return ScheduleRealtimeNow (MakeEvent (f, a1, a2));
}

template <typename U1, typename U2, typename U3,
          typename T1, typename T2, typename T3>
void
RealtimeScheduleRealtimeNow (void (*f) (U1,U2,U3), T1 a1, T2 a2, T3 a3)
{
  return ScheduleRealtimeNow (MakeEvent (f, a1, a2, a3));
}

template <typename U1, typename U2, typename U3, typename U4,
          typename T1, typename T2, typename T3, typename T4>
void
RealtimeScheduleRealtimeNow (void (*f) (U1,U2,U3,U4), T1 a1, T2 a2, T3 a3, T4 a4) 
{
  return ScheduleRealtimeNow (MakeEvent (f, a1, a2, a3, a4));
}

template <typename U1, typename U2, typename U3, typename U4, typename U5,
          typename T1, typename T2, typename T3, typename T4, typename T5>
void
RealtimeScheduleRealtimeNow (void (*f) (U1,U2,U3,U4,U5), T1 a1, T2 a2, T3 a3, T4 a4, T5 a5) 
{
  return ScheduleRealtimeNow (MakeEvent (f, a1, a2, a3, a4, a5));
}

} // namespace ns3

#endif /* REALTIME_SIMULATOR_H */
