#ifdef RUN_SELF_TESTS
#include "test.h"
#include "object.h"
#include "boolean.h"
#include "integer.h"
#include "uinteger.h"
#include "enum.h"
#include "random-variable.h"
#include "double.h"
#include "object-vector.h"
#include "integer-trace-source.h"
#include "trace-source-accessor.h"

namespace ns3 {

class AttributeTest : public Test
{
public:
  AttributeTest ();
  virtual bool RunTests (void);
private:
  void NotifySource1 (int64_t old, int64_t n) {
    m_gotNew = n;
  }
  int64_t m_gotNew;
};

class Derived : public Object
{
public:
  static TypeId GetTypeId (void) {
    static TypeId tid = TypeId ("Derived")
      .SetParent<Object> ()
      ;
    return tid;
  }
};

class AttributeObjectTest : public Object
{
public:
  enum TestEnum {
    TEST_A,
    TEST_B,
    TEST_C
  };
  static TypeId GetTypeId (void) {
    static TypeId tid = TypeId ("AttributeObjectTest")
      .SetParent<Object> ()
      .AddAttribute ("TestBoolName", "help text",
		     Boolean (false),
		     MakeBooleanAccessor (&AttributeObjectTest::m_boolTest),
		     MakeBooleanChecker ())
      .AddAttribute ("TestBoolA", "help text",
		     Boolean (false),
		     MakeBooleanAccessor (&AttributeObjectTest::DoSetTestB,
					   &AttributeObjectTest::DoGetTestB),
		     MakeBooleanChecker ())
      .AddAttribute ("TestPtr", "help text", 
		     Ptr<Derived> (0),
		     MakePtrAccessor (&AttributeObjectTest::m_derived),
		     MakePtrChecker<Derived> ())
      .AddAttribute ("TestInt16", "help text",
		     Integer (-2),
		     MakeIntegerAccessor (&AttributeObjectTest::m_int16),
		     MakeIntegerChecker<int16_t> ())
      .AddAttribute ("TestInt16WithBounds", "help text",
		     Integer (-2),
		     MakeIntegerAccessor (&AttributeObjectTest::m_int16WithBounds),
		     MakeIntegerChecker (-5, 10))
      .AddAttribute ("TestInt16SetGet", "help text",
		     Integer (6),
		     MakeIntegerAccessor (&AttributeObjectTest::DoSetInt16,
				       &AttributeObjectTest::DoGetInt16),
		     MakeIntegerChecker<int16_t> ())
      .AddAttribute ("TestUint8", "help text",
		     Uinteger (1),
		     MakeUintegerAccessor (&AttributeObjectTest::m_uint8),
		     MakeUintegerChecker<uint8_t> ())
      .AddAttribute ("TestEnum", "help text",
		     Enum (TEST_A),
		     MakeEnumAccessor (&AttributeObjectTest::m_enum),
		     MakeEnumChecker (TEST_A, "TestA",
				      TEST_B, "TestB",
				      TEST_C, "TestC"))
      .AddAttribute ("TestRandom", "help text",
		     ConstantVariable (1.0),
		     MakeRandomVariableAccessor (&AttributeObjectTest::m_random),
		     MakeRandomVariableChecker ())
      .AddAttribute ("TestFloat", "help text",
		     Double (-1.1),
		     MakeDoubleAccessor (&AttributeObjectTest::m_float),
		     MakeDoubleChecker<float> ())
      .AddAttribute ("TestVector1", "help text",
		     ObjectVector (),
		     MakeObjectVectorAccessor (&AttributeObjectTest::m_vector1),
		     MakeObjectVectorChecker ())
      .AddAttribute ("TestVector2", "help text",
		     ObjectVector (),
		     MakeObjectVectorAccessor (&AttributeObjectTest::DoGetVectorN,
						&AttributeObjectTest::DoGetVector),
		     MakeObjectVectorChecker ())
      .AddAttribute ("IntegerTraceSource1", "help text",
		     Integer (-2),
		     MakeIntegerAccessor (&AttributeObjectTest::m_intSrc1),
		     MakeIntegerChecker<int8_t> ())
      .AddAttribute ("IntegerTraceSource2", "help text",
		     Integer (-2),
		     MakeIntegerAccessor (&AttributeObjectTest::DoSetIntSrc,
					  &AttributeObjectTest::DoGetIntSrc),
		     MakeIntegerChecker<int8_t> ())
      .AddTraceSource ("Source1", "help test",
		       MakeTraceSourceAccessor (&AttributeObjectTest::m_intSrc1))
      ;
        
    return tid;
  }

  void AddToVector1 (void) {
    m_vector1.push_back (CreateObject<Derived> ());
  }
  void AddToVector2 (void) {
    m_vector2.push_back (CreateObject<Derived> ());
  }

private:
  void DoSetTestB (bool v) {
    m_boolTestA = v;
  }
  bool DoGetTestB (void) const {
    return m_boolTestA;
  }
  int16_t DoGetInt16 (void) const {
    return m_int16SetGet;
  }
  void DoSetInt16 (int16_t v) {
    m_int16SetGet = v;
  }
  uint32_t DoGetVectorN (void) const {
    return m_vector2.size ();
  }
  Ptr<Derived> DoGetVector (uint32_t i) const {
    return m_vector2[i];
  }
  void DoSetIntSrc (int8_t v) {
    m_intSrc2 = v;
  }
  int8_t DoGetIntSrc (void) const {
    return m_intSrc2;
  }
  bool m_boolTestA;
  bool m_boolTest;
  Ptr<Derived> m_derived;
  int16_t m_int16;
  int16_t m_int16WithBounds;
  int16_t m_int16SetGet;
  uint8_t m_uint8;
  float m_float;
  enum TestEnum m_enum;
  RandomVariable m_random;
  std::vector<Ptr<Derived> > m_vector1;
  std::vector<Ptr<Derived> > m_vector2;
  IntegerTraceSource<int8_t> m_intSrc1;
  IntegerTraceSource<int8_t> m_intSrc2;
};


#define CHECK_GET_STR(p,name,value)		\
  {						\
    std::string expected = value;		\
    std::string got;				\
    bool ok = p->GetAttribute (name, got);	\
    NS_TEST_ASSERT (ok);			\
    NS_TEST_ASSERT_EQUAL (got, expected);	\
  }
#define CHECK_GET_PARAM(p,name,type,value)		\
  {							\
    const type expected = value;			\
    type got = value;					\
    Attribute v = p->GetAttribute (name);		\
    got = v;						\
    NS_TEST_ASSERT_EQUAL (got.Get (), expected.Get ());	\
  }

NS_OBJECT_ENSURE_REGISTERED (AttributeObjectTest);

AttributeTest::AttributeTest ()
  : Test ("Attribute")
{}
bool 
AttributeTest::RunTests (void)
{
  bool result = true;

  AttributeList params;
  Ptr<AttributeObjectTest> p;
  NS_TEST_ASSERT (params.Set ("AttributeObjectTest::TestBoolName", "false"));
  p = CreateObject<AttributeObjectTest> (params);
  CHECK_GET_STR (p, "TestBoolName", "false");
  CHECK_GET_PARAM (p, "TestBoolName", Boolean, false);

  NS_TEST_ASSERT (p->SetAttribute("TestBoolName", "true"));
  CHECK_GET_STR (p, "TestBoolName", "true");
  CHECK_GET_PARAM (p, "TestBoolName", Boolean, true);

  NS_TEST_ASSERT (p->SetAttribute("TestBoolName", Boolean (false)));
  CHECK_GET_STR (p, "TestBoolName", "false");
  CHECK_GET_PARAM (p, "TestBoolName", Boolean, false);

  p = CreateObjectWith<AttributeObjectTest> ("TestBoolName", "true");
  CHECK_GET_STR (p, "TestBoolName", "true");
  CHECK_GET_PARAM (p, "TestBoolName", Boolean, true);

  p = CreateObjectWith<AttributeObjectTest> ("TestBoolName", Boolean (true));
  CHECK_GET_STR (p, "TestBoolName", "true");
  CHECK_GET_PARAM (p, "TestBoolName", Boolean, true);

  NS_TEST_ASSERT (p->SetAttribute("TestBoolA", "false"));
  CHECK_GET_STR (p, "TestBoolA", "false");
  CHECK_GET_PARAM (p, "TestBoolA", Boolean, false);

  NS_TEST_ASSERT (p->SetAttribute("TestBoolA", "true"));
  CHECK_GET_STR (p, "TestBoolA", "true");
  CHECK_GET_PARAM (p, "TestBoolA", Boolean, true);


  Ptr<Derived> derived = p->GetAttribute ("TestPtr");
  NS_TEST_ASSERT (derived == 0);
  derived = Create<Derived> ();
  NS_TEST_ASSERT (p->SetAttribute("TestPtr", derived));
  Ptr<Derived> stored = p->GetAttribute ("TestPtr");
  NS_TEST_ASSERT (stored == derived);
  Ptr<Object> storedBase = p->GetAttribute ("TestPtr");
  NS_TEST_ASSERT (stored == storedBase);
  Ptr<AttributeObjectTest> x = p->GetAttribute ("TestPtr");
  NS_TEST_ASSERT (x == 0);

  p = CreateObjectWith<AttributeObjectTest> ("TestPtr", Create<Derived> ());
  NS_TEST_ASSERT (p != 0);
  derived = 0;
  derived = p->GetAttribute ("TestPtr");
  NS_TEST_ASSERT (derived != 0);

  CHECK_GET_STR (p, "TestInt16", "-2");
  CHECK_GET_PARAM (p, "TestInt16", Integer, -2);

  NS_TEST_ASSERT (p->SetAttribute("TestInt16", "-5"));
  CHECK_GET_STR (p, "TestInt16", "-5");
  CHECK_GET_PARAM (p, "TestInt16", Integer, -5);

  NS_TEST_ASSERT (p->SetAttribute("TestInt16", Integer (+2)));
  CHECK_GET_STR (p, "TestInt16", "2");
  CHECK_GET_PARAM (p, "TestInt16", Integer, +2);

  NS_TEST_ASSERT (p->SetAttribute("TestInt16", Integer (-32768)));
  CHECK_GET_STR (p, "TestInt16", "-32768");
  CHECK_GET_PARAM (p, "TestInt16", Integer, -32768);

  NS_TEST_ASSERT (!p->SetAttribute("TestInt16", Integer (-32769)));
  CHECK_GET_STR (p, "TestInt16", "-32768");
  CHECK_GET_PARAM (p, "TestInt16", Integer, -32768);

  NS_TEST_ASSERT (p->SetAttribute("TestInt16", Integer (32767)));
  CHECK_GET_STR (p, "TestInt16", "32767");
  CHECK_GET_PARAM (p, "TestInt16", Integer, 32767);

  NS_TEST_ASSERT (!p->SetAttribute("TestInt16", Integer (32768)));
  CHECK_GET_STR (p, "TestInt16", "32767");
  CHECK_GET_PARAM (p, "TestInt16", Integer, 32767);

  NS_TEST_ASSERT (p->SetAttribute("TestInt16WithBounds", Integer (10)));
  CHECK_GET_STR (p, "TestInt16WithBounds", "10");
  CHECK_GET_PARAM (p, "TestInt16WithBounds", Integer, 10);
  NS_TEST_ASSERT (!p->SetAttribute("TestInt16WithBounds", Integer (11)));
  CHECK_GET_STR (p, "TestInt16WithBounds", "10");
  CHECK_GET_PARAM (p, "TestInt16WithBounds", Integer, 10);

  NS_TEST_ASSERT (p->SetAttribute("TestInt16WithBounds", Integer (-5)));
  CHECK_GET_STR (p, "TestInt16WithBounds", "-5");
  CHECK_GET_PARAM (p, "TestInt16WithBounds", Integer, -5);
  NS_TEST_ASSERT (!p->SetAttribute("TestInt16WithBounds", Integer (-6)));
  CHECK_GET_STR (p, "TestInt16WithBounds", "-5");
  CHECK_GET_PARAM (p, "TestInt16WithBounds", Integer, -5);

  CHECK_GET_STR (p, "TestInt16SetGet", "6");
  CHECK_GET_PARAM (p, "TestInt16SetGet", Integer, 6);
  NS_TEST_ASSERT (p->SetAttribute("TestInt16SetGet", Integer (0)));
  CHECK_GET_STR (p, "TestInt16SetGet", "0");
  CHECK_GET_PARAM (p, "TestInt16SetGet", Integer, 0);

  CHECK_GET_STR (p, "TestUint8", "1");
  CHECK_GET_PARAM (p, "TestUint8", Uinteger, 1);
  NS_TEST_ASSERT (p->SetAttribute("TestUint8", Uinteger (0)));
  CHECK_GET_STR (p, "TestUint8", "0");
  CHECK_GET_PARAM (p, "TestUint8", Uinteger, 0);
  NS_TEST_ASSERT (p->SetAttribute("TestUint8", Uinteger (255)));
  CHECK_GET_STR (p, "TestUint8", "255");
  CHECK_GET_PARAM (p, "TestUint8", Uinteger, 255);
  NS_TEST_ASSERT (p->SetAttribute("TestUint8", "255"));
  CHECK_GET_STR (p, "TestUint8", "255");
  CHECK_GET_PARAM (p, "TestUint8", Uinteger, 255);
  NS_TEST_ASSERT (!p->SetAttribute("TestUint8", "256"));
  CHECK_GET_STR (p, "TestUint8", "255");
  CHECK_GET_PARAM (p, "TestUint8", Uinteger, 255);
  NS_TEST_ASSERT (!p->SetAttribute("TestUint8", "-1"));
  CHECK_GET_STR (p, "TestUint8", "255");
  CHECK_GET_PARAM (p, "TestUint8", Uinteger, 255);
  NS_TEST_ASSERT (!p->SetAttribute("TestUint8", Uinteger (-1)));
  CHECK_GET_STR (p, "TestUint8", "255");
  CHECK_GET_PARAM (p, "TestUint8", Uinteger, 255);

  CHECK_GET_STR (p, "TestFloat", "-1.1");
  NS_TEST_ASSERT (p->SetAttribute("TestFloat", Double ((float)+2.3)));
  CHECK_GET_PARAM (p, "TestFloat", Double, (float)+2.3);

  CHECK_GET_STR (p, "TestEnum", "TestA");
  CHECK_GET_PARAM (p, "TestEnum", Enum, AttributeObjectTest::TEST_A);
  NS_TEST_ASSERT (p->SetAttribute("TestEnum", Enum (AttributeObjectTest::TEST_C)));
  CHECK_GET_STR (p, "TestEnum", "TestC");
  CHECK_GET_PARAM (p, "TestEnum", Enum, AttributeObjectTest::TEST_C);
  NS_TEST_ASSERT (p->SetAttribute("TestEnum", "TestB"));
  CHECK_GET_STR (p, "TestEnum", "TestB");
  CHECK_GET_PARAM (p, "TestEnum", Enum, AttributeObjectTest::TEST_B);
  NS_TEST_ASSERT (!p->SetAttribute("TestEnum", "TestD"));
  CHECK_GET_STR (p, "TestEnum", "TestB");
  CHECK_GET_PARAM (p, "TestEnum", Enum, AttributeObjectTest::TEST_B);
  NS_TEST_ASSERT (!p->SetAttribute("TestEnum", Enum (5)));
  CHECK_GET_STR (p, "TestEnum", "TestB");
  CHECK_GET_PARAM (p, "TestEnum", Enum, AttributeObjectTest::TEST_B);

  RandomVariable ran = p->GetAttribute ("TestRandom");
  NS_TEST_ASSERT (p->SetAttribute("TestRandom", UniformVariable (0.0, 1.0)));
  NS_TEST_ASSERT (p->SetAttribute("TestRandom", ConstantVariable (10.0)));

  {
    ObjectVector vector = p->GetAttribute ("TestVector1");
    NS_TEST_ASSERT_EQUAL (vector.GetN (), 0);
    p->AddToVector1 ();
    NS_TEST_ASSERT_EQUAL (vector.GetN (), 0);
    vector = p->GetAttribute ("TestVector1");
    NS_TEST_ASSERT_EQUAL (vector.GetN (), 1);
    Ptr<Object> a = vector.Get (0);
    NS_TEST_ASSERT_UNEQUAL (a, 0);
    p->AddToVector1 ();
    NS_TEST_ASSERT_EQUAL (vector.GetN (), 1);
    vector = p->GetAttribute ("TestVector1");
    NS_TEST_ASSERT_EQUAL (vector.GetN (), 2);
  }

  {
    ObjectVector vector = p->GetAttribute ("TestVector2");
    NS_TEST_ASSERT_EQUAL (vector.GetN (), 0);
    p->AddToVector2 ();
    NS_TEST_ASSERT_EQUAL (vector.GetN (), 0);
    vector = p->GetAttribute ("TestVector2");
    NS_TEST_ASSERT_EQUAL (vector.GetN (), 1);
    Ptr<Object> a = vector.Get (0);
    NS_TEST_ASSERT_UNEQUAL (a, 0);
    p->AddToVector2 ();
    NS_TEST_ASSERT_EQUAL (vector.GetN (), 1);
    vector = p->GetAttribute ("TestVector2");
    NS_TEST_ASSERT_EQUAL (vector.GetN (), 2);
  }

  NS_TEST_ASSERT (AttributeList::GetGlobal ()->Set ("AttributeObjectTest::TestBoolName", "true"));
  p = CreateObjectWith<AttributeObjectTest> ();
  Boolean boolV = p->GetAttribute ("TestBoolName");
  NS_TEST_ASSERT_EQUAL (boolV, Boolean (true));

  NS_TEST_ASSERT (AttributeList::GetGlobal ()->Set ("AttributeObjectTest::TestBoolName", "false"));
  p = CreateObjectWith<AttributeObjectTest> ();
  boolV = p->GetAttribute ("TestBoolName");
  NS_TEST_ASSERT_EQUAL (boolV, Boolean (false));

  Integer i = p->GetAttribute ("IntegerTraceSource1");
  NS_TEST_ASSERT_EQUAL (i.Get (), -2);
  NS_TEST_ASSERT (p->SetAttribute ("IntegerTraceSource1", Integer (+5)));
  i = p->GetAttribute ("IntegerTraceSource1");
  NS_TEST_ASSERT_EQUAL (i.Get (), +5);
  NS_TEST_ASSERT (p->SetAttribute ("IntegerTraceSource1", Integer (127)));
  NS_TEST_ASSERT (!p->SetAttribute ("IntegerTraceSource1", Integer (128)));
  NS_TEST_ASSERT (p->SetAttribute ("IntegerTraceSource1", Integer (-128)));
  NS_TEST_ASSERT (!p->SetAttribute ("IntegerTraceSource1", Integer (-129)));

  i = p->GetAttribute ("IntegerTraceSource2");
  NS_TEST_ASSERT_EQUAL (i.Get (), -2);
  NS_TEST_ASSERT (p->SetAttribute ("IntegerTraceSource2", Integer (+5)));
  i = p->GetAttribute ("IntegerTraceSource2");
  NS_TEST_ASSERT_EQUAL (i.Get (), +5);
  NS_TEST_ASSERT (p->SetAttribute ("IntegerTraceSource2", Integer (127)));
  NS_TEST_ASSERT (!p->SetAttribute ("IntegerTraceSource2", Integer (128)));
  NS_TEST_ASSERT (p->SetAttribute ("IntegerTraceSource2", Integer (-128)));
  NS_TEST_ASSERT (!p->SetAttribute ("IntegerTraceSource2", Integer (-129)));

  m_gotNew = -2;
  NS_TEST_ASSERT (p->SetAttribute ("IntegerTraceSource1", Integer (-1)));
  NS_TEST_ASSERT (p->TraceSourceConnect ("Source1", MakeCallback (&AttributeTest::NotifySource1, this)));
  NS_TEST_ASSERT (p->SetAttribute ("IntegerTraceSource1", Integer (0)));
  NS_TEST_ASSERT_EQUAL (m_gotNew, 0);
  NS_TEST_ASSERT (p->TraceSourceDisconnect ("Source1", MakeCallback (&AttributeTest::NotifySource1, this)));
  NS_TEST_ASSERT (p->SetAttribute ("IntegerTraceSource1", Integer (1)));
  NS_TEST_ASSERT_EQUAL (m_gotNew, 0);

  return result;
}



static AttributeTest g_parameterTest;

} // namespace ns3


#endif /* RUN_SELF_TESTS */
