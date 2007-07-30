#include <iostream>

#include "ns3/internet-node.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/inet-socket-address.h"
#include "ns3/nstime.h"

using namespace ns3;

static void
GenerateTraffic (Ptr<Socket> socket, uint32_t size)
{
  std::cout << "at=" << Simulator::Now ().GetSeconds () << "s, tx bytes=" << size << std::endl;
  socket->Send (0, size);
  if (size > 0)
    {
      Simulator::Schedule (Seconds (0.5), &GenerateTraffic, socket, size - 50);
    }
  else
    {
      socket->Close ();
    }
}

static void
SocketPrinter (Ptr<Socket> socket, uint32_t size, const Address &from)
{
  std::cout << "at=" << Simulator::Now ().GetSeconds () << "s, rx bytes=" << size << std::endl;
}

static void
PrintTraffic (Ptr<Socket> socket)
{
  socket->RecvDummy (MakeCallback (&SocketPrinter));
}

void
RunSimulation (void)
{
  Ptr<Node> a = Create<InternetNode> ();

  InterfaceId iid = InterfaceId::LookupByName ("Udp");
  Ptr<SocketFactory> socketFactory = a->QueryInterface<SocketFactory> (iid);

  Ptr<Socket> sink = socketFactory->CreateSocket ();
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  sink->Bind (local);

  Ptr<Socket> source = socketFactory->CreateSocket ();
  InetSocketAddress remote = InetSocketAddress (Ipv4Address::GetLoopback (), 80);
  source->Connect (remote);

  GenerateTraffic (source, 500);
  PrintTraffic (sink);


  Simulator::Run ();

  Simulator::Destroy ();
}

int main (int argc, char *argv[])
{
  RunSimulation ();

  return 0;
}
