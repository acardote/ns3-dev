/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */
#ifndef STATIC_ROUTER_H
#define STATIC_ROUTER_H

#include <stdint.h>
#include <list>
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/node.h"
#include "ns3/channel.h"
#include "ns3/ipv4-address.h"
#include "ns3/routing-environment.h"

namespace ns3 {

/**
 * \brief A single link record for a link state advertisement.
 *
 * The StaticRouterLinkRecord is modeled after the OSPF link record field of
 * a Link State Advertisement.  Right now we will only see two types of link
 * records corresponding to a stub network and a point-to-point link (channel).
 *
 * For Type 3 link (Stub), 
 */
class StaticRouterLinkRecord
{
public:
  //
  // For Type 1 link (PointToPoint), set m_linkId to Router ID of 
  // neighboring router.
  // 
  // For Type 3 link (Stub), set m_linkId to neighbor's IP address
  //  
  Ipv4Address m_linkId;         

  //
  // For Type 1 link (PointToPoint), set m_linkData to local IP address  
  // 
  // For Type 3 link (Stub), set m_linkData to mask 0xffffffff
  //
  Ipv4Address m_linkData;    // for links to RouterLSA, 
  /**
   * Enumeration of the possible types of Static Router Link Records.  These
   * are defined in the OSPF spec.  We currently only use PointToPoint and
   * StubNetwork types.
   */
  enum LinkType {
    PointToPoint = 1,   /**< Record representing a point to point channel */
    TransitNetwork,     /**< Unused -- for future OSPF compatibility  */
    StubNetwork,        /**< Record represents a leaf node network */
    VirtualLink         /**< Unused -- for future OSPF compatibility  */
  } m_linkType;
  /**
   * An abstract cost associated with forwarding a packet across a link.  
   * A sum of metrics must have a well-defined meaning.  That is, you shouldn't
   * use bandwidth as a metric (how does the sum of the bandwidth of two hops
   * relate to the cost of sending a packet); rather you should use something
   * like delay.
   */
  uint32_t m_metric;  
};

/**  
 * \brief a Link State Advertisement (LSA) for a router, used in static routing
 * 
 * Roughly equivalent to a static incarnation of the OSPF link state header
 * combined with a list of Link Records.  Since it's static, there's
 * no need for age or sequence number.  See RFC 2328, Appendix A.
 */
class StaticRouterLSA
{
public:
  /**
   * Create a blank Static Router Link State Advertisement.  On completion,
   * any Ipv4Address variables initialized to 0.0.0.0 and the list of Link 
   * State Records is empty.
   */
  StaticRouterLSA();
  /**
   * Copy constructor for a Static Router Link State Advertisement.
   * Takes a piece of memory and constructs a semantically identical copy of
   * the given LSA.
   *
   * @param lsa The existing LSA to be used as the source.
   */
  StaticRouterLSA (StaticRouterLSA& lsa);
  /**
   * Destroy an existing Static Router Link State Advertisement.  Any Static
   * router Link Records present in the list are freed.
   */
  ~StaticRouterLSA();
  /**
   * Assignment operator for a Static Router Link State Advertisement.
   * Takes an existing Static Router Link State Advertisement and overwrites
   * it to make a semantically identical copy of a given prototype LSA.
   *
   * If there are any Static Router Link Records present in the existing 
   * LSA, they are freed before the assignment happens.
   *
   * @param lsa The existing LSA to be used as the source.
   * @returns Reference to the overwritten LSA.
   */
  StaticRouterLSA& operator= (StaticRouterLSA& lsa);
  /**
   * Add a given Static Router Link Record to a given Static Router Link
   * State Advertisement.
   *
   * @param lr The Static Router Link Record to be added.
   * @returns The number of link records in the list.
   */
  uint32_t AddLinkRecord (StaticRouterLinkRecord* lr);
  /**
   * Release all of the Static Router Link Records present in the Static
   * Router Link State Advertisement and make the list of link records empty.
   */
  void ClearLinkRecords(void);
  /**
   * Check to see of the list of Static Router Link Records present in the
   * Static Router Link State Advertisement is empty.
   *
   * @returns True if the list is empty, false otherwise.
   */
  bool IsEmpty(void);
  /**
   * Print the contents of the Static Router Link State Advertisement and
   * any Static Router Link Records present in the list.  Quite verbose.
   */
  void Print (std::ostream &os);
  /**
   * The Link State ID is defined by the OSPF spec.  We always set it to the
   * router ID of the router making the advertisement.
   *
   * @see RoutingEnvironment::AllocateRouterId ()
   * @see StaticRouter::GetRouterId ()
   */
  Ipv4Address  m_linkStateId;
  /**
   * The Advertising Router is defined by the OSPF spec.  We always set it to 
   * the router ID of the router making the advertisement.
   *
   * @see RoutingEnvironment::AllocateRouterId ()
   * @see StaticRouter::GetRouterId ()
   */
  Ipv4Address  m_advertisingRtr;

  typedef std::list<StaticRouterLinkRecord*> ListOfLinkRecords_t;
  ListOfLinkRecords_t m_linkRecords;
};

std::ostream& operator<< (std::ostream& os, StaticRouterLSA& lsa);

/**
 * \brief An interface aggregated to a node to provide static routing info
 *
 * An interface aggregated to a node that provides static routing information
 * to a global route manager.  The presence of the interface indicates that
 * the node is a router.  The interface is the mechanism by which the router
 * advertises its connections to neighboring routers.  We're basically 
 * allowing the route manager to query for link state advertisements.
 */
class StaticRouter : public Object
{
public:
  /**
   * The Interface ID of the Static Router interface.
   *
   * @see Object::QueryInterface ()
   */
  static const InterfaceId iid;
  /**
   * Create a Static Router class and aggregate its interface onto the Node
   * provided.
   *
   * @param lsa The existing Node onto which this router will be aggregated.
   */
  StaticRouter (Ptr<Node> node);
  /**
   * Get the Router ID associated with this Static Router.  The Router IDs
   * are allocated in the RoutingEnvironment -- one per Router, starting at
   * 0.0.0.1 and incrementing with each instantiation of a router.
   *
   * @see RoutingEnvironment::AllocateRouterId ()
   * @returns The Router ID associated with the Static Router.
   */
  Ipv4Address GetRouterId(void);
  /**
   * Get the Number of Static Router Link State Advertisements that this
   * router can export.
   *
   * This is a fairly expensive operation in that every time it is called
   * the current list of LSAs is built by walking connected point-to-point
   * channels and peeking into adjacent IPV4 stacks to get address information.
   * This is done to allow for limited dymanics of the Static Routing 
   * environment.  By that we mean that you can discover new link state 
   * advertisements after a network topology change by calling GetNumLSAs and
   * then by reading those advertisements.
   *
   * @see StaticRouterLSA
   * @see StaticRouter::GetLSA ()
   * @returns The number of Static Router Link State Advertisements.
   */
  uint32_t GetNumLSAs (void);
  /**
   * Get a Static Router Link State Advertisements that this router has said
   * that it can export.
   *
   * This is a fairly inexpensive expensive operation in that the hard work
   * was done in GetNumLSAs.  We just copy the indicated Static Router Link
   * State Advertisement into the requested StaticRouterLSA object.
   *
   * You must call StaticRouter::GetNumLSAs before calling this method in 
   * order to discover the adjacent routers and build the advertisements.
   * GetNumLSAs will return the number of LSAs this router advertises.  
   * The parameter n (requested LSA number) must be in the range 0 to 
   * GetNumLSAs() - 1.
   *
   * @see StaticRouterLSA
   * @see StaticRouter::GetNumLSAs ()
   * @param n The index number of the LSA you want to read.
   * @param lsa The StaticRouterLSA class to receive the LSA information.
   * @returns The number of Static Router Link State Advertisements.
   */
  bool GetLSA (uint32_t n, StaticRouterLSA &lsa);

protected:
  virtual ~StaticRouter ();
  void ClearLSAs (void);

  Ptr<NetDevice> GetAdjacent(Ptr<NetDevice> nd, Ptr<Channel> ch);
  uint32_t FindIfIndexForDevice(Ptr<Node> node, Ptr<NetDevice> nd);

  Ptr<Node>     m_node;

  typedef std::list<StaticRouterLSA*> ListOfLSAs_t;
  ListOfLSAs_t m_LSAs;

  Ipv4Address m_routerId;

private:
};

} // namespace ns3

#endif /* STATIC_ROUTER_H */
