#ifndef INTERFACE_ID_DEFAULT_VALUE_H
#define INTERFACE_ID_DEFAULT_VALUE_H

#include "default-value.h"
#include "object.h"

namespace ns3 {

class InterfaceIdDefaultValue : public DefaultValueBase
{
public:
  /**
   * \param name the name of this default value.
   * \param help the help text associated to this default value
   * \param iid the interface id which all objects created
   *        through this "default value" must support.
   * \param defaultValue the name of the object to create
   *        by default.
   */
  InterfaceIdDefaultValue (std::string name, 
			   std::string help,
			   InterfaceId iid,
			   std::string defaultValue);
  /**
   * \returns the InterfaceId of the object selected by the user.
   */
  InterfaceId GetValue (void) const;
  /**
   * \param interfaceId the new ClassId selected.
   *
   * Override the currently-selected value.
   */
  void SetValue (InterfaceId interfaceId);
  /**
   * \param name the new object selected.
   *
   * Override the currently-selected value.
   */
  void SetValue (std::string name);
private:
  virtual bool DoParseValue (const std::string &value);
  virtual std::string DoGetType (void) const;
  virtual std::string DoGetDefaultValue (void) const;
  std::string m_defaultName;
  std::string m_name;
  InterfaceId m_interfaceId;
};

} // namespace ns3

#endif /* INTERFACE_ID_DEFAULT_VALUE_H */
