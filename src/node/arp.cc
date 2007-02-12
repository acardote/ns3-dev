/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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
#include "ns3/packet.h"
#include "arp.h"
#include "arp-header.h"
#include "arp-cache.h"
#include "net-device.h"
#include "ipv4-interface.h"
#include "node.h"
#include "ipv4.h"

#define TRACE(x)

namespace ns3 {

Arp::Arp (Node *node)
  : m_node (node)
{}

Arp *
Arp::Copy (Node *node)
{
  return new Arp (node);
}

ArpCache *
Arp::FindCache (NetDevice *device)
{
  for (CacheList::const_iterator i = m_cacheList.begin (); i != m_cacheList.end (); i++)
    {
      if ((*i)->GetDevice () == device)
	{
	  return *i;
	}
    }
  Ipv4Interface *interface = m_node->GetIpv4 ()->FindInterfaceForDevice (device);
  ArpCache * cache = new ArpCache (device, interface);
  assert (device->IsBroadcast ());
  device->SetLinkChangeCallback (MakeCallback (&ArpCache::Flush, cache));
  m_cacheList.push_back (cache);
  return cache;
}

void 
Arp::Receive(Packet& packet, NetDevice *device)
{
  ArpCache *cache = FindCache (device);
  ArpHeader arp;
  packet.Peek (arp);
  packet.Remove (arp);
  if (arp.IsRequest () && 
      arp.GetDestinationIpv4Address () == cache->GetInterface ()->GetAddress ()) 
    {
      TRACE ("got request from " << arp.GetSourceIpv4Address () << " -- send reply");
      SendArpReply (cache, arp.GetSourceIpv4Address (),
                    arp.GetSourceHardwareAddress ());
    } 
  else if (arp.IsReply () &&
           arp.GetDestinationIpv4Address ().IsEqual (cache->GetInterface ()->GetAddress ()) &&
           arp.GetDestinationHardwareAddress ().IsEqual (device->GetAddress ())) 
    {
      Ipv4Address from = arp.GetSourceIpv4Address ();
      ArpCache::Entry *entry = cache->Lookup (from);
      if (entry != 0)
        {
          if (entry->IsWaitReply ()) 
            {
              TRACE ("got reply from " << arp.GetSourceIpv4Address ()
                     << " for waiting entry -- flush");
              MacAddress from_mac = arp.GetSourceHardwareAddress ();
              Packet waiting = entry->MarkAlive (from_mac);
	      cache->GetInterface ()->Send (waiting, arp.GetSourceIpv4Address ());
            } 
          else 
            {
              // ignore this reply which might well be an attempt 
              // at poisening my arp cache.
              TRACE ("got reply from " << arp.GetSourceIpv4Address () << 
                     " for non-waiting entry -- drop");
	      // XXX report packet as dropped.
            }
        } 
      else 
        {
          TRACE ("got reply for unknown entry -- drop");
	  // XXX report packet as dropped.
        }
    }
}
bool 
Arp::Lookup (Packet &packet, Ipv4Address destination, 
	     NetDevice *device,
	     MacAddress *hardwareDestination)
{
  ArpCache *cache = FindCache (device);
  ArpCache::Entry *entry = cache->Lookup (destination);
  if (entry != 0)
    {
      if (entry->IsExpired ()) 
        {
          if (entry->IsDead ()) 
            {
              TRACE ("dead entry for " << destination << " expired -- send arp request");
              entry->MarkWaitReply (packet);
              SendArpRequest (cache, destination);
            } 
          else if (entry->IsAlive ()) 
            {
              TRACE ("alive entry for " << destination << " expired -- send arp request");
              entry->MarkWaitReply (packet);
              SendArpRequest (cache, destination);
            } 
          else if (entry->IsWaitReply ()) 
            {
              TRACE ("wait reply for " << destination << " expired -- drop");
              entry->MarkDead ();
	      // XXX report packet as 'dropped'
            }
        } 
      else 
        {
          if (entry->IsDead ()) 
            {
              TRACE ("dead entry for " << destination << " valid -- drop");
	      // XXX report packet as 'dropped'
            } 
          else if (entry->IsAlive ()) 
            {
              TRACE ("alive entry for " << destination << " valid -- send");
	      *hardwareDestination = entry->GetMacAddress ();
              return true;
            } 
          else if (entry->IsWaitReply ()) 
            {
              TRACE ("wait reply for " << destination << " valid -- drop previous");
              Packet old = entry->UpdateWaitReply (packet);
	      // XXX report 'old' packet as 'dropped'
            }
        }

    }
  else
    {
      // This is our first attempt to transmit data to this destination.
      TRACE ("no entry for " << to << " -- send arp request");
      entry = cache->Add (destination);
      entry->MarkWaitReply (packet);
      SendArpRequest (cache, destination);
    }
  return false;
}

void
Arp::SendArpRequest (ArpCache const *cache, Ipv4Address to)
{
  ArpHeader arp;
  arp.SetRequest (cache->GetDevice ()->GetAddress (),
		  cache->GetInterface ()->GetAddress (), 
                  cache->GetDevice ()->GetBroadcast (),
                  to);
  Packet packet;
  packet.Add (arp);
  cache->GetDevice ()->Send (packet, cache->GetDevice ()->GetBroadcast (), 0x0806);
}

void
Arp::SendArpReply (ArpCache const *cache, Ipv4Address toIp, MacAddress toMac)
{
  ArpHeader arp;
  arp.SetReply (cache->GetDevice ()->GetAddress (),
                cache->GetInterface ()->GetAddress (),
                toMac, toIp);
  Packet packet;
  packet.Add (arp);
  cache->GetDevice ()->Send (packet, toMac, 0x0806);
}

}//namespace ns3
