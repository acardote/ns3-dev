/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007, 2008 University of Washington
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

#include "ns3/log.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/mac48-address.h"
#include "ns3/llc-snap-header.h"
#include "ns3/error-model.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "point-to-point-net-device.h"
#include "point-to-point-channel.h"
#include "ppp-header.h"

NS_LOG_COMPONENT_DEFINE ("PointToPointNetDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (PointToPointNetDevice);

TypeId 
PointToPointNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PointToPointNetDevice")
    .SetParent<NetDevice> ()
    .AddConstructor<PointToPointNetDevice> ()
    .AddAttribute ("Address", 
                   "The MAC address of this device.",
                   Mac48AddressValue (Mac48Address ("ff:ff:ff:ff:ff:ff")),
                   MakeMac48AddressAccessor (&PointToPointNetDevice::m_address),
                   MakeMac48AddressChecker ())
    .AddAttribute ("FrameSize", 
                   "The maximum size of a packet sent over this device.",
                   UintegerValue (DEFAULT_FRAME_SIZE),
                   MakeUintegerAccessor (&PointToPointNetDevice::SetFrameSize,
                                         &PointToPointNetDevice::GetFrameSize),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("DataRate", 
                   "The default data rate for point to point links",
                   DataRateValue (DataRate ("32768b/s")),
                   MakeDataRateAccessor (&PointToPointNetDevice::m_bps),
                   MakeDataRateChecker ())
    .AddAttribute ("ReceiveErrorModel", 
                   "The receiver error model used to simulate packet loss",
                   PointerValue (),
                   MakePointerAccessor (&PointToPointNetDevice::m_receiveErrorModel),
                   MakePointerChecker<ErrorModel> ())
    .AddAttribute ("TxQueue",
                   "A queue to use as the transmit queue in the device.",
                   PointerValue (),
                   MakePointerAccessor (&PointToPointNetDevice::m_queue),
                   MakePointerChecker<Queue> ())
    .AddAttribute ("InterframeGap", 
                   "The time to wait between packet (frame) transmissions",
                   TimeValue (Seconds (0.0)),
                   MakeTimeAccessor (&PointToPointNetDevice::m_tInterframeGap),
                   MakeTimeChecker ())
    .AddTraceSource ("Rx", 
                     "Trace source to fire on reception of a MAC packet.",
                     MakeTraceSourceAccessor (&PointToPointNetDevice::m_rxTrace))
    .AddTraceSource ("Drop",
                     "Trace source to fire on when a MAC packet is dropped.",

                     MakeTraceSourceAccessor (&PointToPointNetDevice::m_dropTrace))

    ;
  return tid;
}


PointToPointNetDevice::PointToPointNetDevice () 
: 
  m_txMachineState (READY),
  m_channel (0), 
  m_name (""),
  m_linkUp (false)
{
  NS_LOG_FUNCTION (this);

  //
  // A quick sanity check to ensure consistent constants.
  //
  PppHeader ppp;
  NS_ASSERT_MSG (PPP_OVERHEAD == ppp.GetSerializedSize (), 
                 "PointToPointNetDevice::PointToPointNetDevice(): PPP_OVERHEAD inconsistent");

  m_frameSize = DEFAULT_FRAME_SIZE;
  m_mtu = MtuFromFrameSize (m_frameSize);
}

PointToPointNetDevice::~PointToPointNetDevice ()
{
}

  void 
PointToPointNetDevice::AddHeader(Ptr<Packet> p, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_ASSERT_MSG (protocolNumber == 0x800,
    "PointToPointNetDevice::AddHeader(): protocolNumber must be 0x800");
  PppHeader ppp;
  p->AddHeader (ppp);
}

  bool 
PointToPointNetDevice::ProcessHeader(Ptr<Packet> p, uint16_t& param)
{
  NS_LOG_FUNCTION_NOARGS ();
  PppHeader ppp;
  p->RemoveHeader (ppp);
  param = 0x800;
  return true;
}

  void 
PointToPointNetDevice::DoDispose()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_node = 0;
  m_channel = 0;
  m_receiveErrorModel = 0;
  NetDevice::DoDispose ();
}

  void 
PointToPointNetDevice::SetDataRate(DataRate bps)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_bps = bps;
}

  void 
PointToPointNetDevice::SetInterframeGap(Time t)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_tInterframeGap = t;
}

  bool
PointToPointNetDevice::TransmitStart (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  NS_LOG_LOGIC ("UID is " << p->GetUid () << ")");
//
// This function is called to start the process of transmitting a packet.
// We need to tell the channel that we've started wiggling the wire and
// schedule an event that will be executed when the transmission is complete.
//
  NS_ASSERT_MSG(m_txMachineState == READY, "Must be READY to transmit");
  m_txMachineState = BUSY;
  Time txTime = Seconds (m_bps.CalculateTxTime(p->GetSize()));
  Time txCompleteTime = txTime + m_tInterframeGap;

  NS_LOG_LOGIC ("Schedule TransmitCompleteEvent in " << 
    txCompleteTime.GetSeconds () << "sec");

  Simulator::Schedule (txCompleteTime, 
    &PointToPointNetDevice::TransmitComplete, this);

  return m_channel->TransmitStart(p, this, txTime); 
}

  void 
PointToPointNetDevice::TransmitComplete (void)
{
  NS_LOG_FUNCTION_NOARGS ();
//
// This function is called to when we're all done transmitting a packet.
// We try and pull another packet off of the transmit queue.  If the queue
// is empty, we are done, otherwise we need to start transmitting the
// next packet.
//
  NS_ASSERT_MSG(m_txMachineState == BUSY, "Must be BUSY if transmitting");
  m_txMachineState = READY;
  Ptr<Packet> p = m_queue->Dequeue ();
  if (p == 0)
    {
//
// No packet was on the queue, so we just exit.
//
      return;
    }
//
// Got another packet off of the queue, so start the transmit process agin.
//
  TransmitStart(p);
}

  bool 
PointToPointNetDevice::Attach (Ptr<PointToPointChannel> ch)
{
  NS_LOG_FUNCTION (this << &ch);

  m_channel = ch;

  m_channel->Attach(this);

//
// This device is up whenever it is attached to a channel.  A better plan
// would be to have the link come up when both devices are attached, but this
// is not done for now.
//
  NotifyLinkUp ();
  return true;
}

  void
PointToPointNetDevice::SetQueue (Ptr<Queue> q)
{
  NS_LOG_FUNCTION (this << q);
  m_queue = q;
}

  void
PointToPointNetDevice::SetReceiveErrorModel (Ptr<ErrorModel> em)
{
  NS_LOG_FUNCTION (this << em);
  m_receiveErrorModel = em;
}

 void
PointToPointNetDevice::Receive (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  uint16_t protocol = 0;

  if (m_receiveErrorModel && m_receiveErrorModel->IsCorrupt (packet) ) 
    {
// 
// If we have an error model and it indicates that it is time to lose a
// corrupted packet, don't forward this packet up, let it go.
//
      m_dropTrace (packet);
    }
  else 
    {
// 
// Hit the receive trace hook, strip off the point-to-point protocol header
// and forward this packet up the protocol stack.
//
      m_rxTrace (packet);
      ProcessHeader(packet, protocol);
      m_rxCallback (this, packet, protocol, GetRemote ());
      if (!m_promiscCallback.IsNull ())
        {
          m_promiscCallback (this, packet, protocol, GetRemote (), GetAddress (), NetDevice::PACKET_HOST);
        }
    }
}

  Ptr<Queue> 
PointToPointNetDevice::GetQueue(void) const 
{ 
  NS_LOG_FUNCTION_NOARGS ();
  return m_queue;
}

  void
PointToPointNetDevice::NotifyLinkUp (void)
{
  m_linkUp = true;
  if (!m_linkChangeCallback.IsNull ())
    {
      m_linkChangeCallback ();
    }
}

  void 
PointToPointNetDevice::SetName(const std::string name)
{
  m_name = name;
}

  std::string 
PointToPointNetDevice::GetName(void) const
{
  return m_name;
}

  void 
PointToPointNetDevice::SetIfIndex(const uint32_t index)
{
  m_ifIndex = index;
}

  uint32_t 
PointToPointNetDevice::GetIfIndex(void) const
{
  return m_ifIndex;
}

  Ptr<Channel> 
PointToPointNetDevice::GetChannel (void) const
{
  return m_channel;
}

//
// This is a point-to-point device, so we really don't need any kind of address
// information.  However, the base class NetDevice wants us to define the
// methods to get and set the address.  Rather than be rude and assert, we let
// clients get and set the address, but simply ignore them.
  void 
PointToPointNetDevice::SetAddress (Mac48Address addr)
{
  m_address = addr;
}

  Address 
PointToPointNetDevice::GetAddress (void) const
{
  return m_address;
}

  bool 
PointToPointNetDevice::IsLinkUp (void) const
{
  return m_linkUp;
}

  void 
PointToPointNetDevice::SetLinkChangeCallback (Callback<void> callback)
{
  m_linkChangeCallback = callback;
}

//
// This is a point-to-point device, so every transmission is a broadcast to
// all of the devices on the network.
//
  bool 
PointToPointNetDevice::IsBroadcast (void) const
{
  return true;
}

//
// We don't really need any addressing information since this is a 
// point-to-point device.  The base class NetDevice wants us to return a
// broadcast address, so we make up something reasonable.
//
  Address
PointToPointNetDevice::GetBroadcast (void) const
{
  return Mac48Address ("ff:ff:ff:ff:ff:ff");
}

//
// We don't deal with multicast here.  It doesn't make sense to include some
// of the one destinations on the network but exclude some of the others.
//
  bool 
PointToPointNetDevice::IsMulticast (void) const
{
  return false;
}

  Address 
PointToPointNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  return Mac48Address ("01:00:5e:00:00:00");
}

Address
PointToPointNetDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION(this << addr);
  return Mac48Address ("33:33:00:00:00:00");
}

  bool 
PointToPointNetDevice::IsPointToPoint (void) const
{
  return true;
}

  bool 
PointToPointNetDevice::IsBridge (void) const
{
  return false;
}

  bool 
PointToPointNetDevice::Send(
  Ptr<Packet> packet, 
  const Address &dest, 
  uint16_t protocolNumber)
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_LOGIC ("p=" << packet << ", dest=" << &dest);
  NS_LOG_LOGIC ("UID is " << packet->GetUid ());

//
// If IsLinkUp() is false it means there is no channel to send any packet 
// over so we just return an error.
//
  if (IsLinkUp () == false)
    {
      return false;
    }

//
// Stick a point to point protocol header on the packet in preparation for
// shoving it out the door.
//
  AddHeader(packet, protocolNumber);

//
// If there's a transmission in progress, we enque the packet for later
// transmission; otherwise we send it now.
//
  if (m_txMachineState == READY) 
    {
// 
// Even if the transmitter is immediately available, we still enqueue and 
// dequeue the packet to hit the tracing hooks.
//
      m_queue->Enqueue (packet);
      packet = m_queue->Dequeue ();
      return TransmitStart (packet);
    }
  else
    {
      return m_queue->Enqueue(packet);
    }
}

bool
PointToPointNetDevice::SendFrom (Ptr<Packet> packet, 
                                 const Address &source, 
                                 const Address &dest, 
                                 uint16_t protocolNumber)
{
  return false;
}

  Ptr<Node> 
PointToPointNetDevice::GetNode (void) const
{
  return m_node;
}

  void 
PointToPointNetDevice::SetNode (Ptr<Node> node)
{
  m_node = node;
}

  bool 
PointToPointNetDevice::NeedsArp (void) const
{
  return false;
}

  void 
PointToPointNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  m_rxCallback = cb;
}

void
PointToPointNetDevice::SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb)
{
  NS_FATAL_ERROR ("not implemented");
  m_promiscCallback = cb;
}

  bool
PointToPointNetDevice::SupportsSendFrom (void) const
{
  return false;
}

Address 
PointToPointNetDevice::GetRemote (void) const
{
  NS_ASSERT (m_channel->GetNDevices () == 2);
  for (uint32_t i = 0; i < m_channel->GetNDevices (); ++i)
    {
      Ptr<NetDevice> tmp = m_channel->GetDevice (i);
      if (tmp != this)
        {
          return tmp->GetAddress ();
        }
    }
  NS_ASSERT (false);
  // quiet compiler.
  return Address ();
}

  uint32_t
PointToPointNetDevice::MtuFromFrameSize (uint32_t frameSize)
{
  NS_LOG_FUNCTION (frameSize);
  NS_ASSERT_MSG (frameSize <= std::numeric_limits<uint16_t>::max (), 
                 "PointToPointNetDevice::MtuFromFrameSize(): Frame size should be derived from 16-bit quantity: " << 
                 frameSize);
  PppHeader ppp;
  NS_ASSERT_MSG ((uint32_t)frameSize >= ppp.GetSerializedSize (), 
                 "PointToPointNetDevice::MtuFromFrameSize(): Given frame size too small to support PPP");
  return frameSize - ppp.GetSerializedSize ();
}
  
  uint32_t
PointToPointNetDevice::FrameSizeFromMtu (uint32_t mtu)
{
  NS_LOG_FUNCTION (mtu);

  PppHeader ppp;
  return mtu + ppp.GetSerializedSize ();
}

  void 
PointToPointNetDevice::SetFrameSize (uint16_t frameSize)
{
  NS_LOG_FUNCTION (frameSize);

  m_frameSize = frameSize;
  m_mtu = MtuFromFrameSize (frameSize);

  NS_LOG_LOGIC ("m_frameSize = " << m_frameSize);
  NS_LOG_LOGIC ("m_mtu = " << m_mtu);
}

  uint16_t
PointToPointNetDevice::GetFrameSize (void) const
{
  return m_frameSize;
}

  bool
PointToPointNetDevice::SetMtu (uint16_t mtu)
{
  NS_LOG_FUNCTION (mtu);

  uint32_t newFrameSize = FrameSizeFromMtu (mtu);

  if (newFrameSize > std::numeric_limits<uint16_t>::max ())
    {
      NS_LOG_WARN ("PointToPointNetDevice::SetMtu(): Frame size overflow, MTU not set.");
      return false;
    }

  m_frameSize = newFrameSize;
  m_mtu = mtu;

  NS_LOG_LOGIC ("m_frameSize = " << m_frameSize);
  NS_LOG_LOGIC ("m_mtu = " << m_mtu);

  return true;
}

  uint16_t
PointToPointNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_mtu;
}


} // namespace ns3
