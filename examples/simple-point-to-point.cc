/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * ns-2 simple.tcl script (ported from ns-2)
 * Originally authored by Steve McCanne, 12/19/1996
 */

// Port of ns-2/tcl/ex/simple.tcl to ns-3
//
// Network topology
//
//  n0
//     \ 5 Mb/s, 2ms
//      \          1.5Mb/s, 10ms
//       n2 -------------------------n3
//      /
//     / 5 Mb/s, 2ms
//   n1
//
// - all links are point-to-point links with indicated one-way BW/delay
// - CBR/UDP flows from n0 to n3, and from n3 to n1
// - FTP/TCP flow from n0 to n3, starting at time 1.2 to time 1.35 sec.
// - UDP packet size of 210 bytes, with per-packet interval 0.00375 sec.
//   (i.e., DataRate of 448,000 bps)
// - DropTail queues 
// - Tracing of queues and packet receptions to file 
//   "simple-point-to-point.tr"

#include "ns3/log.h"
#include "ns3/command-line.h"
#include "ns3/default-value.h"
#include "ns3/ptr.h"
#include "ns3/random-variable.h"

#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"

#include "ns3/ascii-trace.h"
#include "ns3/pcap-trace.h"
#include "ns3/internet-node.h"
#include "ns3/point-to-point-channel.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/ipv4-address.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4.h"
#include "ns3/socket.h"
#include "ns3/ipv4-route.h"
#include "ns3/point-to-point-topology.h"
#include "ns3/onoff-application.h"
#include "ns3/packet-sink.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SimplePointToPointExample");

int 
main (int argc, char *argv[])
{
  // Users may find it convenient to turn on explicit debugging
  // for selected modules; the below lines suggest how to do this
#if 0 
  LogComponentEnable ("SimplePointToPointExample", LOG_LEVEL_INFO);

  LogComponentEnable("Object", LOG_LEVEL_ALL);
  LogComponentEnable("Queue", LOG_LEVEL_ALL);
  LogComponentEnable("DropTailQueue", LOG_LEVEL_ALL);
  LogComponentEnable("Channel", LOG_LEVEL_ALL);
  LogComponentEnable("CsmaChannel", LOG_LEVEL_ALL);
  LogComponentEnable("NetDevice", LOG_LEVEL_ALL);
  LogComponentEnable("CsmaNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable("Ipv4L3Protocol", LOG_LEVEL_ALL);
  LogComponentEnable("PacketSocket", LOG_LEVEL_ALL);
  LogComponentEnable("Socket", LOG_LEVEL_ALL);
  LogComponentEnable("UdpSocket", LOG_LEVEL_ALL);
  LogComponentEnable("UdpL4Protocol", LOG_LEVEL_ALL);
  LogComponentEnable("Ipv4L3Protocol", LOG_LEVEL_ALL);
  LogComponentEnable("Ipv4StaticRouting", LOG_LEVEL_ALL);
  LogComponentEnable("Ipv4Interface", LOG_LEVEL_ALL);
  LogComponentEnable("ArpIpv4Interface", LOG_LEVEL_ALL);
  LogComponentEnable("Ipv4LoopbackInterface", LOG_LEVEL_ALL);
  LogComponentEnable("OnOffApplication", LOG_LEVEL_ALL);
  LogComponentEnable("PacketSinkApplication", LOG_LEVEL_ALL);
  LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_ALL);
  LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_ALL);
#endif

  // Set up some default values for the simulation.  Use the Bind()
  // technique to tell the system what subclass of Queue to use,
  // and what the queue limit is

  // The below Bind command tells the queue factory which class to
  // instantiate, when the queue factory is invoked in the topology code
  DefaultValue::Bind ("Queue", "DropTailQueue");

  DefaultValue::Bind ("OnOffApplicationPacketSize", "210");
  DefaultValue::Bind ("OnOffApplicationDataRate", "448kb/s");

  //DefaultValue::Bind ("DropTailQueue::m_maxPackets", 30);   

  // Allow the user to override any of the defaults and the above
  // Bind()s at run-time, via command-line arguments
  CommandLine::Parse (argc, argv);

  // Here, we will explicitly create four nodes.  In more sophisticated
  // topologies, we could configure a node factory.
  NS_LOG_INFO ("Create nodes.");
  Ptr<Node> n0 = Create<InternetNode> ();
  Ptr<Node> n1 = Create<InternetNode> (); 
  Ptr<Node> n2 = Create<InternetNode> (); 
  Ptr<Node> n3 = Create<InternetNode> ();

  // We create the channels first without any IP addressing information
  NS_LOG_INFO ("Create channels.");
  Ptr<PointToPointChannel> channel0 = 
    PointToPointTopology::AddPointToPointLink (
      n0, n2, DataRate(5000000), MilliSeconds(2));

  Ptr<PointToPointChannel> channel1 = 
    PointToPointTopology::AddPointToPointLink (
      n1, n2, DataRate(5000000), MilliSeconds(2));
  
  Ptr<PointToPointChannel> channel2 = 
    PointToPointTopology::AddPointToPointLink (
      n2, n3, DataRate(1500000), MilliSeconds(10));
  
  // Later, we add IP addresses.  
  NS_LOG_INFO ("Assign IP Addresses.");
  PointToPointTopology::AddIpv4Addresses (
      channel0, n0, Ipv4Address("10.1.1.1"),
      n2, Ipv4Address("10.1.1.2"));
  
  PointToPointTopology::AddIpv4Addresses (
      channel1, n1, Ipv4Address("10.1.2.1"),
      n2, Ipv4Address("10.1.2.2"));
  
  PointToPointTopology::AddIpv4Addresses (
      channel2, n2, Ipv4Address("10.1.3.1"),
      n3, Ipv4Address("10.1.3.2"));

  // Finally, we add static routes.  These three steps (Channel and
  // NetDevice creation, IP Address assignment, and routing) are 
  // separated because there may be a need to postpone IP Address
  // assignment (emulation) or modify to use dynamic routing
  NS_LOG_INFO ("Add Static Routes.");
  PointToPointTopology::AddIpv4Routes(n0, n2, channel0);
  PointToPointTopology::AddIpv4Routes(n1, n2, channel1);
  PointToPointTopology::AddIpv4Routes(n2, n3, channel2);

  // Create the OnOff application to send UDP datagrams of size
  // 210 bytes at a rate of 448 Kb/s
  NS_LOG_INFO ("Create Applications.");
  uint16_t port = 9;   // Discard port (RFC 863)
  Ptr<OnOffApplication> ooff = Create<OnOffApplication> (
    n0, 
    InetSocketAddress ("10.1.3.2", port), 
    "Udp",
    ConstantVariable(1), 
    ConstantVariable(0));
  // Start the application
  ooff->Start(Seconds(1.0));
  ooff->Stop (Seconds(10.0));

  // Create an optional packet sink to receive these packets
  Ptr<PacketSink> sink = Create<PacketSink> (
    n3,
    InetSocketAddress (Ipv4Address::GetAny (), port),
    "Udp");
  // Start the sink
  sink->Start (Seconds (1.0));
  sink->Stop (Seconds (10.0));

  // Create a similar flow from n3 to n1, starting at time 1.1 seconds
  ooff = Create<OnOffApplication> (
    n3, 
    InetSocketAddress ("10.1.2.1", port), 
    "Udp",
    ConstantVariable(1), 
    ConstantVariable(0));
  // Start the application
  ooff->Start(Seconds(1.1));
  ooff->Stop (Seconds(10.0));

  // Create a packet sink to receive these packets
  sink = Create<PacketSink> (
    n1,
    InetSocketAddress (Ipv4Address::GetAny (), port),
    "Udp");
  // Start the sink
  sink->Start (Seconds (1.1));
  sink->Stop (Seconds (10.0));

  // Here, finish off packet routing configuration
  // This will likely set by some global StaticRouting object in the future
  NS_LOG_INFO ("Set Default Routes.");
  Ptr<Ipv4> ipv4;
  ipv4 = n0->QueryInterface<Ipv4> (Ipv4::iid);
  ipv4->SetDefaultRoute (Ipv4Address ("10.1.1.2"), 1);
  ipv4 = n3->QueryInterface<Ipv4> (Ipv4::iid);
  ipv4->SetDefaultRoute (Ipv4Address ("10.1.3.1"), 1);
  
  // Configure tracing of all enqueue, dequeue, and NetDevice receive events
  // Trace output will be sent to the simple-point-to-point.tr file
  NS_LOG_INFO ("Configure Tracing.");
  AsciiTrace asciitrace ("simple-point-to-point.tr");
  asciitrace.TraceAllQueues ();
  asciitrace.TraceAllNetDeviceRx ();

  // Also configure some tcpdump traces; each interface will be traced
  // The output files will be named 
  // simple-point-to-point.pcap-<nodeId>-<interfaceId>
  // and can be read by the "tcpdump -r" command (use "-tt" option to
  // display timestamps correctly)
  PcapTrace pcaptrace ("simple-point-to-point.pcap");
  pcaptrace.TraceAllIp ();

  NS_LOG_INFO ("Run Simulation.");
  Simulator::StopAt (Seconds (10));
  Simulator::Run ();    
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
