///////////////////////////////////////////////////////////////////////////////
// glInfo.cpp
// ==========
// get GL vendor, version, supported extensions and other states using glGet*
// functions and store them glInfo struct variable
//
// To get valid OpenGL infos, OpenGL rendering context (RC) must be opened
// before calling glInfo::getInfo(). Otherwise it returns false.
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2005-10-04
// UPDATED: 2013-03-06
//
// Copyright (c) 2005-2013 Song Ho Ahn
///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include "glInfo.h"



// WGL specific extensions for v3.0+ //////////////////////////////////////////
#ifdef _WIN32
#include <windows.h>
#ifndef WGLGETEXTENSIONSSTRINGARB_DEF
#define WGLGETEXTENSIONSSTRINGARB_DEF
typedef const char* (WINAPI * PFNWGLGETEXTENSIONSSTRINGARBPROC)(HDC hdc);
PFNWGLGETEXTENSIONSSTRINGARBPROC    pwglGetExtensionsStringARB = 0;
#define wglGetExtensionsStringARB  pwglGetExtensionsStringARB
#endif
#endif

// version 2.0 or greater
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C



///////////////////////////////////////////////////////////////////////////////
// extract openGL info
// This function must be called after GL rendering context opened.
///////////////////////////////////////////////////////////////////////////////
void glInfo::getInfo(unsigned int param)
{
    std::string str;

    // get vendor string
    str = (const char*)glGetString(GL_VENDOR);
    this->vendor = str;             // check NULL return value

    // get renderer string
    str = (const char*)glGetString(GL_RENDERER);
    this->renderer = str;           // check NULL return value

    // get version string
    str = (const char*)glGetString(GL_VERSION);
    this->version = str;            // check NULL return value

    // get all extensions as a string
    str = (const char*)glGetString(GL_EXTENSIONS);

    // split extensions
    if(str.size() > 0)
    {
        char* str2 = new char[str.size() + 1];
        strcpy(str2, str.c_str());
        char* tok = strtok(str2, " ");
        while(tok)
        {
            this->extensions.push_back(tok);    // put a extension into struct
            tok = strtok(0, " ");               // next token
        }
        delete [] str2;
    }

    // get GLSL version string (v2.0+)
    str = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    if(str.size() > 0)
        this->glslVersion = str;
    else
    {
        // "GL_SHADING_LANGUAGE_VERSION" token is added later (v2.0) after the
        // first GLSL included in OpenGL (v1.5). If "GL_SHADING_LANGUAGE_VERSION"
        // is invalid token but "GL_ARB_shading_language_100" is supported, then
        // the GLSL version should be 1.0.rev.51
        if(isExtensionSupported("GL_ARB_shading_language_100"))
            glslVersion = "1.0.51"; // the first GLSL version
        else
            glslVersion = "";
    }

    // get WGL specific extensions for v3.0+
#ifdef _WIN32 //===========================================
    wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
    if(wglGetExtensionsStringARB && param)
    {
        str = (const char*)wglGetExtensionsStringARB((HDC)param);
        if(str.size() > 0)
        {
            char* str2 = new char[str.size() + 1];
            strcpy(str2, str.c_str());
            char* tok = strtok(str2, " ");
            while(tok)
            {
                this->extensions.push_back(tok);    // put a extension into struct
                tok = strtok(0, " ");               // next token
            }
            delete [] str2;
        }
    }
#endif //==================================================

    // sort extension by alphabetical order
    std::sort(this->extensions.begin(), this->extensions.end());

    // get number of color bits
    glGetIntegerv(GL_RED_BITS, &this->redBits);
    glGetIntegerv(GL_GREEN_BITS, &this->greenBits);
    glGetIntegerv(GL_BLUE_BITS, &this->blueBits);
    glGetIntegerv(GL_ALPHA_BITS, &this->alphaBits);

    // get depth bits
    glGetIntegerv(GL_DEPTH_BITS, &this->depthBits);

    // get stecil bits
    glGetIntegerv(GL_STENCIL_BITS, &this->stencilBits);

    // get max number of lights allowed
    glGetIntegerv(GL_MAX_LIGHTS, &this->maxLights);

    // get max texture resolution
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &this->maxTextureSize);

    // get max number of clipping planes
    glGetIntegerv(GL_MAX_CLIP_PLANES, &this->maxClipPlanes);

    // get max modelview and projection matrix stacks
    glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &this->maxModelViewStacks);
    glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH, &this->maxProjectionStacks);
    glGetIntegerv(GL_MAX_ATTRIB_STACK_DEPTH, &this->maxAttribStacks);

    // get max texture stacks
    glGetIntegerv(GL_MAX_TEXTURE_STACK_DEPTH, &this->maxTextureStacks);
}



///////////////////////////////////////////////////////////////////////////////
// check if the video card support a certain extension
///////////////////////////////////////////////////////////////////////////////
bool glInfo::isExtensionSupported(const std::string& ext)
{
    // search corresponding extension
    std::vector<std::string>::const_iterator iter = this->extensions.begin();
    std::vector<std::string>::const_iterator endIter = this->extensions.end();

    while(iter != endIter)
    {
        if(ext == *iter)
            return true;
        else
            ++iter;
    }
    return false;
}



///////////////////////////////////////////////////////////////////////////////
// print OpenGL info to screen and save to a file
///////////////////////////////////////////////////////////////////////////////
void glInfo::printSelf()
{
    std::stringstream ss;

    ss << std::endl; // blank line
    ss << "OpenGL Driver Info" << std::endl;
    ss << "==================" << std::endl;
    ss << "Vendor: " << this->vendor << std::endl;
    ss << "Version: " << this->version << std::endl;
    ss << "GLSL Version: " << this->glslVersion << std::endl;
    ss << "Renderer: " << this->renderer << std::endl;

    ss << std::endl;
    ss << "Color Bits(R,G,B,A): (" << this->redBits << ", " << this->greenBits
       << ", " << this->blueBits << ", " << this->alphaBits << ")\n";
    ss << "Depth Bits: " << this->depthBits << std::endl;
    ss << "Stencil Bits: " << this->stencilBits << std::endl;

    ss << std::endl;
    ss << "Max Texture Size: " << this->maxTextureSize << "x" << this->maxTextureSize << std::endl;
    ss << "Max Lights: " << this->maxLights << std::endl;
    ss << "Max Clip Planes: " << this->maxClipPlanes << std::endl;
    ss << "Max Modelview Matrix Stacks: " << this->maxModelViewStacks << std::endl;
    ss << "Max Projection Matrix Stacks: " << this->maxProjectionStacks << std::endl;
    ss << "Max Attribute Stacks: " << this->maxAttribStacks << std::endl;
    ss << "Max Texture Stacks: " << this->maxTextureStacks << std::endl;

    ss << std::endl;
    ss << "Total Number of Extensions: " << this->extensions.size() << std::endl;
    ss << "==============================" << std::endl;
    for(unsigned int i = 0; i < this->extensions.size(); ++i)
        ss << this->extensions.at(i) << std::endl;

    ss << "======================================================================" << std::endl;

    std::cout << ss.str() << std::endl;
}
