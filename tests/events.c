//========================================================================
// Event linter (event spewer)
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
// This test hooks every available callback and outputs their arguments
//
// Log messages go to stdout, error messages to stderr
//
// Every event also gets a (sequential) number to aid discussion of logs
//
//========================================================================

#include <GL/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>

static GLboolean keyrepeat  = 0;
static GLboolean systemkeys = 1;
static unsigned int counter = 0;

static const char* get_key_name(int key)
{
    switch (key)
    {
        case GLFW_KEY_UNKNOWN:      return "unknown";
        case GLFW_KEY_SPACE:        return "space";
        case GLFW_KEY_ESC:          return "escape";
        case GLFW_KEY_F1:           return "F1";
        case GLFW_KEY_F2:           return "F2";
        case GLFW_KEY_F3:           return "F3";
        case GLFW_KEY_F4:           return "F4";
        case GLFW_KEY_F5:           return "F5";
        case GLFW_KEY_F6:           return "F6";
        case GLFW_KEY_F7:           return "F7";
        case GLFW_KEY_F8:           return "F8";
        case GLFW_KEY_F9:           return "F9";
        case GLFW_KEY_F10:          return "F10";
        case GLFW_KEY_F11:          return "F11";
        case GLFW_KEY_F12:          return "F12";
        case GLFW_KEY_F13:          return "F13";
        case GLFW_KEY_F14:          return "F14";
        case GLFW_KEY_F15:          return "F15";
        case GLFW_KEY_F16:          return "F16";
        case GLFW_KEY_F17:          return "F17";
        case GLFW_KEY_F18:          return "F18";
        case GLFW_KEY_F19:          return "F19";
        case GLFW_KEY_F20:          return "F20";
        case GLFW_KEY_F21:          return "F21";
        case GLFW_KEY_F22:          return "F22";
        case GLFW_KEY_F23:          return "F23";
        case GLFW_KEY_F24:          return "F24";
        case GLFW_KEY_F25:          return "F25";
        case GLFW_KEY_UP:           return "up";
        case GLFW_KEY_DOWN:         return "down";
        case GLFW_KEY_LEFT:         return "left";
        case GLFW_KEY_RIGHT:        return "right";
        case GLFW_KEY_LSHIFT:       return "left shift";
        case GLFW_KEY_RSHIFT:       return "right shift";
        case GLFW_KEY_LCTRL:        return "left control";
        case GLFW_KEY_RCTRL:        return "right control";
        case GLFW_KEY_LALT:         return "left alt";
        case GLFW_KEY_RALT:         return "right alt";
        case GLFW_KEY_TAB:          return "tab";
        case GLFW_KEY_ENTER:        return "enter";
        case GLFW_KEY_BACKSPACE:    return "backspace";
        case GLFW_KEY_INSERT:       return "insert";
        case GLFW_KEY_DEL:          return "delete";
        case GLFW_KEY_PAGEUP:       return "page up";
        case GLFW_KEY_PAGEDOWN:     return "page down";
        case GLFW_KEY_HOME:         return "home";
        case GLFW_KEY_END:          return "end";
        case GLFW_KEY_KP_0:         return "keypad 0";
        case GLFW_KEY_KP_1:         return "keypad 1";
        case GLFW_KEY_KP_2:         return "keypad 2";
        case GLFW_KEY_KP_3:         return "keypad 3";
        case GLFW_KEY_KP_4:         return "keypad 4";
        case GLFW_KEY_KP_5:         return "keypad 5";
        case GLFW_KEY_KP_6:         return "keypad 6";
        case GLFW_KEY_KP_7:         return "keypad 7";
        case GLFW_KEY_KP_8:         return "keypad 8";
        case GLFW_KEY_KP_9:         return "keypad 9";
        case GLFW_KEY_KP_DIVIDE:    return "keypad divide";
        case GLFW_KEY_KP_MULTIPLY:  return "keypad multiply";
        case GLFW_KEY_KP_SUBTRACT:  return "keypad subtract";
        case GLFW_KEY_KP_ADD:       return "keypad add";
        case GLFW_KEY_KP_DECIMAL:   return "keypad decimal";
        case GLFW_KEY_KP_EQUAL:     return "keypad equal";
        case GLFW_KEY_KP_ENTER:     return "keypad enter";
        case GLFW_KEY_KP_NUM_LOCK:  return "keypad num lock";
        case GLFW_KEY_CAPS_LOCK:    return "caps lock";
        case GLFW_KEY_SCROLL_LOCK:  return "scroll lock";
        case GLFW_KEY_PAUSE:        return "pause";
        case GLFW_KEY_LSUPER:       return "left super";
        case GLFW_KEY_RSUPER:       return "right super";
        case GLFW_KEY_MENU:         return "menu";
    }

    return NULL;
}

static const char* get_action_name(int action)
{
    switch (action)
    {
        case GLFW_PRESS:
            return "was pressed";
        case GLFW_RELEASE:
            return "was released";
    }

    return "caused unknown action";
}

static const char* get_button_name(int button)
{
    switch (button)
    {
        case GLFW_MOUSE_BUTTON_LEFT:
            return "left";
        case GLFW_MOUSE_BUTTON_RIGHT:
            return "right";
        case GLFW_MOUSE_BUTTON_MIDDLE:
            return "middle";
    }

    return NULL;
}

static const char* get_character_string(int character)
{
    // This assumes UTF-8, which is stupid
    static char result[6 + 1];

    int length = wctomb(result, character);
    if (length == -1)
        length = 0;

    result[length] = '\0';
    return result;
}

static void window_size_callback(GLFWwindow window, int width, int height)
{
    printf("%08x at %0.3f: Window size: %i %i\n",
           counter++,
           glfwGetTime(),
           width,
           height);

    glViewport(0, 0, width, height);
}

static int window_close_callback(GLFWwindow window)
{
    printf("%08x at %0.3f: Window close\n", counter++, glfwGetTime());
    return 1;
}

static void window_refresh_callback(GLFWwindow window)
{
    printf("%08x at %0.3f: Window refresh\n", counter++, glfwGetTime());
}

static void window_focus_callback(GLFWwindow window, int activated)
{
    printf("%08x at %0.3f: Window %s\n",
           counter++,
           glfwGetTime(),
           activated ? "activated" : "deactivated");
}

static void window_iconify_callback(GLFWwindow window, int iconified)
{
    printf("%08x at %0.3f: Window was %s\n",
           counter++,
           glfwGetTime(),
           iconified ? "iconified" : "restored");
}

static void mouse_button_callback(GLFWwindow window, int button, int action)
{
    const char* name = get_button_name(button);

    printf("%08x at %0.3f: Mouse button %i", counter++, glfwGetTime(), button);

    if (name)
        printf(" (%s) was %s\n", name, get_action_name(action));
    else
        printf(" was %s\n", get_action_name(action));
}

static void mouse_position_callback(GLFWwindow window, int x, int y)
{
    printf("%08x at %0.3f: Mouse position: %i %i\n", counter++, glfwGetTime(), x, y);
}

static void scroll_callback(GLFWwindow window, int x, int y)
{
    printf("%08x at %0.3f: Scroll: %i %i\n", counter++, glfwGetTime(), x, y);
}

static void key_callback(GLFWwindow window, int key, int action)
{
    const char* name = get_key_name(key);

    printf("%08x at %0.3f: Key 0x%04x", counter++, glfwGetTime(), key);

    if (name)
        printf(" (%s) was %s\n", name, get_action_name(action));
    else if (isgraph(key))
        printf(" (%c) was %s\n", key, get_action_name(action));
    else
        printf(" was %s\n", get_action_name(action));

    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case 'R':
        {
            keyrepeat = !keyrepeat;
            if (keyrepeat)
                glfwEnable(window, GLFW_KEY_REPEAT);
            else
                glfwDisable(window, GLFW_KEY_REPEAT);

            printf("(( key repeat %s ))\n", keyrepeat ? "enabled" : "disabled");
            break;
        }

        case 'S':
        {
            systemkeys = !systemkeys;
            if (systemkeys)
                glfwEnable(window, GLFW_SYSTEM_KEYS);
            else
                glfwDisable(window, GLFW_SYSTEM_KEYS);

            printf("(( system keys %s ))\n", systemkeys ? "enabled" : "disabled");
            break;
        }
    }
}

static void char_callback(GLFWwindow window, int character)
{
    printf("%08x at %0.3f: Character 0x%04x (%s) input\n",
           counter++,
           glfwGetTime(),
           character,
           get_character_string(character));
}

int main(void)
{
    GLFWwindow window;

    setlocale(LC_ALL, "");

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW: %s\n", glfwErrorString(glfwGetError()));
        exit(1);
    }

    printf("Library initialized\n");

    window = glfwOpenWindow(0, 0, GLFW_WINDOWED, "Event Linter", NULL);
    if (!window)
    {
        glfwTerminate();

        fprintf(stderr, "Failed to open GLFW window: %s\n", glfwErrorString(glfwGetError()));
        exit(1);
    }

    printf("Window opened\n");

    glfwSwapInterval(1);

    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetWindowCloseCallback(window, window_close_callback);
    glfwSetWindowRefreshCallback(window, window_refresh_callback);
    glfwSetWindowFocusCallback(window, window_focus_callback);
    glfwSetWindowIconifyCallback(window, window_iconify_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetMousePosCallback(window, mouse_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);

    printf("Key repeat should be %s\n", keyrepeat ? "enabled" : "disabled");
    printf("System keys should be %s\n", systemkeys ? "enabled" : "disabled");

    printf("Main loop starting\n");

    while (glfwIsWindow(window) == GL_TRUE)
    {
        glfwWaitEvents();
        glClear(GL_COLOR_BUFFER_BIT);
        glfwSwapBuffers();
        glfwPollEvents();
    }

    glfwTerminate();
    exit(0);
}

