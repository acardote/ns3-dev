/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INRIA
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
#ifndef EUI48_ADDRESS_H
#define EUI48_ADDRESS_H

#include <stdint.h>
#include <ostream>

namespace ns3 {

class Address;

/**
 * \brief an EUI-48 address
 *
 * This class can contain 48 bit IEEE addresses.
 */
class Eui48Address
{
public:
  Eui48Address ();
  /**
   * \param str a string representing the new Eui48Address
   *
   * The format of the string is "xx:xx:xx:xx:xx:xx"
   */
  Eui48Address (const char *str);

  /**
   * \param buffer address in network order
   *
   * Copy the input address to our internal buffer.
   */
  void CopyFrom (const uint8_t buffer[6]);
  /**
   * \param buffer address in network order
   *
   * Copy the internal address to the input buffer.
   */
  void CopyTo (uint8_t buffer[6]) const;
  /**
   * \returns a new Address instance
   *
   * Convert an instance of this class to a polymorphic Address instance.
   */
  Address ConvertTo (void) const;
  /**
   * \returns true if the address matches, false otherwise.
   */
  static bool IsMatchingType (const Address &address);
  /**
   * \param address a polymorphic address
   *
   * Convert a polymorphic address to an Eui48Address instance.
   * The conversion performs a type check.
   */
  static Eui48Address ConvertFrom (const Address &address);
  /**
   * Allocate a new Eui48Address.
   */
  static Eui48Address Allocate (void);
private:
  static uint8_t GetType (void);
  uint8_t m_address[6];
};

bool operator == (const Eui48Address &a, const Eui48Address &b);
bool operator != (const Eui48Address &a, const Eui48Address &b);
std::ostream& operator<< (std::ostream& os, const Eui48Address & address);

} // namespace ns3

#endif /* EUI48_ADDRESS_H */
