//========================================================================
// GLFW 3.3 Unix commons - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2014 Jonas Ådahl <jadahl@gmail.com>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#pragma once
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>

#ifdef __NetBSD__
#define ppoll pollts
#endif

static inline int
initWakeup(int fds[2])
{
    return pipe2(fds, O_CLOEXEC | O_NONBLOCK);
}

static inline void
wakeUp(int fd) {
    while (write(fd, "w", 1) < 0 && errno == EINTR);
}

/*
 * Uses ppoll to wait until an event occurs.
 *
 * If an event is received from the "wakeup" file descriptor,
 *   this file descriptor is drained.
 *
 * @param timeout: timeout in seconds. strictly negative
 *     values mean "no timeout".
 */
static inline int
ppollWithTimeout(struct pollfd *fds, nfds_t nfds, double timeout)
{
    for (nfds_t i = 0; i < nfds; i++)
    {
        fds[i].revents = 0;
    }

    int res;
    if(timeout >= 0.0)
    {
        const long seconds = (long) timeout;
        const long nanoseconds = (long) ((timeout - seconds) * 1e9);
        struct timespec tv = { seconds, nanoseconds };
        res = ppoll(fds, nfds, &tv, NULL);
    }
    else
    {
        res = ppoll(fds, nfds, NULL, NULL);
    }

    if(res > 0)
    {
        if (fds[0].revents & POLLIN)
        {
            // an empty event has been posted: now that we are woken up,
            // we can ignore other potential empty events.
            static char drain_buf[64];
            while(read(fds[0].fd, drain_buf, sizeof(drain_buf)) < 0 && errno == EINTR);
        }
    }
    return res;
}

static inline void
closeFds(int *fds, size_t count)
{
    while(count--) {
        if (*fds > 0) {
            close(*fds);
            *fds = -1;
        }
        fds++;
    }
}

