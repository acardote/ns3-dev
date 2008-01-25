/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 Georgia Tech Research Corporation
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
 * Author: Raj Bhattacharjea <raj.b@gatech.edu>
 */
#ifndef TCP_H
#define TCP_H

#include "ns3/default-value.h"
#include "socket-factory.h"

namespace ns3 {

class Socket;

/**
 * \brief API to create TCP socket instances 
 *
 * This abstract class defines the API for TCP sockets.
 * This class also holds the global default variables used to
 * initialize newly created sockets, such as values that are
 * set through the sysctl or proc interfaces in Linux.

 * All TCP implementations must provide an implementation of CreateSocket
 * below, and should make use of the default values configured below.
 * 
 * \see TcpImpl
 *
 */
class Tcp : public SocketFactory
{
public:
  static const InterfaceId iid;

  Tcp ();

  virtual Ptr<Socket> CreateSocket (void) = 0;

public:
  static NumericDefaultValue<uint32_t>       defaultSegSize;  
  static NumericDefaultValue<uint32_t>       defaultAdvWin;   
  static NumericDefaultValue<uint32_t>       defaultSSThresh; 
  static NumericDefaultValue<uint32_t>       defaultTxBuffer; 
  static NumericDefaultValue<uint32_t>       defaultRxBuffer; 
  static NumericDefaultValue<uint32_t>       defaultInitialCWnd; 
  static NumericDefaultValue<uint32_t>       defaultConnTimeout;  
  static NumericDefaultValue<uint32_t>       defaultConnCount;  

};

} // namespace ns3

#endif /* UDP_H */
