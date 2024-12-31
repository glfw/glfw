//========================================================================
// GLFW 3.5 OGC - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2024 Alberto Mardegan <info@mardy.it>
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

#include "internal.h"

#if defined(GLFW_BUILD_OGC_THREAD)

#include <assert.h>
#include <ogcsys.h>
#include <string.h>

#define MAX_THREADS 32
#define INITIAL_STORAGE_CAPACITY 64
#define INCREMENT_STORAGE_CAPACITY 64

typedef struct _GLFWthreadStorageOgc {
    size_t capacity;
    size_t used;
    void *data[0]; /* Array of pointers to the stored data element */
} _GLFWthreadStorageOgc;

#define RESERVED_PTR ((void*)0x1)

typedef struct _GLFWthreadMapOgc {
    lwp_t threadId;
    _GLFWthreadStorageOgc *storage;
} _GLFWthreadMapOgc;

static _GLFWthreadMapOgc threadMap[MAX_THREADS];
static mutex_t threadMapMutex = 0; // TODO initialize!

static _GLFWthreadMapOgc *storageEntryForThread()
{
    lwp_t currentThreadId = LWP_GetSelf();

    /* We are not creating entries here and only our thread can modify our
     * entry, so we don't need to lock the mutex here. */
    for (int i = 0; i < MAX_THREADS; i++) {
        _GLFWthreadMapOgc *entry = &threadMap[i];
        if (entry->threadId == currentThreadId) {
            return entry;
        }
    }
    return NULL;
}

static inline _GLFWthreadStorageOgc *storageForThread()
{
    _GLFWthreadMapOgc *entry = storageEntryForThread();
    return entry ? entry->storage : NULL;
}

static _GLFWthreadMapOgc *createStorageEntryForThread()
{
    size_t initialSize = sizeof(_GLFWthreadStorageOgc) +
        INITIAL_STORAGE_CAPACITY * sizeof(void*);
    _GLFWthreadStorageOgc *storage = _glfw_calloc(1, initialSize);
    if (!storage) {
        _glfwInputError(GLFW_OUT_OF_MEMORY,
                        "OGC: Failed to allocate TLS data");
        return NULL;
    }

    storage->capacity = INITIAL_STORAGE_CAPACITY;
    storage->used = 0;

    if (!LWP_MutexLock(threadMapMutex)) {
        _glfw_free(storage);
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "OGC: Failed to lock TLS mutex");
        return NULL;
    }

    _GLFWthreadMapOgc *entry = NULL;
    lwp_t currentThreadId = LWP_GetSelf();
    for (int i = 0; i < MAX_THREADS; i++) {
        entry = &threadMap[i];
        if (!entry->threadId) {
            entry->threadId = currentThreadId;
            entry->storage = storage;
            break;
        }
    }
    if (entry->threadId != currentThreadId) {
        // We didn't find a free slot
        entry = NULL;
    }

    LWP_MutexUnlock(threadMapMutex);
    return entry;
}

//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

GLFWbool _glfwPlatformCreateTls(_GLFWtls* tls)
{
    assert(tls->ogc.allocated == GLFW_FALSE);

    _GLFWthreadMapOgc *entry = storageEntryForThread();
    if (!entry) {
        entry = createStorageEntryForThread();
        if (!entry) {
            // The error has already been logged
            return GLFW_FALSE;
        }
    }
    _GLFWthreadStorageOgc *storage = entry->storage;

    if (storage->used >= storage->capacity) {
        size_t newSize = sizeof(_GLFWthreadStorageOgc) +
            (storage->capacity + INCREMENT_STORAGE_CAPACITY) * sizeof(void*);
        storage = _glfw_realloc(storage, newSize);
        if (!storage) {
            _glfwInputError(GLFW_OUT_OF_MEMORY,
                            "OGC: Failed to reallocate TLS data");
            return GLFW_FALSE;
        }
        // No need to lock the map, since we are changing our own thread's
        // entry
        entry->storage = storage;
        storage->capacity += INCREMENT_STORAGE_CAPACITY;
    }

    // Find the first free entry in the TLS data
    int index = -1;
    for (int i = 0; i < storage->capacity; i++) {
        if (!storage->data[i]) {
            index = i;
            break;
        }
    }

    /* We know that storage->capacity > storage->used, so there must be free
     * slots */
    assert(index >= 0);

    storage->data[index] = RESERVED_PTR;
    storage->used++;

    tls->ogc.index = index;
    tls->ogc.allocated = GLFW_TRUE;
    return GLFW_TRUE;
}

void _glfwPlatformDestroyTls(_GLFWtls* tls)
{
    if (tls->ogc.allocated) {
        _GLFWthreadMapOgc *entry = storageEntryForThread();
        if (entry) {
            assert(entry->storage->capacity > tls->ogc.index);
            assert(entry->storage->used > 0);
            entry->storage->data[tls->ogc.index] = NULL;
            entry->storage->used--;
        }
    }
    memset(tls, 0, sizeof(_GLFWtls));
}

void* _glfwPlatformGetTls(_GLFWtls* tls)
{
    assert(tls->ogc.allocated == GLFW_TRUE);
    _GLFWthreadMapOgc *entry = storageEntryForThread();
    void *ptr = entry->storage->data[tls->ogc.index];
    return ptr == RESERVED_PTR ? NULL : ptr;
}

void _glfwPlatformSetTls(_GLFWtls* tls, void* value)
{
    assert(tls->posix.allocated == GLFW_TRUE);
    _GLFWthreadMapOgc *entry = storageEntryForThread();
    entry->storage->data[tls->ogc.index] = value;
}

GLFWbool _glfwPlatformCreateMutex(_GLFWmutex* mutex)
{
    assert(mutex->ogc.allocated == GLFW_FALSE);

    if (LWP_MutexInit(&mutex->ogc.handle, false) != 0)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "OGC: Failed to create mutex");
        return GLFW_FALSE;
    }

    return mutex->ogc.allocated = GLFW_TRUE;
}

void _glfwPlatformDestroyMutex(_GLFWmutex* mutex)
{
    if (mutex->ogc.allocated)
        LWP_MutexDestroy(mutex->ogc.handle);
    memset(mutex, 0, sizeof(_GLFWmutex));
}

void _glfwPlatformLockMutex(_GLFWmutex* mutex)
{
    assert(mutex->ogc.allocated == GLFW_TRUE);
    LWP_MutexLock(mutex->ogc.handle);
}

void _glfwPlatformUnlockMutex(_GLFWmutex* mutex)
{
    assert(mutex->ogc.allocated == GLFW_TRUE);
    LWP_MutexUnlock(mutex->ogc.handle);
}

void _glfwThreadInitMutexOgc()
{
    LWP_MutexInit(&threadMapMutex, false);
}

#endif // GLFW_BUILD_OGC_THREAD

