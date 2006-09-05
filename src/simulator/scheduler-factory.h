/* -*-    Mode:C++; c-basic-offset:4; tab-width:4; indent-tabs-mode:f -*- */
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
#ifndef SCHEDULER_FACTORY_H
#define SCHEDULER_FACTORY_H

namespace ns3 {

class Scheduler;

/**
 * \brief a base class to create event schedulers
 *
 * If you want to make the core simulation engine use a new
 * event scheduler without editing the code of the simulator,
 * you need to create a subclass of this base class and implement
 * the ns3::SchedulerFactory::realCreate method.
 */
class SchedulerFactory {
public:
    virtual ~SchedulerFactory ();
    Scheduler *create (void) const;
private:
    /**
     * \returns a newly-created scheduler. The caller takes 
     *      ownership of the returned pointer.
     */
    virtual Scheduler *realCreate (void) const = 0;
};

}; // namespace ns3

#endif /* SCHEDULER_FACTORY_H */
