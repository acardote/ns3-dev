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
// - all links are serial links with indicated one-way BW/delay
// - CBR/UDP flows from n0 to n3, and from n3 to n1
// - FTP/TCP flow from n0 to n3, starting at time 1.2 to time 1.35 sec.
// - UDP packet size of 210 bytes, with per-packet interval 0.00375 sec.
// - DropTail queues 
// - Tracing of queues and packet receptions to file out.tr

#include <iostream>
#include <string>
#include <cassert>

#include "ns3/debug.h"

#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/trace-writer.h"

#include "ns3/internet-node.h"
#include "ns3/serial-channel.h"
#include "ns3/mac-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/arp-ipv4-interface.h"
#include "ns3/ipv4.h"
#include "ns3/udp-socket.h"
#include "ns3/ipv4-route.h"
#include "ns3/drop-tail.h"
#include "ns3/trace-writer.h"
#include "ns3/llc-snap-header.h"
#include "ns3/arp-header.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"

using namespace ns3;

class Tracer : public TraceWriter{
public:
  Tracer ()
  {
  };

  Tracer (std::string const &filename) 
  {
    Open(filename);
  };

  Tracer (char const *filename) : m_tracer(filename)
  {
    Open(filename);
  };

  ~Tracer () {};

  void LogEnqueue (std::string const &name, const Packet &p)
  {
    m_filestr << name << " que ";
    PrintLlcPacket (p, m_filestr);
    m_filestr << std::endl;
  }

  void LogDequeue (std::string const &name, const Packet &p)
  {
    m_filestr << name << " deq ";
    PrintLlcPacket (p, m_filestr);
    m_filestr << std::endl;
  }
  void LogDrop (std::string const &name, const Packet &p)
  {
    m_filestr << name << " dro ";
    PrintLlcPacket (p, m_filestr);
    m_filestr << std::endl;
  }

  void PrintLlcPacket (Packet p, std::ostream &os)
  {
    LlcSnapHeader llc;
    p.Peek (llc);
    p.Remove (llc);
    switch (llc.GetType ())
      {
      case 0x0800: {
        Ipv4Header ipv4;
        p.Peek (ipv4);
        p.Remove (ipv4);
        if (ipv4.GetProtocol () == 17)
          {
            UdpHeader udp;
            p.Peek (udp);
            p.Remove (udp);
            os << "udp payload=" << p.GetSize () 
               << " from="<< ipv4.GetSource () << ":" << udp.GetSource ()
               << " to="<< ipv4.GetDestination () << ":" << udp.GetDestination ();
          }
      } break;
      case 0x0806: {
        ArpHeader arp;
        p.Peek (arp);
        p.Remove (arp);
        os << "arp ";
        if (arp.IsRequest ())
          {
            os << "request from=" << arp.GetSourceIpv4Address ()
               << ", for=" << arp.GetDestinationIpv4Address ();
          }
        else
          {
            os << "reply from=" << arp.GetSourceIpv4Address ()
               << ", for=" << arp.GetDestinationIpv4Address ();
          }
      } break;
      }
  }

protected:
  TraceWriter m_tracer;
};


static void
GenerateTraffic (UdpSocket *socket, uint32_t size)
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
UdpSocketPrinter (UdpSocket *socket, uint32_t size, Ipv4Address from, uint16_t fromPort)
{
  std::cout << "Node: " << socket->GetNode()->GetId () 
            << " at=" << Simulator::Now ().GetSeconds () << "s,"
            << " rx bytes=" << size << std::endl;
}

static void
PrintTraffic (UdpSocket *socket)
{
  socket->SetDummyRxCallback (MakeCallback (&UdpSocketPrinter));
}

#if 0
static void
PrintRoutingTable (InternetNode *a, std::string name)
{
  Ipv4 *ipv4 = a->GetIpv4 ();
  std::cout << "interfaces node="<<name<<std::endl;
  for (uint32_t i = 0; i < ipv4->GetNInterfaces (); i++)
    {
      Ipv4Interface *interface = ipv4->GetInterface (i);
      std::cout << "interface addr="<<interface->GetAddress () 
                << ", netmask="<<interface->GetNetworkMask ()
                << std::endl;
    }

  std::cout << "routing table:" << std::endl;
  for (uint32_t i = 0; i < ipv4->GetNRoutes (); i++)
    {
      Ipv4Route *route = ipv4->GetRoute (i);
      std::cout << (*route) << std::endl;
    }
  std::cout << "node end" << std::endl;
}
#endif

static SerialChannel * 
AddDuplexLink(
  InternetNode* a, 
  const Ipv4Address& addra,
  const MacAddress& macaddra, 
  InternetNode* b, 
  const Ipv4Address& addrb,
  const MacAddress& macaddrb, 
  // const Rate& rate, 
  // const Time& delay,
  TraceContainer &traceContainer,
  std::string &name) 
{
  std::string qName;
  SerialChannel* channel = new SerialChannel();

  // Duplex link is assumed to be subnetted as a /30
  // May run this unnumbered in the future?
  Ipv4Mask netmask("255.255.255.252");
  assert(netmask.IsMatch(addra,addrb));

  qName = name + "::Queue A";
  DropTailQueue* dtqa = new DropTailQueue(qName);
  dtqa->RegisterTraces (traceContainer);

  SerialNetDevice* neta = new SerialNetDevice(a, macaddra);
  neta->AddQueue(dtqa);
  Ipv4Interface *interfA = new ArpIpv4Interface (a, neta);
  uint32_t indexA = a->GetIpv4 ()->AddInterface (interfA);
  channel->Attach (neta);
  neta->Attach (channel);
  
  interfA->SetAddress (addra);
  interfA->SetNetworkMask (netmask);
  interfA->SetUp ();

  qName = name + "::Queue B";
  DropTailQueue* dtqb = new DropTailQueue(qName);
  dtqb->RegisterTraces (traceContainer);

  SerialNetDevice* netb = new SerialNetDevice(b, macaddrb);
  netb->AddQueue(dtqb);
  Ipv4Interface *interfB = new ArpIpv4Interface (b, netb);
  uint32_t indexB = b->GetIpv4 ()->AddInterface (interfB);
  channel->Attach (netb);
  netb->Attach (channel);

  interfB->SetAddress (addrb);
  interfB->SetNetworkMask (netmask);
  interfB->SetUp ();

  a->GetIpv4 ()->AddHostRouteTo (addrb, indexA);
  b->GetIpv4 ()->AddHostRouteTo (addra, indexB);

  NS_DEBUG_UNCOND("Adding interface " << indexA << " to node " << a->GetId());
  NS_DEBUG_UNCOND("Adding interface " << indexB << " to node " << b->GetId());

  //PrintRoutingTable (a, "a");
  //PrintRoutingTable (b, "b");

  return channel;
}

static void
SetupTrace (TraceContainer &container, Tracer &tracer)
{
  container.SetCallback ("Queue::Enqueue",
                         MakeCallback (&Tracer::LogEnqueue, &tracer));
  
  container.SetCallback ("Queue::Dequeue",
                         MakeCallback (&Tracer::LogDequeue, &tracer));
  
  container.SetCallback ("Queue::Drop",
                         MakeCallback (&Tracer::LogDrop, &tracer));

}

int main (int argc, char *argv[])
{
  // ** Here, some kind of factory or topology object will instantiates 
  // ** four identical nodes; for now, we just explicitly create them
  InternetNode *n0 = new InternetNode();
  InternetNode *n1 = new InternetNode();
  InternetNode *n2 = new InternetNode();
  InternetNode *n3 = new InternetNode();

  TraceContainer traceContainer;

  n0->SetName(std::string("Node 0"));
  n1->SetName(std::string("Node 1"));
  n2->SetName(std::string("Node 2"));
  n3->SetName(std::string("Node 3"));
  
  Tracer tracer("serial-net-test.log");
    
  std::string channelName;
    
  channelName = "Channel 1";
  SerialChannel* ch1 = AddDuplexLink (
      n0, Ipv4Address("10.1.1.1"), MacAddress("00:00:00:00:00:01"), 
      n2, Ipv4Address("10.1.1.2"), MacAddress("00:00:00:00:00:02"), 
      traceContainer, channelName);
  SetupTrace (traceContainer, tracer);

  channelName = "Channel 2";
  SerialChannel* ch2 = AddDuplexLink (
      n1, Ipv4Address("10.1.2.1"), MacAddress("00:00:00:00:00:03"), 
      n2, Ipv4Address("10.1.2.2"), MacAddress("00:00:00:00:00:04"), 
      traceContainer, channelName);
  SetupTrace (traceContainer, tracer);

  channelName = "Channel 3";
  SerialChannel* ch3 = AddDuplexLink (
      n2, Ipv4Address("10.1.3.1"), MacAddress("00:00:00:00:00:05"), 
      n3, Ipv4Address("10.1.3.2"), MacAddress("00:00:00:00:00:06"), 
      traceContainer, channelName);
  SetupTrace (traceContainer, tracer);
  
  UdpSocket *source0 = new UdpSocket (n0);
  UdpSocket *source3 = new UdpSocket (n3);
  UdpSocket *sink3 = new UdpSocket(n3);
  sink3->Bind (80);
  UdpSocket *sink1 = new UdpSocket(n1);
  sink1->Bind (80);

  source3->SetDefaultDestination (Ipv4Address ("10.1.2.1"), 80);
  source0->SetDefaultDestination (Ipv4Address ("10.1.3.2"), 80);

  // Here, finish off packet routing configuration
  n0->GetIpv4()->SetDefaultRoute (Ipv4Address ("10.1.1.2"), 1);
  n3->GetIpv4()->SetDefaultRoute (Ipv4Address ("10.1.3.1"), 1);

  PrintTraffic (sink3);
  GenerateTraffic (source0, 100);

  PrintTraffic (sink1);
  GenerateTraffic (source3, 100);

  Simulator::StopAt (Seconds(3.0));

  Simulator::Run ();
    
  // The below deletes will be managed by future topology objects
  // or containers or smart pointers
  delete n0;
  delete n1;
  delete n2;
  delete n3;
  delete ch1;
  delete ch2;
  delete ch3;
  delete source3;
  delete source0;
  delete sink3;
  delete sink1;

  Simulator::Destroy ();
}
