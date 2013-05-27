//========================================================================
// Version information dumper
// Copyright (c) Camilla Berglund <elmindreda@elmindreda.org>
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
//
// This test is a pale imitation of glxinfo(1), except not really
//
// It dumps GLFW and OpenGL version information
//
//========================================================================

#include <GLFW/glfw3.h>
#include <GL/glext.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getopt.h"

#ifdef _MSC_VER
#define strcasecmp(x, y) _stricmp(x, y)
#endif

#define API_OPENGL          "gl"
#define API_OPENGL_ES       "es"

#define PROFILE_NAME_CORE   "core"
#define PROFILE_NAME_COMPAT "compat"

#define STRATEGY_NAME_NONE "none"
#define STRATEGY_NAME_LOSE "lose"

static void usage(void)
{
    printf("Usage: glfwinfo [-h] [-a API] [-m MAJOR] [-n MINOR] [-d] [-l] [-f] [-p PROFILE] [-r STRATEGY]\n");
    printf("available APIs: " API_OPENGL " " API_OPENGL_ES "\n");
    printf("available profiles: " PROFILE_NAME_CORE " " PROFILE_NAME_COMPAT "\n");
    printf("available strategies: " STRATEGY_NAME_NONE " " STRATEGY_NAME_LOSE "\n");
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static const char* get_client_api_name(int api)
{
    if (api == GLFW_OPENGL_API)
        return "OpenGL";
    else if (api == GLFW_OPENGL_ES_API)
        return "OpenGL ES";

    return "Unknown API";
}

static const char* get_profile_name_gl(GLint mask)
{
    if (mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
        return PROFILE_NAME_COMPAT;
    if (mask & GL_CONTEXT_CORE_PROFILE_BIT)
        return PROFILE_NAME_CORE;

    return "unknown";
}

static const char* get_profile_name_glfw(int profile)
{
    if (profile == GLFW_OPENGL_COMPAT_PROFILE)
        return PROFILE_NAME_COMPAT;
    if (profile == GLFW_OPENGL_CORE_PROFILE)
        return PROFILE_NAME_CORE;

    return "unknown";
}

static const char* get_strategy_name_gl(GLint strategy)
{
    if (strategy == GL_LOSE_CONTEXT_ON_RESET_ARB)
        return STRATEGY_NAME_LOSE;
    if (strategy == GL_NO_RESET_NOTIFICATION_ARB)
        return STRATEGY_NAME_NONE;

    return "unknown";
}

static const char* get_strategy_name_glfw(int strategy)
{
    if (strategy == GLFW_LOSE_CONTEXT_ON_RESET)
        return STRATEGY_NAME_LOSE;
    if (strategy == GLFW_NO_RESET_NOTIFICATION)
        return STRATEGY_NAME_NONE;

    return "unknown";
}

static void list_extensions(int api, int major, int minor)
{
    int i;
    GLint count;
    const GLubyte* extensions;

    printf("%s context supported extensions:\n", get_client_api_name(api));

    if (api == GLFW_OPENGL_API && major > 2)
    {
        PFNGLGETSTRINGIPROC glGetStringi = (PFNGLGETSTRINGIPROC) glfwGetProcAddress("glGetStringi");
        if (!glGetStringi)
        {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        glGetIntegerv(GL_NUM_EXTENSIONS, &count);

        for (i = 0;  i < count;  i++)
            puts((const char*) glGetStringi(GL_EXTENSIONS, i));
    }
    else
    {
        extensions = glGetString(GL_EXTENSIONS);
        while (*extensions != '\0')
        {
            if (*extensions == ' ')
                putchar('\n');
            else
                putchar(*extensions);

            extensions++;
        }
    }

    putchar('\n');
}

static GLboolean valid_version(void)
{
    int major, minor, revision;

    glfwGetVersion(&major, &minor, &revision);

    printf("GLFW header version: %u.%u.%u\n",
           GLFW_VERSION_MAJOR,
           GLFW_VERSION_MINOR,
           GLFW_VERSION_REVISION);

    printf("GLFW library version: %u.%u.%u\n", major, minor, revision);

    if (major != GLFW_VERSION_MAJOR)
    {
        printf("*** ERROR: GLFW major version mismatch! ***\n");
        return GL_FALSE;
    }

    if (minor != GLFW_VERSION_MINOR || revision != GLFW_VERSION_REVISION)
        printf("*** WARNING: GLFW version mismatch! ***\n");

    printf("GLFW library version string: \"%s\"\n", glfwGetVersionString());
    return GL_TRUE;
}

int main(int argc, char** argv)
{
    int ch, api = 0, profile = 0, strategy = 0, major = 1, minor = 0, revision;
    GLboolean debug = GL_FALSE, forward = GL_FALSE, list = GL_FALSE;
    GLint flags, mask;
    GLFWwindow* window;

    if (!valid_version())
        exit(EXIT_FAILURE);

    while ((ch = getopt(argc, argv, "a:dfhlm:n:p:r:")) != -1)
    {
        switch (ch)
        {
            case 'a':
                if (strcasecmp(optarg, API_OPENGL) == 0)
                    api = GLFW_OPENGL_API;
                else if (strcasecmp(optarg, API_OPENGL_ES) == 0)
                    api = GLFW_OPENGL_ES_API;
                else
                {
                    usage();
                    exit(EXIT_FAILURE);
                }
            case 'd':
                debug = GL_TRUE;
                break;
            case 'f':
                forward = GL_TRUE;
                break;
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case 'l':
                list = GL_TRUE;
                break;
            case 'm':
                major = atoi(optarg);
                break;
            case 'n':
                minor = atoi(optarg);
                break;
            case 'p':
                if (strcasecmp(optarg, PROFILE_NAME_CORE) == 0)
                    profile = GLFW_OPENGL_CORE_PROFILE;
                else if (strcasecmp(optarg, PROFILE_NAME_COMPAT) == 0)
                    profile = GLFW_OPENGL_COMPAT_PROFILE;
                else
                {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case 'r':
                if (strcasecmp(optarg, STRATEGY_NAME_NONE) == 0)
                    strategy = GLFW_NO_RESET_NOTIFICATION;
                else if (strcasecmp(optarg, STRATEGY_NAME_LOSE) == 0)
                    strategy = GLFW_LOSE_CONTEXT_ON_RESET;
                else
                {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    argc -= optind;
    argv += optind;

    // Initialize GLFW and create window

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    if (major != 1 || minor != 0)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
    }

    if (api != 0)
        glfwWindowHint(GLFW_CLIENT_API, api);

    if (debug)
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    if (forward)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    if (profile != 0)
        glfwWindowHint(GLFW_OPENGL_PROFILE, profile);

    if (strategy)
        glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, strategy);

    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

    window = glfwCreateWindow(200, 200, "Version", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    // Report client API version

    api = glfwGetWindowAttrib(window, GLFW_CLIENT_API);
    major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    revision = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);

    printf("%s context version string: \"%s\"\n",
           get_client_api_name(api),
           glGetString(GL_VERSION));

    printf("%s context version parsed by GLFW: %u.%u.%u\n",
           get_client_api_name(api),
           major, minor, revision);

    // Report client API context properties

    if (api == GLFW_OPENGL_API)
    {
        if (major >= 3)
        {
            glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
            printf("%s context flags (0x%08x):", get_client_api_name(api), flags);

            if (flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
                printf(" forward-compatible");
            if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
                printf(" debug");
            if (flags & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT_ARB)
                printf(" robustness");
            putchar('\n');

            printf("%s context flags parsed by GLFW:", get_client_api_name(api));

            if (glfwGetWindowAttrib(window, GLFW_OPENGL_FORWARD_COMPAT))
                printf(" forward-compatible");
            if (glfwGetWindowAttrib(window, GLFW_OPENGL_DEBUG_CONTEXT))
                printf(" debug");
            if (glfwGetWindowAttrib(window, GLFW_CONTEXT_ROBUSTNESS) != GLFW_NO_ROBUSTNESS)
                printf(" robustness");
            putchar('\n');
        }

        if (major > 3 || (major == 3 && minor >= 2))
        {
            int profile = glfwGetWindowAttrib(window, GLFW_OPENGL_PROFILE);

            glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &mask);
            printf("%s profile mask (0x%08x): %s\n",
                   get_client_api_name(api),
                   mask,
                   get_profile_name_gl(mask));

            printf("%s profile mask parsed by GLFW: %s\n",
                   get_client_api_name(api),
                   get_profile_name_glfw(profile));
        }

        if (glfwExtensionSupported("GL_ARB_robustness"))
        {
            int robustness;
            GLint strategy;
            glGetIntegerv(GL_RESET_NOTIFICATION_STRATEGY_ARB, &strategy);

            printf("%s robustness strategy (0x%08x): %s\n",
                   get_client_api_name(api),
                   strategy,
                   get_strategy_name_gl(strategy));

            robustness = glfwGetWindowAttrib(window, GLFW_CONTEXT_ROBUSTNESS);

            printf("%s robustness strategy parsed by GLFW: %s\n",
                   get_client_api_name(api),
                   get_strategy_name_glfw(robustness));
        }
    }

    printf("%s context renderer string: \"%s\"\n",
           get_client_api_name(api),
           glGetString(GL_RENDERER));
    printf("%s context vendor string: \"%s\"\n",
           get_client_api_name(api),
           glGetString(GL_VENDOR));

    if (major > 1)
    {
        printf("%s context shading language version: \"%s\"\n",
               get_client_api_name(api),
               glGetString(GL_SHADING_LANGUAGE_VERSION));
    }

    // Report client API extensions
    if (list)
        list_extensions(api, major, minor);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

