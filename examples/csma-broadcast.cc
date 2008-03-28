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
 */

//
// Example of the sending of a datagram to a broadcast address
//
// Network topology
//     ==============
//       |          |
//       n0    n1   n2   
//       |     |       
//     ==========
//
//   n0 originates UDP broadcast to 255.255.255.255/discard port, which 
//   is replicated and received on both n1 and n2

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/helper-module.h"
#include "ns3/simulator-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CsmaBroadcastExample");

int 
main (int argc, char *argv[])
{
  // Users may find it convenient to turn on explicit debugging
  // for selected modules; the below lines suggest how to do this
#if 0
  LogComponentEnable ("CsmaBroadcastExample", LOG_LEVEL_INFO);

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
  // Enable the below logging command to see the packets being received
  LogComponentEnable("PacketSinkApplication", LOG_LEVEL_ALL);
#endif

  // Set up some default values for the simulation.  Use the Bind()

  // Allow the user to override any of the defaults and the above
  // Bind()s at run-time, via command-line arguments
  CommandLine cmd;
  cmd.Parse (argc, argv);

  NS_LOG_INFO ("Create nodes.");
  NodeContainer c;
  c.Create (3);
  NodeContainer c0 = NodeContainer (c.Get (0), c.Get (1));
  NodeContainer c1 = NodeContainer (c.Get (0), c.Get (2));

  NS_LOG_INFO ("Build Topology.");
  CsmaHelper csma;
  // Also configure some tcpdump traces; each interface will be traced
  // The output files will be named 
  // csma-broadcast.pcap-<nodeId>-<interfaceId>
  // and can be read by the "tcpdump -tt -r" command 
  csma.EnablePcap ("csma-broadcast.pcap");
  std::ofstream ascii;
  ascii.open ("csma-broadcast.tr");
  csma.EnableAscii (ascii);
  csma.SetChannelParameter ("BitRate", DataRate(5000000));
  csma.SetChannelParameter ("Delay", MilliSeconds(2));

  NetDeviceContainer n0 = csma.Build (c0);
  NetDeviceContainer n1 = csma.Build (c1);


  InternetStackHelper internet;
  internet.Build (c0);
  internet.Build (c1);

  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.0.0", "255.255.255.0");
  ipv4.Allocate (n0);
  ipv4.SetBase ("192.168.1.0", "255.255.255.0");
  ipv4.Allocate (n1);


  // RFC 863 discard port ("9") indicates packet should be thrown away
  // by the system.  We allow this silent discard to be overridden
  // by the PacketSink application.
  uint16_t port = 9;

  // Create the OnOff application to send UDP datagrams of size
  // 512 bytes (default) at a rate of 500 Kb/s (default) from n0
  NS_LOG_INFO ("Create Applications.");
  OnOffHelper onoff;
  onoff.SetUdpRemote (Ipv4Address ("255.255.255.255"), port);
  onoff.SetAppAttribute ("OnTime", ConstantVariable (1));
  onoff.SetAppAttribute ("OffTime", ConstantVariable (0));

  ApplicationContainer app = onoff.Build (c0.Get (0));
  // Start the application
  app.Start (Seconds (1.0));
  app.Stop (Seconds (10.0));
  
  // Create an optional packet sink to receive these packets
  PacketSinkHelper sink;
  sink.SetupUdp (Ipv4Address::GetAny (), port);
  sink.Build (c0.Get (1));
  sink.Build (c1.Get (1));



  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();    
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
