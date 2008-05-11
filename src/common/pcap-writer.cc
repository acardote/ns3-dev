/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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

/*
 * Documentation kindly pointed out by Tom Henderson:
 * http://wiki.ethereal.com/Development/LibpcapFileFormat
 */

#include <fstream>

#include "ns3/simulator.h"
#include "pcap-writer.h"
#include "packet.h"


namespace ns3 {

enum {
  PCAP_ETHERNET = 1,
  PCAP_PPP      = 9,
  PCAP_RAW_IP   = 101,
  PCAP_80211    = 105,
};

PcapWriter::PcapWriter ()
{
  m_writer = 0;
}

PcapWriter::~PcapWriter ()
{
  delete m_writer;
}

void
PcapWriter::Open (std::string const &name)
{
  m_writer = new std::ofstream ();
  m_writer->open (name.c_str ());
}

void 
PcapWriter::WriteEthernetHeader (void)
{
  WriteHeader (PCAP_ETHERNET);
}

void 
PcapWriter::WriteIpHeader (void)
{
  WriteHeader (PCAP_RAW_IP);
}

void
PcapWriter::WriteWifiHeader (void)
{
  WriteHeader (PCAP_80211);
}

void 
PcapWriter::WritePppHeader (void)
{
  WriteHeader (PCAP_PPP);
}

void 
PcapWriter::WriteHeader (uint32_t network)
{
  Write32 (0xa1b2c3d4);
  Write16 (2);
  Write16 (4);
  Write32 (0);
  Write32 (0);
  Write32 (0xffff);
  Write32 (network);
}

void 
PcapWriter::WritePacket (Ptr<const Packet> packet)
{
  if (m_writer != 0) 
    {
      uint64_t current = Simulator::Now ().GetMicroSeconds ();
      uint64_t s = current / 1000000;
      uint64_t us = current % 1000000;
      Write32 (s & 0xffffffff);
      Write32 (us & 0xffffffff);
      Write32 (packet->GetSize ());
      Write32 (packet->GetSize ());
      WriteData (packet->PeekData (), packet->GetSize ());
    }
}

void
PcapWriter::WriteData (uint8_t const*buffer, uint32_t size)
{
  m_writer->write ((char const *)buffer, size);
}

void
PcapWriter::Write32 (uint32_t data)
{
  uint8_t buffer[4];
  buffer[0] = (data >> 0) & 0xff;
  buffer[1] = (data >> 8) & 0xff;
  buffer[2] = (data >> 16) & 0xff;
  buffer[3] = (data >> 24) & 0xff;
  WriteData (buffer, 4);
}

void
PcapWriter::Write16 (uint16_t data)
{
  uint8_t buffer[2];
  buffer[0] = (data >> 0) & 0xff;
  buffer[1] = (data >> 8) & 0xff;
  WriteData (buffer, 2);
}

} // namespace ns3
