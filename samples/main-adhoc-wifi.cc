/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006,2007 INRIA
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

#include "ns3/wifi-net-device.h"
#include "ns3/wifi-channel.h"
#include "ns3/simulator.h"
#include "ns3/callback.h"
#include "ns3/ptr.h"
#include "ns3/node.h"
#include "ns3/internet-node.h"
#include "ns3/onoff-application.h"
#include "ns3/static-mobility-model.h"
#include "ns3/ipv4.h"
#include "ns3/random-variable.h"
#include "ns3/inet-socket-address.h"
#include "ns3/global-route-manager.h"
#include "ns3/packet.h"
#include "ns3/socket.h"
#include "ns3/socket-factory.h"


#include <iostream>

using namespace ns3;
static uint32_t g_bytesTotal = 0;

static Ptr<Node>
CreateAdhocNode (Ptr<WifiChannel> channel,
                 Position position, const char *address)
{
  Ptr<Node> node = Create<InternetNode> ();  
  Ptr<AdhocWifiNetDevice> device = Create<AdhocWifiNetDevice> (node);
  device->ConnectTo (channel);
  Ptr<MobilityModel> mobility = Create<StaticMobilityModel> ();
  mobility->Set (position);
  node->AddInterface (mobility);
  
  Ptr<Ipv4> ipv4 = node->QueryInterface<Ipv4> (Ipv4::iid);
  uint32_t index = ipv4->AddInterface (device);
  ipv4->SetAddress (index, Ipv4Address (address));
  ipv4->SetNetworkMask (index, Ipv4Mask ("255.255.0.0"));
  ipv4->SetUp (index);
  return node;
}

static void
SetPosition (Ptr<Node> node, Position position)
{
  Ptr<MobilityModel> mobility = node->QueryInterface<MobilityModel> (MobilityModel::iid);
  mobility->Set (position);
}

static Position
GetPosition (Ptr<Node> node)
{
  Ptr<MobilityModel> mobility = node->QueryInterface<MobilityModel> (MobilityModel::iid);
  return mobility->Get ();
}

static void 
AdvancePosition (Ptr<Node> node) 
{
  Position pos = GetPosition (node);
  pos.x += 50.0;
  if (pos.x >= 2100.0) {
    return;
  }
  SetPosition (node, pos);
  //std::cout << "x="<<pos.x << std::endl;
  Simulator::Schedule (Seconds (1.0), &AdvancePosition, node);
}

static void
Printer (Ptr<Node> node)
{
  Ptr<MobilityModel> mobility = node->QueryInterface<MobilityModel> (MobilityModel::iid);
  Position position = mobility->Get ();
  double mbs = ((g_bytesTotal * 8.0) / 1000000);
  g_bytesTotal = 0;
  std::cout << position.x << " " << mbs << std::endl;
  if (Simulator::Now ().GetSeconds () < 43.0)
    {
      Simulator::Schedule (Seconds (1.0), &Printer, node);
    }
}

static void
ReceivePacket (Ptr<Socket> socket, const Packet &packet, const Address &address)
{
  g_bytesTotal += packet.GetSize ();
}

static Ptr<Socket>
SetupUdpReceive (Ptr<Node> node, uint16_t port)
{
  InterfaceId iid = InterfaceId::LookupByName ("Udp");
  Ptr<SocketFactory> socketFactory = node->QueryInterface<SocketFactory> (iid);

  Ptr<Socket> sink = socketFactory->CreateSocket ();
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), port);
  sink->Bind (local);
  sink->SetRecvCallback (MakeCallback (&ReceivePacket));
  return sink;
}

int main (int argc, char *argv[])
{
  Simulator::SetLinkedList ();

  Packet::EnableMetadata ();

  //Simulator::EnableLogTo ("80211.log");

  // enable rts cts all the time.
  DefaultValue::Bind ("WifiRtsCtsThreshold", "0");
  // disable fragmentation
  DefaultValue::Bind ("WifiFragmentationThreshold", "2200");
  DefaultValue::Bind ("WifiRateControlAlgorithm", "Aarf");
  //DefaultValue::Bind ("WifiRateControlAlgorithm", "Arf");

  Ptr<WifiChannel> channel = Create<WifiChannel> ();

  Ptr<Node> a = CreateAdhocNode (channel, 
                                 Position (5.0,0.0,0.0),
                                 "192.168.0.1");
  Ptr<Node> b = CreateAdhocNode (channel,
                                 Position (0.0, 0.0, 0.0),
                                 "192.168.0.2");

  Simulator::Schedule (Seconds (1.0), &AdvancePosition, b);

  Ptr<Application> app = Create<OnOffApplication> (a, InetSocketAddress ("192.168.0.2", 10), 
                                                   "Udp", 
                                                   ConstantVariable (42),
                                                   ConstantVariable (0),
                                                   DataRate (60000000),
                                                   2048);

  app->Start (Seconds (0.5));
  app->Stop (Seconds (43.0));

  Simulator::Schedule (Seconds (0.5), &Printer, b);
  Ptr<Socket> recvSink = SetupUdpReceive (b, 10);

  GlobalRouteManager::PopulateRoutingTables ();

  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}
