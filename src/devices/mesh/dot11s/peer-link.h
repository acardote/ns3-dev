/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/* 
 * Copyright (c) 2009 IITP RAS
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
 * Authors: Kirill Andreev <andreev@iitp.ru>
 *          Aleksey Kovalenko <kovalenko@iitp.ru>
 */

#ifndef PEERLLINK_H_
#define PEERLLINK_H_

#include "ns3/nstime.h"
#include "ns3/callback.h"
#include "ns3/mac48-address.h"
#include "ie-dot11s-beacon-timing.h"
#include "ie-dot11s-peer-management.h"
#include "ie-dot11s-configuration.h"
#include "peer-manager-plugin.h"
namespace ns3 {
namespace dot11s {
/**
 * \ingroup dot11s
 * 
 * \brief Peer link model for 802.11s Peer Management protocol 
 */
class PeerLink : public Object
{
public:
  /// Support object system
  static TypeId GetTypeId();
  /// C-tor create empty link
  PeerLink ();
  
  /// Process beacon received from peer
  void SetBeaconInformation (Time lastBeacon, Time BeaconInterval);
  /**
   * \brief Method used to detecet peer link changes
   * 
   * \param bool if true - opened new link, if false - link closed
   */
  void  SetLinkStatusCallback (Callback<void, uint32_t, Mac48Address, bool> cb);
  /**
   * \name Peer link geeters/setters
   * \{
   */
  void  SetPeerAddress (Mac48Address macaddr);
  void  SetInterface (uint32_t interface);
  void  SetLocalLinkId (uint16_t id);
  void  SetPeerLinkId (uint16_t id);
  void  SetLocalAid (uint16_t aid);
  void  SetPeerAid (uint16_t aid);
  void  SetBeaconTimingElement (IeDot11sBeaconTiming beaconTiming);
  void  SetPeerLinkDescriptorElement (IeDot11sPeerManagement peerLinkElement);
  Mac48Address GetPeerAddress () const;
  uint16_t GetLocalAid () const;
  Time  GetLastBeacon () const;
  Time  GetBeaconInterval () const;
  IeDot11sBeaconTiming GetBeaconTimingElement ()const;
  IeDot11sPeerManagement GetPeerLinkDescriptorElement ()const;
  void  ClearTimingElement ();
  //\}
  
  /**
   * \name MLME
   * \{
   */
  /// MLME-CancelPeerLink.request
  void MLMECancelPeerLink (dot11sReasonCode reason);
  /// MLME-PassivePeerLinkOpen.request
  void MLMEPassivePeerLinkOpen ();
  /// MLME-ActivePeerLinkOpen.request
  void MLMEActivePeerLinkOpen ();
  /// MLME-PeeringRequestReject
  void MLMEPeeringRequestReject ();
  /// Callback type for MLME-SignalPeerLinkStatus event
  typedef Callback<void, uint32_t, Mac48Address, bool> SignalStatusCallback; 
  /// Set callback
  void MLMESetSignalStatusCallback (SignalStatusCallback);
  //\}

  /**
   * \name Link response to received management frames
   * 
   * \attention In all this methods {local/peer}LinkID correspond to _peer_ station, as written in
   * received frame, e.g. I am peerLinkID and peer link is localLinkID .
   * 
   * TODO is that clear?
   * 
   * \{
   */
  /// Close link
  void Close (uint16_t localLinkID, uint16_t peerLinkID, dot11sReasonCode reason);
  /// Accept open link
  void OpenAccept (uint16_t localLinkId, IeDot11sConfiguration conf);
  /// Reject open link 
  void OpenReject (uint16_t localLinkId, IeDot11sConfiguration conf, dot11sReasonCode reason);
  /// Confirm accept
  void ConfirmAccept (
    uint16_t localLinkId,
    uint16_t peerLinkId,
    uint16_t peerAid,
    IeDot11sConfiguration conf
  );
  /// Confirm reject
  void  ConfirmReject (
    uint16_t localLinkId,
    uint16_t peerLinkId,
    IeDot11sConfiguration  conf,
    dot11sReasonCode reason
  );
  //\}
  
  /// True if link is established
  bool  LinkIsEstab () const;
  /// True if link is idle. Link can be deleted in this state 
  bool  LinkIsIdle () const;
  /**
   * Set pointer to MAC-plugin, which is responsible for sending peer
   * link management frames
   */
  void SetMacPlugin(Ptr<Dot11sPeerManagerMacPlugin> plugin);
private:
  /// Peer link states, see 802.11s draft 11B.3.3.1
  enum  PeerState {
    IDLE,       
    LISTEN,
    OPN_SNT,
    CNF_RCVD,
    OPN_RCVD,
    ESTAB,
    HOLDING,
  };
  /// Peer link events, see 802.11s draft 11B.3.3.2
  enum  PeerEvent
  {
    CNCL,       ///< MLME-CancelPeerLink
    PASOPN,     ///< MLME-PassivePeerLinkOpen
    ACTOPN,     ///< MLME-ActivePeerLinkOpen
    CLS_ACPT,   ///< PeerLinkClose_Accept
    OPN_ACPT,   ///< PeerLinkOpen_Accept
    OPN_RJCT,   ///< PeerLinkOpen_Reject
    REQ_RJCT,   ///< PeerLinkOpenReject by internal reason
    CNF_ACPT,   ///< PeerLinkConfirm_Accept
    CNF_RJCT,   ///< PeerLinkConfirm_Reject
    TOR1,       ///< Timeout of retry timer
    TOR2,       ///< also timeout of retry timer
    TOC,        ///< Timeout of confirm timer
    TOH,        ///< Timeout of holding (gracefull closing) timer
  };
  
private:
  /// State transition
  void StateMachine (PeerEvent event, dot11sReasonCode = REASON11S_RESERVED);
  
  /** 
   * \name Event handlers
   * \{ 
   */
  void ClearRetryTimer ();
  void ClearConfirmTimer ();
  void ClearHoldingTimer ();
  void SetHoldingTimer ();
  void SetRetryTimer ();
  void SetConfirmTimer ();
  //\}

  /** 
   * \name Work with management frames
   * \{
   */
  void SendPeerLinkClose (dot11sReasonCode reasoncode);
  void SendPeerLinkOpen ();
  void SendPeerLinkConfirm ();
  //\}
  
  /** 
   * \name Timeout handlers 
   * \{
   */
  void HoldingTimeout ();
  void RetryTimeout ();
  void ConfirmTimeout ();
  //\}
  
private:
  ///The number of interface I am associated with
  uint32_t m_interface;
  /// pointer to mac plugin, which is responsible for peer management
  Ptr<Dot11sPeerManagerMacPlugin> m_macPlugin;
  /// Peer address
  Mac48Address m_peerAddress;
  /// My ID of this link
  uint16_t m_localLinkId;
  /// Peer ID of this link
  uint16_t m_peerLinkId;
  /// My association ID
  uint16_t m_assocId;
  /// Assoc Id assigned to me by peer
  uint16_t m_peerAssocId;
    
  /// When last beacon was sent (TODO or received?)
  Time  m_lastBeacon;
  /// Current beacon interval on corresponding interface
  Time  m_beaconInterval;
  
  /// Current state
  PeerState m_state;
  /// Mesh interface configuration
  IeDot11sConfiguration m_configuration;
  
  // State is a bitfield as defined as follows:
  // This are states for a given
  IeDot11sBeaconTiming m_beaconTiming;

  /**
   * \name Timers & counters used for internal state transitions
   * \{
   */
  Time     m_retryTimeout;
  EventId  m_retryTimer;
  Time     m_holdingTimeout;
  EventId  m_holdingTimer;
  Time     m_confirmTimeout;
  EventId  m_confirmTimer;
  uint16_t m_maxRetries;
  uint16_t m_retryCounter;
  EventId  m_beaconLossTimer;
  uint16_t  m_maxBeaconLoss;
  //\}
  
  /// ?
  void BeaconLoss ();
   
  /// How to report my status change
  SignalStatusCallback m_linkStatusCallback;
};
  
} // namespace dot11s
} //namespace ns3
#endif /* PEERLLINK_H_ */
