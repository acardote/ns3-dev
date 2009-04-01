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
 *         Pavel Boyko <boyko@iitp.ru>
 */


#include "dot11s-helper.h"

#include "ns3/mesh-point-device.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-phy.h"
#include "ns3/wifi-channel.h"
#include "ns3/wifi-remote-station-manager.h"
#include "ns3/mesh-wifi-interface-mac.h"

namespace ns3 {
namespace dot11s {

MeshWifiHelper::MeshWifiHelper ()
{
}

MeshWifiHelper::~MeshWifiHelper ()
{
}

  void
MeshWifiHelper::SetRemoteStationManager (std::string type,
    std::string n0, const AttributeValue &v0,
    std::string n1, const AttributeValue &v1,
    std::string n2, const AttributeValue &v2,
    std::string n3, const AttributeValue &v3,
    std::string n4, const AttributeValue &v4,
    std::string n5, const AttributeValue &v5,
    std::string n6, const AttributeValue &v6,
    std::string n7, const AttributeValue &v7)
{
  m_stationManager = ObjectFactory ();
  m_stationManager.SetTypeId (type);
  m_stationManager.Set (n0, v0);
  m_stationManager.Set (n1, v1);
  m_stationManager.Set (n2, v2);
  m_stationManager.Set (n3, v3);
  m_stationManager.Set (n4, v4);
  m_stationManager.Set (n5, v5);
  m_stationManager.Set (n6, v6);
  m_stationManager.Set (n7, v7);
}

  void
MeshWifiHelper::SetMac (std::string type,
    std::string n0, const AttributeValue &v0,
    std::string n1, const AttributeValue &v1,
    std::string n2, const AttributeValue &v2,
    std::string n3, const AttributeValue &v3,
    std::string n4, const AttributeValue &v4,
    std::string n5, const AttributeValue &v5,
    std::string n6, const AttributeValue &v6,
    std::string n7, const AttributeValue &v7)
{
  m_meshMac = ObjectFactory ();
  m_meshMac.SetTypeId (type);
  m_meshMac.Set (n0, v0);
  m_meshMac.Set (n1, v1);
  m_meshMac.Set (n2, v2);
  m_meshMac.Set (n3, v3);
  m_meshMac.Set (n4, v4);
  m_meshMac.Set (n5, v5);
  m_meshMac.Set (n6, v6);
  m_meshMac.Set (n7, v7);
}
void
MeshWifiHelper::SetPeerManager (std::string type,
    std::string n0, const AttributeValue &v0,
    std::string n1, const AttributeValue &v1,
    std::string n2, const AttributeValue &v2,
    std::string n3, const AttributeValue &v3,
    std::string n4, const AttributeValue &v4,
    std::string n5, const AttributeValue &v5,
    std::string n6, const AttributeValue &v6,
    std::string n7, const AttributeValue &v7)
{
  m_peerMan = ObjectFactory ();
  m_peerMan.SetTypeId (type);
  m_peerMan.Set (n0, v0);
  m_peerMan.Set (n1, v1);
  m_peerMan.Set (n2, v2);
  m_peerMan.Set (n3, v3);
  m_peerMan.Set (n4, v4);
  m_peerMan.Set (n5, v5);
  m_peerMan.Set (n6, v6);
  m_peerMan.Set (n7, v7);
}
void
MeshWifiHelper::SetRouting (std::string type,
    std::string n0, const AttributeValue &v0,
    std::string n1, const AttributeValue &v1,
    std::string n2, const AttributeValue &v2,
    std::string n3, const AttributeValue &v3,
    std::string n4, const AttributeValue &v4,
    std::string n5, const AttributeValue &v5,
    std::string n6, const AttributeValue &v6,
    std::string n7, const AttributeValue &v7)
{
  m_routing = ObjectFactory ();
  m_routing.SetTypeId (type);
  m_routing.Set (n0, v0);
  m_routing.Set (n1, v1);
  m_routing.Set (n2, v2);
  m_routing.Set (n3, v3);
  m_routing.Set (n4, v4);
  m_routing.Set (n5, v5);
  m_routing.Set (n6, v6);
  m_routing.Set (n7, v7);
}
void
MeshWifiHelper::SetL2RoutingNetDevice (std::string type,
    std::string n0, const AttributeValue &v0,
    std::string n1, const AttributeValue &v1,
    std::string n2, const AttributeValue &v2,
    std::string n3, const AttributeValue &v3,
    std::string n4, const AttributeValue &v4,
    std::string n5, const AttributeValue &v5,
    std::string n6, const AttributeValue &v6,
    std::string n7, const AttributeValue &v7
    )
{
  m_deviceFactory = ObjectFactory ();
  m_deviceFactory.SetTypeId (type);
  m_deviceFactory.Set (n0, v0);
  m_deviceFactory.Set (n1, v1);
  m_deviceFactory.Set (n2, v2);
  m_deviceFactory.Set (n3, v3);
  m_deviceFactory.Set (n4, v4);
  m_deviceFactory.Set (n5, v5);
  m_deviceFactory.Set (n6, v6);
  m_deviceFactory.Set (n7, v7);
}
NetDeviceContainer
MeshWifiHelper::Install (const WifiPhyHelper &phyHelper, NodeContainer c) const
{
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
  {
    Ptr<Node> node = *i;
    // Create a mesh point device:
    Ptr<MeshPointDevice> mp = m_deviceFactory.Create<MeshPointDevice> ();
    std::vector<Ptr<NetDevice> > mpInterfacess;
    // Creating interface:
    {
      Ptr<WifiNetDevice> device = CreateObject<WifiNetDevice> ();
      //Creating MAC for this interface
      Ptr<MeshWifiInterfaceMac> mac = m_meshMac.Create<MeshWifiInterfaceMac> ();
      Ptr<WifiRemoteStationManager> manager = m_stationManager.Create<WifiRemoteStationManager> ();
      Ptr<WifiPhy> phy = phyHelper.Create (node, device);
      mac->SetAddress (Mac48Address::Allocate ());
      device->SetMac (mac);
      device->SetPhy (phy);
      device->SetRemoteStationManager (manager);
      node->AddDevice (device);
      mpInterfacess.push_back (device);
    }
    node -> AddDevice (mp);
    for (std::vector<Ptr<NetDevice> > ::iterator iter=mpInterfacess.begin ();iter != mpInterfacess.end(); ++iter)
      mp->AddInterface (*iter);
    mpInterfacess.clear ();
    //Install protocols:
    Ptr<PeerManagementProtocol> peer = m_peerMan.Create<PeerManagementProtocol> ();
    NS_ASSERT(peer->Install(mp));
    Ptr<HwmpProtocol> hwmp = m_routing.Create<HwmpProtocol> ();
    peer->SetPeerLinkStatusCallback(MakeCallback(&HwmpProtocol::PeerLinkStatus, hwmp));
    hwmp->SetNeighboursCallback(MakeCallback(&PeerManagementProtocol::GetActiveLinks, peer));
    NS_ASSERT(hwmp->Install(mp));
    devices.Add (mp);
  }
  return devices;
}

NetDeviceContainer
MeshWifiHelper::Install (const WifiPhyHelper &phy, Ptr<Node> node) const
{
  return Install (phy, NodeContainer (node));
}
  
} // namespace dot11s
} //namespace ns3

