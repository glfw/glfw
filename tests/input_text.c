//========================================================================
// Input Test
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
// Copyright (c) Daijiro Fukuda <fukuda@clear-code.com>
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
// For font handiling, I reffered to https://github.com/Immediate-Mode-UI/Nuklear/wiki/Complete-font-guide.
// For nuklear handling, I reffered to tests/window.c.
//
// Currently, it is made for Japanese input only.
// You have to select the correct font to display Japanese texts.
// You can set TTF_FONT_FILEPATH to set default font or you can choose a font by GUI
// if fontconfig libary is enabled in X11 or Wayland.
//
// To handle other languages, you need to add correct ranges to nk_font_config.
//
//========================================================================

// Please comment out and set font filepath here to change default font
// #define TTF_FONT_FILEPATH ""

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdarg.h>

#define NK_IMPLEMENTATION
#define NK_INCLUDE_STANDARD_IO
#define NK_KEYSTATE_BASED_INPUT
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_BUTTON_TRIGGER_ON_RELEASE

// To increase the number of characters that can be entered at one time
#define NK_INPUT_MAX 64

#include <nuklear.h>

#define NK_GLFW_GL2_IMPLEMENTATION
#include <nuklear_glfw_gl2.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "getopt.h"

#if defined(FONTCONFIG_ENABLED)
    #include <fontconfig/fontconfig.h>
#endif

#define MAX_BUFFER_LEN 1024

// https://github.com/Immediate-Mode-UI/Nuklear/wiki/Complete-font-guide
// To handle other languages, you need to fix these ranges.
static nk_rune rangesJapan[] = {
    0x0020, 0x007E,
    0x3001, 0x3003,     /* 、。〃 */
    0x3005, 0x301F,     /* 々〆〇 and brackets */
    0x3036, 0x3036,     /* 〶 */
    0x3041, 0x309F,     /* Hiragana */
    0x30A0, 0x30FF,     /* Katakana */
    0x4E00, 0x9FFF,    /* All Kanji */
    0
};

#define MAX_FONTS_LEN 512
#define MAX_FONT_FAMILY_NAME_LEN 128
#define MAX_FONT_FILEPATH_LEN 256

static struct nk_font* currentFont;
static char** fontFamilyNames;
static char** fontFilePaths;
static int fontNum = 0;
static int currentFontIndex = 0;

static int currentIMEStatus = GLFW_FALSE;
#define MAX_PREDIT_LEN 128
static char preeditBuf[MAX_PREDIT_LEN] = "";

void usage(void)
{
    printf("Usage: input_text [-h] [-s]\n");
    printf("Options:\n");
    printf("  -s Use on-the-spot sytle on X11. This is ignored on other platforms.\n");
    printf("  -h Show this help\n");
}

static size_t encode_utf8(char* s, unsigned int ch)
{
    size_t count = 0;

    if (ch < 0x80)
        s[count++] = (char) ch;
    else if (ch < 0x800)
    {
        s[count++] = (ch >> 6) | 0xc0;
        s[count++] = (ch & 0x3f) | 0x80;
    }
    else if (ch < 0x10000)
    {
        s[count++] = (ch >> 12) | 0xe0;
        s[count++] = ((ch >> 6) & 0x3f) | 0x80;
        s[count++] = (ch & 0x3f) | 0x80;
    }
    else if (ch < 0x110000)
    {
        s[count++] = (ch >> 18) | 0xf0;
        s[count++] = ((ch >> 12) & 0x3f) | 0x80;
        s[count++] = ((ch >> 6) & 0x3f) | 0x80;
        s[count++] = (ch & 0x3f) | 0x80;
    }

    return count;
}

#if (!defined(FONTCONFIG_ENABLED) || defined(TTF_FONT_FILEPATH))
static void init_font_list()
{
    fontFamilyNames = (char**) malloc(sizeof(char*) * MAX_FONTS_LEN);
    fontFilePaths = (char**) malloc(sizeof(char*) * MAX_FONTS_LEN);

    fontFamilyNames[0] = "GLFW default";
    fontFilePaths[0] = "";
    fontNum++;

#if defined(TTF_FONT_FILEPATH)
    fontFamilyNames[fontNum] = "Custom";
    fontFilePaths[fontNum] = TTF_FONT_FILEPATH;
    currentFontIndex = fontNum;
    fontNum++;
#endif
}
#else
static void init_font_list()
{
    FcConfig* config = FcInitLoadConfigAndFonts();
    FcFontSet* fontset = FcConfigGetFonts(config, FcSetSystem);

    fontFamilyNames = (char**) malloc(sizeof(char*) * MAX_FONTS_LEN);
    fontFilePaths = (char**) malloc(sizeof(char*) * MAX_FONTS_LEN);

    fontFamilyNames[0] = "GLFW default";
    fontFilePaths[0] = "";
    fontNum++;

    if (!fontset)
    {
        printf("init_font_list failed.\n");
        FcConfigDestroy(config);
        return;
    }

    for (int i = 0; i < fontset->nfont; i++)
    {
        FcValue fvalue, dvalue;
        if (FcResultMatch == FcPatternGet(fontset->fonts[i], FC_FAMILY, 0, &fvalue))
        {
            if (FcResultMatch == FcPatternGet(fontset->fonts[i], FC_FILE, 0, &dvalue))
            {
                const char* familyName = (const char*) fvalue.u.s;
                const char* filePath = (const char*) dvalue.u.s;
                int existsFamily = GLFW_FALSE;
                int existingIndex = 0;

                if (!strstr(filePath, ".ttf"))
                {
                    continue;
                }

                for (int j = 1; j < fontNum; ++j)
                {
                    if (strcmp(fontFamilyNames[j], familyName) == 0)
                    {
                        existsFamily = GLFW_TRUE;
                        existingIndex = j;
                        break;
                    }
                }

                if (existsFamily)
                {
                    if (strstr(filePath, "regular") || strstr(filePath, "Regular"))
                    {
                        if (strlen(familyName) < MAX_FONT_FAMILY_NAME_LEN && strlen(filePath) < MAX_FONT_FILEPATH_LEN)
                        {
                            free(fontFamilyNames[existingIndex]);
                            fontFamilyNames[existingIndex] = (char*) malloc(sizeof(char) * (1 + strlen(familyName)));
                            strcpy(fontFamilyNames[existingIndex], familyName);

                            free(fontFilePaths[existingIndex]);
                            fontFilePaths[existingIndex] = (char*) malloc(sizeof(char) * (1 + strlen(filePath)));
                            strcpy(fontFilePaths[existingIndex], filePath);
                        }
                    }
                }
                else
                {
                    if (strlen(familyName) < MAX_FONT_FAMILY_NAME_LEN && strlen(filePath) < MAX_FONT_FILEPATH_LEN)
                    {
                        fontFamilyNames[fontNum] = (char*) malloc(sizeof(char) * (1 + strlen(familyName)));
                        strcpy(fontFamilyNames[fontNum], familyName);

                        fontFilePaths[fontNum] = (char*) malloc(sizeof(char) * (1 + strlen(filePath)));
                        strcpy(fontFilePaths[fontNum], filePath);

                        fontNum++;
                    }
                }

                if (MAX_FONTS_LEN <= fontNum)
                {
                    printf("MAX_FONTS_LEN reached. Could not load some fonts.\n");
                    break;
                }
            }
        }
    }

    FcConfigDestroy(config);
}
#endif

static void deinit_font_list()
{
#if !(!defined(FONTCONFIG_ENABLED) || defined(TTF_FONT_FILEPATH))
    for (int i = 1; i < fontNum; ++i)
    {
        free(fontFamilyNames[i]);
        free(fontFilePaths[i]);
    }
#endif
    free(fontFamilyNames);
    free(fontFilePaths);
}

// https://github.com/Immediate-Mode-UI/Nuklear/wiki/Complete-font-guide
static void update_font(struct nk_context* nk, float height)
{
    struct nk_font_atlas* atlas;

    nk_glfw3_font_stash_begin(&atlas);

    if (currentFontIndex == 0)
    {
        currentFont = nk_font_atlas_add_default(atlas, height, 0);
    }
    else
    {
        struct nk_font* new_font;
        struct nk_font_config cfg;
        cfg = nk_font_config(0);
        cfg.range = rangesJapan;
        cfg.oversample_h = 1;
        cfg.oversample_v = 1;
        cfg.pixel_snap = true;

        new_font = nk_font_atlas_add_from_file(atlas, fontFilePaths[currentFontIndex], height, &cfg);
        if (new_font)
        {
            currentFont = new_font;
            printf("Succeeded to load font file: %s\n", fontFilePaths[currentFontIndex]);
        }
        else
            printf("Failed to load font file: %s\n", fontFilePaths[currentFontIndex]);
    }

    nk_glfw3_font_stash_end();
    nk_style_set_font(nk, &currentFont->handle);
}

static void set_menu_buttons(GLFWwindow* window, struct nk_context* nk, int height)
{
    static int windowedX, windowedY, windowedWidth, windowedHeight;

    nk_layout_row_dynamic(nk, height, 2);
    if (nk_button_label(nk, "Toggle Fullscreen"))
    {
        if (glfwGetWindowMonitor(window))
        {
            glfwSetWindowMonitor(window, NULL,
                                 windowedX, windowedY,
                                 windowedWidth, windowedHeight, 0);
        }
        else
        {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwGetWindowPos(window, &windowedX, &windowedY);
            glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
            glfwSetWindowMonitor(window, monitor,
                                 0, 0, mode->width, mode->height,
                                 mode->refreshRate);
        }
    }

    {
        int auto_iconify = glfwGetWindowAttrib(window, GLFW_AUTO_ICONIFY);
        if (nk_checkbox_label(nk, "Auto Iconify", &auto_iconify))
            glfwSetWindowAttrib(window, GLFW_AUTO_ICONIFY, auto_iconify);
    }
}

static int set_font_selecter(GLFWwindow* window, struct nk_context* nk, int height, int fontHeight)
{
    int newSelectedIndex;

    nk_layout_row_begin(nk, NK_DYNAMIC, height, 2);

    nk_layout_row_push(nk, 1.f / 3.f);
    nk_label(nk, "Font", NK_TEXT_LEFT);

    nk_layout_row_push(nk, 2.f / 3.f);
    newSelectedIndex = nk_combo(nk, (const char**) fontFamilyNames, fontNum, currentFontIndex, fontHeight, nk_vec2(300, 400));

    nk_layout_row_end(nk);

    if (newSelectedIndex == currentFontIndex)
        return GLFW_FALSE;

    currentFontIndex = newSelectedIndex;
    return GLFW_TRUE;
}

static void set_ime_buttons(GLFWwindow* window, struct nk_context* nk, int height)
{
    nk_layout_row_dynamic(nk, height, 2);

    if (nk_button_label(nk, "Toggle IME status"))
    {
        glfwSetInputMode(window, GLFW_IME, !currentIMEStatus);
    }

    if (nk_button_label(nk, "Reset preedit text"))
    {
        glfwResetPreeditText(window);
    }
}

static void set_preedit_cursor_edit(GLFWwindow* window, struct nk_context* nk, int height, int* isAutoUpdating)
{
    static int lastX = -1, lastY = -1, lastH = -1;
    static char xBuf[12] = "", yBuf[12] = "", hBuf[12] = "";

    const nk_flags flags = NK_EDIT_FIELD |
                           NK_EDIT_SIG_ENTER |
                           NK_EDIT_GOTO_END_ON_ACTIVATE;
    nk_flags events;
    int x, y, h;

    glfwGetPreeditCursorPos(window, &x, &y, &h);

    if (x != lastX)
        sprintf(xBuf, "%i", x);
    if (y != lastY)
        sprintf(yBuf, "%i", y);
    if (h != lastH)
        sprintf(hBuf, "%i", h);

    nk_layout_row_begin(nk, NK_DYNAMIC, height, 5);

    nk_layout_row_push(nk, 4.f / 8.f);
    nk_label(nk, "Preedit cursor (x,y,h)", NK_TEXT_LEFT);

    nk_layout_row_push(nk, 1.f / 8.f);
    events = nk_edit_string_zero_terminated(nk, flags, xBuf,
                                            sizeof(xBuf),
                                            nk_filter_decimal);
    if (events & NK_EDIT_COMMITED)
    {
        x = atoi(xBuf);
        *isAutoUpdating = GLFW_FALSE;
        glfwSetPreeditCursorPos(window, x, y, h);
    }
    else if (events & NK_EDIT_DEACTIVATED)
        sprintf(xBuf, "%i", x);

    nk_layout_row_push(nk, 1.f / 8.f);
    events = nk_edit_string_zero_terminated(nk, flags, yBuf,
                                            sizeof(yBuf),
                                            nk_filter_decimal);
    if (events & NK_EDIT_COMMITED)
    {
        y = atoi(yBuf);
        *isAutoUpdating = GLFW_FALSE;
        glfwSetPreeditCursorPos(window, x, y, h);
    }
    else if (events & NK_EDIT_DEACTIVATED)
        sprintf(yBuf, "%i", y);

    nk_layout_row_push(nk, 1.f / 8.f);
    events = nk_edit_string_zero_terminated(nk, flags, hBuf,
                                            sizeof(hBuf),
                                            nk_filter_decimal);
    if (events & NK_EDIT_COMMITED)
    {
        h = atoi(hBuf);
        *isAutoUpdating = GLFW_FALSE;
        glfwSetPreeditCursorPos(window, x, y, h);
    }
    else if (events & NK_EDIT_DEACTIVATED)
        sprintf(hBuf, "%i", h);

    nk_layout_row_push(nk, 1.f / 8.f);
    nk_checkbox_label(nk, "Auto", isAutoUpdating);

    nk_layout_row_end(nk);

    lastX = x;
    lastY = y;
    lastH = h;
}

static void set_ime_stauts_labels(GLFWwindow* window, struct nk_context* nk, int height)
{
    nk_layout_row_dynamic(nk, height, 1);
    nk_value_bool(nk, "IME status", currentIMEStatus);
}

static void set_preedit_labels(GLFWwindow* window, struct nk_context* nk, int height)
{
    nk_layout_row_begin(nk, NK_DYNAMIC, height, 5);

    nk_layout_row_push(nk, 1.f / 3.f);
    nk_label(nk, "Preedit info:", NK_TEXT_LEFT);

    nk_layout_row_push(nk, 2.f / 3.f);
    nk_label(nk, (const char*) preeditBuf, NK_TEXT_LEFT);

    nk_layout_row_end(nk);
}

// If it is possible to take the text-cursor position calculated in `nk_do_edit` function in `deps/nuklear.h`,
// we can set preedit-cursor position more easily.
// However, there doesn't seem to be a way to do that, so this does a simplified calculation only for the end
// of the text. (Can not trace the cursor movement)
static void update_preedit_pos(GLFWwindow* window, struct nk_context* nk, struct nk_user_font* f, char* boxBuffer, int boxLen)
{
    float lineWidth = 0;
    int totalLines = 1;

    const char* text;
    int textPos = 0;

    struct nk_str nkString;
    nk_str_init_fixed(&nkString, boxBuffer, (nk_size) MAX_BUFFER_LEN);
    nkString.buffer.allocated = (nk_size) boxLen;
    nkString.len = nk_utf_len(boxBuffer, boxLen);

    text = nk_str_get_const(&nkString);

    while (textPos < boxLen)
    {
        nk_rune unicode = 0;
        int remainedBoxLen = boxLen - textPos;
        int nextGlyphSize = nk_utf_decode(text + textPos, &unicode, remainedBoxLen);
        if (!nextGlyphSize)
            break;

        if (unicode == '\n')
        {
            textPos++;
            totalLines++;
            lineWidth = 0;
            continue;
        }

        textPos += nextGlyphSize;
        lineWidth += f->width(f->userdata, f->height, text + textPos, nextGlyphSize);
    }

    {
        // I don't know how to get these info.
        int widgetLayoutX = 10;
        int widgetLayoutY = 220;

        int preeditPosX = widgetLayoutX + lineWidth;
        int preeditPosY = widgetLayoutY + totalLines * (f->height + nk->style.edit.row_padding);

        glfwSetPreeditCursorPos(window, preeditPosX, preeditPosY, 0);
    }
}

static void ime_callback(GLFWwindow* window)
{
    currentIMEStatus = glfwGetInputMode(window, GLFW_IME);
    printf("IME switched: %s\n", currentIMEStatus ? "ON" : "OFF");
}

static void preedit_callback(GLFWwindow* window, int strLength,
                             unsigned int* string, int blockLength,
                             int* blocks, int focusedBlock)
{
    int blockIndex = -1, blockCount = 0;
    if (strLength == 0 || blockLength == 0)
    {
        strcpy(preeditBuf, "(empty)");
        return;
    }

    strcpy(preeditBuf, "");

    for (int i = 0; i < strLength; i++)
    {
        char encoded[5] = "";

        if (blockCount == 0)
        {
            if (blockIndex == focusedBlock)
            {
                if (strlen(preeditBuf) + strlen("]") < MAX_PREDIT_LEN)
                    strcat(preeditBuf, "]");
            }
            blockIndex++;
            blockCount = blocks[blockIndex];
            if (blockIndex == focusedBlock)
            {
                if (strlen(preeditBuf) + strlen("[") < MAX_PREDIT_LEN)
                    strcat(preeditBuf, "[");
            }
        }
        encode_utf8(encoded, string[i]);
        if (strlen(preeditBuf) + strlen(encoded) < MAX_PREDIT_LEN)
            strcat(preeditBuf, encoded);
        blockCount--;
    }
    if (blockIndex == focusedBlock)
    {
        if (strlen(preeditBuf) + strlen("]") < MAX_PREDIT_LEN)
            strcat(preeditBuf, "]");
    }
}

int main(int argc, char** argv)
{
    GLFWwindow* window;
    struct nk_context* nk;
    int width, height;
    char boxBuffer[MAX_BUFFER_LEN] = "Input text here.\nここに入力してください。";
    int boxLen = strlen(boxBuffer);
    int isAutoUpdatingPreeditPosEnabled = GLFW_TRUE;
    int ch;

    while ((ch = getopt(argc, argv, "hs")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);

            case 's':
                glfwInitHint(GLFW_X11_ONTHESPOT, GLFW_TRUE);
                break;
        }
    }

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    glfwWindowHint(GLFW_WIN32_KEYBOARD_MENU, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    window = glfwCreateWindow(600, 600, "Input Text", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetIMEStatusCallback(window, ime_callback);
    glfwSetPreeditCallback(window, preedit_callback);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(0);

    nk = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS);
    init_font_list();
    update_font(nk, 18);

    while (!glfwWindowShouldClose(window))
    {
        struct nk_rect area;

        glfwGetWindowSize(window, &width, &height);

        area = nk_rect(0.f, 0.f, (float) width, (float) height);
        nk_window_set_bounds(nk, "main", area);

        nk_glfw3_new_frame();
        if (nk_begin(nk, "main", area, 0))
        {
            set_menu_buttons(window, nk, 30);
            if (set_font_selecter(window, nk, 30, 18))
                update_font(nk, 18);
            set_ime_buttons(window, nk, 30);
            set_preedit_cursor_edit(window, nk, 30, &isAutoUpdatingPreeditPosEnabled);
            set_ime_stauts_labels(window, nk, 30);
            set_preedit_labels(window, nk, 30);

            nk_layout_row_dynamic(nk, height - 250, 1);
            nk_edit_string(nk, NK_EDIT_BOX, boxBuffer, &boxLen, MAX_BUFFER_LEN, nk_filter_default);
        }
        nk_end(nk);

        glClear(GL_COLOR_BUFFER_BIT);
        nk_glfw3_render(NK_ANTI_ALIASING_ON);
        glfwSwapBuffers(window);

        if (isAutoUpdatingPreeditPosEnabled)
            update_preedit_pos(window, nk, &currentFont->handle, boxBuffer, boxLen);

        glfwWaitEvents();
    }

    deinit_font_list();

    nk_glfw3_shutdown();
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
