#ifndef OBJECT_FACTORY_H
#define OBJECT_FACTORY_H

#include "attribute.h"
#include "object.h"

namespace ns3 {

class ObjectFactory
{
public:
  ObjectFactory ();

  void SetTypeId (TypeId tid);
  void SetTypeId (std::string tid);
  void SetTypeId (const char *tid);
  void Set (std::string name, Attribute value);

  TypeId GetTypeId (void) const;

  Ptr<Object> Create (void) const;
  template <typename T>
  Ptr<T> Create (void) const;

private:
  TypeId m_tid;
  AttributeList m_parameters;
};

} // namespace ns3

namespace ns3 {

template <typename T>
Ptr<T> 
ObjectFactory::Create (void) const
{
  Ptr<Object> object = Create ();
  return object->GetObject<T> ();
}

} // namespace ns3

#endif /* OBJECT_FACTORY_H */
