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

#include <utility>
#include <vector>
#include <queue>
#include "ns3/assert.h"
#include "ns3/fatal-error.h"
#include "ns3/debug.h"
#include "ns3/node-list.h"
#include "ns3/ipv4.h"
#include "global-router-interface.h"
#include "global-route-manager-impl.h"
#include "candidate-queue.h"

NS_DEBUG_COMPONENT_DEFINE ("GlobalRouteManager");

namespace ns3 {

// ---------------------------------------------------------------------------
//
// SPFVertex Implementation
//
// ---------------------------------------------------------------------------

SPFVertex::SPFVertex () : 
  m_vertexType (VertexUnknown), 
  m_vertexId ("255.255.255.255"), 
  m_lsa (0),
  m_distanceFromRoot (SPF_INFINITY), 
  m_rootOif (SPF_INFINITY),
  m_nextHop ("0.0.0.0"),
  m_parent (0),
  m_children ()
{
}

SPFVertex::SPFVertex (GlobalRouterLSA* lsa) : 
  m_vertexType (VertexRouter), 
  m_vertexId (lsa->GetLinkStateId ()),
  m_lsa (lsa),
  m_distanceFromRoot (SPF_INFINITY), 
  m_rootOif (SPF_INFINITY),
  m_nextHop ("0.0.0.0"),
  m_parent (0),
  m_children ()
{
}

SPFVertex::~SPFVertex ()
{
  for ( ListOfSPFVertex_t::iterator i = m_children.begin ();
        i != m_children.end ();
        i++)
    {
      SPFVertex *p = *i;
      delete p;
      p = 0;
      *i = 0;
    }
  m_children.clear ();
}

  void 
SPFVertex::SetVertexType (SPFVertex::VertexType type)
{
  m_vertexType = type;
}

  SPFVertex::VertexType 
SPFVertex::GetVertexType (void) const
{
  return m_vertexType;
}

  void 
SPFVertex::SetVertexId (Ipv4Address id)
{
  m_vertexId = id;
}

  Ipv4Address
SPFVertex::GetVertexId (void) const
{
  return m_vertexId;
}

  void 
SPFVertex::SetLSA (GlobalRouterLSA* lsa)
{
  m_lsa = lsa;
}

  GlobalRouterLSA* 
SPFVertex::GetLSA (void) const
{
  return m_lsa;
}

  void 
SPFVertex::SetDistanceFromRoot (uint32_t distance)
{
  m_distanceFromRoot = distance;
}

  uint32_t
SPFVertex::GetDistanceFromRoot (void) const
{
  return m_distanceFromRoot;
}

  void 
SPFVertex::SetOutgoingInterfaceId (uint32_t id)
{
  m_rootOif = id;
}

  uint32_t 
SPFVertex::GetOutgoingInterfaceId (void) const
{
  return m_rootOif;
}

  void 
SPFVertex::SetNextHop (Ipv4Address nextHop)
{
  m_nextHop = nextHop;
}

  Ipv4Address
SPFVertex::GetNextHop (void) const
{
  return m_nextHop;
}

  void
SPFVertex::SetParent (SPFVertex* parent)
{
  m_parent = parent;
}

  SPFVertex* 
SPFVertex::GetParent (void) const
{
  return m_parent;
}

  uint32_t 
SPFVertex::GetNChildren (void) const
{
  return m_children.size ();
}

  SPFVertex* 
SPFVertex::GetChild (uint32_t n) const
{
  uint32_t j = 0;

  for ( ListOfSPFVertex_t::const_iterator i = m_children.begin ();
        i != m_children.end ();
        i++, j++)
    {
      if (j == n)
        {
          return *i;
        }
    }
  NS_ASSERT_MSG(false, "Index <n> out of range.");
  return 0;
}

  uint32_t 
SPFVertex::AddChild (SPFVertex* child)
{
  m_children.push_back (child);
  return m_children.size ();
}

// ---------------------------------------------------------------------------
//
// GlobalRouteManagerLSDB Implementation
//
// ---------------------------------------------------------------------------

GlobalRouteManagerLSDB::GlobalRouteManagerLSDB ()
:
  m_database ()
{
  NS_DEBUG ("GlobalRouteManagerLSDB::GlobalRouteManagerLSDB ()");
}

GlobalRouteManagerLSDB::~GlobalRouteManagerLSDB ()
{
  NS_DEBUG ("GlobalRouteManagerLSDB::~GlobalRouteManagerLSDB ()");

  LSDBMap_t::iterator i;
  for (i= m_database.begin (); i!= m_database.end (); i++)
    {
      NS_DEBUG ("GlobalRouteManagerLSDB::~GlobalRouteManagerLSDB ():free LSA");
      GlobalRouterLSA* temp = i->second;
      delete temp;
    }
  NS_DEBUG ("GlobalRouteManagerLSDB::~GlobalRouteManagerLSDB ():  clear map");
  m_database.clear ();
}

  void
GlobalRouteManagerLSDB::Initialize ()
{
  NS_DEBUG ("GlobalRouteManagerLSDB::Initialize ()");

  LSDBMap_t::iterator i;
  for (i= m_database.begin (); i!= m_database.end (); i++)
    {
      GlobalRouterLSA* temp = i->second;
      temp->SetStatus (GlobalRouterLSA::LSA_SPF_NOT_EXPLORED);
    }
}

  void
GlobalRouteManagerLSDB::Insert (Ipv4Address addr, GlobalRouterLSA* lsa)
{
  NS_DEBUG ("GlobalRouteManagerLSDB::Insert ()");
  m_database.insert (LSDBPair_t (addr, lsa));
}

  GlobalRouterLSA*
GlobalRouteManagerLSDB::GetLSA (Ipv4Address addr) const
{
  NS_DEBUG ("GlobalRouteManagerLSDB::GetLSA ()");
//
// Look up an LSA by its address.
//
  LSDBMap_t::const_iterator i;
  for (i= m_database.begin (); i!= m_database.end (); i++)
  {
    if (i->first == addr)
    {
      return i->second;
    }
  }
  return 0;
}

// ---------------------------------------------------------------------------
//
// GlobalRouteManagerImpl Implementation
//
// ---------------------------------------------------------------------------

GlobalRouteManagerImpl::GlobalRouteManagerImpl () 
: 
  m_spfroot (0) 
{
  NS_DEBUG ("GlobalRouteManagerImpl::GlobalRoutemanagerImpl ()");
  m_lsdb = new GlobalRouteManagerLSDB ();
}

GlobalRouteManagerImpl::~GlobalRouteManagerImpl ()
{
  NS_DEBUG ("GlobalRouteManagerImpl::~GlobalRouteManagerImpl ()");

  if (m_lsdb)
    {
      delete m_lsdb;
    }
}

  void
GlobalRouteManagerImpl::DebugUseLsdb (GlobalRouteManagerLSDB* lsdb)
{
  NS_DEBUG ("GlobalRouteManagerImpl::DebugUseLsdb ()");

  if (m_lsdb)
    {
      delete m_lsdb;
    }
  m_lsdb = lsdb;
}

//
// In order to build the routing database, we need at least one of the nodes
// to participate as a router.  Eventually we expect to provide a mechanism
// for selecting a subset of the nodes to participate; for now, we just make
// all nodes routers.  We do this by walking the list of nodes in the system
// and aggregating a Global Router Interface to each of the nodes.
//
  void
GlobalRouteManagerImpl::SelectRouterNodes () 
{
  NS_DEBUG ("GlobalRouteManagerImpl::SelectRouterNodes ()");

  typedef std::vector < Ptr<Node> >::iterator Iterator;
  for (Iterator i = NodeList::Begin (); i != NodeList::End (); i++)
    {
      Ptr<Node> node = *i;
      NS_DEBUG ("GlobalRouteManagerImpl::SelectRouterNodes (): "
        "Adding GlobalRouter interface to node " <<
                node->GetId ());

      Ptr<GlobalRouter> globalRouter = Create<GlobalRouter> (node);
      node->AddInterface (globalRouter);
    }
}

//
// In order to build the routing database, we need to walk the list of nodes
// in the system and look for those that support the GlobalRouter interface.
// These routers will export a number of Link State Advertisements (LSAs)
// that describe the links and networks that are "adjacent" (i.e., that are
// on the other side of a point-to-point link).  We take these LSAs and put
// add them to the Link State DataBase (LSDB) from which the routes will 
// ultimately be computed.
//
  void
GlobalRouteManagerImpl::BuildGlobalRoutingDatabase () 
{
  NS_DEBUG ("GlobalRouteManagerImpl::BuildGlobalRoutingDatabase()");
//
// Walk the list of nodes looking for the GlobalRouter Interface.
//
  typedef std::vector < Ptr<Node> >::iterator Iterator;
  for (Iterator i = NodeList::Begin (); i != NodeList::End (); i++)
    {
      Ptr<Node> node = *i;

      Ptr<GlobalRouter> rtr = 
        node->QueryInterface<GlobalRouter> (GlobalRouter::iid);
//      
// Ignore nodes that aren't participating in routing.
//
      if (!rtr)
        {
          continue;
        }
//
// You must call DiscoverLSAs () before trying to use any routing info or to
// update LSAs.  DiscoverLSAs () drives the process of discovering routes in
// the GlobalRouter.  Afterward, you may use GetNumLSAs (), which is a very
// computationally inexpensive call.  If you call GetNumLSAs () before calling 
// DiscoverLSAs () will get zero as the number since no routes have been 
// found.
//
      uint32_t numLSAs = rtr->DiscoverLSAs ();
      NS_DEBUG ("Discover LSAs:  Found " << numLSAs << " LSAs");

      for (uint32_t j = 0; j < numLSAs; ++j)
        {
          GlobalRouterLSA* lsa = new GlobalRouterLSA ();
//
// This is the call to actually fetch a Link State Advertisement from the 
// router.
//
          rtr->GetLSA (j, *lsa);
          NS_DEBUG ("LSA " << j);
          NS_DEBUG (*lsa);
//
// Write the newly discovered link state advertisement to the database.
//
          m_lsdb->Insert (lsa->GetLinkStateId (), lsa); 
        }
    }
}

//
// For each node that is a global router (which is determined by the presence
// of an aggregated GlobalRouter interface), run the Dijkstra SPF calculation
// on the database rooted at that router, and populate the node forwarding
// tables.
//
// This function parallels RFC2328, Section 16.1.1, and quagga ospfd
//
// This calculation yields the set of intra-area routes associated
// with an area (called hereafter Area A).  A router calculates the
// shortest-path tree using itself as the root.  The formation
// of the shortest path tree is done here in two stages.  In the
// first stage, only links between routers and transit networks are
// considered.  Using the Dijkstra algorithm, a tree is formed from
// this subset of the link state database.  In the second stage,
// leaves are added to the tree by considering the links to stub
// networks.
//
// The area's link state database is represented as a directed graph.  
// The graph's vertices are routers, transit networks and stub networks.  
//
// The first stage of the procedure (i.e., the Dijkstra algorithm)
// can now be summarized as follows. At each iteration of the
// algorithm, there is a list of candidate vertices.  Paths from
// the root to these vertices have been found, but not necessarily
// the shortest ones.  However, the paths to the candidate vertex
// that is closest to the root are guaranteed to be shortest; this
// vertex is added to the shortest-path tree, removed from the
// candidate list, and its adjacent vertices are examined for
// possible addition to/modification of the candidate list.  The
// algorithm then iterates again.  It terminates when the candidate
// list becomes empty. 
//
  void
GlobalRouteManagerImpl::InitializeRoutes ()
{
  NS_DEBUG ("GlobalRouteManagerImpl::InitializeRoutes ()");
//
// Walk the list of nodes in the system.
//
  typedef std::vector < Ptr<Node> >::iterator Iterator;
  for (Iterator i = NodeList::Begin (); i != NodeList::End (); i++)
    {
      Ptr<Node> node = *i;
//
// Look for the GlobalRouter interface that indicates that the node is
// participating in routing.
//
      Ptr<GlobalRouter> rtr = 
        node->QueryInterface<GlobalRouter> (GlobalRouter::iid);
//
// if the node has a global router interface, then run the global routing
// algorithms.
//
      if (rtr && rtr->GetNumLSAs () )
        {
          SPFCalculate (rtr->GetRouterId ());
        }
    }
}

//
// This method is derived from quagga ospf_spf_next ().  See RFC2328 Section 
// 16.1 (2) for further details.
//
// We're passed a parameter <v> that is a vertex which is already in the SPF
// tree.  A vertex represents a router node.  We also get a reference to the
// SPF candidate queue, which is a priority queue containing the shortest paths
// to the networks we know about.
//
// We examine the links in v's LSA and update the list of candidates with any
// vertices not already on the list.  If a lower-cost path is found to a
// vertex already on the candidate list, store the new (lower) cost.
//
  void
GlobalRouteManagerImpl::SPFNext (SPFVertex* v, CandidateQueue& candidate)
{
  SPFVertex* w = 0;
  GlobalRouterLSA* w_lsa = 0;
  uint32_t distance = 0;

  NS_DEBUG ("GlobalRouteManagerImpl::SPFNext ()");
//
// Always true for now, since all our LSAs are RouterLSAs.
//
  if (v->GetVertexType () == SPFVertex::VertexRouter) 
    {
      if (true)
        {
          NS_DEBUG ("SPFNext: Examining " << v->GetVertexId () << "'s " <<
            v->GetLSA ()->GetNLinkRecords () << " link records");
//
// Walk the list of link records in the link state advertisement associated 
// with the "current" router (represented by vertex <v>).
//
          for (uint32_t i = 0; i < v->GetLSA ()->GetNLinkRecords (); ++i)
            {
//
// (a) If this is a link to a stub network, examine the next link in V's LSA.
// Links to stub networks will be considered in the second stage of the
// shortest path calculation.
//
              GlobalRouterLinkRecord *l = v->GetLSA ()->GetLinkRecord (i);
              if (l->GetLinkType () == GlobalRouterLinkRecord::StubNetwork)
                {
                  NS_DEBUG ("SPFNext: Found a Stub record to " << 
                    l->GetLinkId ());
                  continue;
                }
//
// (b) Otherwise, W is a transit vertex (router or transit network).  Look up
// the vertex W's LSA (router-LSA or network-LSA) in Area A's link state
// database. 
//
              if (l->GetLinkType () == GlobalRouterLinkRecord::PointToPoint)
                {
//
// Lookup the link state advertisement of the new link -- we call it <w> in
// the link state database.
//
                  w_lsa = m_lsdb->GetLSA (l->GetLinkId ());
                  NS_ASSERT (w_lsa);
                  NS_DEBUG ("SPFNext:  Found a P2P record from " << 
                    v->GetVertexId () << " to " << w_lsa->GetLinkStateId ());
//
// (c) If vertex W is already on the shortest-path tree, examine the next
// link in the LSA.
//
// If the link is to a router that is already in the shortest path first tree
// then we have it covered -- ignore it.
//
                  if (w_lsa->GetStatus () == 
                      GlobalRouterLSA::LSA_SPF_IN_SPFTREE) 
                    {
                      NS_DEBUG ("SPFNext: Skipping->  LSA "<< 
                        w_lsa->GetLinkStateId () << " already in SPF tree");
                      continue;
                    }
//
// The link is to a router we haven't dealt with yet.
//
// (d) Calculate the link state cost D of the resulting path from the root to 
// vertex W.  D is equal to the sum of the link state cost of the (already 
// calculated) shortest path to vertex V and the advertised cost of the link
// between vertices V and W.  
//
                  distance = v->GetDistanceFromRoot () + l->GetMetric ();

                  NS_DEBUG ("SPFNext: Considering w_lsa " << 
                    w_lsa->GetLinkStateId ());

                  if (w_lsa->GetStatus () == 
                      GlobalRouterLSA::LSA_SPF_NOT_EXPLORED)
                    {
//
// If we haven't yet considered the link represented by <w> we have to create 
// a new SPFVertex to represent it.
//
                        w = new SPFVertex (w_lsa);
//
// We need to figure out how to actually get to the new router represented
// by <w>.  This will (among other things) find the next hop address to send
// packets destined for this network to, and also find the outbound interface
// used to forward the packets.
//
                      if (SPFNexthopCalculation (v, w, l, distance))
                        {
                          w_lsa->SetStatus (
                            GlobalRouterLSA::LSA_SPF_CANDIDATE);
//
// Push this new vertex onto the priority queue (ordered by distance from the
// root node).
//
                          candidate.Push (w);
                          NS_DEBUG ("SPFNext:  Pushing " << 
                            w->GetVertexId () << ", parent vertexId: " << 
                                    v->GetVertexId ());
                        }
                    }
                } else if (w_lsa->GetStatus () == 
                           GlobalRouterLSA::LSA_SPF_CANDIDATE)
                    {
//
// We have already considered the link represented by <w>.  What wse have to
// do now is to decide if this new router represents a route with a shorter
// distance metric.
//
// So, locate the vertex in the candidate queue and take a look at the 
// distance.
                      w = candidate.Find (w_lsa->GetLinkStateId ());
                      if (w->GetDistanceFromRoot () < distance)
                        {
//
// This is not a shorter path, so don't do anything.
//
                          continue;
                        }
                      else if (w->GetDistanceFromRoot () == distance)
                        {
//
// This path is one with an equal cost.  Do nothing for now -- we're not doing
// equal-cost multipath cases yet.
//
                        }
                      else
                        {
// 
// this path represents a new, lower-cost path to <w> (the vertex we found in
// the current link record of the link state advertisement of the current root
// (vertex <v>)
//
// N.B. the nexthop_calculation is conditional, if it finds a valid nexthop
// it will call spf_add_parents, which will flush the old parents
//
                          if (SPFNexthopCalculation (v, w, l, distance))
                            {
//
// If we've changed the cost to get to the vertex represented by <w>, we 
// must reorder the priority queue keyed to that cost.
//
                              candidate.Reorder ();
                            }
                        }    
                    }  // point-to-point
            } // for loop
        } 
    }
}

//
// This method is derived from quagga ospf_next_hop_calculation() 16.1.1.  
//
// Calculate the next hop IP address and the outgoing interface required to
// get packets from the root through <v> (parent) to vertex <w> (destination),
// over a given distance.
//
// For now, this is greatly simplified from the quagga code
//                  
  int
GlobalRouteManagerImpl::SPFNexthopCalculation (
  SPFVertex* v, 
  SPFVertex* w,
  GlobalRouterLinkRecord* l,
  uint32_t distance)
{
  NS_DEBUG ("GlobalRouteManagerImpl::SPFNexthopCalculation ()");
//
// The vertex m_spfroot is a distinguished vertex representing the node at
// the root of the calculations.  That is, it is the node for which we are
// calculating the routes.
//
// There are two distinct cases for calculating the next hop information.
// First, if we're considering a hop from the root to an "adjacent" network
// (one that is on the other side of a point-to-point link connected to the
// root), then we need to store the information needed to forward down that
// link.  The second case is if the network is not directly adjacent.  In that
// case we need to use the forwarding information from the vertex on the path
// to the destination that is directly adjacent [node 1] in both cases of the
// diagram below.
// 
// (1) [root] -> [point-to-point] -> [node 1]
// (2) [root] -> [point-to-point] -> [node 1] -> [point-to-point] -> [node 2]
//
// We call the propagation of next hop information down vertices of a path
// "inheriting" the next hop information.
//
// The point-to-point link information is only useful in this calculation when
// we are examining the root node. 
//
  if (v == m_spfroot)
    {
//
// In this case <v> is the root node, which means it is the starting point
// for the packets forwarded by that node.  This also means that the next hop
// address of packets headed for some arbitrary off-network destination must
// be the destination at the other end of one of the links off of the root
// node if this root node is a router.  We then need to see if this node <w>
// is a router.
//
      if (w->GetVertexType () == SPFVertex::VertexRouter) 
        {
//
// In the case of point-to-point links, the link data field (m_linkData) of a
// Global Router Link Record contains the local IP address.  If we look at the
// link record describing the link from the perspecive of <w> (the remote
// node from the viewpoint of <v>) back to the root node, we can discover the
// IP address of the router to which <v> is adjacent.  This is a distinguished
// address -- the next hop address to get from <v> to <w> and all networks 
// accessed through that path.
//
// SPFGetNextLink () is a little odd.  used in this way it is just going to
// return the link record describing the link from <w> to <v>.  Think of it as
// SPFGetLink.
//
          GlobalRouterLinkRecord *linkRemote = 0;
          linkRemote = SPFGetNextLink (w, v, linkRemote);
// 
// At this point, <l> is the Global Router Link Record describing the point-
// to point link from <v> to <w> from the perspective of <v>; and <linkRemote>
// is the Global Router Link Record describing that same link from the 
// perspective of <w> (back to <v>).  Now we can just copy the next hop 
// address from the m_linkData member variable.
// 
// The next hop member variable we put in <w> has the sense "in order to get
// from the root node to the host represented by vertex <w>, you have to send
// the packet to the next hop address specified in w->m_nextHop.
//
          w->SetNextHop(linkRemote->GetLinkData ());
// 
// Now find the outgoing interface corresponding to the point to point link
// from the perspective of <v> -- remember that <l> is the link "from"
// <v> "to" <w>.
//
          w->SetOutgoingInterfaceId (
            FindOutgoingInterfaceId (l->GetLinkData ()));

          NS_DEBUG ("SPFNexthopCalculation: Next hop from " << 
            v->GetVertexId () << " to " << w->GetVertexId () << 
            " goes through next hop " << w->GetNextHop () <<
            " via outgoing interface " << w->GetOutgoingInterfaceId ());
        }
    }
  else 
    {
//
// If we're calculating the next hop information from a node (v) that is 
// *not* the root, then we need to "inherit" the information needed to
// forward the packet from the vertex closer to the root.  That is, we'll
// still send packets to the next hop address of the router adjacent to the
// root on the path toward <w>.
//
// Above, when we were considering the root node, we calculated the next hop
// address and outgoing interface required to get off of the root network.  
// At this point, we are further away from the root network along one of the
// (shortest) paths.  So the next hop and outoing interface remain the same
// (are inherited).
//
      w->SetNextHop (v->GetNextHop ());
      w->SetOutgoingInterfaceId (v->GetOutgoingInterfaceId ());
    }
//
// In all cases, we need valid values for the distance metric and a parent.
//
  w->SetDistanceFromRoot (distance);
  w->SetParent (v);

  return 1;
}

//
// This method is derived from quagga ospf_get_next_link ()
//
// First search the Global Router Link Records of vertex <v> for one
// representing a point-to point link to vertex <w>.
//
// What is done depends on prev_link.  Contrary to appearances, prev_link just
// acts as a flag here.  If prev_link is NULL, we return the first Global
// Router Link Record we find that describes a point-to-point link from <v> 
// to <w>.  If prev_link is not NULL, we return a Global Router Link Record
// representing a possible *second* link from <v> to <w>.
//
// BUGBUG FIXME:  This seems to be a bug.  Shouldn't this function look for
// any link records after pre_link and not just after the first?
//
  GlobalRouterLinkRecord* 
GlobalRouteManagerImpl::SPFGetNextLink (
  SPFVertex* v,
  SPFVertex* w,
  GlobalRouterLinkRecord* prev_link) 
{
  NS_DEBUG ("GlobalRouteManagerImpl::SPFGetNextLink ()");

  bool skip = true;
  GlobalRouterLinkRecord* l;
//
// If prev_link is 0, we are really looking for the first link, not the next 
// link.
//
  if (prev_link == 0)
    {
      skip = false;
    }
//  
// Iterate through the Global Router Link Records advertised by the vertex
// <v> looking for records representing the point-to-point links off of this
// vertex.
//
  for (uint32_t i = 0; i < v->GetLSA ()->GetNLinkRecords (); ++i)
    {
      l = v->GetLSA ()->GetLinkRecord (i);
      if (l->GetLinkType () != GlobalRouterLinkRecord::PointToPoint)
        {
          continue;
        }
//
// The link ID of a link record representing a point-to-point link is set to
// the router ID of the neighboring router -- the router to which the link
// connects from the perspective of <v> in this case.  The vertex ID is also
// set to the router ID (using the link state advertisement of a router node).
// We're just checking to see if the link <l> is actually the link from <v> to
// <w>.
//
      if (l->GetLinkId () == w->GetVertexId ()) {
        NS_DEBUG ("SPFGetNextLink: Found matching link l:  linkId = " <<
          l->GetLinkId () << " linkData = " << l->GetLinkData ());
//
// If skip is false, don't (not too surprisingly) skip the link found -- it's 
// the one we're interested in.  That's either because we didn't pass in a 
// previous link, and we're interested in the first one, or because we've 
// skipped a previous link and moved forward to the next (which is then the
// one we want).
//
        if (skip == false) 
          {
            NS_DEBUG ("SPFGetNextLink: Returning the found link");
            return l;
          }
        else
          {
//
// Skip is true and we've found a link from <v> to <w>.  We want the next one.
// Setting skip to false gets us the next point-to-point global router link
// record in the LSA from <v>.
//
            NS_DEBUG ("SPFGetNextLink: Skipping the found link");
            skip = false;
            continue;
          }
      }
    }
  return 0;
}
  
//
// Used for unit tests.
//
  void
GlobalRouteManagerImpl::DebugSPFCalculate (Ipv4Address root)
{
  NS_DEBUG ("GlobalRouteManagerImpl::DebugSPFCalculate ()");
  SPFCalculate (root);
}

// quagga ospf_spf_calculate
  void
GlobalRouteManagerImpl::SPFCalculate (Ipv4Address root)
{
  NS_DEBUG ("GlobalRouteManagerImpl::SPFCalculate (): "
    "root = " << root);

  SPFVertex *v;
//
// Initialize the Link State Database.
//
  m_lsdb->Initialize ();
//
// The candidate queue is a priority queue of SPFVertex objects, with the top
// of the queue being the closest vertex in terms of distance from the root
// of the tree.  Initially, this queue is empty.
//
  CandidateQueue candidate;
  NS_ASSERT(candidate.Size () == 0);
//
// Initialize the shortest-path tree to only contain the router doing the 
// calculation.  Each router (and corresponding network) is a vertex in the
// shortest path first (SPF) tree.
//
  v = new SPFVertex (m_lsdb->GetLSA (root));
// 
// This vertex is the root of the SPF tree and it is distance 0 from the root.
// We also mark this vertex as being in the SPF tree.
//
  m_spfroot= v;
  v->SetDistanceFromRoot (0);
  v->GetLSA ()->SetStatus (GlobalRouterLSA::LSA_SPF_IN_SPFTREE);

  for (;;)
    {
//
// The operations we need to do are given in the OSPF RFC which we reference
// as we go along.
//
// RFC2328 16.1. (2). 
//
// We examine the Global Router Link Records in the Link State 
// Advertisements of the current vertex.  If there are any point-to-point
// links to unexplored adjacent vertices we add them to the tree and update
// the distance and next hop information on how to get there.  We also add
// the new vertices to the candidate queue (the priority queue ordered by
// shortest path).  If the new vertices represent shorter paths, we use them
// and update the path cost.
//
      SPFNext (v, candidate);
//
// RFC2328 16.1. (3). 
//
// If at this step the candidate list is empty, the shortest-path tree (of
// transit vertices) has been completely built and this stage of the
// procedure terminates. 
//
      if (candidate.Size () == 0)
        {
          break;
        }
//
// Choose the vertex belonging to the candidate list that is closest to the
// root, and add it to the shortest-path tree (removing it from the candidate
// list in the process).
//
// Recall that in the previous step, we created SPFVertex structures for each
// of the routers found in the Global Router Link Records and added tehm to 
// the candidate list.
//
      v = candidate.Pop ();
      NS_DEBUG ("SPFCalculate: Popped vertex " << v->GetVertexId ());
//
// Update the status field of the vertex to indicate that it is in the SPF
// tree.
//
      v->GetLSA ()->SetStatus (GlobalRouterLSA::LSA_SPF_IN_SPFTREE);
//
// The current vertex has a parent pointer.  By calling this rather oddly 
// named method (blame quagga) we add the current vertex to the list of 
// children of that parent vertex.  In the next hop calculation called during
// SPFNext, the parent pointer was set but the vertex has been orphaned up
// to now.
//
      SPFVertexAddParent (v);
//
// Note that when there is a choice of vertices closest to the root, network
// vertices must be chosen before router vertices in order to necessarily
// find all equal-cost paths. We don't do this at this moment, we should add
// the treatment above codes. -- kunihiro. 
//
// RFC2328 16.1. (4). 
//
// This is the method that actually adds the routes.  It'll walk the list
// of nodes in the system, looking for the node corresponding to the router
// ID of the root of the tree -- that is the router we're building the routes
// for.  It looks for the Ipv4 interface of that node and remembers it.  So
// we are only actually adding routes to that one node at the root of the SPF 
// tree.
//
// We're going to pop of a pointer to every vertex in the tree except the 
// root in order of distance from the root.  For each of the vertices, we call
// SPFIntraAddRouter ().  Down in SPFIntraAddRouter, we look at all of the 
// point-to-point Global Router Link Records (the links to nodes adjacent to
// the node represented by the vertex).  We add a route to the IP address 
// specified by the m_linkData field of each of those link records.  This will
// be the *local* IP address associated with the interface attached to the 
// link.  We use the outbound interface and next hop information present in 
// the vertex <v> which have possibly been inherited from the root.
//
// To summarize, we're going to look at the node represented by <v> and loop
// through its point-to-point links, adding a *host* route to the local IP
// address (at the <v> side) for each of those links.
//
      SPFIntraAddRouter (v);
//
// RFC2328 16.1. (5). 
//
// Iterate the algorithm by returning to Step 2 until there are no more
// candidate vertices.
//
    }
//
// Second stage of SPF calculation procedure's  
// NOTYET:  ospf_spf_process_stubs (area, area->spf, new_table);
//
// We're all done setting the routing information for the node at the root of
// the SPF tree.  Delete all of the vertices and corresponding resources.  Go
// possibly do it again for the next router.
//
  delete m_spfroot;
  m_spfroot = 0;
}

//
// XXX This should probably be a method on Ipv4
//
// Return the interface index corresponding to a given IP address
//
  uint32_t
GlobalRouteManagerImpl::FindOutgoingInterfaceId (Ipv4Address a)
{
//
// We have an IP address <a> and a vertex ID of the root of the SPF tree.  
// The question is what interface index does this address correspond to.
// The answer is a little complicated since we have to find a pointer to
// the node corresponding to the vertex ID, find the Ipv4 interface on that
// node in order to iterate the interfaces and find the one corresponding to
// the address in question.
//
  Ipv4Address routerId = m_spfroot->GetVertexId ();
//
// Walk the list of nodes in the system looking for the one corresponding to
// the node at the root of the SPF tree.  This is the node for which we are
// building the routing table.
//
  std::vector<Ptr<Node> >::iterator i = NodeList::Begin (); 
  for (; i != NodeList::End (); i++)
    {
      Ptr<Node> node = *i;

      Ptr<GlobalRouter> rtr = 
        node->QueryInterface<GlobalRouter> (GlobalRouter::iid);
//
// If the node doesn't have a GlobalRouter interface it can't be the one
// we're interested in.
//
      if (rtr == 0)
        {
          continue;
        }

      if (rtr->GetRouterId () == routerId)
        {
//
// This is the node we're building the routing table for.  We're going to need
// the Ipv4 interface to look for the ipv4 interface index.  Since this node
// is participating in routing IP version 4 packets, it certainly must have 
// an Ipv4 interface.
//
          Ptr<Ipv4> ipv4 = node->QueryInterface<Ipv4> (Ipv4::iid);
          NS_ASSERT_MSG (ipv4, 
            "GlobalRouteManagerImpl::FindOutgoingInterfaceId (): "
            "QI for <Ipv4> interface failed");
//
// Look through the interfaces on this node for one that has the IP address
// we're looking for.  If we find one, return the corresponding interface
// index.
//
          for (uint32_t i = 0; i < ipv4->GetNInterfaces (); i++)
            {
              if (ipv4->GetAddress (i) == a)
                {
                  NS_DEBUG (
                    "GlobalRouteManagerImpl::FindOutgoingInterfaceId (): "
                    "Interface match for " << a);
                  return i;
                }
            }
        }
    }
//
// Couldn't find it.
//
  return 0;
}

//
// This method is derived from quagga ospf_intra_add_router ()
//
// This is where we are actually going to add the host routes to the routing
// tables of the individual nodes.
//
// The vertex passed as a parameter has just been added to the SPF tree.
// This vertex must have a valid m_root_oid, corresponding to the outgoing
// interface on the root router of the tree that is the first hop on the path
// to the vertex.  The vertex must also have a next hop address, corresponding
// to the next hop on the path to the vertex.  The vertex has an m_lsa field
// that has some number of link records.  For each point to point link record,
// the m_linkData is the local IP address of the link.  This corresponds to
// a destination IP address, reachable from the root, to which we add a host
// route.
//
  void
GlobalRouteManagerImpl::SPFIntraAddRouter (SPFVertex* v)
{
  NS_DEBUG ("GlobalRouteManagerImpl::SPFIntraAddRouter ()");

  NS_ASSERT_MSG (m_spfroot, 
    "GlobalRouteManagerImpl::SPFIntraAddRouter (): Root pointer not set");
//
// The root of the Shortest Path First tree is the router to which we are 
// going to write the actual routing table entries.  The vertex corresponding
// to this router has a vertex ID which is the router ID of that node.  We're
// going to use this ID to discover which node it is that we're actually going
// to update.
//
  Ipv4Address routerId = m_spfroot->GetVertexId ();

  NS_DEBUG ("GlobalRouteManagerImpl::SPFIntraAddRouter (): "
    "Vertex ID = " << routerId);
//
// We need to walk the list of nodes looking for the one that has the router
// ID corresponding to the root vertex.  This is the one we're going to write
// the routing information to.
//
  std::vector<Ptr<Node> >::iterator i = NodeList::Begin (); 
  for (; i != NodeList::End (); i++)
    {
      Ptr<Node> node = *i;
//
// The router ID is accessible through the GlobalRouter interface, so we need
// to QI for that interface.  If there's no GlobalRouter interface, the node
// in question cannot be the router we want, so we continue.
// 
      Ptr<GlobalRouter> rtr = 
        node->QueryInterface<GlobalRouter> (GlobalRouter::iid);

      if (rtr == 0)
        {
          NS_DEBUG ("GlobalRouteManagerImpl::SPFIntraAddRouter (): "
            "No GlobalRouter interface on node " << node->GetId ());
          continue;
        }
//
// If the router ID of the current node is equal to the router ID of the 
// root of the SPF tree, then this node is the one for which we need to 
// write the routing tables.
//
      NS_DEBUG ("GlobalRouteManagerImpl::SPFIntraAddRouter (): "
        "Considering router " << rtr->GetRouterId ());

      if (rtr->GetRouterId () == routerId)
        {
          NS_DEBUG ("GlobalRouteManagerImpl::SPFIntraAddRouter (): "
            "setting routes for node " << node->GetId ());
//
// Routing information is updated using the Ipv4 interface.  We need to QI
// for that interface.  If the node is acting as an IP version 4 router, it
// should absolutely have an Ipv4 interface.
//
          Ptr<Ipv4> ipv4 = node->QueryInterface<Ipv4> (Ipv4::iid);
          NS_ASSERT_MSG (ipv4, 
            "GlobalRouteManagerImpl::SPFIntraAddRouter (): "
            "QI for <Ipv4> interface failed");
//
// Get the Global Router Link State Advertisement from the vertex we're
// adding the routes to.  The LSA will have a number of attached Global Router
// Link Records corresponding to links off of that vertex / node.  We're going
// to be interested in the records corresponding to point-to-point links.
//
          GlobalRouterLSA *lsa = v->GetLSA ();
          NS_ASSERT_MSG (lsa, 
            "GlobalRouteManagerImpl::SPFIntraAddRouter (): "
            "Expected valid LSA in SPFVertex* v");

          uint32_t nLinkRecords = lsa->GetNLinkRecords ();
//
// Iterate through the link records on the vertex to which we're going to add
// routes.  To make sure we're being clear, we're going to add routing table
// entries to the tables on the node corresping to the root of the SPF tree.
// These entries will have routes to the IP addresses we find from looking at
// the local side of the point-to-point links found on the node described by
// the vertex <v>.
//
          for (uint32_t j = 0; j < nLinkRecords; j += 2)
            {
//
// We are only concerned about point-to-point links
//
              GlobalRouterLinkRecord *lr = lsa->GetLinkRecord (j);
              if (lr->GetLinkType () != GlobalRouterLinkRecord::PointToPoint)
                {
                  continue;
                }

              NS_DEBUG ("GlobalRouteManagerImpl::SPFIntraAddRouter (): "
                " Node " << node->GetId () <<
                " add route to " << lr->GetLinkData () <<
                " using next hop " << v->GetNextHop () <<
                " via interface " << v->GetOutgoingInterfaceId ());
//
// Here's why we did all of that work.  We're going to add a host route to the
// host address found in the m_linkData field of the point-to-point link
// record.  In the case of a point-to-point link, this is the local IP address
// of the node connected to the link.  Each of these point-to-point links
// will correspond to a local interface that has an IP address to which
// the node at the root of the SPF tree can send packets.  The vertex <v> 
// (corresponding to the node that has these links and interfaces) has 
// an m_nextHop address precalculated for us that is the address to which the
// root node should send packets to be forwarded to these IP addresses.
// Similarly, the vertex <v> has an m_rootOif (outbound interface index) to
// which the packets should be send for forwarding.
//
              ipv4->AddHostRouteTo (lr->GetLinkData (), v->GetNextHop (),
                v->GetOutgoingInterfaceId ());
            }
//
// Done adding the routes for the selected node.
//
          return;
        }
    }
}

// Derived from quagga ospf_vertex_add_parents ()
//
// This is a somewhat oddly named method (blame quagga).  Although you might
// expect it to add a parent *to* something, it actually adds a vertex
// to the list of children *in* each of its parents. 
//
// Given a pointer to a vertex, it links back to the vertex's parent that it
// already has set and adds itself to that vertex's list of children.
//
// For now, only one parent (not doing equal-cost multipath)
//
  void
GlobalRouteManagerImpl::SPFVertexAddParent (SPFVertex* v)
{
  v->GetParent ()->AddChild (v);
}

} // namespace ns3

#ifdef RUN_SELF_TESTS

// ---------------------------------------------------------------------------
//
// Unit Tests
//
// ---------------------------------------------------------------------------

#include "ns3/test.h"
#include "ns3/simulator.h"

namespace ns3 {

class GlobalRouterTestNode : public Node
{
public:
  GlobalRouterTestNode ();

private:
  virtual void DoAddDevice (Ptr<NetDevice> device) const {};
  virtual TraceResolver *DoCreateTraceResolver (TraceContext const &context);
};

GlobalRouterTestNode::GlobalRouterTestNode ()
{
//  Ptr<Ipv4L3Protocol> ipv4 = Create<Ipv4L3Protocol> (this);
}

  TraceResolver*
GlobalRouterTestNode::DoCreateTraceResolver (TraceContext const &context)
{
  return 0;
}

class GlobalRouteManagerImplTest : public Test {
public:
  GlobalRouteManagerImplTest ();
  virtual ~GlobalRouteManagerImplTest ();
  virtual bool RunTests (void);
};

GlobalRouteManagerImplTest::GlobalRouteManagerImplTest ()
  : Test ("GlobalRouteManagerImpl")
{
}

GlobalRouteManagerImplTest::~GlobalRouteManagerImplTest ()
{}

  bool
GlobalRouteManagerImplTest::RunTests (void)
{
  bool ok = true;

  CandidateQueue candidate;

  for (int i = 0; i < 100; ++i)
    {
      SPFVertex *v = new SPFVertex;
      v->SetDistanceFromRoot (rand () % 100);
      candidate.Push (v);
    }

  uint32_t lastDistance = 0;

  for (int i = 0; i < 100; ++i)
    {
      SPFVertex *v = candidate.Pop ();
      if (v->GetDistanceFromRoot () < lastDistance)
        {
          ok = false;
        }
      lastDistance = v->GetDistanceFromRoot ();
      delete v;
      v = 0;
    }

  // Build fake link state database; four routers (0-3), 3 point-to-point
  // links
  //
  //   n0
  //      \ link 0
  //       \          link 2
  //        n2 -------------------------n3
  //       /
  //      / link 1
  //    n1
  //
  //  link0:  10.1.1.1/30, 10.1.1.2/30
  //  link1:  10.1.2.1/30, 10.1.2.2/30
  //  link2:  10.1.3.1/30, 10.1.3.2/30
  //
  // Router 0
  GlobalRouterLinkRecord* lr0 = new GlobalRouterLinkRecord (
    GlobalRouterLinkRecord::PointToPoint, 
    "0.0.0.2",  // router ID 0.0.0.2
    "10.1.1.1", // local ID
    1);         // metric

  GlobalRouterLinkRecord* lr1 = new GlobalRouterLinkRecord (
    GlobalRouterLinkRecord::StubNetwork,
    "10.1.1.1",
    "255.255.255.252",
    1);

  GlobalRouterLSA* lsa0 = new GlobalRouterLSA ();
  lsa0->SetLinkStateId ("0.0.0.0");
  lsa0->SetAdvertisingRouter ("0.0.0.0");
  lsa0->AddLinkRecord (lr0);
  lsa0->AddLinkRecord (lr1);

  // Router 1
  GlobalRouterLinkRecord* lr2 = new GlobalRouterLinkRecord (
    GlobalRouterLinkRecord::PointToPoint,
    "0.0.0.2",
    "10.1.2.1",
    1);

  GlobalRouterLinkRecord* lr3 = new GlobalRouterLinkRecord (
    GlobalRouterLinkRecord::StubNetwork,
    "10.1.2.1",
    "255.255.255.252",
    1);

  GlobalRouterLSA* lsa1 = new GlobalRouterLSA ();
  lsa1->SetLinkStateId ("0.0.0.1");
  lsa1->SetAdvertisingRouter ("0.0.0.1");
  lsa1->AddLinkRecord (lr2);
  lsa1->AddLinkRecord (lr3);
  
  // Router 2 
  GlobalRouterLinkRecord* lr4 = new GlobalRouterLinkRecord (
    GlobalRouterLinkRecord::PointToPoint,
    "0.0.0.0",
    "10.1.1.2",
    1);

  GlobalRouterLinkRecord* lr5 = new GlobalRouterLinkRecord (
    GlobalRouterLinkRecord::StubNetwork,
    "10.1.1.2",
    "255.255.255.252",
    1);

  GlobalRouterLinkRecord* lr6 = new GlobalRouterLinkRecord (
    GlobalRouterLinkRecord::PointToPoint,
    "0.0.0.1",
    "10.1.2.2",
    1);

  GlobalRouterLinkRecord* lr7 = new GlobalRouterLinkRecord (
    GlobalRouterLinkRecord::StubNetwork,
    "10.1.2.2",
    "255.255.255.252",
    1);

  GlobalRouterLinkRecord* lr8 = new GlobalRouterLinkRecord (
    GlobalRouterLinkRecord::PointToPoint,
    "0.0.0.3",
    "10.1.3.2",
    1);

  GlobalRouterLinkRecord* lr9 = new GlobalRouterLinkRecord (
    GlobalRouterLinkRecord::StubNetwork,
    "10.1.3.2",
    "255.255.255.252",
    1);

  GlobalRouterLSA* lsa2 = new GlobalRouterLSA ();
  lsa2->SetLinkStateId ("0.0.0.2");
  lsa2->SetAdvertisingRouter ("0.0.0.2");
  lsa2->AddLinkRecord (lr4);
  lsa2->AddLinkRecord (lr5);
  lsa2->AddLinkRecord (lr6);
  lsa2->AddLinkRecord (lr7);
  lsa2->AddLinkRecord (lr8);
  lsa2->AddLinkRecord (lr9);

  // Router 3
  GlobalRouterLinkRecord* lr10 = new GlobalRouterLinkRecord (
    GlobalRouterLinkRecord::PointToPoint,
    "0.0.0.2",
    "10.1.2.1",
    1);

  GlobalRouterLinkRecord* lr11 = new GlobalRouterLinkRecord (
    GlobalRouterLinkRecord::StubNetwork,
    "10.1.2.1",
    "255.255.255.252",
    1);

  GlobalRouterLSA* lsa3 = new GlobalRouterLSA ();
  lsa3->SetLinkStateId ("0.0.0.3");
  lsa3->SetAdvertisingRouter ("0.0.0.3");
  lsa3->AddLinkRecord (lr10);
  lsa3->AddLinkRecord (lr11);

  // Test the database 
  GlobalRouteManagerLSDB* srmlsdb = new GlobalRouteManagerLSDB ();
  srmlsdb->Insert (lsa0->GetLinkStateId (), lsa0);
  srmlsdb->Insert (lsa1->GetLinkStateId (), lsa1);
  srmlsdb->Insert (lsa2->GetLinkStateId (), lsa2);
  srmlsdb->Insert (lsa3->GetLinkStateId (), lsa3);
  NS_ASSERT (lsa2 == srmlsdb->GetLSA (lsa2->GetLinkStateId ()));

  // next, calculate routes based on the manually created LSDB
  GlobalRouteManagerImpl* srm = new GlobalRouteManagerImpl ();
  srm->DebugUseLsdb (srmlsdb);  // manually add in an LSDB
  // Note-- this will succeed without any nodes in the topology
  // because the NodeList is empty
  srm->DebugSPFCalculate (lsa0->GetLinkStateId ());  // node n0

  Simulator::Run ();
  Simulator::Destroy ();

  // This delete clears the srm, which deletes the LSDB, which clears 
  // all of the LSAs, which each destroys the attached LinkRecords.
  delete srm;

  return ok;
}

// Instantiate this class for the unit tests
// XXX here we should do some verification of the routes built
static GlobalRouteManagerImplTest g_globalRouteManagerTest;

} // namespace ns3

#endif 
