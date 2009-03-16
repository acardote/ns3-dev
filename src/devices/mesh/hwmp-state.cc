/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008,2009 IITP RAS
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
 * Author: Kirill Andreev <andreev@iitp.ru>
 */


#include "ns3/hwmp-state.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("HwmpState");
TypeId
HwmpState::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::HwmpState")
                      .SetParent<Object> ()
                      .AddConstructor<HwmpState> ()
                      ;
  return tid;
}

HwmpState::HwmpState():
    m_myPreq(m_preqQueue.end()),
    m_preqId(1),
    m_myDsn(1),
    m_disabled(false),
    m_maxTtl(10)
{
}
void
HwmpState::SetRequestRouteCallback(
  Callback<HwmpRtable::LookupResult, const Mac48Address&> cb)
{
  m_requestRouteCallback = cb;
}

void
HwmpState::SetRequestRootPathCallback(
  Callback<HwmpRtable::LookupResult, uint32_t> cb)
{
  m_requestRootPathCallback = cb;
}

//Setting MAC
void
HwmpState::SetMac(Ptr<MeshWifiMac> mac)
{
  mac->SetPeerStatusCallback(MakeCallback(&HwmpState::PeerStatus, this));
  mac->SetPreqReceivedCallback(MakeCallback(&HwmpState::ReceivePreq, this));
  mac->SetPrepReceivedCallback(MakeCallback(&HwmpState::ReceivePrep, this));
  mac->SetPerrReceivedCallback(MakeCallback(&HwmpState::ReceivePerr, this));
  m_address = mac->GetAddress();
  m_preqCallback = MakeCallback(&MeshWifiMac::SendPreq, mac);
  m_prepCallback = MakeCallback(&MeshWifiMac::SendPrep, mac);
  m_perrCallback = MakeCallback(&MeshWifiMac::SendPerr, mac);
}
HwmpState::~HwmpState()
{
  m_preqQueue.clear();
}
//Interaction with HWMP:
void
HwmpState::SetRoutingInfoCallback(
  Callback<void, INFO> cb
)
{
  m_routingInfoCallback = cb;
}

void
HwmpState::SetRetransmittersOfPerrCallback(
  Callback<std::vector<Mac48Address>, std::vector<HwmpRtable::FailedDestination>, uint32_t> cb)
{
  m_retransmittersOfPerrCallback = cb;
}

void
HwmpState::RequestDestination(Mac48Address dst)
{
  if (m_preqQueue.end() == m_myPreq)
    {
      WifiPreqInformationElement preq;
      //fill PREQ:
      preq.SetHopcount(0);
      preq.SetTTL(m_maxTtl);
      preq.SetPreqID(m_preqId++);
      if (m_preqId == MAX_PREQ_ID)
        m_preqId = 0;
      preq.SetLifetime(TIME_TO_TU(dot11sParameters::dot11MeshHWMPactivePathTimeout));
      preq.SetOriginatorSeqNumber(m_myDsn++);
      if (m_myDsn == MAX_DSN)
        m_myDsn = 0;
      preq.SetOriginatorAddress(m_address);
      preq.AddDestinationAddressElement(false, false, dst, 0); //DO = 0, RF = 0
      if (m_preqTimer.IsRunning())
        {
          NS_LOG_DEBUG("No my preq");
          m_preqQueue.push_back(preq);
          //set iterator position to my preq:
          m_myPreq = m_preqQueue.end() -1;
        }
      else
        {
          NS_LOG_DEBUG("Send PREQ now, "<<preq.GetPreqID()<<" destinations, now is "<<Simulator::Now());
          m_preqCallback(preq);
          NS_ASSERT(!m_preqTimer.IsRunning());
          m_preqTimer = Simulator::Schedule(dot11sParameters::dot11MeshHWMPpreqMinInterval, &HwmpState::SendOnePreq, this);
        }
    }
  else
    {
      NS_ASSERT(m_myPreq->GetOriginatorAddress() == m_address);
      NS_LOG_DEBUG("add a destination "<<dst);
      m_myPreq->AddDestinationAddressElement(false, false, dst, 0); //DO = 0, RF = 0
    }
}
void
HwmpState::SendPathError(std::vector<HwmpRtable::FailedDestination> destinations)
{
  std::vector<Mac48Address> receivers =  m_retransmittersOfPerrCallback(destinations, m_ifIndex);
  NS_LOG_DEBUG("SendPathError started");
  if (receivers.size() == 0)
    return;
  NS_LOG_DEBUG(m_address<<" Should Send PERR to");
  for (unsigned int i = 0; i < receivers.size(); i ++)
    {
      AddPerrReceiver(receivers[i]);
      NS_LOG_DEBUG(receivers[i]);
    }
  NS_LOG_DEBUG("To tel about failure with");
  for (unsigned int i = 0; i < destinations.size(); i ++)
    {
      m_myPerr.AddAddressUnit(destinations[i]);
      NS_LOG_DEBUG(destinations[i].destination);
    }
  if (!m_perrTimer.IsRunning())
    {
      m_perrCallback(m_myPerr,m_myPerrReceivers);
      m_myPerr.ResetPerr();
      m_perrTimer = Simulator::Schedule(dot11sParameters::dot11MeshHWMPperrMinInterval,&HwmpState::SendOnePerr,this);
    }
}
//needed to fill routing information structure
void
HwmpState::SetAssociatedIfaceId(uint32_t interface)
{
  m_ifIndex = interface;
}

uint32_t
HwmpState::GetAssociatedIfaceId()
{
  return m_ifIndex;
}

//Interaction with MAC:
void
HwmpState::ReceivePreq(WifiPreqInformationElement& preq,  const Mac48Address& from, const uint32_t& metric)
{
  if (m_disabled)
    return;
  if (preq.GetOriginatorAddress() == m_address)
    return;
  preq.DecrementTtl();
  preq.IncrementMetric(metric);
  if (preq.GetTtl() == 0)
    return;
  //acceptance cretirea:
  std::map<Mac48Address, uint32_t, mac48addrComparator>::iterator i = m_dsnDatabase.find(preq.GetOriginatorAddress());
  if (i == m_dsnDatabase.end())
    {
      m_dsnDatabase[preq.GetOriginatorAddress()] = preq.GetOriginatorSeqNumber();
      m_preqMetricDatabase[preq.GetOriginatorAddress()] = preq.GetMetric();
    }
  else
    {
      if (i->second > preq.GetOriginatorSeqNumber())
        return;
      if (i->second == preq.GetOriginatorSeqNumber())
        {
          //find metric
          std::map<Mac48Address, uint32_t, mac48addrComparator>::iterator j =
            m_preqMetricDatabase.find(preq.GetOriginatorAddress());
          NS_ASSERT(j != m_dsnDatabase.end());
          if (j->second <= preq.GetMetric())
            return;
        }
      m_dsnDatabase[preq.GetOriginatorAddress()] = preq.GetOriginatorSeqNumber();
      m_preqMetricDatabase[preq.GetOriginatorAddress()] = preq.GetMetric();
    }
  NS_LOG_DEBUG(
    "PREQ from "<< preq.GetOriginatorAddress()
    <<", at "<< m_address
    <<", TTL ="<< (int)preq.GetTtl()
    <<", metric = "<< preq.GetMetric()
    <<", hopcount = "<< (int)preq.GetHopCount()
    <<", preqId = "<< preq.GetPreqID()
    <<", transmitter is "<<from);
  //fill routingTable
  INFO newInfo;
  newInfo.me = m_address;
  newInfo.destination = preq.GetOriginatorAddress();
  newInfo.nextHop = from;
  newInfo.metric = preq.GetMetric();
  newInfo.lifetime = TU_TO_TIME(preq.GetLifetime());
  newInfo.outPort = m_ifIndex;
  newInfo.dsn = preq.GetOriginatorSeqNumber();
  newInfo.type = INFO_PREQ;
  //check if can answer:
  std::vector<Ptr<DestinationAddressUnit> > destinations = preq.GetDestinationList ();
  for (std::vector<Ptr<DestinationAddressUnit> >::iterator i = destinations.begin(); i != destinations.end(); i++)
    {
      if ((*i)->GetDestinationAddress() == Mac48Address::GetBroadcast())
        {
          //only proactive PREQ contains destination
          //address as broadcast! Proactive preq MUST
          //have destination count equal to 1 and
          //per destination flags DO and RF
          NS_ASSERT(preq.GetDestCount() == 1);
          NS_ASSERT(((*i)->IsDo()) && ((*i)->IsRf()));
          NS_LOG_DEBUG("PROACTIVE PREQ RECEIVED");
          newInfo.type = INFO_PROACTIVE;
          m_routingInfoCallback(newInfo);
          if (!preq.IsNeedNotPrep())
            {
              SendPrep(
                preq.GetOriginatorAddress(),
                m_address,
                from,
                preq.GetMetric(),
                preq.GetOriginatorSeqNumber(),
                m_myDsn ++,
                preq.GetLifetime()
              );
              if (m_myDsn == MAX_DSN)
                m_myDsn = 0;
            }
          break;
        }
      if ((*i)->GetDestinationAddress()==m_address)
        {
          preq.DelDestinationAddressElement((*i)->GetDestinationAddress());
          SendPrep(
            preq.GetOriginatorAddress(),
            m_address,
            from,
            0,
            preq.GetOriginatorSeqNumber(),
            m_myDsn++,
            preq.GetLifetime()
          );
          if (m_myDsn == MAX_DSN)
            m_myDsn = 0;
          continue;
        }
      //check if can answer:
      HwmpRtable::LookupResult result = m_requestRouteCallback((*i)->GetDestinationAddress());
      if ((!((*i)->IsDo())) && (result.retransmitter!=Mac48Address::GetBroadcast()))
        {
          //have a valid information and acn answer
          if ((*i)->IsRf())
            (*i)->SetFlags(true, false); //DO = 1, RF = 0 (as it was)
          else
            {
              //send a PREP and delete destination
              preq.DelDestinationAddressElement((*i)->GetDestinationAddress());
              SendPrep(
                preq.GetOriginatorAddress(),
                (*i)->GetDestinationAddress(),
                result.retransmitter,
                result.metric,
                preq.GetOriginatorSeqNumber(),
                result.seqnum,
                preq.GetLifetime()
              );
              continue;
            }
        }
    }
  m_routingInfoCallback(newInfo);
  //chack if must retransmit:
  if (preq.GetDestCount() == 0)
    return;
  if (m_preqTimer.IsRunning())
    {
      m_preqQueue.push_back(preq);
    }
  else
    {
      m_preqCallback(preq);
      NS_ASSERT(!m_preqTimer.IsRunning());
      m_preqTimer = Simulator::Schedule(dot11sParameters::dot11MeshHWMPpreqMinInterval, &HwmpState::SendOnePreq, this);
    }
}

void
HwmpState::ReceivePrep(WifiPrepInformationElement& prep, const Mac48Address& from, const uint32_t& metric)
{
  if (m_disabled)
    return;
  prep.DecrementTtl();
  prep.IncrementMetric(metric);
  //acceptance cretirea:
  std::map<Mac48Address, uint32_t, mac48addrComparator>::iterator i = m_dsnDatabase.find(prep.GetDestinationAddress());
  if (i == m_dsnDatabase.end())
    {
      m_dsnDatabase[prep.GetDestinationAddress()] = prep.GetDestinationSeqNumber();
    }
  else
    if (i->second > prep.GetDestinationSeqNumber())
      return;
  //update routing info
  HwmpRtable::LookupResult result = m_requestRouteCallback(prep.GetDestinationAddress());
  if (result.retransmitter == Mac48Address::GetBroadcast())
    //try to look for default route
    result = m_requestRootPathCallback(m_ifIndex);
  if ((result.retransmitter == Mac48Address::GetBroadcast())&&(m_address != prep.GetDestinationAddress()))
    return;
  INFO newInfo;
  newInfo.me = m_address;
  newInfo.destination = prep.GetOriginatorAddress();
  newInfo.source = prep.GetDestinationAddress();
  newInfo.nextHop = from;
  newInfo.metric = prep.GetMetric();
  newInfo.lifetime = TU_TO_TIME(prep.GetLifetime());
  newInfo.outPort = m_ifIndex;
  newInfo.dsn = prep.GetOriginatorSeqNumber();
  newInfo.prevHop = result.retransmitter;
  newInfo.type = INFO_PREP;
  NS_LOG_DEBUG("Path to "<<newInfo.source<<", i am "<<m_address<<", precursor is "<<from);
  NS_LOG_DEBUG("Path to "<<newInfo.destination<<", i am "<<m_address<<", precursor is "<<result.retransmitter);
  m_routingInfoCallback(newInfo);
  if (prep.GetDestinationAddress() == m_address)
    {
      NS_LOG_DEBUG("Destination resolved"<<newInfo.destination);
      return;
    }
  m_prepCallback(prep, result.retransmitter);
}

void
HwmpState::ReceivePerr(WifiPerrInformationElement& perr, const Mac48Address& from)
{
  if (m_disabled)
    return;
  NS_LOG_DEBUG(m_address<<" RECEIVED PERR from "<<from);
  /**
   * Check forwarding conditions:
   */
  std::vector<HwmpRtable::FailedDestination> destinations = perr.GetAddressUnitVector();
  for (unsigned int i = 0; i < destinations.size(); i ++)
    {
      /**
       * Lookup for a valid routing information
       */
      HwmpRtable::LookupResult result = m_requestRouteCallback(destinations[i].destination);
      if (
        (result.retransmitter != from)
        ||(result.seqnum >= destinations[i].seqnum)
      )

        perr.DeleteAddressUnit(destinations[i].destination);
    }
  NS_LOG_DEBUG("Retransmit "<<(int)perr.GetNumOfDest());
  if (perr.GetNumOfDest() == 0)
    return;
  destinations = perr.GetAddressUnitVector();
  SendPathError(destinations);
}

void
HwmpState::PeerStatus(const Mac48Address peerAddress, const bool status, const uint32_t metric)
{
  INFO newInfo;
  newInfo.me = m_address;
  newInfo.destination = peerAddress;
  newInfo.nextHop = peerAddress;
  newInfo.metric = metric;
  newInfo.outPort = m_ifIndex;
  newInfo.dsn = 0;
  if (status)
    newInfo.type = INFO_NEW_PEER;
  else
    newInfo.type = INFO_FAILED_PEER;
  m_routingInfoCallback(newInfo);
}

bool
HwmpState::SetRoot()
{
#if 0
  //TODO:: delete this lines!!!!!!!
  if (m_address != Mac48Address("00:00:00:00:00:10"))
    return false;
  //TODO
#endif
  Simulator::Schedule(dot11sParameters::dot11MeshHWMPactiveRootTimeout, &HwmpState::SendProactivePreq, this);
  return true;
}

void
HwmpState::SendProactivePreq()
{
  NS_LOG_DEBUG("Sending proactive PREQ");
  WifiPreqInformationElement preq;
  //By default: must answer
  preq.SetHopcount(0);
  preq.SetTTL(m_maxTtl);
  preq.SetPreqID(m_preqId++);
  if (m_preqId == MAX_PREQ_ID)
    m_preqId = 0;
  preq.SetLifetime(TIME_TO_TU(dot11sParameters::dot11MeshHWMPpathToRootInterval));
  preq.SetOriginatorSeqNumber(m_myDsn++);
  if (m_myDsn == MAX_DSN)
    m_myDsn = 0;
  preq.SetOriginatorAddress(m_address);
  preq.AddDestinationAddressElement(
    true,
    true,
    Mac48Address::GetBroadcast()
    ,0);
  if (m_preqTimer.IsRunning())
    m_preqQueue.push_back(preq);
  else
    {
      NS_LOG_DEBUG("Send now "<<preq.GetPreqID());
      m_preqCallback(preq);
      NS_ASSERT(!m_preqTimer.IsRunning());
      m_preqTimer = Simulator::Schedule(dot11sParameters::dot11MeshHWMPpreqMinInterval, &HwmpState::SendOnePreq, this);
    }
  Simulator::Schedule(dot11sParameters::dot11MeshHWMPactiveRootTimeout, &HwmpState::SendProactivePreq, this);
}

void
HwmpState::AddPerrReceiver(Mac48Address receiver)
{
  /**
   * add new vector of addresses to m_perrReceiversand check
   * duplicates
   */
  for (unsigned int j = 0; j < m_myPerrReceivers.size(); j++)
    if (m_myPerrReceivers[j] == receiver)
      return;
  m_myPerrReceivers.push_back(receiver);
}

void
HwmpState::UnSetRoot()
{
}

void
HwmpState::Disable()
{
  m_disabled = true;
}

void
HwmpState::Enable()
{
  m_disabled = false;
}

Mac48Address
HwmpState::GetAddress()
{
  return m_address;
}

void
HwmpState::SendOnePreq()
{
  if (m_preqQueue.size() == 0)
    return;
  if (m_myPreq == m_preqQueue.begin())
    m_myPreq == m_preqQueue.end();
  WifiPreqInformationElement preq = m_preqQueue[0];
  NS_LOG_DEBUG(
    "Sending PREQ from "<<preq.GetOriginatorAddress() <<
    " destinations are  "<<(int)preq.GetDestCount()<<
    ", at "<<Simulator::Now()<<
    ", store in queue "<<m_preqQueue.size()<<
    " preqs"<<", I am "<<m_address);
  m_preqCallback(preq);
  //erase first!
  m_preqQueue.erase(m_preqQueue.begin());
  //reschedule sending PREQ
  NS_ASSERT(!m_preqTimer.IsRunning());
  m_preqTimer = Simulator::Schedule(dot11sParameters::dot11MeshHWMPpreqMinInterval, &HwmpState::SendOnePreq, this);
}

void
HwmpState::SendPrep(Mac48Address dst,
                    Mac48Address src,
                    Mac48Address retransmitter,
                    uint32_t initMetric,
                    uint32_t dsn,
                    uint32_t originatorDsn,
                    uint32_t lifetime)
{
  WifiPrepInformationElement prep;
  prep.SetHopcount(0);
  prep.SetTTL(m_maxTtl);
  prep.SetDestinationAddress(dst);
  prep.SetDestinationSeqNumber(dsn);
  prep.SetLifetime(lifetime);
  prep.SetMetric(0);
  prep.SetOriginatorAddress(src);
  prep.SetOriginatorSeqNumber(originatorDsn);
  m_prepCallback(prep, retransmitter);
}

void
HwmpState::SendOnePerr()
{
  if (m_myPerr.GetNumOfDest() == 0)
    return;
  m_perrCallback(m_myPerr, m_myPerrReceivers);
  m_myPerr.ResetPerr();
}
}//namespace ns3
