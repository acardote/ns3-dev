/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 INRIA
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
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "ns3/system-wall-clock-ms.h"
#include "ns3/packet.h"
#include "ns3/packet-metadata.h"
#include <iostream>
#include <sstream>

using namespace ns3;

template <int N>
class BenchHeader : public Header
{
public:
  BenchHeader ();
  bool IsOk (void) const;

  static uint32_t GetUid (void);

  std::string GetName (void) const;
  void Print (std::ostream &os) const;
  uint32_t GetSerializedSize (void) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
private:
  bool m_ok;
};

template <int N>
BenchHeader<N>::BenchHeader ()
  : m_ok (false)
{}

template <int N>
bool 
BenchHeader<N>::IsOk (void) const
{
  return m_ok;
}

template <int N>
uint32_t 
BenchHeader<N>::GetUid (void)
{
  std::ostringstream oss;
  oss << "BenchHeader" << N;
  static uint32_t uid = AllocateUid<BenchHeader<N> > (oss.str ());
  return uid;
}

template <int N>
std::string 
BenchHeader<N>::GetName (void) const
{
  std::ostringstream oss;
  oss << N;
  return oss.str ();
}

template <int N>
void 
BenchHeader<N>::Print (std::ostream &os) const
{
  NS_ASSERT (false);
}
template <int N>
uint32_t 
BenchHeader<N>::GetSerializedSize (void) const
{
  return N;
}
template <int N>
void 
BenchHeader<N>::Serialize (Buffer::Iterator start) const
{
  start.WriteU8 (N, N);
}
template <int N>
uint32_t
BenchHeader<N>::Deserialize (Buffer::Iterator start)
{
  m_ok = true;
  for (int i = 0; i < N; i++)
    {
      if (start.ReadU8 () != N)
        {
          m_ok = false;
        }
    }
  return N;
}



static void 
benchPtrA (uint32_t n)
{
  BenchHeader<25> ipv4;
  BenchHeader<8> udp;

  for (uint32_t i = 0; i < n; i++) {
      Packet p (2000);
      p.AddHeader (udp);
      p.AddHeader (ipv4);
      Packet o = p;
      o.RemoveHeader (ipv4);
      o.RemoveHeader (udp);
  }
}

static void 
benchPtrB (uint32_t n)
{
  BenchHeader<25> ipv4;
  BenchHeader<8> udp;

  for (uint32_t i = 0; i < n; i++) {
      Packet p (2000);
      p.AddHeader (udp);
      p.AddHeader (ipv4);
  }
}

static void
ptrC2 (Packet p)
{
  BenchHeader<8> udp;

  p.RemoveHeader (udp);
}

static void 
ptrC1 (Packet p)
{
  BenchHeader<25> ipv4;
  p.RemoveHeader (ipv4);
  ptrC2 (p);
}

static void
benchPtrC (uint32_t n)
{
  BenchHeader<25> ipv4;
  BenchHeader<8> udp;

  for (uint32_t i = 0; i < n; i++) {
      Packet p (2000);
      p.AddHeader (udp);
      p.AddHeader (ipv4);
      ptrC1 (p);
  }
}

#if 0
static void
benchPrint (uint32_t n)
{
  PacketPrinter printer;
  BenchHeader<25> ipv4;
  BenchHeader<8> udp;
  Packet p (2000);
  p.AddHeader (udp);
  p.AddHeader (ipv4);

  for (uint32_t i = 0; i < n; i++) 
    {
      p.Print (std::cerr, printer);
    }  
}
#endif


static void
runBench (void (*bench) (uint32_t), uint32_t n, char const *name)
{
  SystemWallClockMs time;
  time.Start ();
  (*bench) (n);
  unsigned long long deltaMs = time.End ();
  double ps = n;
  ps *= 1000;
  ps /= deltaMs;
  std::cout << name<<"=" << ps << " packets/s" << std::endl;
}

int main (int argc, char *argv[])
{
  uint32_t n = 0;
  while (argc > 0) {
      if (strncmp ("--n=", argv[0],strlen ("--n=")) == 0) 
        {
          char const *nAscii = argv[0] + strlen ("--n=");
          n = atoi (nAscii);
        }
      argc--;
      argv++;
  }


  runBench (&benchPtrA, n, "a");
  runBench (&benchPtrB, n, "b");
  runBench (&benchPtrC, n, "c");

  Packet::EnableMetadata ();
  //runBench (&benchPrint, n, "print");
  PacketMetadata::SetOptOne (false);
  runBench (&benchPtrA, n, "meta-a");
  runBench (&benchPtrB, n, "meta-b");
  runBench (&benchPtrC, n, "meta-c");
  PacketMetadata::SetOptOne (true);
  runBench (&benchPtrA, n, "meta-a-opt");
  runBench (&benchPtrB, n, "meta-b-opt");
  runBench (&benchPtrC, n, "meta-c-opt");


  return 0;
}
