#include <iostream>
#include "ns3/object.h"
#include "ns3/pointer.h"
#include "ns3/object-vector.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/helper-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Main");

void
PrintAttributes (TypeId tid, std::ostream &os)
{
  os << "<ul>"<<std::endl;
  for (uint32_t j = 0; j < tid.GetAttributeN (); j++)
    {
      os << "<li><b>" << tid.GetAttributeName (j) << "</b>: "
		<< tid.GetAttributeHelp (j) << std::endl;
      Ptr<const AttributeChecker> checker = tid.GetAttributeChecker (j);
      os << "  <ul>" << std::endl << "    <li>Type: \\ref " <<  checker->GetType ();
      if (checker->HasTypeConstraints ())
	{
	  os << " -> " << checker->GetTypeConstraints ();
	}
      os << "</li>" << std::endl;
      uint32_t flags = tid.GetAttributeFlags (j);
      os << "    <li>Flags: ";
      if (flags & TypeId::ATTR_SET)
	{
	  os << "write ";
	}
      if (flags & TypeId::ATTR_GET)
	{
	  os << "read ";
	}
      if (flags & TypeId::ATTR_CONSTRUCT)
	{
	  os << "construct ";
	}
      os << "</li>" << std::endl;
      os << "  </ul> " << std::endl;
      
    }
  os << "</ul>" << std::endl;
}

class StaticInformation
{
public:
  void RecordAggregationInfo (std::string a, std::string b);
  void Gather (TypeId tid);
  void Print (void) const;

  std::vector<std::string> Get (TypeId tid);

private:
  std::string GetCurrentPath (void) const;
  void DoGather (TypeId tid);
  void RecordOutput (TypeId tid);
  bool HasAlreadyBeenProcessed (TypeId tid) const;
  std::vector<std::pair<TypeId,std::string> > m_output;
  std::vector<std::string> m_currentPath;
  std::vector<TypeId> m_alreadyProcessed;
  std::vector<std::pair<TypeId,TypeId> > m_aggregates;
};

void 
StaticInformation::RecordAggregationInfo (std::string a, std::string b)
{
  m_aggregates.push_back (std::make_pair (TypeId::LookupByName (a), TypeId::LookupByName (b)));
}

void 
StaticInformation::Print (void) const
{
  for (std::vector<std::pair<TypeId,std::string> >::const_iterator i = m_output.begin (); i != m_output.end (); ++i)
    {
      std::pair<TypeId,std::string> item = *i;
      std::cout << item.first.GetName () << " -> " << item.second << std::endl;
    }
}

std::string
StaticInformation::GetCurrentPath (void) const
{
  std::ostringstream oss;
  for (std::vector<std::string>::const_iterator i = m_currentPath.begin (); i != m_currentPath.end (); ++i)
    {
      std::string item = *i;
      oss << "/" << item;
    }
  return oss.str ();
}

void
StaticInformation::RecordOutput (TypeId tid)
{
  m_output.push_back (std::make_pair (tid, GetCurrentPath ()));
}

bool
StaticInformation::HasAlreadyBeenProcessed (TypeId tid) const
{
  for (uint32_t i = 0; i < m_alreadyProcessed.size (); ++i)
    {
      if (m_alreadyProcessed[i] == tid)
	{
	  return true;
	}
    }
  return false;
}

std::vector<std::string> 
StaticInformation::Get (TypeId tid)
{
  std::vector<std::string> paths;
  for (uint32_t i = 0; i < m_output.size (); ++i)
    {
      std::pair<TypeId,std::string> tmp = m_output[i];
      if (tmp.first == tid)
	{
	  paths.push_back (tmp.second);
	}
    }
  return paths;
}

void
StaticInformation::Gather (TypeId tid)
{
  DoGather (tid);

  std::sort (m_output.begin (), m_output.end ());
  m_output.erase (std::unique (m_output.begin (), m_output.end ()), m_output.end ());
}

void 
StaticInformation::DoGather (TypeId tid)
{
  NS_LOG_FUNCTION;
  if (HasAlreadyBeenProcessed (tid))
    {
      return;
    }
  RecordOutput (tid);
  for (uint32_t i = 0; i < tid.GetAttributeN (); ++i)
    {
      Ptr<const AttributeChecker> checker = tid.GetAttributeChecker (i);
      const PointerChecker *ptrChecker = dynamic_cast<const PointerChecker *> (PeekPointer (checker));
      if (ptrChecker != 0)
	{
	  TypeId pointee = ptrChecker->GetPointeeTypeId ();
	  m_currentPath.push_back (tid.GetAttributeName (i));
	  m_alreadyProcessed.push_back (tid);
	  DoGather (pointee);
	  m_alreadyProcessed.pop_back ();
	  m_currentPath.pop_back ();
	  continue;
	}
      // attempt to cast to an object vector.
      const ObjectVectorChecker *vectorChecker = dynamic_cast<const ObjectVectorChecker *> (PeekPointer (checker));
      if (vectorChecker != 0)
	{
	  TypeId item = vectorChecker->GetItemTypeId ();
	  m_currentPath.push_back (tid.GetAttributeName (i) + "/[i]");
	  m_alreadyProcessed.push_back (tid);
	  DoGather (item);
	  m_alreadyProcessed.pop_back ();
	  m_currentPath.pop_back ();
	  continue;
	}
    }
  for (uint32_t j = 0; j < TypeId::GetRegisteredN (); j++)
    {
      TypeId child = TypeId::GetRegistered (j);
      if (child.IsChildOf (tid))
	{
	  m_currentPath.push_back ("$%" + child.GetName ());
	  m_alreadyProcessed.push_back (tid);
	  DoGather (child);
	  m_alreadyProcessed.pop_back ();
	  m_currentPath.pop_back ();
	}
    }
  for (uint32_t k = 0; k < m_aggregates.size (); ++k)
    {
      std::pair<TypeId,TypeId> tmp = m_aggregates[k];
      if (tmp.first == tid || tmp.second == tid)
	{
	  TypeId other;
	  if (tmp.first == tid)
	    {
	      other = tmp.second;
	    }
	  if (tmp.second == tid)
	    {
	      other = tmp.first;
	    }
	  // Note: we insert a % in the path below to ensure that doxygen does not
	  // attempt to resolve the typeid names included in the string.
	  m_currentPath.push_back ("$%" + other.GetName ());
	  m_alreadyProcessed.push_back (tid);
	  DoGather (other);
	  m_alreadyProcessed.pop_back ();
	  m_currentPath.pop_back ();	  
	}
    }
}

int main (int argc, char *argv[])
{
  NodeContainer c; c.Create (1);

  StaticInformation info;
  info.RecordAggregationInfo ("ns3::Node", "ns3::Tcp");
  info.RecordAggregationInfo ("ns3::Node", "ns3::Udp");
  info.RecordAggregationInfo ("ns3::Node", "ns3::PacketSocketFactory");
  info.RecordAggregationInfo ("ns3::Node", "ns3::olsr::Agent");
  info.RecordAggregationInfo ("ns3::Node", "ns3::MobilityModel");
  info.RecordAggregationInfo ("ns3::Node", "ns3::Ipv4L4Demux");
  info.RecordAggregationInfo ("ns3::Node", "ns3::Ipv4L3Protocol");
  info.RecordAggregationInfo ("ns3::Node", "ns3::ArpL3Protocol");

  for (uint32_t i = 0; i < Config::GetRootNamespaceObjectN (); ++i)
    {
      Ptr<Object> object = Config::GetRootNamespaceObject (i);
      info.Gather (object->GetInstanceTypeId ());
    }

  for (uint32_t i = 0; i < TypeId::GetRegisteredN (); i++)
    {
      std::cout << "/*!" << std::endl;
      TypeId tid = TypeId::GetRegistered (i);
      if (tid.MustHideFromDocumentation ())
	{
	  continue;
	}
      std::cout << "\\fn static TypeId " << tid.GetName () << "::GetTypeId (void)" << std::endl;
      std::cout << "\\brief This method returns the TypeId associated to \\ref " << tid.GetName () 
		<< std::endl << std::endl;
      std::vector<std::string> paths = info.Get (tid);
      if (!paths.empty ())
	{
	  std::cout << "This object is accessible through the following paths with Config::Set and Config::Connect:" 
		    << std::endl;
	  std::cout << "<ul>" << std::endl;
	  for (uint32_t k = 0; k < paths.size (); ++k)
	    {
	      std::string path = paths[k];
	      std::cout << "<li>" << path << "</li>" << std::endl;
	    }
	  std::cout << "</ul>" << std::endl;
	}
      if (tid.GetAttributeN () == 0)
	{
	  std::cout << "No Attributes defined for this type." << std::endl;
	}
      else
	{
	  std::cout << "Attributes defined for this type:" << std::endl;
	  PrintAttributes (tid, std::cout);
	}
      bool hasAttributesInParent = false;
      TypeId tmp = tid.GetParent ();
      while (tmp.GetParent () != tmp)
	{
	  if (tmp.GetAttributeN () != 0)
	    {
	      hasAttributesInParent = true;
	    }
	  tmp = tmp.GetParent ();
	}
      if (hasAttributesInParent)
	{
	  tmp = tid.GetParent ();
	  while (tmp.GetParent () != tmp)
	    {
	      if (tmp.GetAttributeN () != 0)
		{
		  std::cout << "Attributes defined in parent class " << tmp.GetName () << ":<br>" << std::endl;
		  PrintAttributes (tmp, std::cout);
		}
	      tmp = tmp.GetParent ();
	    }
	}
      std::cout << "*/" << std::endl;
    }


  return 0;
}
