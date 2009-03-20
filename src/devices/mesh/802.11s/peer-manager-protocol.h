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
 * Authors: Kirill Andreev <andreev@iitp.ru>
 *          Aleksey Kovalenko <kovalenko@iitp.ru>
 */


#ifndef DOT11S_PEER_MAN_H
#define DOT11S_PEER_MAN_H

#include "ns3/mac48-address.h"
#include "ns3/wifi-net-device.h"
#include "ns3/ie-dot11s-peer-management.h"
#include "ns3/ie-dot11s-beacon-timing.h"
#include "ns3/ie-dot11s-configuration.h"
#include "ns3/event-id.h"

#include "peer-manager-plugin.h"

#include <list>
namespace ns3 {
class Dot11sPeerManagerMacPlugin;
/**
 * \ingroup dot11s
 */
class WifiPeerLinkDescriptor : public RefCountBase
{
public:
  WifiPeerLinkDescriptor ();
  /**
   * Beacon loss processing:
   */
  void  SetBeaconInformation (Time lastBeacon, Time BeaconInterval);
  void  SetMaxBeaconLoss (uint8_t maxBeaconLoss);
  /**
   * \brief Methods used to detecet peer link changes
   * \param bool if true - opened new link, if
   * false - link closed
   */
  void  SetLinkStatusCallback (Callback<void, Mac48Address, Mac48Address, bool> cb);
  /**
   * Peer link geeters/setters
   */
  void  SetPeerAddress (Mac48Address macaddr);
  /**
   * Debug purpose
   */
  void  SetLocalAddress (Mac48Address macaddr);
  void  SetLocalLinkId  (uint16_t id);
  void  SetPeerLinkId   (uint16_t id);
  void  SetLocalAid     (uint16_t aid);
  void  SetPeerAid      (uint16_t aid);
  void  SetBeaconTimingElement (IeDot11sBeaconTiming beaconTiming);
  void  SetPeerLinkDescriptorElement (IeDot11sPeerManagement peerLinkElement);
  Mac48Address GetPeerAddress () const;
  /**
   * Debug purpose
   */
  Mac48Address GetLocalAddress () const;
  uint16_t GetLocalAid () const;
  Time  GetLastBeacon () const;
  Time  GetBeaconInterval () const;
  IeDot11sBeaconTiming    GetBeaconTimingElement () const;
  IeDot11sPeerManagement  GetPeerLinkDescriptorElement () const;
  void  ClearTimingElement ();
  /* MLME */
  void  MLMECancelPeerLink (dot11sReasonCode reason);
  void  MLMEPassivePeerLinkOpen ();
  void  MLMEActivePeerLinkOpen ();
  void  MLMEPeeringRequestReject ();
#if 0
  void  MLMEBindSecurityAssociation ();
#endif
  void  PeerLinkClose (uint16_t localLinkID,uint16_t peerLinkID, dot11sReasonCode reason);
  void  PeerLinkOpenAccept (uint16_t localLinkId, IeDot11sConfiguration  conf);
  void  PeerLinkOpenReject (uint16_t localLinkId, IeDot11sConfiguration  conf, dot11sReasonCode reason);
  void  PeerLinkConfirmAccept (
    uint16_t localLinkId,
    uint16_t peerLinkId,
    uint16_t peerAid,
    IeDot11sConfiguration  conf
  );
  void  PeerLinkConfirmReject (
    uint16_t localLinkId,
    uint16_t peerLinkId,
    IeDot11sConfiguration  conf,
    dot11sReasonCode reason
  );
  bool  LinkIsEstab () const;
  bool  LinkIsIdle  () const;
private:
  enum  PeerState {
    IDLE,
    LISTEN,
    OPN_SNT,
    CNF_RCVD,
    OPN_RCVD,
    ESTAB,
    HOLDING,
  };
  enum  PeerEvent
  {
    CNCL,  /** MLME-CancelPeerLink */
    PASOPN,  /** MLME-PassivePeerLinkOpen */
    ACTOPN,  /** MLME-ActivePeerLinkOpen */
    //BNDSA,     /** MLME-BindSecurityAssociation */
    CLS_ACPT, /** PeerLinkClose_Accept */
    //CLS_IGNR, /** PeerLinkClose_Ignore */
    OPN_ACPT, /** PeerLinkOpen_Accept */
    //OPN_IGNR, /** PeerLinkOpen_Ignore */
    OPN_RJCT, /** PeerLinkOpen_Reject */
    REQ_RJCT, /** PeerLinkOpenReject by internal reason */
    CNF_ACPT, /** PeerLinkConfirm_Accept */
    //CNF_IGNR, /** PeerLinkConfirm_Ignore */
    CNF_RJCT, /** PeerLinkConfirm_Reject */
    TOR1,
    TOR2,
    TOC,
    TOH,
  };
private:
  void StateMachine (PeerEvent event,dot11sReasonCode = REASON11S_RESERVED);
  /** Events handlers */
  void ClearRetryTimer   ();
  void ClearConfirmTimer ();
  void ClearHoldingTimer ();
  void SetHoldingTimer   ();
  void SetRetryTimer     ();
  void SetConfirmTimer   ();

  void SendPeerLinkClose (dot11sReasonCode reasoncode);
  void SendPeerLinkOpen ();
  void SendPeerLinkConfirm ();
  /** Private Event */
  void HoldingTimeout ();
  void RetryTimeout   ();
  void ConfirmTimeout ();
private:
  Mac48Address m_peerAddress;
  Mac48Address m_localAddress;
  uint16_t m_localLinkId;
  uint16_t m_peerLinkId;
  // Used for beacon timing:
  // All values are stored in microseconds!
  Time  m_lastBeacon;
  Time  m_beaconInterval;
  uint16_t m_assocId; //Assigned Assoc ID
  uint16_t m_peerAssocId; //Assoc Id assigned to me by peer
  //State of our peer Link:
  PeerState m_state;

  IeDot11sConfiguration  m_configuration;
  // State is a bitfield as defined as follows:
  // This are states for a given
  IeDot11sBeaconTiming  m_beaconTiming;

  EventId  m_retryTimer;
  EventId  m_holdingTimer;
  EventId  m_confirmTimer;
  uint16_t m_retryCounter;
  /**
   * Beacon loss timers:
   */
  EventId  m_beaconLossTimer;
  uint8_t  m_maxBeaconLoss;
  void  BeaconLoss ();
  Callback<void, Mac48Address, Mac48Address, bool>  m_linkStatusCallback;
};
/**
 * \ingroup dot11s
 */
class Dot11sPeerManagerProtocol : public Object
{
public:
  Dot11sPeerManagerProtocol ();
  ~Dot11sPeerManagerProtocol ();
  static TypeId GetTypeId ();
  bool AttachPorts(std::vector<Ptr<WifiNetDevice> >);
  /** \brief Methods that handle beacon sending/receiving procedure.
   * This methods interact with MAC_layer plug-in
   * \{
   */
  /**
   * \brief When we are sending a beacon - we add a timing element to 
   * it and remember the time, when we sent a beacon (for BCA)
   * \param IeDot11sBeaconTiming is a beacon timing element that
   * should be present in beacon
   * \param port is a port sending a beacon
   * \param currentTbtt is a time of beacon sending
   * \param beaconInterval is a beacon interval on this port
   */
  IeDot11sBeaconTiming SendBeacon(uint32_t port, Time currentTbtt, Time beaconInterval);
  /**
   * \brief When we receive a beacon from peer-station, we remember
   * its beacon timing element (needed for peer choosing mechanism),
   * and remember beacon timers - last beacon and beacon interval to
   * detect beacon loss and cancel links
   * \param port is a port on which beacon was received
   * \param timingElement is a timing element of remote beacon
   */
  void ReceiveBeacon(uint32_t port, IeDot11sBeaconTiming timingElement, Mac48Address peerAddress, Time receivingTime, Time beaconInterval);
  /**
   * \}
   */
  /**
   * \brief Methods that handle Peer link management frames
   * interaction:
   * \{
   */
  /**
   * Deliver Peer link management information to the protocol-part
   * \param void is returning value - we pass a frame and forget
   * about it
   * \param uint32_t - is a port ID of a given MAC (portID rather
   * than MAC address, beacause many ports may have the same MAC)
   * \param Mac48Address is address of peer
   * \param uint16_t is association ID, which peer has assigned to
   * us
   * \param IeDot11sConfiguration is mesh configuration element
   * taken from the peer management frame
   * \param IeDot11sPeerManagement is peer link management element
   */
  void ReceivePeerLinkFrame(
      uint32_t port,
      Mac48Address peerAddress,
      uint16_t aid,
      IeDot11sConfiguration meshConfig,
      IeDot11sPeerManagement peerManagementElement
      );
  /**
   * \}
   */
private:
  /**
   * All private structures:
   * * peer link descriptors;
   * * information about received beacons
   * * pointers to proper plugins
   * \{
   */
//  struct BeaconInfo
//  {
//    Time referenceTbtt; //When one of my station's beacons was put into a beacon queue;
//    Time beaconInterval; //Beacon interval of my station;
 //   uint16_t aid; //Assoc ID
  //};
  typedef std::map<uint32_t, std::vector<Ptr<WifiPeerLinkDescriptor> > >  PeerDescriptorsMap;
//  typedef std::map<uint32_t, BeaconInfo> BeaconInfoMap;
  typedef std::map<uint32_t, Ptr<Dot11sPeerManagerMacPlugin> > PeerManagerPluginMap;
  /**
   * \}
   */
#if 0
  //Maximum peers that may be opened:
  uint8_t  m_maxNumberOfPeerLinks;
  /**
   * Peer manager identify interface by address
   * of MAC. So, for every interface we store
   * list of peer descriptors.
   */
  PeerDescriptorsMap m_peerDescriptors;
  /**
   * List of MAC pointers - to iteract with each
   * mac
   */
  MeshMacMap m_macPointers;
  uint8_t  m_numberOfActivePeers; //number of established peer links
  uint16_t m_assocId;  //last stored assoc ID
  uint16_t m_localLinkId;  //last stored local link ID
  //This Variables used in beacon miss auto-cleanup:
  //How many beacons may we lose before the link is
  //considered to be broken:
  uint8_t  m_maxBeaconLoss;
  //Periodically we scan the peer manager list of peers
  //and check if the too many  beacons were lost:
  Time  m_peerLinkCleanupPeriod;
  EventId  m_cleanupEvent;
  Ptr<WifiPeerLinkDescriptor> AddDescriptor (
    Mac48Address portAddress,
    Mac48Address peerAddress,
    Time lastBeacon,
    Time beaconInterval
  );
  void  PeerCleanup ();
  //Mechanism of choosing PEERs:
  bool  ShouldSendOpen (Mac48Address portAddress, Mac48Address peerAddress);
  bool  ShouldAcceptOpen (
    Mac48Address portAddress,
    Mac48Address peerAddress,
    dot11sReasonCode & reasonCode
  );
  //Needed for Beacon Collision Avoidance module:
  BeaconInfoMap m_myBeaconInfo;
  /**
   * Peer link Open/Close callbacks: We need to
   * inform MAC about this events.
   * \brief Interaction with peer link
   * descriptor - notify that peer link was
   * opened or closed
   * \param status true - peer link opened, peer
   * link closed otherwise
   */
  void PeerLinkStatus (Mac48Address portAddress, Mac48Address peerAddress, bool status);
#endif
};
} //namespace ns3
#endif
