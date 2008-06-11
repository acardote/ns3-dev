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

#ifndef YANS_WIFI_PHY_H
#define YANS_WIFI_PHY_H

#include <vector>
#include <list>
#include <stdint.h>
#include "ns3/callback.h"
#include "ns3/event-id.h"
#include "ns3/packet.h"
#include "ns3/object.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/random-variable.h"
#include "wifi-phy.h"
#include "wifi-mode.h"
#include "wifi-preamble.h"
#include "wifi-phy-standard.h"


namespace ns3 {

class RandomUniform;
class RxEvent;
class YansWifiChannel;


/**
 * \brief 802.11 PHY layer model
 *
 * This PHY implements a model of 802.11a. The model
 * implemented here is based on the model described
 * in "Yet Another Network Simulator", 
 * (http://cutebugs.net/files/wns2-yans.pdf).
 *
 *
 * This PHY model depends on a channel loss and delay
 * model as provided by the ns3::PropagationLossModel
 * and ns3::PropagationDelayModel classes, both of which are
 * members of the ns3::YansWifiChannel class.
 */
class YansWifiPhy : public WifiPhy
{
public:

  static TypeId GetTypeId (void);

  YansWifiPhy ();
  virtual ~YansWifiPhy ();

  void SetChannel (Ptr<YansWifiChannel> channel);
  void StartReceivePacket (Ptr<Packet> packet,
                           double rxPowerDbm,
                           WifiMode mode,
                           WifiPreamble preamble);

  void SetStandard (enum WifiPhyStandard standard);
  void SetRxNoise (double ratio);
  void SetTxPowerStart (double start);
  void SetTxPowerEnd (double end);
  void SetNTxPower (uint32_t n);
  void SetTxGain (double gain);
  void SetRxGain (double gain);
  void SetEdThreshold (double threshold);
  double GetRxNoise (void) const;
  double GetTxGain (void) const;
  double GetRxGain (void) const;
  double GetEdThreshold (void) const;


  virtual double GetTxPowerStart (void) const;
  virtual double GetTxPowerEnd (void) const;
  virtual uint32_t GetNTxPower (void) const;
  virtual void SetReceiveOkCallback (WifiPhy::SyncOkCallback callback);
  virtual void SetReceiveErrorCallback (WifiPhy::SyncErrorCallback callback);
  virtual void SendPacket (Ptr<const Packet> packet, WifiMode mode, enum WifiPreamble preamble, uint8_t txPowerLevel);
  virtual void RegisterListener (WifiPhyListener *listener);
  virtual bool IsStateCcaBusy (void);
  virtual bool IsStateIdle (void);
  virtual bool IsStateBusy (void);
  virtual bool IsStateSync (void);
  virtual bool IsStateTx (void);
  virtual Time GetStateDuration (void);
  virtual Time GetDelayUntilIdle (void);
  virtual Time GetLastRxStartTime (void) const;
  virtual Time CalculateTxDuration (uint32_t size, WifiMode payloadMode, enum WifiPreamble preamble) const;
  virtual uint32_t GetNModes (void) const;
  virtual WifiMode GetMode (uint32_t mode) const;
  virtual double CalculateSnr (WifiMode txMode, double ber) const;
  virtual Ptr<WifiChannel> GetChannel (void) const;

private:
  class NiChange {
  public:
    NiChange (Time time, double delta);
    Time GetTime (void) const;
    double GetDelta (void) const;
    bool operator < (NiChange const &o) const;
  private:
    Time m_time;
    double m_delta;
  };
  typedef std::vector<WifiMode> Modes;
  typedef std::list<WifiPhyListener *> Listeners;
  typedef std::list<Ptr<RxEvent> > Events;
  typedef std::vector <NiChange> NiChanges;

private:
  virtual void DoDispose (void);
  void Configure80211aParameters (void);
  void PrintModes (void) const;
  void Configure80211a (void);
  void ConfigureHolland (void);
  char const *StateToString (enum State state);
  enum YansWifiPhy::State GetState (void);
  double GetEdThresholdW (void) const;
  double DbmToW (double dbm) const;
  double DbToRatio (double db) const;
  double WToDbm (double w) const;
  double RatioToDb (double ratio) const;
  Time GetMaxPacketDuration (void) const;
  void CancelRx (void);
  double GetPowerDbm (uint8_t power) const;
  void NotifyTxStart (Time duration);
  void NotifyWakeup (void);
  void NotifySyncStart (Time duration);
  void NotifySyncEndOk (void);
  void NotifySyncEndError (void);
  void NotifyCcaBusyStart (Time duration);
  void LogPreviousIdleAndCcaBusyStates (void);
  void SwitchToTx (Time txDuration);
  void SwitchToSync (Time syncDuration);
  void SwitchFromSync (void);
  void SwitchMaybeToCcaBusy (Time duration);
  void AppendEvent (Ptr<RxEvent> event);
  double CalculateNoiseInterferenceW (Ptr<RxEvent> event, NiChanges *ni) const;
  double CalculateSnr (double signal, double noiseInterference, WifiMode mode) const;
  double CalculateChunkSuccessRate (double snir, Time delay, WifiMode mode) const;
  double CalculatePer (Ptr<const RxEvent> event, NiChanges *ni) const;
  void EndSync (Ptr<Packet> packet, Ptr<RxEvent> event);
  double Log2 (double val) const;
  double GetBpskBer (double snr, uint32_t signalSpread, uint32_t phyRate) const;
  double GetQamBer (double snr, unsigned int m, uint32_t signalSpread, uint32_t phyRate) const;
  uint32_t Factorial (uint32_t k) const;
  double Binomial (uint32_t k, double p, uint32_t n) const;
  double CalculatePdOdd (double ber, unsigned int d) const;
  double CalculatePdEven (double ber, unsigned int d) const;
  double CalculatePd (double ber, unsigned int d) const;
  double GetFecBpskBer (double snr, double nbits, 
                        uint32_t signalSpread, uint32_t phyRate,
                        uint32_t dFree, uint32_t adFree) const;
  double GetFecQamBer (double snr, uint32_t nbits, 
                       uint32_t signalSpread,
                       uint32_t phyRate,
                       uint32_t m, uint32_t dfree,
                       uint32_t adFree, uint32_t adFreePlusOne) const;
  double GetChunkSuccessRate (WifiMode mode, double snr, uint32_t nbits) const;
private:
  uint64_t m_txPrepareDelayUs;
  uint64_t m_plcpLongPreambleDelayUs;
  uint64_t m_plcpShortPreambleDelayUs;
  WifiMode m_longPlcpHeaderMode;
  WifiMode m_shortPlcpHeaderMode;
  uint32_t m_plcpHeaderLength;
  Time     m_maxPacketDuration;

  double   m_edThresholdW; /* unit: W */
  double   m_txGainDb;
  double   m_rxGainDb;
  double   m_rxNoiseRatio;
  double   m_txPowerBaseDbm;
  double   m_txPowerEndDbm;
  uint32_t m_nTxPower;

  
  bool m_syncing;
  Time m_endTx;
  Time m_endSync;
  Time m_endCcaBusy;
  Time m_startTx;
  Time m_startSync;
  Time m_startCcaBusy;
  Time m_previousStateChangeTime;

  Ptr<YansWifiChannel> m_channel;
  SyncOkCallback m_syncOkCallback;
  SyncErrorCallback m_syncErrorCallback;
  TracedCallback<Ptr<const Packet>, double, WifiMode, enum WifiPreamble> m_rxOkTrace;
  TracedCallback<Ptr<const Packet>, double> m_rxErrorTrace;
  TracedCallback<Ptr<const Packet>,WifiMode,WifiPreamble,uint8_t> m_txTrace;
  Modes m_modes;
  Listeners m_listeners;
  EventId m_endSyncEvent;
  Events m_events;
  UniformVariable m_random;
  TracedCallback<Time,Time,enum YansWifiPhy::State> m_stateLogger;
  WifiPhyStandard m_standard;
};

} // namespace ns3


#endif /* YANS_WIFI_PHY_H */
