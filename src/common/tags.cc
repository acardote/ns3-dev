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
#include "tags.h"
#include <string.h>

namespace ns3 {

TagsPrettyPrinterRegistry::PrettyPrinters TagsPrettyPrinterRegistry::gPrettyPrinters;

void 
TagsPrettyPrinterRegistry::record (uint32_t uid, void (*cb) (uint8_t [Tags::SIZE], std::ostream &))
{
	for (PrettyPrintersI i = gPrettyPrinters.begin (); 
	     i != gPrettyPrinters.end (); i++) {
		if (i->first == uid) {
			i->second = cb;
			return;
		}
	}
	gPrettyPrinters.push_back (std::make_pair (uid, cb));
}
void 
TagsPrettyPrinterRegistry::prettyPrint (uint32_t uid, uint8_t buf[Tags::SIZE], std::ostream &os)
{
	for (PrettyPrintersI i = gPrettyPrinters.begin (); 
	     i != gPrettyPrinters.end (); i++) {
		if (i->first == uid) {
			if (i->second == 0) {
				os << "tag uid="<<uid<<" null pretty printer."<<std::endl;
			} else {
				(*(i->second)) (buf, os);
			}
			return;
		}
	}
	os << "tag uid="<<uid<<" no pretty printer registered."<< std::endl;
}



uint32_t
Tags::UidFactory::create (void)
{
	static uint32_t uid = 0;
	uid++;
	return uid;
}


#ifdef USE_FREE_LIST

struct Tags::TagData *Tags::gFree = 0;
uint32_t Tags::gN_free = 0;

struct Tags::TagData *
Tags::allocData (void)
{
	struct Tags::TagData *retval;
	if (gFree != 0) {
		retval = gFree;
		gFree = gFree->m_next;
		gN_free--;
	} else {
		retval = new struct Tags::TagData ();
	}
	return retval;
}

void
Tags::freeData (struct TagData *data)
{
	if (gN_free > 1000) {
		delete data;
		return;
	}
	gN_free++;
	data->m_next = gFree;
	gFree = data;
}
#else
struct Tags::TagData *
Tags::allocData (void)
{
	struct Tags::TagData *retval;
	retval = new struct Tags::TagData ();
	return retval;
}

void
Tags::freeData (struct TagData *data)
{
	delete data;
}
#endif

bool
Tags::remove (uint32_t id)
{
	bool found = false;
	for (struct TagData *cur = m_next; cur != 0; cur = cur->m_next) {
		if (cur->m_id == id) {
			found = true;
		}
	}
	if (!found) {
		return false;
	}
	struct TagData *start = 0;
	struct TagData **prevNext = &start;
	for (struct TagData *cur = m_next; cur != 0; cur = cur->m_next) {
		if (cur->m_id == id) {
			/**
			 * XXX
			 * Note: I believe that we could optimize this to
			 * avoid copying each TagData located after the target id
			 * and just link the already-copied list to the next tag.
			 */
			continue;
		}
		struct TagData *copy = allocData ();
		copy->m_id = cur->m_id;
		copy->m_count = 1;
		copy->m_next = 0;
		memcpy (copy->m_data, cur->m_data, Tags::SIZE);
		*prevNext = copy;
		prevNext = &copy->m_next;
	}
	*prevNext = 0;
	removeAll ();
	m_next = start;
	return true;
}

bool
Tags::update (uint8_t const*buffer, uint32_t id)
{
	if (!remove (id)) {
		return false;
	}
	struct TagData *newStart = allocData ();
	newStart->m_count = 1;
	newStart->m_next = 0;
	newStart->m_id = id;
	memcpy (newStart->m_data, buffer, Tags::SIZE);
	newStart->m_next = m_next;
	m_next = newStart;
	return true;
}

void 
Tags::prettyPrint (std::ostream &os)
{
	for (struct TagData *cur = m_next; cur != 0; cur = cur->m_next) {
		TagsPrettyPrinterRegistry::prettyPrint (cur->m_id, cur->m_data, os);
	}
}


}; // namespace ns3

#ifdef RUN_SELF_TESTS

#include "ns3/test.h"
#include <iomanip>
#include <iostream>

namespace ns3 {

class TagsTest : Test {
public:
	TagsTest ();
	virtual ~TagsTest ();
	virtual bool runTests (void);
};

struct myTagA {
	uint8_t a;
};
struct myTagB {
	uint32_t b;
};
struct myTagC {
	uint8_t c [Tags::SIZE];
};
struct myInvalidTag {
	uint8_t invalid [Tags::SIZE+1];
};

static void 
myTagAPrettyPrinterCb (struct myTagA *a, std::ostream &os)
{
	os << "struct myTagA, a="<<(uint32_t)a->a<<std::endl;
}
static void 
myTagBPrettyPrinterCb (struct myTagB *b, std::ostream &os)
{
	os << "struct myTagB, b="<<b->b<<std::endl;
}
static void 
myTagCPrettyPrinterCb (struct myTagC *c, std::ostream &os)
{
	os << "struct myTagC, c="<<(uint32_t)c->c[0]<<std::endl;
}


static TagPrettyPrinter<struct myTagA> gMyTagAPrettyPrinter (&myTagAPrettyPrinterCb);
static TagPrettyPrinter<struct myTagB> gMyTagBPrettyPrinter (&myTagBPrettyPrinterCb);
static TagPrettyPrinter<struct myTagC> gMyTagCPrettyPrinter (&myTagCPrettyPrinterCb);


TagsTest::TagsTest ()
	: Test ("Tags")
{}
TagsTest::~TagsTest ()
{}

bool 
TagsTest::runTests (void)
{
	bool ok = true;

	// build initial tag.
	Tags tags;
	struct myTagA a;
	a.a = 10;
	tags.add (&a);
	a.a = 0;
	tags.peek (&a);
	if (a.a != 10) {
		ok = false;
	}
	//tags.prettyPrint (std::cout);
	struct myTagB b;
	b.b = 0xff;
	tags.add (&b);
	b.b = 0;
	tags.peek (&b);
	if (b.b != 0xff) {
		ok = false;
	}
	//tags.prettyPrint (std::cout);

	// make sure copy contains copy.
	Tags other = tags;
	//other.prettyPrint (std::cout);
	//tags.prettyPrint (std::cout);
	struct myTagA oA;
	oA.a = 0;
	other.peek (&oA);
	if (oA.a != 10) {
		ok = false;
	}
	struct myTagB oB;
	other.peek (&oB);
	if (oB.b != 0xff) {
		ok = false;
	}
	// remove data.
	other.remove (&oA);
	if (other.peek (&oA)) {
		ok = false;
	}
	//other.prettyPrint (std::cout);
	if (!tags.peek (&oA)) {
		ok = false;
	}
	other.remove (&oB);
	if (other.peek (&oB)) {
		ok = false;
	}
	if (!tags.peek (&oB)) {
		ok = false;
	}

	other = tags;
	Tags another = other;
	struct myTagC c;
	c.c[0] = 0x66;
	another.add (&c);
	c.c[0] = 0;
	another.peek (&c);
	if (!another.peek (&c)) {
		ok = false;
	}
	if (tags.peek (&c)) {
		ok = false;
	}

	other = other;
	//other.prettyPrint (std::cout);

	//struct myInvalidTag invalid;
	//tags.add (&invalid);

	return ok;
}

static TagsTest gTagsTest;


}; // namespace ns3

#endif /* RUN_SELF_TESTS */

