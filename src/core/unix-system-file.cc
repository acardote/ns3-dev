/* -*- Mode:NS3; -*- */
/*
 * Copyright (c) 2005 INRIA
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
#include "system-file.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>
#include <string.h>
#include <list>

#define noTRACE_SYS_FILE 1

#ifdef TRACE_SYS_FILE
#include <iostream>
# define TRACE(x) \
std::cout << "SYS FILE TRACE " << this << " " << x << std::endl;
#else /* TRACE_SYS_FILE */
# define TRACE(format,...)
#endif /* TRACE_SYS_FILE */

#define BUFFER_SIZE (4096)


namespace ns3 {

class SystemFilePrivate {
public:
    SystemFilePrivate ();
    ~SystemFilePrivate ();

    void Open (char const *filename);
    void Write (uint8_t const*buffer, uint32_t size);
private:
    uint8_t m_data[BUFFER_SIZE];
    uint32_t m_current;
    int m_fd;
};

SystemFilePrivate::SystemFilePrivate ()
    : m_current (0)
{}
SystemFilePrivate::~SystemFilePrivate ()
{
    ::write (m_fd, m_data, m_current);
    ::close (m_fd);
}


void
SystemFilePrivate::Open (char const *filename)
{
    m_fd = ::open (filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    assert (m_fd != -1);
}

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif /* min */

void
SystemFilePrivate::Write (uint8_t const*buffer, uint32_t size)
{
    while (size > 0) 
      {
        uint32_t toCopy = min (BUFFER_SIZE - m_current, size);
        memcpy (m_data + m_current, buffer, toCopy);
        size -= toCopy;
        m_current += toCopy;
        buffer += toCopy;
        if (m_current == BUFFER_SIZE) 
          {
            ssize_t written = 0;
            written = ::write (m_fd, m_data, BUFFER_SIZE);
            assert (written == BUFFER_SIZE);
            m_current = 0;
          }
      }
}

SystemFile::SystemFile ()
    : m_priv (new SystemFilePrivate ())
{}
SystemFile::~SystemFile ()
{
    delete m_priv;
    m_priv = 0;
}

void 
SystemFile::Open (char const *filename)
{
    m_priv->Open (filename);
}
void 
SystemFile::Write (uint8_t const*buffer, uint32_t size)
{
    m_priv->Write (buffer, size);
}

}; // namespace
