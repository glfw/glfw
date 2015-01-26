//========================================================================
// Context creation and information tool
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

#include <GLFW/glfw3.h>
#include <GL/glext.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getopt.h"

#ifdef _MSC_VER
#define strcasecmp(x, y) _stricmp(x, y)
#endif

#define API_NAME_OPENGL     "gl"
#define API_NAME_OPENGL_ES  "es"

#define PROFILE_NAME_CORE   "core"
#define PROFILE_NAME_COMPAT "compat"

#define STRATEGY_NAME_NONE  "none"
#define STRATEGY_NAME_LOSE  "lose"

#define BEHAVIOR_NAME_NONE  "none"
#define BEHAVIOR_NAME_FLUSH "flush"

static void usage(void)
{
    printf("Usage: glfwinfo [OPTION]...\n");
    printf("Options:\n");
    printf("  -a, --client-api=API      the client API to use ("
                                        API_NAME_OPENGL " or "
                                        API_NAME_OPENGL_ES ")\n");
    printf("  -b, --behavior=BEHAVIOR   the release behavior to use ("
                                        BEHAVIOR_NAME_NONE " or "
                                        BEHAVIOR_NAME_FLUSH ")\n");
    printf("  -d, --debug               request a debug context\n");
    printf("  -f, --forward             require a forward-compatible context\n");
    printf("  -h, --help                show this help\n");
    printf("  -l, --list-extensions     list all client API extensions\n");
    printf("  -m, --major=MAJOR         the major number of the required "
                                        "client API version\n");
    printf("  -n, --minor=MINOR         the minor number of the required "
                                        "client API version\n");
    printf("  -p, --profile=PROFILE     the OpenGL profile to use ("
                                        PROFILE_NAME_CORE " or "
                                        PROFILE_NAME_COMPAT ")\n");
    printf("  -s, --robustness=STRATEGY the robustness strategy to use ("
                                        STRATEGY_NAME_NONE " or "
                                        STRATEGY_NAME_LOSE ")\n");
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static const char* get_api_name(int api)
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

    printf("%s context supported extensions:\n", get_api_name(api));

    if (api == GLFW_OPENGL_API && major > 2)
    {
        PFNGLGETSTRINGIPROC glGetStringi =
            (PFNGLGETSTRINGIPROC) glfwGetProcAddress("glGetStringi");
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
    int ch, profile = 0, strategy = 0, behavior = 0;
    int api = 0, major = 1, minor = 0, revision;
    GLboolean debug = GL_FALSE, forward = GL_FALSE, list = GL_FALSE;
    GLint flags, mask;
    GLFWwindow* window;

    enum { API, BEHAVIOR, DEBUG, FORWARD, HELP, EXTENSIONS,
           MAJOR, MINOR, PROFILE, ROBUSTNESS };
    const struct option options[] =
    {
        { "behavior", 1, NULL, BEHAVIOR },
        { "client-api", 1, NULL, API },
        { "debug", 0, NULL, DEBUG },
        { "forward", 0, NULL, FORWARD },
        { "help", 0, NULL, HELP },
        { "list-extensions", 0, NULL, EXTENSIONS },
        { "major", 1, NULL, MAJOR },
        { "minor", 1, NULL, MINOR },
        { "profile", 1, NULL, PROFILE },
        { "robustness", 1, NULL, ROBUSTNESS },
        { NULL, 0, NULL, 0 }
    };

    while ((ch = getopt_long(argc, argv, "a:b:dfhlm:n:p:s:", options, NULL)) != -1)
    {
        switch (ch)
        {
            case 'a':
            case API:
                if (strcasecmp(optarg, API_NAME_OPENGL) == 0)
                    api = GLFW_OPENGL_API;
                else if (strcasecmp(optarg, API_NAME_OPENGL_ES) == 0)
                    api = GLFW_OPENGL_ES_API;
                else
                {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case 'b':
            case BEHAVIOR:
                if (strcasecmp(optarg, BEHAVIOR_NAME_NONE) == 0)
                    behavior = GLFW_RELEASE_BEHAVIOR_NONE;
                else if (strcasecmp(optarg, BEHAVIOR_NAME_FLUSH) == 0)
                    behavior = GLFW_RELEASE_BEHAVIOR_FLUSH;
                else
                {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case 'd':
            case DEBUG:
                debug = GL_TRUE;
                break;
            case 'f':
            case FORWARD:
                forward = GL_TRUE;
                break;
            case 'h':
            case HELP:
                usage();
                exit(EXIT_SUCCESS);
            case 'l':
            case EXTENSIONS:
                list = GL_TRUE;
                break;
            case 'm':
            case MAJOR:
                major = atoi(optarg);
                break;
            case 'n':
            case MINOR:
                minor = atoi(optarg);
                break;
            case 'p':
            case PROFILE:
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
            case 's':
            case ROBUSTNESS:
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

    // Initialize GLFW and create window

    if (!valid_version())
        exit(EXIT_FAILURE);

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    if (major != 1 || minor != 0)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
    }

    if (api)
        glfwWindowHint(GLFW_CLIENT_API, api);
    if (debug)
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    if (forward)
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    if (profile)
        glfwWindowHint(GLFW_OPENGL_PROFILE, profile);
    if (strategy)
        glfwWindowHint(GLFW_CONTEXT_ROBUSTNESS, strategy);
    if (behavior)
        glfwWindowHint(GLFW_CONTEXT_RELEASE_BEHAVIOR, behavior);

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
           get_api_name(api),
           glGetString(GL_VERSION));

    printf("%s context version parsed by GLFW: %u.%u.%u\n",
           get_api_name(api),
           major, minor, revision);

    // Report client API context properties

    if (api == GLFW_OPENGL_API)
    {
        if (major >= 3)
        {
            glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
            printf("%s context flags (0x%08x):", get_api_name(api), flags);

            if (flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
                printf(" forward-compatible");
            if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
                printf(" debug");
            if (flags & GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT_ARB)
                printf(" robustness");
            putchar('\n');

            printf("%s context flags parsed by GLFW:", get_api_name(api));

            if (glfwGetWindowAttrib(window, GLFW_OPENGL_FORWARD_COMPAT))
                printf(" forward-compatible");
            if (glfwGetWindowAttrib(window, GLFW_OPENGL_DEBUG_CONTEXT))
                printf(" debug");
            if (glfwGetWindowAttrib(window, GLFW_CONTEXT_ROBUSTNESS) == GLFW_LOSE_CONTEXT_ON_RESET)
                printf(" robustness");
            putchar('\n');
        }

        if (major >= 4 || (major == 3 && minor >= 2))
        {
            int profile = glfwGetWindowAttrib(window, GLFW_OPENGL_PROFILE);

            glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &mask);
            printf("%s profile mask (0x%08x): %s\n",
                   get_api_name(api),
                   mask,
                   get_profile_name_gl(mask));

            printf("%s profile mask parsed by GLFW: %s\n",
                   get_api_name(api),
                   get_profile_name_glfw(profile));
        }

        if (glfwExtensionSupported("GL_ARB_robustness"))
        {
            int robustness;
            GLint strategy;
            glGetIntegerv(GL_RESET_NOTIFICATION_STRATEGY_ARB, &strategy);

            printf("%s robustness strategy (0x%08x): %s\n",
                   get_api_name(api),
                   strategy,
                   get_strategy_name_gl(strategy));

            robustness = glfwGetWindowAttrib(window, GLFW_CONTEXT_ROBUSTNESS);

            printf("%s robustness strategy parsed by GLFW: %s\n",
                   get_api_name(api),
                   get_strategy_name_glfw(robustness));
        }
    }

    printf("%s context renderer string: \"%s\"\n",
           get_api_name(api),
           glGetString(GL_RENDERER));
    printf("%s context vendor string: \"%s\"\n",
           get_api_name(api),
           glGetString(GL_VENDOR));

    if (major >= 2)
    {
        printf("%s context shading language version: \"%s\"\n",
               get_api_name(api),
               glGetString(GL_SHADING_LANGUAGE_VERSION));
    }

    // Report client API extensions
    if (list)
        list_extensions(api, major, minor);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

