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
#include <cassert>
#include <list>
#include <utility>
#include <iostream>
#include "debug.h"
#include "ns3/core-config.h"

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

namespace ns3 {

typedef std::list<std::pair <std::string, DebugComponent *> > ComponentList;
typedef std::list<std::pair <std::string, DebugComponent *> >::iterator ComponentListI;

static ComponentList g_components;

DebugComponent::DebugComponent (std::string name)
  : m_isEnabled (false)
{
  for (ComponentListI i = g_components.begin ();
       i != g_components.end ();
       i++)
    {
      assert (i->first.compare (name) != 0);
    }
  g_components.push_back (std::make_pair (name, this));
}
bool 
DebugComponent::IsEnabled (void)
{
  return m_isEnabled;
}
void 
DebugComponent::Enable (void)
{
  m_isEnabled = true;
}
void 
DebugComponent::Disable (void)
{
  m_isEnabled = false;
}

void 
DebugComponentEnable (char const *name)
{
  for (ComponentListI i = g_components.begin ();
       i != g_components.end ();
       i++)
    {
      if (i->first.compare (name) == 0) 
	{
	  i->second->Enable ();
	  break;
	}
    }  
}
void 
DebugComponentDisable (char const *name)
{
  for (ComponentListI i = g_components.begin ();
       i != g_components.end ();
       i++)
    {
      if (i->first.compare (name) == 0) 
	{
	  i->second->Disable ();
	  break;
	}
    }  
}

void
DebugComponentEnableEnvVar (void)
{
#ifdef HAVE_GETENV
  char *envVar = getenv("NS3_DEBUG");
  if (envVar == 0)
    {
      return;
    }
  std::string env = envVar;
  std::string::size_type cur = 0;
  std::string::size_type next = 0;
  while (true)
    {
      next = env.find_first_of (";", cur);
      if (next == std::string::npos) 
	{
	  std::string tmp = env.substr (cur, next);
	  DebugComponentEnable (tmp.c_str ());
	}
      cur = next;
    }
#endif
}

void 
DebugComponentPrintList (void)
{
  for (ComponentListI i = g_components.begin ();
       i != g_components.end ();
       i++)
    {
      std::cout << i->first << "=" << (i->second->IsEnabled ()?"enabled":"disabled") << std::endl;
    }
}



}; // namespace ns3
