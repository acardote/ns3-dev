/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2007,2008, 2009 INRIA, UDcast
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
 * Author: Mohamed Amine Ismail <amine.ismail@sophia.inria.fr>
 *                              <amine.ismail@udcast.com>
 */

#include <fstream>
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/inet-socket-address.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/csma-helper.h"
#include "ns3/test.h"
#include "ns3/simulator.h"

using namespace ns3;

/**
 * Test that all the udp packets generated by an udpClient application are
 * correctly received by an udpServer application
 */

class UdpClientServerTestCase: public TestCase
{
public:
  UdpClientServerTestCase ();
  virtual ~UdpClientServerTestCase ();

private:
  virtual bool DoRun (void);

};

UdpClientServerTestCase::UdpClientServerTestCase () :
  TestCase ("Test that all the udp packets generated by an udpClient application are correctly received by an udpServer application")
{
}

UdpClientServerTestCase::~UdpClientServerTestCase ()
{
}

bool UdpClientServerTestCase::DoRun (void)
{
    NodeContainer n;
    n.Create (2);

    InternetStackHelper internet;
    internet.Install (n);

    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate(5000000)));
    csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
    csma.SetDeviceAttribute ("Mtu", UintegerValue (1400));
    NetDeviceContainer d = csma.Install (n);

    Ipv4AddressHelper ipv4;

    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i = ipv4.Assign (d);

    uint16_t port = 4000;
    UdpServerHelper server (port);
    ApplicationContainer apps = server.Install (n.Get(1));
    apps.Start (Seconds (1.0));
    apps.Stop (Seconds (10.0));

    uint32_t MaxPacketSize = 1024;
    Time interPacketInterval = Seconds (1.);
    uint32_t maxPacketCount = 10;
    UdpClientHelper client (i.GetAddress (1), port);
    client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
    client.SetAttribute ("Interval", TimeValue (interPacketInterval));
    client.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
    apps = client.Install (n.Get (0));
    apps.Start (Seconds (2.0));
    apps.Stop (Seconds (10.0));

    Simulator::Run ();
    Simulator::Destroy ();

    if ((server.GetServer ()->GetLost () != 0)
        || (server.GetServer ()->GetReceived () != 8))
      {
        return true; // there was an error
      }
    return false;

}

/**
 * Test that all the udp packets generated by an udpTraceClient application are
 * correctly received by an udpServer application
 */

class UdpTraceClientServerTestCase: public TestCase
{
public:
  UdpTraceClientServerTestCase ();
  virtual ~UdpTraceClientServerTestCase ();

private:
  virtual bool DoRun (void);

};

UdpTraceClientServerTestCase::UdpTraceClientServerTestCase () :
  TestCase ("Test that all the udp packets generated by an udpTraceClient application are correctly received by an udpServer application")
{
}

UdpTraceClientServerTestCase::~UdpTraceClientServerTestCase ()
{
}

bool UdpTraceClientServerTestCase::DoRun (void)
{


  NodeContainer n;
  n.Create (2);

  InternetStackHelper internet;
  internet.Install (n);


  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate(5000000)));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  csma.SetDeviceAttribute ("Mtu", UintegerValue (1400));
  NetDeviceContainer d = csma.Install (n);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (d);

  uint16_t port = 4000;
  UdpServerHelper server (port);
  ApplicationContainer apps = server.Install (n.Get(1));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  uint32_t MaxPacketSize = 1400;
  UdpTraceClientHelper client (i.GetAddress (1), port,"");
  client.SetAttribute ("MaxPacketSize", UintegerValue (MaxPacketSize));
  apps = client.Install (n.Get (0));
  apps.Start (Seconds (2.0));
  apps.Stop (Seconds (10.0));

  Simulator::Run ();
  Simulator::Destroy ();

  if ((server.GetServer ()->GetLost () != 0)||
      (server.GetServer ()->GetReceived () != 247))
    {
      return true; // there was an error
    }

  return false;
}


/**
 * Test that all the PacketLossCounter class checks loss correctly in different cases
 */

class PacketLossCounterTestCase: public TestCase
{
public:
  PacketLossCounterTestCase ();
  virtual ~PacketLossCounterTestCase ();

private:
  virtual bool DoRun (void);

};

PacketLossCounterTestCase::PacketLossCounterTestCase () :
  TestCase ("Test that all the PacketLossCounter class checks loss correctly in different cases")
{
}

PacketLossCounterTestCase::~PacketLossCounterTestCase ()
{
}

bool PacketLossCounterTestCase::DoRun (void)
{
  PacketLossCounter lossCounter(32);
  for (uint32_t i=0;i<64;i++)
    {
      lossCounter.NotifyReceived(i);
    }

  if (lossCounter.GetLost()!=0) // Check that 0 packets are lost
    {
      return true;
    }
  for (uint32_t i=65;i<128;i++) // drop (1) seqNum 64
    {
      lossCounter.NotifyReceived(i);
    }
  if (lossCounter.GetLost()!=1) //chek that 1 packet is lost
    {
      return true;
    }
  for (uint32_t i=134;i<200;i++) // drop seqNum 128,129,130,131,132,133
    {
      lossCounter.NotifyReceived(i);
    }
  if (lossCounter.GetLost()!=7) //chek that 7 (6+1) packet are lost
    {
      return true;
    }
  // reordering without loss
  lossCounter.NotifyReceived(205);
  lossCounter.NotifyReceived(206);
  lossCounter.NotifyReceived(207);
  lossCounter.NotifyReceived(200);
  lossCounter.NotifyReceived(201);
  lossCounter.NotifyReceived(202);
  lossCounter.NotifyReceived(203);
  lossCounter.NotifyReceived(204);
  for (uint32_t i=205;i<250;i++)
    {
      lossCounter.NotifyReceived(i);
    }
  if (lossCounter.GetLost()!=7)
    {
      return true;
    }

  // reordering with loss
  lossCounter.NotifyReceived(255);
  // drop (2) seqNum 250, 251
  lossCounter.NotifyReceived(252);
  lossCounter.NotifyReceived(253);
  lossCounter.NotifyReceived(254);
  for (uint32_t i=256;i<300;i++)
    {
      lossCounter.NotifyReceived(i);
    }
  if (lossCounter.GetLost()!=9) //chek that 7 (6+1+2) packet are lost
    {
      return true;
    }


  return false;

}
class UdpClientServerTestSuite: public TestSuite
{
public:
  UdpClientServerTestSuite ();
};

UdpClientServerTestSuite::UdpClientServerTestSuite () :
  TestSuite ("udp-client-server", UNIT)
{
  AddTestCase (new UdpTraceClientServerTestCase);
  AddTestCase (new UdpClientServerTestCase);
  AddTestCase (new PacketLossCounterTestCase);
}

UdpClientServerTestSuite udpClientServerTestSuite;
