//========================================================================
// GLFW 3.4 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2023 Andreas O. Jansen
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

GLFWAPI int glfwThemeGetAttribute(const GLFWtheme* theme, int attribute)
{
    assert(theme != NULL);
    assert((attribute & ~(GLFW_THEME_ATTRIBUTE_HAS_COLOR |
                          GLFW_THEME_ATTRIBUTE_HIGH_CONTRAST |
                          GLFW_THEME_ATTRIBUTE_VIBRANT)) == 0);
    
    return ((_GLFWtheme*) theme)->flags & attribute ? GLFW_TRUE : GLFW_FALSE;
}

GLFWAPI void glfwThemeSetAttribute(GLFWtheme* theme, int attribute, int value)
{
    assert(theme != NULL);
    assert(value == GLFW_TRUE || value == GLFW_FALSE);
    assert((attribute & ~(GLFW_THEME_ATTRIBUTE_HAS_COLOR |
                          GLFW_THEME_ATTRIBUTE_HIGH_CONTRAST |
                          GLFW_THEME_ATTRIBUTE_VIBRANT)) == 0);
    
    _GLFWtheme* _theme = (_GLFWtheme*) theme;
    
    if (value == GLFW_TRUE)
        _theme->flags |= attribute;
    else
        _theme->flags &= ~attribute;
}

GLFWAPI void glfwThemeGetColor(const GLFWtheme* theme, float* red, float* green, float* blue, float* alpha)
{
    assert(theme != NULL);
    assert(red != NULL && green != NULL && blue != NULL && alpha != NULL);
    
    const float* color = ((_GLFWtheme*) theme)->color;
    
    *red   = color[0];
    *green = color[1];
    *blue  = color[2];
    *alpha = color[3];
}

GLFWAPI void glfwThemeSetColor(GLFWtheme* theme, float red, float green, float blue, float alpha)
{
    assert(theme != NULL);
    
    float* color = ((_GLFWtheme*) theme)->color;
    
    color[0] = red;
    color[1] = green;
    color[2] = blue;
    color[3] = alpha;
}
