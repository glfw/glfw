//
//  theme.c
//  glfw
//
//  Created by Andreas Ormevik Jansen on 28/01/2023.
//

#include "internal.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>


GLFWAPI GLFWtheme* glfwCreateTheme(void)
{
    _GLFWtheme* theme = _glfw_calloc(1, sizeof(_GLFWtheme));
    
    theme->variation = GLFW_THEME_DEFAULT;
    theme->flags = 0;
    
    return (GLFWtheme*) theme;
}

GLFWAPI void glfwDestroyTheme(GLFWtheme* theme)
{
    _glfw_free((_GLFWtheme*) theme);
}

GLFWAPI void glfwCopyTheme(const GLFWtheme* source, GLFWtheme* target)
{
    memcpy(target, source, sizeof(_GLFWtheme));
}

GLFWAPI int glfwThemeGetVariation(const GLFWtheme* theme)
{
    assert(theme != NULL);
    return ((_GLFWtheme*) theme)->variation;
}

GLFWAPI void glfwThemeSetVariation(GLFWtheme* theme, int value)
{
    assert(theme != NULL);
    assert(value == GLFW_THEME_DARK || value == GLFW_THEME_DEFAULT || value == GLFW_THEME_LIGHT);
    
    ((_GLFWtheme*) theme)->variation = value;
}

GLFWAPI int glfwThemeGetFlags(const GLFWtheme* theme)
{
    assert(theme != NULL);
    return ((_GLFWtheme*) theme)->flags;
}

GLFWAPI void glfwThemeSetFlags(GLFWtheme* theme, int value)
{
    assert(theme != NULL);
    assert((value & ~(GLFW_THEME_FLAG_HAS_COLOR | GLFW_THEME_FLAG_HIGH_CONTRAST | GLFW_THEME_FLAG_VIBRANT)) == 0);
    
    ((_GLFWtheme*) theme)->flags = value;
}

GLFWAPI void glfwThemeGetColor(const GLFWtheme* theme, float* red, float* green, float* blue, float* alpha)
{
    assert(theme != NULL);
    assert(red != NULL && green != NULL && blue != NULL && alpha != NULL);
    
    const _GLFWtheme* iTheme = ((_GLFWtheme*) theme);
    
    *red   = iTheme->color[0] / 255.0f;
    *green = iTheme->color[1] / 255.0f;
    *blue  = iTheme->color[2] / 255.0f;
    *alpha = iTheme->color[3] / 255.0f;
}

GLFWAPI void glfwThemeSetColor(GLFWtheme* theme, float red, float green, float blue, float alpha)
{
    assert(theme != NULL);
    
    _GLFWtheme* iTheme = ((_GLFWtheme*) theme);
    
    iTheme->color[0] = red * 255.0f;
    iTheme->color[1] = green * 255.0f;
    iTheme->color[2] = blue * 255.0f;
    iTheme->color[3] = alpha * 255.0f;
}
