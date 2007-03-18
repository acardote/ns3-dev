/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 University of Washington
 * All rights reserved.
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
 */

#if 0
#include <list>
#include <cassert>
#endif

#include "ns3/debug.h"
#include "ns3/internet-node.h"
#include "ns3/mac-address.h"
#include "ns3/packet.h"
#include "ns3/arp-ipv4-interface.h"
#include "ns3/ipv4-address.h"
#include "ns3/serial-channel.h"
#include "ns3/serial-net-device.h"
#include "ns3/trace-writer.h"
#include "ns3/drop-tail.h"
#include "ns3/arp-ipv4-interface.h"
#include "ns3/ipv4.h"
#include "ns3/trace-context.h"
#include "ns3/datagram-socket.h"
#include "ns3/simulator.h"
#include "ns3/node-list.h"
#include "ns3/trace-root.h"


using namespace ns3;

class Logger : public TraceWriter{
public:
  Logger ()
  {
    NS_DEBUG_UNCOND("**** Logger()");
  }

  Logger (std::string const &filename) 
  {
    NS_DEBUG_UNCOND("**** Logger(string const &)");
    Open(filename);
  }

  Logger (char const *filename) : m_tracer(filename)
  {
    NS_DEBUG_UNCOND("**** Logger(char const *)");
    Open(filename);
  }

  ~Logger () {}

  void Log (TraceContext const &context, const Packet &p)
  {
    NodeList::NodeIndex nodeIndex;
    context.Get (nodeIndex);
    m_filestr << "node=" << NodeList::GetNode (nodeIndex)->GetId () << " ";
    Ipv4::InterfaceIndex interfaceIndex;
    context.Get (interfaceIndex);
    m_filestr << "interface=" << interfaceIndex << " ";
    enum Queue::TraceType type;
    context.Get (type);
    switch (type) 
      {
      case Queue::ENQUEUE:
        m_filestr << "enqueue";
        break;
      case Queue::DEQUEUE:
        m_filestr << "dequeue";
        break;
      case Queue::DROP:
        m_filestr << "drop";
        break;
      }
    m_filestr << " bytes=" << p.GetSize () << std::endl;
  }

protected:
  TraceWriter m_tracer;
};

static void
GenerateTraffic (DatagramSocket *socket, uint32_t size)
{
  std::cout << "Node: " << socket->GetNode()->GetId () 
            << " at=" << Simulator::Now ().GetSeconds () << "s,"
            << " tx bytes=" << size << std::endl;
  socket->SendDummy (size);
  if (size > 50)
    {
      Simulator::Schedule (Seconds (0.5), &GenerateTraffic, socket, size - 50);
    }
}

static void
DatagramSocketPrinter (DatagramSocket *socket, uint32_t size, Ipv4Address from, uint16_t fromPort)
{
  std::cout << "Node: " << socket->GetNode()->GetId () 
            << " at=" << Simulator::Now ().GetSeconds () << "s,"
            << " rx bytes=" << size << std::endl;
}

static void
PrintTraffic (DatagramSocket *socket)
{
  socket->SetDummyRxCallback (MakeCallback (&DatagramSocketPrinter));
}


int main (int argc, char *argv[])
{
  NS_DEBUG_UNCOND("Serial Net Device Test");

  // create two nodes and a simple SerialChannel
  InternetNode a;
  InternetNode b;
  SerialChannel ch = SerialChannel ("Test Channel", 1000, Seconds (0.1));

  NodeList::Add (&a);
  NodeList::Add (&b);

  // create two NetDevices and assign one to each node
  // Note:  this would normally be done also in conjunction with
  //        creating a Channel
  //        Here, we do not care about the Device Address (point-to-point)
  //        but more generally, we would use a subclass such as MacAddress
  //        as follows:    MacAddress addra("00:00:00:00:00:01");
  //        so we'll pretend and give them simple MacAddresses here
   
  MacAddress addra("00:00:00:00:00:01");
  SerialNetDevice neta(&a, addra);

  DropTailQueue dtqa;

  neta.AddQueue(&dtqa);
  neta.SetName("a.eth0"); 

  MacAddress addrb("00:00:00:00:00:02");
  SerialNetDevice netb(&b, addrb);

  DropTailQueue dtqb;

  netb.AddQueue(&dtqb);
  netb.SetName("b.eth0"); 

  // bind the two NetDevices together by using a simple Channel
  // this method changed to do a bidirectional binding
  neta.Attach (&ch);
  netb.Attach (&ch);

  // Some simple prints to see whether it is working
  NS_DEBUG_UNCOND("neta.GetMtu() <= " << neta.GetMtu());
  NS_DEBUG_UNCOND("netb.GetMtu() <= " << netb.GetMtu());
  NS_DEBUG_UNCOND("neta.GetAddress() <= " << neta.GetAddress());
  NS_DEBUG_UNCOND("netb.GetAddress() <= " << netb.GetAddress());

  // Note:  InternetNode constructor instantiates multiple Layer-3
  // protocols and registers them with the L3Demux object.
  // This takes care of Layer-2 -> Layer-3 bindings.
  //  XXX TODO:  will need to create a dummy IPv4 object for insertion
  //             into the Demux

  // We now need to bind the InternetNode to the various interfaces.
  // to get the Layer-3 -> Layer-2 bindings.

  // We do this by calling an "AddArpIpv4Interface(neta)" function on node a.
  // This should:  
  // i) create an Ipv4ArpInterface object (subclass of pure virtual
  //    Ipv4Interface object)
  // ii) add the Ipv4ArpInterface object to the InternetNode's internal
  //     vector of Ipv4Interfaces (keyed off of ifIndex)

  NS_DEBUG_UNCOND("Adding ARP Interface to InternetNode a");
  ArpIpv4Interface* arpipv4interfacep = new ArpIpv4Interface(&a, &neta);
  uint32_t indexA = (&a)->GetIpv4 ()->AddInterface (arpipv4interfacep);
  NS_DEBUG_UNCOND("Adding Interface " << indexA);


  // iii) give the interface an IP address

  NS_DEBUG_UNCOND("Giving IP address to ARP Interface");
  arpipv4interfacep->SetAddress(Ipv4Address("10.1.1.1"));
  arpipv4interfacep->SetNetworkMask(Ipv4Mask("255.255.255.0"));

  // iv) set the interface's state to "UP"

  NS_DEBUG_UNCOND("Setting ARP interface to UP");
  arpipv4interfacep->SetUp();

  a.GetIpv4()->SetDefaultRoute (Ipv4Address ("10.1.1.2"), 1);


  NS_DEBUG_UNCOND("Adding ARP Interface to InternetNode b");
  ArpIpv4Interface* arpipv4interfacepb = new ArpIpv4Interface(&b, &netb);
  uint32_t indexB = (&b)->GetIpv4 ()->AddInterface (arpipv4interfacepb);
  NS_DEBUG_UNCOND("Adding Interface " << indexB);


  // iii) give the interface an IP address

  NS_DEBUG_UNCOND("Giving IP address to ARP Interface");
  arpipv4interfacepb->SetAddress(Ipv4Address("10.1.1.2"));
  arpipv4interfacepb->SetNetworkMask(Ipv4Mask("255.255.255.0"));

  // iv) set the interface's state to "UP"

  NS_DEBUG_UNCOND("Setting ARP interface to UP");
  arpipv4interfacepb->SetUp();

  b.GetIpv4()->SetDefaultRoute (Ipv4Address ("10.1.1.1"), 1);


  DatagramSocket *source = new DatagramSocket (&a);
  DatagramSocket *sink = new DatagramSocket(&b);
  sink->Bind (80);
  source->SetDefaultDestination (Ipv4Address ("10.1.1.2"), 80);

  Logger logger("serial-net-test.log");

  TraceRoot::Connect ("/nodes/*/ipv4/interfaces/*/netdevice/queue/*", 
                      MakeCallback (&Logger::Log, &logger));

  PrintTraffic (sink);
  GenerateTraffic (source, 100);

  Simulator::Run ();

  Simulator::Destroy ();

  delete source;
  delete sink;

  return 0;
}
