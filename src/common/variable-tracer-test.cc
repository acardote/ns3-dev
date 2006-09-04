/* -*-	Mode:C++; c-basic-offset:8; tab-width:8; indent-tabs-mode:t -*- */
/*
 * Copyright (c) 2006 INRIA
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

#include "ui-variable-tracer.h"
#include "si-variable-tracer.h"
#include "ns3/test.h"
#include "ns3/callback.h"


namespace ns3 {

class Foo {
public:
	void notify (uint64_t old_val, uint64_t new_val) {}
};

class VariableTracerTest: public Test {
public:
	VariableTracerTest ();
	void run_unsigned_tests (void);
	void run_signed_unsigned_tests (void);
	virtual bool run_tests (void);
};
void
VariableTracerTest::run_unsigned_tests (void)
{
	UiVariableTracer<uint32_t> var, ovar, tmp;
	uint32_t utmp;
	Foo *foo = new Foo ();
	
	var.set_callback (make_callback (&Foo::notify, foo));

	var = 10;
	ovar = var;

	if (var == ovar) {
	}
	if (var != ovar) {
	}
	if (var > ovar) {
	}
	if (var >= ovar) {
	}
	if (var < ovar) {
	}
	if (var <= ovar) {
	}

	if (var == 1) {
	}
	if (var != 1) {
	}
	if (var > 1) {
	}
	if (var >= 1) {
	}
	if (var < 1) {
	}
	if (var <= 1) {
	}

	if (1 == ovar) {
	}
	if (1 != ovar) {
	}
	if (1 > ovar) {
	}
	if (1 >= ovar) {
	}
	if (1 < ovar) {
	}
	if (1 <= ovar) {
	}

	var++;
	++var;
	var--;
	--var;

	tmp = var + ovar;
	tmp = var - ovar;
	tmp = var / ovar;
	tmp = var * ovar;
	tmp = var << ovar;
	tmp = var >> ovar;
	tmp = var & ovar;
	tmp = var | ovar;
	tmp = var ^ ovar;

	tmp = var + 1;
	tmp = var - 1;
	tmp = var / 1;
	tmp = var * 1;
	tmp = var << 1;
	tmp = var >> 1;
	tmp = var & 1;
	tmp = var | 1;
	tmp = var ^ 1;

	tmp = 1 + ovar;
	tmp = 1 - ovar;
	tmp = 1 / ovar;
	tmp = 1 * ovar;
	tmp = 1 << ovar;
	tmp = 1 >> ovar;
	tmp = 1 & ovar;
	tmp = 1 | ovar;
	tmp = 1 ^ ovar;

	tmp += var;
	tmp -= var;
	tmp /= var;
	tmp *= var;
	tmp <<= var;
	tmp >>= var;
	tmp &= var;
	tmp |= var;
	tmp ^= var;

	tmp += 1;
	tmp -= 1;
	tmp /= 1;
	tmp *= 1;
	tmp <<= 1;
	tmp >>= 1;
	tmp &= 1;
	tmp |= 1;
	tmp ^= 1;


	utmp = var + ovar;
	utmp = var - ovar;
	utmp = var / ovar;
	utmp = var * ovar;
	utmp = var << ovar;
	utmp = var >> ovar;
	utmp = var & ovar;
	utmp = var | ovar;
	utmp = var ^ ovar;

	utmp = var + 1;
	utmp = var - 1;
	utmp = var / 1;
	utmp = var * 1;
	utmp = var << 1;
	utmp = var >> 1;
	utmp = var & 1;
	utmp = var | 1;
	utmp = var ^ 1;

	utmp = 1 + ovar;
	utmp = 1 - ovar;
	utmp = 1 / ovar;
	utmp = 1 * ovar;
	utmp = 1 << ovar;
	utmp = 1 >> ovar;
	utmp = 1 & ovar;
	utmp = 1 | ovar;
	utmp = 1 ^ ovar;

	utmp += var;
	utmp -= var;
	utmp /= var;
	utmp *= var;
	utmp <<= var;
	utmp >>= var;
	utmp &= var;
	utmp |= var;
	utmp ^= var;

	utmp += 1;
	utmp -= 1;
	utmp /= 1;
	utmp *= 1;
	utmp <<= 1;
	utmp >>= 1;
	utmp &= 1;
	utmp |= 1;
	utmp ^= 1;
}

void
VariableTracerTest::run_signed_unsigned_tests (void)
{
	unsigned short utmp = 10;
	unsigned int uitmp = 7;
	short stmp = 5;
	utmp = stmp;
	utmp += stmp;
	uitmp = utmp;
	utmp = uitmp;

	UiVariableTracer<unsigned short> uvar = 10;
	UiVariableTracer<unsigned int> uivar = 5;
	SiVariableTracer<short> svar = 5;
	SiVariableTracer<int> sivar = 5;
	uvar = svar;
	svar = uvar;
	uvar += svar;
	svar += uvar;

	uvar = sivar;
	sivar = uvar;
	uvar += sivar;
	sivar += uvar;

	uivar = uvar;
	uvar = uivar;
	uivar += uvar;
	uvar += uivar;

	sivar = svar;
	svar = sivar;
	sivar += svar;
	svar += sivar;
}

bool 
VariableTracerTest::run_tests (void)
{
	run_unsigned_tests ();
	run_signed_unsigned_tests ();

	return true;
}

VariableTracerTest::VariableTracerTest ()
	: Test ("VariableTracer") {}

static VariableTracerTest g_variable_tracer_test;

}; // namespace ns3


