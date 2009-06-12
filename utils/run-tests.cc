/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005 INRIA
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

#include "ns3/test.h"
#include "ns3/packet-metadata.h"
#include "ns3/random-variable.h"


int main (int argc, char *argv[])
{
  if (argc > 1)
    {
      if (std::string (argv[1]) == "--ListTests")
        {
#ifdef RUN_SELF_TESTS
          ns3::TestManager::PrintTestNames (std::cout);
#endif
        }
      else
        {
          // run the test named by argv[1]
#ifdef RUN_SELF_TESTS
          bool success = ns3::TestManager::RunTest (argv[1]);
          if (!success)
            {
              return 1;
            }
#else
          std::cerr << "Unit tests not enabled" << std::endl;
          return 1;
#endif
        }      
    }
  else
    {
      // run all tests
#ifdef RUN_SELF_TESTS
      ns3::PacketMetadata::Enable ();
      ns3::TestManager::EnableVerbose ();
      bool success = ns3::TestManager::RunTests ();
      if (!success)
        {
          return 1;
        }
#endif /* RUN_SELF_TESTS */
    }
  return 0;
}

