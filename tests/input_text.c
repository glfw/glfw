//========================================================================
// Input Test
// Copyright (c) Camilla Löwy <elmindreda@glfw.org>
// Copyright (c) Daijiro Fukuda <fukuda@clear-code.com>
// Copyright (c) Takuro Ashie <ashie@clear-code.com>
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
// To handle other languages, you need to add correct ranges to nk_font_config.
// 
// On X11 or Wayland, you can choose a font by GUI if "fontconfig" libarary is enabled.
// 
// On Win32, "Yu Mincho" is selected by default if it is installed. This font is
// included in the FOD packages, so it will be installed automatically when you
// enable Japanese input on your environment, or you can install it by
// "Manage optional features" in "Apps & features".
// Refer: https://learn.microsoft.com/en-us/typography/fonts/windows_10_font_list#japanese-supplemental-fonts
// 
// On macOS, "Arial Unicode MS" is selected by default if it is installed.
// I assume that this font is usually installed, but if it is not installed,
// please install it manually.
// 
// You can also specify a TTF filepath and use your own favorite font by setting
// TTF_FONT_FILEPATH below.
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
// https://unicode-table.com
// To handle other languages, you need to fix these ranges.
static nk_rune rangesJapan[] = {
    0x0020, 0x007E, // Basic Latin
    0x2000, 0x206F, // General Punctuation
    0x3000, 0x303F, // CJK Symbols and Punctuation
    0x3041, 0x309F, // Hiragana
    0x30A0, 0x30FF, // Katakana
    0x4E00, 0x9FFF, // All Kanji
    0xFF01, 0xFFEF, // Halfwidth and Fullwidth Forms
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
#define MAX_PREEDIT_LEN 128
static char preeditBuf[MAX_PREEDIT_LEN] = "";

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

static int add_font(const char* familyName, const char* ttfFilePath, int checkExistence)
{
    if (MAX_FONTS_LEN <= fontNum)
        return GLFW_FALSE;

    if (MAX_FONT_FAMILY_NAME_LEN <= strlen(familyName) || MAX_FONT_FILEPATH_LEN <= strlen(ttfFilePath))
        return GLFW_FALSE;

    if (checkExistence)
    {
        FILE* fp = fopen(ttfFilePath, "rb");
        if (!fp)
            return GLFW_FALSE;
        fclose(fp);
    }

    fontFamilyNames[fontNum] = (char*) malloc(1 + strlen(familyName));
    assert(fontFamilyNames[fontNum]);
    strcpy(fontFamilyNames[fontNum], familyName);

    fontFilePaths[fontNum] = (char*) malloc(1 + strlen(ttfFilePath));
    assert(fontFilePaths[fontNum]);
    strcpy(fontFilePaths[fontNum], ttfFilePath);

    fontNum++;

    return GLFW_TRUE;
}

static int replace_font(int index, const char* familyName, const char* ttfFilePath, int checkExistence)
{
    if (index == 0 || fontNum <= index)
        return GLFW_FALSE;
    if (MAX_FONT_FAMILY_NAME_LEN <= strlen(familyName) || MAX_FONT_FILEPATH_LEN <= strlen(ttfFilePath))
        return GLFW_FALSE;

    if (checkExistence)
    {
        FILE* fp = fopen(ttfFilePath, "rb");
        if (!fp)
            return GLFW_FALSE;
        fclose(fp);
    }

    free(fontFamilyNames[index]);
    free(fontFilePaths[index]);

    fontFamilyNames[index] = (char*) malloc(1 + strlen(familyName));
    assert(fontFamilyNames[index]);
    strcpy(fontFamilyNames[index], familyName);

    fontFilePaths[index] = (char*) malloc(1 + strlen(ttfFilePath));
    assert(fontFilePaths[index]);
    strcpy(fontFilePaths[index], ttfFilePath);

    return GLFW_TRUE;
}

#if defined(TTF_FONT_FILEPATH)
static int load_custom_font()
{
    if (MAX_FONTS_LEN <= fontNum)
        return GLFW_FALSE;
    if (!(TTF_FONT_FILEPATH && *TTF_FONT_FILEPATH))
        return GLFW_FALSE;

    return add_font("Custom", TTF_FONT_FILEPATH, GLFW_TRUE);
}
#endif

#if defined(FONTCONFIG_ENABLED)
static void load_font_list_by_fontconfig()
{
    FcConfig* config = FcInitLoadConfigAndFonts();
    FcFontSet* fontset = FcConfigGetFonts(config, FcSetSystem);

    if (!fontset)
    {
        printf("load_font_list_by_fontconfig failed.\n");
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
                    // Prefer "regular" to the others.
                    if (strstr(filePath, "regular") || strstr(filePath, "Regular"))
                        replace_font(existingIndex, familyName, filePath, GLFW_FALSE);
                }
                else
                    add_font(familyName, filePath, GLFW_FALSE);

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

static void load_default_font_for_each_platform()
{
    int hasSucceeded = GLFW_FALSE;
    if (MAX_FONTS_LEN <= fontNum)
        return;

    if (glfwGetPlatform() == GLFW_PLATFORM_COCOA)
        hasSucceeded = add_font("Arial Unicode MS", "/Library/Fonts/Arial Unicode.ttf", GLFW_TRUE);
    else if(glfwGetPlatform() == GLFW_PLATFORM_WIN32)
    {
        // Use "Yu Mincho" since it is the only TTF for Japanese in the FOD packages on Windows10 and Windows11.
        // https://learn.microsoft.com/en-us/typography/fonts/windows_10_font_list#japanese-supplemental-fonts
        char filepath[MAX_FONT_FILEPATH_LEN];
        char* winDir = getenv("systemroot");
        if (winDir)
            snprintf(filepath, MAX_FONT_FILEPATH_LEN, "%s\\Fonts\\Yumin.ttf", winDir);
        else
            strcpy(filepath, "C:\\Windows\\Fonts\\Yumin.ttf");
        hasSucceeded = add_font("Yu Mincho Regular", filepath, GLFW_TRUE);
    }

    if (hasSucceeded)
        currentFontIndex = fontNum - 1;
}

static void init_font_list()
{
    int useCustomFont = GLFW_FALSE;
    int customFontIndex = 0;

    fontFamilyNames = (char**) malloc(sizeof(char*) * MAX_FONTS_LEN);
    assert(fontFamilyNames);
    fontFilePaths = (char**) malloc(sizeof(char*) * MAX_FONTS_LEN);
    assert(fontFilePaths);

    fontFamilyNames[0] = "GLFW default";
    fontFilePaths[0] = "";
    fontNum++;

#if defined(TTF_FONT_FILEPATH)
    useCustomFont = load_custom_font();
    if (useCustomFont)
        customFontIndex = fontNum - 1;
#endif

    load_default_font_for_each_platform();

#if defined(FONTCONFIG_ENABLED)
    load_font_list_by_fontconfig();
#endif

    if (useCustomFont)
        currentFontIndex = customFontIndex;
}

static void deinit_font_list()
{
    for (int i = 1; i < fontNum; ++i)
    {
        free(fontFamilyNames[i]);
        free(fontFilePaths[i]);
    }

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
    static int lastX = -1, lastY = -1, lastW = -1, lastH = -1;
    static char xBuf[12] = "", yBuf[12] = "", wBuf[12] = "", hBuf[12] = "";

    const nk_flags flags = NK_EDIT_FIELD |
                           NK_EDIT_SIG_ENTER |
                           NK_EDIT_GOTO_END_ON_ACTIVATE;
    nk_flags events;
    int x, y, w, h;

    glfwGetPreeditCursorRectangle(window, &x, &y, &w, &h);

    if (x != lastX)
        sprintf(xBuf, "%i", x);
    if (y != lastY)
        sprintf(yBuf, "%i", y);
    if (w != lastW)
        sprintf(wBuf, "%i", w);
    if (h != lastH)
        sprintf(hBuf, "%i", h);

    nk_layout_row_begin(nk, NK_DYNAMIC, height, 5);

    nk_layout_row_push(nk, 4.f / 9.f);
    nk_label(nk, "Preedit cursor (x,y,w,h)", NK_TEXT_LEFT);

    nk_layout_row_push(nk, 1.f / 9.f);
    events = nk_edit_string_zero_terminated(nk, flags, xBuf,
                                            sizeof(xBuf),
                                            nk_filter_decimal);
    if (events & NK_EDIT_COMMITED)
    {
        x = atoi(xBuf);
        *isAutoUpdating = GLFW_FALSE;
        glfwSetPreeditCursorRectangle(window, x, y, w, h);
    }
    else if (events & NK_EDIT_DEACTIVATED)
        sprintf(xBuf, "%i", x);

    nk_layout_row_push(nk, 1.f / 9.f);
    events = nk_edit_string_zero_terminated(nk, flags, yBuf,
                                            sizeof(yBuf),
                                            nk_filter_decimal);
    if (events & NK_EDIT_COMMITED)
    {
        y = atoi(yBuf);
        *isAutoUpdating = GLFW_FALSE;
        glfwSetPreeditCursorRectangle(window, x, y, w, h);
    }
    else if (events & NK_EDIT_DEACTIVATED)
        sprintf(yBuf, "%i", y);

    nk_layout_row_push(nk, 1.f / 9.f);
    events = nk_edit_string_zero_terminated(nk, flags, wBuf,
                                            sizeof(wBuf),
                                            nk_filter_decimal);
    if (events & NK_EDIT_COMMITED)
    {
        w = atoi(wBuf);
        *isAutoUpdating = GLFW_FALSE;
        glfwSetPreeditCursorRectangle(window, x, y, w, h);
    }
    else if (events & NK_EDIT_DEACTIVATED)
        sprintf(wBuf, "%i", w);

    nk_layout_row_push(nk, 1.f / 9.f);
    events = nk_edit_string_zero_terminated(nk, flags, hBuf,
                                            sizeof(hBuf),
                                            nk_filter_decimal);
    if (events & NK_EDIT_COMMITED)
    {
        h = atoi(hBuf);
        *isAutoUpdating = GLFW_FALSE;
        glfwSetPreeditCursorRectangle(window, x, y, w, h);
    }
    else if (events & NK_EDIT_DEACTIVATED)
        sprintf(hBuf, "%i", h);

    nk_layout_row_push(nk, 1.f / 9.f);
    nk_checkbox_label(nk, "Auto", isAutoUpdating);

    nk_layout_row_end(nk);

    lastX = x;
    lastY = y;
    lastW = w;
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
static void update_cursor_pos(GLFWwindow* window, struct nk_context* nk, struct nk_user_font* f, char* boxBuffer, int boxLen)
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

        int lineHeight = f->height + nk->style.edit.row_padding;

        int cursorPosX = widgetLayoutX + lineWidth;
        int cursorPosY = widgetLayoutY + lineHeight * (totalLines - 1);
        int cursorHeight = lineHeight;
        int cursorWidth;

        // Keep the value of width since it doesn't need to be updated.
        glfwGetPreeditCursorRectangle(window, NULL, NULL, &cursorWidth, NULL);

        glfwSetPreeditCursorRectangle(window, cursorPosX, cursorPosY, cursorWidth, cursorHeight);
    }
}

static void ime_callback(GLFWwindow* window)
{
    currentIMEStatus = glfwGetInputMode(window, GLFW_IME);
    printf("IME switched: %s\n", currentIMEStatus ? "ON" : "OFF");
}

static void preedit_callback(GLFWwindow* window, int preeditCount,
                             unsigned int* preeditString, int blockCount,
                             int* blockSizes, int focusedBlock, int caret)
{
    int blockIndex = -1, remainingBlockSize = 0;
    if (preeditCount == 0 || blockCount == 0)
    {
        strcpy(preeditBuf, "(empty)");
        return;
    }

    strcpy(preeditBuf, "");

    for (int i = 0; i < preeditCount; i++)
    {
        char encoded[5] = "";
        size_t encodedCount = 0;

        if (i == caret)
        {
            if (strlen(preeditBuf) + strlen("|") < MAX_PREEDIT_LEN)
                strcat(preeditBuf, "|");
        }
        if (remainingBlockSize == 0)
        {
            if (blockIndex == focusedBlock)
            {
                if (strlen(preeditBuf) + strlen("]") < MAX_PREEDIT_LEN)
                    strcat(preeditBuf, "]");
            }
            blockIndex++;
            remainingBlockSize = blockSizes[blockIndex];
            if (blockIndex == focusedBlock)
            {
                if (strlen(preeditBuf) + strlen("[") < MAX_PREEDIT_LEN)
                    strcat(preeditBuf, "[");
            }
        }
        encodedCount = encode_utf8(encoded, preeditString[i]);
        encoded[encodedCount] = '\0';
        if (strlen(preeditBuf) + strlen(encoded) < MAX_PREEDIT_LEN)
            strcat(preeditBuf, encoded);
        remainingBlockSize--;
    }
    if (blockIndex == focusedBlock)
    {
        if (strlen(preeditBuf) + strlen("]") < MAX_PREEDIT_LEN)
            strcat(preeditBuf, "]");
    }
    if (caret == preeditCount)
    {
        if (strlen(preeditBuf) + strlen("|") < MAX_PREEDIT_LEN)
            strcat(preeditBuf, "|");
    }
}

int main(int argc, char** argv)
{
    GLFWwindow* window;
    struct nk_context* nk;
    int width, height;
    char boxBuffer[MAX_BUFFER_LEN] = "Input text here.";
    int boxLen = strlen(boxBuffer);
    int isAutoUpdatingCursorPosEnabled = GLFW_TRUE;
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

    currentIMEStatus = glfwGetInputMode(window, GLFW_IME);
    glfwSetPreeditCursorRectangle(window, 0, 0, 1, 1);
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
            set_preedit_cursor_edit(window, nk, 30, &isAutoUpdatingCursorPosEnabled);
            set_ime_stauts_labels(window, nk, 30);
            set_preedit_labels(window, nk, 30);

            nk_layout_row_dynamic(nk, height - 250, 1);
            nk_edit_string(nk, NK_EDIT_BOX, boxBuffer, &boxLen, MAX_BUFFER_LEN, nk_filter_default);
        }
        nk_end(nk);

        glClear(GL_COLOR_BUFFER_BIT);
        nk_glfw3_render(NK_ANTI_ALIASING_ON);
        glfwSwapBuffers(window);

        if (isAutoUpdatingCursorPosEnabled)
            update_cursor_pos(window, nk, &currentFont->handle, boxBuffer, boxLen);

        glfwWaitEvents();
    }

    deinit_font_list();

    nk_glfw3_shutdown();
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
