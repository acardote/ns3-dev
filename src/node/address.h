#ifndef ADDRESS_H
#define ADDRESS_H

#include <stdint.h>
#include <ostream>

namespace ns3 {

class Address 
{
public:
  Address (uint8_t type, const uint8_t *buffer, uint8_t len);
  Address (const Address & address);
  Address &operator = (const Address &address);

  uint8_t GetLength (void) const;
  void CopyTo (uint8_t *buffer) const;
  uint8_t GetType (void) const;

  static uint8_t Register (void);
private:
  friend bool operator == (const Address &a, const Address &b);
  friend bool operator < (const Address &a, const Address &b);
  friend std::ostream& operator<< (std::ostream& os, const Address & address);

  Address ();
  enum {
    MAX_SIZE = 14
  };
  uint8_t m_type;
  uint8_t m_len;
  uint8_t m_data[MAX_SIZE];
};

bool operator == (const Address &a, const Address &b);
bool operator != (const Address &a, const Address &b);
bool operator < (const Address &a, const Address &b);
std::ostream& operator<< (std::ostream& os, const Address & address);

} // namespace ns3


#endif /* ADDRESS_H */
