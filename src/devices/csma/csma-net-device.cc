/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 Emmanuelle Laprise
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
 * Author: Emmanuelle Laprise <emmanuelle.laprise@bluekazoo.ca>
 */

#include "ns3/log.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/ethernet-header.h"
#include "ns3/ethernet-trailer.h"
#include "ns3/llc-snap-header.h"
#include "ns3/error-model.h"
#include "ns3/enum.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/trace-source-accessor.h"
#include "csma-net-device.h"
#include "csma-channel.h"

NS_LOG_COMPONENT_DEFINE ("CsmaNetDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CsmaNetDevice);

  TypeId 
CsmaNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CsmaNetDevice")
    .SetParent<NetDevice> ()
    .AddConstructor<CsmaNetDevice> ()
    .AddAttribute ("Address", 
                   "The MAC address of this device.",
                   Mac48AddressValue (Mac48Address ("ff:ff:ff:ff:ff:ff")),
                   MakeMac48AddressAccessor (&CsmaNetDevice::m_address),
                   MakeMac48AddressChecker ())
    .AddAttribute ("PayloadLength", 
                   "The max PHY-level payload length of packets sent over this device.",
                   UintegerValue (DEFAULT_PAYLOAD_LENGTH),
                   MakeUintegerAccessor (&CsmaNetDevice::SetMaxPayloadLength,
                                         &CsmaNetDevice::GetMaxPayloadLength),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("MTU", 
                   "The MAC-level MTU (client payload) of packets sent over this device.",
                   UintegerValue (DEFAULT_MTU),
                   MakeUintegerAccessor (&CsmaNetDevice::SetMacMtu,
                                         &CsmaNetDevice::GetMacMtu),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("EncapsulationMode", 
                   "The link-layer encapsulation type to use.",
                   EnumValue (LLC),
                   MakeEnumAccessor (&CsmaNetDevice::SetEncapsulationMode),
                   MakeEnumChecker (ETHERNET_V1, "EthernetV1",
                                    IP_ARP, "IpArp",
                                    RAW, "Raw",
                                    LLC, "Llc"))
    .AddAttribute ("SendEnable", 
                   "Enable or disable the transmitter section of the device.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&CsmaNetDevice::m_sendEnable),
                   MakeBooleanChecker ())
    .AddAttribute ("ReceiveEnable",
                   "Enable or disable the receiver section of the device.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&CsmaNetDevice::m_receiveEnable),
                   MakeBooleanChecker ())
    .AddAttribute ("RxErrorModel", 
                   "The receiver error model used to simulate packet loss",
                   PointerValue (),
                   MakePointerAccessor (&CsmaNetDevice::m_receiveErrorModel),
                   MakePointerChecker<ErrorModel> ())
    .AddAttribute ("TxQueue", 
                   "A queue to use as the transmit queue in the device.",
                   PointerValue (),
                   MakePointerAccessor (&CsmaNetDevice::m_queue),
                   MakePointerChecker<Queue> ())
    .AddTraceSource ("Rx", 
                     "The trace source to fire on reception of a MAC packet.",
                     MakeTraceSourceAccessor (&CsmaNetDevice::m_rxTrace))
    .AddTraceSource ("Drop", 
                     "Trace source to fire on when a MAC packet is dropped.",
                     MakeTraceSourceAccessor (&CsmaNetDevice::m_dropTrace))
    ;
  return tid;
}

CsmaNetDevice::CsmaNetDevice ()
  : m_name (""),
    m_linkUp (false)
{
  NS_LOG_FUNCTION (this);
  m_txMachineState = READY;
  m_tInterframeGap = Seconds (0);
  m_channel = 0; 

  m_encapMode = LLC;
  m_maxPayloadLength = DEFAULT_PAYLOAD_LENGTH;
  m_mtu = MacMtuFromPayload (m_maxPayloadLength);

  NS_LOG_LOGIC ("m_encapMode = " << m_encapMode);
  NS_LOG_LOGIC ("m_maxPayloadLength = " << m_maxPayloadLength);
  NS_LOG_LOGIC ("m_mtu = " << m_mtu);
}

CsmaNetDevice::~CsmaNetDevice()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_queue = 0;
}

  void 
CsmaNetDevice::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_channel = 0;
  m_node = 0;
  NetDevice::DoDispose ();
}

  uint16_t
CsmaNetDevice::MacMtuFromPayload (uint16_t payloadLength)
{
  NS_LOG_FUNCTION (payloadLength);

  switch (m_encapMode) 
    {
    case RAW:
    case IP_ARP:
    case ETHERNET_V1:
      return payloadLength;
    case LLC: 
      {
        LlcSnapHeader llc;

        NS_ASSERT_MSG (payloadLength >= llc.GetSerializedSize (), "CsmaNetDevice::MacMtuFromPayload(): "
                       "Given payload too small to support LLC mode");
        return payloadLength - llc.GetSerializedSize ();
      }
    }

  NS_ASSERT_MSG (false, "CsmaNetDevice::MacMtuFromPayload(): Unexpected encapsulation mode");
  return 0;
}
  
  uint16_t
CsmaNetDevice::PayloadFromMacMtu (uint16_t mtu)
{
  NS_LOG_FUNCTION (mtu);

  switch (m_encapMode) 
    {
    case RAW:
    case IP_ARP:
    case ETHERNET_V1:
      return mtu;
    case LLC: 
      {
        LlcSnapHeader llc;
        return mtu + llc.GetSerializedSize ();
      }
    }

  NS_ASSERT_MSG (false, "CsmaNetDevice::PayloadFromMacMtu(): Unexpected encapsulation mode");
  return 0;
}

  void 
CsmaNetDevice::SetEncapsulationMode (enum EncapsulationMode mode)
{
  NS_LOG_FUNCTION (mode);

  m_encapMode = mode;
  m_mtu = MacMtuFromPayload (m_maxPayloadLength);

  NS_LOG_LOGIC ("m_encapMode = " << m_encapMode);
  NS_LOG_LOGIC ("m_maxPayloadLength = " << m_maxPayloadLength);
  NS_LOG_LOGIC ("m_mtu = " << m_mtu);
}

  CsmaNetDevice::EncapsulationMode
CsmaNetDevice::GetEncapsulationMode (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_encapMode;
}
  
  void 
CsmaNetDevice::SetMacMtu (uint16_t mtu)
{
  NS_LOG_FUNCTION (mtu);

  m_maxPayloadLength = PayloadFromMacMtu (mtu);
  m_mtu = mtu;

  NS_LOG_LOGIC ("m_encapMode = " << m_encapMode);
  NS_LOG_LOGIC ("m_maxPayloadLength = " << m_maxPayloadLength);
  NS_LOG_LOGIC ("m_mtu = " << m_mtu);
}

  uint16_t
CsmaNetDevice::GetMacMtu (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_mtu;
}

//
// The SetMtu method in the net device base class always refers to the MAC-level MTU by definition.
//
  bool 
CsmaNetDevice::SetMtu (const uint16_t mtu)
{
  NS_LOG_FUNCTION (mtu);
  SetMacMtu (mtu);
  return true;
}

//
// The GetMtu method in the net device base class always refers to the MAC-level MTU by definition.
//
  uint16_t 
CsmaNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return GetMacMtu ();
}

  void 
CsmaNetDevice::SetMaxPayloadLength (uint16_t maxPayloadLength)
{
  NS_LOG_FUNCTION (maxPayloadLength);

  m_maxPayloadLength = maxPayloadLength;
  m_mtu = MacMtuFromPayload (maxPayloadLength);

  NS_LOG_LOGIC ("m_encapMode = " << m_encapMode);
  NS_LOG_LOGIC ("m_maxPayloadLength = " << m_maxPayloadLength);
  NS_LOG_LOGIC ("m_mtu = " << m_mtu);
}

  uint16_t
CsmaNetDevice::GetMaxPayloadLength (void) const
{
  return m_maxPayloadLength;
}

  void 
CsmaNetDevice::SetAddress (Mac48Address self)
{
  NS_LOG_FUNCTION (self);
  m_address = self;
}

  void
CsmaNetDevice::SetSendEnable (bool sendEnable)
{
  NS_LOG_FUNCTION (sendEnable);
  m_sendEnable = sendEnable;
}

  void
CsmaNetDevice::SetReceiveEnable (bool receiveEnable)
{
  NS_LOG_FUNCTION (receiveEnable);
  m_receiveEnable = receiveEnable;
}

  bool
CsmaNetDevice::IsSendEnabled (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_sendEnable;
}

  bool
CsmaNetDevice::IsReceiveEnabled (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_receiveEnable;
}

  void 
CsmaNetDevice::SetInterframeGap (Time t)
{
  NS_LOG_FUNCTION (t);
  m_tInterframeGap = t;
}

  void 
CsmaNetDevice::SetBackoffParams (Time slotTime, uint32_t minSlots, uint32_t maxSlots, uint32_t ceiling, uint32_t maxRetries)
{
  NS_LOG_FUNCTION (slotTime << minSlots << maxSlots << ceiling << maxRetries);
  m_backoff.m_slotTime = slotTime;
  m_backoff.m_minSlots = minSlots;
  m_backoff.m_maxSlots = maxSlots;
  m_backoff.m_ceiling = ceiling;
  m_backoff.m_maxRetries = maxRetries;
}

  void 
CsmaNetDevice::AddHeader (Ptr<Packet> p,   Mac48Address source,  Mac48Address dest,  uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (p << source << dest << protocolNumber);

  if (m_encapMode == RAW)
    {
      return;
    }

  EthernetHeader header (false);
  header.SetSource (source);
  header.SetDestination (dest);

  EthernetTrailer trailer;

  NS_LOG_LOGIC ("p->GetSize () = " << p->GetSize ());
  NS_LOG_LOGIC ("m_encapMode = " << m_encapMode);
  NS_LOG_LOGIC ("m_mtu = " << m_mtu);
  NS_LOG_LOGIC ("m_maxPayloadLength = " << m_maxPayloadLength);

  uint16_t lengthType = 0;
  switch (m_encapMode) 
    {
    case IP_ARP:
      NS_LOG_LOGIC ("Encapsulating packet as IP_ARP (type interpretation)");
      //
      // This corresponds to the type interpretation of the lengthType field.
      //
      lengthType = protocolNumber;
      break;
    case ETHERNET_V1:
      NS_LOG_LOGIC ("Encapsulating packet as ETHERNET_V1 "
        "(length interpretation)");
      //
      // This corresponds to the length interpretation of the lengthType field.
      // The ethernet header and trailer are not counted, see RFC 1042 and
      // http://standards.ieee.org/getieee802/download/802.3-2005_section1.pdf,
      // Section 3.2.6 a.  We just include the size of the "payload."
      //
      lengthType = p->GetSize ();
      NS_ASSERT_MSG (lengthType <= m_maxPayloadLength,
                     "CsmaNetDevice::AddHeader(): 802.3 Length/Type field: "
                     "length interpretation must not exceed device max payload length");
      break;
    case LLC: 
      {
        NS_LOG_LOGIC ("Encapsulating packet as LLC (length interpretation)");

        LlcSnapHeader llc;
        llc.SetType (protocolNumber);
        p->AddHeader (llc);
        //
        // This corresponds to the length interpretation of the lengthType field,
        // but with an LLC/SNAP header added to the payload.
        //      
        lengthType = p->GetSize ();
        NS_ASSERT_MSG (lengthType <= m_maxPayloadLength,
          "CsmaNetDevice::AddHeader(): 802.3 Length/Type field with LLC/SNAP: "
          "length interpretation must not exceed device max payload length");
      }
      break;
    case RAW:
      NS_ASSERT_MSG (false, "CsmaNetDevice::AddHeader(): RAW packet encapsulation not supported");
      break;
    }

  NS_LOG_LOGIC ("header.SetLengthType (" << lengthType << ")");
  header.SetLengthType (lengthType);
  p->AddHeader (header);

  trailer.CalcFcs (p);
  p->AddTrailer (trailer);
}

  bool 
CsmaNetDevice::ProcessHeader (Ptr<Packet> p, uint16_t & param)
{
  NS_LOG_FUNCTION (p << param);

  if (m_encapMode == RAW)
    {
      return true;
    }

  EthernetTrailer trailer;
      
  p->RemoveTrailer (trailer);
  trailer.CheckFcs (p);

  EthernetHeader header (false);
  p->RemoveHeader (header);

  if ((header.GetDestination () != GetBroadcast ()) &&
      (header.GetDestination () != GetAddress ()))
    {
      return false;
    }

  switch (m_encapMode)
    {
    case ETHERNET_V1:
    case IP_ARP:
      param = header.GetLengthType ();
      break;
    case LLC: {
      LlcSnapHeader llc;
      p->RemoveHeader (llc);
      param = llc.GetType ();
    } break;
    case RAW:
      NS_ASSERT (false);
      break;
    }
  return true;
}

  void
CsmaNetDevice::TransmitStart ()
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_LOG_LOGIC ("m_currentPkt=" << m_currentPkt);
  NS_LOG_LOGIC ("UID is " << m_currentPkt->GetUid ());
  //
  // This function is called to start the process of transmitting a packet.
  // We need to tell the channel that we've started wiggling the wire and
  // schedule an event that will be executed when it's time to tell the 
  // channel that we're done wiggling the wire.
  //
  NS_ASSERT_MSG ((m_txMachineState == READY) || (m_txMachineState == BACKOFF), 
                 "Must be READY to transmit. Tx state is: " << m_txMachineState);

  //
  // Only transmit if send side of net device is enabled
  //
  if (IsSendEnabled () == false)
    {
      return;
    }

  if (m_channel->GetState () != IDLE)
    {
      //
      // The channel is busy -- backoff and rechedule TransmitStart ()
      //
      m_txMachineState = BACKOFF;

      if (m_backoff.MaxRetriesReached ())
        { 
          //
          // Too many retries, abort transmission of packet
          //
          TransmitAbort ();
        } 
      else 
        {
          m_backoff.IncrNumRetries ();
          Time backoffTime = m_backoff.GetBackoffTime ();

          NS_LOG_LOGIC ("Channel busy, backing off for " << backoffTime.GetSeconds () << " sec");

          Simulator::Schedule (backoffTime, &CsmaNetDevice::TransmitStart, this);
        }
    } 
  else 
    {
      //
      // The channel is free, transmit the packet
      //
      m_txMachineState = BUSY;
      Time tEvent = Seconds (m_bps.CalculateTxTime (m_currentPkt->GetSize ()));
      
      NS_LOG_LOGIC ("Schedule TransmitCompleteEvent in " << tEvent.GetSeconds () << "sec");
      
      Simulator::Schedule (tEvent, &CsmaNetDevice::TransmitCompleteEvent, this);

      if (m_channel->TransmitStart (m_currentPkt, m_deviceId) == false)
        {
          NS_LOG_WARN ("Channel transmit start did not work at " << tEvent.GetSeconds () << "sec");
          m_txMachineState = READY;
        } 
      else 
        {
          //
          // Transmission succeeded, reset the backoff time parameters.
          //
          m_backoff.ResetBackoffTime ();
        }
    }
}


  void
CsmaNetDevice::TransmitAbort (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_LOG_LOGIC ("Pkt UID is " << m_currentPkt->GetUid () << ")");

  //
  // Since we were transmitting a packet, that packet had better be on the transmit queue.
  //
  m_currentPkt = m_queue->Dequeue ();
  NS_ASSERT_MSG (m_currentPkt != 0, "No Packet on queue during CsmaNetDevice::TransmitAbort()");

  //
  // The last one failed.  Let's try to transmit the next one (if there)
  //
  m_backoff.ResetBackoffTime ();
  m_txMachineState = READY;
  TransmitStart ();
}

  void
CsmaNetDevice::TransmitCompleteEvent (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  //
  // This function is called to finish the  process of transmitting a packet.
  // We need to tell the channel that we've stopped wiggling the wire and
  // schedule an event that will be executed when it's time to re-enable
  // the transmitter after the interframe gap.
  //
  NS_ASSERT_MSG (m_txMachineState == BUSY, "Must be BUSY if transmitting");
  NS_ASSERT (m_channel->GetState () == TRANSMITTING);
  m_txMachineState = GAP;

  NS_LOG_LOGIC ("Pkt UID is " << m_currentPkt->GetUid () << ")");
  m_channel->TransmitEnd (); 

  NS_LOG_LOGIC ("Schedule TransmitReadyEvent in " << m_tInterframeGap.GetSeconds () << "sec");

  Simulator::Schedule (m_tInterframeGap, &CsmaNetDevice::TransmitReadyEvent, this);
}

  void
CsmaNetDevice::TransmitReadyEvent (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  //
  // This function is called to enable the transmitter after the interframe
  // gap has passed.  If there are pending transmissions, we use this opportunity
  // to start the next transmit.
  //
  NS_ASSERT_MSG (m_txMachineState == GAP, "Must be in interframe gap");
  m_txMachineState = READY;

  //
  // Get the next packet from the queue for transmitting
  //
  if (m_queue->IsEmpty ())
    {
      return;
    }
  else
    {
      m_currentPkt = m_queue->Dequeue ();
      NS_ASSERT_MSG (m_currentPkt != 0, "CsmaNetDevice::TransmitReadyEvent(): IsEmpty false but no Packet on queue?");
      TransmitStart ();
    }
}

  bool
CsmaNetDevice::Attach (Ptr<CsmaChannel> ch)
{
  NS_LOG_FUNCTION (this << &ch);

  m_channel = ch;

  m_deviceId = m_channel->Attach (this);

  //
  // The channel provides us with the transmitter data rate.
  //
  m_bps = m_channel->GetDataRate ();

  //
  // We use the Ethernet interframe gap of 96 bit times.
  //
  m_tInterframeGap = Seconds (m_bps.CalculateTxTime (96/8));

  //
  // This device is up whenever a channel is attached to it.
  //
  NotifyLinkUp ();
  return true;
}

  void
CsmaNetDevice::SetQueue (Ptr<Queue> q)
{
  NS_LOG_FUNCTION (q);
  m_queue = q;
}

  void
CsmaNetDevice::SetReceiveErrorModel (Ptr<ErrorModel> em)
{
  NS_LOG_FUNCTION (em);
  m_receiveErrorModel = em; 
}

  void
CsmaNetDevice::Receive (Ptr<Packet> packet, Ptr<CsmaNetDevice> senderDevice)
{
  NS_LOG_FUNCTION (packet << senderDevice);
  NS_LOG_LOGIC ("UID is " << packet->GetUid ());

  //
  // We never forward up packets that we sent. Real devices don't do this since
  // their receivers are disabled during send, so we don't. Drop the packet
  // silently (no tracing) since it would really never get here in a real device.
  // 
  if (senderDevice == this)
    {
      return;
    }

  // 
  // Only receive if the send side of net device is enabled
  //
  if (IsReceiveEnabled () == false)
    {
      m_dropTrace (packet);
      return;
    }

  if (m_encapMode == RAW)
    {
      m_rxTrace (packet);
      if (!m_promiscRxCallback.IsNull ())
        {
          m_promiscRxCallback (this, packet, 0, GetBroadcast (), GetAddress (), PACKET_HOST);
        }
      m_rxCallback (this, packet, 0, GetBroadcast ());
      return;
    }

  //
  // Trace sinks will expect complete packets, not packets without some of the
  // headers.
  //
  Ptr<Packet> originalPacket = packet->Copy ();

  EthernetTrailer trailer;
  packet->RemoveTrailer (trailer);
  trailer.CheckFcs (packet);

  EthernetHeader header (false);
  packet->RemoveHeader (header);

  NS_LOG_LOGIC ("Pkt source is " << header.GetSource ());
  NS_LOG_LOGIC ("Pkt destination is " << header.GetDestination ());

  if (m_receiveErrorModel && m_receiveErrorModel->IsCorrupt (packet) )
    {
      NS_LOG_LOGIC ("Dropping pkt due to error model ");
      m_dropTrace (packet);
    }
  else
    {
      //
      // variable <protocol> must be initialized to avoid a compiler warning in the RAW case that breaks the optimized build.
      //
      uint16_t protocol = 0;

      switch (m_encapMode)
        {
        case ETHERNET_V1:
        case IP_ARP:
          protocol = header.GetLengthType ();
          break;
        case LLC: 
          {
            LlcSnapHeader llc;
            packet->RemoveHeader (llc);
            protocol = llc.GetType ();
          } 
          break;
        case RAW:
          NS_ASSERT (false);
          break;
        }

      PacketType packetType;
      
      if (header.GetDestination ().IsBroadcast ())
        {
          packetType = PACKET_BROADCAST;
          m_rxTrace (originalPacket);
        }
      else if (header.GetDestination ().IsMulticast ())
        {
          packetType = PACKET_MULTICAST;          
          m_rxTrace (originalPacket);
        }
      else if (header.GetDestination () == m_address)
        {
          packetType = PACKET_HOST;
          m_rxTrace (originalPacket);
        }
      else
        {
          packetType = PACKET_OTHERHOST;
        }
      
      if (!m_promiscRxCallback.IsNull ())
        {
          m_promiscRxCallback (this, packet, protocol, header.GetSource (), header.GetDestination (), packetType);
        }

      if (packetType != PACKET_OTHERHOST)
        {
          m_rxCallback (this, packet, protocol, header.GetSource ());
        }
    }
}

  Ptr<Queue>
CsmaNetDevice::GetQueue (void) const 
{ 
  NS_LOG_FUNCTION_NOARGS ();
  return m_queue;
}

  void
CsmaNetDevice::NotifyLinkUp (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_linkUp = true;
  if (m_linkChangeCallback.IsNull () == false)
    {
      m_linkChangeCallback ();
    }
}

  void 
CsmaNetDevice::SetName (const std::string name)
{
  NS_LOG_FUNCTION (name);
  m_name = name;
}

  std::string 
CsmaNetDevice::GetName (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_name;
}

  void 
CsmaNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (index);
  m_ifIndex = index;
}

  uint32_t 
CsmaNetDevice::GetIfIndex (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_ifIndex;
}

  Ptr<Channel> 
CsmaNetDevice::GetChannel (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_channel;
}

  Address 
CsmaNetDevice::GetAddress (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_address;
}

  bool 
CsmaNetDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_linkUp;
}

  void 
CsmaNetDevice::SetLinkChangeCallback (Callback<void> callback)
{
  NS_LOG_FUNCTION (&callback);
  m_linkChangeCallback = callback;
}

  bool 
CsmaNetDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}

  Address
CsmaNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return Mac48Address ("ff:ff:ff:ff:ff:ff");
}

  bool 
CsmaNetDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}

  Address 
CsmaNetDevice::GetMulticast (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return Mac48Address::GetMulticastPrefix ();
}

  Address 
CsmaNetDevice::MakeMulticastAddress (Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION (multicastGroup);

  Mac48Address ad = Mac48Address::GetMulticast (multicastGroup);

  //
  // Implicit conversion (operator Address ()) is defined for Mac48Address, so
  // use it by just returning the EUI-48 address which is automagically converted
  // to an Address.
  //
  NS_LOG_LOGIC ("multicast address is " << ad);

  return ad;
}

  bool 
CsmaNetDevice::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return false;
}

  bool
CsmaNetDevice::Send (Ptr<Packet> packet,const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (packet << dest << protocolNumber);
  return SendFrom (packet, m_address, dest, protocolNumber);
}

  bool
CsmaNetDevice::SendFrom (Ptr<Packet> packet, const Address& src, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (packet << src << dest << protocolNumber);
  NS_LOG_LOGIC ("p=" << packet);
  NS_LOG_LOGIC ("UID is " << packet->GetUid () << ")");

  NS_ASSERT (IsLinkUp ());

  //
  // Only transmit if send side of net device is enabled
  //
  if (IsSendEnabled () == false)
    {
      return false;
    }

  Mac48Address destination = Mac48Address::ConvertFrom (dest);
  Mac48Address source = Mac48Address::ConvertFrom (src);
  AddHeader (packet, source, destination, protocolNumber);

  //
  // Place the packet to be sent on the send queue
  //
  if (m_queue->Enqueue(packet) == false)
    {
      return false;
    }

  //
  // If the device is idle, we need to start a transmission. Otherwise,
  // the transmission will be started when the current packet finished
  // transmission (see TransmitCompleteEvent)
  //
  if (m_txMachineState == READY) 
    {
      //
      // The next packet to be transmitted goes in m_currentPkt
      //
      m_currentPkt = m_queue->Dequeue ();
      if (m_currentPkt != 0)
        {
          TransmitStart ();
        }
    }
  return true;
}

  Ptr<Node> 
CsmaNetDevice::GetNode (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_node;
}

  void 
CsmaNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (node);

  m_node = node;
  int count = -1;
  if (m_name.size () == 0)
    {
      for (uint32_t i = 0; i < node->GetNDevices (); i++)
        {
          Ptr<NetDevice> dev = node->GetDevice (i);
          if (dynamic_cast<CsmaNetDevice*> (PeekPointer (dev)))
            {
              count++;
              if (dev == this)
                {
                  break;
                }
            }
        }
      std::ostringstream s;
      s << "eth" << count;
      m_name = s.str ();
    }
}

  bool 
CsmaNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  if ((m_encapMode == IP_ARP) || (m_encapMode == LLC))
    {
      return true;
    } 
  else 
    {
      return false;
    }
}

  void 
CsmaNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (&cb);
  m_rxCallback = cb;
}

  void 
CsmaNetDevice::SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION (&cb);
  m_promiscRxCallback = cb;
}

  bool 
CsmaNetDevice::SupportsSendFrom () const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}

} // namespace ns3
