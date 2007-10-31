/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005 INRIA
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

#ifndef DCA_TXOP_H
#define DCA_TXOP_H

#include <stdint.h>
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/callback-trace-source.h"
#include "ns3/nstime.h"
#include "wifi-mac-header.h"
#include "wifi-mode.h"

namespace ns3 {

class Dcf;
class WifiMacQueue;
class MacLow;
class WifiPhy;
class MacParameters;
class MacTxMiddle;

/**
 * \brief handle packet fragmentation and retransmissions.
 *
 * This class implements the packet fragmentation and 
 * retransmission policy. It uses the ns3::MacLow and ns3::Dcf
 * helper classes to respectively send packets and decide when 
 * to send them. Packets are stored in a ns3::WifiMacQueue until
 * they can be sent.
 *
 * The policy currently implemented uses a simple fragmentation
 * threshold: any packet bigger than this threshold is fragmented
 * in fragments whose size is smaller than the threshold.
 *
 * The retransmission policy is also very simple: every packet is
 * retransmitted until it is either successfully transmitted or
 * it has been retransmitted up until the ssrc or slrc thresholds.
 *
 * The rts/cts policy is similar to the fragmentation policy: when
 * a packet is bigger than a threshold, the rts/cts protocol is used.
 */
class DcaTxop 
{
public:
  typedef Callback <void, WifiMacHeader const&> TxOk;
  typedef Callback <void, WifiMacHeader const&> TxFailed;

  /**
   * \param minCw forwarded to ns3::Dcf constructor
   * \param maxCw forwarded to ns3::Dcf constructor
   *
   * Initialized from \valueref{WifiMaxSsrc}, \valueref{WifiMaxSlrc},
   * \valueref{WifiRtsCtsThreshold}, and, \valueref{WifiFragmentationThreshold}.
   */
  DcaTxop (uint32_t minCw, uint32_t maxCw);
  ~DcaTxop ();

  void SetLow (MacLow *low);
  void SetPhy (Ptr<WifiPhy> phy);
  void SetParameters (MacParameters *parameters);
  void SetTxMiddle (MacTxMiddle *txMiddle);
  /**
   * \param callback the callback to invoke when a 
   * packet transmission was completed successfully.
   */
  void SetTxOkCallback (TxOk callback);
  /**
   * \param callback the callback to invoke when a 
   * packet transmission was completed unsuccessfully.
   */
  void SetTxFailedCallback (TxFailed callback);

  void SetDifs (Time difs);
  void SetEifs (Time eifs);
  void SetCwBounds (uint32_t min, uint32_t max);
  void SetMaxQueueSize (uint32_t size);
  void SetMaxQueueDelay (Time delay);

  /**
   * \param packet packet to send
   * \param hdr header of packet to send.
   *
   * Store the packet in the internal queue until it
   * can be sent safely.
   */
  void Queue (Packet packet, WifiMacHeader const &hdr);
private:
  class AccessListener;
  class TransmissionListener;
  class NavListener;
  class PhyListener;
  friend class AccessListener;
  friend class TransmissionListener;

  MacLow *Low (void);
  MacParameters *Parameters (void);

  /* event handlers */
  void AccessGrantedNow (void);
  bool AccessingAndWillNotify (void);
  bool AccessNeeded (void);
  void GotCts (double snr, WifiMode txMode);
  void MissedCts (void);
  void GotAck (double snr, WifiMode txMode);
  void MissedAck (void);
  void StartNext (void);

  bool NeedRts (void);
  bool NeedFragmentation (void);
  uint32_t GetNFragments (void);
  uint32_t GetLastFragmentSize (void);
  uint32_t GetNextFragmentSize (void);
  uint32_t GetFragmentSize (void);
  bool IsLastFragment (void);
  void NextFragment (void);
  Packet GetFragmentPacket (WifiMacHeader *hdr);

  Dcf *m_dcf;
  TxOk m_txOkCallback;
  TxFailed m_txFailedCallback;
  WifiMacQueue *m_queue;
  MacTxMiddle *m_txMiddle;
  MacLow *m_low;
  MacParameters *m_parameters;
  TransmissionListener *m_transmissionListener;
  AccessListener *m_accessListener;
  NavListener *m_navListener;
  PhyListener *m_phyListener;
  

  Packet m_currentPacket;
  bool m_hasCurrent;
  WifiMacHeader m_currentHdr;
  uint32_t m_ssrc;
  uint32_t m_slrc;
  uint8_t m_fragmentNumber;

  /* 80211-dca-acktimeout
   * param1: slrc
   */
  CallbackTraceSource<uint32_t> m_acktimeoutTrace;
  /* 80211-dca-ctstimeout
   * param1: ssrc
   */
  CallbackTraceSource<uint32_t> m_ctstimeoutTrace;
};

} //namespace ns3



#endif /* DCA_TXOP_H */
