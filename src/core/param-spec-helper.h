#ifndef PARAM_SPEC_HELPER_H
#define PARAM_SPEC_HELPER_H

namespace ns3 {

template <typename T1, typename V>
Ptr<ParamSpec>
MakeParamSpecHelper (T1 a1, V initialValue);

template <typename T1, typename T2, typename V>
Ptr<ParamSpec>
MakeParamSpecHelper (T1 a1, T2 a2, V initialValue);

template <typename T1, typename V, typename CHECKER>
Ptr<ParamSpec>
MakeParamSpecHelperWithChecker (T1 a1, V initialValue, CHECKER checker);

template <typename T1, typename T2, typename V, typename CHECKER>
Ptr<ParamSpec>
MakeParamSpecHelperWithChecker (T1 a1, T2 a2, V initialValue, CHECKER checker);

} // namespace ns3

/***************************************************************
 *        The implementation of the above functions.
 ***************************************************************/

#include "type-traits.h"

namespace ns3 {

template <typename T, typename U, typename CHECKER>
class ParamSpecHelper : public ParamSpec
{
public:
  ParamSpecHelper (PValue initialValue, CHECKER checker);
  virtual bool Set (ObjectBase * object, PValue value) const;
  virtual bool Get (const ObjectBase * object, PValue value) const;
  virtual bool Check (PValue value) const;
  virtual PValue GetInitialValue (void) const;
  virtual PValue CreateValue (void) const;

private:
  virtual bool DoSet (T *object, const U *v) const = 0;
  virtual bool DoGet (const T *object, U *v) const = 0;
  PValue m_initialValue;
  CHECKER m_checker;
};

template <typename T>
class ParamSpecHelperSimpleChecker
{
public:
  bool Check (const T &value) const {return true;}
};

template <typename T, typename U, typename V, typename CHECKER>
Ptr<ParamSpec>
DoMakeParamSpecHelperOne (U T::*memberVariable, V initialValue, CHECKER checker)
{
  class MemberVariable : public ParamSpecHelper<T,V,CHECKER>
    {
    public:
      MemberVariable (U T::*memberVariable, V initialValue, CHECKER checker)
	: ParamSpecHelper<T,V,CHECKER> (initialValue, checker),
	m_memberVariable (memberVariable)
	{}
    private:
      virtual bool DoSet (T *object, const V *v) const {
	(object->*m_memberVariable) = v->Get ();
	return true;
      }
      virtual bool DoGet (const T *object, V *v) const {
	v->Set (object->*m_memberVariable);
	return true;
      }
      
      U T::*m_memberVariable;
    };
  return Ptr<ParamSpec> (new MemberVariable (memberVariable, initialValue, checker), false);
}
template <typename T, typename U, typename V>
Ptr<ParamSpec>
DoMakeParamSpecHelperOne (U T::*memberVariable, V initialValue)
{
  return DoMakeParamSpecHelperOne (memberVariable, initialValue, ParamSpecHelperSimpleChecker<U> ());
}


template <typename T, typename U, typename V, typename CHECKER>
Ptr<ParamSpec>
DoMakeParamSpecHelperOne (U (T::*getter) (void) const, 
			  V initialValue, CHECKER checker)
{
  class MemberMethod : public ParamSpecHelper<T,V,CHECKER>
    {
    public:
      MemberMethod (U (T::*getter) (void) const,
		    V initialValue, CHECKER checker)
	: ParamSpecHelper<T,V,CHECKER> (initialValue, checker),
	m_getter (getter)
	{}
    private:
      virtual bool DoSet (T *object, const V *v) const {
	return false;
      }
      virtual bool DoGet (const T *object, V *v) const {
	v->Set ((object->*m_getter) ());
	return true;
      }
      U (T::*m_getter) (void) const;
    };
  return Ptr<ParamSpec> (new MemberMethod (getter, initialValue, checker), false);
}

template <typename T, typename U, typename V>
Ptr<ParamSpec>
DoMakeParamSpecHelperOne (U (T::*getter) (void) const, 
			  V initialValue)
{
  return DoMakeParamSpecHelperOne (getter, initialValue, ParamSpecHelperSimpleChecker<U> ());
}



template <typename T, typename U, typename V, typename CHECKER>
Ptr<ParamSpec>
DoMakeParamSpecHelperOne (void (T::*setter) (U), 
			  V initialValue, CHECKER checker)
{
  class MemberMethod : public ParamSpecHelper<T,V,CHECKER>
    {
    public:
      MemberMethod (void (T::*setter) (U), 
		    V initialValue, CHECKER checker)
	: ParamSpecHelper<T,V,CHECKER> (initialValue, checker),
	m_setter (setter)
	{}
    private:
      virtual bool DoSet (T *object, const V *v) const {
	(object->*m_setter) (v->Get ());
	return true;
      }
      virtual void DoGet (const T *object, V *v) const {
	return false;
      }
      void (T::*m_setter) (U);
    };
  return Ptr<ParamSpec> (new MemberMethod (setter, initialValue, checker), false);
}

template <typename T, typename U, typename V>
Ptr<ParamSpec>
DoMakeParamSpecHelperOne (void (T::*setter) (U), 
			  V initialValue)
{
  return DoMakeParamSpecHelperOne (setter, initialValue, 
				   ParamSpecHelperSimpleChecker<typename TypeTraits<U>::ReferencedType> ());
}


template <typename T, typename U, typename V, typename W, typename CHECKER>
Ptr<ParamSpec>
DoMakeParamSpecHelperTwo (void (T::*setter) (U), 
			  V (T::*getter) (void) const, 
			  W initialValue, 
			  CHECKER checker = ParamSpecHelperSimpleChecker<V> ())
{
  class MemberMethod : public ParamSpecHelper<T,W,CHECKER>
    {
    public:
      MemberMethod (void (T::*setter) (U), 
		    V (T::*getter) (void) const,
		    W initialValue, CHECKER checker)
	: ParamSpecHelper<T,W,CHECKER> (initialValue, checker),
	m_setter (setter),
	m_getter (getter)
	{}
    private:
      virtual bool DoSet (T *object, const W *v) const {
	(object->*m_setter) (v->Get ());
	return true;
      }
      virtual bool DoGet (const T *object, W *v) const {
	v->Set ((object->*m_getter) ());
	return true;
      }
      void (T::*m_setter) (U);
      V (T::*m_getter) (void) const;
    };
  return Ptr<ParamSpec> (new MemberMethod (setter, getter, initialValue, checker), false);
}

template <typename T, typename U, typename V, typename W>
Ptr<ParamSpec>
DoMakeParamSpecHelperTwo (void (T::*setter) (U), 
		      V (T::*getter) (void) const, 
		      W initialValue)
{
  return DoMakeParamSpecHelperTwo (setter, getter, initialValue, ParamSpecHelperSimpleChecker<V> ());
}

template <typename T, typename U, typename V, typename W, typename CHECKER>
Ptr<ParamSpec>
DoMakeParamSpecHelperTwo (V (T::*getter) (void) const, 
			  void (T::*setter) (U), 
			  W initialValue, 
			  CHECKER checker)
{
  return DoMakeParamSpecHelperTwo (setter, getter, initialValue, checker);
}

template <typename T, typename U, typename V, typename W, typename CHECKER>
Ptr<ParamSpec>
DoMakeParamSpecHelperTwo (V (T::*getter) (void) const, 
			  void (T::*setter) (U), 
			  W initialValue)
{
  return DoMakeParamSpecHelperTwo (setter, getter, initialValue);
}


template <typename T, typename U, typename CHECKER>
ParamSpecHelper<T,U,CHECKER>::ParamSpecHelper (PValue initialValue, CHECKER checker)
  : m_initialValue (initialValue),
    m_checker (checker)
{}
template <typename T, typename U, typename CHECKER>
bool 
ParamSpecHelper<T,U,CHECKER>::Set (ObjectBase * object, PValue val) const
{
  const U *value = val.DynCast<const U*> ();
  if (value == 0)
    {
      return false;
    }
  T *obj = dynamic_cast<T *> (object);
  if (obj == 0)
    {
      return false;
    }
  return DoSet (obj, value);
}
template <typename T, typename U, typename CHECKER>
bool 
ParamSpecHelper<T,U,CHECKER>::Get (const ObjectBase * object, PValue val) const
{
  U *value = val.DynCast<U*> ();
  if (value == 0)
    {
      return false;
    }
  const T *obj = dynamic_cast<const T *> (object);
  if (obj == 0)
    {
      return false;
    }
  return DoGet (obj, value);
}
template <typename T, typename U, typename CHECKER>
bool 
ParamSpecHelper<T,U,CHECKER>::Check (PValue value) const
{
  const U *val = value.DynCast<const U*> ();
  if (val == 0)
    {
      return false;
    }
  return m_checker.Check (val->Get ());
}
template <typename T, typename U, typename CHECKER>
PValue
ParamSpecHelper<T,U,CHECKER>::GetInitialValue (void) const
{
  return m_initialValue;
}

template <typename T, typename U, typename CHECKER>
PValue
ParamSpecHelper<T,U,CHECKER>::CreateValue (void) const
{
  return m_initialValue.Copy ();
}



template <typename T1, typename V>
Ptr<ParamSpec>
MakeParamSpecHelper (T1 a1, V initialValue)
{
  return DoMakeParamSpecHelperOne (a1, initialValue);
}

template <typename T1, typename T2, typename V>
Ptr<ParamSpec>
MakeParamSpecHelper (T1 a1, T2 a2, V initialValue)
{
  return DoMakeParamSpecHelperTwo (a1, a2, initialValue);
}

template <typename T1, typename V, typename CHECKER>
Ptr<ParamSpec>
MakeParamSpecHelperWithChecker (T1 a1, V initialValue, CHECKER checker)
{
  return DoMakeParamSpecHelperOne (a1, initialValue, checker);
}

template <typename T1, typename T2, typename V, typename CHECKER>
Ptr<ParamSpec>
MakeParamSpecHelperWithChecker (T1 a1, T2 a2, V initialValue, CHECKER checker)
{
  return DoMakeParamSpecHelperTwo (a1, a2, initialValue, checker);
}

} // namespace ns3

#endif /* PARAM_SPEC_HELPER_H */
