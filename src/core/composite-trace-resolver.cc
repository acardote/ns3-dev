/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INRIA
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
#include "composite-trace-resolver.h"
#include "log.h"

NS_LOG_COMPONENT_DEFINE ("CompositeTraceResolver");

namespace ns3 {

CompositeTraceResolver::CompositeTraceResolver ()
{}

CompositeTraceResolver::~CompositeTraceResolver ()
{
  for (TraceItems::iterator i = m_items.begin (); i != m_items.end (); i++)
    {
      delete *i;
    }
  m_items.clear ();
}

void 
CompositeTraceResolver::AddItem (ResolveItem *item)
{
  m_items.push_back (item);
}

void 
CompositeTraceResolver::Add (std::string name, 
                             Callback<Ptr<TraceResolver> > createResolver)
{
  class MakerResolveItem : public ResolveItem
  {
  public:
    virtual void Connect (std::string subpath, const CallbackBase &cb, const TraceContext &context)
    {maker ()->Connect (subpath, cb, context);}
    virtual void Disconnect (std::string subpath, const CallbackBase &cb)
    {maker ()->Disconnect (subpath, cb);}
    virtual void CollectSources (std::string path, const TraceContext &context, 
                                 SourceCollection *collection)
    {
      path.append ("/");
      path.append (this->name);
      TraceContext ctx = context;
      ctx.Union (this->context);
      this->maker ()->CollectSources (path, ctx, collection);
    }
    virtual void TraceAll (std::ostream &os, const TraceContext &context)
    {
      TraceContext ctx = context;
      ctx.Union (this->context);
      this->maker ()->TraceAll (os, ctx);
    }
    Callback<Ptr<TraceResolver> > maker;
  } *item = new MakerResolveItem ();
  item->name = name;
  item->context = TraceContext ();
  item->maker = createResolver;
  AddItem (item);
}

void 
CompositeTraceResolver::AddSource (std::string name,
                                   const TraceDoc &doc,
                                   const TraceSource &trace)
{
  DoAddSource (name, doc, trace, TraceContext ());
}

void 
CompositeTraceResolver::DoAddSource (std::string name,
                                     const TraceDoc &doc,
                                     const TraceSource &trace, 
                                     const TraceContext &context)
{
  class SourceResolveItem : public ResolveItem
  {
  public:
    virtual void Connect (std::string subpath, const CallbackBase &cb, const TraceContext &context)
    {if (subpath == "") {trace->AddCallback (cb, context);}}
    virtual void Disconnect (std::string subpath, const CallbackBase &cb)
    {if (subpath == "") {trace->RemoveCallback (cb);}}
    virtual void CollectSources (std::string path, const TraceContext &context, 
                                 SourceCollection *collection)
    {
      path.append ("/");
      path.append (this->name);
      TraceContext ctx = context;
      ctx.Union (this->context);
      collection->AddUnique (path, ctx, this->doc);
    }
    virtual void TraceAll (std::ostream &os, const TraceContext &context)
    {
      TraceContext ctx = context;
      ctx.Union (this->context);
      this->trace->ConnectPrinter (os, ctx);
    }
    TraceSource *trace;
    TraceDoc doc;
  } *item = new SourceResolveItem ();
  item->name = name;
  item->context = context;
  item->trace = const_cast<TraceSource *> (&trace);
  item->doc = doc;
  AddItem (item);
}




void 
CompositeTraceResolver::AddComposite (std::string name, Ptr<Object> composite)
{
  DoAddComposite (name, composite, TraceContext ());
}
void 
CompositeTraceResolver::DoAddComposite (std::string name, Ptr<Object> composite, const TraceContext &context)
{
  class CompositeResolveItem : public ResolveItem
  {
  public:
    virtual void Connect (std::string subpath, const CallbackBase &cb, const TraceContext &context)
    {composite->GetTraceResolver ()->Connect (subpath, cb, context);}
    virtual void Disconnect (std::string subpath, const CallbackBase &cb)
    {composite->TraceDisconnect (subpath, cb);}
    virtual void CollectSources (std::string path, const TraceContext &context, 
                                 SourceCollection *collection)
    {
      path.append ("/");
      path.append (this->name);
      TraceContext ctx = context;
      ctx.Union (this->context);
      this->composite->GetTraceResolver ()->CollectSources (path, ctx, collection);
    }
    virtual void TraceAll (std::ostream &os, const TraceContext &context)
    {
      TraceContext ctx = context;
      ctx.Union (this->context);
      this->composite->GetTraceResolver ()->TraceAll (os, ctx);
    }

    Ptr<Object> composite;
  } *item = new CompositeResolveItem ();
  item->name = name;
  item->context = context;
  item->composite = composite;
  AddItem (item);
}

void
CompositeTraceResolver::SetParentResolver (Ptr<TraceResolver> resolver)
{
  m_parent = resolver;
}

void 
CompositeTraceResolver::Connect (std::string path, CallbackBase const &cb, const TraceContext &context)
{
  NS_LOG_LOGIC ("connect path="<<path);
  class ConnectOperation : public Operation
  {
  public:
    ConnectOperation (const CallbackBase &cb, const TraceContext &context)
      : m_cb (cb), m_context (context)
    {}
    virtual void Do (std::string subpath, ResolveItem *item) const
    {
      NS_LOG_LOGIC ("connect to path="<<subpath<<" name="<<item->name);
      TraceContext context = m_context;
      context.Union (item->context);
      item->Connect (subpath, m_cb, context);
    }
    virtual void DoParent (std::string path, Ptr<TraceResolver> parent) const
    {
      if (parent != 0)
        {
          parent->Connect (path, m_cb, m_context);
        }
    }
  private:
    const CallbackBase &m_cb;
    const TraceContext &m_context;
  } operation  = ConnectOperation (cb, context);
  DoRecursiveOperation (path, operation);
}
void 
CompositeTraceResolver::DoRecursiveOperation (std::string path, 
                                              const Operation &operation)
{
  if (path == "")
    {
      return;
    }
  std::string id = GetElement (path);
  std::string subpath = GetSubpath (path);

  if (id == "*")
    {
      for (TraceItems::const_iterator i = m_items.begin (); i != m_items.end (); i++)
	{
          operation.Do (subpath, *i);
        }
      operation.DoParent (path, m_parent);
      return;
    }
  std::string::size_type start, end;
  start = id.find_first_of ("(", 0);
  end = id.find_first_of (")", 0);
  if (start != 0 || end != (id.size ()-1))
    {
      for (TraceItems::const_iterator i = m_items.begin (); i != m_items.end (); i++)
	{
	  if ((*i)->name == id)
	    {
              operation.Do (subpath, *i);
              operation.DoParent (path, m_parent);
              return;
	    }
	}
    }
  std::list<std::string> names;
  std::string alternatives = std::string (id, start+1, end-1);
  std::string::size_type next_pos, cur_pos;
  next_pos = 0;
  cur_pos = 0;
  while (true)
    {
      std::string element;
      next_pos = alternatives.find ("|", cur_pos);
      if (next_pos == std::string::npos)
	{
	  element = std::string (alternatives, cur_pos, alternatives.size ());
	  names.push_back (element);
	  break;
	}
      element = std::string (alternatives, cur_pos, next_pos);
      names.push_back (element);
      cur_pos = next_pos + 1;
    }
  for (std::list<std::string>::const_iterator i = names.begin (); i != names.end (); i++)
    {
      for (TraceItems::const_iterator j = m_items.begin (); j != m_items.end (); j++)
	{
	  if ((*j)->name == *i)
	    {
              operation.Do (subpath, *j);
	      break;
	    }
	}
    }
  operation.DoParent (path, m_parent);
}

void 
CompositeTraceResolver::Disconnect (std::string path, CallbackBase const &cb)
{
  NS_LOG_LOGIC ("disconnect path="<<path);
  class DisconnectOperation : public Operation
  {
  public:
    DisconnectOperation (const CallbackBase &cb)
      : m_cb (cb)
    {}
    virtual void Do (std::string subpath, ResolveItem *item) const
    {
      NS_LOG_LOGIC ("disconnect from path="<<subpath<<" name="<<item->name);
      item->Disconnect (subpath, m_cb);
    }
    virtual void DoParent (std::string path, Ptr<TraceResolver> parent) const
    {
      if (parent != 0)
        {
          parent->Disconnect (path, m_cb);
        }
    }
  private:
    const CallbackBase &m_cb;
  } operation  = DisconnectOperation (cb);
  DoRecursiveOperation (path, operation);
}

void 
CompositeTraceResolver::CollectSources (std::string path, const TraceContext &context, 
                                        SourceCollection *collection)
{
  for (TraceItems::const_iterator i = m_items.begin (); i != m_items.end (); i++)
    {
      NS_LOG_LOGIC ("print " << (*i)->name);
      (*i)->CollectSources (path, context, collection);
    }
  if (m_parent != 0)
    {
      m_parent->CollectSources (path, context, collection);
    }
}
void 
CompositeTraceResolver::TraceAll (std::ostream &os, const TraceContext &context)
{
  for (TraceItems::const_iterator i = m_items.begin (); i != m_items.end (); i++)
    {
      NS_LOG_LOGIC ("print " << (*i)->name);
      (*i)->TraceAll (os, context);
    }
  if (m_parent != 0)
    {
      m_parent->TraceAll (os, context);
    }  
}


}//namespace ns3

#ifdef RUN_SELF_TESTS

#include "test.h"
#include "trace-context-element.h"

namespace ns3 {

class TraceSourceTest : public TraceContextElement
{
public:
  enum Sources {
    DOUBLEA,
    DOUBLEB,
    UINT16_T
  };
  static uint16_t GetUid (void) 
  {static uint16_t uid = AllocateUid<TraceSourceTest> ("TraceSourceTest"); return uid;}
  void Print (std::ostream &os)
  {os << "tracesource=";
    if (m_sources == DOUBLEA) {os << "doubleA";}
    else if (m_sources == DOUBLEB) {os << "doubleB";}
    else if (m_sources == UINT16_T) {os << "uint16_t";}
  }
  std::string GetTypeName (void) {return "ns3::TraceSourceTest";}
  TraceSourceTest () : m_sources (TraceSourceTest::DOUBLEA) {}
  TraceSourceTest (enum Sources sources) :m_sources (sources) {}
  bool IsDoubleA (void) const {return m_sources == TraceSourceTest::DOUBLEA;}
  bool IsDoubleB (void) const {return m_sources == TraceSourceTest::DOUBLEB;}
  bool IsUint16 (void) const {return m_sources == TraceSourceTest::UINT16_T;}
private:
  enum TraceSourceTest::Sources m_sources;
};

class SubTraceSourceTest : public TraceContextElement
{
public:
  enum Sources {
    INT,
  };
  static uint16_t GetUid (void) 
  {static uint16_t uid = AllocateUid<SubTraceSourceTest> ("SubTraceSourceTest"); return uid;}
  void Print (std::ostream &os)
  {os << "subtracesource=int";}
  std::string GetTypeName (void) const {return "ns3::SubTraceSourceTest";}
  SubTraceSourceTest () : m_sources (SubTraceSourceTest::INT) {}
  SubTraceSourceTest (enum Sources sources) : m_sources (sources) {}
private:
  enum Sources m_sources;
};

class CompositeTraceResolverTest : public Test
{
public:
  CompositeTraceResolverTest ();
  virtual ~CompositeTraceResolverTest ();
  virtual bool RunTests (void);
private:
  void TraceDouble (TraceContext const &context, double v);
  void TraceInt (TraceContext const &context, int v);
  Ptr<TraceResolver> CreateSubResolver ();


  bool m_gotDoubleA;
  bool m_gotDoubleB;
  CallbackTraceSource<int> m_traceInt;
  bool m_gotInt;
};

CompositeTraceResolverTest::CompositeTraceResolverTest ()
  : Test ("CompositeTraceResolver")
{}
CompositeTraceResolverTest::~CompositeTraceResolverTest ()
{}
void 
CompositeTraceResolverTest::TraceDouble (TraceContext const &context, double v)
{
  TraceSourceTest source;
  context.GetElement (source);
  if (source.IsDoubleA ())
    {
      m_gotDoubleA = true;
    }
  else if (source.IsDoubleB ())
    {
      m_gotDoubleB = true;
    }
  else
    {
      NS_FATAL_ERROR ("should not get any other trace source in this sink");
    }
  
}

void 
CompositeTraceResolverTest::TraceInt (TraceContext const &context, int v)
{
  m_gotInt = true;
}

Ptr<TraceResolver>
CompositeTraceResolverTest::CreateSubResolver (void)
{
  Ptr<CompositeTraceResolver> subresolver = Create<CompositeTraceResolver> ();
  subresolver->AddSource ("trace-int", TraceDoc ("test source"), m_traceInt, 
                          SubTraceSourceTest (SubTraceSourceTest::INT));
  return subresolver;
}
bool 
CompositeTraceResolverTest::RunTests (void)
{
  bool ok = true;

  CallbackTraceSource<double> traceDoubleA;
  CallbackTraceSource<double> traceDoubleB;
  TraceContext context;

  CompositeTraceResolver resolver;

  resolver.AddSource ("trace-double-a", TraceDoc ("test source"), traceDoubleA, 
                      TraceSourceTest (TraceSourceTest::DOUBLEA));
  resolver.AddSource ("trace-double-b", TraceDoc ("test source"), traceDoubleB, 
                      TraceSourceTest (TraceSourceTest::DOUBLEB));

  resolver.Connect ("/*", MakeCallback (&CompositeTraceResolverTest::TraceDouble, this), TraceContext ());

  m_gotDoubleA = false;
  m_gotDoubleB = false;
  traceDoubleA (0);
  if (!m_gotDoubleA || m_gotDoubleB)
    {
      ok = false;
    }
  m_gotDoubleA = false;
  traceDoubleA (0);
  traceDoubleB (0);
  if (!m_gotDoubleA || !m_gotDoubleB)
    {
      ok = false;
    }
  m_gotDoubleA = false;
  m_gotDoubleB = false;

  resolver.Disconnect ("/*", MakeCallback (&CompositeTraceResolverTest::TraceDouble, this));

  m_gotDoubleA = false;
  m_gotDoubleB = false;
  traceDoubleA (0);
  traceDoubleB (0);
  if (m_gotDoubleA || m_gotDoubleB)
    {
      ok = false;
    }

  resolver.Connect ("/trace-double-a", 
		    MakeCallback (&CompositeTraceResolverTest::TraceDouble, this), TraceContext ());
  m_gotDoubleA = false;
  m_gotDoubleB = false;
  traceDoubleA (0);
  traceDoubleB (0);
  if (!m_gotDoubleA || m_gotDoubleB)
    {
      ok = false;
    }
  resolver.Disconnect ("/trace-double-a", 
		       MakeCallback (&CompositeTraceResolverTest::TraceDouble, this));

  resolver.Connect ("/(trace-double-a)", 
		    MakeCallback (&CompositeTraceResolverTest::TraceDouble, this), TraceContext ());
  m_gotDoubleA = false;
  m_gotDoubleB = false;
  traceDoubleA (0);
  traceDoubleB (0);
  if (!m_gotDoubleA || m_gotDoubleB)
    {
      ok = false;
    }
  resolver.Disconnect ("/trace-double-a", 
		       MakeCallback (&CompositeTraceResolverTest::TraceDouble, this));

  resolver.Connect ("/(trace-double-a|trace-double-b)", 
		    MakeCallback (&CompositeTraceResolverTest::TraceDouble, this), TraceContext ());
  m_gotDoubleA = false;
  m_gotDoubleB = false;
  traceDoubleA (0);
  traceDoubleB (0);
  if (!m_gotDoubleA || !m_gotDoubleB)
    {
      ok = false;
    }
  resolver.Disconnect ("/trace-double-a", 
		       MakeCallback (&CompositeTraceResolverTest::TraceDouble, this));
  m_gotDoubleA = false;
  m_gotDoubleB = false;
  traceDoubleA (0);
  traceDoubleB (0);
  if (m_gotDoubleA || !m_gotDoubleB)
    {
      ok = false;
    }


  resolver.Disconnect ("/(trace-double-a|trace-double-b)", 
		       MakeCallback (&CompositeTraceResolverTest::TraceDouble, this));
  m_gotDoubleA = false;
  m_gotDoubleB = false;
  traceDoubleA (0);
  traceDoubleB (0);
  if (m_gotDoubleA || m_gotDoubleB)
    {
      ok = false;
    }

  resolver.Add ("subresolver", 
                MakeCallback (&CompositeTraceResolverTest::CreateSubResolver, this));

  resolver.Connect ("/subresolver/trace-int", 
		    MakeCallback (&CompositeTraceResolverTest::TraceInt, this), TraceContext ());
  m_gotInt = false;
  m_traceInt (1);
  if (!m_gotInt)
    {
      ok = false;
    }

  resolver.Disconnect ("/subresolver/trace-int", 
		       MakeCallback (&CompositeTraceResolverTest::TraceInt, this));
  m_gotInt = false;
  m_traceInt (1);
  if (m_gotInt)
    {
      ok = false;
    }

  resolver.Connect ("/*/trace-int", 
		    MakeCallback (&CompositeTraceResolverTest::TraceInt, this), TraceContext ());
  m_gotInt = false;
  m_traceInt (1);
  if (!m_gotInt)
    {
      ok = false;
    }

  resolver.Disconnect ("/subresolver/trace-int", 
		       MakeCallback (&CompositeTraceResolverTest::TraceInt, this));
  m_gotInt = false;
  m_traceInt (1);
  if (m_gotInt)
    {
      ok = false;
    }

  SVTraceSource<uint16_t> source;

  resolver.AddSource ("uint16_t", TraceDoc ("test source"), source, TraceSourceTest (TraceSourceTest::UINT16_T));
  

  return ok;
}

static CompositeTraceResolverTest g_compositeTraceResolverTest;

}//namespace ns3


#endif /* RUN_SELF_TESTS */
