/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "ns3/ptr.h"
#include "ns3/packet.h"
#include "ns3/header.h"
#include <iostream>

using namespace ns3;

/* A sample Header implementation
 */
class MyHeader : public Header 
{
public:
  static uint32_t GetUid (void);

  MyHeader ();
  virtual ~MyHeader ();

  void SetData (uint16_t data);
  uint16_t GetData (void) const;

  std::string GetName (void) const;
  void Print (std::ostream &os) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  uint32_t GetSerializedSize (void) const;
private:
  uint16_t m_data;
};

MyHeader::MyHeader ()
{
  // we must provide a public default constructor, 
  // implicit or explicit, but never private.
}
MyHeader::~MyHeader ()
{}

uint32_t
MyHeader::GetUid (void)
{
  // This string is used by the internals of the packet
  // code to keep track of the packet metadata.
  // You need to make sure that this string is absolutely
  // unique. The code will detect any duplicate string.
  static uint32_t uid = AllocateUid<MyHeader> ("MyHeader.test.nsnam.org");
  return uid;
}

std::string 
MyHeader::GetName (void) const
{
  // This string is used to identify the type of 
  // my header by the packet printing routines.
  return "MYHEADER";
}
void 
MyHeader::Print (std::ostream &os) const
{
  // This method is invoked by the packet printing
  // routines to print the content of my header.
  os << "data=" << m_data << std::endl;
}
uint32_t
MyHeader::GetSerializedSize (void) const
{
  // we reserve 2 bytes for our header.
  return 2;
}
void
MyHeader::Serialize (Buffer::Iterator start) const
{
  // we can serialize two bytes at the start of the buffer.
  // we write them in network byte order.
  start.WriteHtonU16 (m_data);
}
uint32_t
MyHeader::Deserialize (Buffer::Iterator start)
{
  // we can deserialize two bytes from the start of the buffer.
  // we read them in network byte order and store them
  // in host byte order.
  m_data = start.ReadNtohU16 ();

  // we return the number of bytes effectively read.
  return 2;
}

void 
MyHeader::SetData (uint16_t data)
{
  m_data = data;
}
uint16_t 
MyHeader::GetData (void) const
{
  return m_data;
}



int main (int argc, char *argv[])
{
  // instantiate a header.
  MyHeader sourceHeader;
  sourceHeader.SetData (2);

  // instantiate a packet
  Ptr<Packet> p = Create<Packet> ();

  // and store my header into the packet.
  p->AddHeader (sourceHeader);

  // print the content of my packet on the standard output.
  p->Print (std::cout);

  // you can now remove the header from the packet:
  MyHeader destinationHeader;
  p->RemoveHeader (destinationHeader);

  // and check that the destination and source
  // headers contain the same values.
  NS_ASSERT (sourceHeader.GetData () == destinationHeader.GetData ());

  return 0;
}
