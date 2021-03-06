/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 Strasbourg University
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
 * Author: David Gross <gdavid.devel@gmail.com>
 */

// Network topology
// //
// //
// //                               +------------+
// //  +------------+           |---|  Router 1  |---|
// //  |   Host 0   |--|        |   [------------]   |
// //  [------------]  |        |                    |
// //                  |  +------------+             |
// //                  +--|            |       +------------+
// //                     |  Router 0  |       |  Router 2  |
// //                  +--|            |       [------------]
// //                  |  [------------]             |
// //  +------------+  |        |                    |
// //  |   Host 1   |--|        |   +------------+   |
// //  [------------]           |---|  Router 3  |---|
// //                               [------------]   
// //                                     
// //
// // - Tracing of queues and packet receptions to file "loose-routing-ipv6.tr"

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/simulator-module.h"
#include "ns3/helper-module.h"
#include "ns3/ipv6-header.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LooseRoutingIpv6Example");

int main (int argc, char **argv)
{
#if 0 
  LogComponentEnable("Ipv6ExtensionLooseRouting", LOG_LEVEL_ALL);
  LogComponentEnable("Ipv6Extension", LOG_LEVEL_ALL);
  LogComponentEnable("Ipv6L3Protocol", LOG_LEVEL_ALL);
  LogComponentEnable("Ipv6StaticRouting", LOG_LEVEL_ALL);
  LogComponentEnable("Ipv6Interface", LOG_LEVEL_ALL);
  LogComponentEnable("Ipv6Interface", LOG_LEVEL_ALL);
  LogComponentEnable("NdiscCache", LOG_LEVEL_ALL);
#endif

  CommandLine cmd;
  cmd.Parse (argc, argv);

  NS_LOG_INFO ("Create nodes.");
  Ptr<Node> h0 = CreateObject<Node> ();
  Ptr<Node> h1 = CreateObject<Node> ();
  Ptr<Node> r0 = CreateObject<Node> ();
  Ptr<Node> r1 = CreateObject<Node> ();
  Ptr<Node> r2 = CreateObject<Node> ();
  Ptr<Node> r3 = CreateObject<Node> ();

  NodeContainer net1 (h0, r0);
  NodeContainer net2 (h1, r0);
  NodeContainer net3 (r0, r1);
  NodeContainer net4 (r1, r2);
  NodeContainer net5 (r2, r3);
  NodeContainer net6 (r3, r0);
  NodeContainer all;
  all.Add (h0);
  all.Add (h1);
  all.Add (r0);
  all.Add (r1);
  all.Add (r2);
  all.Add (r3);

  NS_LOG_INFO ("Create IPv6 Internet Stack");
  InternetStackHelper internetv6;
  internetv6.Install (all);

  NS_LOG_INFO ("Create channels.");
  CsmaHelper csma;
  csma.SetDeviceAttribute ("Mtu", UintegerValue(1500));
  csma.SetChannelAttribute ("DataRate", DataRateValue(5000000));
  csma.SetChannelAttribute ("Delay", TimeValue(MilliSeconds (2)));
  NetDeviceContainer d1 = csma.Install (net1);
  NetDeviceContainer d2 = csma.Install (net2);
  NetDeviceContainer d3 = csma.Install (net3);
  NetDeviceContainer d4 = csma.Install (net4);
  NetDeviceContainer d5 = csma.Install (net5);
  NetDeviceContainer d6 = csma.Install (net6);

  NS_LOG_INFO ("Create networks and assign IPv6 Addresses.");
  Ipv6AddressHelper ipv6;

  ipv6.NewNetwork (Ipv6Address ("2001:1::"), 64);
  Ipv6InterfaceContainer i1 = ipv6.Assign (d1);
  i1.SetRouter (1, true);

  ipv6.NewNetwork (Ipv6Address ("2001:2::"), 64);
  Ipv6InterfaceContainer i2 = ipv6.Assign (d2);
  i2.SetRouter (1, true);

  ipv6.NewNetwork (Ipv6Address ("2001:3::"), 64);
  Ipv6InterfaceContainer i3 = ipv6.Assign (d3);
  i3.SetRouter (0, true);
  i3.SetRouter (1, true);

  ipv6.NewNetwork (Ipv6Address ("2001:4::"), 64);
  Ipv6InterfaceContainer i4 = ipv6.Assign (d4);
  i4.SetRouter (0, true);
  i4.SetRouter (1, true);

  ipv6.NewNetwork (Ipv6Address ("2001:5::"), 64);
  Ipv6InterfaceContainer i5 = ipv6.Assign (d5);
  i5.SetRouter (0, true);
  i5.SetRouter (1, true);

  ipv6.NewNetwork (Ipv6Address ("2001:6::"), 64);
  Ipv6InterfaceContainer i6 = ipv6.Assign (d6);
  i6.SetRouter (0, true);
  i6.SetRouter (1, true);

  NS_LOG_INFO ("Create Applications.");

  /**
   * ICMPv6 Echo from h0 to h1 port 7
   */
  uint32_t packetSize = 1024;
  uint32_t maxPacketCount = 1;
  Time interPacketInterval = Seconds (1.0);

  std::vector<Ipv6Address> routersAddress;
  routersAddress.push_back (i3.GetAddress (1, 1));
  routersAddress.push_back (i4.GetAddress (1, 1));
  routersAddress.push_back (i5.GetAddress (1, 1));
  routersAddress.push_back (i6.GetAddress (1, 1));
  routersAddress.push_back (i2.GetAddress (0, 1));

  Ping6Helper client;
  /* remote address is first routers in RH0 => source routing */
  client.SetRemote (i1.GetAddress (1, 1));
  client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client.SetAttribute ("Interval", TimeValue(interPacketInterval));
  client.SetAttribute ("PacketSize", UintegerValue (packetSize));
  client.SetRoutersAddress (routersAddress);
  ApplicationContainer apps = client.Install (h0);
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll (ascii.CreateFileStream ("loose-routing-ipv6.tr"));
  csma.EnablePcapAll ("loose-routing-ipv6", true);

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}

