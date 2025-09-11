
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_BUTTON_TRIGGER_ON_RELEASE
#include <nuklear.h>

#define NK_GLFW_GL2_IMPLEMENTATION
#include <nuklear_glfw_gl2.h>



static unsigned char LastCharPressed = 0;


void nk_process(GLFWwindow*, struct nk_context*, float, float);


static void GLFW_DebugCallback(int err_code, const char* description)
{
    printf("GLFW error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    { if (action == GLFW_PRESS) LastCharPressed = key; }



int main(int argc, char** argv)
{
    glfwSetErrorCallback(GLFW_DebugCallback);

    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_WAYLAND_ZWLR_KEYBOARD_ON_FOCUS,     GLFW_TRUE); // no need to call glfwWaylandSetKeyboardFocus manually
    glfwWindowHint(GLFW_WAYLAND_USE_ZWLR, GLFW_WAYLAND_ZWLR_LAYER_TOP);

    GLFWwindow* window = glfwCreateWindow(600, 400, "Don't Care", NULL, NULL);
    if (!window) return 1;

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback    (window, key_callback);
    gladLoadGL            (glfwGetProcAddress);


    struct nk_context* nk = nk_glfw3_init(window, NK_GLFW3_INSTALL_CALLBACKS);

    struct nk_font_atlas* atlas;
    nk_glfw3_font_stash_begin(&atlas);
    nk_glfw3_font_stash_end();


    int width, height;
    while (!glfwWindowShouldClose(window))
    {
        glfwGetWindowSize(window, &width, &height);

        struct nk_rect area = nk_rect(0.f, 0.f, (float) width, (float) height);
        nk_window_set_bounds(nk, "main", area);

        nk_glfw3_new_frame();
        if (nk_begin(nk, "main", area, 0))
            nk_process(window, nk, width, height);

        nk_end(nk);

        glClear(GL_COLOR_BUFFER_BIT);
        nk_glfw3_render(NK_ANTI_ALIASING_ON);

        glfwSwapBuffers(window);
        glfwWaitEvents();
    }

    nk_glfw3_shutdown();
    glfwTerminate();

    return 0;
}


void nk_process(GLFWwindow* window, struct nk_context* ctx, float width, float height)
{
    nk_flags events;
    const nk_flags flags = NK_EDIT_FIELD |
                           NK_EDIT_SIG_ENTER |
                           NK_EDIT_GOTO_END_ON_ACTIVATE;

    nk_layout_row_dynamic(ctx, 30, 4);
    nk_spacing(ctx, 3);

    if (nk_button_label(ctx, "Exit"))
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    nk_layout_row_dynamic(ctx, (height / 4), 1);
    nk_layout_row_dynamic(ctx, 30, 1);
    {
        static char inputBuff[64];
        sprintf(inputBuff, "Window focus: %b | Char received: %i",
                glfwGetWindowAttrib(window, GLFW_FOCUSED), LastCharPressed);

        nk_label(ctx, inputBuff, NK_TEXT_CENTERED);
    }

    nk_layout_row_dynamic(ctx, 30, 2);
    {
        static char exclusiveBuff[12];

        nk_label(ctx, "Exclusive zone (anchor required):", NK_TEXT_LEFT);
        events = nk_edit_string_zero_terminated(ctx, flags, exclusiveBuff,
                                                sizeof(exclusiveBuff),
                                                nk_filter_decimal);
        if (events & NK_EDIT_COMMITED)
        {
            int zone = atoi(exclusiveBuff);
            glfwWaylandZwlrSetExclusiveZone(window, zone);
        }
    }
    {
        static char exclusiveBuff[12];

        nk_label(ctx, "Margin all edges (anchor required):", NK_TEXT_LEFT);
        events = nk_edit_string_zero_terminated(ctx, flags, exclusiveBuff,
                                                sizeof(exclusiveBuff),
                                                nk_filter_decimal);
        if (events & NK_EDIT_COMMITED)
        {
            int margin = atoi(exclusiveBuff);
            glfwWaylandZwlrSetMargin(window, margin, margin, margin, margin);
        }
    }

    nk_layout_row_dynamic(ctx, 30, 4);
    nk_spacing(ctx, 1);

    {
        static int         pickedLayer  = 0;
        static const char* layerItems[] = {
            "Layer Top",    "Layer Overlay",
            "Layer Bottom", "Layer Background"
        };
        static const int layerFlags[]   = {
            GLFW_WAYLAND_ZWLR_LAYER_TOP,    GLFW_WAYLAND_ZWLR_LAYER_OVERLAY,
            GLFW_WAYLAND_ZWLR_LAYER_BOTTOM, GLFW_WAYLAND_ZWLR_LAYER_BACKGROUND
        };
        if (nk_combo_begin_label(ctx, layerItems[pickedLayer], nk_vec2(200, 200)))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            for (int i = 0; i < 4; i++)
            {
                if (nk_combo_item_label(ctx, layerItems[i], NK_TEXT_LEFT))
                {
                    pickedLayer = i;
                    glfwWaylandZwlrSetLayer(window, layerFlags[i]);
                }
            }
            nk_combo_end(ctx);
        }
    }
    {
        static int         pickedAnchor  = 0;
        static const char* anchorItems[] = {
            "Anchor Top" , "Anchor Bottom",
            "Anchor Left", "Anchor Right"
        };
        static const int anchorFlags[]   = {
            GLFW_WAYLAND_ZWLR_ANCHOR_TOP,  GLFW_WAYLAND_ZWLR_ANCHOR_BOTTOM,
            GLFW_WAYLAND_ZWLR_ANCHOR_LEFT, GLFW_WAYLAND_ZWLR_ANCHOR_RIGHT
        };
        if (nk_combo_begin_label(ctx, anchorItems[pickedAnchor], nk_vec2(200, 200)))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            for (int i = 0; i < 4; i++)
            {
                if (nk_combo_item_label(ctx, anchorItems[i], NK_TEXT_LEFT))
                {
                    pickedAnchor = i;
                    glfwWaylandZwlrSetAnchor(window, anchorFlags[i]); // Flags can be combined, but..
                }                                                     // ..it's pain in ass to implement here
            }
            nk_combo_end(ctx);
        }
    }

    nk_layout_row_dynamic(ctx, 30, 2);
    {
        if (nk_button_label(ctx, "Request keybord focus"))
            glfwWaylandZwlrSetKeyboardFocus(window, GLFW_TRUE);

        if (nk_button_label(ctx, "Release keybord focus"))
            glfwWaylandZwlrSetKeyboardFocus(window, GLFW_FALSE);
    }

}



