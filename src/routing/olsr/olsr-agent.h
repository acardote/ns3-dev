/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INESC Porto
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
 * Author: Gustavo J. A. M. Carneiro  <gjc@inescporto.pt>
 */

#ifndef OLSR_AGENT_H
#define OLSR_AGENT_H

#include "ns3/node.h"
#include "ns3/component-manager.h"

namespace ns3 {


class OlsrAgent : public Object
{
public:
  static const InterfaceId iid;
  static const ClassId cid;

  virtual void Start () = 0;
  virtual void SetMainInterface (uint32_t interface) = 0;
};


}; // namespace ns3

#endif /* OLSR_AGENT_H */

