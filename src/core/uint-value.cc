#include "uint-value.h"
#include "fatal-error.h"
#include <sstream>

namespace ns3 {

UintValue::UintValue (uint64_t value)
  : m_value (value)
{}
Attribute
UintValue::Copy (void) const
{
  return Attribute::Create<UintValue> (*this);
}

void 
UintValue::Set (uint64_t value)
{
  m_value = value;
}
uint64_t 
UintValue::Get (void) const
{
  return m_value;
}
std::string 
UintValue::SerializeToString (Ptr<const AttributeChecker> checker) const
{
  std::ostringstream oss;
  oss << m_value;
  return oss.str ();
}
bool 
UintValue::DeserializeFromString (std::string value, Ptr<const AttributeChecker> checker)
{
  uint64_t v;
  std::istringstream iss;
  iss.str (value);
  iss >> v;
  bool ok = !iss.bad () && !iss.fail ();
  if (ok)
    {
      m_value = v;
    }
  return ok;
}

UintValue::UintValue (Attribute value)
{
  const UintValue *v = value.DynCast<const UintValue *> ();
  if (v == 0)
    {
      NS_FATAL_ERROR ("assigning non-Uint value to Uint value.");
    }
  m_value = v->m_value;
}
UintValue::operator Attribute () const
{
  return Attribute::Create<UintValue> (*this);
}


Ptr<const AttributeChecker> MakeUintChecker (uint64_t min, uint64_t max)
{
  struct Checker : public AttributeChecker
  {
    Checker (uint64_t minValue, uint64_t maxValue)
      : m_minValue (minValue),
      m_maxValue (maxValue) {}
    virtual bool Check (Attribute value) const {
      const UintValue *v = value.DynCast<const UintValue *> ();
      if (v == 0)
	{
	  return false;
	}
      return v->Get () >= m_minValue && v->Get () <= m_maxValue;
    }
    uint64_t m_minValue;
    uint64_t m_maxValue;
  } *checker = new Checker (min, max);
  return Ptr<const AttributeChecker> (checker, false);
}


} // namespace ns3
