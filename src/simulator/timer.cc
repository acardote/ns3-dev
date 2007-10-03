#include "timer.h"
#include "simulator.h"
#include "simulation-singleton.h"
#include "event-garbage-collector.h"

namespace ns3 {

Timer::Timer ()
  : m_flags (0),
    m_delay (FemtoSeconds (0)),
    m_event (),
    m_impl (0)
{}

Timer::Timer (enum SchedulePolicy schedulePolicy, 
	      enum DestroyPolicy destroyPolicy)
  : m_flags (schedulePolicy | destroyPolicy),
    m_delay (FemtoSeconds (0)),
    m_event (),
    m_impl (0)
{}

Timer::Timer (enum GarbageCollectPolicy policy)
  : m_flags (GARBAGE_COLLECT),
    m_delay (FemtoSeconds (0)),
    m_event (),
    m_impl (0)
{}

Timer::~Timer ()
{
  if (m_flags & CHECK_ON_DESTROY)
    {
      if (m_event.IsRunning ())
	{
	  NS_FATAL_ERROR ("Event is still running while destroying.");
	}
    }
  else if (m_flags & CANCEL_ON_DESTROY)
    {
      m_event.Cancel ();
    }
  else if (m_flags & REMOVE_ON_DESTROY)
    {
      Simulator::Remove (m_event);
    }
  delete m_impl;
}

void 
Timer::SetDelay (const Time &time)
{
  m_delay = time;
}
Time 
Timer::GetDelay (void) const
{
  return m_delay;
}


void 
Timer::Cancel (void)
{
  Simulator::Cancel (m_event);
}
void 
Timer::Remove (void)
{
  Simulator::Remove (m_event);
}
bool 
Timer::IsExpired (void) const
{
  return m_event.IsExpired ();
}
bool 
Timer::IsRunning (void) const
{
  return m_event.IsRunning ();
}
bool
Timer::IsSuspended (void) const
{
  return (m_flags & TIMER_SUSPENDED) == TIMER_SUSPENDED;
}

void 
Timer::Schedule (void)
{
  Schedule (m_delay);
}

void 
Timer::Schedule (Time delay)
{
  NS_ASSERT (m_impl != 0);
  if (m_flags & CHECK_ON_SCHEDULE)
    {
      if (m_event.IsRunning ())
	{
	  NS_FATAL_ERROR ("Event is still running while re-scheduling.");
	}
    }
  else if (m_flags & CANCEL_ON_SCHEDULE)
    {
      m_event.Cancel ();
    }
  else if (m_flags & REMOVE_ON_SCHEDULE)
    {
      Simulator::Remove (m_event);
    }
  m_event = m_impl->Schedule (delay);
  if (m_flags & GARBAGE_COLLECT)
    {
      SimulationSingleton<EventGarbageCollector>::Get ()->Track (m_event);
    }
}

void
Timer::Suspend (void)
{
  NS_ASSERT (IsRunning ());
  m_delayLeft = Simulator::GetDelayLeft (m_event);
  Simulator::Remove (m_event);
  m_flags |= TIMER_SUSPENDED;
}

void
Timer::Resume (void)
{
  NS_ASSERT (m_flags & TIMER_SUSPENDED);
  m_event = m_impl->Schedule (m_delayLeft);
  m_flags &= ~TIMER_SUSPENDED;
}


} // namespace ns3


#ifdef RUN_SELF_TESTS
#include "ns3/test.h"

namespace {
void bari (int)
{}
void bar2i (int, int)
{}
void bar3i (int, int, int)
{}
void bar4i (int, int, int, int)
{}
void bar5i (int, int, int, int, int)
{}
void bar6i (int, int, int, int, int, int)
{}
void barcir (const int &)
{}
void barir (int &)
{}
void barip (int *)
{}
void barcip (const int *)
{}
}

namespace ns3 {

class TimerTests : public Test
{
public:
  TimerTests ();
  virtual bool RunTests (void);
  void bazi (int) {}
  void baz2i (int, int) {}
  void baz3i (int, int, int) {}
  void baz4i (int, int, int, int) {}
  void baz5i (int, int, int, int, int) {}
  void baz6i (int, int, int, int, int, int) {}
  void bazcir (const int&) {}
  void bazir (int&) {}
  void bazip (int *) {}
  void bazcip (const int *) {}
};

TimerTests::TimerTests ()
  : Test ("Timer")
{}

bool
TimerTests::RunTests (void)
{
  bool result = true;

  Timer timer;

  timer.SetFunction (&bari);
  timer.SetArguments (1);
  timer.SetDelay (Seconds (10.0));
  NS_TEST_ASSERT (!timer.IsRunning ());
  NS_TEST_ASSERT (timer.IsExpired ());
  NS_TEST_ASSERT (!timer.IsSuspended ());
  timer.Schedule ();
  NS_TEST_ASSERT (timer.IsRunning ());
  NS_TEST_ASSERT (!timer.IsExpired ());
  NS_TEST_ASSERT (!timer.IsSuspended ());
  timer.Suspend ();
  NS_TEST_ASSERT (!timer.IsRunning ());
  NS_TEST_ASSERT (timer.IsExpired ());
  NS_TEST_ASSERT (timer.IsSuspended ());
  timer.Resume ();
  NS_TEST_ASSERT (timer.IsRunning ());
  NS_TEST_ASSERT (!timer.IsExpired ());
  NS_TEST_ASSERT (!timer.IsSuspended ());

  int a = 0;
  int &b = a;
  const int &c = a;

  timer.SetFunction (&bari);
  timer.SetArguments (2);
  timer.SetArguments (a);
  timer.SetArguments (b);
  timer.SetArguments (c);
  timer.SetFunction (&barir);
  timer.SetArguments (2);
  timer.SetArguments (a);
  timer.SetArguments (b);
  timer.SetArguments (c);
  timer.SetFunction (&barcir);
  timer.SetArguments (2);
  timer.SetArguments (a);
  timer.SetArguments (b);
  timer.SetArguments (c);
  // the following call cannot possibly work and is flagged by
  // a runtime error.
  //timer.SetArguments (0.0);
  timer.SetDelay (Seconds (1.0));
  timer.Schedule ();

  timer.SetFunction (&TimerTests::bazi, this);
  timer.SetArguments (3);
  timer.SetFunction (&TimerTests::bazir, this);
  timer.SetArguments (3);
  timer.SetFunction (&TimerTests::bazcir, this);
  timer.SetArguments (3);

  timer.SetFunction (&bar2i);
  timer.SetArguments (1, 1);
  timer.SetFunction (&bar3i);
  timer.SetArguments (1, 1, 1);
  timer.SetFunction (&bar4i);
  timer.SetArguments (1, 1, 1, 1);
  timer.SetFunction (&bar5i);
  timer.SetArguments (1, 1, 1, 1, 1);
  //timer.SetFunction (&bar6i);
  //timer.SetArguments (1, 1, 1, 1, 1, 1);

  timer.SetFunction (&TimerTests::baz2i, this);
  timer.SetArguments (1, 1);
  timer.SetFunction (&TimerTests::baz3i, this);
  timer.SetArguments (1, 1, 1);
  timer.SetFunction (&TimerTests::baz4i, this);
  timer.SetArguments (1, 1, 1, 1);
  timer.SetFunction (&TimerTests::baz5i, this);
  timer.SetArguments (1, 1, 1, 1, 1);
  //timer.SetFunction (&TimerTests::baz6i, this);
  //timer.SetArguments (1, 1, 1, 1, 1, 1);


  Simulator::Run ();
  Simulator::Destroy ();
		  
  return result;
}

TimerTests g_tests;

} // namespace ns3

#endif /* RUN_SELF_TESTS */
