/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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
#ifdef RUN_SELF_TESTS

#include "ns3/test.h"
#include "ns3/simulator.h"
#include "dcf-manager.h"


namespace ns3 {

class DcfManagerTest;

class DcfStateTest : public DcfState 
{
public:
  DcfStateTest (DcfManagerTest *test, uint32_t i);
  void QueueTx (uint64_t txTime, uint64_t expectedGrantTime);
private:
  friend class DcfManagerTest;
  virtual void DoNotifyAccessGranted (void);
  virtual void DoNotifyInternalCollision (void);
  virtual void DoNotifyCollision (void);

  typedef std::pair<uint64_t,uint64_t> ExpectedGrant;
  typedef std::list<ExpectedGrant> ExpectedGrants;
  struct ExpectedCollision {
    uint64_t at;
    uint32_t nSlots;
  };
  typedef std::list<struct ExpectedCollision> ExpectedCollisions;

  ExpectedCollisions m_expectedInternalCollision;
  ExpectedCollisions m_expectedCollision;
  ExpectedGrants m_expectedGrants;
  DcfManagerTest *m_test;
  uint32_t m_i;
};


class DcfManagerTest : public Test
{
public:
  DcfManagerTest ();
  virtual bool RunTests (void);


  void NotifyAccessGranted (uint32_t i);
  void NotifyInternalCollision (uint32_t i);
  void NotifyCollision (uint32_t i);


private:
  void StartTest (uint64_t slotTime, uint64_t sifs, uint64_t eifsNoDifsNoSifs, uint32_t ackTimeoutValue = 20);
  void AddDcfState (uint32_t aifsn);
  void EndTest (void);
  void ExpectInternalCollision (uint64_t time, uint32_t from, uint32_t nSlots);
  void ExpectCollision (uint64_t time, uint32_t from, uint32_t nSlots);
  void AddRxOkEvt (uint64_t at, uint64_t duration);
  void AddRxErrorEvt (uint64_t at, uint64_t duration);
  void AddRxInsideSifsEvt (uint64_t at, uint64_t duration);
  void AddTxEvt (uint64_t at, uint64_t duration);
  void AddNavReset (uint64_t at, uint64_t duration);
  void AddNavStart (uint64_t at, uint64_t duration);
  void AddAckTimeoutReset (uint64_t at);
  void AddAccessRequest (uint64_t at, uint64_t txTime, 
                         uint64_t expectedGrantTime, uint32_t from);
  void AddAccessRequestWithAckTimeout (uint64_t at, uint64_t txTime, 
                                  uint64_t expectedGrantTime, uint32_t from);
  ///\param ackDelay is delay of the ack after txEnd
  void AddAccessRequestWithSuccessfullAck (uint64_t at, uint64_t txTime, 
                                  uint64_t expectedGrantTime, uint32_t ackDelay, uint32_t from);
  void DoAccessRequest (uint64_t txTime, uint64_t expectedGrantTime, DcfStateTest *state);
  
  typedef std::vector<DcfStateTest *> DcfStates;

  DcfManager *m_dcfManager;
  DcfStates m_dcfStates;
  uint32_t m_ackTimeoutValue;
  bool m_result;
};



DcfStateTest::DcfStateTest (DcfManagerTest *test, uint32_t i) 
  : m_test (test), m_i(i)
{}
void 
DcfStateTest::QueueTx (uint64_t txTime, uint64_t expectedGrantTime)
{
  m_expectedGrants.push_back (std::make_pair (txTime, expectedGrantTime));
}
void 
DcfStateTest::DoNotifyAccessGranted (void)
{
  m_test->NotifyAccessGranted (m_i);
}
void
DcfStateTest::DoNotifyInternalCollision (void)
{
  m_test->NotifyInternalCollision (m_i);
}
void 
DcfStateTest::DoNotifyCollision (void)
{
  m_test->NotifyCollision (m_i);
}



DcfManagerTest::DcfManagerTest ()
  : Test ("DcfManager")
{}

void 
DcfManagerTest::NotifyAccessGranted (uint32_t i)
{
  DcfStateTest *state = m_dcfStates[i];
  bool result = true;
  NS_TEST_ASSERT (!state->m_expectedGrants.empty ());
  std::pair<uint64_t, uint64_t> expected = state->m_expectedGrants.front ();
  state->m_expectedGrants.pop_front ();
  NS_TEST_ASSERT_EQUAL (Simulator::Now (), MicroSeconds (expected.second));
  m_dcfManager->NotifyTxStartNow (MicroSeconds (expected.first));
  m_dcfManager->NotifyAckTimeoutStartNow (MicroSeconds (m_ackTimeoutValue + expected.first));
  if (!result)
    {
      m_result = result;
    }
}
void
DcfManagerTest::AddTxEvt (uint64_t at, uint64_t duration)
{
  Simulator::Schedule (MicroSeconds (at) - Now (), 
                       &DcfManager::NotifyTxStartNow, m_dcfManager, 
                       MicroSeconds (duration));
}
void 
DcfManagerTest::NotifyInternalCollision (uint32_t i)
{
  DcfStateTest *state = m_dcfStates[i];
  bool result = true;
  NS_TEST_ASSERT (!state->m_expectedInternalCollision.empty ());
  struct DcfStateTest::ExpectedCollision expected = state->m_expectedInternalCollision.front ();
  state->m_expectedInternalCollision.pop_front ();
  NS_TEST_ASSERT_EQUAL (Simulator::Now (), MicroSeconds (expected.at));
  state->StartBackoffNow (expected.nSlots);
  if (!result)
    {
      m_result = result;
    }
}
void 
DcfManagerTest::NotifyCollision (uint32_t i)
{
  DcfStateTest *state = m_dcfStates[i];
  bool result = true;
  NS_TEST_ASSERT (!state->m_expectedCollision.empty ());
  struct DcfStateTest::ExpectedCollision expected = state->m_expectedCollision.front ();
  state->m_expectedCollision.pop_front ();
  NS_TEST_ASSERT_EQUAL (Simulator::Now (), MicroSeconds (expected.at));
  state->StartBackoffNow (expected.nSlots);
  if (!result)
    {
      m_result = result;
    }
}


void 
DcfManagerTest::ExpectInternalCollision (uint64_t time, uint32_t nSlots, uint32_t from)
{
  DcfStateTest *state = m_dcfStates[from];
  struct DcfStateTest::ExpectedCollision col;
  col.at = time;
  col.nSlots = nSlots;
  state->m_expectedInternalCollision.push_back (col);
}
void 
DcfManagerTest::ExpectCollision (uint64_t time, uint32_t nSlots, uint32_t from)
{
  DcfStateTest *state = m_dcfStates[from];
  struct DcfStateTest::ExpectedCollision col;
  col.at = time;
  col.nSlots = nSlots;
  state->m_expectedCollision.push_back (col);
}

void
DcfManagerTest::StartTest (uint64_t slotTime, uint64_t sifs, uint64_t eifsNoDifsNoSifs, uint32_t ackTimeoutValue)
{
  m_dcfManager = new DcfManager ();
  m_dcfManager->SetSlot (MicroSeconds (slotTime));
  m_dcfManager->SetSifs (MicroSeconds (sifs));
  m_dcfManager->SetEifsNoDifs (MicroSeconds (eifsNoDifsNoSifs+sifs));
  m_ackTimeoutValue = ackTimeoutValue;
}

void
DcfManagerTest::AddDcfState (uint32_t aifsn)
{
  DcfStateTest *state = new DcfStateTest (this, m_dcfStates.size ());
  state->SetAifsn (aifsn);
  m_dcfStates.push_back (state);
  m_dcfManager->Add (state);
}

void
DcfManagerTest::EndTest (void)
{
  bool result = true;
  Simulator::Run ();
  Simulator::Destroy ();
  for (DcfStates::const_iterator i = m_dcfStates.begin (); i != m_dcfStates.end (); i++)
    {
      DcfStateTest *state = *i;
      NS_TEST_ASSERT (state->m_expectedGrants.empty ());
      NS_TEST_ASSERT (state->m_expectedInternalCollision.empty ());
      NS_TEST_ASSERT (state->m_expectedCollision.empty ());
      delete state;
    }
  m_dcfStates.clear ();
  delete m_dcfManager;
  if (!result)
    {
      m_result = result;
    }
}

void 
DcfManagerTest::AddRxOkEvt (uint64_t at, uint64_t duration)
{
  Simulator::Schedule (MicroSeconds (at) - Now (), 
                       &DcfManager::NotifyRxStartNow, m_dcfManager, 
                       MicroSeconds (duration));
  Simulator::Schedule (MicroSeconds (at+duration) - Now (), 
                       &DcfManager::NotifyRxEndOkNow, m_dcfManager);
}
void
DcfManagerTest::AddRxInsideSifsEvt (uint64_t at, uint64_t duration)
{
  Simulator::Schedule (MicroSeconds (at) - Now (), 
                       &DcfManager::NotifyRxStartNow, m_dcfManager, 
                       MicroSeconds (duration));
}
void 
DcfManagerTest::AddRxErrorEvt (uint64_t at, uint64_t duration)
{
  Simulator::Schedule (MicroSeconds (at) - Now (), 
                       &DcfManager::NotifyRxStartNow, m_dcfManager, 
                       MicroSeconds (duration));
  Simulator::Schedule (MicroSeconds (at+duration) - Now (), 
                       &DcfManager::NotifyRxEndErrorNow, m_dcfManager);
}

void 
DcfManagerTest::AddNavReset (uint64_t at, uint64_t duration)
{
  Simulator::Schedule (MicroSeconds (at) - Now (), 
                       &DcfManager::NotifyNavResetNow, m_dcfManager, 
                       MicroSeconds (duration));
}
void 
DcfManagerTest::AddNavStart (uint64_t at, uint64_t duration)
{
  Simulator::Schedule (MicroSeconds (at) - Now (), 
                       &DcfManager::NotifyNavStartNow, m_dcfManager, 
                       MicroSeconds (duration));
}
void 
DcfManagerTest::AddAckTimeoutReset (uint64_t at)
{
  Simulator::Schedule (MicroSeconds (at) - Now (), 
                       &DcfManager::NotifyAckTimeoutResetNow, m_dcfManager);
}
void 
DcfManagerTest::AddAccessRequest (uint64_t at, uint64_t txTime, 
                                  uint64_t expectedGrantTime, uint32_t from)
{
  AddAccessRequestWithSuccessfullAck (at, txTime, expectedGrantTime, 0, from);
}
void 
DcfManagerTest::AddAccessRequestWithAckTimeout (uint64_t at, uint64_t txTime, 
                                  uint64_t expectedGrantTime, uint32_t from)
{
  Simulator::Schedule (MicroSeconds (at) - Now (), 
                       &DcfManagerTest::DoAccessRequest, this,
                       txTime, expectedGrantTime, m_dcfStates[from]);
}
void 
DcfManagerTest::AddAccessRequestWithSuccessfullAck (uint64_t at, uint64_t txTime, 
                                  uint64_t expectedGrantTime, uint32_t ackDelay, uint32_t from)
{
  NS_ASSERT(ackDelay < m_ackTimeoutValue);
  Simulator::Schedule (MicroSeconds (at) - Now (), 
                       &DcfManagerTest::DoAccessRequest, this,
                       txTime, expectedGrantTime, m_dcfStates[from]);
  AddAckTimeoutReset (expectedGrantTime + txTime + ackDelay);
}
void
DcfManagerTest::DoAccessRequest (uint64_t txTime, uint64_t expectedGrantTime, DcfStateTest *state)
{
  state->QueueTx (txTime, expectedGrantTime);
  m_dcfManager->RequestAccess (state);
}




bool 
DcfManagerTest::RunTests (void)
{
  m_result = true;

  //  0      3       4    5      8       9  10   12
  //  | sifs | aifsn | tx | sifs | aifsn |   | tx | 
  //
  StartTest (1, 3, 10);
  AddDcfState (1);
  AddAccessRequest (1, 1, 4, 0);
  AddAccessRequest (10, 2, 10, 0);
  EndTest ();
  // Check that receiving inside SIFS shall be cancelled properly:
  //  0      3       4    5      8     9     12       13 14
  //  | sifs | aifsn | tx | sifs | ack | sifs | aifsn |  |tx | 
  //
  StartTest (1, 3, 10);
  AddDcfState (1);
  AddAccessRequest (1, 1, 4, 0);
  AddRxInsideSifsEvt (6, 10);
  AddTxEvt(8, 1);
  AddAccessRequest (14, 2, 14, 0);
  EndTest ();


  // The test below mainly intends to test the case where the medium
  // becomes busy in the middle of a backoff slot: the backoff counter
  // must not be decremented for this backoff slot. This is the case
  // below for the backoff slot starting at time 78us.
  //
  //  20          60     66      70        74        78  80    100     106      110      114      118   120
  //   |    rx     | sifs | aifsn | bslot0  | bslot1  |   | rx   | sifs  |  aifsn | bslot2 | bslot3 | tx  |
  //        |
  //       30 request access. backoff slots: 4
  StartTest (4, 6, 10);
  AddDcfState (1);
  AddRxOkEvt (20, 40);
  AddRxOkEvt (80, 20);
  AddAccessRequest (30, 2, 118, 0);
  ExpectCollision (30, 4, 0); // backoff: 4 slots
  EndTest ();

  // Test the case where the backoff slots is zero.
  //
  //  20          60     66      70   72
  //   |    rx     | sifs | aifsn | tx | 
  //        |
  //       30 request access. backoff slots: 0
  StartTest (4, 6 , 10);
  AddDcfState (1);
  AddRxOkEvt (20, 40);
  AddAccessRequest (30, 2, 70, 0);
  ExpectCollision (30, 0, 0); // backoff: 0 slots
  EndTest ();

  // The test below is subject to some discussion because I am 
  // not sure I understand the intent of the spec here.
  // i.e., what happens if you make a request to get access
  // to the medium during the difs idle time after a busy period ?
  // do you need to start a backoff ? Or do you need to wait until
  // the end of difs and access the medium ?
  // Here, we wait until the end of difs and access the medium.
  //
  //  20    60     66      70   72
  //   | rx  | sifs | aifsn | tx |
  //           |
  //          62 request access.
  //
  StartTest (4, 6 , 10);
  AddDcfState (1);
  AddRxOkEvt (20, 40);
  AddAccessRequest (62, 2, 70, 0);
  EndTest ();


  // Test an EIFS
  // 
  //  20          60     66           76      80       84       88       92       96   98
  //   |    rx     | sifs | acktxttime | aifsn | bslot0 | bslot1 | bslot2 | bslot3 | tx |
  //        |      | <---------eifs----------->|
  //       30 request access. backoff slots: 4
  StartTest (4, 6, 10);
  AddDcfState (1);
  AddRxErrorEvt (20, 40);
  AddAccessRequest (30, 2, 96, 0);
  ExpectCollision (30, 4, 0); // backoff: 4 slots  
  EndTest ();

  // Test an EIFS which is interupted by a successfull transmission.
  // 
  //  20          60      66  69     75     81      85       89       93       97      101  103
  //   |    rx     | sifs  |   |  rx  | sifs | aifsn | bslot0 | bslot1 | bslot2 | bslot3 | tx |
  //        |      | <--eifs-->|
  //       30 request access. backoff slots: 4
  StartTest (4, 6, 10);
  AddDcfState (1);
  AddRxErrorEvt (20, 40);
  AddAccessRequest (30, 2, 101, 0);
  ExpectCollision (30, 4, 0); // backoff: 4 slots  
  AddRxOkEvt (69, 6);
  EndTest ();


  // Test two DCFs which suffer an internal collision. the first DCF has a higher
  // priority than the second DCF.
  //
  //      20          60      66      70       74       78    88
  // DCF0  |    rx     | sifs  | aifsn | bslot0 | bslot1 | tx  |
  // DCF1  |    rx     | sifs  | aifsn | aifsn  | aifsn  |     | sifs | aifsn | aifsn | aifsn | bslot |  tx  |
  //                                                                 94      98     102     106     110    112
  StartTest (4, 6, 10);
  AddDcfState (1); // high priority DCF
  AddDcfState (3); // low priority DCF
  AddRxOkEvt (20, 40);
  AddAccessRequest (30, 10, 78, 0);
  ExpectCollision (30, 2, 0); // backoff: 2 slot

  AddAccessRequest (40, 2, 110, 1);
  ExpectCollision (40, 0, 1); // backoff: 0 slot
  ExpectInternalCollision (78, 1, 1); // backoff: 1 slot
  EndTest ();
  
  // Test of AckTimeout handling: First queue requests access and ack procedure fails,
  // inside the ack timeout second queue with higher priority requests access.
  //
  //            20           40      50     60  66      76
  // DCF0 - low  |     tx     | ack timeout |sifs|       |
  // DCF1 - high |                    |     |sifs|  tx   |
  //                                  ^ request access
  StartTest (4, 6, 10);
  AddDcfState (2); // high priority DCF
  AddDcfState (0); // low priority DCF
  AddAccessRequestWithAckTimeout (20, 20, 20, 0);
  AddAccessRequest (50, 10, 66, 1);
  EndTest ();

  // Test of AckTimeout handling: 
  //
  // First queue requests access and ack is 2 us delayed (got ack interval at the picture),
  // inside this interval second queue with higher priority requests access.
  //
  //            20           40  41   42    48      58
  // DCF0 - low  |     tx     |got ack |sifs|       |
  // DCF1 - high |                |    |sifs|  tx   |
  //                              ^ request access
  StartTest (4, 6, 10);
  AddDcfState (2); // high priority DCF
  AddDcfState (0); // low priority DCF
  AddAccessRequestWithSuccessfullAck (20, 20, 20, 2, 0);
  AddAccessRequest (41, 10, 48, 1);
  EndTest ();

  //Repeat the same but with one queue:
  //            20           40  41   42    48      58
  // DCF0 - low  |     tx     |got ack |sifs|       |
  //                              ^ request access
  StartTest (4, 6, 10);
  AddDcfState (2);
  AddAccessRequestWithSuccessfullAck (20, 20, 20, 2, 0);
  AddAccessRequest (41, 10, 56, 0);
  EndTest ();

  //Repeat the same when ack was delayed:
  //and request the next access before previous tx end:
  //            20       39  40       42              64      74
  // DCF0 - low  |     tx     |got ack |sifs + 4 * slot|       |
  //                      ^ request access
  StartTest (4, 6, 10);
  AddDcfState (2);
  AddAccessRequestWithSuccessfullAck (20, 20, 20, 2, 0);
  AddAccessRequest (39, 10, 64, 0);
  ExpectCollision (39, 2, 0); // backoff: 2 slot
  EndTest ();

  //
  // test simple NAV count. This scenario modelizes a simple DATA+ACK handshake
  // where the data rate used for the ACK is higher than expected by the DATA source
  // so, the data exchange completes before the end of nav.
  // 
  StartTest (4, 6, 10);
  AddDcfState (1);
  AddRxOkEvt (20, 40);
  AddNavStart (60, 15);
  AddRxOkEvt (66, 5);
  AddNavStart (71, 0);
  AddAccessRequest (30, 10, 93, 0);
  ExpectCollision (30, 2, 0); // backoff: 2 slot
  EndTest ();

  //
  // test more complex NAV handling by a CF-poll. This scenario modelizes a 
  // simple DATA+ACK handshake interrupted by a CF-poll which resets the
  // NAV counter.
  // 
  StartTest (4, 6, 10);
  AddDcfState (1);
  AddRxOkEvt (20, 40);
  AddNavStart (60, 15);
  AddRxOkEvt (66, 5);
  AddNavReset (71, 2);
  AddAccessRequest (30, 10, 91, 0);
  ExpectCollision (30, 2, 0); // backoff: 2 slot
  EndTest ();

  
  StartTest (4, 6, 10);
  AddDcfState (2);
  AddRxOkEvt (20, 40);
  AddAccessRequest (80, 10, 80, 0);
  EndTest ();


  StartTest (4, 6, 10);
  AddDcfState (2);
  AddRxOkEvt (20, 40);
  AddRxOkEvt (78, 8);
  AddAccessRequest (30, 50, 108, 0);
  ExpectCollision (30, 3, 0); // backoff: 3 slots
  EndTest ();
 

  return m_result;
}



static DcfManagerTest g_dcf_manager_test;

} // namespace ns3

#endif /* RUN_SELF_TESTS */
