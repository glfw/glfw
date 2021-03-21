//========================================================================
// GLFW 3.4 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2021 Emmanuel Gil Peyrot <linkmauve@linkmauve.fr>
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
// Please use C89 style variable declarations in this file because VS 2010
//========================================================================

#define _XOPEN_SOURCE 500

#include "cache.h"

#ifndef HAVE_XXHASH
#error "libxxhash is required for the compose cache"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#define XXH_INLINE_ALL
#include <xxhash.h>

static GLFWbool
hashString(const char *string, size_t len, XXH128_canonical_t *out)
{
    XXH3_state_t *state;
    XXH_errorcode err;
    XXH128_hash_t hash;

    state = XXH3_createState();
    if (!state)
        return GLFW_FALSE;

    err = XXH3_128bits_reset(state);
    if (err != XXH_OK)
    {
        XXH3_freeState(state);
        return GLFW_FALSE;
    }

    err = XXH3_128bits_update(state, string, len);
    if (err != XXH_OK)
    {
        XXH3_freeState(state);
        return GLFW_FALSE;
    }

    hash = XXH3_128bits_digest(state);
    XXH3_freeState(state);
    XXH128_canonicalFromHash(out, hash);
    return GLFW_TRUE;
}

GLFWbool
cacheGetPathFromString(const char *string, char **cachePath)
{
    const char *home, *xdg;
    char *path;
    int length;
    char *hash_part;
    XXH128_canonical_t hash;

    /* Mostly copied over from xkb_context_include_path_append_default(). */
    home = getenv("HOME");
    xdg = getenv("XDG_CACHE_HOME");
    if (xdg != NULL)
    {
        length = strlen(xdg) + strlen("/glfw/") + 32 + 1;
        path = malloc(length);
        sprintf(path, "%s/glfw/", xdg);
    }
    else if (home != NULL)
    {
        /* XDG_CACHE_HOME fallback is $HOME/.cache/ */
        length = strlen(home) + strlen("/.cache/glfw/") + 32 + 1;
        path = malloc(length);
        sprintf(path, "%s/.cache/glfw/", home);
    }
    else
    {
        return GLFW_FALSE;
    }

    if (!hashString(string, strlen(string), &hash))
    {
        free(path);
        return GLFW_FALSE;
    }

    hash_part = strrchr(path, '/') + 1;
    *hash_part = '\0';
    mkdir(path, 0755);

    for (int i = 0; i < 16; i++)
    {
        sprintf(hash_part, "%2.2x", hash.digest[i]);
        hash_part += 2;
    }

    *cachePath = path;
    return GLFW_TRUE;
}

GLFWbool
cacheRead(const char *path, _GLFWmapping** out, int* count)
{
    FILE *f;
    int length, i;
    _GLFWmapping* mappings;
    _GLFWmapping* mapping;

#define FREAD(ptr, size, nmemb, stream) \
    do \
    { \
        if (fread(ptr, size, nmemb, stream) < nmemb) \
            return GLFW_FALSE; \
    } while (0)
#define FREAD_n(ptr, nmemb, stream) \
    FREAD(ptr, sizeof(*ptr), nmemb, stream)
#define FREAD_1(value, stream) \
    FREAD_n(&value, 1, stream)

    f = fopen(path, "rb");
    if (!f)
        return GLFW_FALSE;

    FREAD_1(length, f);
    mappings = malloc(length * sizeof(_GLFWmapping));
    if (!mappings)
        goto error;

    for (i = 0; i < length; ++i)
    {
        mapping = &mappings[i];
        FREAD_n(mapping->name, 128, f);
        FREAD_n(mapping->guid, 32, f);
        mapping->guid[32] = '\0';
        FREAD_n(mapping->buttons, 15, f);
        FREAD_n(mapping->axes, 6, f);
    }

#undef FREAD_1
#undef FREAD_n
#undef FREAD

    fclose(f);
    *count = length;
    *out = mappings;
    return GLFW_TRUE;

error:
    fclose(f);
    unlink(path);
    return GLFW_FALSE;
}

void
cacheWrite(const char *path, const _GLFWmapping* mappings, int count)
{
    FILE *f;
    int i;
    const _GLFWmapping* mapping;

#define FWRITE(ptr, size, nmemb, stream) \
    do \
    { \
        if (fwrite(ptr, size, nmemb, stream) < nmemb) \
            goto error; \
    } while (0)
#define FWRITE_n(ptr, nmemb, stream) \
    FWRITE(ptr, sizeof(*ptr), nmemb, stream)
#define FWRITE_1(value, stream) \
    FWRITE_n(&value, 1, stream)

    f = fopen(path, "wb");
    if (!f)
        return;

    FWRITE_1(count, f);
    for (i = 0; i < count; ++i) {
        mapping = &mappings[i];
        FWRITE_n(mapping->name, 128, f);
        FWRITE_n(mapping->guid, 32, f);
        FWRITE_n(mapping->buttons, 15, f);
        FWRITE_n(mapping->axes, 6, f);
    }

    if (fclose(f) != 0)
        goto fclose_error;

    return;

#undef FWRITE_1
#undef FWRITE_n
#undef FWRITE

error:
    fclose(f);
fclose_error:
    unlink(path);
}
