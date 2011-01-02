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
        // Printable keys
        case GLFW_KEY_A:            return "a";
        case GLFW_KEY_B:            return "b";
        case GLFW_KEY_C:            return "c";
        case GLFW_KEY_D:            return "d";
        case GLFW_KEY_E:            return "e";
        case GLFW_KEY_F:            return "f";
        case GLFW_KEY_G:            return "g";
        case GLFW_KEY_H:            return "h";
        case GLFW_KEY_I:            return "i";
        case GLFW_KEY_J:            return "j";
        case GLFW_KEY_K:            return "k";
        case GLFW_KEY_L:            return "l";
        case GLFW_KEY_M:            return "m";
        case GLFW_KEY_N:            return "n";
        case GLFW_KEY_O:            return "o";
        case GLFW_KEY_P:            return "p";
        case GLFW_KEY_Q:            return "q";
        case GLFW_KEY_R:            return "r";
        case GLFW_KEY_S:            return "s";
        case GLFW_KEY_T:            return "t";
        case GLFW_KEY_U:            return "u";
        case GLFW_KEY_V:            return "v";
        case GLFW_KEY_W:            return "w";
        case GLFW_KEY_X:            return "x";
        case GLFW_KEY_Y:            return "y";
        case GLFW_KEY_Z:            return "z";
        case GLFW_KEY_1:            return "1";
        case GLFW_KEY_2:            return "2";
        case GLFW_KEY_3:            return "3";
        case GLFW_KEY_4:            return "4";
        case GLFW_KEY_5:            return "5";
        case GLFW_KEY_6:            return "6";
        case GLFW_KEY_7:            return "7";
        case GLFW_KEY_8:            return "8";
        case GLFW_KEY_9:            return "9";
        case GLFW_KEY_0:            return "0";
        case GLFW_KEY_SPACE:        return "space";
        case GLFW_KEY_MINUS:        return "-";
        case GLFW_KEY_EQUAL:        return "=";
        case GLFW_KEY_LEFT_SQUARE_BRACKET: return "[";
        case GLFW_KEY_RIGHT_SQUARE_BRACKET: return "]";
        case GLFW_KEY_BACKSLASH:    return "\\";
        case GLFW_KEY_SEMICOLON:    return ";";
        case GLFW_KEY_APOSTROPHE:   return "'";
        case GLFW_KEY_GRAVE_ACCENT: return "`";
        case GLFW_KEY_COMMA:        return ",";
        case GLFW_KEY_PERIOD:       return ".";
        case GLFW_KEY_SLASH:        return "/";
        case GLFW_KEY_WORLD_1:      return "world 1";
        case GLFW_KEY_WORLD_2:      return "world 2";

        // Special keys
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
        case GLFW_KEY_LEFT_SHIFT:   return "left shift";
        case GLFW_KEY_RIGHT_SHIFT:  return "right shift";
        case GLFW_KEY_LEFT_CONTROL: return "left control";
        case GLFW_KEY_RIGHT_CONTROL: return "right control";
        case GLFW_KEY_LEFT_ALT:     return "left alt";
        case GLFW_KEY_RIGHT_ALT:    return "right alt";
        case GLFW_KEY_TAB:          return "tab";
        case GLFW_KEY_ENTER:        return "enter";
        case GLFW_KEY_BACKSPACE:    return "backspace";
        case GLFW_KEY_INSERT:       return "insert";
        case GLFW_KEY_DELETE:       return "delete";
        case GLFW_KEY_PAGE_UP:      return "page up";
        case GLFW_KEY_PAGE_DOWN:    return "page down";
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
        case GLFW_KEY_NUM_LOCK:     return "num lock";
        case GLFW_KEY_CAPS_LOCK:    return "caps lock";
        case GLFW_KEY_SCROLL_LOCK:  return "scroll lock";
        case GLFW_KEY_PAUSE:        return "pause";
        case GLFW_KEY_LEFT_SUPER:   return "left super";
        case GLFW_KEY_RIGHT_SUPER:  return "right super";
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
    else
        printf(" was %s\n", get_action_name(action));

    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_R:
        {
            keyrepeat = !keyrepeat;
            if (keyrepeat)
                glfwEnable(window, GLFW_KEY_REPEAT);
            else
                glfwDisable(window, GLFW_KEY_REPEAT);

            printf("(( key repeat %s ))\n", keyrepeat ? "enabled" : "disabled");
            break;
        }

        case GLFW_KEY_S:
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

    glfwSetWindowSizeCallback(window_size_callback);
    glfwSetWindowCloseCallback(window_close_callback);
    glfwSetWindowRefreshCallback(window_refresh_callback);
    glfwSetWindowFocusCallback(window_focus_callback);
    glfwSetWindowIconifyCallback(window_iconify_callback);
    glfwSetMouseButtonCallback(mouse_button_callback);
    glfwSetMousePosCallback(mouse_position_callback);
    glfwSetScrollCallback(scroll_callback);
    glfwSetKeyCallback(key_callback);
    glfwSetCharCallback(char_callback);

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

