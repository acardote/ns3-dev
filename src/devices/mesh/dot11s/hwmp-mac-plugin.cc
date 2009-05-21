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

#include "ns3/mesh-wifi-interface-mac.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/log.h"
#include "hwmp-mac-plugin.h"
#include "dot11s-mac-header.h"
#include "hwmp-protocol.h"
#include "hwmp-tag.h"
#include "ie-dot11s-preq.h"
#include "ie-dot11s-prep.h"

namespace ns3 {
namespace dot11s {

NS_LOG_COMPONENT_DEFINE ("HwmpMacPlugin");
HwmpMacPlugin::HwmpMacPlugin (uint32_t ifIndex, Ptr<HwmpProtocol> protocol):
  m_ifIndex (ifIndex),
  m_protocol (protocol)
{
}
HwmpMacPlugin::~HwmpMacPlugin ()
{
}
void
HwmpMacPlugin::SetParent (Ptr<MeshWifiInterfaceMac> parent)
{
  m_parent = parent;
}
bool
HwmpMacPlugin::Receive (Ptr<Packet> packet, const WifiMacHeader & header)
{
  //TODO: here we fix only mesh header
  if(header.IsData())
  {
    MeshHeader meshHdr;
    HwmpTag tag;
    if(packet->PeekPacketTag (tag))
    {
      NS_ASSERT (false);
    }
    packet->RemoveHeader(meshHdr);
    m_stats.recvData ++;
    m_stats.recvDataBytes += packet->GetSize ();
    //TODO: address extension
    Mac48Address destination;
    switch (meshHdr.GetAddressExt ())
    {
      case 0:
        destination = header.GetAddr3 ();
        break;
      default:
        NS_ASSERT(false);
    };
    tag.SetSeqno (meshHdr.GetMeshSeqno ());
    if(meshHdr.GetMeshTtl () == 0)
    {
      NS_ASSERT(false);
      return false;
    }
    tag.SetTtl (meshHdr.GetMeshTtl () - 1);
    if(m_protocol->GetAddress() != destination)
      packet->AddPacketTag(tag);
    if (destination == Mac48Address::GetBroadcast ())
      if(m_protocol->DropDataFrame (meshHdr.GetMeshSeqno (), header.GetAddr4 ()) )
        return false;
  }
  if(header.IsAction())
  {
    m_stats.recvMgt ++;
    m_stats.recvMgtBytes += packet->GetSize ();
    WifiMeshActionHeader actionHdr;
    packet->RemoveHeader (actionHdr);
    WifiMeshActionHeader::ActionValue actionValue = actionHdr.GetAction ();
    if(actionHdr.GetCategory () != WifiMeshActionHeader::MESH_PATH_SELECTION)
      return true;
    switch (actionValue.pathSelection)
    {
      case WifiMeshActionHeader::PATH_REQUEST:
        {
          IePreq preq;
          m_stats.recvPreq ++;
          packet->RemoveHeader (preq);
          if(preq.GetOriginatorAddress () == m_protocol->GetAddress ())
            return false;
          if (preq.GetTtl () == 0)
            return false;
          preq.DecrementTtl ();
          m_protocol->ReceivePreq (preq, header.GetAddr2 (), m_ifIndex, header.GetAddr3 (), m_parent->GetLinkMetric(header.GetAddr2 ()));
          return false;
        }
      case WifiMeshActionHeader::PATH_REPLY:
        {
          IePrep prep;
          m_stats.recvPrep ++;
          packet->RemoveHeader (prep);
          if(prep.GetTtl () == 0)
            return false;
          prep.DecrementTtl ();
          m_protocol->ReceivePrep (prep, header.GetAddr2 (), m_ifIndex, header.GetAddr3 (), m_parent->GetLinkMetric(header.GetAddr2 ()));
          return false;
        }
      case WifiMeshActionHeader::PATH_ERROR:
        {
          IePerr perr;
          m_stats.recvPerr ++;
          packet->RemoveHeader (perr);
          m_protocol->ReceivePerr (perr, header.GetAddr2 (), m_ifIndex, header.GetAddr3 ());
          return false;
        }
      case WifiMeshActionHeader::ROOT_ANNOUNCEMENT:
        return false;
    }
  }
  return true;
}
bool
HwmpMacPlugin::UpdateOutcomingFrame (Ptr<Packet> packet, WifiMacHeader & header, Mac48Address from, Mac48Address to)
{
  if(!header.IsData ())
    return true;
  HwmpTag tag;
  bool tagExists = packet->RemovePacketTag(tag);
  if (!tagExists)
  {
     //do it this way to silence compiler
     NS_ASSERT (false);
  }
  m_stats.sentData ++;
  m_stats.sentDataBytes += packet->GetSize ();
  MeshHeader meshHdr;
  meshHdr.SetMeshSeqno(tag.GetSeqno());
  meshHdr.SetMeshTtl(tag.GetTtl());
  packet->AddHeader(meshHdr);
  header.SetAddr1(tag.GetAddress());
  return true;
}
void
HwmpMacPlugin::SendPreq(IePreq preq)
{
  m_preqQueue.push_back (preq);
  SendOnePreq ();
}
void
HwmpMacPlugin::RequestDestination (Mac48Address dst, uint32_t originator_seqno, uint32_t dst_seqno)
{
  for(std::vector<IePreq>::iterator i = m_preqQueue.begin (); i != m_preqQueue.end (); i ++)
    if(i->MayAddAddress(m_protocol->GetAddress ()))
    {
      i->AddDestinationAddressElement (m_protocol->GetDoFlag(), m_protocol->GetRfFlag(), dst, dst_seqno);
      return;
    }
  IePreq preq;
  //fill PREQ:
  preq.SetHopcount (0);
  preq.SetTTL (m_protocol->GetMaxTtl ());
  preq.SetPreqID (m_protocol->GetNextPreqId ());
  preq.SetOriginatorAddress (m_protocol->GetAddress ());
  preq.SetOriginatorSeqNumber (originator_seqno);
  preq.SetLifetime (m_protocol->GetActivePathLifetime ());
  preq.AddDestinationAddressElement (m_protocol->GetDoFlag (), m_protocol->GetRfFlag (), dst, dst_seqno);
  m_preqQueue.push_back (preq);
  //set iterator position to my preq:
  SendOnePreq ();
}
void
HwmpMacPlugin::SendOnePreq ()
{
  if(m_preqTimer.IsRunning ())
    return;
  if (m_preqQueue.size () == 0)
    return;
  //reschedule sending PREQ
  NS_ASSERT (!m_preqTimer.IsRunning());
  m_preqTimer = Simulator::Schedule (m_protocol->GetPreqMinInterval (), &HwmpMacPlugin::SendOnePreq, this);
  Ptr<Packet> packet  = Create<Packet> ();
  packet->AddHeader(m_preqQueue[0]);
  //Action header:
  WifiMeshActionHeader actionHdr;
  WifiMeshActionHeader::ActionValue action;
  action.pathSelection = WifiMeshActionHeader::PATH_REQUEST;
  actionHdr.SetAction (WifiMeshActionHeader::MESH_PATH_SELECTION, action);
  packet->AddHeader (actionHdr);
  //create 802.11 header:
  WifiMacHeader hdr;
  hdr.SetAction ();
  hdr.SetDsNotFrom ();
  hdr.SetDsNotTo ();
  hdr.SetAddr2 (m_parent->GetAddress ());
  hdr.SetAddr3 (m_protocol->GetAddress ());
  //Send Management frame
  std::vector <Mac48Address> receivers = m_protocol->GetPreqReceivers (m_ifIndex);
  for(std::vector<Mac48Address>::const_iterator i = receivers.begin (); i != receivers.end (); i ++)
  {
    hdr.SetAddr1 (*i);
    m_stats.sentPreq ++;
    m_stats.sentMgt ++;
    m_stats.sentMgtBytes += packet->GetSize ();
    m_parent->SendManagementFrame(packet, hdr);
  }
  //erase queue
  m_preqQueue.erase (m_preqQueue.begin());
}
void
HwmpMacPlugin::SendOnePerr()
{
  if(m_perrTimer.IsRunning ())
    return;
  if(m_myPerr.receivers.size () >= m_protocol->GetUnicastPerrThreshold ())
  {
    m_myPerr.receivers.clear ();
    m_myPerr.receivers.push_back (Mac48Address::GetBroadcast ());
  }
  m_perrTimer = Simulator::Schedule (m_protocol->GetPerrMinInterval (), &HwmpMacPlugin::SendOnePerr, this);
//Create packet
  Ptr<Packet> packet  = Create<Packet> ();
  packet->AddHeader(m_myPerr.perr);
  //Action header:
  WifiMeshActionHeader actionHdr;
  WifiMeshActionHeader::ActionValue action;
  action.pathSelection = WifiMeshActionHeader::PATH_ERROR;
  actionHdr.SetAction (WifiMeshActionHeader::MESH_PATH_SELECTION, action);
  packet->AddHeader (actionHdr);
  //create 802.11 header:
  WifiMacHeader hdr;
  hdr.SetAction ();
  hdr.SetDsNotFrom ();
  hdr.SetDsNotTo ();
  hdr.SetAddr2 (m_parent->GetAddress ());
  hdr.SetAddr3 (m_protocol->GetAddress ());
  //Send Management frame
  for(std::vector<Mac48Address>::const_iterator i = m_myPerr.receivers.begin (); i != m_myPerr.receivers.end (); i ++)
  {
    hdr.SetAddr1 (*i);
    m_stats.sentPerr ++;
    m_stats.sentMgt ++;
    m_stats.sentMgtBytes += packet->GetSize ();
    m_parent->SendManagementFrame(packet, hdr);
  }
  m_myPerr.perr.ResetPerr ();
  m_myPerr.receivers.clear ();
}
void
HwmpMacPlugin::SendPrep (IePrep prep, Mac48Address receiver)
{
  //Create packet
  Ptr<Packet> packet  = Create<Packet> ();
  packet->AddHeader(prep);
  //Action header:
  WifiMeshActionHeader actionHdr;
  WifiMeshActionHeader::ActionValue action;
  action.pathSelection = WifiMeshActionHeader::PATH_REPLY;
  actionHdr.SetAction (WifiMeshActionHeader::MESH_PATH_SELECTION, action);
  packet->AddHeader (actionHdr);
  //create 802.11 header:
  WifiMacHeader hdr;
  hdr.SetAction ();
  hdr.SetDsNotFrom ();
  hdr.SetDsNotTo ();
  hdr.SetAddr1 (receiver);
  hdr.SetAddr2 (m_parent->GetAddress ());
  hdr.SetAddr3 (m_protocol->GetAddress ());
  //Send Management frame
  m_stats.sentPrep ++;
  m_stats.sentMgt ++;
  m_stats.sentMgtBytes += packet->GetSize ();
  m_parent->SendManagementFrame(packet, hdr);
}
void
HwmpMacPlugin::SendPerr(IePerr perr, std::vector<Mac48Address> receivers)
{
  m_myPerr.perr.Merge(perr);
  for(unsigned int i = 0; i < receivers.size (); i ++)
  {
    bool should_add = true;
    for (unsigned int j = 0; j < m_myPerr.receivers.size (); j ++)
      if(receivers[j] == m_myPerr.receivers[i])
        should_add = false;
    if(should_add)
      m_myPerr.receivers.push_back(receivers[i]);
  }
  SendOnePerr ();
}
uint32_t
HwmpMacPlugin::GetLinkMetric(Mac48Address peerAddress) const
{
  return m_parent->GetLinkMetric(peerAddress);
}
uint16_t
HwmpMacPlugin::GetChannelId () const
{
  return m_parent->GetFrequencyChannel ();
}
void
HwmpMacPlugin::Statistics::Print (std::ostream & os) const
{
  os << "<Statistics "
    "sentPreq= \"" << sentPreq << "\"\n"
    "sentPrep=\"" << sentPrep << "\"\n"
    "sentPerr=\"" << sentPerr << "\"\n"
    "recvPreq=\"" << recvPreq << "\"\n"
    "recvPrep=\"" << recvPrep << "\"\n"
    "recvPerr=\"" << recvPerr << "\"\n"
    "sentMgt=\"" << sentMgt << "\"\n"
    "sentMgtBytes=\"" << (double)sentMgtBytes  / 1024.0 << "K\"\n"
    "recvMgt=\"" << recvMgt << "\"\n"
    "recvMgtBytes=\"" << (double)recvMgtBytes / 1204.0 << "K\"\n"
    "sentData=\"" << sentData << "\"\n"
    "sentDataBytes=\"" << (double)sentDataBytes / 1024.0 << "K\"\n"
    "recvData=\"" << recvData << "\"\n"
    "recvDataBytes=\"" << (double)recvDataBytes / 1024.0 << "K\"/>\n";
}
void
HwmpMacPlugin::Report (std::ostream & os) const
{
  os << "<HwmpMacPlugin\n"
    "address =\""<< m_parent->GetAddress () <<"\">\n";
  m_stats.Print(os);
  os << "</HwmpMacPlugin>\n";
}
void
HwmpMacPlugin::ResetStats ()
{
  m_stats = Statistics::Statistics ();
}

} //namespace dot11s
}//namespace ns3
