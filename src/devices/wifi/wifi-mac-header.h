/* -*-  Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 INRIA
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
#ifndef WIFI_MAC_HEADER_H
#define WIFI_MAC_HEADER_H

#include "ns3/header.h"
#include "ns3/mac48-address.h"
#include <stdint.h>

namespace ns3 {

enum WifiMacType_e {
  WIFI_MAC_CTL_RTS = 0,
  WIFI_MAC_CTL_CTS,
  WIFI_MAC_CTL_ACK,
  WIFI_MAC_CTL_BACKREQ,
  WIFI_MAC_CTL_BACKRESP,

  WIFI_MAC_MGT_BEACON,
  WIFI_MAC_MGT_ASSOCIATION_REQUEST,
  WIFI_MAC_MGT_ASSOCIATION_RESPONSE,
  WIFI_MAC_MGT_DISASSOCIATION,
  WIFI_MAC_MGT_REASSOCIATION_REQUEST,
  WIFI_MAC_MGT_REASSOCIATION_RESPONSE,
  WIFI_MAC_MGT_PROBE_REQUEST,
  WIFI_MAC_MGT_PROBE_RESPONSE,
  WIFI_MAC_MGT_AUTHENTICATION,
  WIFI_MAC_MGT_DEAUTHENTICATION,

  WIFI_MAC_DATA,
  WIFI_MAC_DATA_CFACK,
  WIFI_MAC_DATA_CFPOLL,
  WIFI_MAC_DATA_CFACK_CFPOLL,
  WIFI_MAC_DATA_NULL,
  WIFI_MAC_DATA_NULL_CFACK,
  WIFI_MAC_DATA_NULL_CFPOLL,
  WIFI_MAC_DATA_NULL_CFACK_CFPOLL,
  WIFI_MAC_QOSDATA,
  WIFI_MAC_QOSDATA_CFACK,
  WIFI_MAC_QOSDATA_CFPOLL,
  WIFI_MAC_QOSDATA_CFACK_CFPOLL,
  WIFI_MAC_QOSDATA_NULL,
  WIFI_MAC_QOSDATA_NULL_CFPOLL,
  WIFI_MAC_QOSDATA_NULL_CFACK_CFPOLL,
};

class WifiMacHeader : public Header 
{
public:
  WifiMacHeader ();
  ~WifiMacHeader ();
  static uint32_t GetUid (void);
  std::string GetName (void) const;
  void Print (std::ostream &os) const;
  uint32_t GetSerializedSize (void) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);


  void SetAssocReq (void);
  void SetAssocResp (void);
  void SetProbeReq (void);
  void SetProbeResp (void);
  void SetTypeData (void);
  void SetDsFrom (void);
  void SetDsNotFrom (void);
  void SetDsTo (void);
  void SetDsNotTo (void);
  void SetAddr1 (Mac48Address address);
  void SetAddr2 (Mac48Address address);
  void SetAddr3 (Mac48Address address);
  void SetAddr4 (Mac48Address address);
  void SetType (enum WifiMacType_e type);
  void SetDuration (uint16_t duration);
  void SetDurationUs (uint64_t duration);
  void SetId (uint16_t id);
  void SetSequenceNumber (uint16_t seq);
  void SetFragmentNumber (uint8_t frag);
  void SetNoMoreFragments (void);
  void SetMoreFragments (void);
  void SetRetry (void);
  void SetNoRetry (void);
  void SetQosTid (uint8_t tid);
  void SetQosTxopLimit (uint8_t txop);

  Mac48Address GetAddr1 (void) const;
  Mac48Address GetAddr2 (void) const;
  Mac48Address GetAddr3 (void) const;
  Mac48Address GetAddr4 (void) const;
  enum WifiMacType_e GetType (void) const;
  bool IsFromDs (void) const;
  bool IsToDs (void) const;
  bool IsData (void) const;
  bool IsQosData (void) const;
  bool IsCtl (void) const;
  bool IsMgt (void) const;
  bool IsCfpoll (void) const;
  bool IsRts (void) const;
  bool IsCts (void) const;
  bool IsAck (void) const;
  bool IsAssocReq (void) const;
  bool IsAssocResp (void) const;
  bool IsReassocReq (void) const;
  bool IsReassocResp (void) const;
  bool IsProbeReq (void) const;
  bool IsProbeResp (void) const;
  bool IsBeacon (void) const;
  bool IsDisassociation (void) const;
  bool IsAuthentication (void) const;
  bool IsDeauthentication (void) const;
  uint16_t GetDuration (void) const;
  uint64_t GetDurationUs (void) const;
  uint16_t GetSequenceControl (void) const;
  uint16_t GetSequenceNumber (void) const;
  uint16_t GetFragmentNumber (void) const;
  bool IsRetry (void) const;
  bool IsMoreFragments (void) const;
  bool IsQosBlockAck (void) const;
  bool IsQosNoAck (void) const;
  bool IsQosAck (void) const;
  uint8_t GetQosTid (void) const;
  uint8_t GetQosTxopLimit (void) const;

  void SetDurationS (double duration);
  uint32_t GetSize (void) const;
  char const *GetTypeString (void) const;


private:
  uint16_t GetFrameControl (void) const;
  uint16_t GetQosControl (void) const;
  void SetFrameControl (uint16_t control);
  void SetSequenceControl (uint16_t seq);
  void SetQosControl (uint16_t qos);
  void PrintFrameControl (std::ostream &os) const;

  uint16_t m_ctrlType      : 2;
  uint16_t m_ctrlSubtype   : 4;
  uint16_t m_ctrlToDs     : 1;
  uint16_t m_ctrlFromDs   : 1;
  uint16_t m_ctrlMoreFrag : 1;
  uint16_t m_ctrlRetry     : 1;
  uint16_t m_ctrlPwrMgt   : 1;
  uint16_t m_ctrlMoreData : 1;
  uint16_t m_ctrlWep       : 1;
  uint16_t m_ctrlOrder     : 1;
  uint16_t m_duration;
  Mac48Address m_addr1;
  Mac48Address m_addr2;
  Mac48Address m_addr3;
  uint16_t m_seqFrag : 4;
  uint16_t m_seqSeq  : 12;
  Mac48Address m_addr4;
  uint16_t m_qosTid : 4;
  uint16_t m_qosEosp : 1;
  uint16_t m_qosAckPolicy : 2;
  uint16_t m_qosStuff: 8;
};

} // namespace ns3



#endif /* WIFI_MAC_HEADER_H */
