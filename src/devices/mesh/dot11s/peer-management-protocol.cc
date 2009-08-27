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

#include "peer-management-protocol.h"
#include "ie-dot11s-configuration.h"
#include "ie-dot11s-id.h"
#include "ns3/mesh-point-device.h"
#include "ns3/simulator.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/random-variable.h"
#include "ns3/mesh-wifi-interface-mac.h"
#include "ns3/mesh-wifi-interface-mac-plugin.h"
#include "ns3/wifi-net-device.h"

NS_LOG_COMPONENT_DEFINE ("PeerManagementProtocol");
namespace ns3 {
namespace dot11s {
/***************************************************
 * PeerManager
 ***************************************************/
NS_OBJECT_ENSURE_REGISTERED (PeerManagementProtocol);

TypeId
PeerManagementProtocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::dot11s::PeerManagementProtocol")
  .SetParent<Object> ()
  .AddConstructor<PeerManagementProtocol> ()
  // maximum number of peer links. Now we calculate the total
  // number of peer links on all interfaces
  .AddAttribute ( "MaxNumberOfPeerLinks",
                  "Maximum number of peer links",
                  UintegerValue (32),
                  MakeUintegerAccessor (
                      &PeerManagementProtocol::m_maxNumberOfPeerLinks),
                  MakeUintegerChecker<uint8_t> ()
                )
  .AddAttribute ( "MaxBeaconLossForBeaconTiming",
                  "If maximum number of beacons were lost, station will not included in beacon timing element",
                  UintegerValue (3),
                  MakeUintegerAccessor (
                      &PeerManagementProtocol::m_maxBeaconLostForBeaconTiming),
                  MakeUintegerChecker<uint8_t> ()
                )
                ;
  return tid;
}
PeerManagementProtocol::PeerManagementProtocol () :
  m_lastAssocId (0), m_lastLocalLinkId (1), m_maxBeaconLostForBeaconTiming (3)
{
}
PeerManagementProtocol::~PeerManagementProtocol ()
{
}
void
PeerManagementProtocol::DoDispose ()
{
  //cancel cleanup event and go through the map of peer links,
  //deleting each
  for (PeerLinksMap::iterator j = m_peerLinks.begin (); j != m_peerLinks.end (); j++)
    {
      for (PeerLinksOnInterface::iterator i = j->second.begin (); i != j->second.end (); i++)
        {
          (*i) = 0;
        }
      j->second.clear ();
    }
  m_peerLinks.clear ();
  //cleaning beacon structures:
  for (BeaconInfoMap::iterator i = m_neighbourBeacons.begin (); i != m_neighbourBeacons.end (); i++)
    {
      i->second.clear ();
    }
  m_neighbourBeacons.clear ();
}

bool
PeerManagementProtocol::Install (Ptr<MeshPointDevice> mp)
{
  std::vector<Ptr<NetDevice> > interfaces = mp->GetInterfaces ();
  for (std::vector<Ptr<NetDevice> >::iterator i = interfaces.begin (); i != interfaces.end (); i++)
    {
      Ptr<WifiNetDevice> wifiNetDev = (*i)->GetObject<WifiNetDevice> ();
      if (wifiNetDev == 0)
        {
          return false;
        }
      Ptr<MeshWifiInterfaceMac> mac = wifiNetDev->GetMac ()->GetObject<MeshWifiInterfaceMac> ();
      if (mac == 0)
        {
          return false;
        }
      Ptr<PeerManagementProtocolMac> plugin = Create<PeerManagementProtocolMac> ((*i)->GetIfIndex (), this);
      mac->InstallPlugin (plugin);
      m_plugins[(*i)->GetIfIndex ()] = plugin;
      PeerLinksOnInterface newmap;
      m_peerLinks[(*i)->GetIfIndex ()] = newmap;
    }
  // Mesh point aggregates all installed protocols
  m_address = Mac48Address::ConvertFrom (mp->GetAddress ());
  mp->AggregateObject (this);
  return true;
}

Ptr<IeBeaconTiming>
PeerManagementProtocol::GetBeaconTimingElement (uint32_t interface)
{
  Ptr<IeBeaconTiming> retval = Create<IeBeaconTiming> ();
  BeaconInfoMap::iterator i = m_neighbourBeacons.find (interface);
  if (i == m_neighbourBeacons.end ())
    {
      return retval;
    }
  bool cleaned = false;
  while (!cleaned)
    {
      BeaconsOnInterface::iterator start = i->second.begin ();
      for (BeaconsOnInterface::iterator j = start; j != i->second.end (); j++)
        {
          //check beacon loss and make a timing element
          //if last beacon was m_maxBeaconLostForBeaconTiming beacons ago - we do not put it to the
          //timing element
          if ((j->second.referenceTbtt + j->second.beaconInterval * Scalar (m_maxBeaconLostForBeaconTiming))
              < Simulator::Now ())
            {
              start = j;
              i->second.erase (j);
              break;
            }
        }
      cleaned = true;
    }
  for (BeaconsOnInterface::const_iterator j = i->second.begin (); j != i->second.end (); j++)
    {
      retval->AddNeighboursTimingElementUnit (j->second.aid, j->second.referenceTbtt,
          j->second.beaconInterval);
    }
  return retval;
}

void
PeerManagementProtocol::FillBeaconInfo (uint32_t interface, Mac48Address peerAddress, Time receivingTime,
    Time beaconInterval)
{
  BeaconInfoMap::iterator i = m_neighbourBeacons.find (interface);
  if (i == m_neighbourBeacons.end ())
    {
      BeaconsOnInterface newMap;
      m_neighbourBeacons[interface] = newMap;
    }
  i = m_neighbourBeacons.find (interface);
  BeaconsOnInterface::iterator j = i->second.find (peerAddress);
  if (j == i->second.end ())
    {
      BeaconInfo newInfo;
      newInfo.referenceTbtt = receivingTime;
      newInfo.beaconInterval = beaconInterval;
      newInfo.aid = m_lastAssocId++;
      if (m_lastAssocId == 0xff)
        {
          m_lastAssocId = 0;
        }
      i->second[peerAddress] = newInfo;
    }
  else
    {
      j->second.referenceTbtt = receivingTime;
      j->second.beaconInterval = beaconInterval;
    }
}

void
PeerManagementProtocol::UpdatePeerBeaconTiming (uint32_t interface, bool meshBeacon,
    IeBeaconTiming timingElement, Mac48Address peerAddress, Time receivingTime, Time beaconInterval)
{
  FillBeaconInfo (interface, peerAddress, receivingTime, beaconInterval);
  if (!meshBeacon)
    {
      return;
    }
  //BCA:
  PeerManagementProtocolMacMap::iterator plugin = m_plugins.find (interface);
  NS_ASSERT (plugin != m_plugins.end ());
  Time shift = GetNextBeaconShift (interface);
  if (TimeToTu (shift) != 0)
    {
      plugin->second->SetBeaconShift (shift);
    }
  //PM STATE Machine
  //Check that a given beacon is not from our interface
  for (PeerManagementProtocolMacMap::const_iterator i = m_plugins.begin (); i != m_plugins.end (); i++)
    {
      if (i->second->GetAddress () == peerAddress)
        {
          return;
        }
    }
  Ptr<PeerLink> peerLink = FindPeerLink (interface, peerAddress);
  if (peerLink != 0)
    {
      peerLink->SetBeaconTimingElement (timingElement);
      peerLink->SetBeaconInformation (receivingTime, beaconInterval);
    }
  else
    {
      if (ShouldSendOpen (interface, peerAddress))
        {
          peerLink = InitiateLink (interface, peerAddress, Mac48Address::GetBroadcast (), receivingTime,
              beaconInterval);
          peerLink->SetBeaconTimingElement (timingElement);
          peerLink->MLMEActivePeerLinkOpen ();
        }
    }
}

void
PeerManagementProtocol::ReceivePeerLinkFrame (uint32_t interface, Mac48Address peerAddress,
    Mac48Address peerMeshPointAddress, uint16_t aid, IePeerManagement peerManagementElement,
    IeConfiguration meshConfig)
{
  Ptr<PeerLink> peerLink = FindPeerLink (interface, peerAddress);
  if (peerManagementElement.SubtypeIsOpen ())
    {
      PmpReasonCode reasonCode;
      bool reject = !(ShouldAcceptOpen (interface, peerAddress, reasonCode));
      if (peerLink == 0)
        {
          peerLink = InitiateLink (interface, peerAddress, peerMeshPointAddress, Simulator::Now (), Seconds (
              1.0));
        }
      if (!reject)
        {
          peerLink->OpenAccept (peerManagementElement.GetLocalLinkId (), meshConfig, peerMeshPointAddress);
        }
      else
        {
          peerLink->OpenReject (peerManagementElement.GetLocalLinkId (), meshConfig, peerMeshPointAddress,
              reasonCode);
        }
    }
  if (peerLink == 0)
    {
      return;
    }
  if (peerManagementElement.SubtypeIsConfirm ())
    {
      peerLink->ConfirmAccept (peerManagementElement.GetLocalLinkId (),
          peerManagementElement.GetPeerLinkId (), aid, meshConfig, peerMeshPointAddress);
    }
  if (peerManagementElement.SubtypeIsClose ())
    {
      peerLink->Close (peerManagementElement.GetLocalLinkId (), peerManagementElement.GetPeerLinkId (),
          peerManagementElement.GetReasonCode ());
    }
}
void
PeerManagementProtocol::ConfigurationMismatch (uint32_t interface, Mac48Address peerAddress)
{
  Ptr<PeerLink> peerLink = FindPeerLink (interface, peerAddress);
  if (peerLink != 0)
    {
      peerLink->MLMECancelPeerLink (REASON11S_MESH_CAPABILITY_POLICY_VIOLATION);
    }
}
void
PeerManagementProtocol::TransmissionFailure (uint32_t interface, Mac48Address peerAddress)
{
  NS_LOG_DEBUG("transmission failed between "<<GetAddress () << " and " << peerAddress << " failed, link will be colsed");
  Ptr<PeerLink> peerLink = FindPeerLink(interface, peerAddress);
  if (peerLink != 0)
    {
      peerLink->TransmissionFailure ();
    }
}
void
PeerManagementProtocol::TransmissionSuccess (uint32_t interface, Mac48Address peerAddress)
{
  NS_LOG_DEBUG("transmission success "<<GetAddress () << " and " << peerAddress << " failed, link will be colsed");
  Ptr<PeerLink> peerLink = FindPeerLink(interface, peerAddress);
  if (peerLink != 0)
    {
      peerLink->TransmissionSuccess ();
    }
}
Ptr<PeerLink>
PeerManagementProtocol::InitiateLink (uint32_t interface, Mac48Address peerAddress,
    Mac48Address peerMeshPointAddress, Time lastBeacon, Time beaconInterval)
{
  Ptr<PeerLink> new_link = CreateObject<PeerLink> ();
  if (m_lastLocalLinkId == 0xff)
    {
      m_lastLocalLinkId = 0;
    }
  //find a beacon entry
  BeaconInfoMap::iterator beaconsOnInterface = m_neighbourBeacons.find (interface);
  if (beaconsOnInterface == m_neighbourBeacons.end ())
    {
      FillBeaconInfo (interface, peerAddress, lastBeacon, beaconInterval);
    }
  beaconsOnInterface = m_neighbourBeacons.find (interface);
  BeaconsOnInterface::iterator beacon = beaconsOnInterface->second.find (peerAddress);
  if (beacon == beaconsOnInterface->second.end ())
    {
      FillBeaconInfo (interface, peerAddress, lastBeacon, beaconInterval);
    }
  beacon = beaconsOnInterface->second.find (peerAddress);
  //find a peer link  - it must not exist
  if (FindPeerLink (interface, peerAddress) != 0)
    {
      NS_FATAL_ERROR ("Peer link must not exist.");
    }
  // Plugin must exist
  PeerManagementProtocolMacMap::iterator plugin = m_plugins.find (interface);
  NS_ASSERT (plugin != m_plugins.end ());
  PeerLinksMap::iterator iface = m_peerLinks.find (interface);
  NS_ASSERT (iface != m_peerLinks.end ());
  new_link->SetLocalAid (beacon->second.aid);
  new_link->SetInterface (interface);
  new_link->SetLocalLinkId (m_lastLocalLinkId++);
  new_link->SetPeerAddress (peerAddress);
  new_link->SetPeerMeshPointAddress (peerMeshPointAddress);
  new_link->SetBeaconInformation (lastBeacon, beaconInterval);
  new_link->SetMacPlugin (plugin->second);
  new_link->MLMESetSignalStatusCallback (MakeCallback (&PeerManagementProtocol::PeerLinkStatus, this));
  iface->second.push_back (new_link);
  return new_link;
}
Ptr<PeerLink>
PeerManagementProtocol::FindPeerLink (uint32_t interface, Mac48Address peerAddress)
{
  PeerLinksMap::iterator iface = m_peerLinks.find (interface);
  NS_ASSERT (iface != m_peerLinks.end ());
  for (PeerLinksOnInterface::iterator i = iface->second.begin (); i != iface->second.end (); i++)
    {
      if ((*i)->GetPeerAddress () == peerAddress)
        {
          if ((*i)->LinkIsIdle ())
            {
              (*i) = 0;
              (iface->second).erase (i);
              return 0;
            }
          else
            {
              return (*i);
            }
        }
    }
  return 0;
}
void
PeerManagementProtocol::SetPeerLinkStatusCallback (
    Callback<void, Mac48Address, Mac48Address, uint32_t, bool> cb)
{
  m_peerStatusCallback = cb;
}
std::vector<Mac48Address>
PeerManagementProtocol::GetActiveLinks (uint32_t interface)
{
  std::vector<Mac48Address> retval;
  PeerLinksMap::iterator iface = m_peerLinks.find (interface);
  NS_ASSERT (iface != m_peerLinks.end ());
  for (PeerLinksOnInterface::iterator i = iface->second.begin (); i != iface->second.end (); i++)
    {
      if ((*i)->LinkIsEstab ())
        {
          retval.push_back ((*i)->GetPeerAddress ());
        }
    }
  return retval;
}
bool
PeerManagementProtocol::IsActiveLink (uint32_t interface, Mac48Address peerAddress)
{
  Ptr<PeerLink> peerLink = FindPeerLink (interface, peerAddress);
  if (peerLink != 0)
    {
      return (peerLink->LinkIsEstab ());
    }
  return false;
}
bool
PeerManagementProtocol::ShouldSendOpen (uint32_t interface, Mac48Address peerAddress)
{
  return (m_stats.linksTotal <= m_maxNumberOfPeerLinks);
}
bool
PeerManagementProtocol::ShouldAcceptOpen (uint32_t interface, Mac48Address peerAddress,
    PmpReasonCode & reasonCode)
{
  if (m_stats.linksTotal > m_maxNumberOfPeerLinks)
    {
      reasonCode = REASON11S_MESH_MAX_PEERS;
      return false;
    }
  return true;
}
Time
PeerManagementProtocol::GetNextBeaconShift (uint32_t interface)
{
  //REMINDER:: in timing element  1) last beacon reception time is measured in units of 256 microseconds
  //                              2) beacon interval is mesured in units of 1024 microseconds
  //                              3) hereafter TU = 1024 microseconds
  //So, the shift is a random integer variable uniformly distributed in [-15;-1] U [1;15]
  static int maxShift = 15;
  static int minShift = 1;
  PeerLinksMap::iterator iface = m_peerLinks.find (interface);
  NS_ASSERT (iface != m_peerLinks.end ());
  PeerManagementProtocolMacMap::iterator plugin = m_plugins.find (interface);
  NS_ASSERT (plugin != m_plugins.end ());
  std::pair<Time, Time> myBeacon = plugin->second->GetBeaconInfo ();
  if (Simulator::Now () + TuToTime (maxShift) > myBeacon.first + myBeacon.second)
    {
      return MicroSeconds (0);
    }
  for (PeerLinksOnInterface::iterator i = iface->second.begin (); i != iface->second.end (); i++)
    {
      IeBeaconTiming::NeighboursTimingUnitsList neighbours;
      if ((*i)->LinkIsIdle ())
        {
          continue;
        }
      neighbours = (*i)->GetBeaconTimingElement ().GetNeighboursTimingElementsList ();
      //Going through all my timing elements and detecting future beacon collisions
      for (IeBeaconTiming::NeighboursTimingUnitsList::const_iterator j = neighbours.begin (); j
          != neighbours.end (); j++)
        {
          //We apply MBAC only if beacon Intervals are equal
          if ((*j)->GetBeaconInterval () == TimeToTu (myBeacon.second))
            {
              //Apply MBCA if future beacons may coinside
              if ((TimeToTu (myBeacon.first) - ((*j)->GetLastBeacon () / 4)) % ((*j)->GetBeaconInterval ())
                  == 0)
                {
                  UniformVariable randomSign (-1, 1);
                  UniformVariable randomShift (minShift, maxShift);
                  int beaconShift = randomShift.GetInteger (minShift, maxShift) * ((randomSign.GetValue ()
                      >= 0) ? 1 : -1);
                  NS_LOG_DEBUG ("Apply MBCA: Shift value = " << beaconShift << " beacon TUs");
                  //Do not shift to the past!
                  return (TuToTime (beaconShift) + Simulator::Now () < myBeacon.first) ? TuToTime (
                      beaconShift) : TuToTime (0);
                }
            }
        }
    }
  return MicroSeconds (0);
}
Time
PeerManagementProtocol::TuToTime (uint32_t x)
{
  return MicroSeconds (x * 1024);
}
uint32_t
PeerManagementProtocol::TimeToTu (Time x)
{
  return (uint32_t) (x.GetMicroSeconds () / 1024);
}
void
PeerManagementProtocol::PeerLinkStatus (uint32_t interface, Mac48Address peerAddress,
    Mac48Address peerMeshPointAddress, PeerLink::PeerState ostate, PeerLink::PeerState nstate)
{
  PeerManagementProtocolMacMap::iterator plugin = m_plugins.find (interface);
  NS_ASSERT (plugin != m_plugins.end ());
  NS_LOG_DEBUG ("Link between me:" << m_address << " my interface:" << plugin->second->GetAddress ()
      << " and peer mesh point:" << peerMeshPointAddress << " and its interface:" << peerAddress
      << ", at my interface ID:" << interface << ". State movement:" << ostate << " -> " << nstate);
  if ((nstate == PeerLink::ESTAB) && (ostate != PeerLink::ESTAB))
    {
      m_stats.linksOpened++;
      m_stats.linksTotal++;
      if (!m_peerStatusCallback.IsNull ())
        {
          m_peerStatusCallback (peerMeshPointAddress, peerAddress, interface, true);
        }
    }
  if ((ostate == PeerLink::ESTAB) && (nstate != PeerLink::ESTAB))
    {
      m_stats.linksClosed++;
      m_stats.linksTotal--;
      if (!m_peerStatusCallback.IsNull ())
        {
          m_peerStatusCallback (peerMeshPointAddress, peerAddress, interface, false);
        }
    }
  if (nstate == PeerLink::IDLE)
    {
      Ptr<PeerLink> link = FindPeerLink (interface, peerAddress);
      NS_ASSERT (link == 0);
    }
}
uint8_t
PeerManagementProtocol::GetNumberOfLinks ()
{
  return m_stats.linksTotal;
}
Ptr<IeMeshId>
PeerManagementProtocol::GetMeshId () const
{
  NS_ASSERT (m_meshId != 0);
  return m_meshId;
}
void
PeerManagementProtocol::SetMeshId (std::string s)
{
  m_meshId = Create<IeMeshId> (s);
}
Mac48Address
PeerManagementProtocol::GetAddress ()
{
  return m_address;
}
PeerManagementProtocol::Statistics::Statistics (uint16_t t) :
  linksTotal (t), linksOpened (0), linksClosed (0)
{
}
void
PeerManagementProtocol::Statistics::Print (std::ostream & os) const
{
  os << "<Statistics "
    "linksTotal=\"" << linksTotal << "\" "
    "linksOpened=\"" << linksOpened << "\" "
    "linksClosed=\"" << linksClosed << "\"/>" << std::endl;
}
void
PeerManagementProtocol::Report (std::ostream & os) const
{
  os << "<PeerManagementProtocol>" << std::endl;
  m_stats.Print (os);
  for (PeerManagementProtocolMacMap::const_iterator plugins = m_plugins.begin (); plugins != m_plugins.end (); plugins++)
    {
      //Take statistics from plugin:
      plugins->second->Report (os);
      //Print all active peer links:
      PeerLinksMap::const_iterator iface = m_peerLinks.find (plugins->second->m_ifIndex);
      NS_ASSERT (iface != m_peerLinks.end ());
      for (PeerLinksOnInterface::const_iterator i = iface->second.begin (); i != iface->second.end (); i++)
        {
          (*i)->Report (os);
        }
    }
  os << "</PeerManagementProtocol>" << std::endl;
}
void
PeerManagementProtocol::ResetStats ()
{
  m_stats = Statistics::Statistics (m_stats.linksTotal); // don't reset number of links
  for (PeerManagementProtocolMacMap::const_iterator plugins = m_plugins.begin (); plugins != m_plugins.end (); plugins++)
    {
      plugins->second->ResetStats ();
    }
}

} // namespace dot11s
} //namespace ns3

