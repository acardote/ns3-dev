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
#ifndef COMPOSITE_TRACE_RESOLVER_H
#define COMPOSITE_TRACE_RESOLVER_H

#include <vector>
#include "callback.h"
#include "ptr.h"
#include "trace-resolver.h"
#include "callback-trace-source.h"
#include "uv-trace-source.h"
#include "sv-trace-source.h"
#include "fv-trace-source.h"
#include "array-trace-resolver.h"

namespace ns3 {

/**
 * \brief a helper class to aggregate contained TraceResolver and other trace sources.
 * \ingroup lowleveltracing
 */
class CompositeTraceResolver : public TraceResolver
{
public:
  CompositeTraceResolver ();
  virtual ~CompositeTraceResolver ();
  /**
   * \param name name of trace source
   * \param trace a callback trace source
   * \param context the context associated to this trace source
   *
   * Add a callback trace source in this resolver. This trace
   * source will match the name specified during namespace 
   * resolution. The TraceContext of this trace source will also
   * be automatically extended to contain the input context.
   */
  template <typename T1, typename T2,
            typename T3, typename T4,
            typename T>
  void Add (std::string name,
            CallbackTraceSource<T1,T2,T3,T4> &trace, T const &context);
  /**
   * \param name name of trace source
   * \param trace a signed variable trace source
   * \param context the context associated to this trace source
   *
   * Add a signed variable trace source in this resolver. 
   * This trace source will match the name specified during namespace 
   * resolution. The TraceContext of this trace source will also
   * be automatically extended to contain the input context.
   */
  template <typename T>
  void Add (std::string name,
            SVTraceSource<T> &trace, T const &context);
  /**
   * \param name name of trace source
   * \param trace an unsigned variable trace source
   * \param context the context associated to this trace source
   *
   * Add an unsigned variable trace source in this resolver. 
   * This trace source will match the name specified during namespace 
   * resolution. The TraceContext of this trace source will also
   * be automatically extended to contain the input context.
   */
  template <typename T>
  void Add (std::string name,
            UVTraceSource<T> &trace, T const &context);
  /**
   * \param name name of trace source
   * \param trace a floating-point variable trace source
   * \param context the context associated to this trace source
   *
   * Add a floating-point variable trace source in this resolver. 
   * This trace source will match the name specified during namespace 
   * resolution. The TraceContext of this trace source will also
   * be automatically extended to contain the input context.
   */
  template <typename T>
  void Add (std::string name,
            FVTraceSource<T> &trace, T const &context);
  /**
   * \param name name of child trace resolver
   * \param createResolver a trace resolver constructor
   *
   * Add a child trace resolver to this resolver. This child
   * trace resolver will match the name specified during
   * namespace resolution. When this happens, the constructor
   * will be invoked to create the child trace resolver.
   */
  void Add (std::string name, 
            Callback<Ptr<TraceResolver> > createResolver);

  void AddChild (std::string name, Ptr<Object> child);

  template <typename T>
  void AddChild (std::string name, Ptr<Object> child, const T &contextElement);

  template <typename ITERATOR, typename INDEX>
  void AddArray (std::string name, 
                 ITERATOR begin, ITERATOR end, INDEX index);


  void SetParent (Ptr<TraceResolver> parent);

  virtual void Connect (std::string path, CallbackBase const &cb, const TraceContext &context);
  virtual void Disconnect (std::string path, CallbackBase const &cb);

private:
  class CompositeItem 
  {
  public:
    virtual ~CompositeItem () {}
    virtual void Connect (std::string subpath, const CallbackBase &cb, const TraceContext &context) = 0;
    virtual void Disconnect (std::string subpath, const CallbackBase &cb) = 0;

    std::string name;
    TraceContext context;
  };
  typedef std::vector<CompositeItem *> TraceItems;
  enum Operation {
    CONNECT,
    DISCONNECT
  };

  void AddItem (CompositeItem *item);
  void OperationOne (std::string subpath, 
                     TraceItems::const_iterator i,
                     const CallbackBase &cb,
                     const TraceContext &context,
                     enum Operation op);
  void DoRecursiveOperation (std::string path, 
                             const CallbackBase &cb, 
                             const TraceContext &context,
                             enum Operation op);
  void DoRecursiveOperationForParent (std::string path, 
                                      const CallbackBase &cb, 
                                      const TraceContext &context, 
                                      enum Operation op);
  void DoAddChild (std::string name, Ptr<Object> child, const TraceContext &context);

  CompositeTraceResolver::TraceItems m_items;
  Ptr<TraceResolver> m_parent;
};

}//namespace ns3

namespace ns3 {



template <typename T1, typename T2,
          typename T3, typename T4,
          typename T>
void 
CompositeTraceResolver::Add (std::string name,
                             CallbackTraceSource<T1,T2,T3,T4> &trace, 
                             T const &context)
{
  class CallbackCompositeItem : public CompositeItem
  {
  public:
    virtual void Connect (std::string subpath, const CallbackBase &cb, const TraceContext &context)
    {if (subpath == "") {trace->AddCallback (cb, context);}}
    virtual void Disconnect (std::string subpath, const CallbackBase &cb)
    {if (subpath == "") {trace->RemoveCallback (cb);}}

    CallbackTraceSource<T1,T2,T3,T4> *trace;
  } *item = new CallbackCompositeItem ();
  item->name = name;
  item->context.Add (context);
  item->trace = &trace;
  AddItem (item);
}
template <typename T>
void 
CompositeTraceResolver::Add (std::string name,
                             SVTraceSource<T> &trace, T const &context)
{
  class SVCompositeItem : public CompositeItem
  {
  public:
    virtual void Connect (std::string subpath, const CallbackBase &cb, const TraceContext &context)
    {if (subpath == "") {trace->AddCallback (cb, context);}}
    virtual void Disconnect (std::string subpath, const CallbackBase &cb)
    {if (subpath == "") {trace->RemoveCallback (cb);}}

    SVTraceSource<T> *trace;
  } *item = new SVCompositeItem ();
  item->name = name;
  item->context.Add (context);
  item->trace = &trace;
  AddItem (item);
}
template <typename T>
void 
CompositeTraceResolver::Add (std::string name,
                             UVTraceSource<T> &trace, T const &context)
{
  class UVCompositeItem : public CompositeItem
  {
  public:
    virtual void Connect (std::string subpath, const CallbackBase &cb, const TraceContext &context)
    {if (subpath == "") {trace->AddCallback (cb, context);}}
    virtual void Disconnect (std::string subpath, const CallbackBase &cb)
    {if (subpath == "") {trace->RemoveCallback (cb);}}

    UVTraceSource<T> *trace;
  } *item = new UVCompositeItem ();
  item->name = name;
  item->context.Add (context);
  item->trace = &trace;
  AddItem (item);
}
template <typename T>
void 
CompositeTraceResolver::Add (std::string name,
                             FVTraceSource<T> &trace, T const &context)
{
  class FVCompositeItem : public CompositeItem
  {
  public:
    virtual void Connect (std::string subpath, const CallbackBase &cb, const TraceContext &context)
    {if (subpath == "") {trace->AddCallback (cb, context);}}
    virtual void Disconnect (std::string subpath, const CallbackBase &cb)
    {if (subpath == "") {trace->RemoveCallback (cb);}}

    FVTraceSource<T> *trace;
  } *item = new FVCompositeItem ();
  item->name = name;
  item->context.Add (context);
  item->trace = &trace;
  AddItem (item);
}

template <typename ITERATOR, typename INDEX>
void 
CompositeTraceResolver::AddArray (std::string name, 
                                  ITERATOR begin, ITERATOR end, INDEX index)
{
  class ArrayCompositeItem : public CompositeItem
  {
  public:
    virtual void Connect (std::string subpath, const CallbackBase &cb, const TraceContext &context)
    {array->Connect (subpath, cb, context);}
    virtual void Disconnect (std::string subpath, const CallbackBase &cb)
    {array->Disconnect (subpath, cb);}

    Ptr<ArrayTraceResolver<INDEX> > array;
  } *item = new ArrayCompositeItem ();
  item->name = name;
  item->context = TraceContext ();
  item->array = Create<ArrayTraceResolver<INDEX> > ();
  item->array->SetIterators (begin, end);
  AddItem (item);
}

template <typename T>
void 
CompositeTraceResolver::AddChild (std::string name, Ptr<Object> child, const T &contextElement)
{
  TraceContext context;
  context.Add (contextElement);
  DoAddChild (name, child, context);
}



}//namespace ns3

#endif /* COMPOSITE_TRACE_RESOLVER_H */
