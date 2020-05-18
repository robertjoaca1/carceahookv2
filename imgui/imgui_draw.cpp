// dear imgui, v1.60
// (drawing and font code)

// Contains implementation for
// - Default styles
// - ImDrawList
// - ImDrawData
// - ImFontAtlas
// - ImFont
// - Default font data

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <stdio.h>      // vsnprintf, sscanf, printf
#if !defined(alloca)
#ifdef _WIN32
#include <malloc.h>     // alloca
#if !defined(alloca)
#define alloca _alloca  // for clang with MS Codegen
#endif
#elif defined(__GLIBC__) || defined(__sun)
#include <alloca.h>     // alloca
#else
#include <stdlib.h>     // alloca
#endif
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4505) // unreferenced local function has been removed (stb stuff)
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

#ifdef __clang__
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning : use of old-style cast                              // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"            // warning : comparing floating point with == or != is unsafe   // storing and comparing against same constants ok.
#pragma clang diagnostic ignored "-Wglobal-constructors"    // warning : declaration requires a global destructor           // similar to above, not sure what the exact difference it.
#pragma clang diagnostic ignored "-Wsign-conversion"        // warning : implicit conversion changes signedness             //
#if __has_warning("-Wcomma")
#pragma clang diagnostic ignored "-Wcomma"                  // warning : possible misuse of comma operator here             //
#endif
#if __has_warning("-Wreserved-id-macro")
#pragma clang diagnostic ignored "-Wreserved-id-macro"      // warning : macro name is a reserved identifier                //
#endif
#if __has_warning("-Wdouble-promotion")
#pragma clang diagnostic ignored "-Wdouble-promotion"       // warning: implicit conversion from 'float' to 'double' when passing argument to function
#endif
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-function"          // warning: 'xxxx' defined but not used
#pragma GCC diagnostic ignored "-Wdouble-promotion"         // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"               // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#endif

//-------------------------------------------------------------------------
// STB libraries implementation
//-------------------------------------------------------------------------

// Compile time options:
//#define IMGUI_STB_NAMESPACE           ImGuiStb
//#define IMGUI_STB_TRUETYPE_FILENAME   "my_folder/stb_truetype.h"
//#define IMGUI_STB_RECT_PACK_FILENAME  "my_folder/stb_rect_pack.h"
//#define IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
//#define IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION

#ifdef IMGUI_STB_NAMESPACE
namespace IMGUI_STB_NAMESPACE
{
#endif

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4456)                             // declaration of 'xx' hides previous local declaration
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#pragma clang diagnostic ignored "-Wcast-qual"              // warning : cast from 'const xxxx *' to 'xxx *' drops const qualifier //
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"              // warning: comparison is always true due to limited range of data type [-Wtype-limits]
#pragma GCC diagnostic ignored "-Wcast-qual"                // warning: cast from type 'const xxxx *' to type 'xxxx *' casts away qualifiers
#endif

#ifndef IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION
#define STBRP_STATIC
#define STBRP_ASSERT(x)    IM_ASSERT(x)
#define STB_RECT_PACK_IMPLEMENTATION
#endif
#ifdef IMGUI_STB_RECT_PACK_FILENAME
#include IMGUI_STB_RECT_PACK_FILENAME
#else
#include "stb_rect_pack.h"
#endif

#ifndef IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
#define STBTT_malloc(x,u)  ((void)(u), ImGui::MemAlloc(x))
#define STBTT_free(x,u)    ((void)(u), ImGui::MemFree(x))
#define STBTT_assert(x)    IM_ASSERT(x)
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#else
#define STBTT_DEF extern
#endif
#ifdef IMGUI_STB_TRUETYPE_FILENAME
#include IMGUI_STB_TRUETYPE_FILENAME
#else
#include "stb_truetype.h"
#endif

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning (pop)
#endif

#ifdef IMGUI_STB_NAMESPACE
} // namespace ImGuiStb
using namespace IMGUI_STB_NAMESPACE;
#endif

//-----------------------------------------------------------------------------
// Style functions
//-----------------------------------------------------------------------------

void ImGui::StyleColorsDark(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_ChildBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border]                 = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_ModalWindowDarkening]   = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
}

void ImGui::StyleColorsClassic(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.70f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.11f, 0.11f, 0.14f, 0.92f);
    colors[ImGuiCol_Border]                 = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.43f, 0.43f, 0.43f, 0.39f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.47f, 0.47f, 0.69f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.42f, 0.41f, 0.64f, 0.69f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.27f, 0.27f, 0.54f, 0.83f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.32f, 0.32f, 0.63f, 0.87f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.40f, 0.40f, 0.55f, 0.80f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.40f, 0.40f, 0.80f, 0.30f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.40f, 0.40f, 0.80f, 0.40f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.41f, 0.39f, 0.80f, 0.60f);
    colors[ImGuiCol_Button]                 = ImVec4(0.35f, 0.40f, 0.61f, 0.62f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.40f, 0.48f, 0.71f, 0.79f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.46f, 0.54f, 0.80f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.40f, 0.40f, 0.90f, 0.45f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.45f, 0.45f, 0.90f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.53f, 0.53f, 0.87f, 0.80f);
    colors[ImGuiCol_Separator]              = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.60f, 0.60f, 0.70f, 1.00f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.70f, 0.70f, 0.90f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(1.00f, 1.00f, 1.00f, 0.16f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.78f, 0.82f, 1.00f, 0.60f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.78f, 0.82f, 1.00f, 0.90f);
    colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
    colors[ImGuiCol_ModalWindowDarkening]   = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
}

// Those light colors are better suited with a thicker font than the default one + FrameBorder
void ImGui::StyleColorsLight(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    //colors[ImGuiCol_TextHovered]          = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    //colors[ImGuiCol_TextActive]           = ImVec4(1.00f, 1.00f, 0.00f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
    colors[ImGuiCol_Border]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.69f, 0.69f, 0.69f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.46f, 0.54f, 0.80f, 0.60f);
    colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.80f, 0.80f, 0.80f, 0.56f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_ModalWindowDarkening]   = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_NavHighlight]           = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
}

//-----------------------------------------------------------------------------
// ImDrawListData
//-----------------------------------------------------------------------------

ImDrawListSharedData::ImDrawListSharedData()
{
    Font = NULL;
    FontSize = 0.0f;
    CurveTessellationTol = 0.0f;
    ClipRectFullscreen = ImVec4(-8192.0f, -8192.0f, +8192.0f, +8192.0f);
    
    // Const data
    for (int i = 0; i < IM_ARRAYSIZE(CircleVtx12); i++)
    {
        const float a = ((float)i * 2 * IM_PI) / (float)IM_ARRAYSIZE(CircleVtx12);
        CircleVtx12[i] = ImVec2(cosf(a), sinf(a));
    }
}

//-----------------------------------------------------------------------------
// ImDrawList
//-----------------------------------------------------------------------------

void ImDrawList::Clear()
{
    CmdBuffer.resize(0);
    IdxBuffer.resize(0);
    VtxBuffer.resize(0);
    Flags = ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedFill;
    _VtxCurrentIdx = 0;
    _VtxWritePtr = NULL;
    _IdxWritePtr = NULL;
    _ClipRectStack.resize(0);
    _TextureIdStack.resize(0);
    _Path.resize(0);
    _ChannelsCurrent = 0;
    _ChannelsCount = 1;
    // NB: Do not clear channels so our allocations are re-used after the first frame.
}

void ImDrawList::ClearFreeMemory()
{
    CmdBuffer.clear();
    IdxBuffer.clear();
    VtxBuffer.clear();
    _VtxCurrentIdx = 0;
    _VtxWritePtr = NULL;
    _IdxWritePtr = NULL;
    _ClipRectStack.clear();
    _TextureIdStack.clear();
    _Path.clear();
    _ChannelsCurrent = 0;
    _ChannelsCount = 1;
    for (int i = 0; i < _Channels.Size; i++)
    {
        if (i == 0) memset(&_Channels[0], 0, sizeof(_Channels[0]));  // channel 0 is a copy of CmdBuffer/IdxBuffer, don't destruct again
        _Channels[i].CmdBuffer.clear();
        _Channels[i].IdxBuffer.clear();
    }
    _Channels.clear();
}

ImDrawList* ImDrawList::CloneOutput() const
{
    ImDrawList* dst = IM_NEW(ImDrawList(NULL));
    dst->CmdBuffer = CmdBuffer;
    dst->IdxBuffer = IdxBuffer;
    dst->VtxBuffer = VtxBuffer;
    dst->Flags = Flags;
    return dst;
}

// Using macros because C++ is a terrible language, we want guaranteed inline, no code in header, and no overhead in Debug builds
#define GetCurrentClipRect()    (_ClipRectStack.Size ? _ClipRectStack.Data[_ClipRectStack.Size-1]  : _Data->ClipRectFullscreen)
#define GetCurrentTextureId()   (_TextureIdStack.Size ? _TextureIdStack.Data[_TextureIdStack.Size-1] : NULL)

void ImDrawList::AddDrawCmd()
{
    ImDrawCmd draw_cmd;
    draw_cmd.ClipRect = GetCurrentClipRect();
    draw_cmd.TextureId = GetCurrentTextureId();

    IM_ASSERT(draw_cmd.ClipRect.x <= draw_cmd.ClipRect.z && draw_cmd.ClipRect.y <= draw_cmd.ClipRect.w);
    CmdBuffer.push_back(draw_cmd);
}

void ImDrawList::AddCallback(ImDrawCallback callback, void* callback_data)
{
    ImDrawCmd* current_cmd = CmdBuffer.Size ? &CmdBuffer.back() : NULL;
    if (!current_cmd || current_cmd->ElemCount != 0 || current_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        current_cmd = &CmdBuffer.back();
    }
    current_cmd->UserCallback = callback;
    current_cmd->UserCallbackData = callback_data;

    AddDrawCmd(); // Force a new command after us (see comment below)
}

// Our scheme may appears a bit unusual, basically we want the most-common calls AddLine AddRect etc. to not have to perform any check so we always have a command ready in the stack.
// The cost of figuring out if a new command has to be added or if we can merge is paid in those Update** functions only.
void ImDrawList::UpdateClipRect()
{
    // If current command is used with different settings we need to add a new command
    const ImVec4 curr_clip_rect = GetCurrentClipRect();
    ImDrawCmd* curr_cmd = CmdBuffer.Size > 0 ? &CmdBuffer.Data[CmdBuffer.Size-1] : NULL;
    if (!curr_cmd || (curr_cmd->ElemCount != 0 && memcmp(&curr_cmd->ClipRect, &curr_clip_rect, sizeof(ImVec4)) != 0) || curr_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        return;
    }

    // Try to merge with previous command if it matches, else use current command
    ImDrawCmd* prev_cmd = CmdBuffer.Size > 1 ? curr_cmd - 1 : NULL;
    if (curr_cmd->ElemCount == 0 && prev_cmd && memcmp(&prev_cmd->ClipRect, &curr_clip_rect, sizeof(ImVec4)) == 0 && prev_cmd->TextureId == GetCurrentTextureId() && prev_cmd->UserCallback == NULL)
        CmdBuffer.pop_back();
    else
        curr_cmd->ClipRect = curr_clip_rect;
}

void ImDrawList::UpdateTextureID()
{
    // If current command is used with different settings we need to add a new command
    const ImTextureID curr_texture_id = GetCurrentTextureId();
    ImDrawCmd* curr_cmd = CmdBuffer.Size ? &CmdBuffer.back() : NULL;
    if (!curr_cmd || (curr_cmd->ElemCount != 0 && curr_cmd->TextureId != curr_texture_id) || curr_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        return;
    }

    // Try to merge with previous command if it matches, else use current command
    ImDrawCmd* prev_cmd = CmdBuffer.Size > 1 ? curr_cmd - 1 : NULL;
    if (curr_cmd->ElemCount == 0 && prev_cmd && prev_cmd->TextureId == curr_texture_id && memcmp(&prev_cmd->ClipRect, &GetCurrentClipRect(), sizeof(ImVec4)) == 0 && prev_cmd->UserCallback == NULL)
        CmdBuffer.pop_back();
    else
        curr_cmd->TextureId = curr_texture_id;
}

#undef GetCurrentClipRect
#undef GetCurrentTextureId

// Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level ImGui::PushClipRect() to affect logic (hit-testing and widget culling)
void ImDrawList::PushClipRect(ImVec2 cr_min, ImVec2 cr_max, bool intersect_with_current_clip_rect)
{
    ImVec4 cr(cr_min.x, cr_min.y, cr_max.x, cr_max.y);
    if (intersect_with_current_clip_rect && _ClipRectStack.Size)
    {
        ImVec4 current = _ClipRectStack.Data[_ClipRectStack.Size-1];
        if (cr.x < current.x) cr.x = current.x;
        if (cr.y < current.y) cr.y = current.y;
        if (cr.z > current.z) cr.z = current.z;
        if (cr.w > current.w) cr.w = current.w;
    }
    cr.z = ImMax(cr.x, cr.z);
    cr.w = ImMax(cr.y, cr.w);

    _ClipRectStack.push_back(cr);
    UpdateClipRect();
}

void ImDrawList::PushClipRectFullScreen()
{
    PushClipRect(ImVec2(_Data->ClipRectFullscreen.x, _Data->ClipRectFullscreen.y), ImVec2(_Data->ClipRectFullscreen.z, _Data->ClipRectFullscreen.w));
}

void ImDrawList::PopClipRect()
{
    IM_ASSERT(_ClipRectStack.Size > 0);
    _ClipRectStack.pop_back();
    UpdateClipRect();
}

void ImDrawList::PushTextureID(ImTextureID texture_id)
{
    _TextureIdStack.push_back(texture_id);
    UpdateTextureID();
}

void ImDrawList::PopTextureID()
{
    IM_ASSERT(_TextureIdStack.Size > 0);
    _TextureIdStack.pop_back();
    UpdateTextureID();
}

void ImDrawList::ChannelsSplit(int channels_count)
{
    IM_ASSERT(_ChannelsCurrent == 0 && _ChannelsCount == 1);
    int old_channels_count = _Channels.Size;
    if (old_channels_count < channels_count)
        _Channels.resize(channels_count);
    _ChannelsCount = channels_count;

    // _Channels[] (24/32 bytes each) hold storage that we'll swap with this->_CmdBuffer/_IdxBuffer
    // The content of _Channels[0] at this point doesn't matter. We clear it to make state tidy in a debugger but we don't strictly need to.
    // When we switch to the next channel, we'll copy _CmdBuffer/_IdxBuffer into _Channels[0] and then _Channels[1] into _CmdBuffer/_IdxBuffer
    memset(&_Channels[0], 0, sizeof(ImDrawChannel));
    for (int i = 1; i < channels_count; i++)
    {
        if (i >= old_channels_count)
        {
            IM_PLACEMENT_NEW(&_Channels[i]) ImDrawChannel();
        }
        else
        {
            _Channels[i].CmdBuffer.resize(0);
            _Channels[i].IdxBuffer.resize(0);
        }
        if (_Channels[i].CmdBuffer.Size == 0)
        {
            ImDrawCmd draw_cmd;
            draw_cmd.ClipRect = _ClipRectStack.back();
            draw_cmd.TextureId = _TextureIdStack.back();
            _Channels[i].CmdBuffer.push_back(draw_cmd);
        }
    }
}

void ImDrawList::ChannelsMerge()
{
    // Note that we never use or rely on channels.Size because it is merely a buffer that we never shrink back to 0 to keep all sub-buffers ready for use.
    if (_ChannelsCount <= 1)
        return;

    ChannelsSetCurrent(0);
    if (CmdBuffer.Size && CmdBuffer.back().ElemCount == 0)
        CmdBuffer.pop_back();

    int new_cmd_buffer_count = 0, new_idx_buffer_count = 0;
    for (int i = 1; i < _ChannelsCount; i++)
    {
        ImDrawChannel& ch = _Channels[i];
        if (ch.CmdBuffer.Size && ch.CmdBuffer.back().ElemCount == 0)
            ch.CmdBuffer.pop_back();
        new_cmd_buffer_count += ch.CmdBuffer.Size;
        new_idx_buffer_count += ch.IdxBuffer.Size;
    }
    CmdBuffer.resize(CmdBuffer.Size + new_cmd_buffer_count);
    IdxBuffer.resize(IdxBuffer.Size + new_idx_buffer_count);

    ImDrawCmd* cmd_write = CmdBuffer.Data + CmdBuffer.Size - new_cmd_buffer_count;
    _IdxWritePtr = IdxBuffer.Data + IdxBuffer.Size - new_idx_buffer_count;
    for (int i = 1; i < _ChannelsCount; i++)
    {
        ImDrawChannel& ch = _Channels[i];
        if (int sz = ch.CmdBuffer.Size) { memcpy(cmd_write, ch.CmdBuffer.Data, sz * sizeof(ImDrawCmd)); cmd_write += sz; }
        if (int sz = ch.IdxBuffer.Size) { memcpy(_IdxWritePtr, ch.IdxBuffer.Data, sz * sizeof(ImDrawIdx)); _IdxWritePtr += sz; }
    }
    UpdateClipRect(); // We call this instead of AddDrawCmd(), so that empty channels won't produce an extra draw call.
    _ChannelsCount = 1;
}

void ImDrawList::ChannelsSetCurrent(int idx)
{
    IM_ASSERT(idx < _ChannelsCount);
    if (_ChannelsCurrent == idx) return;
    memcpy(&_Channels.Data[_ChannelsCurrent].CmdBuffer, &CmdBuffer, sizeof(CmdBuffer)); // copy 12 bytes, four times
    memcpy(&_Channels.Data[_ChannelsCurrent].IdxBuffer, &IdxBuffer, sizeof(IdxBuffer));
    _ChannelsCurrent = idx;
    memcpy(&CmdBuffer, &_Channels.Data[_ChannelsCurrent].CmdBuffer, sizeof(CmdBuffer));
    memcpy(&IdxBuffer, &_Channels.Data[_ChannelsCurrent].IdxBuffer, sizeof(IdxBuffer));
    _IdxWritePtr = IdxBuffer.Data + IdxBuffer.Size;
}

// NB: this can be called with negative count for removing primitives (as long as the result does not underflow)
void ImDrawList::PrimReserve(int idx_count, int vtx_count)
{
    ImDrawCmd& draw_cmd = CmdBuffer.Data[CmdBuffer.Size-1];
    draw_cmd.ElemCount += idx_count;

    int vtx_buffer_old_size = VtxBuffer.Size;
    VtxBuffer.resize(vtx_buffer_old_size + vtx_count);
    _VtxWritePtr = VtxBuffer.Data + vtx_buffer_old_size;

    int idx_buffer_old_size = IdxBuffer.Size;
    IdxBuffer.resize(idx_buffer_old_size + idx_count);
    _IdxWritePtr = IdxBuffer.Data + idx_buffer_old_size;
}

// Fully unrolled with inline call to keep our debug builds decently fast.
void ImDrawList::PrimRect(const ImVec2& a, const ImVec2& c, ImU32 col)
{
    ImVec2 b(c.x, a.y), d(a.x, c.y), uv(_Data->TexUvWhitePixel);
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

void ImDrawList::PrimRectUV(const ImVec2& a, const ImVec2& c, const ImVec2& uv_a, const ImVec2& uv_c, ImU32 col)
{
    ImVec2 b(c.x, a.y), d(a.x, c.y), uv_b(uv_c.x, uv_a.y), uv_d(uv_a.x, uv_c.y);
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

void ImDrawList::PrimQuadUV(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col)
{
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

// TODO: Thickness anti-aliased lines cap are missing their AA fringe.
void ImDrawList::AddPolyline(const ImVec2* points, const int points_count, ImU32 col, bool closed, float thickness)
{
    if (points_count < 2)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;

    int count = points_count;
    if (!closed)
        count = points_count-1;

    const bool thick_line = thickness > 1.0f;
    if (Flags & ImDrawListFlags_AntiAliasedLines)
    {
        // Anti-aliased stroke
        const float AA_SIZE = 1.0f;
        const ImU32 col_trans = col & ~IM_COL32_A_MASK;

        const int idx_count = thick_line ? count*18 : count*12;
        const int vtx_count = thick_line ? points_count*4 : points_count*3;
        PrimReserve(idx_count, vtx_count);

        // Temporary buffer
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * (thick_line ? 5 : 3) * sizeof(ImVec2));
        ImVec2* temp_points = temp_normals + points_count;

        for (int i1 = 0; i1 < count; i1++)
        {
            const int i2 = (i1+1) == points_count ? 0 : i1+1;
            ImVec2 diff = points[i2] - points[i1];
            diff *= ImInvLength(diff, 1.0f);
            temp_normals[i1].x = diff.y;
            temp_normals[i1].y = -diff.x;
        }
        if (!closed)
            temp_normals[points_count-1] = temp_normals[points_count-2];

        if (!thick_line)
        {
            if (!closed)
            {
                temp_points[0] = points[0] + temp_normals[0] * AA_SIZE;
                temp_points[1] = points[0] - temp_normals[0] * AA_SIZE;
                temp_points[(points_count-1)*2+0] = points[points_count-1] + temp_normals[points_count-1] * AA_SIZE;
                temp_points[(points_count-1)*2+1] = points[points_count-1] - temp_normals[points_count-1] * AA_SIZE;
            }

            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = _VtxCurrentIdx;
            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1+1) == points_count ? 0 : i1+1;
                unsigned int idx2 = (i1+1) == points_count ? _VtxCurrentIdx : idx1+3;

                // Average normals
                ImVec2 dm = (temp_normals[i1] + temp_normals[i2]) * 0.5f;
                float dmr2 = dm.x*dm.x + dm.y*dm.y;
                if (dmr2 > 0.000001f)
                {
                    float scale = 1.0f / dmr2;
                    if (scale > 100.0f) scale = 100.0f;
                    dm *= scale;
                }
                dm *= AA_SIZE;
                temp_points[i2*2+0] = points[i2] + dm;
                temp_points[i2*2+1] = points[i2] - dm;

                // Add indexes
                _IdxWritePtr[0] = (ImDrawIdx)(idx2+0); _IdxWritePtr[1] = (ImDrawIdx)(idx1+0); _IdxWritePtr[2] = (ImDrawIdx)(idx1+2);
                _IdxWritePtr[3] = (ImDrawIdx)(idx1+2); _IdxWritePtr[4] = (ImDrawIdx)(idx2+2); _IdxWritePtr[5] = (ImDrawIdx)(idx2+0);
                _IdxWritePtr[6] = (ImDrawIdx)(idx2+1); _IdxWritePtr[7] = (ImDrawIdx)(idx1+1); _IdxWritePtr[8] = (ImDrawIdx)(idx1+0);
                _IdxWritePtr[9] = (ImDrawIdx)(idx1+0); _IdxWritePtr[10]= (ImDrawIdx)(idx2+0); _IdxWritePtr[11]= (ImDrawIdx)(idx2+1);
                _IdxWritePtr += 12;

                idx1 = idx2;
            }

            // Add vertexes
            for (int i = 0; i < points_count; i++)
            {
                _VtxWritePtr[0].pos = points[i];          _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
                _VtxWritePtr[1].pos = temp_points[i*2+0]; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;
                _VtxWritePtr[2].pos = temp_points[i*2+1]; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col_trans;
                _VtxWritePtr += 3;
            }
        }
        else
        {
            const float half_inner_thickness = (thickness - AA_SIZE) * 0.5f;
            if (!closed)
            {
                temp_points[0] = points[0] + temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[1] = points[0] + temp_normals[0] * (half_inner_thickness);
                temp_points[2] = points[0] - temp_normals[0] * (half_inner_thickness);
                temp_points[3] = points[0] - temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[(points_count-1)*4+0] = points[points_count-1] + temp_normals[points_count-1] * (half_inner_thickness + AA_SIZE);
                temp_points[(points_count-1)*4+1] = points[points_count-1] + temp_normals[points_count-1] * (half_inner_thickness);
                temp_points[(points_count-1)*4+2] = points[points_count-1] - temp_normals[points_count-1] * (half_inner_thickness);
                temp_points[(points_count-1)*4+3] = points[points_count-1] - temp_normals[points_count-1] * (half_inner_thickness + AA_SIZE);
            }

            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = _VtxCurrentIdx;
            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1+1) == points_count ? 0 : i1+1;
                unsigned int idx2 = (i1+1) == points_count ? _VtxCurrentIdx : idx1+4;

                // Average normals
                ImVec2 dm = (temp_normals[i1] + temp_normals[i2]) * 0.5f;
                float dmr2 = dm.x*dm.x + dm.y*dm.y;
                if (dmr2 > 0.000001f)
                {
                    float scale = 1.0f / dmr2;
                    if (scale > 100.0f) scale = 100.0f;
                    dm *= scale;
                }
                ImVec2 dm_out = dm * (half_inner_thickness + AA_SIZE);
                ImVec2 dm_in = dm * half_inner_thickness;
                temp_points[i2*4+0] = points[i2] + dm_out;
                temp_points[i2*4+1] = points[i2] + dm_in;
                temp_points[i2*4+2] = points[i2] - dm_in;
                temp_points[i2*4+3] = points[i2] - dm_out;

                // Add indexes
                _IdxWritePtr[0]  = (ImDrawIdx)(idx2+1); _IdxWritePtr[1]  = (ImDrawIdx)(idx1+1); _IdxWritePtr[2]  = (ImDrawIdx)(idx1+2);
                _IdxWritePtr[3]  = (ImDrawIdx)(idx1+2); _IdxWritePtr[4]  = (ImDrawIdx)(idx2+2); _IdxWritePtr[5]  = (ImDrawIdx)(idx2+1);
                _IdxWritePtr[6]  = (ImDrawIdx)(idx2+1); _IdxWritePtr[7]  = (ImDrawIdx)(idx1+1); _IdxWritePtr[8]  = (ImDrawIdx)(idx1+0);
                _IdxWritePtr[9]  = (ImDrawIdx)(idx1+0); _IdxWritePtr[10] = (ImDrawIdx)(idx2+0); _IdxWritePtr[11] = (ImDrawIdx)(idx2+1);
                _IdxWritePtr[12] = (ImDrawIdx)(idx2+2); _IdxWritePtr[13] = (ImDrawIdx)(idx1+2); _IdxWritePtr[14] = (ImDrawIdx)(idx1+3);
                _IdxWritePtr[15] = (ImDrawIdx)(idx1+3); _IdxWritePtr[16] = (ImDrawIdx)(idx2+3); _IdxWritePtr[17] = (ImDrawIdx)(idx2+2);
                _IdxWritePtr += 18;

                idx1 = idx2;
            }

            // Add vertexes
            for (int i = 0; i < points_count; i++)
            {
                _VtxWritePtr[0].pos = temp_points[i*4+0]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col_trans;
                _VtxWritePtr[1].pos = temp_points[i*4+1]; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
                _VtxWritePtr[2].pos = temp_points[i*4+2]; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
                _VtxWritePtr[3].pos = temp_points[i*4+3]; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col_trans;
                _VtxWritePtr += 4;
            }
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Stroke
        const int idx_count = count*6;
        const int vtx_count = count*4;      // FIXME-OPT: Not sharing edges
        PrimReserve(idx_count, vtx_count);

        for (int i1 = 0; i1 < count; i1++)
        {
            const int i2 = (i1+1) == points_count ? 0 : i1+1;
            const ImVec2& p1 = points[i1];
            const ImVec2& p2 = points[i2];
            ImVec2 diff = p2 - p1;
            diff *= ImInvLength(diff, 1.0f);

            const float dx = diff.x * (thickness * 0.5f);
            const float dy = diff.y * (thickness * 0.5f);
            _VtxWritePtr[0].pos.x = p1.x + dy; _VtxWritePtr[0].pos.y = p1.y - dx; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr[1].pos.x = p2.x + dy; _VtxWritePtr[1].pos.y = p2.y - dx; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
            _VtxWritePtr[2].pos.x = p2.x - dy; _VtxWritePtr[2].pos.y = p2.y + dx; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
            _VtxWritePtr[3].pos.x = p1.x - dy; _VtxWritePtr[3].pos.y = p1.y + dx; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
            _VtxWritePtr += 4;

            _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx+1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx+2);
            _IdxWritePtr[3] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[4] = (ImDrawIdx)(_VtxCurrentIdx+2); _IdxWritePtr[5] = (ImDrawIdx)(_VtxCurrentIdx+3);
            _IdxWritePtr += 6;
            _VtxCurrentIdx += 4;
        }
    }
}

void ImDrawList::AddConvexPolyFilled(const ImVec2* points, const int points_count, ImU32 col)
{
    const ImVec2 uv = _Data->TexUvWhitePixel;

    if (Flags & ImDrawListFlags_AntiAliasedFill)
    {
        // Anti-aliased Fill
        const float AA_SIZE = 1.0f;
        const ImU32 col_trans = col & ~IM_COL32_A_MASK;
        const int idx_count = (points_count-2)*3 + points_count*6;
        const int vtx_count = (points_count*2);
        PrimReserve(idx_count, vtx_count);

        // Add indexes for fill
        unsigned int vtx_inner_idx = _VtxCurrentIdx;
        unsigned int vtx_outer_idx = _VtxCurrentIdx+1;
        for (int i = 2; i < points_count; i++)
        {
            _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+((i-1)<<1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_inner_idx+(i<<1));
            _IdxWritePtr += 3;
        }

        // Compute normals
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * sizeof(ImVec2));
        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            const ImVec2& p0 = points[i0];
            const ImVec2& p1 = points[i1];
            ImVec2 diff = p1 - p0;
            diff *= ImInvLength(diff, 1.0f);
            temp_normals[i0].x = diff.y;
            temp_normals[i0].y = -diff.x;
        }

        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            // Average normals
            const ImVec2& n0 = temp_normals[i0];
            const ImVec2& n1 = temp_normals[i1];
            ImVec2 dm = (n0 + n1) * 0.5f;
            float dmr2 = dm.x*dm.x + dm.y*dm.y;
            if (dmr2 > 0.000001f)
            {
                float scale = 1.0f / dmr2;
                if (scale > 100.0f) scale = 100.0f;
                dm *= scale;
            }
            dm *= AA_SIZE * 0.5f;

            // Add vertices
            _VtxWritePtr[0].pos = (points[i1] - dm); _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;        // Inner
            _VtxWritePtr[1].pos = (points[i1] + dm); _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;  // Outer
            _VtxWritePtr += 2;

            // Add indexes for fringes
            _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx+(i1<<1)); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+(i0<<1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_outer_idx+(i0<<1));
            _IdxWritePtr[3] = (ImDrawIdx)(vtx_outer_idx+(i0<<1)); _IdxWritePtr[4] = (ImDrawIdx)(vtx_outer_idx+(i1<<1)); _IdxWritePtr[5] = (ImDrawIdx)(vtx_inner_idx+(i1<<1));
            _IdxWritePtr += 6;
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Fill
        const int idx_count = (points_count-2)*3;
        const int vtx_count = points_count;
        PrimReserve(idx_count, vtx_count);
        for (int i = 0; i < vtx_count; i++)
        {
            _VtxWritePtr[0].pos = points[i]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr++;
        }
        for (int i = 2; i < points_count; i++)
        {
            _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx+i-1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx+i);
            _IdxWritePtr += 3;
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
}

void ImDrawList::PathArcToFast(const ImVec2& centre, float radius, int a_min_of_12, int a_max_of_12)
{
    if (radius == 0.0f || a_min_of_12 > a_max_of_12)
    {
        _Path.push_back(centre);
        return;
    }
    _Path.reserve(_Path.Size + (a_max_of_12 - a_min_of_12 + 1));
    for (int a = a_min_of_12; a <= a_max_of_12; a++)
    {
        const ImVec2& c = _Data->CircleVtx12[a % IM_ARRAYSIZE(_Data->CircleVtx12)];
        _Path.push_back(ImVec2(centre.x + c.x * radius, centre.y + c.y * radius));
    }
}

void ImDrawList::PathArcTo(const ImVec2& centre, float radius, float a_min, float a_max, int num_segments)
{
    if (radius == 0.0f)
    {
        _Path.push_back(centre);
        return;
    }
    _Path.reserve(_Path.Size + (num_segments + 1));
    for (int i = 0; i <= num_segments; i++)
    {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        _Path.push_back(ImVec2(centre.x + cosf(a) * radius, centre.y + sinf(a) * radius));
    }
}

static void PathBezierToCasteljau(ImVector<ImVec2>* path, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float tess_tol, int level)
{
    float dx = x4 - x1;
    float dy = y4 - y1;
    float d2 = ((x2 - x4) * dy - (y2 - y4) * dx);
    float d3 = ((x3 - x4) * dy - (y3 - y4) * dx);
    d2 = (d2 >= 0) ? d2 : -d2;
    d3 = (d3 >= 0) ? d3 : -d3;
    if ((d2+d3) * (d2+d3) < tess_tol * (dx*dx + dy*dy))
    {
        path->push_back(ImVec2(x4, y4));
    }
    else if (level < 10)
    {
        float x12 = (x1+x2)*0.5f,       y12 = (y1+y2)*0.5f;
        float x23 = (x2+x3)*0.5f,       y23 = (y2+y3)*0.5f;
        float x34 = (x3+x4)*0.5f,       y34 = (y3+y4)*0.5f;
        float x123 = (x12+x23)*0.5f,    y123 = (y12+y23)*0.5f;
        float x234 = (x23+x34)*0.5f,    y234 = (y23+y34)*0.5f;
        float x1234 = (x123+x234)*0.5f, y1234 = (y123+y234)*0.5f;

        PathBezierToCasteljau(path, x1,y1,        x12,y12,    x123,y123,  x1234,y1234, tess_tol, level+1);
        PathBezierToCasteljau(path, x1234,y1234,  x234,y234,  x34,y34,    x4,y4,       tess_tol, level+1);
    }
}

void ImDrawList::PathBezierCurveTo(const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, int num_segments)
{
    ImVec2 p1 = _Path.back();
    if (num_segments == 0)
    {
        // Auto-tessellated
        PathBezierToCasteljau(&_Path, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, _Data->CurveTessellationTol, 0);
    }
    else
    {
        float t_step = 1.0f / (float)num_segments;
        for (int i_step = 1; i_step <= num_segments; i_step++)
        {
            float t = t_step * i_step;
            float u = 1.0f - t;
            float w1 = u*u*u;
            float w2 = 3*u*u*t;
            float w3 = 3*u*t*t;
            float w4 = t*t*t;
            _Path.push_back(ImVec2(w1*p1.x + w2*p2.x + w3*p3.x + w4*p4.x, w1*p1.y + w2*p2.y + w3*p3.y + w4*p4.y));
        }
    }
}

void ImDrawList::PathRect(const ImVec2& a, const ImVec2& b, float rounding, int rounding_corners)
{
    rounding = ImMin(rounding, fabsf(b.x - a.x) * ( ((rounding_corners & ImDrawCornerFlags_Top)  == ImDrawCornerFlags_Top)  || ((rounding_corners & ImDrawCornerFlags_Bot)   == ImDrawCornerFlags_Bot)   ? 0.5f : 1.0f ) - 1.0f);
    rounding = ImMin(rounding, fabsf(b.y - a.y) * ( ((rounding_corners & ImDrawCornerFlags_Left) == ImDrawCornerFlags_Left) || ((rounding_corners & ImDrawCornerFlags_Right) == ImDrawCornerFlags_Right) ? 0.5f : 1.0f ) - 1.0f);

    if (rounding <= 0.0f || rounding_corners == 0)
    {
        PathLineTo(a);
        PathLineTo(ImVec2(b.x, a.y));
        PathLineTo(b);
        PathLineTo(ImVec2(a.x, b.y));
    }
    else
    {
        const float rounding_tl = (rounding_corners & ImDrawCornerFlags_TopLeft) ? rounding : 0.0f;
        const float rounding_tr = (rounding_corners & ImDrawCornerFlags_TopRight) ? rounding : 0.0f;
        const float rounding_br = (rounding_corners & ImDrawCornerFlags_BotRight) ? rounding : 0.0f;
        const float rounding_bl = (rounding_corners & ImDrawCornerFlags_BotLeft) ? rounding : 0.0f;
        PathArcToFast(ImVec2(a.x + rounding_tl, a.y + rounding_tl), rounding_tl, 6, 9);
        PathArcToFast(ImVec2(b.x - rounding_tr, a.y + rounding_tr), rounding_tr, 9, 12);
        PathArcToFast(ImVec2(b.x - rounding_br, b.y - rounding_br), rounding_br, 0, 3);
        PathArcToFast(ImVec2(a.x + rounding_bl, b.y - rounding_bl), rounding_bl, 3, 6);
    }
}

void ImDrawList::AddLine(const ImVec2& a, const ImVec2& b, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    PathLineTo(a + ImVec2(0.5f,0.5f));
    PathLineTo(b + ImVec2(0.5f,0.5f));
    PathStroke(col, false, thickness);
}

// a: upper-left, b: lower-right. we don't render 1 px sized rectangles properly.
void ImDrawList::AddRect(const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners_flags, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    if (Flags & ImDrawListFlags_AntiAliasedLines)
        PathRect(a + ImVec2(0.5f,0.5f), b - ImVec2(0.50f,0.50f), rounding, rounding_corners_flags);
    else
        PathRect(a + ImVec2(0.5f,0.5f), b - ImVec2(0.49f,0.49f), rounding, rounding_corners_flags); // Better looking lower-right corner and rounded non-AA shapes.
    PathStroke(col, true, thickness);
}

void ImDrawList::AddRectFilled(const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners_flags)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    if (rounding > 0.0f)
    {
        PathRect(a, b, rounding, rounding_corners_flags);
        PathFillConvex(col);
    }
    else
    {
        PrimReserve(6, 4);
        PrimRect(a, b, col);
    }
}

void ImDrawList::AddRectFilledMultiColor(const ImVec2& a, const ImVec2& c, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left)
{
    if (((col_upr_left | col_upr_right | col_bot_right | col_bot_left) & IM_COL32_A_MASK) == 0)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;
    PrimReserve(6, 4);
    PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+1)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+2));
    PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+2)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+3));
    PrimWriteVtx(a, uv, col_upr_left);
    PrimWriteVtx(ImVec2(c.x, a.y), uv, col_upr_right);
    PrimWriteVtx(c, uv, col_bot_right);
    PrimWriteVtx(ImVec2(a.x, c.y), uv, col_bot_left);
}

void ImDrawList::AddQuad(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathLineTo(d);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddQuadFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathLineTo(d);
    PathFillConvex(col);
}

void ImDrawList::AddTriangle(const ImVec2& a, const ImVec2& b, const ImVec2& c, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddTriangleFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathFillConvex(col);
}

void ImDrawList::AddCircle(const ImVec2& centre, float radius, ImU32 col, int num_segments, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const float a_max = IM_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(centre, radius-0.5f, 0.0f, a_max, num_segments);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddCircleFilled(const ImVec2& centre, float radius, ImU32 col, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const float a_max = IM_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(centre, radius, 0.0f, a_max, num_segments);
    PathFillConvex(col);
}

void ImDrawList::AddBezierCurve(const ImVec2& pos0, const ImVec2& cp0, const ImVec2& cp1, const ImVec2& pos1, ImU32 col, float thickness, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(pos0);
    PathBezierCurveTo(cp0, cp1, pos1, num_segments);
    PathStroke(col, false, thickness);
}

void ImDrawList::AddText(const ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end, float wrap_width, const ImVec4* cpu_fine_clip_rect)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    if (text_end == NULL)
        text_end = text_begin + strlen(text_begin);
    if (text_begin == text_end)
        return;

    // Pull default font/size from the shared ImDrawListSharedData instance
    if (font == NULL)
        font = _Data->Font;
    if (font_size == 0.0f)
        font_size = _Data->FontSize;

    IM_ASSERT(font->ContainerAtlas->TexID == _TextureIdStack.back());  // Use high-level ImGui::PushFont() or low-level ImDrawList::PushTextureId() to change font.

    ImVec4 clip_rect = _ClipRectStack.back();
    if (cpu_fine_clip_rect)
    {
        clip_rect.x = ImMax(clip_rect.x, cpu_fine_clip_rect->x);
        clip_rect.y = ImMax(clip_rect.y, cpu_fine_clip_rect->y);
        clip_rect.z = ImMin(clip_rect.z, cpu_fine_clip_rect->z);
        clip_rect.w = ImMin(clip_rect.w, cpu_fine_clip_rect->w);
    }
    font->RenderText(this, font_size, pos, col, clip_rect, text_begin, text_end, wrap_width, cpu_fine_clip_rect != NULL);
}

void ImDrawList::AddText(const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end)
{
    AddText(NULL, 0.0f, pos, col, text_begin, text_end);
}

void ImDrawList::AddImage(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    PrimReserve(6, 4);
    PrimRectUV(a, b, uv_a, uv_b, col);

    if (push_texture_id)
        PopTextureID();
}

void ImDrawList::AddImageQuad(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    PrimReserve(6, 4);
    PrimQuadUV(a, b, c, d, uv_a, uv_b, uv_c, uv_d, col);

    if (push_texture_id)
        PopTextureID();
}

void ImDrawList::AddImageRounded(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, ImU32 col, float rounding, int rounding_corners)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    if (rounding <= 0.0f || (rounding_corners & ImDrawCornerFlags_All) == 0)
    {
        AddImage(user_texture_id, a, b, uv_a, uv_b, col);
        return;
    }

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    int vert_start_idx = VtxBuffer.Size;
    PathRect(a, b, rounding, rounding_corners);
    PathFillConvex(col);
    int vert_end_idx = VtxBuffer.Size;
    ImGui::ShadeVertsLinearUV(VtxBuffer.Data + vert_start_idx, VtxBuffer.Data + vert_end_idx, a, b, uv_a, uv_b, true);

    if (push_texture_id)
        PopTextureID();
}

//-----------------------------------------------------------------------------
// ImDrawData
//-----------------------------------------------------------------------------

// For backward compatibility: convert all buffers from indexed to de-indexed, in case you cannot render indexed. Note: this is slow and most likely a waste of resources. Always prefer indexed rendering!
void ImDrawData::DeIndexAllBuffers()
{
    ImVector<ImDrawVert> new_vtx_buffer;
    TotalVtxCount = TotalIdxCount = 0;
    for (int i = 0; i < CmdListsCount; i++)
    {
        ImDrawList* cmd_list = CmdLists[i];
        if (cmd_list->IdxBuffer.empty())
            continue;
        new_vtx_buffer.resize(cmd_list->IdxBuffer.Size);
        for (int j = 0; j < cmd_list->IdxBuffer.Size; j++)
            new_vtx_buffer[j] = cmd_list->VtxBuffer[cmd_list->IdxBuffer[j]];
        cmd_list->VtxBuffer.swap(new_vtx_buffer);
        cmd_list->IdxBuffer.resize(0);
        TotalVtxCount += cmd_list->VtxBuffer.Size;
    }
}

// Helper to scale the ClipRect field of each ImDrawCmd. Use if your final output buffer is at a different scale than ImGui expects, or if there is a difference between your window resolution and framebuffer resolution.
void ImDrawData::ScaleClipRects(const ImVec2& scale)
{
    for (int i = 0; i < CmdListsCount; i++)
    {
        ImDrawList* cmd_list = CmdLists[i];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            ImDrawCmd* cmd = &cmd_list->CmdBuffer[cmd_i];
            cmd->ClipRect = ImVec4(cmd->ClipRect.x * scale.x, cmd->ClipRect.y * scale.y, cmd->ClipRect.z * scale.x, cmd->ClipRect.w * scale.y);
        }
    }
}

//-----------------------------------------------------------------------------
// Shade functions
//-----------------------------------------------------------------------------

// Generic linear color gradient, write to RGB fields, leave A untouched.
void ImGui::ShadeVertsLinearColorGradientKeepAlpha(ImDrawVert* vert_start, ImDrawVert* vert_end, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1)
{
    ImVec2 gradient_extent = gradient_p1 - gradient_p0;
    float gradient_inv_length2 = 1.0f / ImLengthSqr(gradient_extent);
    for (ImDrawVert* vert = vert_start; vert < vert_end; vert++)
    {
        float d = ImDot(vert->pos - gradient_p0, gradient_extent);
        float t = ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);
        int r = ImLerp((int)(col0 >> IM_COL32_R_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_R_SHIFT) & 0xFF, t);
        int g = ImLerp((int)(col0 >> IM_COL32_G_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_G_SHIFT) & 0xFF, t);
        int b = ImLerp((int)(col0 >> IM_COL32_B_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_B_SHIFT) & 0xFF, t);
        vert->col = (r << IM_COL32_R_SHIFT) | (g << IM_COL32_G_SHIFT) | (b << IM_COL32_B_SHIFT) | (vert->col & IM_COL32_A_MASK);
    }
}

// Scan and shade backward from the end of given vertices. Assume vertices are text only (= vert_start..vert_end going left to right) so we can break as soon as we are out the gradient bounds.
void ImGui::ShadeVertsLinearAlphaGradientForLeftToRightText(ImDrawVert* vert_start, ImDrawVert* vert_end, float gradient_p0_x, float gradient_p1_x)
{
    float gradient_extent_x = gradient_p1_x - gradient_p0_x;
    float gradient_inv_length2 = 1.0f / (gradient_extent_x * gradient_extent_x);
    int full_alpha_count = 0;
    for (ImDrawVert* vert = vert_end - 1; vert >= vert_start; vert--)
    {
        float d = (vert->pos.x - gradient_p0_x) * (gradient_extent_x);
        float alpha_mul = 1.0f - ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);
        if (alpha_mul >= 1.0f && ++full_alpha_count > 2)
            return; // Early out
        int a = (int)(((vert->col >> IM_COL32_A_SHIFT) & 0xFF) * alpha_mul);
        vert->col = (vert->col & ~IM_COL32_A_MASK) | (a << IM_COL32_A_SHIFT);
    }
}

// Distribute UV over (a, b) rectangle
void ImGui::ShadeVertsLinearUV(ImDrawVert* vert_start, ImDrawVert* vert_end, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, bool clamp)
{
    const ImVec2 size = b - a;
    const ImVec2 uv_size = uv_b - uv_a;
    const ImVec2 scale = ImVec2(
        size.x != 0.0f ? (uv_size.x / size.x) : 0.0f,
        size.y != 0.0f ? (uv_size.y / size.y) : 0.0f);

    if (clamp)
    {
        const ImVec2 min = ImMin(uv_a, uv_b);
        const ImVec2 max = ImMax(uv_a, uv_b);

        for (ImDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
            vertex->uv = ImClamp(uv_a + ImMul(ImVec2(vertex->pos.x, vertex->pos.y) - a, scale), min, max);
    }
    else
    {
        for (ImDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
            vertex->uv = uv_a + ImMul(ImVec2(vertex->pos.x, vertex->pos.y) - a, scale);
    }
}

//-----------------------------------------------------------------------------
// ImFontConfig
//-----------------------------------------------------------------------------

ImFontConfig::ImFontConfig()
{
    FontData = NULL;
    FontDataSize = 0;
    FontDataOwnedByAtlas = true;
    FontNo = 0;
    SizePixels = 0.0f;
    OversampleH = 3;
    OversampleV = 1;
    PixelSnapH = false;
    GlyphExtraSpacing = ImVec2(0.0f, 0.0f);
    GlyphOffset = ImVec2(0.0f, 0.0f);
    GlyphRanges = NULL;
    MergeMode = false;
    RasterizerFlags = 0x00;
    RasterizerMultiply = 1.0f;
    memset(Name, 0, sizeof(Name));
    DstFont = NULL;
}

//-----------------------------------------------------------------------------
// ImFontAtlas
//-----------------------------------------------------------------------------

// A work of art lies ahead! (. = white layer, X = black layer, others are blank)
// The white texels on the top left are the ones we'll use everywhere in ImGui to render filled shapes.
const int FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF = 90;
const int FONT_ATLAS_DEFAULT_TEX_DATA_H      = 27;
const unsigned int FONT_ATLAS_DEFAULT_TEX_DATA_ID = 0x80000000;
static const char FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF * FONT_ATLAS_DEFAULT_TEX_DATA_H + 1] =
{
    "..-         -XXXXXXX-    X    -           X           -XXXXXXX          -          XXXXXXX"
    "..-         -X.....X-   X.X   -          X.X          -X.....X          -          X.....X"
    "---         -XXX.XXX-  X...X  -         X...X         -X....X           -           X....X"
    "X           -  X.X  - X.....X -        X.....X        -X...X            -            X...X"
    "XX          -  X.X  -X.......X-       X.......X       -X..X.X           -           X.X..X"
    "X.X         -  X.X  -XXXX.XXXX-       XXXX.XXXX       -X.X X.X          -          X.X X.X"
    "X..X        -  X.X  -   X.X   -          X.X          -XX   X.X         -         X.X   XX"
    "X...X       -  X.X  -   X.X   -    XX    X.X    XX    -      X.X        -        X.X      "
    "X....X      -  X.X  -   X.X   -   X.X    X.X    X.X   -       X.X       -       X.X       "
    "X.....X     -  X.X  -   X.X   -  X..X    X.X    X..X  -        X.X      -      X.X        "
    "X......X    -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -         X.X   XX-XX   X.X         "
    "X.......X   -  X.X  -   X.X   -X.....................X-          X.X X.X-X.X X.X          "
    "X........X  -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -           X.X..X-X..X.X           "
    "X.........X -XXX.XXX-   X.X   -  X..X    X.X    X..X  -            X...X-X...X            "
    "X..........X-X.....X-   X.X   -   X.X    X.X    X.X   -           X....X-X....X           "
    "X......XXXXX-XXXXXXX-   X.X   -    XX    X.X    XX    -          X.....X-X.....X          "
    "X...X..X    ---------   X.X   -          X.X          -          XXXXXXX-XXXXXXX          "
    "X..X X..X   -       -XXXX.XXXX-       XXXX.XXXX       ------------------------------------"
    "X.X  X..X   -       -X.......X-       X.......X       -    XX           XX    -           "
    "XX    X..X  -       - X.....X -        X.....X        -   X.X           X.X   -           "
    "      X..X          -  X...X  -         X...X         -  X..X           X..X  -           "
    "       XX           -   X.X   -          X.X          - X...XXXXXXXXXXXXX...X -           "
    "------------        -    X    -           X           -X.....................X-           "
    "                    ----------------------------------- X...XXXXXXXXXXXXX...X -           "
    "                                                      -  X..X           X..X  -           "
    "                                                      -   X.X           X.X   -           "
    "                                                      -    XX           XX    -           "
};

static const ImVec2 FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[ImGuiMouseCursor_COUNT][3] =
{
    // Pos ........ Size ......... Offset ......
    { ImVec2(0,3),  ImVec2(12,19), ImVec2( 0, 0) }, // ImGuiMouseCursor_Arrow
    { ImVec2(13,0), ImVec2(7,16),  ImVec2( 4, 8) }, // ImGuiMouseCursor_TextInput
    { ImVec2(31,0), ImVec2(23,23), ImVec2(11,11) }, // ImGuiMouseCursor_ResizeAll
    { ImVec2(21,0), ImVec2( 9,23), ImVec2( 5,11) }, // ImGuiMouseCursor_ResizeNS
    { ImVec2(55,18),ImVec2(23, 9), ImVec2(11, 5) }, // ImGuiMouseCursor_ResizeEW
    { ImVec2(73,0), ImVec2(17,17), ImVec2( 9, 9) }, // ImGuiMouseCursor_ResizeNESW
    { ImVec2(55,0), ImVec2(17,17), ImVec2( 9, 9) }, // ImGuiMouseCursor_ResizeNWSE
};

ImFontAtlas::ImFontAtlas()
{
    Flags = 0x00;
    TexID = NULL;
    TexDesiredWidth = 0;
    TexGlyphPadding = 1;

    TexPixelsAlpha8 = NULL;
    TexPixelsRGBA32 = NULL;
    TexWidth = TexHeight = 0;
    TexUvScale = ImVec2(0.0f, 0.0f);
    TexUvWhitePixel = ImVec2(0.0f, 0.0f);
    for (int n = 0; n < IM_ARRAYSIZE(CustomRectIds); n++)
        CustomRectIds[n] = -1;
}

ImFontAtlas::~ImFontAtlas()
{
    Clear();
}

void    ImFontAtlas::ClearInputData()
{
    for (int i = 0; i < ConfigData.Size; i++)
        if (ConfigData[i].FontData && ConfigData[i].FontDataOwnedByAtlas)
        {
            ImGui::MemFree(ConfigData[i].FontData);
            ConfigData[i].FontData = NULL;
        }

    // When clearing this we lose access to the font name and other information used to build the font.
    for (int i = 0; i < Fonts.Size; i++)
        if (Fonts[i]->ConfigData >= ConfigData.Data && Fonts[i]->ConfigData < ConfigData.Data + ConfigData.Size)
        {
            Fonts[i]->ConfigData = NULL;
            Fonts[i]->ConfigDataCount = 0;
        }
    ConfigData.clear();
    CustomRects.clear();
    for (int n = 0; n < IM_ARRAYSIZE(CustomRectIds); n++)
        CustomRectIds[n] = -1;
}

void    ImFontAtlas::ClearTexData()
{
    if (TexPixelsAlpha8)
        ImGui::MemFree(TexPixelsAlpha8);
    if (TexPixelsRGBA32)
        ImGui::MemFree(TexPixelsRGBA32);
    TexPixelsAlpha8 = NULL;
    TexPixelsRGBA32 = NULL;
}

void    ImFontAtlas::ClearFonts()
{
    for (int i = 0; i < Fonts.Size; i++)
        IM_DELETE(Fonts[i]);
    Fonts.clear();
}

void    ImFontAtlas::Clear()
{
    ClearInputData();
    ClearTexData();
    ClearFonts();
}

void    ImFontAtlas::GetTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
    // Build atlas on demand
    if (TexPixelsAlpha8 == NULL)
    {
        if (ConfigData.empty())
            AddFontDefault();
        Build();
    }

    *out_pixels = TexPixelsAlpha8;
    if (out_width) *out_width = TexWidth;
    if (out_height) *out_height = TexHeight;
    if (out_bytes_per_pixel) *out_bytes_per_pixel = 1;
}

void    ImFontAtlas::GetTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
    // Convert to RGBA32 format on demand
    // Although it is likely to be the most commonly used format, our font rendering is 1 channel / 8 bpp
    if (!TexPixelsRGBA32)
    {
        unsigned char* pixels = NULL;
        GetTexDataAsAlpha8(&pixels, NULL, NULL);
        if (pixels)
        {
            TexPixelsRGBA32 = (unsigned int*)ImGui::MemAlloc((size_t)(TexWidth * TexHeight * 4));
            const unsigned char* src = pixels;
            unsigned int* dst = TexPixelsRGBA32;
            for (int n = TexWidth * TexHeight; n > 0; n--)
                *dst++ = IM_COL32(255, 255, 255, (unsigned int)(*src++));
        }
    }

    *out_pixels = (unsigned char*)TexPixelsRGBA32;
    if (out_width) *out_width = TexWidth;
    if (out_height) *out_height = TexHeight;
    if (out_bytes_per_pixel) *out_bytes_per_pixel = 4;
}

ImFont* ImFontAtlas::AddFont(const ImFontConfig* font_cfg)
{
    IM_ASSERT(font_cfg->FontData != NULL && font_cfg->FontDataSize > 0);
    IM_ASSERT(font_cfg->SizePixels > 0.0f);

    // Create new font
    if (!font_cfg->MergeMode)
        Fonts.push_back(IM_NEW(ImFont));
    else
        IM_ASSERT(!Fonts.empty()); // When using MergeMode make sure that a font has already been added before. You can use ImGui::GetIO().Fonts->AddFontDefault() to add the default imgui font.

    ConfigData.push_back(*font_cfg);
    ImFontConfig& new_font_cfg = ConfigData.back();
    if (!new_font_cfg.DstFont)
        new_font_cfg.DstFont = Fonts.back();
    if (!new_font_cfg.FontDataOwnedByAtlas)
    {
        new_font_cfg.FontData = ImGui::MemAlloc(new_font_cfg.FontDataSize);
        new_font_cfg.FontDataOwnedByAtlas = true;
        memcpy(new_font_cfg.FontData, font_cfg->FontData, (size_t)new_font_cfg.FontDataSize);
    }

    // Invalidate texture
    ClearTexData();
    return new_font_cfg.DstFont;
}

// Default font TTF is compressed with stb_compress then base85 encoded (see misc/fonts/binary_to_compressed_c.cpp for encoder)
static unsigned int stb_decompress_length(const unsigned char *input);
static unsigned int stb_decompress(unsigned char *output, const unsigned char *input, unsigned int length);
static const char*  GetDefaultCompressedFontDataTTFBase85();
static unsigned int Decode85Byte(char c)                                    { return c >= '\\' ? c-36 : c-35; }
static void         Decode85(const unsigned char* src, unsigned char* dst)
{
    while (*src)
    {
        unsigned int tmp = Decode85Byte(src[0]) + 85*(Decode85Byte(src[1]) + 85*(Decode85Byte(src[2]) + 85*(Decode85Byte(src[3]) + 85*Decode85Byte(src[4]))));
        dst[0] = ((tmp >> 0) & 0xFF); dst[1] = ((tmp >> 8) & 0xFF); dst[2] = ((tmp >> 16) & 0xFF); dst[3] = ((tmp >> 24) & 0xFF);   // We can't assume little-endianness.
        src += 5;
        dst += 4;
    }
}

// Load embedded ProggyClean.ttf at size 13, disable oversampling
ImFont* ImFontAtlas::AddFontDefault(const ImFontConfig* font_cfg_template)
{
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    if (!font_cfg_template)
    {
        font_cfg.OversampleH = font_cfg.OversampleV = 1;
        font_cfg.PixelSnapH = true;
    }
    if (font_cfg.Name[0] == '\0') strcpy(font_cfg.Name, "ProggyClean.ttf, 13px");
    if (font_cfg.SizePixels <= 0.0f) font_cfg.SizePixels = 13.0f;

    const char* ttf_compressed_base85 = GetDefaultCompressedFontDataTTFBase85();
    ImFont* font = AddFontFromMemoryCompressedBase85TTF(ttf_compressed_base85, font_cfg.SizePixels, &font_cfg, GetGlyphRangesDefault());
    font->DisplayOffset.y = 1.0f;
    return font;
}

ImFont* ImFontAtlas::AddFontFromFileTTF(const char* filename, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    int data_size = 0;
    void* data = ImFileLoadToMemory(filename, "rb", &data_size, 0);
    if (!data)
    {
        IM_ASSERT(0); // Could not load file.
        return NULL;
    }
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    if (font_cfg.Name[0] == '\0')
    {
        // Store a short copy of filename into into the font name for convenience
        const char* p;
        for (p = filename + strlen(filename); p > filename && p[-1] != '/' && p[-1] != '\\'; p--) {}
        ImFormatString(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "%s, %.0fpx", p, size_pixels);
    }
    return AddFontFromMemoryTTF(data, data_size, size_pixels, &font_cfg, glyph_ranges);
}

// NB: Transfer ownership of 'ttf_data' to ImFontAtlas, unless font_cfg_template->FontDataOwnedByAtlas == false. Owned TTF buffer will be deleted after Build().
ImFont* ImFontAtlas::AddFontFromMemoryTTF(void* ttf_data, int ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    IM_ASSERT(font_cfg.FontData == NULL);
    font_cfg.FontData = ttf_data;
    font_cfg.FontDataSize = ttf_size;
    font_cfg.SizePixels = size_pixels;
    if (glyph_ranges)
        font_cfg.GlyphRanges = glyph_ranges;
    return AddFont(&font_cfg);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedTTF(const void* compressed_ttf_data, int compressed_ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    const unsigned int buf_decompressed_size = stb_decompress_length((const unsigned char*)compressed_ttf_data);
    unsigned char* buf_decompressed_data = (unsigned char *)ImGui::MemAlloc(buf_decompressed_size);
    stb_decompress(buf_decompressed_data, (const unsigned char*)compressed_ttf_data, (unsigned int)compressed_ttf_size);

    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    IM_ASSERT(font_cfg.FontData == NULL);
    font_cfg.FontDataOwnedByAtlas = true;
    return AddFontFromMemoryTTF(buf_decompressed_data, (int)buf_decompressed_size, size_pixels, &font_cfg, glyph_ranges);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedBase85TTF(const char* compressed_ttf_data_base85, float size_pixels, const ImFontConfig* font_cfg, const ImWchar* glyph_ranges)
{
    int compressed_ttf_size = (((int)strlen(compressed_ttf_data_base85) + 4) / 5) * 4;
    void* compressed_ttf = ImGui::MemAlloc((size_t)compressed_ttf_size);
    Decode85((const unsigned char*)compressed_ttf_data_base85, (unsigned char*)compressed_ttf);
    ImFont* font = AddFontFromMemoryCompressedTTF(compressed_ttf, compressed_ttf_size, size_pixels, font_cfg, glyph_ranges);
    ImGui::MemFree(compressed_ttf);
    return font;
}

int ImFontAtlas::AddCustomRectRegular(unsigned int id, int width, int height)
{
    IM_ASSERT(id >= 0x10000);
    IM_ASSERT(width > 0 && width <= 0xFFFF);
    IM_ASSERT(height > 0 && height <= 0xFFFF);
    CustomRect r;
    r.ID = id;
    r.Width = (unsigned short)width;
    r.Height = (unsigned short)height;
    CustomRects.push_back(r);
    return CustomRects.Size - 1; // Return index
}

int ImFontAtlas::AddCustomRectFontGlyph(ImFont* font, ImWchar id, int width, int height, float advance_x, const ImVec2& offset)
{
    IM_ASSERT(font != NULL);
    IM_ASSERT(width > 0 && width <= 0xFFFF);
    IM_ASSERT(height > 0 && height <= 0xFFFF);
    CustomRect r;
    r.ID = id;
    r.Width = (unsigned short)width;
    r.Height = (unsigned short)height;
    r.GlyphAdvanceX = advance_x;
    r.GlyphOffset = offset;
    r.Font = font;
    CustomRects.push_back(r);
    return CustomRects.Size - 1; // Return index
}

void ImFontAtlas::CalcCustomRectUV(const CustomRect* rect, ImVec2* out_uv_min, ImVec2* out_uv_max)
{
    IM_ASSERT(TexWidth > 0 && TexHeight > 0);   // Font atlas needs to be built before we can calculate UV coordinates
    IM_ASSERT(rect->IsPacked());                // Make sure the rectangle has been packed
    *out_uv_min = ImVec2((float)rect->X * TexUvScale.x, (float)rect->Y * TexUvScale.y);
    *out_uv_max = ImVec2((float)(rect->X + rect->Width) * TexUvScale.x, (float)(rect->Y + rect->Height) * TexUvScale.y);
}

bool ImFontAtlas::GetMouseCursorTexData(ImGuiMouseCursor cursor_type, ImVec2* out_offset, ImVec2* out_size, ImVec2 out_uv_border[2], ImVec2 out_uv_fill[2])
{
    if (cursor_type <= ImGuiMouseCursor_None || cursor_type >= ImGuiMouseCursor_COUNT)
        return false;
    if (Flags & ImFontAtlasFlags_NoMouseCursors)
        return false;

    IM_ASSERT(CustomRectIds[0] != -1);
    ImFontAtlas::CustomRect& r = CustomRects[CustomRectIds[0]];
    IM_ASSERT(r.ID == FONT_ATLAS_DEFAULT_TEX_DATA_ID);
    ImVec2 pos = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][0] + ImVec2((float)r.X, (float)r.Y);
    ImVec2 size = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][1];
    *out_size = size;
    *out_offset = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][2];
    out_uv_border[0] = (pos) * TexUvScale;
    out_uv_border[1] = (pos + size) * TexUvScale;
    pos.x += FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF + 1;
    out_uv_fill[0] = (pos) * TexUvScale;
    out_uv_fill[1] = (pos + size) * TexUvScale;
    return true;
}

bool    ImFontAtlas::Build()
{
    return ImFontAtlasBuildWithStbTruetype(this);
}

void    ImFontAtlasBuildMultiplyCalcLookupTable(unsigned char out_table[256], float in_brighten_factor)
{
    for (unsigned int i = 0; i < 256; i++)
    {
        unsigned int value = (unsigned int)(i * in_brighten_factor);
        out_table[i] = value > 255 ? 255 : (value & 0xFF);
    }
}

void    ImFontAtlasBuildMultiplyRectAlpha8(const unsigned char table[256], unsigned char* pixels, int x, int y, int w, int h, int stride)
{
    unsigned char* data = pixels + x + y * stride;
    for (int j = h; j > 0; j--, data += stride)
        for (int i = 0; i < w; i++)
            data[i] = table[data[i]];
}

bool    ImFontAtlasBuildWithStbTruetype(ImFontAtlas* atlas)
{
    IM_ASSERT(atlas->ConfigData.Size > 0);

    ImFontAtlasBuildRegisterDefaultCustomRects(atlas);

    atlas->TexID = NULL;
    atlas->TexWidth = atlas->TexHeight = 0;
    atlas->TexUvScale = ImVec2(0.0f, 0.0f);
    atlas->TexUvWhitePixel = ImVec2(0.0f, 0.0f);
    atlas->ClearTexData();

    // Count glyphs/ranges
    int total_glyphs_count = 0;
    int total_ranges_count = 0;
    for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[input_i];
        if (!cfg.GlyphRanges)
            cfg.GlyphRanges = atlas->GetGlyphRangesDefault();
        for (const ImWchar* in_range = cfg.GlyphRanges; in_range[0] && in_range[1]; in_range += 2, total_ranges_count++)
            total_glyphs_count += (in_range[1] - in_range[0]) + 1;
    }

    // We need a width for the skyline algorithm. Using a dumb heuristic here to decide of width. User can override TexDesiredWidth and TexGlyphPadding if they wish.
    // Width doesn't really matter much, but some API/GPU have texture size limitations and increasing width can decrease height.
    atlas->TexWidth = (atlas->TexDesiredWidth > 0) ? atlas->TexDesiredWidth : (total_glyphs_count > 4000) ? 4096 : (total_glyphs_count > 2000) ? 2048 : (total_glyphs_count > 1000) ? 1024 : 512;
    atlas->TexHeight = 0;

    // Start packing
    const int max_tex_height = 1024*32;
    stbtt_pack_context spc = {};
    if (!stbtt_PackBegin(&spc, NULL, atlas->TexWidth, max_tex_height, 0, atlas->TexGlyphPadding, NULL))
        return false;
    stbtt_PackSetOversampling(&spc, 1, 1);

    // Pack our extra data rectangles first, so it will be on the upper-left corner of our texture (UV will have small values).
    ImFontAtlasBuildPackCustomRects(atlas, spc.pack_info);

    // Initialize font information (so we can error without any cleanup)
    struct ImFontTempBuildData
    {
        stbtt_fontinfo      FontInfo;
        stbrp_rect*         Rects;
        int                 RectsCount;
        stbtt_pack_range*   Ranges;
        int                 RangesCount;
    };
    ImFontTempBuildData* tmp_array = (ImFontTempBuildData*)ImGui::MemAlloc((size_t)atlas->ConfigData.Size * sizeof(ImFontTempBuildData));
    for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[input_i];
        ImFontTempBuildData& tmp = tmp_array[input_i];
        IM_ASSERT(cfg.DstFont && (!cfg.DstFont->IsLoaded() || cfg.DstFont->ContainerAtlas == atlas));

        const int font_offset = stbtt_GetFontOffsetForIndex((unsigned char*)cfg.FontData, cfg.FontNo);
        IM_ASSERT(font_offset >= 0);
        if (!stbtt_InitFont(&tmp.FontInfo, (unsigned char*)cfg.FontData, font_offset))
        {
            atlas->TexWidth = atlas->TexHeight = 0; // Reset output on failure
            ImGui::MemFree(tmp_array);
            return false;
        }
    }

    // Allocate packing character data and flag packed characters buffer as non-packed (x0=y0=x1=y1=0)
    int buf_packedchars_n = 0, buf_rects_n = 0, buf_ranges_n = 0;
    stbtt_packedchar* buf_packedchars = (stbtt_packedchar*)ImGui::MemAlloc(total_glyphs_count * sizeof(stbtt_packedchar));
    stbrp_rect* buf_rects = (stbrp_rect*)ImGui::MemAlloc(total_glyphs_count * sizeof(stbrp_rect));
    stbtt_pack_range* buf_ranges = (stbtt_pack_range*)ImGui::MemAlloc(total_ranges_count * sizeof(stbtt_pack_range));
    memset(buf_packedchars, 0, total_glyphs_count * sizeof(stbtt_packedchar));
    memset(buf_rects, 0, total_glyphs_count * sizeof(stbrp_rect));              // Unnecessary but let's clear this for the sake of sanity.
    memset(buf_ranges, 0, total_ranges_count * sizeof(stbtt_pack_range));

    // First font pass: pack all glyphs (no rendering at this point, we are working with rectangles in an infinitely tall texture at this point)
    for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[input_i];
        ImFontTempBuildData& tmp = tmp_array[input_i];

        // Setup ranges
        int font_glyphs_count = 0;
        int font_ranges_count = 0;
        for (const ImWchar* in_range = cfg.GlyphRanges; in_range[0] && in_range[1]; in_range += 2, font_ranges_count++)
            font_glyphs_count += (in_range[1] - in_range[0]) + 1;
        tmp.Ranges = buf_ranges + buf_ranges_n;
        tmp.RangesCount = font_ranges_count;
        buf_ranges_n += font_ranges_count;
        for (int i = 0; i < font_ranges_count; i++)
        {
            const ImWchar* in_range = &cfg.GlyphRanges[i * 2];
            stbtt_pack_range& range = tmp.Ranges[i];
            range.font_size = cfg.SizePixels;
            range.first_unicode_codepoint_in_range = in_range[0];
            range.num_chars = (in_range[1] - in_range[0]) + 1;
            range.chardata_for_range = buf_packedchars + buf_packedchars_n;
            buf_packedchars_n += range.num_chars;
        }

        // Pack
        tmp.Rects = buf_rects + buf_rects_n;
        tmp.RectsCount = font_glyphs_count;
        buf_rects_n += font_glyphs_count;
        stbtt_PackSetOversampling(&spc, cfg.OversampleH, cfg.OversampleV);
        int n = stbtt_PackFontRangesGatherRects(&spc, &tmp.FontInfo, tmp.Ranges, tmp.RangesCount, tmp.Rects);
        IM_ASSERT(n == font_glyphs_count);
        stbrp_pack_rects((stbrp_context*)spc.pack_info, tmp.Rects, n);

        // Extend texture height
        for (int i = 0; i < n; i++)
            if (tmp.Rects[i].was_packed)
                atlas->TexHeight = ImMax(atlas->TexHeight, tmp.Rects[i].y + tmp.Rects[i].h);
    }
    IM_ASSERT(buf_rects_n == total_glyphs_count);
    IM_ASSERT(buf_packedchars_n == total_glyphs_count);
    IM_ASSERT(buf_ranges_n == total_ranges_count);

    // Create texture
    atlas->TexHeight = (atlas->Flags & ImFontAtlasFlags_NoPowerOfTwoHeight) ? (atlas->TexHeight + 1) : ImUpperPowerOfTwo(atlas->TexHeight);
    atlas->TexUvScale = ImVec2(1.0f / atlas->TexWidth, 1.0f / atlas->TexHeight);
    atlas->TexPixelsAlpha8 = (unsigned char*)ImGui::MemAlloc(atlas->TexWidth * atlas->TexHeight);
    memset(atlas->TexPixelsAlpha8, 0, atlas->TexWidth * atlas->TexHeight);
    spc.pixels = atlas->TexPixelsAlpha8;
    spc.height = atlas->TexHeight;

    // Second pass: render font characters
    for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[input_i];
        ImFontTempBuildData& tmp = tmp_array[input_i];
        stbtt_PackSetOversampling(&spc, cfg.OversampleH, cfg.OversampleV);
        stbtt_PackFontRangesRenderIntoRects(&spc, &tmp.FontInfo, tmp.Ranges, tmp.RangesCount, tmp.Rects);
        if (cfg.RasterizerMultiply != 1.0f)
        {
            unsigned char multiply_table[256];
            ImFontAtlasBuildMultiplyCalcLookupTable(multiply_table, cfg.RasterizerMultiply);
            for (const stbrp_rect* r = tmp.Rects; r != tmp.Rects + tmp.RectsCount; r++)
                if (r->was_packed)
                    ImFontAtlasBuildMultiplyRectAlpha8(multiply_table, spc.pixels, r->x, r->y, r->w, r->h, spc.stride_in_bytes);
        }
        tmp.Rects = NULL;
    }

    // End packing
    stbtt_PackEnd(&spc);
    ImGui::MemFree(buf_rects);
    buf_rects = NULL;

    // Third pass: setup ImFont and glyphs for runtime
    for (int input_i = 0; input_i < atlas->ConfigData.Size; input_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[input_i];
        ImFontTempBuildData& tmp = tmp_array[input_i];
        ImFont* dst_font = cfg.DstFont; // We can have multiple input fonts writing into a same destination font (when using MergeMode=true)
        if (cfg.MergeMode)
            dst_font->BuildLookupTable();

        const float font_scale = stbtt_ScaleForPixelHeight(&tmp.FontInfo, cfg.SizePixels);
        int unscaled_ascent, unscaled_descent, unscaled_line_gap;
        stbtt_GetFontVMetrics(&tmp.FontInfo, &unscaled_ascent, &unscaled_descent, &unscaled_line_gap);

        const float ascent = ImFloor(unscaled_ascent * font_scale + ((unscaled_ascent > 0.0f) ? +1 : -1));
        const float descent = ImFloor(unscaled_descent * font_scale + ((unscaled_descent > 0.0f) ? +1 : -1));
        ImFontAtlasBuildSetupFont(atlas, dst_font, &cfg, ascent, descent);
        const float off_x = cfg.GlyphOffset.x;
        const float off_y = cfg.GlyphOffset.y + (float)(int)(dst_font->Ascent + 0.5f);

        for (int i = 0; i < tmp.RangesCount; i++)
        {
            stbtt_pack_range& range = tmp.Ranges[i];
            for (int char_idx = 0; char_idx < range.num_chars; char_idx += 1)
            {
                const stbtt_packedchar& pc = range.chardata_for_range[char_idx];
                if (!pc.x0 && !pc.x1 && !pc.y0 && !pc.y1)
                    continue;

                const int codepoint = range.first_unicode_codepoint_in_range + char_idx;
                if (cfg.MergeMode && dst_font->FindGlyphNoFallback((unsigned short)codepoint))
                    continue;

                stbtt_aligned_quad q;
                float dummy_x = 0.0f, dummy_y = 0.0f;
                stbtt_GetPackedQuad(range.chardata_for_range, atlas->TexWidth, atlas->TexHeight, char_idx, &dummy_x, &dummy_y, &q, 0);
                dst_font->AddGlyph((ImWchar)codepoint, q.x0 + off_x, q.y0 + off_y, q.x1 + off_x, q.y1 + off_y, q.s0, q.t0, q.s1, q.t1, pc.xadvance);
            }
        }
    }

    // Cleanup temporaries
    ImGui::MemFree(buf_packedchars);
    ImGui::MemFree(buf_ranges);
    ImGui::MemFree(tmp_array);

    ImFontAtlasBuildFinish(atlas);

    return true;
}

void ImFontAtlasBuildRegisterDefaultCustomRects(ImFontAtlas* atlas)
{
    if (atlas->CustomRectIds[0] >= 0)
        return;
    if (!(atlas->Flags & ImFontAtlasFlags_NoMouseCursors))
        atlas->CustomRectIds[0] = atlas->AddCustomRectRegular(FONT_ATLAS_DEFAULT_TEX_DATA_ID, FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF*2+1, FONT_ATLAS_DEFAULT_TEX_DATA_H);
    else
        atlas->CustomRectIds[0] = atlas->AddCustomRectRegular(FONT_ATLAS_DEFAULT_TEX_DATA_ID, 2, 2);
}

void ImFontAtlasBuildSetupFont(ImFontAtlas* atlas, ImFont* font, ImFontConfig* font_config, float ascent, float descent)
{
    if (!font_config->MergeMode)
    {
        font->ClearOutputData();
        font->FontSize = font_config->SizePixels;
        font->ConfigData = font_config;
        font->ContainerAtlas = atlas;
        font->Ascent = ascent;
        font->Descent = descent;
    }
    font->ConfigDataCount++;
}

void ImFontAtlasBuildPackCustomRects(ImFontAtlas* atlas, void* pack_context_opaque)
{
    stbrp_context* pack_context = (stbrp_context*)pack_context_opaque;

    ImVector<ImFontAtlas::CustomRect>& user_rects = atlas->CustomRects;
    IM_ASSERT(user_rects.Size >= 1); // We expect at least the default custom rects to be registered, else something went wrong.

    ImVector<stbrp_rect> pack_rects;
    pack_rects.resize(user_rects.Size);
    memset(pack_rects.Data, 0, sizeof(stbrp_rect) * user_rects.Size);
    for (int i = 0; i < user_rects.Size; i++)
    {
        pack_rects[i].w = user_rects[i].Width;
        pack_rects[i].h = user_rects[i].Height;
    }
    stbrp_pack_rects(pack_context, &pack_rects[0], pack_rects.Size);
    for (int i = 0; i < pack_rects.Size; i++)
        if (pack_rects[i].was_packed)
        {
            user_rects[i].X = pack_rects[i].x;
            user_rects[i].Y = pack_rects[i].y;
            IM_ASSERT(pack_rects[i].w == user_rects[i].Width && pack_rects[i].h == user_rects[i].Height);
            atlas->TexHeight = ImMax(atlas->TexHeight, pack_rects[i].y + pack_rects[i].h);
        }
}

static void ImFontAtlasBuildRenderDefaultTexData(ImFontAtlas* atlas)
{
    IM_ASSERT(atlas->CustomRectIds[0] >= 0);
    IM_ASSERT(atlas->TexPixelsAlpha8 != NULL);
    ImFontAtlas::CustomRect& r = atlas->CustomRects[atlas->CustomRectIds[0]];
    IM_ASSERT(r.ID == FONT_ATLAS_DEFAULT_TEX_DATA_ID);
    IM_ASSERT(r.IsPacked());

    const int w = atlas->TexWidth;
    if (!(atlas->Flags & ImFontAtlasFlags_NoMouseCursors))
    {
        // Render/copy pixels
        IM_ASSERT(r.Width == FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF * 2 + 1 && r.Height == FONT_ATLAS_DEFAULT_TEX_DATA_H);
        for (int y = 0, n = 0; y < FONT_ATLAS_DEFAULT_TEX_DATA_H; y++)
            for (int x = 0; x < FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF; x++, n++)
            {
                const int offset0 = (int)(r.X + x) + (int)(r.Y + y) * w;
                const int offset1 = offset0 + FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF + 1;
                atlas->TexPixelsAlpha8[offset0] = FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[n] == '.' ? 0xFF : 0x00;
                atlas->TexPixelsAlpha8[offset1] = FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[n] == 'X' ? 0xFF : 0x00;
            }
    }
    else
    {
        IM_ASSERT(r.Width == 2 && r.Height == 2);
        const int offset = (int)(r.X) + (int)(r.Y) * w;
        atlas->TexPixelsAlpha8[offset] = atlas->TexPixelsAlpha8[offset + 1] = atlas->TexPixelsAlpha8[offset + w] = atlas->TexPixelsAlpha8[offset + w + 1] = 0xFF;
    }
    atlas->TexUvWhitePixel = ImVec2((r.X + 0.5f) * atlas->TexUvScale.x, (r.Y + 0.5f) * atlas->TexUvScale.y);
}

void ImFontAtlasBuildFinish(ImFontAtlas* atlas)
{
    // Render into our custom data block
    ImFontAtlasBuildRenderDefaultTexData(atlas);

    // Register custom rectangle glyphs
    for (int i = 0; i < atlas->CustomRects.Size; i++)
    {
        const ImFontAtlas::CustomRect& r = atlas->CustomRects[i];
        if (r.Font == NULL || r.ID > 0x10000)
            continue;

        IM_ASSERT(r.Font->ContainerAtlas == atlas);
        ImVec2 uv0, uv1;
        atlas->CalcCustomRectUV(&r, &uv0, &uv1);
        r.Font->AddGlyph((ImWchar)r.ID, r.GlyphOffset.x, r.GlyphOffset.y, r.GlyphOffset.x + r.Width, r.GlyphOffset.y + r.Height, uv0.x, uv0.y, uv1.x, uv1.y, r.GlyphAdvanceX);
    }

    // Build all fonts lookup tables
    for (int i = 0; i < atlas->Fonts.Size; i++)
        if (atlas->Fonts[i]->DirtyLookupTables)
            atlas->Fonts[i]->BuildLookupTable();
}

// Retrieve list of range (2 int per range, values are inclusive)
const ImWchar*   ImFontAtlas::GetGlyphRangesDefault()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesKorean()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3131, 0x3163, // Korean alphabets
        0xAC00, 0xD79D, // Korean characters
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesChinese()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3000, 0x30FF, // Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
        0x4e00, 0x9FAF, // CJK Ideograms
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesJapanese()
{
    // Store the 1946 ideograms code points as successive offsets from the initial unicode codepoint 0x4E00. Each offset has an implicit +1.
    // This encoding is designed to helps us reduce the source code size.
    // FIXME: Source a list of the revised 2136 joyo kanji list from 2010 and rebuild this.
    // The current list was sourced from http://theinstructionlimit.com/author/renaudbedardrenaudbedard/page/3
    // Note that you may use ImFontAtlas::GlyphRangesBuilder to create your own ranges, by merging existing ranges or adding new characters.
    static const short offsets_from_0x4E00[] =
    {
        -1,0,1,3,0,0,0,0,1,0,5,1,1,0,7,4,6,10,0,1,9,9,7,1,3,19,1,10,7,1,0,1,0,5,1,0,6,4,2,6,0,0,12,6,8,0,3,5,0,1,0,9,0,0,8,1,1,3,4,5,13,0,0,8,2,17,
        4,3,1,1,9,6,0,0,0,2,1,3,2,22,1,9,11,1,13,1,3,12,0,5,9,2,0,6,12,5,3,12,4,1,2,16,1,1,4,6,5,3,0,6,13,15,5,12,8,14,0,0,6,15,3,6,0,18,8,1,6,14,1,
        5,4,12,24,3,13,12,10,24,0,0,0,1,0,1,1,2,9,10,2,2,0,0,3,3,1,0,3,8,0,3,2,4,4,1,6,11,10,14,6,15,3,4,15,1,0,0,5,2,2,0,0,1,6,5,5,6,0,3,6,5,0,0,1,0,
        11,2,2,8,4,7,0,10,0,1,2,17,19,3,0,2,5,0,6,2,4,4,6,1,1,11,2,0,3,1,2,1,2,10,7,6,3,16,0,8,24,0,0,3,1,1,3,0,1,6,0,0,0,2,0,1,5,15,0,1,0,0,2,11,19,
        1,4,19,7,6,5,1,0,0,0,0,5,1,0,1,9,0,0,5,0,2,0,1,0,3,0,11,3,0,2,0,0,0,0,0,9,3,6,4,12,0,14,0,0,29,10,8,0,14,37,13,0,31,16,19,0,8,30,1,20,8,3,48,
        21,1,0,12,0,10,44,34,42,54,11,18,82,0,2,1,2,12,1,0,6,2,17,2,12,7,0,7,17,4,2,6,24,23,8,23,39,2,16,23,1,0,5,1,2,15,14,5,6,2,11,0,8,6,2,2,2,14,
        20,4,15,3,4,11,10,10,2,5,2,1,30,2,1,0,0,22,5,5,0,3,1,5,4,1,0,0,2,2,21,1,5,1,2,16,2,1,3,4,0,8,4,0,0,5,14,11,2,16,1,13,1,7,0,22,15,3,1,22,7,14,
        22,19,11,24,18,46,10,20,64,45,3,2,0,4,5,0,1,4,25,1,0,0,2,10,0,0,0,1,0,1,2,0,0,9,1,2,0,0,0,2,5,2,1,1,5,5,8,1,1,1,5,1,4,9,1,3,0,1,0,1,1,2,0,0,
        2,0,1,8,22,8,1,0,0,0,0,4,2,1,0,9,8,5,0,9,1,30,24,2,6,4,39,0,14,5,16,6,26,179,0,2,1,1,0,0,0,5,2,9,6,0,2,5,16,7,5,1,1,0,2,4,4,7,15,13,14,0,0,
        3,0,1,0,0,0,2,1,6,4,5,1,4,9,0,3,1,8,0,0,10,5,0,43,0,2,6,8,4,0,2,0,0,9,6,0,9,3,1,6,20,14,6,1,4,0,7,2,3,0,2,0,5,0,3,1,0,3,9,7,0,3,4,0,4,9,1,6,0,
        9,0,0,2,3,10,9,28,3,6,2,4,1,2,32,4,1,18,2,0,3,1,5,30,10,0,2,2,2,0,7,9,8,11,10,11,7,2,13,7,5,10,0,3,40,2,0,1,6,12,0,4,5,1,5,11,11,21,4,8,3,7,
        8,8,33,5,23,0,0,19,8,8,2,3,0,6,1,1,1,5,1,27,4,2,5,0,3,5,6,3,1,0,3,1,12,5,3,3,2,0,7,7,2,1,0,4,0,1,1,2,0,10,10,6,2,5,9,7,5,15,15,21,6,11,5,20,
        4,3,5,5,2,5,0,2,1,0,1,7,28,0,9,0,5,12,5,5,18,30,0,12,3,3,21,16,25,32,9,3,14,11,24,5,66,9,1,2,0,5,9,1,5,1,8,0,8,3,3,0,1,15,1,4,8,1,2,7,0,7,2,
        8,3,7,5,3,7,10,2,1,0,0,2,25,0,6,4,0,10,0,4,2,4,1,12,5,38,4,0,4,1,10,5,9,4,0,14,4,2,5,18,20,21,1,3,0,5,0,7,0,3,7,1,3,1,1,8,1,0,0,0,3,2,5,2,11,
        6,0,13,1,3,9,1,12,0,16,6,2,1,0,2,1,12,6,13,11,2,0,28,1,7,8,14,13,8,13,0,2,0,5,4,8,10,2,37,42,19,6,6,7,4,14,11,18,14,80,7,6,0,4,72,12,36,27,
        7,7,0,14,17,19,164,27,0,5,10,7,3,13,6,14,0,2,2,5,3,0,6,13,0,0,10,29,0,4,0,3,13,0,3,1,6,51,1,5,28,2,0,8,0,20,2,4,0,25,2,10,13,10,0,16,4,0,1,0,
        2,1,7,0,1,8,11,0,0,1,2,7,2,23,11,6,6,4,16,2,2,2,0,22,9,3,3,5,2,0,15,16,21,2,9,20,15,15,5,3,9,1,0,0,1,7,7,5,4,2,2,2,38,24,14,0,0,15,5,6,24,14,
        5,5,11,0,21,12,0,3,8,4,11,1,8,0,11,27,7,2,4,9,21,59,0,1,39,3,60,62,3,0,12,11,0,3,30,11,0,13,88,4,15,5,28,13,1,4,48,17,17,4,28,32,46,0,16,0,
        18,11,1,8,6,38,11,2,6,11,38,2,0,45,3,11,2,7,8,4,30,14,17,2,1,1,65,18,12,16,4,2,45,123,12,56,33,1,4,3,4,7,0,0,0,3,2,0,16,4,2,4,2,0,7,4,5,2,26,
        2,25,6,11,6,1,16,2,6,17,77,15,3,35,0,1,0,5,1,0,38,16,6,3,12,3,3,3,0,9,3,1,3,5,2,9,0,18,0,25,1,3,32,1,72,46,6,2,7,1,3,14,17,0,28,1,40,13,0,20,
        15,40,6,38,24,12,43,1,1,9,0,12,6,0,6,2,4,19,3,7,1,48,0,9,5,0,5,6,9,6,10,15,2,11,19,3,9,2,0,1,10,1,27,8,1,3,6,1,14,0,26,0,27,16,3,4,9,6,2,23,
        9,10,5,25,2,1,6,1,1,48,15,9,15,14,3,4,26,60,29,13,37,21,1,6,4,0,2,11,22,23,16,16,2,2,1,3,0,5,1,6,4,0,0,4,0,0,8,3,0,2,5,0,7,1,7,3,13,2,4,10,
        3,0,2,31,0,18,3,0,12,10,4,1,0,7,5,7,0,5,4,12,2,22,10,4,2,15,2,8,9,0,23,2,197,51,3,1,1,4,13,4,3,21,4,19,3,10,5,40,0,4,1,1,10,4,1,27,34,7,21,
        2,17,2,9,6,4,2,3,0,4,2,7,8,2,5,1,15,21,3,4,4,2,2,17,22,1,5,22,4,26,7,0,32,1,11,42,15,4,1,2,5,0,19,3,1,8,6,0,10,1,9,2,13,30,8,2,24,17,19,1,4,
        4,25,13,0,10,16,11,39,18,8,5,30,82,1,6,8,18,77,11,13,20,75,11,112,78,33,3,0,0,60,17,84,9,1,1,12,30,10,49,5,32,158,178,5,5,6,3,3,1,3,1,4,7,6,
        19,31,21,0,2,9,5,6,27,4,9,8,1,76,18,12,1,4,0,3,3,6,3,12,2,8,30,16,2,25,1,5,5,4,3,0,6,10,2,3,1,0,5,1,19,3,0,8,1,5,2,6,0,0,0,19,1,2,0,5,1,2,5,
        1,3,7,0,4,12,7,3,10,22,0,9,5,1,0,2,20,1,1,3,23,30,3,9,9,1,4,191,14,3,15,6,8,50,0,1,0,0,4,0,0,1,0,2,4,2,0,2,3,0,2,0,2,2,8,7,0,1,1,1,3,3,17,11,
        91,1,9,3,2,13,4,24,15,41,3,13,3,1,20,4,125,29,30,1,0,4,12,2,21,4,5,5,19,11,0,13,11,86,2,18,0,7,1,8,8,2,2,22,1,2,6,5,2,0,1,2,8,0,2,0,5,2,1,0,
        2,10,2,0,5,9,2,1,2,0,1,0,4,0,0,10,2,5,3,0,6,1,0,1,4,4,33,3,13,17,3,18,6,4,7,1,5,78,0,4,1,13,7,1,8,1,0,35,27,15,3,0,0,0,1,11,5,41,38,15,22,6,
        14,14,2,1,11,6,20,63,5,8,27,7,11,2,2,40,58,23,50,54,56,293,8,8,1,5,1,14,0,1,12,37,89,8,8,8,2,10,6,0,0,0,4,5,2,1,0,1,1,2,7,0,3,3,0,4,6,0,3,2,
        19,3,8,0,0,0,4,4,16,0,4,1,5,1,3,0,3,4,6,2,17,10,10,31,6,4,3,6,10,126,7,3,2,2,0,9,0,0,5,20,13,0,15,0,6,0,2,5,8,64,50,3,2,12,2,9,0,0,11,8,20,
        109,2,18,23,0,0,9,61,3,0,28,41,77,27,19,17,81,5,2,14,5,83,57,252,14,154,263,14,20,8,13,6,57,39,38,
    };
    static ImWchar base_ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3000, 0x30FF, // Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
    };
    static bool full_ranges_unpacked = false;
    static ImWchar full_ranges[IM_ARRAYSIZE(base_ranges) + IM_ARRAYSIZE(offsets_from_0x4E00)*2 + 1];
    if (!full_ranges_unpacked)
    {
        // Unpack
        int codepoint = 0x4e00;
        memcpy(full_ranges, base_ranges, sizeof(base_ranges));
        ImWchar* dst = full_ranges + IM_ARRAYSIZE(base_ranges);
        for (int n = 0; n < IM_ARRAYSIZE(offsets_from_0x4E00); n++, dst += 2)
            dst[0] = dst[1] = (ImWchar)(codepoint += (offsets_from_0x4E00[n] + 1));
        dst[0] = 0;
        full_ranges_unpacked = true;
    }
    return &full_ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesCyrillic()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
        0x2DE0, 0x2DFF, // Cyrillic Extended-A
        0xA640, 0xA69F, // Cyrillic Extended-B
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesThai()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin
        0x2010, 0x205E, // Punctuations
        0x0E00, 0x0E7F, // Thai
        0,
    };
    return &ranges[0];
}

//-----------------------------------------------------------------------------
// ImFontAtlas::GlyphRangesBuilder
//-----------------------------------------------------------------------------

void ImFontAtlas::GlyphRangesBuilder::AddText(const char* text, const char* text_end)
{
    while (text_end ? (text < text_end) : *text)
    {
        unsigned int c = 0;
        int c_len = ImTextCharFromUtf8(&c, text, text_end);
        text += c_len;
        if (c_len == 0)
            break;
        if (c < 0x10000)
            AddChar((ImWchar)c);
    }
}

void ImFontAtlas::GlyphRangesBuilder::AddRanges(const ImWchar* ranges)
{
    for (; ranges[0]; ranges += 2)
        for (ImWchar c = ranges[0]; c <= ranges[1]; c++)
            AddChar(c);
}

void ImFontAtlas::GlyphRangesBuilder::BuildRanges(ImVector<ImWchar>* out_ranges)
{
    for (int n = 0; n < 0x10000; n++)
        if (GetBit(n))
        {
            out_ranges->push_back((ImWchar)n);
            while (n < 0x10000 && GetBit(n + 1))
                n++;
            out_ranges->push_back((ImWchar)n);
        }
    out_ranges->push_back(0);
}

//-----------------------------------------------------------------------------
// ImFont
//-----------------------------------------------------------------------------

ImFont::ImFont()
{
    Scale = 1.0f;
    FallbackChar = (ImWchar)'?';
    DisplayOffset = ImVec2(0.0f, 0.0f);
    ClearOutputData();
}

ImFont::~ImFont()
{
    // Invalidate active font so that the user gets a clear crash instead of a dangling pointer.
    // If you want to delete fonts you need to do it between Render() and NewFrame().
    // FIXME-CLEANUP
    /*
    ImGuiContext& g = *GImGui;
    if (g.Font == this)
        g.Font = NULL;
    */
    ClearOutputData();
}

void    ImFont::ClearOutputData()
{
    FontSize = 0.0f;
    Glyphs.clear();
    IndexAdvanceX.clear();
    IndexLookup.clear();
    FallbackGlyph = NULL;
    FallbackAdvanceX = 0.0f;
    ConfigDataCount = 0;
    ConfigData = NULL;
    ContainerAtlas = NULL;
    Ascent = Descent = 0.0f;
    DirtyLookupTables = true;
    MetricsTotalSurface = 0;
}

void ImFont::BuildLookupTable()
{
    int max_codepoint = 0;
    for (int i = 0; i != Glyphs.Size; i++)
        max_codepoint = ImMax(max_codepoint, (int)Glyphs[i].Codepoint);

    IM_ASSERT(Glyphs.Size < 0xFFFF); // -1 is reserved
    IndexAdvanceX.clear();
    IndexLookup.clear();
    DirtyLookupTables = false;
    GrowIndex(max_codepoint + 1);
    for (int i = 0; i < Glyphs.Size; i++)
    {
        int codepoint = (int)Glyphs[i].Codepoint;
        IndexAdvanceX[codepoint] = Glyphs[i].AdvanceX;
        IndexLookup[codepoint] = (unsigned short)i;
    }

    // Create a glyph to handle TAB
    // FIXME: Needs proper TAB handling but it needs to be contextualized (or we could arbitrary say that each string starts at "column 0" ?)
    if (FindGlyph((unsigned short)' '))
    {
        if (Glyphs.back().Codepoint != '\t')   // So we can call this function multiple times
            Glyphs.resize(Glyphs.Size + 1);
        ImFontGlyph& tab_glyph = Glyphs.back();
        tab_glyph = *FindGlyph((unsigned short)' ');
        tab_glyph.Codepoint = '\t';
        tab_glyph.AdvanceX *= 4;
        IndexAdvanceX[(int)tab_glyph.Codepoint] = (float)tab_glyph.AdvanceX;
        IndexLookup[(int)tab_glyph.Codepoint] = (unsigned short)(Glyphs.Size-1);
    }

    FallbackGlyph = FindGlyphNoFallback(FallbackChar);
    FallbackAdvanceX = FallbackGlyph ? FallbackGlyph->AdvanceX : 0.0f;
    for (int i = 0; i < max_codepoint + 1; i++)
        if (IndexAdvanceX[i] < 0.0f)
            IndexAdvanceX[i] = FallbackAdvanceX;
}

void ImFont::SetFallbackChar(ImWchar c)
{
    FallbackChar = c;
    BuildLookupTable();
}

void ImFont::GrowIndex(int new_size)
{
    IM_ASSERT(IndexAdvanceX.Size == IndexLookup.Size);
    if (new_size <= IndexLookup.Size)
        return;
    IndexAdvanceX.resize(new_size, -1.0f);
    IndexLookup.resize(new_size, (unsigned short)-1);
}

void ImFont::AddGlyph(ImWchar codepoint, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float advance_x)
{
    Glyphs.resize(Glyphs.Size + 1);
    ImFontGlyph& glyph = Glyphs.back();
    glyph.Codepoint = (ImWchar)codepoint;
    glyph.X0 = x0; 
    glyph.Y0 = y0; 
    glyph.X1 = x1; 
    glyph.Y1 = y1;
    glyph.U0 = u0; 
    glyph.V0 = v0; 
    glyph.U1 = u1; 
    glyph.V1 = v1;
    glyph.AdvanceX = advance_x + ConfigData->GlyphExtraSpacing.x;  // Bake spacing into AdvanceX

    if (ConfigData->PixelSnapH)
        glyph.AdvanceX = (float)(int)(glyph.AdvanceX + 0.5f);
    
    // Compute rough surface usage metrics (+1 to account for average padding, +0.99 to round)
    DirtyLookupTables = true;
    MetricsTotalSurface += (int)((glyph.U1 - glyph.U0) * ContainerAtlas->TexWidth + 1.99f) * (int)((glyph.V1 - glyph.V0) * ContainerAtlas->TexHeight + 1.99f);
}

void ImFont::AddRemapChar(ImWchar dst, ImWchar src, bool overwrite_dst)
{
    IM_ASSERT(IndexLookup.Size > 0);    // Currently this can only be called AFTER the font has been built, aka after calling ImFontAtlas::GetTexDataAs*() function.
    int index_size = IndexLookup.Size;

    if (dst < index_size && IndexLookup.Data[dst] == (unsigned short)-1 && !overwrite_dst) // 'dst' already exists
        return;
    if (src >= index_size && dst >= index_size) // both 'dst' and 'src' don't exist -> no-op
        return;

    GrowIndex(dst + 1);
    IndexLookup[dst] = (src < index_size) ? IndexLookup.Data[src] : (unsigned short)-1;
    IndexAdvanceX[dst] = (src < index_size) ? IndexAdvanceX.Data[src] : 1.0f;
}

const ImFontGlyph* ImFont::FindGlyph(ImWchar c) const
{
    if (c >= IndexLookup.Size)
        return FallbackGlyph;
    const unsigned short i = IndexLookup[c];
    if (i == (unsigned short)-1)
        return FallbackGlyph;
    return &Glyphs.Data[i];
}

const ImFontGlyph* ImFont::FindGlyphNoFallback(ImWchar c) const
{
    if (c >= IndexLookup.Size)
        return NULL;
    const unsigned short i = IndexLookup[c];
    if (i == (unsigned short)-1)
        return NULL;
    return &Glyphs.Data[i];
}

const char* ImFont::CalcWordWrapPositionA(float scale, const char* text, const char* text_end, float wrap_width) const
{
    // Simple word-wrapping for English, not full-featured. Please submit failing cases!
    // FIXME: Much possible improvements (don't cut things like "word !", "word!!!" but cut within "word,,,,", more sensible support for punctuations, support for Unicode punctuations, etc.)

    // For references, possible wrap point marked with ^
    //  "aaa bbb, ccc,ddd. eee   fff. ggg!"
    //      ^    ^    ^   ^   ^__    ^    ^

    // List of hardcoded separators: .,;!?'"

    // Skip extra blanks after a line returns (that includes not counting them in width computation)
    // e.g. "Hello    world" --> "Hello" "World"

    // Cut words that cannot possibly fit within one line.
    // e.g.: "The tropical fish" with ~5 characters worth of width --> "The tr" "opical" "fish"

    float line_width = 0.0f;
    float word_width = 0.0f;
    float blank_width = 0.0f;
    wrap_width /= scale; // We work with unscaled widths to avoid scaling every characters

    const char* word_end = text;
    const char* prev_word_end = NULL;
    bool inside_word = true;

    const char* s = text;
    while (s < text_end)
    {
        unsigned int c = (unsigned int)*s;
        const char* next_s;
        if (c < 0x80)
            next_s = s + 1;
        else
            next_s = s + ImTextCharFromUtf8(&c, s, text_end);
        if (c == 0)
            break;

        if (c < 32)
        {
            if (c == '\n')
            {
                line_width = word_width = blank_width = 0.0f;
                inside_word = true;
                s = next_s;
                continue;
            }
            if (c == '\r')
            {
                s = next_s;
                continue;
            }
        }

        const float char_width = ((int)c < IndexAdvanceX.Size ? IndexAdvanceX[(int)c] : FallbackAdvanceX);
        if (ImCharIsSpace(c))
        {
            if (inside_word)
            {
                line_width += blank_width;
                blank_width = 0.0f;
                word_end = s;
            }
            blank_width += char_width;
            inside_word = false;
        }
        else
        {
            word_width += char_width;
            if (inside_word)
            {
                word_end = next_s;
            }
            else
            {
                prev_word_end = word_end;
                line_width += word_width + blank_width;
                word_width = blank_width = 0.0f;
            }

            // Allow wrapping after punctuation.
            inside_word = !(c == '.' || c == ',' || c == ';' || c == '!' || c == '?' || c == '\"');
        }

        // We ignore blank width at the end of the line (they can be skipped)
        if (line_width + word_width >= wrap_width)
        {
            // Words that cannot possibly fit within an entire line will be cut anywhere.
            if (word_width < wrap_width)
                s = prev_word_end ? prev_word_end : word_end;
            break;
        }

        s = next_s;
    }

    return s;
}

ImVec2 ImFont::CalcTextSizeA(float size, float max_width, float wrap_width, const char* text_begin, const char* text_end, const char** remaining) const
{
    if (!text_end)
        text_end = text_begin + strlen(text_begin); // FIXME-OPT: Need to avoid this.

    const float line_height = size;
    const float scale = size / FontSize;

    ImVec2 text_size = ImVec2(0,0);
    float line_width = 0.0f;

    const bool word_wrap_enabled = (wrap_width > 0.0f);
    const char* word_wrap_eol = NULL;

    const char* s = text_begin;
    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
            {
                word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - line_width);
                if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
                    word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
            }

            if (s >= word_wrap_eol)
            {
                if (text_size.x < line_width)
                    text_size.x = line_width;
                text_size.y += line_height;
                line_width = 0.0f;
                word_wrap_eol = NULL;

                // Wrapping skips upcoming blanks
                while (s < text_end)
                {
                    const char c = *s;
                    if (ImCharIsSpace((unsigned int)c)) { s++; } else if (c == '\n') { s++; break; } else { break; }
                }
                continue;
            }
        }

        // Decode and advance source
        const char* prev_s = s;
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
        {
            s += 1;
        }
        else
        {
            s += ImTextCharFromUtf8(&c, s, text_end);
            if (c == 0) // Malformed UTF-8?
                break;
        }

        if (c < 32)
        {
            if (c == '\n')
            {
                text_size.x = ImMax(text_size.x, line_width);
                text_size.y += line_height;
                line_width = 0.0f;
                continue;
            }
            if (c == '\r')
                continue;
        }

        const float char_width = ((int)c < IndexAdvanceX.Size ? IndexAdvanceX[(int)c] : FallbackAdvanceX) * scale;
        if (line_width + char_width >= max_width)
        {
            s = prev_s;
            break;
        }

        line_width += char_width;
    }

    if (text_size.x < line_width)
        text_size.x = line_width;

    if (line_width > 0 || text_size.y == 0.0f)
        text_size.y += line_height;

    if (remaining)
        *remaining = s;

    return text_size;
}

void ImFont::RenderChar(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, unsigned short c) const
{
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') // Match behavior of RenderText(), those 4 codepoints are hard-coded.
        return;
    if (const ImFontGlyph* glyph = FindGlyph(c))
    {
        float scale = (size >= 0.0f) ? (size / FontSize) : 1.0f;
        pos.x = (float)(int)pos.x + DisplayOffset.x;
        pos.y = (float)(int)pos.y + DisplayOffset.y;
        draw_list->PrimReserve(6, 4);
        draw_list->PrimRectUV(ImVec2(pos.x + glyph->X0 * scale, pos.y + glyph->Y0 * scale), ImVec2(pos.x + glyph->X1 * scale, pos.y + glyph->Y1 * scale), ImVec2(glyph->U0, glyph->V0), ImVec2(glyph->U1, glyph->V1), col);
    }
}

void ImFont::RenderText(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, const ImVec4& clip_rect, const char* text_begin, const char* text_end, float wrap_width, bool cpu_fine_clip) const
{
    if (!text_end)
        text_end = text_begin + strlen(text_begin); // ImGui functions generally already provides a valid text_end, so this is merely to handle direct calls.

    // Align to be pixel perfect
    pos.x = (float)(int)pos.x + DisplayOffset.x;
    pos.y = (float)(int)pos.y + DisplayOffset.y;
    float x = pos.x;
    float y = pos.y;
    if (y > clip_rect.w)
        return;

    const float scale = size / FontSize;
    const float line_height = FontSize * scale;
    const bool word_wrap_enabled = (wrap_width > 0.0f);
    const char* word_wrap_eol = NULL;

    // Skip non-visible lines
    const char* s = text_begin;
    if (!word_wrap_enabled && y + line_height < clip_rect.y)
        while (s < text_end && *s != '\n')  // Fast-forward to next line
            s++;

    // Reserve vertices for remaining worse case (over-reserving is useful and easily amortized)
    const int vtx_count_max = (int)(text_end - s) * 4;
    const int idx_count_max = (int)(text_end - s) * 6;
    const int idx_expected_size = draw_list->IdxBuffer.Size + idx_count_max;
    draw_list->PrimReserve(idx_count_max, vtx_count_max);

    ImDrawVert* vtx_write = draw_list->_VtxWritePtr;
    ImDrawIdx* idx_write = draw_list->_IdxWritePtr;
    unsigned int vtx_current_idx = draw_list->_VtxCurrentIdx;

    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
            {
                word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - (x - pos.x));
                if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
                    word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
            }

            if (s >= word_wrap_eol)
            {
                x = pos.x;
                y += line_height;
                word_wrap_eol = NULL;

                // Wrapping skips upcoming blanks
                while (s < text_end)
                {
                    const char c = *s;
                    if (ImCharIsSpace((unsigned int)c)) { s++; } else if (c == '\n') { s++; break; } else { break; }
                }
                continue;
            }
        }

        // Decode and advance source
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
        {
            s += 1;
        }
        else
        {
            s += ImTextCharFromUtf8(&c, s, text_end);
            if (c == 0) // Malformed UTF-8?
                break;
        }

        if (c < 32)
        {
            if (c == '\n')
            {
                x = pos.x;
                y += line_height;

                if (y > clip_rect.w)
                    break;
                if (!word_wrap_enabled && y + line_height < clip_rect.y)
                    while (s < text_end && *s != '\n')  // Fast-forward to next line
                        s++;
                continue;
            }
            if (c == '\r')
                continue;
        }

        float char_width = 0.0f;
        if (const ImFontGlyph* glyph = FindGlyph((unsigned short)c))
        {
            char_width = glyph->AdvanceX * scale;

            // Arbitrarily assume that both space and tabs are empty glyphs as an optimization
            if (c != ' ' && c != '\t')
            {
                // We don't do a second finer clipping test on the Y axis as we've already skipped anything before clip_rect.y and exit once we pass clip_rect.w
                float x1 = x + glyph->X0 * scale;
                float x2 = x + glyph->X1 * scale;
                float y1 = y + glyph->Y0 * scale;
                float y2 = y + glyph->Y1 * scale;
                if (x1 <= clip_rect.z && x2 >= clip_rect.x)
                {
                    // Render a character
                    float u1 = glyph->U0;
                    float v1 = glyph->V0;
                    float u2 = glyph->U1;
                    float v2 = glyph->V1;

                    // CPU side clipping used to fit text in their frame when the frame is too small. Only does clipping for axis aligned quads.
                    if (cpu_fine_clip)
                    {
                        if (x1 < clip_rect.x)
                        {
                            u1 = u1 + (1.0f - (x2 - clip_rect.x) / (x2 - x1)) * (u2 - u1);
                            x1 = clip_rect.x;
                        }
                        if (y1 < clip_rect.y)
                        {
                            v1 = v1 + (1.0f - (y2 - clip_rect.y) / (y2 - y1)) * (v2 - v1);
                            y1 = clip_rect.y;
                        }
                        if (x2 > clip_rect.z)
                        {
                            u2 = u1 + ((clip_rect.z - x1) / (x2 - x1)) * (u2 - u1);
                            x2 = clip_rect.z;
                        }
                        if (y2 > clip_rect.w)
                        {
                            v2 = v1 + ((clip_rect.w - y1) / (y2 - y1)) * (v2 - v1);
                            y2 = clip_rect.w;
                        }
                        if (y1 >= y2)
                        {
                            x += char_width;
                            continue;
                        }
                    }

                    // We are NOT calling PrimRectUV() here because non-inlined causes too much overhead in a debug builds. Inlined here:
                    {
                        idx_write[0] = (ImDrawIdx)(vtx_current_idx); idx_write[1] = (ImDrawIdx)(vtx_current_idx+1); idx_write[2] = (ImDrawIdx)(vtx_current_idx+2);
                        idx_write[3] = (ImDrawIdx)(vtx_current_idx); idx_write[4] = (ImDrawIdx)(vtx_current_idx+2); idx_write[5] = (ImDrawIdx)(vtx_current_idx+3);
                        vtx_write[0].pos.x = x1; vtx_write[0].pos.y = y1; vtx_write[0].col = col; vtx_write[0].uv.x = u1; vtx_write[0].uv.y = v1;
                        vtx_write[1].pos.x = x2; vtx_write[1].pos.y = y1; vtx_write[1].col = col; vtx_write[1].uv.x = u2; vtx_write[1].uv.y = v1;
                        vtx_write[2].pos.x = x2; vtx_write[2].pos.y = y2; vtx_write[2].col = col; vtx_write[2].uv.x = u2; vtx_write[2].uv.y = v2;
                        vtx_write[3].pos.x = x1; vtx_write[3].pos.y = y2; vtx_write[3].col = col; vtx_write[3].uv.x = u1; vtx_write[3].uv.y = v2;
                        vtx_write += 4;
                        vtx_current_idx += 4;
                        idx_write += 6;
                    }
                }
            }
        }

        x += char_width;
    }

    // Give back unused vertices
    draw_list->VtxBuffer.resize((int)(vtx_write - draw_list->VtxBuffer.Data));
    draw_list->IdxBuffer.resize((int)(idx_write - draw_list->IdxBuffer.Data));
    draw_list->CmdBuffer[draw_list->CmdBuffer.Size-1].ElemCount -= (idx_expected_size - draw_list->IdxBuffer.Size);
    draw_list->_VtxWritePtr = vtx_write;
    draw_list->_IdxWritePtr = idx_write;
    draw_list->_VtxCurrentIdx = (unsigned int)draw_list->VtxBuffer.Size;
}

//-----------------------------------------------------------------------------
// Internals Drawing Helpers
//-----------------------------------------------------------------------------

static inline float ImAcos01(float x)
{
    if (x <= 0.0f) return IM_PI * 0.5f;
    if (x >= 1.0f) return 0.0f;
    return acosf(x);
    //return (-0.69813170079773212f * x * x - 0.87266462599716477f) * x + 1.5707963267948966f; // Cheap approximation, may be enough for what we do.
}

// FIXME: Cleanup and move code to ImDrawList.
void ImGui::RenderRectFilledRangeH(ImDrawList* draw_list, const ImRect& rect, ImU32 col, float x_start_norm, float x_end_norm, float rounding)
{
    if (x_end_norm == x_start_norm)
        return;
    if (x_start_norm > x_end_norm)
        ImSwap(x_start_norm, x_end_norm);

    ImVec2 p0 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_start_norm), rect.Min.y);
    ImVec2 p1 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_end_norm), rect.Max.y);
    if (rounding == 0.0f)
    {
        draw_list->AddRectFilled(p0, p1, col, 0.0f);
        return;
    }

    rounding = ImClamp(ImMin((rect.Max.x - rect.Min.x) * 0.5f, (rect.Max.y - rect.Min.y) * 0.5f) - 1.0f, 0.0f, rounding);
    const float inv_rounding = 1.0f / rounding;
    const float arc0_b = ImAcos01(1.0f - (p0.x - rect.Min.x) * inv_rounding);
    const float arc0_e = ImAcos01(1.0f - (p1.x - rect.Min.x) * inv_rounding);
    const float x0 = ImMax(p0.x, rect.Min.x + rounding);
    if (arc0_b == arc0_e)
    {
        draw_list->PathLineTo(ImVec2(x0, p1.y));
        draw_list->PathLineTo(ImVec2(x0, p0.y));
    }
    else if (arc0_b == 0.0f && arc0_e == IM_PI*0.5f)
    {
        draw_list->PathArcToFast(ImVec2(x0, p1.y - rounding), rounding, 3, 6); // BL
        draw_list->PathArcToFast(ImVec2(x0, p0.y + rounding), rounding, 6, 9); // TR
    }
    else
    {
        draw_list->PathArcTo(ImVec2(x0, p1.y - rounding), rounding, IM_PI - arc0_e, IM_PI - arc0_b, 3); // BL
        draw_list->PathArcTo(ImVec2(x0, p0.y + rounding), rounding, IM_PI + arc0_b, IM_PI + arc0_e, 3); // TR
    }
    if (p1.x > rect.Min.x + rounding)
    {
        const float arc1_b = ImAcos01(1.0f - (rect.Max.x - p1.x) * inv_rounding);
        const float arc1_e = ImAcos01(1.0f - (rect.Max.x - p0.x) * inv_rounding);
        const float x1 = ImMin(p1.x, rect.Max.x - rounding);
        if (arc1_b == arc1_e)
        {
            draw_list->PathLineTo(ImVec2(x1, p0.y));
            draw_list->PathLineTo(ImVec2(x1, p1.y));
        }
        else if (arc1_b == 0.0f && arc1_e == IM_PI*0.5f)
        {
            draw_list->PathArcToFast(ImVec2(x1, p0.y + rounding), rounding, 9, 12); // TR
            draw_list->PathArcToFast(ImVec2(x1, p1.y - rounding), rounding, 0, 3);  // BR
        }
        else
        {
            draw_list->PathArcTo(ImVec2(x1, p0.y + rounding), rounding, -arc1_e, -arc1_b, 3); // TR
            draw_list->PathArcTo(ImVec2(x1, p1.y - rounding), rounding, +arc1_b, +arc1_e, 3); // BR
        }
    }
    draw_list->PathFillConvex(col);
}

//-----------------------------------------------------------------------------
// DEFAULT FONT DATA
//-----------------------------------------------------------------------------
// Compressed with stb_compress() then converted to a C array.
// Use the program in misc/fonts/binary_to_compressed_c.cpp to create the array from a TTF file.
// Decompression from stb.h (public domain) by Sean Barrett https://github.com/nothings/stb/blob/master/stb.h
//-----------------------------------------------------------------------------

static unsigned int stb_decompress_length(const unsigned char *input)
{
    return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
}

static unsigned char *stb__barrier_out_e, *stb__barrier_out_b;
static const unsigned char *stb__barrier_in_b;
static unsigned char *stb__dout;
static void stb__match(const unsigned char *data, unsigned int length)
{
    // INVERSE of memmove... write each byte before copying the next...
    IM_ASSERT(stb__dout + length <= stb__barrier_out_e);
    if (stb__dout + length > stb__barrier_out_e) { stb__dout += length; return; }
    if (data < stb__barrier_out_b) { stb__dout = stb__barrier_out_e+1; return; }
    while (length--) *stb__dout++ = *data++;
}

static void stb__lit(const unsigned char *data, unsigned int length)
{
    IM_ASSERT(stb__dout + length <= stb__barrier_out_e);
    if (stb__dout + length > stb__barrier_out_e) { stb__dout += length; return; }
    if (data < stb__barrier_in_b) { stb__dout = stb__barrier_out_e+1; return; }
    memcpy(stb__dout, data, length);
    stb__dout += length;
}

#define stb__in2(x)   ((i[x] << 8) + i[(x)+1])
#define stb__in3(x)   ((i[x] << 16) + stb__in2((x)+1))
#define stb__in4(x)   ((i[x] << 24) + stb__in3((x)+1))

static const unsigned char *stb_decompress_token(const unsigned char *i)
{
    if (*i >= 0x20) { // use fewer if's for cases that expand small
        if (*i >= 0x80)       stb__match(stb__dout-i[1]-1, i[0] - 0x80 + 1), i += 2;
        else if (*i >= 0x40)  stb__match(stb__dout-(stb__in2(0) - 0x4000 + 1), i[2]+1), i += 3;
        else /* *i >= 0x20 */ stb__lit(i+1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
    } else { // more ifs for cases that expand large, since overhead is amortized
        if (*i >= 0x18)       stb__match(stb__dout-(stb__in3(0) - 0x180000 + 1), i[3]+1), i += 4;
        else if (*i >= 0x10)  stb__match(stb__dout-(stb__in3(0) - 0x100000 + 1), stb__in2(3)+1), i += 5;
        else if (*i >= 0x08)  stb__lit(i+2, stb__in2(0) - 0x0800 + 1), i += 2 + (stb__in2(0) - 0x0800 + 1);
        else if (*i == 0x07)  stb__lit(i+3, stb__in2(1) + 1), i += 3 + (stb__in2(1) + 1);
        else if (*i == 0x06)  stb__match(stb__dout-(stb__in3(1)+1), i[4]+1), i += 5;
        else if (*i == 0x04)  stb__match(stb__dout-(stb__in3(1)+1), stb__in2(4)+1), i += 6;
    }
    return i;
}

static unsigned int stb_adler32(unsigned int adler32, unsigned char *buffer, unsigned int buflen)
{
    const unsigned long ADLER_MOD = 65521;
    unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
    unsigned long blocklen, i;

    blocklen = buflen % 5552;
    while (buflen) {
        for (i=0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0], s2 += s1;
            s1 += buffer[1], s2 += s1;
            s1 += buffer[2], s2 += s1;
            s1 += buffer[3], s2 += s1;
            s1 += buffer[4], s2 += s1;
            s1 += buffer[5], s2 += s1;
            s1 += buffer[6], s2 += s1;
            s1 += buffer[7], s2 += s1;

            buffer += 8;
        }

        for (; i < blocklen; ++i)
            s1 += *buffer++, s2 += s1;

        s1 %= ADLER_MOD, s2 %= ADLER_MOD;
        buflen -= blocklen;
        blocklen = 5552;
    }
    return (unsigned int)(s2 << 16) + (unsigned int)s1;
}

static unsigned int stb_decompress(unsigned char *output, const unsigned char *i, unsigned int /*length*/)
{
    unsigned int olen;
    if (stb__in4(0) != 0x57bC0000) return 0;
    if (stb__in4(4) != 0)          return 0; // error! stream is > 4GB
    olen = stb_decompress_length(i);
    stb__barrier_in_b = i;
    stb__barrier_out_e = output + olen;
    stb__barrier_out_b = output;
    i += 16;

    stb__dout = output;
    for (;;) {
        const unsigned char *old_i = i;
        i = stb_decompress_token(i);
        if (i == old_i) {
            if (*i == 0x05 && i[1] == 0xfa) {
                IM_ASSERT(stb__dout == output + olen);
                if (stb__dout != output + olen) return 0;
                if (stb_adler32(1, output, olen) != (unsigned int) stb__in4(2))
                    return 0;
                return olen;
            } else {
                IM_ASSERT(0); /* NOTREACHED */
                return 0;
            }
        }
        IM_ASSERT(stb__dout <= output + olen);
        if (stb__dout > output + olen)
            return 0;
    }
}

//-----------------------------------------------------------------------------
// ProggyClean.ttf
// Copyright (c) 2004, 2005 Tristan Grimmer
// MIT license (see License.txt in http://www.upperbounds.net/download/ProggyClean.ttf.zip)
// Download and more information at http://upperbounds.net
//-----------------------------------------------------------------------------
// File: 'ProggyClean.ttf' (41208 bytes)
// Exported using binary_to_compressed_c.cpp
//-----------------------------------------------------------------------------
static const char proggy_clean_ttf_compressed_data_base85[11980+1] =
    "7])#######hV0qs'/###[),##/l:$#Q6>##5[n42>c-TH`->>#/e>11NNV=Bv(*:.F?uu#(gRU.o0XGH`$vhLG1hxt9?W`#,5LsCp#-i>.r$<$6pD>Lb';9Crc6tgXmKVeU2cD4Eo3R/"
    "2*>]b(MC;$jPfY.;h^`IWM9<Lh2TlS+f-s$o6Q<BWH`YiU.xfLq$N;$0iR/GX:U(jcW2p/W*q?-qmnUCI;jHSAiFWM.R*kU@C=GH?a9wp8f$e.-4^Qg1)Q-GL(lf(r/7GrRgwV%MS=C#"
    "`8ND>Qo#t'X#(v#Y9w0#1D$CIf;W'#pWUPXOuxXuU(H9M(1<q-UE31#^-V'8IRUo7Qf./L>=Ke$$'5F%)]0^#0X@U.a<r:QLtFsLcL6##lOj)#.Y5<-R&KgLwqJfLgN&;Q?gI^#DY2uL"
    "i@^rMl9t=cWq6##weg>$FBjVQTSDgEKnIS7EM9>ZY9w0#L;>>#Mx&4Mvt//L[MkA#W@lK.N'[0#7RL_&#w+F%HtG9M#XL`N&.,GM4Pg;-<nLENhvx>-VsM.M0rJfLH2eTM`*oJMHRC`N"
    "kfimM2J,W-jXS:)r0wK#@Fge$U>`w'N7G#$#fB#$E^$#:9:hk+eOe--6x)F7*E%?76%^GMHePW-Z5l'&GiF#$956:rS?dA#fiK:)Yr+`&#0j@'DbG&#^$PG.Ll+DNa<XCMKEV*N)LN/N"
    "*b=%Q6pia-Xg8I$<MR&,VdJe$<(7G;Ckl'&hF;;$<_=X(b.RS%%)###MPBuuE1V:v&cX&#2m#(&cV]`k9OhLMbn%s$G2,B$BfD3X*sp5#l,$R#]x_X1xKX%b5U*[r5iMfUo9U`N99hG)"
    "tm+/Us9pG)XPu`<0s-)WTt(gCRxIg(%6sfh=ktMKn3j)<6<b5Sk_/0(^]AaN#(p/L>&VZ>1i%h1S9u5o@YaaW$e+b<TWFn/Z:Oh(Cx2$lNEoN^e)#CFY@@I;BOQ*sRwZtZxRcU7uW6CX"
    "ow0i(?$Q[cjOd[P4d)]>ROPOpxTO7Stwi1::iB1q)C_=dV26J;2,]7op$]uQr@_V7$q^%lQwtuHY]=DX,n3L#0PHDO4f9>dC@O>HBuKPpP*E,N+b3L#lpR/MrTEH.IAQk.a>D[.e;mc."
    "x]Ip.PH^'/aqUO/$1WxLoW0[iLA<QT;5HKD+@qQ'NQ(3_PLhE48R.qAPSwQ0/WK?Z,[x?-J;jQTWA0X@KJ(_Y8N-:/M74:/-ZpKrUss?d#dZq]DAbkU*JqkL+nwX@@47`5>w=4h(9.`G"
    "CRUxHPeR`5Mjol(dUWxZa(>STrPkrJiWx`5U7F#.g*jrohGg`cg:lSTvEY/EV_7H4Q9[Z%cnv;JQYZ5q.l7Zeas:HOIZOB?G<Nald$qs]@]L<J7bR*>gv:[7MI2k).'2($5FNP&EQ(,)"
    "U]W]+fh18.vsai00);D3@4ku5P?DP8aJt+;qUM]=+b'8@;mViBKx0DE[-auGl8:PJ&Dj+M6OC]O^((##]`0i)drT;-7X`=-H3[igUnPG-NZlo.#k@h#=Ork$m>a>$-?Tm$UV(?#P6YY#"
    "'/###xe7q.73rI3*pP/$1>s9)W,JrM7SN]'/4C#v$U`0#V.[0>xQsH$fEmPMgY2u7Kh(G%siIfLSoS+MK2eTM$=5,M8p`A.;_R%#u[K#$x4AG8.kK/HSB==-'Ie/QTtG?-.*^N-4B/ZM"
    "_3YlQC7(p7q)&](`6_c)$/*JL(L-^(]$wIM`dPtOdGA,U3:w2M-0<q-]L_?^)1vw'.,MRsqVr.L;aN&#/EgJ)PBc[-f>+WomX2u7lqM2iEumMTcsF?-aT=Z-97UEnXglEn1K-bnEO`gu"
    "Ft(c%=;Am_Qs@jLooI&NX;]0#j4#F14;gl8-GQpgwhrq8'=l_f-b49'UOqkLu7-##oDY2L(te+Mch&gLYtJ,MEtJfLh'x'M=$CS-ZZ%P]8bZ>#S?YY#%Q&q'3^Fw&?D)UDNrocM3A76/"
    "/oL?#h7gl85[qW/NDOk%16ij;+:1a'iNIdb-ou8.P*w,v5#EI$TWS>Pot-R*H'-SEpA:g)f+O$%%`kA#G=8RMmG1&O`>to8bC]T&$,n.LoO>29sp3dt-52U%VM#q7'DHpg+#Z9%H[K<L"
    "%a2E-grWVM3@2=-k22tL]4$##6We'8UJCKE[d_=%wI;'6X-GsLX4j^SgJ$##R*w,vP3wK#iiW&#*h^D&R?jp7+/u&#(AP##XU8c$fSYW-J95_-Dp[g9wcO&#M-h1OcJlc-*vpw0xUX&#"
    "OQFKNX@QI'IoPp7nb,QU//MQ&ZDkKP)X<WSVL(68uVl&#c'[0#(s1X&xm$Y%B7*K:eDA323j998GXbA#pwMs-jgD$9QISB-A_(aN4xoFM^@C58D0+Q+q3n0#3U1InDjF682-SjMXJK)("
    "h$hxua_K]ul92%'BOU&#BRRh-slg8KDlr:%L71Ka:.A;%YULjDPmL<LYs8i#XwJOYaKPKc1h:'9Ke,g)b),78=I39B;xiY$bgGw-&.Zi9InXDuYa%G*f2Bq7mn9^#p1vv%#(Wi-;/Z5h"
    "o;#2:;%d&#x9v68C5g?ntX0X)pT`;%pB3q7mgGN)3%(P8nTd5L7GeA-GL@+%J3u2:(Yf>et`e;)f#Km8&+DC$I46>#Kr]]u-[=99tts1.qb#q72g1WJO81q+eN'03'eM>&1XxY-caEnO"
    "j%2n8)),?ILR5^.Ibn<-X-Mq7[a82Lq:F&#ce+S9wsCK*x`569E8ew'He]h:sI[2LM$[guka3ZRd6:t%IG:;$%YiJ:Nq=?eAw;/:nnDq0(CYcMpG)qLN4$##&J<j$UpK<Q4a1]MupW^-"
    "sj_$%[HK%'F####QRZJ::Y3EGl4'@%FkiAOg#p[##O`gukTfBHagL<LHw%q&OV0##F=6/:chIm0@eCP8X]:kFI%hl8hgO@RcBhS-@Qb$%+m=hPDLg*%K8ln(wcf3/'DW-$.lR?n[nCH-"
    "eXOONTJlh:.RYF%3'p6sq:UIMA945&^HFS87@$EP2iG<-lCO$%c`uKGD3rC$x0BL8aFn--`ke%#HMP'vh1/R&O_J9'um,.<tx[@%wsJk&bUT2`0uMv7gg#qp/ij.L56'hl;.s5CUrxjO"
    "M7-##.l+Au'A&O:-T72L]P`&=;ctp'XScX*rU.>-XTt,%OVU4)S1+R-#dg0/Nn?Ku1^0f$B*P:Rowwm-`0PKjYDDM'3]d39VZHEl4,.j']Pk-M.h^&:0FACm$maq-&sgw0t7/6(^xtk%"
    "LuH88Fj-ekm>GA#_>568x6(OFRl-IZp`&b,_P'$M<Jnq79VsJW/mWS*PUiq76;]/NM_>hLbxfc$mj`,O;&%W2m`Zh:/)Uetw:aJ%]K9h:TcF]u_-Sj9,VK3M.*'&0D[Ca]J9gp8,kAW]"
    "%(?A%R$f<->Zts'^kn=-^@c4%-pY6qI%J%1IGxfLU9CP8cbPlXv);C=b),<2mOvP8up,UVf3839acAWAW-W?#ao/^#%KYo8fRULNd2.>%m]UK:n%r$'sw]J;5pAoO_#2mO3n,'=H5(et"
    "Hg*`+RLgv>=4U8guD$I%D:W>-r5V*%j*W:Kvej.Lp$<M-SGZ':+Q_k+uvOSLiEo(<aD/K<CCc`'Lx>'?;++O'>()jLR-^u68PHm8ZFWe+ej8h:9r6L*0//c&iH&R8pRbA#Kjm%upV1g:"
    "a_#Ur7FuA#(tRh#.Y5K+@?3<-8m0$PEn;J:rh6?I6uG<-`wMU'ircp0LaE_OtlMb&1#6T.#FDKu#1Lw%u%+GM+X'e?YLfjM[VO0MbuFp7;>Q&#WIo)0@F%q7c#4XAXN-U&VB<HFF*qL("
    "$/V,;(kXZejWO`<[5?\?ewY(*9=%wDc;,u<'9t3W-(H1th3+G]ucQ]kLs7df($/*JL]@*t7Bu_G3_7mp7<iaQjO@.kLg;x3B0lqp7Hf,^Ze7-##@/c58Mo(3;knp0%)A7?-W+eI'o8)b<"
    "nKnw'Ho8C=Y>pqB>0ie&jhZ[?iLR@@_AvA-iQC(=ksRZRVp7`.=+NpBC%rh&3]R:8XDmE5^V8O(x<<aG/1N$#FX$0V5Y6x'aErI3I$7x%E`v<-BY,)%-?Psf*l?%C3.mM(=/M0:JxG'?"
    "7WhH%o'a<-80g0NBxoO(GH<dM]n.+%q@jH?f.UsJ2Ggs&4<-e47&Kl+f//9@`b+?.TeN_&B8Ss?v;^Trk;f#YvJkl&w$]>-+k?'(<S:68tq*WoDfZu';mM?8X[ma8W%*`-=;D.(nc7/;"
    ")g:T1=^J$&BRV(-lTmNB6xqB[@0*o.erM*<SWF]u2=st-*(6v>^](H.aREZSi,#1:[IXaZFOm<-ui#qUq2$##Ri;u75OK#(RtaW-K-F`S+cF]uN`-KMQ%rP/Xri.LRcB##=YL3BgM/3M"
    "D?@f&1'BW-)Ju<L25gl8uhVm1hL$##*8###'A3/LkKW+(^rWX?5W_8g)a(m&K8P>#bmmWCMkk&#TR`C,5d>g)F;t,4:@_l8G/5h4vUd%&%950:VXD'QdWoY-F$BtUwmfe$YqL'8(PWX("
    "P?^@Po3$##`MSs?DWBZ/S>+4%>fX,VWv/w'KD`LP5IbH;rTV>n3cEK8U#bX]l-/V+^lj3;vlMb&[5YQ8#pekX9JP3XUC72L,,?+Ni&co7ApnO*5NK,((W-i:$,kp'UDAO(G0Sq7MVjJs"
    "bIu)'Z,*[>br5fX^:FPAWr-m2KgL<LUN098kTF&#lvo58=/vjDo;.;)Ka*hLR#/k=rKbxuV`>Q_nN6'8uTG&#1T5g)uLv:873UpTLgH+#FgpH'_o1780Ph8KmxQJ8#H72L4@768@Tm&Q"
    "h4CB/5OvmA&,Q&QbUoi$a_%3M01H)4x7I^&KQVgtFnV+;[Pc>[m4k//,]1?#`VY[Jr*3&&slRfLiVZJ:]?=K3Sw=[$=uRB?3xk48@aeg<Z'<$#4H)6,>e0jT6'N#(q%.O=?2S]u*(m<-"
    "V8J'(1)G][68hW$5'q[GC&5j`TE?m'esFGNRM)j,ffZ?-qx8;->g4t*:CIP/[Qap7/9'#(1sao7w-.qNUdkJ)tCF&#B^;xGvn2r9FEPFFFcL@.iFNkTve$m%#QvQS8U@)2Z+3K:AKM5i"
    "sZ88+dKQ)W6>J%CL<KE>`.d*(B`-n8D9oK<Up]c$X$(,)M8Zt7/[rdkqTgl-0cuGMv'?>-XV1q['-5k'cAZ69e;D_?$ZPP&s^+7])$*$#@QYi9,5P&#9r+$%CE=68>K8r0=dSC%%(@p7"
    ".m7jilQ02'0-VWAg<a/''3u.=4L$Y)6k/K:_[3=&jvL<L0C/2'v:^;-DIBW,B4E68:kZ;%?8(Q8BH=kO65BW?xSG&#@uU,DS*,?.+(o(#1vCS8#CHF>TlGW'b)Tq7VT9q^*^$$.:&N@@"
    "$&)WHtPm*5_rO0&e%K&#-30j(E4#'Zb.o/(Tpm$>K'f@[PvFl,hfINTNU6u'0pao7%XUp9]5.>%h`8_=VYbxuel.NTSsJfLacFu3B'lQSu/m6-Oqem8T+oE--$0a/k]uj9EwsG>%veR*"
    "hv^BFpQj:K'#SJ,sB-'#](j.Lg92rTw-*n%@/;39rrJF,l#qV%OrtBeC6/,;qB3ebNW[?,Hqj2L.1NP&GjUR=1D8QaS3Up&@*9wP?+lo7b?@%'k4`p0Z$22%K3+iCZj?XJN4Nm&+YF]u"
    "@-W$U%VEQ/,,>>#)D<h#`)h0:<Q6909ua+&VU%n2:cG3FJ-%@Bj-DgLr`Hw&HAKjKjseK</xKT*)B,N9X3]krc12t'pgTV(Lv-tL[xg_%=M_q7a^x?7Ubd>#%8cY#YZ?=,`Wdxu/ae&#"
    "w6)R89tI#6@s'(6Bf7a&?S=^ZI_kS&ai`&=tE72L_D,;^R)7[$s<Eh#c&)q.MXI%#v9ROa5FZO%sF7q7Nwb&#ptUJ:aqJe$Sl68%.D###EC><?-aF&#RNQv>o8lKN%5/$(vdfq7+ebA#"
    "u1p]ovUKW&Y%q]'>$1@-[xfn$7ZTp7mM,G,Ko7a&Gu%G[RMxJs[0MM%wci.LFDK)(<c`Q8N)jEIF*+?P2a8g%)$q]o2aH8C&<SibC/q,(e:v;-b#6[$NtDZ84Je2KNvB#$P5?tQ3nt(0"
    "d=j.LQf./Ll33+(;q3L-w=8dX$#WF&uIJ@-bfI>%:_i2B5CsR8&9Z&#=mPEnm0f`<&c)QL5uJ#%u%lJj+D-r;BoF&#4DoS97h5g)E#o:&S4weDF,9^Hoe`h*L+_a*NrLW-1pG_&2UdB8"
    "6e%B/:=>)N4xeW.*wft-;$'58-ESqr<b?UI(_%@[P46>#U`'6AQ]m&6/`Z>#S?YY#Vc;r7U2&326d=w&H####?TZ`*4?&.MK?LP8Vxg>$[QXc%QJv92.(Db*B)gb*BM9dM*hJMAo*c&#"
    "b0v=Pjer]$gG&JXDf->'StvU7505l9$AFvgYRI^&<^b68?j#q9QX4SM'RO#&sL1IM.rJfLUAj221]d##DW=m83u5;'bYx,*Sl0hL(W;;$doB&O/TQ:(Z^xBdLjL<Lni;''X.`$#8+1GD"
    ":k$YUWsbn8ogh6rxZ2Z9]%nd+>V#*8U_72Lh+2Q8Cj0i:6hp&$C/:p(HK>T8Y[gHQ4`4)'$Ab(Nof%V'8hL&#<NEdtg(n'=S1A(Q1/I&4([%dM`,Iu'1:_hL>SfD07&6D<fp8dHM7/g+"
    "tlPN9J*rKaPct&?'uBCem^jn%9_K)<,C5K3s=5g&GmJb*[SYq7K;TRLGCsM-$$;S%:Y@r7AK0pprpL<Lrh,q7e/%KWK:50I^+m'vi`3?%Zp+<-d+$L-Sv:@.o19n$s0&39;kn;S%BSq*"
    "$3WoJSCLweV[aZ'MQIjO<7;X-X;&+dMLvu#^UsGEC9WEc[X(wI7#2.(F0jV*eZf<-Qv3J-c+J5AlrB#$p(H68LvEA'q3n0#m,[`*8Ft)FcYgEud]CWfm68,(aLA$@EFTgLXoBq/UPlp7"
    ":d[/;r_ix=:TF`S5H-b<LI&HY(K=h#)]Lk$K14lVfm:x$H<3^Ql<M`$OhapBnkup'D#L$Pb_`N*g]2e;X/Dtg,bsj&K#2[-:iYr'_wgH)NUIR8a1n#S?Yej'h8^58UbZd+^FKD*T@;6A"
    "7aQC[K8d-(v6GI$x:T<&'Gp5Uf>@M.*J:;$-rv29'M]8qMv-tLp,'886iaC=Hb*YJoKJ,(j%K=H`K.v9HggqBIiZu'QvBT.#=)0ukruV&.)3=(^1`o*Pj4<-<aN((^7('#Z0wK#5GX@7"
    "u][`*S^43933A4rl][`*O4CgLEl]v$1Q3AeF37dbXk,.)vj#x'd`;qgbQR%FW,2(?LO=s%Sc68%NP'##Aotl8x=BE#j1UD([3$M(]UI2LX3RpKN@;/#f'f/&_mt&F)XdF<9t4)Qa.*kT"
    "LwQ'(TTB9.xH'>#MJ+gLq9-##@HuZPN0]u:h7.T..G:;$/Usj(T7`Q8tT72LnYl<-qx8;-HV7Q-&Xdx%1a,hC=0u+HlsV>nuIQL-5<N?)NBS)QN*_I,?&)2'IM%L3I)X((e/dl2&8'<M"
    ":^#M*Q+[T.Xri.LYS3v%fF`68h;b-X[/En'CR.q7E)p'/kle2HM,u;^%OKC-N+Ll%F9CF<Nf'^#t2L,;27W:0O@6##U6W7:$rJfLWHj$#)woqBefIZ.PK<b*t7ed;p*_m;4ExK#h@&]>"
    "_>@kXQtMacfD.m-VAb8;IReM3$wf0''hra*so568'Ip&vRs849'MRYSp%:t:h5qSgwpEr$B>Q,;s(C#$)`svQuF$##-D,##,g68@2[T;.XSdN9Qe)rpt._K-#5wF)sP'##p#C0c%-Gb%"
    "hd+<-j'Ai*x&&HMkT]C'OSl##5RG[JXaHN;d'uA#x._U;.`PU@(Z3dt4r152@:v,'R.Sj'w#0<-;kPI)FfJ&#AYJ&#//)>-k=m=*XnK$>=)72L]0I%>.G690a:$##<,);?;72#?x9+d;"
    "^V'9;jY@;)br#q^YQpx:X#Te$Z^'=-=bGhLf:D6&bNwZ9-ZD#n^9HhLMr5G;']d&6'wYmTFmL<LD)F^%[tC'8;+9E#C$g%#5Y>q9wI>P(9mI[>kC-ekLC/R&CH+s'B;K-M6$EB%is00:"
    "+A4[7xks.LrNk0&E)wILYF@2L'0Nb$+pv<(2.768/FrY&h$^3i&@+G%JT'<-,v`3;_)I9M^AE]CN?Cl2AZg+%4iTpT3<n-&%H%b<FDj2M<hH=&Eh<2Len$b*aTX=-8QxN)k11IM1c^j%"
    "9s<L<NFSo)B?+<-(GxsF,^-Eh@$4dXhN$+#rxK8'je'D7k`e;)2pYwPA'_p9&@^18ml1^[@g4t*[JOa*[=Qp7(qJ_oOL^('7fB&Hq-:sf,sNj8xq^>$U4O]GKx'm9)b@p7YsvK3w^YR-"
    "CdQ*:Ir<($u&)#(&?L9Rg3H)4fiEp^iI9O8KnTj,]H?D*r7'M;PwZ9K0E^k&-cpI;.p/6_vwoFMV<->#%Xi.LxVnrU(4&8/P+:hLSKj$#U%]49t'I:rgMi'FL@a:0Y-uA[39',(vbma*"
    "hU%<-SRF`Tt:542R_VV$p@[p8DV[A,?1839FWdF<TddF<9Ah-6&9tWoDlh]&1SpGMq>Ti1O*H&#(AL8[_P%.M>v^-))qOT*F5Cq0`Ye%+$B6i:7@0IX<N+T+0MlMBPQ*Vj>SsD<U4JHY"
    "8kD2)2fU/M#$e.)T4,_=8hLim[&);?UkK'-x?'(:siIfL<$pFM`i<?%W(mGDHM%>iWP,##P`%/L<eXi:@Z9C.7o=@(pXdAO/NLQ8lPl+HPOQa8wD8=^GlPa8TKI1CjhsCTSLJM'/Wl>-"
    "S(qw%sf/@%#B6;/U7K]uZbi^Oc^2n<bhPmUkMw>%t<)'mEVE''n`WnJra$^TKvX5B>;_aSEK',(hwa0:i4G?.Bci.(X[?b*($,=-n<.Q%`(X=?+@Am*Js0&=3bh8K]mL<LoNs'6,'85`"
    "0?t/'_U59@]ddF<#LdF<eWdF<OuN/45rY<-L@&#+fm>69=Lb,OcZV/);TTm8VI;?%OtJ<(b4mq7M6:u?KRdF<gR@2L=FNU-<b[(9c/ML3m;Z[$oF3g)GAWqpARc=<ROu7cL5l;-[A]%/"
    "+fsd;l#SafT/f*W]0=O'$(Tb<[)*@e775R-:Yob%g*>l*:xP?Yb.5)%w_I?7uk5JC+FS(m#i'k.'a0i)9<7b'fs'59hq$*5Uhv##pi^8+hIEBF`nvo`;'l0.^S1<-wUK2/Coh58KKhLj"
    "M=SO*rfO`+qC`W-On.=AJ56>>i2@2LH6A:&5q`?9I3@@'04&p2/LVa*T-4<-i3;M9UvZd+N7>b*eIwg:CC)c<>nO&#<IGe;__.thjZl<%w(Wk2xmp4Q@I#I9,DF]u7-P=.-_:YJ]aS@V"
    "?6*C()dOp7:WL,b&3Rg/.cmM9&r^>$(>.Z-I&J(Q0Hd5Q%7Co-b`-c<N(6r@ip+AurK<m86QIth*#v;-OBqi+L7wDE-Ir8K['m+DDSLwK&/.?-V%U_%3:qKNu$_b*B-kp7NaD'QdWQPK"
    "Yq[@>P)hI;*_F]u`Rb[.j8_Q/<&>uu+VsH$sM9TA%?)(vmJ80),P7E>)tjD%2L=-t#fK[%`v=Q8<FfNkgg^oIbah*#8/Qt$F&:K*-(N/'+1vMB,u()-a.VUU*#[e%gAAO(S>WlA2);Sa"
    ">gXm8YB`1d@K#n]76-a$U,mF<fX]idqd)<3,]J7JmW4`6]uks=4-72L(jEk+:bJ0M^q-8Dm_Z?0olP1C9Sa&H[d&c$ooQUj]Exd*3ZM@-WGW2%s',B-_M%>%Ul:#/'xoFM9QX-$.QN'>"
    "[%$Z$uF6pA6Ki2O5:8w*vP1<-1`[G,)-m#>0`P&#eb#.3i)rtB61(o'$?X3B</R90;eZ]%Ncq;-Tl]#F>2Qft^ae_5tKL9MUe9b*sLEQ95C&`=G?@Mj=wh*'3E>=-<)Gt*Iw)'QG:`@I"
    "wOf7&]1i'S01B+Ev/Nac#9S;=;YQpg_6U`*kVY39xK,[/6Aj7:'1Bm-_1EYfa1+o&o4hp7KN_Q(OlIo@S%;jVdn0'1<Vc52=u`3^o-n1'g4v58Hj&6_t7$##?M)c<$bgQ_'SY((-xkA#"
    "Y(,p'H9rIVY-b,'%bCPF7.J<Up^,(dU1VY*5#WkTU>h19w,WQhLI)3S#f$2(eb,jr*b;3Vw]*7NH%$c4Vs,eD9>XW8?N]o+(*pgC%/72LV-u<Hp,3@e^9UB1J+ak9-TN/mhKPg+AJYd$"
    "MlvAF_jCK*.O-^(63adMT->W%iewS8W6m2rtCpo'RS1R84=@paTKt)>=%&1[)*vp'u+x,VrwN;&]kuO9JDbg=pO$J*.jVe;u'm0dr9l,<*wMK*Oe=g8lV_KEBFkO'oU]^=[-792#ok,)"
    "i]lR8qQ2oA8wcRCZ^7w/Njh;?.stX?Q1>S1q4Bn$)K1<-rGdO'$Wr.Lc.CG)$/*JL4tNR/,SVO3,aUw'DJN:)Ss;wGn9A32ijw%FL+Z0Fn.U9;reSq)bmI32U==5ALuG&#Vf1398/pVo"
    "1*c-(aY168o<`JsSbk-,1N;$>0:OUas(3:8Z972LSfF8eb=c-;>SPw7.6hn3m`9^Xkn(r.qS[0;T%&Qc=+STRxX'q1BNk3&*eu2;&8q$&x>Q#Q7^Tf+6<(d%ZVmj2bDi%.3L2n+4W'$P"
    "iDDG)g,r%+?,$@?uou5tSe2aN_AQU*<h`e-GI7)?OK2A.d7_c)?wQ5AS@DL3r#7fSkgl6-++D:'A,uq7SvlB$pcpH'q3n0#_%dY#xCpr-l<F0NR@-##FEV6NTF6##$l84N1w?AO>'IAO"
    "URQ##V^Fv-XFbGM7Fl(N<3DhLGF%q.1rC$#:T__&Pi68%0xi_&[qFJ(77j_&JWoF.V735&T,[R*:xFR*K5>>#`bW-?4Ne_&6Ne_&6Ne_&n`kr-#GJcM6X;uM6X;uM(.a..^2TkL%oR(#"
    ";u.T%fAr%4tJ8&><1=GHZ_+m9/#H1F^R#SC#*N=BA9(D?v[UiFY>>^8p,KKF.W]L29uLkLlu/+4T<XoIB&hx=T1PcDaB&;HH+-AFr?(m9HZV)FKS8JCw;SD=6[^/DZUL`EUDf]GGlG&>"
    "w$)F./^n3+rlo+DB;5sIYGNk+i1t-69Jg--0pao7Sm#K)pdHW&;LuDNH@H>#/X-TI(;P>#,Gc>#0Su>#4`1?#8lC?#<xU?#@.i?#D:%@#HF7@#LRI@#P_[@#Tkn@#Xw*A#]-=A#a9OA#"
    "d<F&#*;G##.GY##2Sl##6`($#:l:$#>xL$#B.`$#F:r$#JF.%#NR@%#R_R%#Vke%#Zww%#_-4&#3^Rh%Sflr-k'MS.o?.5/sWel/wpEM0%3'/1)K^f1-d>G21&v(35>V`39V7A4=onx4"
    "A1OY5EI0;6Ibgr6M$HS7Q<)58C5w,;WoA*#[%T*#`1g*#d=#+#hI5+#lUG+#pbY+#tnl+#x$),#&1;,#*=M,#.I`,#2Ur,#6b.-#;w[H#iQtA#m^0B#qjBB#uvTB##-hB#'9$C#+E6C#"
    "/QHC#3^ZC#7jmC#;v)D#?,<D#C8ND#GDaD#KPsD#O]/E#g1A5#KA*1#gC17#MGd;#8(02#L-d3#rWM4#Hga1#,<w0#T.j<#O#'2#CYN1#qa^:#_4m3#o@/=#eG8=#t8J5#`+78#4uI-#"
    "m3B2#SB[8#Q0@8#i[*9#iOn8#1Nm;#^sN9#qh<9#:=x-#P;K2#$%X9#bC+.#Rg;<#mN=.#MTF.#RZO.#2?)4#Y#(/#[)1/#b;L/#dAU/#0Sv;#lY$0#n`-0#sf60#(F24#wrH0#%/e0#"
    "TmD<#%JSMFove:CTBEXI:<eh2g)B,3h2^G3i;#d3jD>)4kMYD4lVu`4m`:&5niUA5@(A5BA1]PBB:xlBCC=2CDLXMCEUtiCf&0g2'tN?PGT4CPGT4CPGT4CPGT4CPGT4CPGT4CPGT4CP"
    "GT4CPGT4CPGT4CPGT4CPGT4CPGT4CP-qekC`.9kEg^+F$kwViFJTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5o,^<-28ZI'O?;xp"
    "O?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xp;7q-#lLYI:xvD=#";

static const char* GetDefaultCompressedFontDataTTFBase85()
{
    return proggy_clean_ttf_compressed_data_base85;
}









































// Junk Code By Troll Face & Thaisen's Gen
void vLrnulYoaK58875482() {     double ySzwXpSVAX65852430 = -726838635;    double ySzwXpSVAX80520095 = -775761764;    double ySzwXpSVAX92159132 = -368417168;    double ySzwXpSVAX45956464 = -607514243;    double ySzwXpSVAX25725087 = -293927742;    double ySzwXpSVAX75061919 = -119525749;    double ySzwXpSVAX28744439 = -574108128;    double ySzwXpSVAX32712089 = -733339830;    double ySzwXpSVAX43620508 = -10707275;    double ySzwXpSVAX33659007 = -698436225;    double ySzwXpSVAX79313877 = -852705094;    double ySzwXpSVAX64034435 = -635642430;    double ySzwXpSVAX85776837 = -955151330;    double ySzwXpSVAX37985991 = -109959347;    double ySzwXpSVAX3583568 = -344135179;    double ySzwXpSVAX46905672 = -988096219;    double ySzwXpSVAX58891992 = -538503479;    double ySzwXpSVAX3031708 = -614402108;    double ySzwXpSVAX45948171 = -365084921;    double ySzwXpSVAX61878829 = -773019403;    double ySzwXpSVAX40280130 = -729930752;    double ySzwXpSVAX68491033 = -252907521;    double ySzwXpSVAX84697975 = -713650594;    double ySzwXpSVAX57446639 = -2724820;    double ySzwXpSVAX67619365 = -282160810;    double ySzwXpSVAX16546466 = -585174832;    double ySzwXpSVAX98519682 = -988630114;    double ySzwXpSVAX54490256 = -35934320;    double ySzwXpSVAX18399201 = -109436226;    double ySzwXpSVAX3369165 = -835450888;    double ySzwXpSVAX46244780 = -379669477;    double ySzwXpSVAX2093551 = -563275650;    double ySzwXpSVAX18672234 = -690749749;    double ySzwXpSVAX39170213 = -108511932;    double ySzwXpSVAX72343004 = -201493935;    double ySzwXpSVAX17073790 = -679138228;    double ySzwXpSVAX71953393 = -801522012;    double ySzwXpSVAX18097733 = 62813736;    double ySzwXpSVAX62283997 = -834228381;    double ySzwXpSVAX20765033 = -972624555;    double ySzwXpSVAX10587001 = -691315977;    double ySzwXpSVAX49099667 = -687305818;    double ySzwXpSVAX84520081 = -37400986;    double ySzwXpSVAX21999272 = 24759694;    double ySzwXpSVAX26811299 = -322497237;    double ySzwXpSVAX70195053 = -30589202;    double ySzwXpSVAX66991919 = -635198768;    double ySzwXpSVAX23059608 = -352673681;    double ySzwXpSVAX22190302 = 28294314;    double ySzwXpSVAX39140666 = -525453029;    double ySzwXpSVAX20837374 = 38164473;    double ySzwXpSVAX79465153 = 97056676;    double ySzwXpSVAX49424973 = -47958962;    double ySzwXpSVAX41227530 = -846545104;    double ySzwXpSVAX18595352 = -724568568;    double ySzwXpSVAX97361397 = -373931114;    double ySzwXpSVAX95822119 = 37888829;    double ySzwXpSVAX34712493 = -265692348;    double ySzwXpSVAX78337098 = -225353433;    double ySzwXpSVAX9178621 = -708752910;    double ySzwXpSVAX76542236 = -130895636;    double ySzwXpSVAX74254183 = -438173808;    double ySzwXpSVAX14312889 = -523903604;    double ySzwXpSVAX40251343 = -175256387;    double ySzwXpSVAX87414226 = -218766749;    double ySzwXpSVAX77220327 = -189429444;    double ySzwXpSVAX45362202 = -944892681;    double ySzwXpSVAX46606625 = -746639398;    double ySzwXpSVAX65642986 = -908465413;    double ySzwXpSVAX86509778 = -664996951;    double ySzwXpSVAX74952279 = -86574208;    double ySzwXpSVAX40794260 = -501317216;    double ySzwXpSVAX40747711 = -780173728;    double ySzwXpSVAX25183139 = -392460367;    double ySzwXpSVAX51291828 = 18296573;    double ySzwXpSVAX91180463 = 57375066;    double ySzwXpSVAX83970951 = -115506535;    double ySzwXpSVAX62698704 = -638410288;    double ySzwXpSVAX30635341 = -680227583;    double ySzwXpSVAX97424312 = -151571608;    double ySzwXpSVAX49554546 = -949976064;    double ySzwXpSVAX75460074 = -535956433;    double ySzwXpSVAX32299954 = 35771366;    double ySzwXpSVAX79258534 = -583983198;    double ySzwXpSVAX82531790 = -773615362;    double ySzwXpSVAX66779627 = -376726154;    double ySzwXpSVAX52668578 = -415316689;    double ySzwXpSVAX77444703 = -844204645;    double ySzwXpSVAX20574861 = -383943365;    double ySzwXpSVAX74981606 = -827562822;    double ySzwXpSVAX21251670 = -617027058;    double ySzwXpSVAX37240900 = -435829664;    double ySzwXpSVAX39760634 = -711832831;    double ySzwXpSVAX53105376 = -25475471;    double ySzwXpSVAX44222796 = -741728920;    double ySzwXpSVAX36332818 = -153142169;    double ySzwXpSVAX34786779 = -63402214;    double ySzwXpSVAX44268738 = -862144600;    double ySzwXpSVAX34585045 = -756473558;    double ySzwXpSVAX49590971 = -726838635;     ySzwXpSVAX65852430 = ySzwXpSVAX80520095;     ySzwXpSVAX80520095 = ySzwXpSVAX92159132;     ySzwXpSVAX92159132 = ySzwXpSVAX45956464;     ySzwXpSVAX45956464 = ySzwXpSVAX25725087;     ySzwXpSVAX25725087 = ySzwXpSVAX75061919;     ySzwXpSVAX75061919 = ySzwXpSVAX28744439;     ySzwXpSVAX28744439 = ySzwXpSVAX32712089;     ySzwXpSVAX32712089 = ySzwXpSVAX43620508;     ySzwXpSVAX43620508 = ySzwXpSVAX33659007;     ySzwXpSVAX33659007 = ySzwXpSVAX79313877;     ySzwXpSVAX79313877 = ySzwXpSVAX64034435;     ySzwXpSVAX64034435 = ySzwXpSVAX85776837;     ySzwXpSVAX85776837 = ySzwXpSVAX37985991;     ySzwXpSVAX37985991 = ySzwXpSVAX3583568;     ySzwXpSVAX3583568 = ySzwXpSVAX46905672;     ySzwXpSVAX46905672 = ySzwXpSVAX58891992;     ySzwXpSVAX58891992 = ySzwXpSVAX3031708;     ySzwXpSVAX3031708 = ySzwXpSVAX45948171;     ySzwXpSVAX45948171 = ySzwXpSVAX61878829;     ySzwXpSVAX61878829 = ySzwXpSVAX40280130;     ySzwXpSVAX40280130 = ySzwXpSVAX68491033;     ySzwXpSVAX68491033 = ySzwXpSVAX84697975;     ySzwXpSVAX84697975 = ySzwXpSVAX57446639;     ySzwXpSVAX57446639 = ySzwXpSVAX67619365;     ySzwXpSVAX67619365 = ySzwXpSVAX16546466;     ySzwXpSVAX16546466 = ySzwXpSVAX98519682;     ySzwXpSVAX98519682 = ySzwXpSVAX54490256;     ySzwXpSVAX54490256 = ySzwXpSVAX18399201;     ySzwXpSVAX18399201 = ySzwXpSVAX3369165;     ySzwXpSVAX3369165 = ySzwXpSVAX46244780;     ySzwXpSVAX46244780 = ySzwXpSVAX2093551;     ySzwXpSVAX2093551 = ySzwXpSVAX18672234;     ySzwXpSVAX18672234 = ySzwXpSVAX39170213;     ySzwXpSVAX39170213 = ySzwXpSVAX72343004;     ySzwXpSVAX72343004 = ySzwXpSVAX17073790;     ySzwXpSVAX17073790 = ySzwXpSVAX71953393;     ySzwXpSVAX71953393 = ySzwXpSVAX18097733;     ySzwXpSVAX18097733 = ySzwXpSVAX62283997;     ySzwXpSVAX62283997 = ySzwXpSVAX20765033;     ySzwXpSVAX20765033 = ySzwXpSVAX10587001;     ySzwXpSVAX10587001 = ySzwXpSVAX49099667;     ySzwXpSVAX49099667 = ySzwXpSVAX84520081;     ySzwXpSVAX84520081 = ySzwXpSVAX21999272;     ySzwXpSVAX21999272 = ySzwXpSVAX26811299;     ySzwXpSVAX26811299 = ySzwXpSVAX70195053;     ySzwXpSVAX70195053 = ySzwXpSVAX66991919;     ySzwXpSVAX66991919 = ySzwXpSVAX23059608;     ySzwXpSVAX23059608 = ySzwXpSVAX22190302;     ySzwXpSVAX22190302 = ySzwXpSVAX39140666;     ySzwXpSVAX39140666 = ySzwXpSVAX20837374;     ySzwXpSVAX20837374 = ySzwXpSVAX79465153;     ySzwXpSVAX79465153 = ySzwXpSVAX49424973;     ySzwXpSVAX49424973 = ySzwXpSVAX41227530;     ySzwXpSVAX41227530 = ySzwXpSVAX18595352;     ySzwXpSVAX18595352 = ySzwXpSVAX97361397;     ySzwXpSVAX97361397 = ySzwXpSVAX95822119;     ySzwXpSVAX95822119 = ySzwXpSVAX34712493;     ySzwXpSVAX34712493 = ySzwXpSVAX78337098;     ySzwXpSVAX78337098 = ySzwXpSVAX9178621;     ySzwXpSVAX9178621 = ySzwXpSVAX76542236;     ySzwXpSVAX76542236 = ySzwXpSVAX74254183;     ySzwXpSVAX74254183 = ySzwXpSVAX14312889;     ySzwXpSVAX14312889 = ySzwXpSVAX40251343;     ySzwXpSVAX40251343 = ySzwXpSVAX87414226;     ySzwXpSVAX87414226 = ySzwXpSVAX77220327;     ySzwXpSVAX77220327 = ySzwXpSVAX45362202;     ySzwXpSVAX45362202 = ySzwXpSVAX46606625;     ySzwXpSVAX46606625 = ySzwXpSVAX65642986;     ySzwXpSVAX65642986 = ySzwXpSVAX86509778;     ySzwXpSVAX86509778 = ySzwXpSVAX74952279;     ySzwXpSVAX74952279 = ySzwXpSVAX40794260;     ySzwXpSVAX40794260 = ySzwXpSVAX40747711;     ySzwXpSVAX40747711 = ySzwXpSVAX25183139;     ySzwXpSVAX25183139 = ySzwXpSVAX51291828;     ySzwXpSVAX51291828 = ySzwXpSVAX91180463;     ySzwXpSVAX91180463 = ySzwXpSVAX83970951;     ySzwXpSVAX83970951 = ySzwXpSVAX62698704;     ySzwXpSVAX62698704 = ySzwXpSVAX30635341;     ySzwXpSVAX30635341 = ySzwXpSVAX97424312;     ySzwXpSVAX97424312 = ySzwXpSVAX49554546;     ySzwXpSVAX49554546 = ySzwXpSVAX75460074;     ySzwXpSVAX75460074 = ySzwXpSVAX32299954;     ySzwXpSVAX32299954 = ySzwXpSVAX79258534;     ySzwXpSVAX79258534 = ySzwXpSVAX82531790;     ySzwXpSVAX82531790 = ySzwXpSVAX66779627;     ySzwXpSVAX66779627 = ySzwXpSVAX52668578;     ySzwXpSVAX52668578 = ySzwXpSVAX77444703;     ySzwXpSVAX77444703 = ySzwXpSVAX20574861;     ySzwXpSVAX20574861 = ySzwXpSVAX74981606;     ySzwXpSVAX74981606 = ySzwXpSVAX21251670;     ySzwXpSVAX21251670 = ySzwXpSVAX37240900;     ySzwXpSVAX37240900 = ySzwXpSVAX39760634;     ySzwXpSVAX39760634 = ySzwXpSVAX53105376;     ySzwXpSVAX53105376 = ySzwXpSVAX44222796;     ySzwXpSVAX44222796 = ySzwXpSVAX36332818;     ySzwXpSVAX36332818 = ySzwXpSVAX34786779;     ySzwXpSVAX34786779 = ySzwXpSVAX44268738;     ySzwXpSVAX44268738 = ySzwXpSVAX34585045;     ySzwXpSVAX34585045 = ySzwXpSVAX49590971;     ySzwXpSVAX49590971 = ySzwXpSVAX65852430;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void rPVvabfTTW73924550() {     double aVzzIoXnEF71969537 = -838740523;    double aVzzIoXnEF23893375 = -485361545;    double aVzzIoXnEF8201316 = -519250089;    double aVzzIoXnEF73031244 = -798262873;    double aVzzIoXnEF94620575 = -814577613;    double aVzzIoXnEF66479121 = -384021984;    double aVzzIoXnEF89233404 = -713073105;    double aVzzIoXnEF82471980 = 79786257;    double aVzzIoXnEF8263003 = -878739941;    double aVzzIoXnEF74888666 = 72588297;    double aVzzIoXnEF39424168 = -730142680;    double aVzzIoXnEF95857478 = 19951369;    double aVzzIoXnEF71985956 = -295042419;    double aVzzIoXnEF21129667 = -642197915;    double aVzzIoXnEF99325994 = 65402654;    double aVzzIoXnEF88411066 = -961095762;    double aVzzIoXnEF52727996 = -55048592;    double aVzzIoXnEF84861 = -279924751;    double aVzzIoXnEF7778013 = -770854018;    double aVzzIoXnEF42428994 = -29922322;    double aVzzIoXnEF28833324 = -585418207;    double aVzzIoXnEF55012057 = -160837238;    double aVzzIoXnEF20288768 = -460168081;    double aVzzIoXnEF41439013 = -225029808;    double aVzzIoXnEF11319016 = -655356843;    double aVzzIoXnEF41268141 = -463830155;    double aVzzIoXnEF96979460 = -889292317;    double aVzzIoXnEF67597381 = -101863274;    double aVzzIoXnEF84531456 = -331818931;    double aVzzIoXnEF71984801 = -652921756;    double aVzzIoXnEF38487330 = -292990415;    double aVzzIoXnEF99704171 = -272722466;    double aVzzIoXnEF28383182 = -583363119;    double aVzzIoXnEF34000777 = -960053698;    double aVzzIoXnEF33817656 = -840227040;    double aVzzIoXnEF85799702 = -893164336;    double aVzzIoXnEF53215328 = -755157881;    double aVzzIoXnEF52187945 = -808692274;    double aVzzIoXnEF35894251 = -828151878;    double aVzzIoXnEF8266971 = -346063778;    double aVzzIoXnEF16256225 = -756589871;    double aVzzIoXnEF81250124 = -957448096;    double aVzzIoXnEF62587059 = -118205570;    double aVzzIoXnEF9407012 = 5416756;    double aVzzIoXnEF38763764 = 15604608;    double aVzzIoXnEF82134248 = -15805319;    double aVzzIoXnEF84770493 = -753974306;    double aVzzIoXnEF31016549 = -362569214;    double aVzzIoXnEF31866031 = -57442675;    double aVzzIoXnEF52052319 = -612463837;    double aVzzIoXnEF56538052 = -904129198;    double aVzzIoXnEF90274873 = -803669644;    double aVzzIoXnEF97673429 = -939748549;    double aVzzIoXnEF37196411 = -67015249;    double aVzzIoXnEF45091226 = -325100351;    double aVzzIoXnEF16957481 = -577903286;    double aVzzIoXnEF3604608 = 74806536;    double aVzzIoXnEF66762302 = -194220281;    double aVzzIoXnEF61712229 = -42906031;    double aVzzIoXnEF53352434 = -250747458;    double aVzzIoXnEF69499661 = -494729668;    double aVzzIoXnEF21636024 = -511209832;    double aVzzIoXnEF97940523 = -588394813;    double aVzzIoXnEF36278202 = -125818186;    double aVzzIoXnEF36401337 = -634421288;    double aVzzIoXnEF39719996 = -357420215;    double aVzzIoXnEF67474296 = -396685513;    double aVzzIoXnEF37985180 = -334988722;    double aVzzIoXnEF87312011 = -801970875;    double aVzzIoXnEF13526292 = -41433010;    double aVzzIoXnEF35195738 = -105937881;    double aVzzIoXnEF540051 = -246356319;    double aVzzIoXnEF64190610 = -451772874;    double aVzzIoXnEF99511042 = -324790241;    double aVzzIoXnEF26172769 = -273332452;    double aVzzIoXnEF47583199 = -627970112;    double aVzzIoXnEF92424997 = 57368332;    double aVzzIoXnEF10881756 = -365584837;    double aVzzIoXnEF2675250 = -140634417;    double aVzzIoXnEF29184767 = -539551524;    double aVzzIoXnEF56497647 = -709855850;    double aVzzIoXnEF65962912 = -426723103;    double aVzzIoXnEF35731351 = 55579400;    double aVzzIoXnEF32479137 = -719355094;    double aVzzIoXnEF15446749 = -748792558;    double aVzzIoXnEF48212457 = -489320772;    double aVzzIoXnEF2030742 = -332973917;    double aVzzIoXnEF91186771 = -416347870;    double aVzzIoXnEF88909550 = -534953348;    double aVzzIoXnEF16860176 = -162323755;    double aVzzIoXnEF82195095 = -867970872;    double aVzzIoXnEF86453025 = -460937600;    double aVzzIoXnEF90475715 = -665786244;    double aVzzIoXnEF82541816 = -477404420;    double aVzzIoXnEF38767309 = -851334111;    double aVzzIoXnEF94620200 = -145380039;    double aVzzIoXnEF83309600 = -269053284;    double aVzzIoXnEF26308858 = -992387385;    double aVzzIoXnEF73005675 = -360161957;    double aVzzIoXnEF99043767 = -838740523;     aVzzIoXnEF71969537 = aVzzIoXnEF23893375;     aVzzIoXnEF23893375 = aVzzIoXnEF8201316;     aVzzIoXnEF8201316 = aVzzIoXnEF73031244;     aVzzIoXnEF73031244 = aVzzIoXnEF94620575;     aVzzIoXnEF94620575 = aVzzIoXnEF66479121;     aVzzIoXnEF66479121 = aVzzIoXnEF89233404;     aVzzIoXnEF89233404 = aVzzIoXnEF82471980;     aVzzIoXnEF82471980 = aVzzIoXnEF8263003;     aVzzIoXnEF8263003 = aVzzIoXnEF74888666;     aVzzIoXnEF74888666 = aVzzIoXnEF39424168;     aVzzIoXnEF39424168 = aVzzIoXnEF95857478;     aVzzIoXnEF95857478 = aVzzIoXnEF71985956;     aVzzIoXnEF71985956 = aVzzIoXnEF21129667;     aVzzIoXnEF21129667 = aVzzIoXnEF99325994;     aVzzIoXnEF99325994 = aVzzIoXnEF88411066;     aVzzIoXnEF88411066 = aVzzIoXnEF52727996;     aVzzIoXnEF52727996 = aVzzIoXnEF84861;     aVzzIoXnEF84861 = aVzzIoXnEF7778013;     aVzzIoXnEF7778013 = aVzzIoXnEF42428994;     aVzzIoXnEF42428994 = aVzzIoXnEF28833324;     aVzzIoXnEF28833324 = aVzzIoXnEF55012057;     aVzzIoXnEF55012057 = aVzzIoXnEF20288768;     aVzzIoXnEF20288768 = aVzzIoXnEF41439013;     aVzzIoXnEF41439013 = aVzzIoXnEF11319016;     aVzzIoXnEF11319016 = aVzzIoXnEF41268141;     aVzzIoXnEF41268141 = aVzzIoXnEF96979460;     aVzzIoXnEF96979460 = aVzzIoXnEF67597381;     aVzzIoXnEF67597381 = aVzzIoXnEF84531456;     aVzzIoXnEF84531456 = aVzzIoXnEF71984801;     aVzzIoXnEF71984801 = aVzzIoXnEF38487330;     aVzzIoXnEF38487330 = aVzzIoXnEF99704171;     aVzzIoXnEF99704171 = aVzzIoXnEF28383182;     aVzzIoXnEF28383182 = aVzzIoXnEF34000777;     aVzzIoXnEF34000777 = aVzzIoXnEF33817656;     aVzzIoXnEF33817656 = aVzzIoXnEF85799702;     aVzzIoXnEF85799702 = aVzzIoXnEF53215328;     aVzzIoXnEF53215328 = aVzzIoXnEF52187945;     aVzzIoXnEF52187945 = aVzzIoXnEF35894251;     aVzzIoXnEF35894251 = aVzzIoXnEF8266971;     aVzzIoXnEF8266971 = aVzzIoXnEF16256225;     aVzzIoXnEF16256225 = aVzzIoXnEF81250124;     aVzzIoXnEF81250124 = aVzzIoXnEF62587059;     aVzzIoXnEF62587059 = aVzzIoXnEF9407012;     aVzzIoXnEF9407012 = aVzzIoXnEF38763764;     aVzzIoXnEF38763764 = aVzzIoXnEF82134248;     aVzzIoXnEF82134248 = aVzzIoXnEF84770493;     aVzzIoXnEF84770493 = aVzzIoXnEF31016549;     aVzzIoXnEF31016549 = aVzzIoXnEF31866031;     aVzzIoXnEF31866031 = aVzzIoXnEF52052319;     aVzzIoXnEF52052319 = aVzzIoXnEF56538052;     aVzzIoXnEF56538052 = aVzzIoXnEF90274873;     aVzzIoXnEF90274873 = aVzzIoXnEF97673429;     aVzzIoXnEF97673429 = aVzzIoXnEF37196411;     aVzzIoXnEF37196411 = aVzzIoXnEF45091226;     aVzzIoXnEF45091226 = aVzzIoXnEF16957481;     aVzzIoXnEF16957481 = aVzzIoXnEF3604608;     aVzzIoXnEF3604608 = aVzzIoXnEF66762302;     aVzzIoXnEF66762302 = aVzzIoXnEF61712229;     aVzzIoXnEF61712229 = aVzzIoXnEF53352434;     aVzzIoXnEF53352434 = aVzzIoXnEF69499661;     aVzzIoXnEF69499661 = aVzzIoXnEF21636024;     aVzzIoXnEF21636024 = aVzzIoXnEF97940523;     aVzzIoXnEF97940523 = aVzzIoXnEF36278202;     aVzzIoXnEF36278202 = aVzzIoXnEF36401337;     aVzzIoXnEF36401337 = aVzzIoXnEF39719996;     aVzzIoXnEF39719996 = aVzzIoXnEF67474296;     aVzzIoXnEF67474296 = aVzzIoXnEF37985180;     aVzzIoXnEF37985180 = aVzzIoXnEF87312011;     aVzzIoXnEF87312011 = aVzzIoXnEF13526292;     aVzzIoXnEF13526292 = aVzzIoXnEF35195738;     aVzzIoXnEF35195738 = aVzzIoXnEF540051;     aVzzIoXnEF540051 = aVzzIoXnEF64190610;     aVzzIoXnEF64190610 = aVzzIoXnEF99511042;     aVzzIoXnEF99511042 = aVzzIoXnEF26172769;     aVzzIoXnEF26172769 = aVzzIoXnEF47583199;     aVzzIoXnEF47583199 = aVzzIoXnEF92424997;     aVzzIoXnEF92424997 = aVzzIoXnEF10881756;     aVzzIoXnEF10881756 = aVzzIoXnEF2675250;     aVzzIoXnEF2675250 = aVzzIoXnEF29184767;     aVzzIoXnEF29184767 = aVzzIoXnEF56497647;     aVzzIoXnEF56497647 = aVzzIoXnEF65962912;     aVzzIoXnEF65962912 = aVzzIoXnEF35731351;     aVzzIoXnEF35731351 = aVzzIoXnEF32479137;     aVzzIoXnEF32479137 = aVzzIoXnEF15446749;     aVzzIoXnEF15446749 = aVzzIoXnEF48212457;     aVzzIoXnEF48212457 = aVzzIoXnEF2030742;     aVzzIoXnEF2030742 = aVzzIoXnEF91186771;     aVzzIoXnEF91186771 = aVzzIoXnEF88909550;     aVzzIoXnEF88909550 = aVzzIoXnEF16860176;     aVzzIoXnEF16860176 = aVzzIoXnEF82195095;     aVzzIoXnEF82195095 = aVzzIoXnEF86453025;     aVzzIoXnEF86453025 = aVzzIoXnEF90475715;     aVzzIoXnEF90475715 = aVzzIoXnEF82541816;     aVzzIoXnEF82541816 = aVzzIoXnEF38767309;     aVzzIoXnEF38767309 = aVzzIoXnEF94620200;     aVzzIoXnEF94620200 = aVzzIoXnEF83309600;     aVzzIoXnEF83309600 = aVzzIoXnEF26308858;     aVzzIoXnEF26308858 = aVzzIoXnEF73005675;     aVzzIoXnEF73005675 = aVzzIoXnEF99043767;     aVzzIoXnEF99043767 = aVzzIoXnEF71969537;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void tNRAOewgXT19620243() {     double wVMPYDJYGo91158495 = -682515349;    double wVMPYDJYGo96191232 = -126969877;    double wVMPYDJYGo32860698 = -96096241;    double wVMPYDJYGo57322692 = -803755043;    double wVMPYDJYGo21627079 = -849769329;    double wVMPYDJYGo79236078 = 49742264;    double wVMPYDJYGo68224038 = -874776510;    double wVMPYDJYGo18729573 = -480725152;    double wVMPYDJYGo77259972 = -150907974;    double wVMPYDJYGo39257453 = -178332845;    double wVMPYDJYGo174334 = -412963595;    double wVMPYDJYGo72345720 = -37830311;    double wVMPYDJYGo29421540 = -668228693;    double wVMPYDJYGo14799455 = -623091135;    double wVMPYDJYGo16527661 = -401006814;    double wVMPYDJYGo69201848 = -188239370;    double wVMPYDJYGo52576583 = -921288438;    double wVMPYDJYGo90921788 = -667132771;    double wVMPYDJYGo28876554 = -258084411;    double wVMPYDJYGo38300094 = -560751854;    double wVMPYDJYGo29590273 = -106693043;    double wVMPYDJYGo81139386 = -750636935;    double wVMPYDJYGo23111616 = -768237160;    double wVMPYDJYGo92724545 = -394166543;    double wVMPYDJYGo27496370 = -330978949;    double wVMPYDJYGo88384993 = -687168062;    double wVMPYDJYGo8770393 = -838652168;    double wVMPYDJYGo15521481 = -549109948;    double wVMPYDJYGo90439365 = -784039319;    double wVMPYDJYGo36976669 = -584603927;    double wVMPYDJYGo5022306 = -676975688;    double wVMPYDJYGo77596621 = -203069341;    double wVMPYDJYGo69273471 = -94228972;    double wVMPYDJYGo99191080 = -452162426;    double wVMPYDJYGo84871397 = -266303974;    double wVMPYDJYGo64393591 = 93639627;    double wVMPYDJYGo90449366 = -840541840;    double wVMPYDJYGo83867460 = -695228393;    double wVMPYDJYGo91586934 = -71905619;    double wVMPYDJYGo39854726 = -148541517;    double wVMPYDJYGo26619686 = -87885013;    double wVMPYDJYGo14257317 = 35773026;    double wVMPYDJYGo24487224 = -964660356;    double wVMPYDJYGo37501972 = -620894943;    double wVMPYDJYGo82749420 = -763661230;    double wVMPYDJYGo31934490 = -957715972;    double wVMPYDJYGo9596835 = -778325951;    double wVMPYDJYGo12101127 = -782456349;    double wVMPYDJYGo69974520 = -957199844;    double wVMPYDJYGo92409815 = 51518798;    double wVMPYDJYGo64140237 = -265623150;    double wVMPYDJYGo10718808 = -387229029;    double wVMPYDJYGo36228633 = -815672461;    double wVMPYDJYGo88870592 = -303313085;    double wVMPYDJYGo90029969 = -131418164;    double wVMPYDJYGo10019109 = -931878414;    double wVMPYDJYGo73079616 = -358732718;    double wVMPYDJYGo40136152 = -701929699;    double wVMPYDJYGo29826322 = -372776094;    double wVMPYDJYGo33242085 = -62601268;    double wVMPYDJYGo70465686 = -111605569;    double wVMPYDJYGo52702557 = -225666563;    double wVMPYDJYGo28290208 = -696685834;    double wVMPYDJYGo40283303 = -566304048;    double wVMPYDJYGo34235148 = -501357158;    double wVMPYDJYGo22577712 = -109894254;    double wVMPYDJYGo3072249 = -943601340;    double wVMPYDJYGo30230460 = -116066267;    double wVMPYDJYGo29928058 = -256787162;    double wVMPYDJYGo52134070 = -394646442;    double wVMPYDJYGo78752481 = -347697531;    double wVMPYDJYGo68709123 = -126060046;    double wVMPYDJYGo99334854 = -495227153;    double wVMPYDJYGo89021828 = -9542895;    double wVMPYDJYGo11680408 = -372866841;    double wVMPYDJYGo15332956 = -42466069;    double wVMPYDJYGo56652162 = -785976579;    double wVMPYDJYGo85609644 = -47342217;    double wVMPYDJYGo9975125 = -630505314;    double wVMPYDJYGo95561879 = -373262978;    double wVMPYDJYGo78788159 = -908842111;    double wVMPYDJYGo96669265 = 43804181;    double wVMPYDJYGo45546961 = -591910104;    double wVMPYDJYGo98029550 = -735558117;    double wVMPYDJYGo72836431 = -218980777;    double wVMPYDJYGo94303498 = -189746660;    double wVMPYDJYGo41367989 = -387396881;    double wVMPYDJYGo80402879 = -790915887;    double wVMPYDJYGo9161111 = -220744263;    double wVMPYDJYGo74852288 = -334425560;    double wVMPYDJYGo91313974 = -547627655;    double wVMPYDJYGo50313214 = -38612142;    double wVMPYDJYGo54041138 = -222452299;    double wVMPYDJYGo58344849 = 90695648;    double wVMPYDJYGo69389039 = 63064051;    double wVMPYDJYGo73917128 = -862218451;    double wVMPYDJYGo85967109 = -267541140;    double wVMPYDJYGo84203920 = -298356309;    double wVMPYDJYGo3266824 = -19537786;    double wVMPYDJYGo60171708 = -682515349;     wVMPYDJYGo91158495 = wVMPYDJYGo96191232;     wVMPYDJYGo96191232 = wVMPYDJYGo32860698;     wVMPYDJYGo32860698 = wVMPYDJYGo57322692;     wVMPYDJYGo57322692 = wVMPYDJYGo21627079;     wVMPYDJYGo21627079 = wVMPYDJYGo79236078;     wVMPYDJYGo79236078 = wVMPYDJYGo68224038;     wVMPYDJYGo68224038 = wVMPYDJYGo18729573;     wVMPYDJYGo18729573 = wVMPYDJYGo77259972;     wVMPYDJYGo77259972 = wVMPYDJYGo39257453;     wVMPYDJYGo39257453 = wVMPYDJYGo174334;     wVMPYDJYGo174334 = wVMPYDJYGo72345720;     wVMPYDJYGo72345720 = wVMPYDJYGo29421540;     wVMPYDJYGo29421540 = wVMPYDJYGo14799455;     wVMPYDJYGo14799455 = wVMPYDJYGo16527661;     wVMPYDJYGo16527661 = wVMPYDJYGo69201848;     wVMPYDJYGo69201848 = wVMPYDJYGo52576583;     wVMPYDJYGo52576583 = wVMPYDJYGo90921788;     wVMPYDJYGo90921788 = wVMPYDJYGo28876554;     wVMPYDJYGo28876554 = wVMPYDJYGo38300094;     wVMPYDJYGo38300094 = wVMPYDJYGo29590273;     wVMPYDJYGo29590273 = wVMPYDJYGo81139386;     wVMPYDJYGo81139386 = wVMPYDJYGo23111616;     wVMPYDJYGo23111616 = wVMPYDJYGo92724545;     wVMPYDJYGo92724545 = wVMPYDJYGo27496370;     wVMPYDJYGo27496370 = wVMPYDJYGo88384993;     wVMPYDJYGo88384993 = wVMPYDJYGo8770393;     wVMPYDJYGo8770393 = wVMPYDJYGo15521481;     wVMPYDJYGo15521481 = wVMPYDJYGo90439365;     wVMPYDJYGo90439365 = wVMPYDJYGo36976669;     wVMPYDJYGo36976669 = wVMPYDJYGo5022306;     wVMPYDJYGo5022306 = wVMPYDJYGo77596621;     wVMPYDJYGo77596621 = wVMPYDJYGo69273471;     wVMPYDJYGo69273471 = wVMPYDJYGo99191080;     wVMPYDJYGo99191080 = wVMPYDJYGo84871397;     wVMPYDJYGo84871397 = wVMPYDJYGo64393591;     wVMPYDJYGo64393591 = wVMPYDJYGo90449366;     wVMPYDJYGo90449366 = wVMPYDJYGo83867460;     wVMPYDJYGo83867460 = wVMPYDJYGo91586934;     wVMPYDJYGo91586934 = wVMPYDJYGo39854726;     wVMPYDJYGo39854726 = wVMPYDJYGo26619686;     wVMPYDJYGo26619686 = wVMPYDJYGo14257317;     wVMPYDJYGo14257317 = wVMPYDJYGo24487224;     wVMPYDJYGo24487224 = wVMPYDJYGo37501972;     wVMPYDJYGo37501972 = wVMPYDJYGo82749420;     wVMPYDJYGo82749420 = wVMPYDJYGo31934490;     wVMPYDJYGo31934490 = wVMPYDJYGo9596835;     wVMPYDJYGo9596835 = wVMPYDJYGo12101127;     wVMPYDJYGo12101127 = wVMPYDJYGo69974520;     wVMPYDJYGo69974520 = wVMPYDJYGo92409815;     wVMPYDJYGo92409815 = wVMPYDJYGo64140237;     wVMPYDJYGo64140237 = wVMPYDJYGo10718808;     wVMPYDJYGo10718808 = wVMPYDJYGo36228633;     wVMPYDJYGo36228633 = wVMPYDJYGo88870592;     wVMPYDJYGo88870592 = wVMPYDJYGo90029969;     wVMPYDJYGo90029969 = wVMPYDJYGo10019109;     wVMPYDJYGo10019109 = wVMPYDJYGo73079616;     wVMPYDJYGo73079616 = wVMPYDJYGo40136152;     wVMPYDJYGo40136152 = wVMPYDJYGo29826322;     wVMPYDJYGo29826322 = wVMPYDJYGo33242085;     wVMPYDJYGo33242085 = wVMPYDJYGo70465686;     wVMPYDJYGo70465686 = wVMPYDJYGo52702557;     wVMPYDJYGo52702557 = wVMPYDJYGo28290208;     wVMPYDJYGo28290208 = wVMPYDJYGo40283303;     wVMPYDJYGo40283303 = wVMPYDJYGo34235148;     wVMPYDJYGo34235148 = wVMPYDJYGo22577712;     wVMPYDJYGo22577712 = wVMPYDJYGo3072249;     wVMPYDJYGo3072249 = wVMPYDJYGo30230460;     wVMPYDJYGo30230460 = wVMPYDJYGo29928058;     wVMPYDJYGo29928058 = wVMPYDJYGo52134070;     wVMPYDJYGo52134070 = wVMPYDJYGo78752481;     wVMPYDJYGo78752481 = wVMPYDJYGo68709123;     wVMPYDJYGo68709123 = wVMPYDJYGo99334854;     wVMPYDJYGo99334854 = wVMPYDJYGo89021828;     wVMPYDJYGo89021828 = wVMPYDJYGo11680408;     wVMPYDJYGo11680408 = wVMPYDJYGo15332956;     wVMPYDJYGo15332956 = wVMPYDJYGo56652162;     wVMPYDJYGo56652162 = wVMPYDJYGo85609644;     wVMPYDJYGo85609644 = wVMPYDJYGo9975125;     wVMPYDJYGo9975125 = wVMPYDJYGo95561879;     wVMPYDJYGo95561879 = wVMPYDJYGo78788159;     wVMPYDJYGo78788159 = wVMPYDJYGo96669265;     wVMPYDJYGo96669265 = wVMPYDJYGo45546961;     wVMPYDJYGo45546961 = wVMPYDJYGo98029550;     wVMPYDJYGo98029550 = wVMPYDJYGo72836431;     wVMPYDJYGo72836431 = wVMPYDJYGo94303498;     wVMPYDJYGo94303498 = wVMPYDJYGo41367989;     wVMPYDJYGo41367989 = wVMPYDJYGo80402879;     wVMPYDJYGo80402879 = wVMPYDJYGo9161111;     wVMPYDJYGo9161111 = wVMPYDJYGo74852288;     wVMPYDJYGo74852288 = wVMPYDJYGo91313974;     wVMPYDJYGo91313974 = wVMPYDJYGo50313214;     wVMPYDJYGo50313214 = wVMPYDJYGo54041138;     wVMPYDJYGo54041138 = wVMPYDJYGo58344849;     wVMPYDJYGo58344849 = wVMPYDJYGo69389039;     wVMPYDJYGo69389039 = wVMPYDJYGo73917128;     wVMPYDJYGo73917128 = wVMPYDJYGo85967109;     wVMPYDJYGo85967109 = wVMPYDJYGo84203920;     wVMPYDJYGo84203920 = wVMPYDJYGo3266824;     wVMPYDJYGo3266824 = wVMPYDJYGo60171708;     wVMPYDJYGo60171708 = wVMPYDJYGo91158495;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void eHiRvgYPPi26278987() {     double LIZNybGNWC91152023 = -555039488;    double LIZNybGNWC34763549 = -169564996;    double LIZNybGNWC34173910 = -53224855;    double LIZNybGNWC42402553 = -624295876;    double LIZNybGNWC30467181 = -218124642;    double LIZNybGNWC14041511 = -749690548;    double LIZNybGNWC36771374 = -579312977;    double LIZNybGNWC87943623 = -184902454;    double LIZNybGNWC60000136 = -964554043;    double LIZNybGNWC35896967 = -915139716;    double LIZNybGNWC20494939 = -372435666;    double LIZNybGNWC86637397 = -873308665;    double LIZNybGNWC61274453 = -689887167;    double LIZNybGNWC68643675 = -968244186;    double LIZNybGNWC61699769 = -363719647;    double LIZNybGNWC43766395 = -521035021;    double LIZNybGNWC8429343 = -740903008;    double LIZNybGNWC13922322 = 35795611;    double LIZNybGNWC49304823 = -876066679;    double LIZNybGNWC10373857 = -72776305;    double LIZNybGNWC9259697 = -917159412;    double LIZNybGNWC59435652 = -160628818;    double LIZNybGNWC59990012 = -127195002;    double LIZNybGNWC36374653 = -275087065;    double LIZNybGNWC55939059 = -757672790;    double LIZNybGNWC4959070 = -595373994;    double LIZNybGNWC78991974 = -39451881;    double LIZNybGNWC50925006 = -58076930;    double LIZNybGNWC3117813 = -330109649;    double LIZNybGNWC13066540 = -260035301;    double LIZNybGNWC38434985 = 35931077;    double LIZNybGNWC62320479 = -594891106;    double LIZNybGNWC54725894 = -296173191;    double LIZNybGNWC99473917 = -328844159;    double LIZNybGNWC67229435 = -525617901;    double LIZNybGNWC7221780 = 13873862;    double LIZNybGNWC2390735 = -940195218;    double LIZNybGNWC53785140 = -690491069;    double LIZNybGNWC26900531 = -784587047;    double LIZNybGNWC11727619 = -857973203;    double LIZNybGNWC14475354 = -175828912;    double LIZNybGNWC99954977 = -158019077;    double LIZNybGNWC84770584 = -423790613;    double LIZNybGNWC96733869 = -177859387;    double LIZNybGNWC66767473 = -136920632;    double LIZNybGNWC72362458 = -769760661;    double LIZNybGNWC37294627 = -159606574;    double LIZNybGNWC87484707 = -291217706;    double LIZNybGNWC88632910 = -520963730;    double LIZNybGNWC12455239 = -635506083;    double LIZNybGNWC77399607 = -944178157;    double LIZNybGNWC25266065 = -769374781;    double LIZNybGNWC11676982 = -707726466;    double LIZNybGNWC99120863 = -835232936;    double LIZNybGNWC17019291 = -988317446;    double LIZNybGNWC31716372 = -294410671;    double LIZNybGNWC74773537 = 57630005;    double LIZNybGNWC97799256 = -778137790;    double LIZNybGNWC86463493 = -866623086;    double LIZNybGNWC25508111 = -622750648;    double LIZNybGNWC35049537 = -610238668;    double LIZNybGNWC85846368 = -421236047;    double LIZNybGNWC84825811 = -854792806;    double LIZNybGNWC46933597 = -604518743;    double LIZNybGNWC97461982 = -851070794;    double LIZNybGNWC58174460 = -777544561;    double LIZNybGNWC31911503 = -477135475;    double LIZNybGNWC61800536 = -261043009;    double LIZNybGNWC1414240 = -342626285;    double LIZNybGNWC54477990 = -277593510;    double LIZNybGNWC41375661 = -580839803;    double LIZNybGNWC54644203 = 49588061;    double LIZNybGNWC87021790 = -179617342;    double LIZNybGNWC37577205 = 81906524;    double LIZNybGNWC95898503 = -896947394;    double LIZNybGNWC9304719 = -659140335;    double LIZNybGNWC74665068 = -736838206;    double LIZNybGNWC63256143 = -949335615;    double LIZNybGNWC69607180 = -38166434;    double LIZNybGNWC83576601 = -987912130;    double LIZNybGNWC67664442 = -335767421;    double LIZNybGNWC91507266 = -748234175;    double LIZNybGNWC62292096 = -537113201;    double LIZNybGNWC90662573 = -694603566;    double LIZNybGNWC35666932 = -315857144;    double LIZNybGNWC13168920 = -194694142;    double LIZNybGNWC50643497 = -887164640;    double LIZNybGNWC55605031 = -460940255;    double LIZNybGNWC82454627 = -340526713;    double LIZNybGNWC35513064 = -131207231;    double LIZNybGNWC32448243 = 56243857;    double LIZNybGNWC4591479 = -62057429;    double LIZNybGNWC67321646 = -823867983;    double LIZNybGNWC1392421 = -61836399;    double LIZNybGNWC76678081 = -147734536;    double LIZNybGNWC28628985 = -754592865;    double LIZNybGNWC15129167 = -303226271;    double LIZNybGNWC37836987 = -819271870;    double LIZNybGNWC99271887 = -326788593;    double LIZNybGNWC8593013 = -555039488;     LIZNybGNWC91152023 = LIZNybGNWC34763549;     LIZNybGNWC34763549 = LIZNybGNWC34173910;     LIZNybGNWC34173910 = LIZNybGNWC42402553;     LIZNybGNWC42402553 = LIZNybGNWC30467181;     LIZNybGNWC30467181 = LIZNybGNWC14041511;     LIZNybGNWC14041511 = LIZNybGNWC36771374;     LIZNybGNWC36771374 = LIZNybGNWC87943623;     LIZNybGNWC87943623 = LIZNybGNWC60000136;     LIZNybGNWC60000136 = LIZNybGNWC35896967;     LIZNybGNWC35896967 = LIZNybGNWC20494939;     LIZNybGNWC20494939 = LIZNybGNWC86637397;     LIZNybGNWC86637397 = LIZNybGNWC61274453;     LIZNybGNWC61274453 = LIZNybGNWC68643675;     LIZNybGNWC68643675 = LIZNybGNWC61699769;     LIZNybGNWC61699769 = LIZNybGNWC43766395;     LIZNybGNWC43766395 = LIZNybGNWC8429343;     LIZNybGNWC8429343 = LIZNybGNWC13922322;     LIZNybGNWC13922322 = LIZNybGNWC49304823;     LIZNybGNWC49304823 = LIZNybGNWC10373857;     LIZNybGNWC10373857 = LIZNybGNWC9259697;     LIZNybGNWC9259697 = LIZNybGNWC59435652;     LIZNybGNWC59435652 = LIZNybGNWC59990012;     LIZNybGNWC59990012 = LIZNybGNWC36374653;     LIZNybGNWC36374653 = LIZNybGNWC55939059;     LIZNybGNWC55939059 = LIZNybGNWC4959070;     LIZNybGNWC4959070 = LIZNybGNWC78991974;     LIZNybGNWC78991974 = LIZNybGNWC50925006;     LIZNybGNWC50925006 = LIZNybGNWC3117813;     LIZNybGNWC3117813 = LIZNybGNWC13066540;     LIZNybGNWC13066540 = LIZNybGNWC38434985;     LIZNybGNWC38434985 = LIZNybGNWC62320479;     LIZNybGNWC62320479 = LIZNybGNWC54725894;     LIZNybGNWC54725894 = LIZNybGNWC99473917;     LIZNybGNWC99473917 = LIZNybGNWC67229435;     LIZNybGNWC67229435 = LIZNybGNWC7221780;     LIZNybGNWC7221780 = LIZNybGNWC2390735;     LIZNybGNWC2390735 = LIZNybGNWC53785140;     LIZNybGNWC53785140 = LIZNybGNWC26900531;     LIZNybGNWC26900531 = LIZNybGNWC11727619;     LIZNybGNWC11727619 = LIZNybGNWC14475354;     LIZNybGNWC14475354 = LIZNybGNWC99954977;     LIZNybGNWC99954977 = LIZNybGNWC84770584;     LIZNybGNWC84770584 = LIZNybGNWC96733869;     LIZNybGNWC96733869 = LIZNybGNWC66767473;     LIZNybGNWC66767473 = LIZNybGNWC72362458;     LIZNybGNWC72362458 = LIZNybGNWC37294627;     LIZNybGNWC37294627 = LIZNybGNWC87484707;     LIZNybGNWC87484707 = LIZNybGNWC88632910;     LIZNybGNWC88632910 = LIZNybGNWC12455239;     LIZNybGNWC12455239 = LIZNybGNWC77399607;     LIZNybGNWC77399607 = LIZNybGNWC25266065;     LIZNybGNWC25266065 = LIZNybGNWC11676982;     LIZNybGNWC11676982 = LIZNybGNWC99120863;     LIZNybGNWC99120863 = LIZNybGNWC17019291;     LIZNybGNWC17019291 = LIZNybGNWC31716372;     LIZNybGNWC31716372 = LIZNybGNWC74773537;     LIZNybGNWC74773537 = LIZNybGNWC97799256;     LIZNybGNWC97799256 = LIZNybGNWC86463493;     LIZNybGNWC86463493 = LIZNybGNWC25508111;     LIZNybGNWC25508111 = LIZNybGNWC35049537;     LIZNybGNWC35049537 = LIZNybGNWC85846368;     LIZNybGNWC85846368 = LIZNybGNWC84825811;     LIZNybGNWC84825811 = LIZNybGNWC46933597;     LIZNybGNWC46933597 = LIZNybGNWC97461982;     LIZNybGNWC97461982 = LIZNybGNWC58174460;     LIZNybGNWC58174460 = LIZNybGNWC31911503;     LIZNybGNWC31911503 = LIZNybGNWC61800536;     LIZNybGNWC61800536 = LIZNybGNWC1414240;     LIZNybGNWC1414240 = LIZNybGNWC54477990;     LIZNybGNWC54477990 = LIZNybGNWC41375661;     LIZNybGNWC41375661 = LIZNybGNWC54644203;     LIZNybGNWC54644203 = LIZNybGNWC87021790;     LIZNybGNWC87021790 = LIZNybGNWC37577205;     LIZNybGNWC37577205 = LIZNybGNWC95898503;     LIZNybGNWC95898503 = LIZNybGNWC9304719;     LIZNybGNWC9304719 = LIZNybGNWC74665068;     LIZNybGNWC74665068 = LIZNybGNWC63256143;     LIZNybGNWC63256143 = LIZNybGNWC69607180;     LIZNybGNWC69607180 = LIZNybGNWC83576601;     LIZNybGNWC83576601 = LIZNybGNWC67664442;     LIZNybGNWC67664442 = LIZNybGNWC91507266;     LIZNybGNWC91507266 = LIZNybGNWC62292096;     LIZNybGNWC62292096 = LIZNybGNWC90662573;     LIZNybGNWC90662573 = LIZNybGNWC35666932;     LIZNybGNWC35666932 = LIZNybGNWC13168920;     LIZNybGNWC13168920 = LIZNybGNWC50643497;     LIZNybGNWC50643497 = LIZNybGNWC55605031;     LIZNybGNWC55605031 = LIZNybGNWC82454627;     LIZNybGNWC82454627 = LIZNybGNWC35513064;     LIZNybGNWC35513064 = LIZNybGNWC32448243;     LIZNybGNWC32448243 = LIZNybGNWC4591479;     LIZNybGNWC4591479 = LIZNybGNWC67321646;     LIZNybGNWC67321646 = LIZNybGNWC1392421;     LIZNybGNWC1392421 = LIZNybGNWC76678081;     LIZNybGNWC76678081 = LIZNybGNWC28628985;     LIZNybGNWC28628985 = LIZNybGNWC15129167;     LIZNybGNWC15129167 = LIZNybGNWC37836987;     LIZNybGNWC37836987 = LIZNybGNWC99271887;     LIZNybGNWC99271887 = LIZNybGNWC8593013;     LIZNybGNWC8593013 = LIZNybGNWC91152023;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void yRObATnjDh71974679() {     double aVfSvBdqGi10340982 = -398814316;    double aVfSvBdqGi7061407 = -911173335;    double aVfSvBdqGi58833291 = -730071007;    double aVfSvBdqGi26694000 = -629788043;    double aVfSvBdqGi57473684 = -253316366;    double aVfSvBdqGi26798469 = -315926301;    double aVfSvBdqGi15762008 = -741016383;    double aVfSvBdqGi24201217 = -745413879;    double aVfSvBdqGi28997106 = -236722078;    double aVfSvBdqGi265754 = -66060859;    double aVfSvBdqGi81245104 = -55256582;    double aVfSvBdqGi63125639 = -931090354;    double aVfSvBdqGi18710037 = 36926559;    double aVfSvBdqGi62313463 = -949137407;    double aVfSvBdqGi78901435 = -830129134;    double aVfSvBdqGi24557178 = -848178628;    double aVfSvBdqGi8277930 = -507142853;    double aVfSvBdqGi4759250 = -351412414;    double aVfSvBdqGi70403364 = -363297071;    double aVfSvBdqGi6244957 = -603605837;    double aVfSvBdqGi10016646 = -438434252;    double aVfSvBdqGi85562982 = -750428515;    double aVfSvBdqGi62812861 = -435264081;    double aVfSvBdqGi87660185 = -444223800;    double aVfSvBdqGi72116413 = -433294909;    double aVfSvBdqGi52075923 = -818711896;    double aVfSvBdqGi90782905 = 11188268;    double aVfSvBdqGi98849106 = -505323611;    double aVfSvBdqGi9025722 = -782330021;    double aVfSvBdqGi78058407 = -191717472;    double aVfSvBdqGi4969961 = -348054196;    double aVfSvBdqGi40212928 = -525237978;    double aVfSvBdqGi95616183 = -907039040;    double aVfSvBdqGi64664221 = -920952887;    double aVfSvBdqGi18283177 = 48305168;    double aVfSvBdqGi85815668 = -99322156;    double aVfSvBdqGi39624773 = 74420824;    double aVfSvBdqGi85464654 = -577027191;    double aVfSvBdqGi82593215 = -28340777;    double aVfSvBdqGi43315374 = -660450943;    double aVfSvBdqGi24838814 = -607124054;    double aVfSvBdqGi32962170 = -264797934;    double aVfSvBdqGi46670749 = -170245397;    double aVfSvBdqGi24828829 = -804171086;    double aVfSvBdqGi10753130 = -916186469;    double aVfSvBdqGi22162700 = -611671296;    double aVfSvBdqGi62120967 = -183958219;    double aVfSvBdqGi68569286 = -711104840;    double aVfSvBdqGi26741400 = -320720876;    double aVfSvBdqGi52812735 = 28476547;    double aVfSvBdqGi85001793 = -305672109;    double aVfSvBdqGi45710000 = -352934161;    double aVfSvBdqGi50232184 = -583650381;    double aVfSvBdqGi50795045 = 28469227;    double aVfSvBdqGi61958034 = -794635254;    double aVfSvBdqGi24777999 = -648385801;    double aVfSvBdqGi44248545 = -375909255;    double aVfSvBdqGi71173106 = -185847207;    double aVfSvBdqGi54577586 = -96493134;    double aVfSvBdqGi5397762 = -434604470;    double aVfSvBdqGi36015563 = -227114570;    double aVfSvBdqGi16912902 = -135692772;    double aVfSvBdqGi15175495 = -963083858;    double aVfSvBdqGi50938699 = 54995394;    double aVfSvBdqGi95295793 = -718006663;    double aVfSvBdqGi41032176 = -530018604;    double aVfSvBdqGi67509455 = 75948686;    double aVfSvBdqGi54045816 = -42120555;    double aVfSvBdqGi44030286 = -897442576;    double aVfSvBdqGi93085767 = -630806979;    double aVfSvBdqGi84932404 = -822599453;    double aVfSvBdqGi22813275 = -930115663;    double aVfSvBdqGi22166035 = -223071638;    double aVfSvBdqGi27087991 = -702846129;    double aVfSvBdqGi81406142 = -996481783;    double aVfSvBdqGi77054475 = -73636319;    double aVfSvBdqGi38892234 = -480183119;    double aVfSvBdqGi37984032 = -631092995;    double aVfSvBdqGi76907055 = -528037331;    double aVfSvBdqGi49953714 = -821623614;    double aVfSvBdqGi89954955 = -534753678;    double aVfSvBdqGi22213620 = -277706892;    double aVfSvBdqGi72107706 = -84602736;    double aVfSvBdqGi56212987 = -710806569;    double aVfSvBdqGi93056614 = -886045363;    double aVfSvBdqGi59259960 = -995120035;    double aVfSvBdqGi89980744 = -941587598;    double aVfSvBdqGi44821139 = -835508268;    double aVfSvBdqGi2706187 = -26317634;    double aVfSvBdqGi93505177 = -303309031;    double aVfSvBdqGi41567123 = -723412902;    double aVfSvBdqGi68451667 = -739731970;    double aVfSvBdqGi30887069 = -380534057;    double aVfSvBdqGi77195453 = -593736307;    double aVfSvBdqGi7299811 = -333336374;    double aVfSvBdqGi7925913 = -371431282;    double aVfSvBdqGi17786675 = -301714077;    double aVfSvBdqGi95732049 = -125240791;    double aVfSvBdqGi29533036 = 13835577;    double aVfSvBdqGi69720954 = -398814316;     aVfSvBdqGi10340982 = aVfSvBdqGi7061407;     aVfSvBdqGi7061407 = aVfSvBdqGi58833291;     aVfSvBdqGi58833291 = aVfSvBdqGi26694000;     aVfSvBdqGi26694000 = aVfSvBdqGi57473684;     aVfSvBdqGi57473684 = aVfSvBdqGi26798469;     aVfSvBdqGi26798469 = aVfSvBdqGi15762008;     aVfSvBdqGi15762008 = aVfSvBdqGi24201217;     aVfSvBdqGi24201217 = aVfSvBdqGi28997106;     aVfSvBdqGi28997106 = aVfSvBdqGi265754;     aVfSvBdqGi265754 = aVfSvBdqGi81245104;     aVfSvBdqGi81245104 = aVfSvBdqGi63125639;     aVfSvBdqGi63125639 = aVfSvBdqGi18710037;     aVfSvBdqGi18710037 = aVfSvBdqGi62313463;     aVfSvBdqGi62313463 = aVfSvBdqGi78901435;     aVfSvBdqGi78901435 = aVfSvBdqGi24557178;     aVfSvBdqGi24557178 = aVfSvBdqGi8277930;     aVfSvBdqGi8277930 = aVfSvBdqGi4759250;     aVfSvBdqGi4759250 = aVfSvBdqGi70403364;     aVfSvBdqGi70403364 = aVfSvBdqGi6244957;     aVfSvBdqGi6244957 = aVfSvBdqGi10016646;     aVfSvBdqGi10016646 = aVfSvBdqGi85562982;     aVfSvBdqGi85562982 = aVfSvBdqGi62812861;     aVfSvBdqGi62812861 = aVfSvBdqGi87660185;     aVfSvBdqGi87660185 = aVfSvBdqGi72116413;     aVfSvBdqGi72116413 = aVfSvBdqGi52075923;     aVfSvBdqGi52075923 = aVfSvBdqGi90782905;     aVfSvBdqGi90782905 = aVfSvBdqGi98849106;     aVfSvBdqGi98849106 = aVfSvBdqGi9025722;     aVfSvBdqGi9025722 = aVfSvBdqGi78058407;     aVfSvBdqGi78058407 = aVfSvBdqGi4969961;     aVfSvBdqGi4969961 = aVfSvBdqGi40212928;     aVfSvBdqGi40212928 = aVfSvBdqGi95616183;     aVfSvBdqGi95616183 = aVfSvBdqGi64664221;     aVfSvBdqGi64664221 = aVfSvBdqGi18283177;     aVfSvBdqGi18283177 = aVfSvBdqGi85815668;     aVfSvBdqGi85815668 = aVfSvBdqGi39624773;     aVfSvBdqGi39624773 = aVfSvBdqGi85464654;     aVfSvBdqGi85464654 = aVfSvBdqGi82593215;     aVfSvBdqGi82593215 = aVfSvBdqGi43315374;     aVfSvBdqGi43315374 = aVfSvBdqGi24838814;     aVfSvBdqGi24838814 = aVfSvBdqGi32962170;     aVfSvBdqGi32962170 = aVfSvBdqGi46670749;     aVfSvBdqGi46670749 = aVfSvBdqGi24828829;     aVfSvBdqGi24828829 = aVfSvBdqGi10753130;     aVfSvBdqGi10753130 = aVfSvBdqGi22162700;     aVfSvBdqGi22162700 = aVfSvBdqGi62120967;     aVfSvBdqGi62120967 = aVfSvBdqGi68569286;     aVfSvBdqGi68569286 = aVfSvBdqGi26741400;     aVfSvBdqGi26741400 = aVfSvBdqGi52812735;     aVfSvBdqGi52812735 = aVfSvBdqGi85001793;     aVfSvBdqGi85001793 = aVfSvBdqGi45710000;     aVfSvBdqGi45710000 = aVfSvBdqGi50232184;     aVfSvBdqGi50232184 = aVfSvBdqGi50795045;     aVfSvBdqGi50795045 = aVfSvBdqGi61958034;     aVfSvBdqGi61958034 = aVfSvBdqGi24777999;     aVfSvBdqGi24777999 = aVfSvBdqGi44248545;     aVfSvBdqGi44248545 = aVfSvBdqGi71173106;     aVfSvBdqGi71173106 = aVfSvBdqGi54577586;     aVfSvBdqGi54577586 = aVfSvBdqGi5397762;     aVfSvBdqGi5397762 = aVfSvBdqGi36015563;     aVfSvBdqGi36015563 = aVfSvBdqGi16912902;     aVfSvBdqGi16912902 = aVfSvBdqGi15175495;     aVfSvBdqGi15175495 = aVfSvBdqGi50938699;     aVfSvBdqGi50938699 = aVfSvBdqGi95295793;     aVfSvBdqGi95295793 = aVfSvBdqGi41032176;     aVfSvBdqGi41032176 = aVfSvBdqGi67509455;     aVfSvBdqGi67509455 = aVfSvBdqGi54045816;     aVfSvBdqGi54045816 = aVfSvBdqGi44030286;     aVfSvBdqGi44030286 = aVfSvBdqGi93085767;     aVfSvBdqGi93085767 = aVfSvBdqGi84932404;     aVfSvBdqGi84932404 = aVfSvBdqGi22813275;     aVfSvBdqGi22813275 = aVfSvBdqGi22166035;     aVfSvBdqGi22166035 = aVfSvBdqGi27087991;     aVfSvBdqGi27087991 = aVfSvBdqGi81406142;     aVfSvBdqGi81406142 = aVfSvBdqGi77054475;     aVfSvBdqGi77054475 = aVfSvBdqGi38892234;     aVfSvBdqGi38892234 = aVfSvBdqGi37984032;     aVfSvBdqGi37984032 = aVfSvBdqGi76907055;     aVfSvBdqGi76907055 = aVfSvBdqGi49953714;     aVfSvBdqGi49953714 = aVfSvBdqGi89954955;     aVfSvBdqGi89954955 = aVfSvBdqGi22213620;     aVfSvBdqGi22213620 = aVfSvBdqGi72107706;     aVfSvBdqGi72107706 = aVfSvBdqGi56212987;     aVfSvBdqGi56212987 = aVfSvBdqGi93056614;     aVfSvBdqGi93056614 = aVfSvBdqGi59259960;     aVfSvBdqGi59259960 = aVfSvBdqGi89980744;     aVfSvBdqGi89980744 = aVfSvBdqGi44821139;     aVfSvBdqGi44821139 = aVfSvBdqGi2706187;     aVfSvBdqGi2706187 = aVfSvBdqGi93505177;     aVfSvBdqGi93505177 = aVfSvBdqGi41567123;     aVfSvBdqGi41567123 = aVfSvBdqGi68451667;     aVfSvBdqGi68451667 = aVfSvBdqGi30887069;     aVfSvBdqGi30887069 = aVfSvBdqGi77195453;     aVfSvBdqGi77195453 = aVfSvBdqGi7299811;     aVfSvBdqGi7299811 = aVfSvBdqGi7925913;     aVfSvBdqGi7925913 = aVfSvBdqGi17786675;     aVfSvBdqGi17786675 = aVfSvBdqGi95732049;     aVfSvBdqGi95732049 = aVfSvBdqGi29533036;     aVfSvBdqGi29533036 = aVfSvBdqGi69720954;     aVfSvBdqGi69720954 = aVfSvBdqGi10340982;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void GlKYQCfZsB17670372() {     double IzPvvHbWby29529940 = -242589141;    double IzPvvHbWby79359263 = -552781668;    double IzPvvHbWby83492673 = -306917159;    double IzPvvHbWby10985447 = -635280213;    double IzPvvHbWby84480188 = -288508081;    double IzPvvHbWby39555426 = -982162053;    double IzPvvHbWby94752640 = -902719788;    double IzPvvHbWby60458810 = -205925288;    double IzPvvHbWby97994075 = -608890111;    double IzPvvHbWby64634540 = -316982001;    double IzPvvHbWby41995269 = -838077497;    double IzPvvHbWby39613881 = -988872034;    double IzPvvHbWby76145620 = -336259715;    double IzPvvHbWby55983250 = -930030627;    double IzPvvHbWby96103101 = -196538603;    double IzPvvHbWby5347960 = -75322236;    double IzPvvHbWby8126517 = -273382699;    double IzPvvHbWby95596178 = -738620434;    double IzPvvHbWby91501905 = -950527464;    double IzPvvHbWby2116057 = -34435369;    double IzPvvHbWby10773595 = 40290912;    double IzPvvHbWby11690313 = -240228212;    double IzPvvHbWby65635709 = -743333159;    double IzPvvHbWby38945717 = -613360535;    double IzPvvHbWby88293768 = -108917016;    double IzPvvHbWby99192775 = 57950198;    double IzPvvHbWby2573838 = 61828418;    double IzPvvHbWby46773206 = -952570285;    double IzPvvHbWby14933631 = -134550409;    double IzPvvHbWby43050276 = -123399643;    double IzPvvHbWby71504936 = -732039469;    double IzPvvHbWby18105378 = -455584854;    double IzPvvHbWby36506472 = -417904893;    double IzPvvHbWby29854524 = -413061616;    double IzPvvHbWby69336917 = -477771765;    double IzPvvHbWby64409556 = -212518193;    double IzPvvHbWby76858812 = -10963135;    double IzPvvHbWby17144170 = -463563309;    double IzPvvHbWby38285899 = -372094518;    double IzPvvHbWby74903129 = -462928682;    double IzPvvHbWby35202275 = 61580804;    double IzPvvHbWby65969362 = -371576812;    double IzPvvHbWby8570914 = 83299817;    double IzPvvHbWby52923788 = -330482785;    double IzPvvHbWby54738787 = -595452307;    double IzPvvHbWby71962941 = -453581949;    double IzPvvHbWby86947308 = -208309864;    double IzPvvHbWby49653864 = -30991975;    double IzPvvHbWby64849890 = -120478046;    double IzPvvHbWby93170230 = -407540818;    double IzPvvHbWby92603978 = -767166061;    double IzPvvHbWby66153934 = 63506454;    double IzPvvHbWby88787386 = -459574292;    double IzPvvHbWby2469227 = -207828609;    double IzPvvHbWby6896778 = -600953067;    double IzPvvHbWby17839627 = 97639071;    double IzPvvHbWby13723555 = -809448509;    double IzPvvHbWby44546956 = -693556624;    double IzPvvHbWby22691679 = -426363197;    double IzPvvHbWby85287412 = -246458279;    double IzPvvHbWby36981588 = -943990471;    double IzPvvHbWby47979435 = -950149503;    double IzPvvHbWby45525179 = 28625120;    double IzPvvHbWby54943800 = -385490468;    double IzPvvHbWby93129604 = -584942533;    double IzPvvHbWby23889892 = -282492644;    double IzPvvHbWby3107409 = -470967141;    double IzPvvHbWby46291096 = -923198100;    double IzPvvHbWby86646333 = -352258862;    double IzPvvHbWby31693546 = -984020410;    double IzPvvHbWby28489147 = 35640898;    double IzPvvHbWby90982347 = -809819390;    double IzPvvHbWby57310279 = -266525917;    double IzPvvHbWby16598777 = -387598782;    double IzPvvHbWby66913782 = 3983827;    double IzPvvHbWby44804232 = -588132276;    double IzPvvHbWby3119400 = -223528030;    double IzPvvHbWby12711921 = -312850375;    double IzPvvHbWby84206930 = 82091772;    double IzPvvHbWby16330827 = -655335067;    double IzPvvHbWby12245468 = -733739939;    double IzPvvHbWby52919973 = -907179608;    double IzPvvHbWby81923316 = -732092240;    double IzPvvHbWby21763400 = -727009592;    double IzPvvHbWby50446297 = -356233583;    double IzPvvHbWby5351002 = -695545924;    double IzPvvHbWby29317991 = -996010562;    double IzPvvHbWby34037246 = -110076285;    double IzPvvHbWby22957747 = -812108549;    double IzPvvHbWby51497291 = -475410837;    double IzPvvHbWby50686002 = -403069685;    double IzPvvHbWby32311857 = -317406511;    double IzPvvHbWby94452490 = 62799888;    double IzPvvHbWby52998487 = -25636239;    double IzPvvHbWby37921541 = -518938212;    double IzPvvHbWby87222840 = 11730306;    double IzPvvHbWby20444184 = -300201933;    double IzPvvHbWby53627113 = -531209715;    double IzPvvHbWby59794184 = -745540253;    double IzPvvHbWby30848895 = -242589141;     IzPvvHbWby29529940 = IzPvvHbWby79359263;     IzPvvHbWby79359263 = IzPvvHbWby83492673;     IzPvvHbWby83492673 = IzPvvHbWby10985447;     IzPvvHbWby10985447 = IzPvvHbWby84480188;     IzPvvHbWby84480188 = IzPvvHbWby39555426;     IzPvvHbWby39555426 = IzPvvHbWby94752640;     IzPvvHbWby94752640 = IzPvvHbWby60458810;     IzPvvHbWby60458810 = IzPvvHbWby97994075;     IzPvvHbWby97994075 = IzPvvHbWby64634540;     IzPvvHbWby64634540 = IzPvvHbWby41995269;     IzPvvHbWby41995269 = IzPvvHbWby39613881;     IzPvvHbWby39613881 = IzPvvHbWby76145620;     IzPvvHbWby76145620 = IzPvvHbWby55983250;     IzPvvHbWby55983250 = IzPvvHbWby96103101;     IzPvvHbWby96103101 = IzPvvHbWby5347960;     IzPvvHbWby5347960 = IzPvvHbWby8126517;     IzPvvHbWby8126517 = IzPvvHbWby95596178;     IzPvvHbWby95596178 = IzPvvHbWby91501905;     IzPvvHbWby91501905 = IzPvvHbWby2116057;     IzPvvHbWby2116057 = IzPvvHbWby10773595;     IzPvvHbWby10773595 = IzPvvHbWby11690313;     IzPvvHbWby11690313 = IzPvvHbWby65635709;     IzPvvHbWby65635709 = IzPvvHbWby38945717;     IzPvvHbWby38945717 = IzPvvHbWby88293768;     IzPvvHbWby88293768 = IzPvvHbWby99192775;     IzPvvHbWby99192775 = IzPvvHbWby2573838;     IzPvvHbWby2573838 = IzPvvHbWby46773206;     IzPvvHbWby46773206 = IzPvvHbWby14933631;     IzPvvHbWby14933631 = IzPvvHbWby43050276;     IzPvvHbWby43050276 = IzPvvHbWby71504936;     IzPvvHbWby71504936 = IzPvvHbWby18105378;     IzPvvHbWby18105378 = IzPvvHbWby36506472;     IzPvvHbWby36506472 = IzPvvHbWby29854524;     IzPvvHbWby29854524 = IzPvvHbWby69336917;     IzPvvHbWby69336917 = IzPvvHbWby64409556;     IzPvvHbWby64409556 = IzPvvHbWby76858812;     IzPvvHbWby76858812 = IzPvvHbWby17144170;     IzPvvHbWby17144170 = IzPvvHbWby38285899;     IzPvvHbWby38285899 = IzPvvHbWby74903129;     IzPvvHbWby74903129 = IzPvvHbWby35202275;     IzPvvHbWby35202275 = IzPvvHbWby65969362;     IzPvvHbWby65969362 = IzPvvHbWby8570914;     IzPvvHbWby8570914 = IzPvvHbWby52923788;     IzPvvHbWby52923788 = IzPvvHbWby54738787;     IzPvvHbWby54738787 = IzPvvHbWby71962941;     IzPvvHbWby71962941 = IzPvvHbWby86947308;     IzPvvHbWby86947308 = IzPvvHbWby49653864;     IzPvvHbWby49653864 = IzPvvHbWby64849890;     IzPvvHbWby64849890 = IzPvvHbWby93170230;     IzPvvHbWby93170230 = IzPvvHbWby92603978;     IzPvvHbWby92603978 = IzPvvHbWby66153934;     IzPvvHbWby66153934 = IzPvvHbWby88787386;     IzPvvHbWby88787386 = IzPvvHbWby2469227;     IzPvvHbWby2469227 = IzPvvHbWby6896778;     IzPvvHbWby6896778 = IzPvvHbWby17839627;     IzPvvHbWby17839627 = IzPvvHbWby13723555;     IzPvvHbWby13723555 = IzPvvHbWby44546956;     IzPvvHbWby44546956 = IzPvvHbWby22691679;     IzPvvHbWby22691679 = IzPvvHbWby85287412;     IzPvvHbWby85287412 = IzPvvHbWby36981588;     IzPvvHbWby36981588 = IzPvvHbWby47979435;     IzPvvHbWby47979435 = IzPvvHbWby45525179;     IzPvvHbWby45525179 = IzPvvHbWby54943800;     IzPvvHbWby54943800 = IzPvvHbWby93129604;     IzPvvHbWby93129604 = IzPvvHbWby23889892;     IzPvvHbWby23889892 = IzPvvHbWby3107409;     IzPvvHbWby3107409 = IzPvvHbWby46291096;     IzPvvHbWby46291096 = IzPvvHbWby86646333;     IzPvvHbWby86646333 = IzPvvHbWby31693546;     IzPvvHbWby31693546 = IzPvvHbWby28489147;     IzPvvHbWby28489147 = IzPvvHbWby90982347;     IzPvvHbWby90982347 = IzPvvHbWby57310279;     IzPvvHbWby57310279 = IzPvvHbWby16598777;     IzPvvHbWby16598777 = IzPvvHbWby66913782;     IzPvvHbWby66913782 = IzPvvHbWby44804232;     IzPvvHbWby44804232 = IzPvvHbWby3119400;     IzPvvHbWby3119400 = IzPvvHbWby12711921;     IzPvvHbWby12711921 = IzPvvHbWby84206930;     IzPvvHbWby84206930 = IzPvvHbWby16330827;     IzPvvHbWby16330827 = IzPvvHbWby12245468;     IzPvvHbWby12245468 = IzPvvHbWby52919973;     IzPvvHbWby52919973 = IzPvvHbWby81923316;     IzPvvHbWby81923316 = IzPvvHbWby21763400;     IzPvvHbWby21763400 = IzPvvHbWby50446297;     IzPvvHbWby50446297 = IzPvvHbWby5351002;     IzPvvHbWby5351002 = IzPvvHbWby29317991;     IzPvvHbWby29317991 = IzPvvHbWby34037246;     IzPvvHbWby34037246 = IzPvvHbWby22957747;     IzPvvHbWby22957747 = IzPvvHbWby51497291;     IzPvvHbWby51497291 = IzPvvHbWby50686002;     IzPvvHbWby50686002 = IzPvvHbWby32311857;     IzPvvHbWby32311857 = IzPvvHbWby94452490;     IzPvvHbWby94452490 = IzPvvHbWby52998487;     IzPvvHbWby52998487 = IzPvvHbWby37921541;     IzPvvHbWby37921541 = IzPvvHbWby87222840;     IzPvvHbWby87222840 = IzPvvHbWby20444184;     IzPvvHbWby20444184 = IzPvvHbWby53627113;     IzPvvHbWby53627113 = IzPvvHbWby59794184;     IzPvvHbWby59794184 = IzPvvHbWby30848895;     IzPvvHbWby30848895 = IzPvvHbWby29529940;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void zuBCIBESna33049637() {     double dsjImXRjRn80986177 = -889487588;    double dsjImXRjRn14307241 = -825411759;    double dsjImXRjRn40115422 = -29494080;    double dsjImXRjRn51705147 = -640467263;    double dsjImXRjRn87764108 = -505078034;    double dsjImXRjRn51603663 = -816940264;    double dsjImXRjRn2688239 = -444328559;    double dsjImXRjRn50257648 = -796408286;    double dsjImXRjRn57602325 = -43715476;    double dsjImXRjRn19871728 = -3963080;    double dsjImXRjRn43814870 = -49630584;    double dsjImXRjRn22963888 = -982332510;    double dsjImXRjRn30390338 = -994268974;    double dsjImXRjRn4717 = 4681331;    double dsjImXRjRn6793564 = -942591989;    double dsjImXRjRn31650365 = -750957866;    double dsjImXRjRn57983516 = -235942553;    double dsjImXRjRn48053277 = -737650231;    double dsjImXRjRn72539416 = -588467280;    double dsjImXRjRn37105429 = -657996593;    double dsjImXRjRn44821824 = -57579767;    double dsjImXRjRn25255013 = -491705704;    double dsjImXRjRn1635067 = -362065067;    double dsjImXRjRn65159830 = 82454771;    double dsjImXRjRn64683492 = -535893450;    double dsjImXRjRn99247580 = -825202269;    double dsjImXRjRn69265273 = -684789220;    double dsjImXRjRn42034856 = -519414367;    double dsjImXRjRn53846656 = -622758553;    double dsjImXRjRn93320373 = -425543916;    double dsjImXRjRn45454636 = -483581116;    double dsjImXRjRn69448246 = -145356903;    double dsjImXRjRn64013967 = 44055135;    double dsjImXRjRn30312033 = -361164304;    double dsjImXRjRn78665450 = -57955536;    double dsjImXRjRn88637116 = -258314450;    double dsjImXRjRn95357627 = -213825762;    double dsjImXRjRn8174823 = -356402977;    double dsjImXRjRn96440100 = -696750829;    double dsjImXRjRn10291565 = -887490992;    double dsjImXRjRn72767765 = 20913169;    double dsjImXRjRn47142822 = -227979086;    double dsjImXRjRn55921069 = -716129703;    double dsjImXRjRn90569027 = -433110501;    double dsjImXRjRn90725241 = -598092266;    double dsjImXRjRn68996503 = -182053122;    double dsjImXRjRn15949963 = -781308640;    double dsjImXRjRn9567077 = -671996491;    double dsjImXRjRn50841242 = -970248706;    double dsjImXRjRn81285644 = -941557218;    double dsjImXRjRn66450487 = -530799237;    double dsjImXRjRn2128762 = -704299631;    double dsjImXRjRn80756189 = -403502431;    double dsjImXRjRn51272620 = -64332120;    double dsjImXRjRn88227813 = -662475446;    double dsjImXRjRn55731164 = -297781884;    double dsjImXRjRn12672174 = -363346693;    double dsjImXRjRn74955592 = -11948852;    double dsjImXRjRn87021655 = -4573813;    double dsjImXRjRn88516527 = -679875766;    double dsjImXRjRn82338390 = -32151045;    double dsjImXRjRn60653383 = -924914193;    double dsjImXRjRn96410991 = -73649733;    double dsjImXRjRn64281951 = -618171560;    double dsjImXRjRn74417092 = -520381965;    double dsjImXRjRn74366624 = -904273681;    double dsjImXRjRn58949920 = -926387645;    double dsjImXRjRn78305 = -533104671;    double dsjImXRjRn21339266 = -937363133;    double dsjImXRjRn18156448 = -584277540;    double dsjImXRjRn36292738 = -437132104;    double dsjImXRjRn49808693 = -879539576;    double dsjImXRjRn51613176 = 59100598;    double dsjImXRjRn62247851 = -700976289;    double dsjImXRjRn64337663 = -578909763;    double dsjImXRjRn97679002 = -829600681;    double dsjImXRjRn69333944 = -775576002;    double dsjImXRjRn11066039 = -928954567;    double dsjImXRjRn74434589 = -319452964;    double dsjImXRjRn95686988 = -253840329;    double dsjImXRjRn83297617 = 56106371;    double dsjImXRjRn59698196 = 87207271;    double dsjImXRjRn91193614 = -549165661;    double dsjImXRjRn72561012 = -681201336;    double dsjImXRjRn26869886 = -894744679;    double dsjImXRjRn43325874 = -779281485;    double dsjImXRjRn88692057 = -741854472;    double dsjImXRjRn12741348 = -891612745;    double dsjImXRjRn42084220 = -698688858;    double dsjImXRjRn22934287 = -760173653;    double dsjImXRjRn75964942 = -894967758;    double dsjImXRjRn20402035 = -101876911;    double dsjImXRjRn21153168 = -251829165;    double dsjImXRjRn7923574 = 83124936;    double dsjImXRjRn27953175 = -755339947;    double dsjImXRjRn12114383 = -54172638;    double dsjImXRjRn50731831 = -54329353;    double dsjImXRjRn91639117 = 2041857;    double dsjImXRjRn16151936 = -912728536;    double dsjImXRjRn16358617 = -889487588;     dsjImXRjRn80986177 = dsjImXRjRn14307241;     dsjImXRjRn14307241 = dsjImXRjRn40115422;     dsjImXRjRn40115422 = dsjImXRjRn51705147;     dsjImXRjRn51705147 = dsjImXRjRn87764108;     dsjImXRjRn87764108 = dsjImXRjRn51603663;     dsjImXRjRn51603663 = dsjImXRjRn2688239;     dsjImXRjRn2688239 = dsjImXRjRn50257648;     dsjImXRjRn50257648 = dsjImXRjRn57602325;     dsjImXRjRn57602325 = dsjImXRjRn19871728;     dsjImXRjRn19871728 = dsjImXRjRn43814870;     dsjImXRjRn43814870 = dsjImXRjRn22963888;     dsjImXRjRn22963888 = dsjImXRjRn30390338;     dsjImXRjRn30390338 = dsjImXRjRn4717;     dsjImXRjRn4717 = dsjImXRjRn6793564;     dsjImXRjRn6793564 = dsjImXRjRn31650365;     dsjImXRjRn31650365 = dsjImXRjRn57983516;     dsjImXRjRn57983516 = dsjImXRjRn48053277;     dsjImXRjRn48053277 = dsjImXRjRn72539416;     dsjImXRjRn72539416 = dsjImXRjRn37105429;     dsjImXRjRn37105429 = dsjImXRjRn44821824;     dsjImXRjRn44821824 = dsjImXRjRn25255013;     dsjImXRjRn25255013 = dsjImXRjRn1635067;     dsjImXRjRn1635067 = dsjImXRjRn65159830;     dsjImXRjRn65159830 = dsjImXRjRn64683492;     dsjImXRjRn64683492 = dsjImXRjRn99247580;     dsjImXRjRn99247580 = dsjImXRjRn69265273;     dsjImXRjRn69265273 = dsjImXRjRn42034856;     dsjImXRjRn42034856 = dsjImXRjRn53846656;     dsjImXRjRn53846656 = dsjImXRjRn93320373;     dsjImXRjRn93320373 = dsjImXRjRn45454636;     dsjImXRjRn45454636 = dsjImXRjRn69448246;     dsjImXRjRn69448246 = dsjImXRjRn64013967;     dsjImXRjRn64013967 = dsjImXRjRn30312033;     dsjImXRjRn30312033 = dsjImXRjRn78665450;     dsjImXRjRn78665450 = dsjImXRjRn88637116;     dsjImXRjRn88637116 = dsjImXRjRn95357627;     dsjImXRjRn95357627 = dsjImXRjRn8174823;     dsjImXRjRn8174823 = dsjImXRjRn96440100;     dsjImXRjRn96440100 = dsjImXRjRn10291565;     dsjImXRjRn10291565 = dsjImXRjRn72767765;     dsjImXRjRn72767765 = dsjImXRjRn47142822;     dsjImXRjRn47142822 = dsjImXRjRn55921069;     dsjImXRjRn55921069 = dsjImXRjRn90569027;     dsjImXRjRn90569027 = dsjImXRjRn90725241;     dsjImXRjRn90725241 = dsjImXRjRn68996503;     dsjImXRjRn68996503 = dsjImXRjRn15949963;     dsjImXRjRn15949963 = dsjImXRjRn9567077;     dsjImXRjRn9567077 = dsjImXRjRn50841242;     dsjImXRjRn50841242 = dsjImXRjRn81285644;     dsjImXRjRn81285644 = dsjImXRjRn66450487;     dsjImXRjRn66450487 = dsjImXRjRn2128762;     dsjImXRjRn2128762 = dsjImXRjRn80756189;     dsjImXRjRn80756189 = dsjImXRjRn51272620;     dsjImXRjRn51272620 = dsjImXRjRn88227813;     dsjImXRjRn88227813 = dsjImXRjRn55731164;     dsjImXRjRn55731164 = dsjImXRjRn12672174;     dsjImXRjRn12672174 = dsjImXRjRn74955592;     dsjImXRjRn74955592 = dsjImXRjRn87021655;     dsjImXRjRn87021655 = dsjImXRjRn88516527;     dsjImXRjRn88516527 = dsjImXRjRn82338390;     dsjImXRjRn82338390 = dsjImXRjRn60653383;     dsjImXRjRn60653383 = dsjImXRjRn96410991;     dsjImXRjRn96410991 = dsjImXRjRn64281951;     dsjImXRjRn64281951 = dsjImXRjRn74417092;     dsjImXRjRn74417092 = dsjImXRjRn74366624;     dsjImXRjRn74366624 = dsjImXRjRn58949920;     dsjImXRjRn58949920 = dsjImXRjRn78305;     dsjImXRjRn78305 = dsjImXRjRn21339266;     dsjImXRjRn21339266 = dsjImXRjRn18156448;     dsjImXRjRn18156448 = dsjImXRjRn36292738;     dsjImXRjRn36292738 = dsjImXRjRn49808693;     dsjImXRjRn49808693 = dsjImXRjRn51613176;     dsjImXRjRn51613176 = dsjImXRjRn62247851;     dsjImXRjRn62247851 = dsjImXRjRn64337663;     dsjImXRjRn64337663 = dsjImXRjRn97679002;     dsjImXRjRn97679002 = dsjImXRjRn69333944;     dsjImXRjRn69333944 = dsjImXRjRn11066039;     dsjImXRjRn11066039 = dsjImXRjRn74434589;     dsjImXRjRn74434589 = dsjImXRjRn95686988;     dsjImXRjRn95686988 = dsjImXRjRn83297617;     dsjImXRjRn83297617 = dsjImXRjRn59698196;     dsjImXRjRn59698196 = dsjImXRjRn91193614;     dsjImXRjRn91193614 = dsjImXRjRn72561012;     dsjImXRjRn72561012 = dsjImXRjRn26869886;     dsjImXRjRn26869886 = dsjImXRjRn43325874;     dsjImXRjRn43325874 = dsjImXRjRn88692057;     dsjImXRjRn88692057 = dsjImXRjRn12741348;     dsjImXRjRn12741348 = dsjImXRjRn42084220;     dsjImXRjRn42084220 = dsjImXRjRn22934287;     dsjImXRjRn22934287 = dsjImXRjRn75964942;     dsjImXRjRn75964942 = dsjImXRjRn20402035;     dsjImXRjRn20402035 = dsjImXRjRn21153168;     dsjImXRjRn21153168 = dsjImXRjRn7923574;     dsjImXRjRn7923574 = dsjImXRjRn27953175;     dsjImXRjRn27953175 = dsjImXRjRn12114383;     dsjImXRjRn12114383 = dsjImXRjRn50731831;     dsjImXRjRn50731831 = dsjImXRjRn91639117;     dsjImXRjRn91639117 = dsjImXRjRn16151936;     dsjImXRjRn16151936 = dsjImXRjRn16358617;     dsjImXRjRn16358617 = dsjImXRjRn80986177;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void tWUXTpXktz39378185() {     double VRkiSkcoEE35640575 = -227015171;    double VRkiSkcoEE61304860 = -304976573;    double VRkiSkcoEE848069 = -414878694;    double VRkiSkcoEE23140089 = -646569674;    double VRkiSkcoEE62215779 = -177513274;    double VRkiSkcoEE65778060 = -946091100;    double VRkiSkcoEE23788943 = -746221232;    double VRkiSkcoEE79432751 = -196976519;    double VRkiSkcoEE45376735 = -90568847;    double VRkiSkcoEE2503714 = -282764350;    double VRkiSkcoEE22426165 = -674987156;    double VRkiSkcoEE85728600 = -68756598;    double VRkiSkcoEE94207652 = -797809279;    double VRkiSkcoEE92971147 = -707422247;    double VRkiSkcoEE37017637 = -849713621;    double VRkiSkcoEE21417901 = -381117430;    double VRkiSkcoEE57815279 = -709542381;    double VRkiSkcoEE15649864 = -801214699;    double VRkiSkcoEE73760017 = -874278828;    double VRkiSkcoEE54739984 = 96637261;    double VRkiSkcoEE78996212 = -625662917;    double VRkiSkcoEE76507602 = -658149812;    double VRkiSkcoEE38104898 = -948808488;    double VRkiSkcoEE66588199 = -716586045;    double VRkiSkcoEE60436108 = -908806902;    double VRkiSkcoEE40488528 = -828911054;    double VRkiSkcoEE71255197 = -139633498;    double VRkiSkcoEE95283856 = -527466227;    double VRkiSkcoEE93744333 = 96996571;    double VRkiSkcoEE87755782 = -716301885;    double VRkiSkcoEE97160164 = 67546358;    double VRkiSkcoEE439857 = -556853431;    double VRkiSkcoEE31669844 = -512462479;    double VRkiSkcoEE24967926 = -41285114;    double VRkiSkcoEE13169608 = -275818796;    double VRkiSkcoEE75963658 = -506310047;    double VRkiSkcoEE70062115 = -64252383;    double VRkiSkcoEE21152062 = -230331998;    double VRkiSkcoEE47209750 = 21300569;    double VRkiSkcoEE34277960 = -545799591;    double VRkiSkcoEE28727166 = -91636989;    double VRkiSkcoEE83817480 = -835511173;    double VRkiSkcoEE46921252 = -556635021;    double VRkiSkcoEE99563427 = 93209833;    double VRkiSkcoEE50709304 = -730609863;    double VRkiSkcoEE24330105 = -250842736;    double VRkiSkcoEE32423675 = -808366024;    double VRkiSkcoEE32994386 = -649648864;    double VRkiSkcoEE93184008 = -869978895;    double VRkiSkcoEE26127307 = -81576512;    double VRkiSkcoEE41564027 = -188014739;    double VRkiSkcoEE91510911 = -119365614;    double VRkiSkcoEE12484192 = -143417888;    double VRkiSkcoEE8688378 = 39781395;    double VRkiSkcoEE60381972 = 41615872;    double VRkiSkcoEE59132973 = -568865360;    double VRkiSkcoEE23199962 = -356168086;    double VRkiSkcoEE34259870 = -698292649;    double VRkiSkcoEE62703981 = -737762772;    double VRkiSkcoEE21727251 = -348602220;    double VRkiSkcoEE94522863 = -706457602;    double VRkiSkcoEE28505086 = -118755005;    double VRkiSkcoEE85688418 = -193973091;    double VRkiSkcoEE57620953 = -374266963;    double VRkiSkcoEE5343550 = -250310709;    double VRkiSkcoEE21986309 = -18133725;    double VRkiSkcoEE54058757 = -556294120;    double VRkiSkcoEE69239727 = -656524165;    double VRkiSkcoEE79801539 = -331603451;    double VRkiSkcoEE61053978 = -243403575;    double VRkiSkcoEE51355786 = -216865048;    double VRkiSkcoEE36663218 = -379210384;    double VRkiSkcoEE68440114 = -722515268;    double VRkiSkcoEE39482057 = -228479237;    double VRkiSkcoEE26012818 = -811725751;    double VRkiSkcoEE95178731 = -790151745;    double VRkiSkcoEE29586351 = -1514792;    double VRkiSkcoEE38541471 = -942018322;    double VRkiSkcoEE15878895 = -985976182;    double VRkiSkcoEE36106003 = -557964166;    double VRkiSkcoEE8064853 = 79454970;    double VRkiSkcoEE38260811 = -489984635;    double VRkiSkcoEE2099849 = -657487333;    double VRkiSkcoEE67617027 = -821426917;    double VRkiSkcoEE46191756 = -428287146;    double VRkiSkcoEE5649253 = -813088028;    double VRkiSkcoEE87955664 = -313435543;    double VRkiSkcoEE22981467 = -452243875;    double VRkiSkcoEE64585953 = 17099013;    double VRkiSkcoEE54036635 = -706953436;    double VRkiSkcoEE52763697 = -50141962;    double VRkiSkcoEE35802245 = -365959734;    double VRkiSkcoEE58448081 = -492569227;    double VRkiSkcoEE25482499 = -630097211;    double VRkiSkcoEE39755096 = -839341989;    double VRkiSkcoEE222081 = -972881984;    double VRkiSkcoEE98129062 = -541538083;    double VRkiSkcoEE89300298 = -82368059;    double VRkiSkcoEE94219877 = -656479458;    double VRkiSkcoEE28722996 = -227015171;     VRkiSkcoEE35640575 = VRkiSkcoEE61304860;     VRkiSkcoEE61304860 = VRkiSkcoEE848069;     VRkiSkcoEE848069 = VRkiSkcoEE23140089;     VRkiSkcoEE23140089 = VRkiSkcoEE62215779;     VRkiSkcoEE62215779 = VRkiSkcoEE65778060;     VRkiSkcoEE65778060 = VRkiSkcoEE23788943;     VRkiSkcoEE23788943 = VRkiSkcoEE79432751;     VRkiSkcoEE79432751 = VRkiSkcoEE45376735;     VRkiSkcoEE45376735 = VRkiSkcoEE2503714;     VRkiSkcoEE2503714 = VRkiSkcoEE22426165;     VRkiSkcoEE22426165 = VRkiSkcoEE85728600;     VRkiSkcoEE85728600 = VRkiSkcoEE94207652;     VRkiSkcoEE94207652 = VRkiSkcoEE92971147;     VRkiSkcoEE92971147 = VRkiSkcoEE37017637;     VRkiSkcoEE37017637 = VRkiSkcoEE21417901;     VRkiSkcoEE21417901 = VRkiSkcoEE57815279;     VRkiSkcoEE57815279 = VRkiSkcoEE15649864;     VRkiSkcoEE15649864 = VRkiSkcoEE73760017;     VRkiSkcoEE73760017 = VRkiSkcoEE54739984;     VRkiSkcoEE54739984 = VRkiSkcoEE78996212;     VRkiSkcoEE78996212 = VRkiSkcoEE76507602;     VRkiSkcoEE76507602 = VRkiSkcoEE38104898;     VRkiSkcoEE38104898 = VRkiSkcoEE66588199;     VRkiSkcoEE66588199 = VRkiSkcoEE60436108;     VRkiSkcoEE60436108 = VRkiSkcoEE40488528;     VRkiSkcoEE40488528 = VRkiSkcoEE71255197;     VRkiSkcoEE71255197 = VRkiSkcoEE95283856;     VRkiSkcoEE95283856 = VRkiSkcoEE93744333;     VRkiSkcoEE93744333 = VRkiSkcoEE87755782;     VRkiSkcoEE87755782 = VRkiSkcoEE97160164;     VRkiSkcoEE97160164 = VRkiSkcoEE439857;     VRkiSkcoEE439857 = VRkiSkcoEE31669844;     VRkiSkcoEE31669844 = VRkiSkcoEE24967926;     VRkiSkcoEE24967926 = VRkiSkcoEE13169608;     VRkiSkcoEE13169608 = VRkiSkcoEE75963658;     VRkiSkcoEE75963658 = VRkiSkcoEE70062115;     VRkiSkcoEE70062115 = VRkiSkcoEE21152062;     VRkiSkcoEE21152062 = VRkiSkcoEE47209750;     VRkiSkcoEE47209750 = VRkiSkcoEE34277960;     VRkiSkcoEE34277960 = VRkiSkcoEE28727166;     VRkiSkcoEE28727166 = VRkiSkcoEE83817480;     VRkiSkcoEE83817480 = VRkiSkcoEE46921252;     VRkiSkcoEE46921252 = VRkiSkcoEE99563427;     VRkiSkcoEE99563427 = VRkiSkcoEE50709304;     VRkiSkcoEE50709304 = VRkiSkcoEE24330105;     VRkiSkcoEE24330105 = VRkiSkcoEE32423675;     VRkiSkcoEE32423675 = VRkiSkcoEE32994386;     VRkiSkcoEE32994386 = VRkiSkcoEE93184008;     VRkiSkcoEE93184008 = VRkiSkcoEE26127307;     VRkiSkcoEE26127307 = VRkiSkcoEE41564027;     VRkiSkcoEE41564027 = VRkiSkcoEE91510911;     VRkiSkcoEE91510911 = VRkiSkcoEE12484192;     VRkiSkcoEE12484192 = VRkiSkcoEE8688378;     VRkiSkcoEE8688378 = VRkiSkcoEE60381972;     VRkiSkcoEE60381972 = VRkiSkcoEE59132973;     VRkiSkcoEE59132973 = VRkiSkcoEE23199962;     VRkiSkcoEE23199962 = VRkiSkcoEE34259870;     VRkiSkcoEE34259870 = VRkiSkcoEE62703981;     VRkiSkcoEE62703981 = VRkiSkcoEE21727251;     VRkiSkcoEE21727251 = VRkiSkcoEE94522863;     VRkiSkcoEE94522863 = VRkiSkcoEE28505086;     VRkiSkcoEE28505086 = VRkiSkcoEE85688418;     VRkiSkcoEE85688418 = VRkiSkcoEE57620953;     VRkiSkcoEE57620953 = VRkiSkcoEE5343550;     VRkiSkcoEE5343550 = VRkiSkcoEE21986309;     VRkiSkcoEE21986309 = VRkiSkcoEE54058757;     VRkiSkcoEE54058757 = VRkiSkcoEE69239727;     VRkiSkcoEE69239727 = VRkiSkcoEE79801539;     VRkiSkcoEE79801539 = VRkiSkcoEE61053978;     VRkiSkcoEE61053978 = VRkiSkcoEE51355786;     VRkiSkcoEE51355786 = VRkiSkcoEE36663218;     VRkiSkcoEE36663218 = VRkiSkcoEE68440114;     VRkiSkcoEE68440114 = VRkiSkcoEE39482057;     VRkiSkcoEE39482057 = VRkiSkcoEE26012818;     VRkiSkcoEE26012818 = VRkiSkcoEE95178731;     VRkiSkcoEE95178731 = VRkiSkcoEE29586351;     VRkiSkcoEE29586351 = VRkiSkcoEE38541471;     VRkiSkcoEE38541471 = VRkiSkcoEE15878895;     VRkiSkcoEE15878895 = VRkiSkcoEE36106003;     VRkiSkcoEE36106003 = VRkiSkcoEE8064853;     VRkiSkcoEE8064853 = VRkiSkcoEE38260811;     VRkiSkcoEE38260811 = VRkiSkcoEE2099849;     VRkiSkcoEE2099849 = VRkiSkcoEE67617027;     VRkiSkcoEE67617027 = VRkiSkcoEE46191756;     VRkiSkcoEE46191756 = VRkiSkcoEE5649253;     VRkiSkcoEE5649253 = VRkiSkcoEE87955664;     VRkiSkcoEE87955664 = VRkiSkcoEE22981467;     VRkiSkcoEE22981467 = VRkiSkcoEE64585953;     VRkiSkcoEE64585953 = VRkiSkcoEE54036635;     VRkiSkcoEE54036635 = VRkiSkcoEE52763697;     VRkiSkcoEE52763697 = VRkiSkcoEE35802245;     VRkiSkcoEE35802245 = VRkiSkcoEE58448081;     VRkiSkcoEE58448081 = VRkiSkcoEE25482499;     VRkiSkcoEE25482499 = VRkiSkcoEE39755096;     VRkiSkcoEE39755096 = VRkiSkcoEE222081;     VRkiSkcoEE222081 = VRkiSkcoEE98129062;     VRkiSkcoEE98129062 = VRkiSkcoEE89300298;     VRkiSkcoEE89300298 = VRkiSkcoEE94219877;     VRkiSkcoEE94219877 = VRkiSkcoEE28722996;     VRkiSkcoEE28722996 = VRkiSkcoEE35640575;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void YCYJQzauhE54757450() {     double nqIquILndE87096812 = -873913618;    double nqIquILndE96252836 = -577606665;    double nqIquILndE57470818 = -137455615;    double nqIquILndE63859788 = -651756723;    double nqIquILndE65499699 = -394083227;    double nqIquILndE77826298 = -780869311;    double nqIquILndE31724541 = -287830003;    double nqIquILndE69231589 = -787459516;    double nqIquILndE4984985 = -625394212;    double nqIquILndE57740901 = 30254571;    double nqIquILndE24245766 = -986540242;    double nqIquILndE69078607 = -62217074;    double nqIquILndE48452370 = -355818537;    double nqIquILndE36992613 = -872710288;    double nqIquILndE47708100 = -495767008;    double nqIquILndE47720306 = 43246940;    double nqIquILndE7672279 = -672102236;    double nqIquILndE68106963 = -800244496;    double nqIquILndE54797527 = -512218643;    double nqIquILndE89729356 = -526923964;    double nqIquILndE13044442 = -723533596;    double nqIquILndE90072302 = -909627304;    double nqIquILndE74104254 = -567540396;    double nqIquILndE92802312 = -20770739;    double nqIquILndE36825832 = -235783336;    double nqIquILndE40543333 = -612063521;    double nqIquILndE37946633 = -886251135;    double nqIquILndE90545506 = -94310309;    double nqIquILndE32657359 = -391211573;    double nqIquILndE38025880 = 81553843;    double nqIquILndE71109863 = -783995288;    double nqIquILndE51782725 = -246625480;    double nqIquILndE59177339 = -50502451;    double nqIquILndE25425435 = 10612198;    double nqIquILndE22498141 = -956002566;    double nqIquILndE191220 = -552106304;    double nqIquILndE88560929 = -267115010;    double nqIquILndE12182715 = -123171666;    double nqIquILndE5363952 = -303355742;    double nqIquILndE69666395 = -970361900;    double nqIquILndE66292657 = -132304623;    double nqIquILndE64990940 = -691913446;    double nqIquILndE94271407 = -256064541;    double nqIquILndE37208667 = -9417882;    double nqIquILndE86695758 = -733249821;    double nqIquILndE21363666 = 20686091;    double nqIquILndE61426330 = -281364800;    double nqIquILndE92907598 = -190653380;    double nqIquILndE79175360 = -619749556;    double nqIquILndE14242720 = -615592912;    double nqIquILndE15410536 = 48352084;    double nqIquILndE27485739 = -887171700;    double nqIquILndE4452995 = -87346027;    double nqIquILndE57491771 = -916722117;    double nqIquILndE41713008 = -19906507;    double nqIquILndE97024509 = -964286315;    double nqIquILndE22148582 = 89933730;    double nqIquILndE64668505 = -16684876;    double nqIquILndE27033957 = -315973387;    double nqIquILndE24956366 = -782019707;    double nqIquILndE39879665 = -894618176;    double nqIquILndE41179034 = -93519695;    double nqIquILndE36574231 = -296247944;    double nqIquILndE66959104 = -606948055;    double nqIquILndE86631037 = -185750141;    double nqIquILndE72463040 = -639914763;    double nqIquILndE9901268 = 88285377;    double nqIquILndE23026936 = -266430736;    double nqIquILndE14494473 = -916707722;    double nqIquILndE47516880 = -943660704;    double nqIquILndE59159377 = -689638050;    double nqIquILndE95489563 = -448930570;    double nqIquILndE62743012 = -396888754;    double nqIquILndE85131132 = -541856743;    double nqIquILndE23436700 = -294619341;    double nqIquILndE48053502 = 68379850;    double nqIquILndE95800895 = -553562763;    double nqIquILndE36895588 = -458122514;    double nqIquILndE6106555 = -287520918;    double nqIquILndE15462166 = -156469428;    double nqIquILndE79117003 = -230698721;    double nqIquILndE45039034 = -595597756;    double nqIquILndE11370147 = -474560754;    double nqIquILndE18414640 = -775618661;    double nqIquILndE22615345 = -966798242;    double nqIquILndE43624125 = -896823589;    double nqIquILndE47329731 = -59279453;    double nqIquILndE1685569 = -133780335;    double nqIquILndE83712426 = -969481296;    double nqIquILndE25473631 = -991716252;    double nqIquILndE78042638 = -542040035;    double nqIquILndE23892424 = -150430134;    double nqIquILndE85148757 = -807198279;    double nqIquILndE80407585 = -521336036;    double nqIquILndE29786730 = 24256275;    double nqIquILndE25113623 = 61215071;    double nqIquILndE28416710 = -295665503;    double nqIquILndE27312303 = -649116487;    double nqIquILndE50577629 = -823667742;    double nqIquILndE14232718 = -873913618;     nqIquILndE87096812 = nqIquILndE96252836;     nqIquILndE96252836 = nqIquILndE57470818;     nqIquILndE57470818 = nqIquILndE63859788;     nqIquILndE63859788 = nqIquILndE65499699;     nqIquILndE65499699 = nqIquILndE77826298;     nqIquILndE77826298 = nqIquILndE31724541;     nqIquILndE31724541 = nqIquILndE69231589;     nqIquILndE69231589 = nqIquILndE4984985;     nqIquILndE4984985 = nqIquILndE57740901;     nqIquILndE57740901 = nqIquILndE24245766;     nqIquILndE24245766 = nqIquILndE69078607;     nqIquILndE69078607 = nqIquILndE48452370;     nqIquILndE48452370 = nqIquILndE36992613;     nqIquILndE36992613 = nqIquILndE47708100;     nqIquILndE47708100 = nqIquILndE47720306;     nqIquILndE47720306 = nqIquILndE7672279;     nqIquILndE7672279 = nqIquILndE68106963;     nqIquILndE68106963 = nqIquILndE54797527;     nqIquILndE54797527 = nqIquILndE89729356;     nqIquILndE89729356 = nqIquILndE13044442;     nqIquILndE13044442 = nqIquILndE90072302;     nqIquILndE90072302 = nqIquILndE74104254;     nqIquILndE74104254 = nqIquILndE92802312;     nqIquILndE92802312 = nqIquILndE36825832;     nqIquILndE36825832 = nqIquILndE40543333;     nqIquILndE40543333 = nqIquILndE37946633;     nqIquILndE37946633 = nqIquILndE90545506;     nqIquILndE90545506 = nqIquILndE32657359;     nqIquILndE32657359 = nqIquILndE38025880;     nqIquILndE38025880 = nqIquILndE71109863;     nqIquILndE71109863 = nqIquILndE51782725;     nqIquILndE51782725 = nqIquILndE59177339;     nqIquILndE59177339 = nqIquILndE25425435;     nqIquILndE25425435 = nqIquILndE22498141;     nqIquILndE22498141 = nqIquILndE191220;     nqIquILndE191220 = nqIquILndE88560929;     nqIquILndE88560929 = nqIquILndE12182715;     nqIquILndE12182715 = nqIquILndE5363952;     nqIquILndE5363952 = nqIquILndE69666395;     nqIquILndE69666395 = nqIquILndE66292657;     nqIquILndE66292657 = nqIquILndE64990940;     nqIquILndE64990940 = nqIquILndE94271407;     nqIquILndE94271407 = nqIquILndE37208667;     nqIquILndE37208667 = nqIquILndE86695758;     nqIquILndE86695758 = nqIquILndE21363666;     nqIquILndE21363666 = nqIquILndE61426330;     nqIquILndE61426330 = nqIquILndE92907598;     nqIquILndE92907598 = nqIquILndE79175360;     nqIquILndE79175360 = nqIquILndE14242720;     nqIquILndE14242720 = nqIquILndE15410536;     nqIquILndE15410536 = nqIquILndE27485739;     nqIquILndE27485739 = nqIquILndE4452995;     nqIquILndE4452995 = nqIquILndE57491771;     nqIquILndE57491771 = nqIquILndE41713008;     nqIquILndE41713008 = nqIquILndE97024509;     nqIquILndE97024509 = nqIquILndE22148582;     nqIquILndE22148582 = nqIquILndE64668505;     nqIquILndE64668505 = nqIquILndE27033957;     nqIquILndE27033957 = nqIquILndE24956366;     nqIquILndE24956366 = nqIquILndE39879665;     nqIquILndE39879665 = nqIquILndE41179034;     nqIquILndE41179034 = nqIquILndE36574231;     nqIquILndE36574231 = nqIquILndE66959104;     nqIquILndE66959104 = nqIquILndE86631037;     nqIquILndE86631037 = nqIquILndE72463040;     nqIquILndE72463040 = nqIquILndE9901268;     nqIquILndE9901268 = nqIquILndE23026936;     nqIquILndE23026936 = nqIquILndE14494473;     nqIquILndE14494473 = nqIquILndE47516880;     nqIquILndE47516880 = nqIquILndE59159377;     nqIquILndE59159377 = nqIquILndE95489563;     nqIquILndE95489563 = nqIquILndE62743012;     nqIquILndE62743012 = nqIquILndE85131132;     nqIquILndE85131132 = nqIquILndE23436700;     nqIquILndE23436700 = nqIquILndE48053502;     nqIquILndE48053502 = nqIquILndE95800895;     nqIquILndE95800895 = nqIquILndE36895588;     nqIquILndE36895588 = nqIquILndE6106555;     nqIquILndE6106555 = nqIquILndE15462166;     nqIquILndE15462166 = nqIquILndE79117003;     nqIquILndE79117003 = nqIquILndE45039034;     nqIquILndE45039034 = nqIquILndE11370147;     nqIquILndE11370147 = nqIquILndE18414640;     nqIquILndE18414640 = nqIquILndE22615345;     nqIquILndE22615345 = nqIquILndE43624125;     nqIquILndE43624125 = nqIquILndE47329731;     nqIquILndE47329731 = nqIquILndE1685569;     nqIquILndE1685569 = nqIquILndE83712426;     nqIquILndE83712426 = nqIquILndE25473631;     nqIquILndE25473631 = nqIquILndE78042638;     nqIquILndE78042638 = nqIquILndE23892424;     nqIquILndE23892424 = nqIquILndE85148757;     nqIquILndE85148757 = nqIquILndE80407585;     nqIquILndE80407585 = nqIquILndE29786730;     nqIquILndE29786730 = nqIquILndE25113623;     nqIquILndE25113623 = nqIquILndE28416710;     nqIquILndE28416710 = nqIquILndE27312303;     nqIquILndE27312303 = nqIquILndE50577629;     nqIquILndE50577629 = nqIquILndE14232718;     nqIquILndE14232718 = nqIquILndE87096812;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void DDqnqlxLim30769570() {     double EOuUbArYOm74018490 = 85435178;    double EOuUbArYOm5900575 = -688193239;    double EOuUbArYOm50166832 = -668570998;    double EOuUbArYOm91722983 = -657554014;    double EOuUbArYOm16228786 = -247896704;    double EOuUbArYOm91291975 = -78562605;    double EOuUbArYOm81770209 = 30371958;    double EOuUbArYOm51947938 = -217999337;    double EOuUbArYOm83370674 = -834904914;    double EOuUbArYOm31241287 = -784606635;    double EOuUbArYOm43926496 = -40628986;    double EOuUbArYOm38705085 = -184319958;    double EOuUbArYOm9078820 = -444181827;    double EOuUbArYOm80310722 = -669208687;    double EOuUbArYOm71420969 = -682532558;    double EOuUbArYOm82999465 = 64595354;    double EOuUbArYOm57512454 = -242022073;    double EOuUbArYOm97323720 = -475630740;    double EOuUbArYOm15957099 = -948739614;    double EOuUbArYOm46482184 = -965021803;    double EOuUbArYOm80510110 = -768212589;    double EOuUbArYOm28762262 = -737749206;    double EOuUbArYOm43750595 = -464946646;    double EOuUbArYOm69159263 = 45140485;    double EOuUbArYOm92790816 = -260051116;    double EOuUbArYOm34722233 = -175586866;    double EOuUbArYOm94837060 = -38353200;    double EOuUbArYOm91132056 = -321959576;    double EOuUbArYOm5560152 = -807444204;    double EOuUbArYOm17739519 = -579666227;    double EOuUbArYOm30230116 = -700424188;    double EOuUbArYOm56224755 = -417547182;    double EOuUbArYOm13450422 = -634194185;    double EOuUbArYOm55348532 = -125502571;    double EOuUbArYOm15277090 = -227972663;    double EOuUbArYOm33151435 = -732702121;    double EOuUbArYOm44530193 = -235020299;    double EOuUbArYOm84511091 = -3404236;    double EOuUbArYOm58595117 = -666206914;    double EOuUbArYOm97453470 = -150755070;    double EOuUbArYOm49454088 = -954227274;    double EOuUbArYOm49831866 = 50931071;    double EOuUbArYOm70721581 = -49544593;    double EOuUbArYOm55753346 = -59413565;    double EOuUbArYOm38680619 = -89141539;    double EOuUbArYOm23930588 = 65335957;    double EOuUbArYOm82076356 = -857069315;    double EOuUbArYOm95163542 = -389423134;    double EOuUbArYOm69400988 = -469493235;    double EOuUbArYOm6842300 = -953611242;    double EOuUbArYOm56768398 = -11002643;    double EOuUbArYOm32398781 = -386484384;    double EOuUbArYOm89594597 = -995265711;    double EOuUbArYOm12036741 = -432814277;    double EOuUbArYOm50259460 = -671019754;    double EOuUbArYOm45256228 = -176815617;    double EOuUbArYOm62149980 = -123246593;    double EOuUbArYOm81007569 = -613711483;    double EOuUbArYOm98932166 = -297502898;    double EOuUbArYOm81506553 = 27690161;    double EOuUbArYOm96454914 = 59790595;    double EOuUbArYOm90638152 = -647668466;    double EOuUbArYOm46387786 = -410555134;    double EOuUbArYOm65631156 = -155238687;    double EOuUbArYOm1011172 = 15817552;    double EOuUbArYOm87701740 = -623081805;    double EOuUbArYOm25254663 = -550125774;    double EOuUbArYOm53730287 = -218679256;    double EOuUbArYOm65033633 = -341236025;    double EOuUbArYOm38269535 = -949830437;    double EOuUbArYOm38469272 = -700384347;    double EOuUbArYOm73001362 = -138617837;    double EOuUbArYOm38728603 = -809423826;    double EOuUbArYOm18503628 = -697984545;    double EOuUbArYOm97028096 = 89205471;    double EOuUbArYOm30678245 = -719143661;    double EOuUbArYOm58040681 = -588204614;    double EOuUbArYOm87997248 = -305533082;    double EOuUbArYOm30478645 = -865717976;    double EOuUbArYOm68860229 = -225387073;    double EOuUbArYOm52645876 = -318517552;    double EOuUbArYOm99673518 = -648930067;    double EOuUbArYOm21731069 = -852466342;    double EOuUbArYOm98717852 = -853832963;    double EOuUbArYOm60971120 = -468663585;    double EOuUbArYOm97831335 = -213939805;    double EOuUbArYOm66630158 = -422281471;    double EOuUbArYOm1413682 = -101379908;    double EOuUbArYOm5089073 = -454482818;    double EOuUbArYOm70020861 = 48842954;    double EOuUbArYOm71001454 = -509455528;    double EOuUbArYOm63522623 = -621308816;    double EOuUbArYOm85578925 = -705901338;    double EOuUbArYOm77088564 = -593897076;    double EOuUbArYOm998556 = -110545665;    double EOuUbArYOm58815935 = -206558808;    double EOuUbArYOm3444080 = -538513796;    double EOuUbArYOm5090426 = -894305906;    double EOuUbArYOm54742175 = 24768882;    double EOuUbArYOm50978878 = 85435178;     EOuUbArYOm74018490 = EOuUbArYOm5900575;     EOuUbArYOm5900575 = EOuUbArYOm50166832;     EOuUbArYOm50166832 = EOuUbArYOm91722983;     EOuUbArYOm91722983 = EOuUbArYOm16228786;     EOuUbArYOm16228786 = EOuUbArYOm91291975;     EOuUbArYOm91291975 = EOuUbArYOm81770209;     EOuUbArYOm81770209 = EOuUbArYOm51947938;     EOuUbArYOm51947938 = EOuUbArYOm83370674;     EOuUbArYOm83370674 = EOuUbArYOm31241287;     EOuUbArYOm31241287 = EOuUbArYOm43926496;     EOuUbArYOm43926496 = EOuUbArYOm38705085;     EOuUbArYOm38705085 = EOuUbArYOm9078820;     EOuUbArYOm9078820 = EOuUbArYOm80310722;     EOuUbArYOm80310722 = EOuUbArYOm71420969;     EOuUbArYOm71420969 = EOuUbArYOm82999465;     EOuUbArYOm82999465 = EOuUbArYOm57512454;     EOuUbArYOm57512454 = EOuUbArYOm97323720;     EOuUbArYOm97323720 = EOuUbArYOm15957099;     EOuUbArYOm15957099 = EOuUbArYOm46482184;     EOuUbArYOm46482184 = EOuUbArYOm80510110;     EOuUbArYOm80510110 = EOuUbArYOm28762262;     EOuUbArYOm28762262 = EOuUbArYOm43750595;     EOuUbArYOm43750595 = EOuUbArYOm69159263;     EOuUbArYOm69159263 = EOuUbArYOm92790816;     EOuUbArYOm92790816 = EOuUbArYOm34722233;     EOuUbArYOm34722233 = EOuUbArYOm94837060;     EOuUbArYOm94837060 = EOuUbArYOm91132056;     EOuUbArYOm91132056 = EOuUbArYOm5560152;     EOuUbArYOm5560152 = EOuUbArYOm17739519;     EOuUbArYOm17739519 = EOuUbArYOm30230116;     EOuUbArYOm30230116 = EOuUbArYOm56224755;     EOuUbArYOm56224755 = EOuUbArYOm13450422;     EOuUbArYOm13450422 = EOuUbArYOm55348532;     EOuUbArYOm55348532 = EOuUbArYOm15277090;     EOuUbArYOm15277090 = EOuUbArYOm33151435;     EOuUbArYOm33151435 = EOuUbArYOm44530193;     EOuUbArYOm44530193 = EOuUbArYOm84511091;     EOuUbArYOm84511091 = EOuUbArYOm58595117;     EOuUbArYOm58595117 = EOuUbArYOm97453470;     EOuUbArYOm97453470 = EOuUbArYOm49454088;     EOuUbArYOm49454088 = EOuUbArYOm49831866;     EOuUbArYOm49831866 = EOuUbArYOm70721581;     EOuUbArYOm70721581 = EOuUbArYOm55753346;     EOuUbArYOm55753346 = EOuUbArYOm38680619;     EOuUbArYOm38680619 = EOuUbArYOm23930588;     EOuUbArYOm23930588 = EOuUbArYOm82076356;     EOuUbArYOm82076356 = EOuUbArYOm95163542;     EOuUbArYOm95163542 = EOuUbArYOm69400988;     EOuUbArYOm69400988 = EOuUbArYOm6842300;     EOuUbArYOm6842300 = EOuUbArYOm56768398;     EOuUbArYOm56768398 = EOuUbArYOm32398781;     EOuUbArYOm32398781 = EOuUbArYOm89594597;     EOuUbArYOm89594597 = EOuUbArYOm12036741;     EOuUbArYOm12036741 = EOuUbArYOm50259460;     EOuUbArYOm50259460 = EOuUbArYOm45256228;     EOuUbArYOm45256228 = EOuUbArYOm62149980;     EOuUbArYOm62149980 = EOuUbArYOm81007569;     EOuUbArYOm81007569 = EOuUbArYOm98932166;     EOuUbArYOm98932166 = EOuUbArYOm81506553;     EOuUbArYOm81506553 = EOuUbArYOm96454914;     EOuUbArYOm96454914 = EOuUbArYOm90638152;     EOuUbArYOm90638152 = EOuUbArYOm46387786;     EOuUbArYOm46387786 = EOuUbArYOm65631156;     EOuUbArYOm65631156 = EOuUbArYOm1011172;     EOuUbArYOm1011172 = EOuUbArYOm87701740;     EOuUbArYOm87701740 = EOuUbArYOm25254663;     EOuUbArYOm25254663 = EOuUbArYOm53730287;     EOuUbArYOm53730287 = EOuUbArYOm65033633;     EOuUbArYOm65033633 = EOuUbArYOm38269535;     EOuUbArYOm38269535 = EOuUbArYOm38469272;     EOuUbArYOm38469272 = EOuUbArYOm73001362;     EOuUbArYOm73001362 = EOuUbArYOm38728603;     EOuUbArYOm38728603 = EOuUbArYOm18503628;     EOuUbArYOm18503628 = EOuUbArYOm97028096;     EOuUbArYOm97028096 = EOuUbArYOm30678245;     EOuUbArYOm30678245 = EOuUbArYOm58040681;     EOuUbArYOm58040681 = EOuUbArYOm87997248;     EOuUbArYOm87997248 = EOuUbArYOm30478645;     EOuUbArYOm30478645 = EOuUbArYOm68860229;     EOuUbArYOm68860229 = EOuUbArYOm52645876;     EOuUbArYOm52645876 = EOuUbArYOm99673518;     EOuUbArYOm99673518 = EOuUbArYOm21731069;     EOuUbArYOm21731069 = EOuUbArYOm98717852;     EOuUbArYOm98717852 = EOuUbArYOm60971120;     EOuUbArYOm60971120 = EOuUbArYOm97831335;     EOuUbArYOm97831335 = EOuUbArYOm66630158;     EOuUbArYOm66630158 = EOuUbArYOm1413682;     EOuUbArYOm1413682 = EOuUbArYOm5089073;     EOuUbArYOm5089073 = EOuUbArYOm70020861;     EOuUbArYOm70020861 = EOuUbArYOm71001454;     EOuUbArYOm71001454 = EOuUbArYOm63522623;     EOuUbArYOm63522623 = EOuUbArYOm85578925;     EOuUbArYOm85578925 = EOuUbArYOm77088564;     EOuUbArYOm77088564 = EOuUbArYOm998556;     EOuUbArYOm998556 = EOuUbArYOm58815935;     EOuUbArYOm58815935 = EOuUbArYOm3444080;     EOuUbArYOm3444080 = EOuUbArYOm5090426;     EOuUbArYOm5090426 = EOuUbArYOm54742175;     EOuUbArYOm54742175 = EOuUbArYOm50978878;     EOuUbArYOm50978878 = EOuUbArYOm74018490;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ivdAGAGvjH76465262() {     double lJTmBHinEb93207448 = -858339648;    double lJTmBHinEb78198432 = -329801571;    double lJTmBHinEb74826214 = -245417150;    double lJTmBHinEb76014430 = -663046184;    double lJTmBHinEb43235289 = -283088420;    double lJTmBHinEb4048933 = -744798357;    double lJTmBHinEb60760843 = -131331448;    double lJTmBHinEb88205531 = -778510747;    double lJTmBHinEb52367644 = -107072947;    double lJTmBHinEb95610074 = 64472222;    double lJTmBHinEb4676662 = -823449901;    double lJTmBHinEb15193327 = -242101638;    double lJTmBHinEb66514402 = -817368101;    double lJTmBHinEb73980510 = -650101907;    double lJTmBHinEb88622635 = -48942026;    double lJTmBHinEb63790247 = -262548254;    double lJTmBHinEb57361041 = -8261918;    double lJTmBHinEb88160648 = -862838761;    double lJTmBHinEb37055639 = -435970007;    double lJTmBHinEb42353284 = -395851334;    double lJTmBHinEb81267059 = -289487425;    double lJTmBHinEb54889592 = -227548903;    double lJTmBHinEb46573443 = -773015725;    double lJTmBHinEb20444795 = -123996250;    double lJTmBHinEb8968171 = 64326778;    double lJTmBHinEb81839085 = -398924772;    double lJTmBHinEb6627993 = 12286949;    double lJTmBHinEb39056157 = -769206251;    double lJTmBHinEb11468062 = -159664592;    double lJTmBHinEb82731386 = -511348399;    double lJTmBHinEb96765091 = 15590539;    double lJTmBHinEb34117205 = -347894057;    double lJTmBHinEb54340711 = -145060038;    double lJTmBHinEb20538836 = -717611300;    double lJTmBHinEb66330830 = -754049596;    double lJTmBHinEb11745323 = -845898158;    double lJTmBHinEb81764232 = -320404258;    double lJTmBHinEb16190607 = -989940354;    double lJTmBHinEb14287802 = 90039345;    double lJTmBHinEb29041226 = 46767191;    double lJTmBHinEb59817548 = -285522416;    double lJTmBHinEb82839058 = -55847807;    double lJTmBHinEb32621746 = -895999379;    double lJTmBHinEb83848305 = -685725264;    double lJTmBHinEb82666275 = -868407377;    double lJTmBHinEb73730829 = -876574696;    double lJTmBHinEb6902697 = -881420960;    double lJTmBHinEb76248120 = -809310269;    double lJTmBHinEb7509479 = -269250405;    double lJTmBHinEb47199795 = -289628607;    double lJTmBHinEb64370583 = -472496595;    double lJTmBHinEb52842716 = 29956232;    double lJTmBHinEb28149801 = -871189623;    double lJTmBHinEb63710922 = -669112113;    double lJTmBHinEb95198203 = -477337567;    double lJTmBHinEb38317856 = -530790745;    double lJTmBHinEb31624989 = -556785847;    double lJTmBHinEb54381419 = -21420901;    double lJTmBHinEb67046259 = -627372962;    double lJTmBHinEb61396204 = -884163648;    double lJTmBHinEb97420940 = -657085306;    double lJTmBHinEb21704686 = -362125197;    double lJTmBHinEb76737469 = -518846155;    double lJTmBHinEb69636257 = -595724549;    double lJTmBHinEb98844982 = -951118317;    double lJTmBHinEb70559456 = -375555844;    double lJTmBHinEb60852616 = 2958399;    double lJTmBHinEb45975567 = 243198;    double lJTmBHinEb7649680 = -896052311;    double lJTmBHinEb76877313 = -203043869;    double lJTmBHinEb82026015 = -942143996;    double lJTmBHinEb41170434 = -18321564;    double lJTmBHinEb73872847 = -852878106;    double lJTmBHinEb8014414 = -382737198;    double lJTmBHinEb82535735 = -10328919;    double lJTmBHinEb98428001 = -133639619;    double lJTmBHinEb22267847 = -331549525;    double lJTmBHinEb62725138 = 12709538;    double lJTmBHinEb37778519 = -255588873;    double lJTmBHinEb35237342 = -59098527;    double lJTmBHinEb74936388 = -517503813;    double lJTmBHinEb30379872 = -178402783;    double lJTmBHinEb31546679 = -399955846;    double lJTmBHinEb64268266 = -870035986;    double lJTmBHinEb18360803 = 61148195;    double lJTmBHinEb43922376 = 85634307;    double lJTmBHinEb5967405 = -476704435;    double lJTmBHinEb90629788 = -475947925;    double lJTmBHinEb25340633 = -140273733;    double lJTmBHinEb28012975 = -123258852;    double lJTmBHinEb80120333 = -189112312;    double lJTmBHinEb27382813 = -198983358;    double lJTmBHinEb49144348 = -262567393;    double lJTmBHinEb52891597 = -25797008;    double lJTmBHinEb31620286 = -296147503;    double lJTmBHinEb38112863 = -923397219;    double lJTmBHinEb6101589 = -537001652;    double lJTmBHinEb62985488 = -200274830;    double lJTmBHinEb85003323 = -734606947;    double lJTmBHinEb12106819 = -858339648;     lJTmBHinEb93207448 = lJTmBHinEb78198432;     lJTmBHinEb78198432 = lJTmBHinEb74826214;     lJTmBHinEb74826214 = lJTmBHinEb76014430;     lJTmBHinEb76014430 = lJTmBHinEb43235289;     lJTmBHinEb43235289 = lJTmBHinEb4048933;     lJTmBHinEb4048933 = lJTmBHinEb60760843;     lJTmBHinEb60760843 = lJTmBHinEb88205531;     lJTmBHinEb88205531 = lJTmBHinEb52367644;     lJTmBHinEb52367644 = lJTmBHinEb95610074;     lJTmBHinEb95610074 = lJTmBHinEb4676662;     lJTmBHinEb4676662 = lJTmBHinEb15193327;     lJTmBHinEb15193327 = lJTmBHinEb66514402;     lJTmBHinEb66514402 = lJTmBHinEb73980510;     lJTmBHinEb73980510 = lJTmBHinEb88622635;     lJTmBHinEb88622635 = lJTmBHinEb63790247;     lJTmBHinEb63790247 = lJTmBHinEb57361041;     lJTmBHinEb57361041 = lJTmBHinEb88160648;     lJTmBHinEb88160648 = lJTmBHinEb37055639;     lJTmBHinEb37055639 = lJTmBHinEb42353284;     lJTmBHinEb42353284 = lJTmBHinEb81267059;     lJTmBHinEb81267059 = lJTmBHinEb54889592;     lJTmBHinEb54889592 = lJTmBHinEb46573443;     lJTmBHinEb46573443 = lJTmBHinEb20444795;     lJTmBHinEb20444795 = lJTmBHinEb8968171;     lJTmBHinEb8968171 = lJTmBHinEb81839085;     lJTmBHinEb81839085 = lJTmBHinEb6627993;     lJTmBHinEb6627993 = lJTmBHinEb39056157;     lJTmBHinEb39056157 = lJTmBHinEb11468062;     lJTmBHinEb11468062 = lJTmBHinEb82731386;     lJTmBHinEb82731386 = lJTmBHinEb96765091;     lJTmBHinEb96765091 = lJTmBHinEb34117205;     lJTmBHinEb34117205 = lJTmBHinEb54340711;     lJTmBHinEb54340711 = lJTmBHinEb20538836;     lJTmBHinEb20538836 = lJTmBHinEb66330830;     lJTmBHinEb66330830 = lJTmBHinEb11745323;     lJTmBHinEb11745323 = lJTmBHinEb81764232;     lJTmBHinEb81764232 = lJTmBHinEb16190607;     lJTmBHinEb16190607 = lJTmBHinEb14287802;     lJTmBHinEb14287802 = lJTmBHinEb29041226;     lJTmBHinEb29041226 = lJTmBHinEb59817548;     lJTmBHinEb59817548 = lJTmBHinEb82839058;     lJTmBHinEb82839058 = lJTmBHinEb32621746;     lJTmBHinEb32621746 = lJTmBHinEb83848305;     lJTmBHinEb83848305 = lJTmBHinEb82666275;     lJTmBHinEb82666275 = lJTmBHinEb73730829;     lJTmBHinEb73730829 = lJTmBHinEb6902697;     lJTmBHinEb6902697 = lJTmBHinEb76248120;     lJTmBHinEb76248120 = lJTmBHinEb7509479;     lJTmBHinEb7509479 = lJTmBHinEb47199795;     lJTmBHinEb47199795 = lJTmBHinEb64370583;     lJTmBHinEb64370583 = lJTmBHinEb52842716;     lJTmBHinEb52842716 = lJTmBHinEb28149801;     lJTmBHinEb28149801 = lJTmBHinEb63710922;     lJTmBHinEb63710922 = lJTmBHinEb95198203;     lJTmBHinEb95198203 = lJTmBHinEb38317856;     lJTmBHinEb38317856 = lJTmBHinEb31624989;     lJTmBHinEb31624989 = lJTmBHinEb54381419;     lJTmBHinEb54381419 = lJTmBHinEb67046259;     lJTmBHinEb67046259 = lJTmBHinEb61396204;     lJTmBHinEb61396204 = lJTmBHinEb97420940;     lJTmBHinEb97420940 = lJTmBHinEb21704686;     lJTmBHinEb21704686 = lJTmBHinEb76737469;     lJTmBHinEb76737469 = lJTmBHinEb69636257;     lJTmBHinEb69636257 = lJTmBHinEb98844982;     lJTmBHinEb98844982 = lJTmBHinEb70559456;     lJTmBHinEb70559456 = lJTmBHinEb60852616;     lJTmBHinEb60852616 = lJTmBHinEb45975567;     lJTmBHinEb45975567 = lJTmBHinEb7649680;     lJTmBHinEb7649680 = lJTmBHinEb76877313;     lJTmBHinEb76877313 = lJTmBHinEb82026015;     lJTmBHinEb82026015 = lJTmBHinEb41170434;     lJTmBHinEb41170434 = lJTmBHinEb73872847;     lJTmBHinEb73872847 = lJTmBHinEb8014414;     lJTmBHinEb8014414 = lJTmBHinEb82535735;     lJTmBHinEb82535735 = lJTmBHinEb98428001;     lJTmBHinEb98428001 = lJTmBHinEb22267847;     lJTmBHinEb22267847 = lJTmBHinEb62725138;     lJTmBHinEb62725138 = lJTmBHinEb37778519;     lJTmBHinEb37778519 = lJTmBHinEb35237342;     lJTmBHinEb35237342 = lJTmBHinEb74936388;     lJTmBHinEb74936388 = lJTmBHinEb30379872;     lJTmBHinEb30379872 = lJTmBHinEb31546679;     lJTmBHinEb31546679 = lJTmBHinEb64268266;     lJTmBHinEb64268266 = lJTmBHinEb18360803;     lJTmBHinEb18360803 = lJTmBHinEb43922376;     lJTmBHinEb43922376 = lJTmBHinEb5967405;     lJTmBHinEb5967405 = lJTmBHinEb90629788;     lJTmBHinEb90629788 = lJTmBHinEb25340633;     lJTmBHinEb25340633 = lJTmBHinEb28012975;     lJTmBHinEb28012975 = lJTmBHinEb80120333;     lJTmBHinEb80120333 = lJTmBHinEb27382813;     lJTmBHinEb27382813 = lJTmBHinEb49144348;     lJTmBHinEb49144348 = lJTmBHinEb52891597;     lJTmBHinEb52891597 = lJTmBHinEb31620286;     lJTmBHinEb31620286 = lJTmBHinEb38112863;     lJTmBHinEb38112863 = lJTmBHinEb6101589;     lJTmBHinEb6101589 = lJTmBHinEb62985488;     lJTmBHinEb62985488 = lJTmBHinEb85003323;     lJTmBHinEb85003323 = lJTmBHinEb12106819;     lJTmBHinEb12106819 = lJTmBHinEb93207448;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void JqoQObKBIU61197903() {     double CBqKAmtdZt31591835 = -673365157;    double CBqKAmtdZt84221830 = -670423111;    double CBqKAmtdZt22831766 = -541980840;    double CBqKAmtdZt59517464 = -853489693;    double CBqKAmtdZt88408194 = -985116529;    double CBqKAmtdZt94757415 = -177837051;    double CBqKAmtdZt50194774 = -750201791;    double CBqKAmtdZt91506666 = 4643752;    double CBqKAmtdZt7621419 = -37762944;    double CBqKAmtdZt27708134 = -800563192;    double CBqKAmtdZt5856387 = -229619659;    double CBqKAmtdZt53878133 = -622186636;    double CBqKAmtdZt49532655 = -442082174;    double CBqKAmtdZt7475866 = -266735296;    double CBqKAmtdZt77853858 = 80951889;    double CBqKAmtdZt50807264 = -584039819;    double CBqKAmtdZt1205456 = -821127040;    double CBqKAmtdZt46833972 = -140183180;    double CBqKAmtdZt58824451 = -992448526;    double CBqKAmtdZt62021721 = -845485946;    double CBqKAmtdZt3111533 = -721570723;    double CBqKAmtdZt28847987 = -897156414;    double CBqKAmtdZt15340744 = -930196041;    double CBqKAmtdZt79365750 = -581349197;    double CBqKAmtdZt12880191 = 39776417;    double CBqKAmtdZt59498713 = -937394656;    double CBqKAmtdZt59988274 = -685633040;    double CBqKAmtdZt99500832 = 45267389;    double CBqKAmtdZt10605433 = -418035053;    double CBqKAmtdZt36625253 = -699281368;    double CBqKAmtdZt96422364 = -365286773;    double CBqKAmtdZt5178246 = -916766046;    double CBqKAmtdZt50668865 = -64847527;    double CBqKAmtdZt50636605 = -925147024;    double CBqKAmtdZt86080274 = -446889539;    double CBqKAmtdZt26104909 = -992524485;    double CBqKAmtdZt44290942 = -391518796;    double CBqKAmtdZt9631958 = -767749913;    double CBqKAmtdZt90359572 = -984786722;    double CBqKAmtdZt20343844 = 51243398;    double CBqKAmtdZt92688802 = 39831198;    double CBqKAmtdZt63155782 = -75613481;    double CBqKAmtdZt96138714 = -929778697;    double CBqKAmtdZt80806325 = -181384219;    double CBqKAmtdZt86619537 = -853679652;    double CBqKAmtdZt32903345 = -748351332;    double CBqKAmtdZt28857586 = -448843628;    double CBqKAmtdZt63033696 = 59676818;    double CBqKAmtdZt65068069 = -305000884;    double CBqKAmtdZt7869365 = -474638450;    double CBqKAmtdZt66315585 = -716929491;    double CBqKAmtdZt79183327 = -955016789;    double CBqKAmtdZt29811858 = -730983438;    double CBqKAmtdZt56809014 = -609787934;    double CBqKAmtdZt58086370 = -333073915;    double CBqKAmtdZt2743849 = -776208744;    double CBqKAmtdZt68881087 = -740227071;    double CBqKAmtdZt43466015 = -960631644;    double CBqKAmtdZt46637274 = -793266111;    double CBqKAmtdZt28909481 = 52278127;    double CBqKAmtdZt34769141 = -492204011;    double CBqKAmtdZt50693941 = -695469180;    double CBqKAmtdZt80901233 = -577321196;    double CBqKAmtdZt70996166 = -338481577;    double CBqKAmtdZt31285769 = -335276419;    double CBqKAmtdZt678141 = -312853613;    double CBqKAmtdZt3209268 = -457339109;    double CBqKAmtdZt98896050 = -516935150;    double CBqKAmtdZt21395591 = -819845758;    double CBqKAmtdZt51748950 = 73476374;    double CBqKAmtdZt6516322 = -92521023;    double CBqKAmtdZt91573498 = 46622873;    double CBqKAmtdZt56474400 = -155396458;    double CBqKAmtdZt38480607 = -943691925;    double CBqKAmtdZt69332918 = -785317145;    double CBqKAmtdZt39955751 = -545957243;    double CBqKAmtdZt32709272 = -967377718;    double CBqKAmtdZt34534418 = -648811823;    double CBqKAmtdZt92746212 = -727669545;    double CBqKAmtdZt79976846 = -211872252;    double CBqKAmtdZt30641128 = -388551029;    double CBqKAmtdZt96954578 = -645309858;    double CBqKAmtdZt34432763 = -649731728;    double CBqKAmtdZt2736069 = -943396603;    double CBqKAmtdZt70309668 = -982351877;    double CBqKAmtdZt17239037 = -410269984;    double CBqKAmtdZt75366388 = -85782609;    double CBqKAmtdZt93859850 = -455059594;    double CBqKAmtdZt92550235 = -492073110;    double CBqKAmtdZt83336426 = -670680795;    double CBqKAmtdZt57223821 = -152297415;    double CBqKAmtdZt824928 = -430887153;    double CBqKAmtdZt62994684 = -974483803;    double CBqKAmtdZt61450091 = -937064850;    double CBqKAmtdZt85574703 = -456552592;    double CBqKAmtdZt41994861 = -264699623;    double CBqKAmtdZt82254548 = -498292285;    double CBqKAmtdZt25142549 = -491297120;    double CBqKAmtdZt49520556 = -846107800;    double CBqKAmtdZt85941396 = -673365157;     CBqKAmtdZt31591835 = CBqKAmtdZt84221830;     CBqKAmtdZt84221830 = CBqKAmtdZt22831766;     CBqKAmtdZt22831766 = CBqKAmtdZt59517464;     CBqKAmtdZt59517464 = CBqKAmtdZt88408194;     CBqKAmtdZt88408194 = CBqKAmtdZt94757415;     CBqKAmtdZt94757415 = CBqKAmtdZt50194774;     CBqKAmtdZt50194774 = CBqKAmtdZt91506666;     CBqKAmtdZt91506666 = CBqKAmtdZt7621419;     CBqKAmtdZt7621419 = CBqKAmtdZt27708134;     CBqKAmtdZt27708134 = CBqKAmtdZt5856387;     CBqKAmtdZt5856387 = CBqKAmtdZt53878133;     CBqKAmtdZt53878133 = CBqKAmtdZt49532655;     CBqKAmtdZt49532655 = CBqKAmtdZt7475866;     CBqKAmtdZt7475866 = CBqKAmtdZt77853858;     CBqKAmtdZt77853858 = CBqKAmtdZt50807264;     CBqKAmtdZt50807264 = CBqKAmtdZt1205456;     CBqKAmtdZt1205456 = CBqKAmtdZt46833972;     CBqKAmtdZt46833972 = CBqKAmtdZt58824451;     CBqKAmtdZt58824451 = CBqKAmtdZt62021721;     CBqKAmtdZt62021721 = CBqKAmtdZt3111533;     CBqKAmtdZt3111533 = CBqKAmtdZt28847987;     CBqKAmtdZt28847987 = CBqKAmtdZt15340744;     CBqKAmtdZt15340744 = CBqKAmtdZt79365750;     CBqKAmtdZt79365750 = CBqKAmtdZt12880191;     CBqKAmtdZt12880191 = CBqKAmtdZt59498713;     CBqKAmtdZt59498713 = CBqKAmtdZt59988274;     CBqKAmtdZt59988274 = CBqKAmtdZt99500832;     CBqKAmtdZt99500832 = CBqKAmtdZt10605433;     CBqKAmtdZt10605433 = CBqKAmtdZt36625253;     CBqKAmtdZt36625253 = CBqKAmtdZt96422364;     CBqKAmtdZt96422364 = CBqKAmtdZt5178246;     CBqKAmtdZt5178246 = CBqKAmtdZt50668865;     CBqKAmtdZt50668865 = CBqKAmtdZt50636605;     CBqKAmtdZt50636605 = CBqKAmtdZt86080274;     CBqKAmtdZt86080274 = CBqKAmtdZt26104909;     CBqKAmtdZt26104909 = CBqKAmtdZt44290942;     CBqKAmtdZt44290942 = CBqKAmtdZt9631958;     CBqKAmtdZt9631958 = CBqKAmtdZt90359572;     CBqKAmtdZt90359572 = CBqKAmtdZt20343844;     CBqKAmtdZt20343844 = CBqKAmtdZt92688802;     CBqKAmtdZt92688802 = CBqKAmtdZt63155782;     CBqKAmtdZt63155782 = CBqKAmtdZt96138714;     CBqKAmtdZt96138714 = CBqKAmtdZt80806325;     CBqKAmtdZt80806325 = CBqKAmtdZt86619537;     CBqKAmtdZt86619537 = CBqKAmtdZt32903345;     CBqKAmtdZt32903345 = CBqKAmtdZt28857586;     CBqKAmtdZt28857586 = CBqKAmtdZt63033696;     CBqKAmtdZt63033696 = CBqKAmtdZt65068069;     CBqKAmtdZt65068069 = CBqKAmtdZt7869365;     CBqKAmtdZt7869365 = CBqKAmtdZt66315585;     CBqKAmtdZt66315585 = CBqKAmtdZt79183327;     CBqKAmtdZt79183327 = CBqKAmtdZt29811858;     CBqKAmtdZt29811858 = CBqKAmtdZt56809014;     CBqKAmtdZt56809014 = CBqKAmtdZt58086370;     CBqKAmtdZt58086370 = CBqKAmtdZt2743849;     CBqKAmtdZt2743849 = CBqKAmtdZt68881087;     CBqKAmtdZt68881087 = CBqKAmtdZt43466015;     CBqKAmtdZt43466015 = CBqKAmtdZt46637274;     CBqKAmtdZt46637274 = CBqKAmtdZt28909481;     CBqKAmtdZt28909481 = CBqKAmtdZt34769141;     CBqKAmtdZt34769141 = CBqKAmtdZt50693941;     CBqKAmtdZt50693941 = CBqKAmtdZt80901233;     CBqKAmtdZt80901233 = CBqKAmtdZt70996166;     CBqKAmtdZt70996166 = CBqKAmtdZt31285769;     CBqKAmtdZt31285769 = CBqKAmtdZt678141;     CBqKAmtdZt678141 = CBqKAmtdZt3209268;     CBqKAmtdZt3209268 = CBqKAmtdZt98896050;     CBqKAmtdZt98896050 = CBqKAmtdZt21395591;     CBqKAmtdZt21395591 = CBqKAmtdZt51748950;     CBqKAmtdZt51748950 = CBqKAmtdZt6516322;     CBqKAmtdZt6516322 = CBqKAmtdZt91573498;     CBqKAmtdZt91573498 = CBqKAmtdZt56474400;     CBqKAmtdZt56474400 = CBqKAmtdZt38480607;     CBqKAmtdZt38480607 = CBqKAmtdZt69332918;     CBqKAmtdZt69332918 = CBqKAmtdZt39955751;     CBqKAmtdZt39955751 = CBqKAmtdZt32709272;     CBqKAmtdZt32709272 = CBqKAmtdZt34534418;     CBqKAmtdZt34534418 = CBqKAmtdZt92746212;     CBqKAmtdZt92746212 = CBqKAmtdZt79976846;     CBqKAmtdZt79976846 = CBqKAmtdZt30641128;     CBqKAmtdZt30641128 = CBqKAmtdZt96954578;     CBqKAmtdZt96954578 = CBqKAmtdZt34432763;     CBqKAmtdZt34432763 = CBqKAmtdZt2736069;     CBqKAmtdZt2736069 = CBqKAmtdZt70309668;     CBqKAmtdZt70309668 = CBqKAmtdZt17239037;     CBqKAmtdZt17239037 = CBqKAmtdZt75366388;     CBqKAmtdZt75366388 = CBqKAmtdZt93859850;     CBqKAmtdZt93859850 = CBqKAmtdZt92550235;     CBqKAmtdZt92550235 = CBqKAmtdZt83336426;     CBqKAmtdZt83336426 = CBqKAmtdZt57223821;     CBqKAmtdZt57223821 = CBqKAmtdZt824928;     CBqKAmtdZt824928 = CBqKAmtdZt62994684;     CBqKAmtdZt62994684 = CBqKAmtdZt61450091;     CBqKAmtdZt61450091 = CBqKAmtdZt85574703;     CBqKAmtdZt85574703 = CBqKAmtdZt41994861;     CBqKAmtdZt41994861 = CBqKAmtdZt82254548;     CBqKAmtdZt82254548 = CBqKAmtdZt25142549;     CBqKAmtdZt25142549 = CBqKAmtdZt49520556;     CBqKAmtdZt49520556 = CBqKAmtdZt85941396;     CBqKAmtdZt85941396 = CBqKAmtdZt31591835;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void nwoegxNVvI67856647() {     double bXkNhIZYib31585364 = -545889299;    double bXkNhIZYib22794147 = -713018236;    double bXkNhIZYib24144978 = -499109454;    double bXkNhIZYib44597325 = -674030523;    double bXkNhIZYib97248296 = -353471851;    double bXkNhIZYib29562848 = -977269862;    double bXkNhIZYib18742110 = -454738258;    double bXkNhIZYib60720717 = -799533565;    double bXkNhIZYib90361582 = -851409014;    double bXkNhIZYib24347648 = -437370063;    double bXkNhIZYib26176992 = -189091731;    double bXkNhIZYib68169810 = -357664998;    double bXkNhIZYib81385569 = -463740649;    double bXkNhIZYib61320085 = -611888348;    double bXkNhIZYib23025968 = -981760963;    double bXkNhIZYib25371812 = -916835469;    double bXkNhIZYib57058215 = -640741609;    double bXkNhIZYib69834505 = -537254802;    double bXkNhIZYib79252720 = -510430793;    double bXkNhIZYib34095484 = -357510398;    double bXkNhIZYib82780957 = -432037097;    double bXkNhIZYib7144253 = -307148297;    double bXkNhIZYib52219140 = -289153883;    double bXkNhIZYib23015859 = -462269719;    double bXkNhIZYib41322880 = -386917436;    double bXkNhIZYib76072790 = -845600585;    double bXkNhIZYib30209856 = -986432753;    double bXkNhIZYib34904357 = -563699600;    double bXkNhIZYib23283880 = 35894632;    double bXkNhIZYib12715123 = -374712741;    double bXkNhIZYib29835044 = -752380007;    double bXkNhIZYib89902103 = -208587808;    double bXkNhIZYib36121289 = -266791743;    double bXkNhIZYib50919443 = -801828757;    double bXkNhIZYib68438313 = -706203463;    double bXkNhIZYib68933098 = 27709768;    double bXkNhIZYib56232310 = -491172175;    double bXkNhIZYib79549637 = -763012592;    double bXkNhIZYib25673170 = -597468138;    double bXkNhIZYib92216736 = -658188288;    double bXkNhIZYib80544470 = -48112701;    double bXkNhIZYib48853443 = -269405563;    double bXkNhIZYib56422075 = -388908951;    double bXkNhIZYib40038224 = -838348662;    double bXkNhIZYib70637590 = -226939053;    double bXkNhIZYib73331313 = -560396002;    double bXkNhIZYib56555378 = -930124251;    double bXkNhIZYib38417277 = -549084539;    double bXkNhIZYib83726458 = -968764745;    double bXkNhIZYib27914788 = -61663336;    double bXkNhIZYib79574954 = -295484498;    double bXkNhIZYib93730585 = -237162537;    double bXkNhIZYib5260206 = -623037446;    double bXkNhIZYib67059285 = -41707785;    double bXkNhIZYib85075690 = -89973193;    double bXkNhIZYib24441112 = -138741002;    double bXkNhIZYib70575007 = -323864354;    double bXkNhIZYib1129119 = 63160265;    double bXkNhIZYib3274445 = -187113088;    double bXkNhIZYib21175507 = -507871266;    double bXkNhIZYib99352991 = -990837109;    double bXkNhIZYib83837752 = -891038659;    double bXkNhIZYib37436837 = -735428198;    double bXkNhIZYib77646460 = -376696273;    double bXkNhIZYib94512604 = -684990056;    double bXkNhIZYib36274889 = -980503923;    double bXkNhIZYib32048522 = 9126744;    double bXkNhIZYib30466127 = -661911892;    double bXkNhIZYib92881772 = -905684885;    double bXkNhIZYib54092869 = -909470732;    double bXkNhIZYib69139501 = -325663295;    double bXkNhIZYib77508578 = -877729018;    double bXkNhIZYib44161336 = -939786664;    double bXkNhIZYib87035984 = -852242506;    double bXkNhIZYib53551014 = -209397697;    double bXkNhIZYib33927514 = -62631534;    double bXkNhIZYib50722178 = -918239347;    double bXkNhIZYib12180916 = -450805221;    double bXkNhIZYib52378269 = -135330666;    double bXkNhIZYib67991567 = -826521434;    double bXkNhIZYib19517412 = -915476335;    double bXkNhIZYib91792579 = -337348215;    double bXkNhIZYib51177899 = -594934855;    double bXkNhIZYib95369092 = -902442032;    double bXkNhIZYib33140168 = 20771756;    double bXkNhIZYib36104459 = -415217471;    double bXkNhIZYib84641897 = -585550362;    double bXkNhIZYib69062003 = -125083958;    double bXkNhIZYib65843752 = -611855565;    double bXkNhIZYib43997202 = -467462462;    double bXkNhIZYib98358090 = -648425878;    double bXkNhIZYib55103191 = -454332440;    double bXkNhIZYib76275192 = -475899505;    double bXkNhIZYib4497663 = 10403127;    double bXkNhIZYib92863745 = -667351179;    double bXkNhIZYib96706717 = -157074042;    double bXkNhIZYib11416606 = -533977365;    double bXkNhIZYib78775614 = 87787322;    double bXkNhIZYib45525620 = -53358607;    double bXkNhIZYib34362701 = -545889299;     bXkNhIZYib31585364 = bXkNhIZYib22794147;     bXkNhIZYib22794147 = bXkNhIZYib24144978;     bXkNhIZYib24144978 = bXkNhIZYib44597325;     bXkNhIZYib44597325 = bXkNhIZYib97248296;     bXkNhIZYib97248296 = bXkNhIZYib29562848;     bXkNhIZYib29562848 = bXkNhIZYib18742110;     bXkNhIZYib18742110 = bXkNhIZYib60720717;     bXkNhIZYib60720717 = bXkNhIZYib90361582;     bXkNhIZYib90361582 = bXkNhIZYib24347648;     bXkNhIZYib24347648 = bXkNhIZYib26176992;     bXkNhIZYib26176992 = bXkNhIZYib68169810;     bXkNhIZYib68169810 = bXkNhIZYib81385569;     bXkNhIZYib81385569 = bXkNhIZYib61320085;     bXkNhIZYib61320085 = bXkNhIZYib23025968;     bXkNhIZYib23025968 = bXkNhIZYib25371812;     bXkNhIZYib25371812 = bXkNhIZYib57058215;     bXkNhIZYib57058215 = bXkNhIZYib69834505;     bXkNhIZYib69834505 = bXkNhIZYib79252720;     bXkNhIZYib79252720 = bXkNhIZYib34095484;     bXkNhIZYib34095484 = bXkNhIZYib82780957;     bXkNhIZYib82780957 = bXkNhIZYib7144253;     bXkNhIZYib7144253 = bXkNhIZYib52219140;     bXkNhIZYib52219140 = bXkNhIZYib23015859;     bXkNhIZYib23015859 = bXkNhIZYib41322880;     bXkNhIZYib41322880 = bXkNhIZYib76072790;     bXkNhIZYib76072790 = bXkNhIZYib30209856;     bXkNhIZYib30209856 = bXkNhIZYib34904357;     bXkNhIZYib34904357 = bXkNhIZYib23283880;     bXkNhIZYib23283880 = bXkNhIZYib12715123;     bXkNhIZYib12715123 = bXkNhIZYib29835044;     bXkNhIZYib29835044 = bXkNhIZYib89902103;     bXkNhIZYib89902103 = bXkNhIZYib36121289;     bXkNhIZYib36121289 = bXkNhIZYib50919443;     bXkNhIZYib50919443 = bXkNhIZYib68438313;     bXkNhIZYib68438313 = bXkNhIZYib68933098;     bXkNhIZYib68933098 = bXkNhIZYib56232310;     bXkNhIZYib56232310 = bXkNhIZYib79549637;     bXkNhIZYib79549637 = bXkNhIZYib25673170;     bXkNhIZYib25673170 = bXkNhIZYib92216736;     bXkNhIZYib92216736 = bXkNhIZYib80544470;     bXkNhIZYib80544470 = bXkNhIZYib48853443;     bXkNhIZYib48853443 = bXkNhIZYib56422075;     bXkNhIZYib56422075 = bXkNhIZYib40038224;     bXkNhIZYib40038224 = bXkNhIZYib70637590;     bXkNhIZYib70637590 = bXkNhIZYib73331313;     bXkNhIZYib73331313 = bXkNhIZYib56555378;     bXkNhIZYib56555378 = bXkNhIZYib38417277;     bXkNhIZYib38417277 = bXkNhIZYib83726458;     bXkNhIZYib83726458 = bXkNhIZYib27914788;     bXkNhIZYib27914788 = bXkNhIZYib79574954;     bXkNhIZYib79574954 = bXkNhIZYib93730585;     bXkNhIZYib93730585 = bXkNhIZYib5260206;     bXkNhIZYib5260206 = bXkNhIZYib67059285;     bXkNhIZYib67059285 = bXkNhIZYib85075690;     bXkNhIZYib85075690 = bXkNhIZYib24441112;     bXkNhIZYib24441112 = bXkNhIZYib70575007;     bXkNhIZYib70575007 = bXkNhIZYib1129119;     bXkNhIZYib1129119 = bXkNhIZYib3274445;     bXkNhIZYib3274445 = bXkNhIZYib21175507;     bXkNhIZYib21175507 = bXkNhIZYib99352991;     bXkNhIZYib99352991 = bXkNhIZYib83837752;     bXkNhIZYib83837752 = bXkNhIZYib37436837;     bXkNhIZYib37436837 = bXkNhIZYib77646460;     bXkNhIZYib77646460 = bXkNhIZYib94512604;     bXkNhIZYib94512604 = bXkNhIZYib36274889;     bXkNhIZYib36274889 = bXkNhIZYib32048522;     bXkNhIZYib32048522 = bXkNhIZYib30466127;     bXkNhIZYib30466127 = bXkNhIZYib92881772;     bXkNhIZYib92881772 = bXkNhIZYib54092869;     bXkNhIZYib54092869 = bXkNhIZYib69139501;     bXkNhIZYib69139501 = bXkNhIZYib77508578;     bXkNhIZYib77508578 = bXkNhIZYib44161336;     bXkNhIZYib44161336 = bXkNhIZYib87035984;     bXkNhIZYib87035984 = bXkNhIZYib53551014;     bXkNhIZYib53551014 = bXkNhIZYib33927514;     bXkNhIZYib33927514 = bXkNhIZYib50722178;     bXkNhIZYib50722178 = bXkNhIZYib12180916;     bXkNhIZYib12180916 = bXkNhIZYib52378269;     bXkNhIZYib52378269 = bXkNhIZYib67991567;     bXkNhIZYib67991567 = bXkNhIZYib19517412;     bXkNhIZYib19517412 = bXkNhIZYib91792579;     bXkNhIZYib91792579 = bXkNhIZYib51177899;     bXkNhIZYib51177899 = bXkNhIZYib95369092;     bXkNhIZYib95369092 = bXkNhIZYib33140168;     bXkNhIZYib33140168 = bXkNhIZYib36104459;     bXkNhIZYib36104459 = bXkNhIZYib84641897;     bXkNhIZYib84641897 = bXkNhIZYib69062003;     bXkNhIZYib69062003 = bXkNhIZYib65843752;     bXkNhIZYib65843752 = bXkNhIZYib43997202;     bXkNhIZYib43997202 = bXkNhIZYib98358090;     bXkNhIZYib98358090 = bXkNhIZYib55103191;     bXkNhIZYib55103191 = bXkNhIZYib76275192;     bXkNhIZYib76275192 = bXkNhIZYib4497663;     bXkNhIZYib4497663 = bXkNhIZYib92863745;     bXkNhIZYib92863745 = bXkNhIZYib96706717;     bXkNhIZYib96706717 = bXkNhIZYib11416606;     bXkNhIZYib11416606 = bXkNhIZYib78775614;     bXkNhIZYib78775614 = bXkNhIZYib45525620;     bXkNhIZYib45525620 = bXkNhIZYib34362701;     bXkNhIZYib34362701 = bXkNhIZYib31585364;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void IflFNeISMz13552340() {     double pTCrxDEBTI50774321 = -389664124;    double pTCrxDEBTI95092004 = -354626569;    double pTCrxDEBTI48804359 = -75955606;    double pTCrxDEBTI28888772 = -679522693;    double pTCrxDEBTI24254800 = -388663566;    double pTCrxDEBTI42319805 = -543505615;    double pTCrxDEBTI97732742 = -616441663;    double pTCrxDEBTI96978310 = -260044975;    double pTCrxDEBTI59358552 = -123577048;    double pTCrxDEBTI88716434 = -688291205;    double pTCrxDEBTI86927157 = -971912646;    double pTCrxDEBTI44658052 = -415446678;    double pTCrxDEBTI38821153 = -836926923;    double pTCrxDEBTI54989873 = -592781568;    double pTCrxDEBTI40227634 = -348170431;    double pTCrxDEBTI6162594 = -143979077;    double pTCrxDEBTI56906802 = -406981455;    double pTCrxDEBTI60671433 = -924462822;    double pTCrxDEBTI351262 = 2338814;    double pTCrxDEBTI29966584 = -888339929;    double pTCrxDEBTI83537906 = 46688068;    double pTCrxDEBTI33271582 = -896947994;    double pTCrxDEBTI55041988 = -597222962;    double pTCrxDEBTI74301390 = -631406454;    double pTCrxDEBTI57500234 = -62539543;    double pTCrxDEBTI23189643 = 31061509;    double pTCrxDEBTI42000788 = -935792604;    double pTCrxDEBTI82828457 = 89053726;    double pTCrxDEBTI29191789 = -416325756;    double pTCrxDEBTI77706991 = -306394913;    double pTCrxDEBTI96370019 = -36365281;    double pTCrxDEBTI67794552 = -138934683;    double pTCrxDEBTI77011577 = -877657596;    double pTCrxDEBTI16109746 = -293937486;    double pTCrxDEBTI19492054 = -132280397;    double pTCrxDEBTI47526986 = -85486269;    double pTCrxDEBTI93466348 = -576556133;    double pTCrxDEBTI11229152 = -649548711;    double pTCrxDEBTI81365853 = -941221880;    double pTCrxDEBTI23804492 = -460666027;    double pTCrxDEBTI90907930 = -479407843;    double pTCrxDEBTI81860635 = -376184441;    double pTCrxDEBTI18322239 = -135363737;    double pTCrxDEBTI68133183 = -364660361;    double pTCrxDEBTI14623247 = 93795109;    double pTCrxDEBTI23131555 = -402306656;    double pTCrxDEBTI81381718 = -954475896;    double pTCrxDEBTI19501855 = -968971674;    double pTCrxDEBTI21834949 = -768521915;    double pTCrxDEBTI68272284 = -497680701;    double pTCrxDEBTI87177140 = -756978450;    double pTCrxDEBTI14174520 = -920721922;    double pTCrxDEBTI43815409 = -498961358;    double pTCrxDEBTI18733467 = -278005621;    double pTCrxDEBTI30014434 = -996291006;    double pTCrxDEBTI17502739 = -492716130;    double pTCrxDEBTI40050016 = -757403608;    double pTCrxDEBTI74502968 = -444549153;    double pTCrxDEBTI71388537 = -516983151;    double pTCrxDEBTI1065158 = -319725075;    double pTCrxDEBTI319017 = -607713011;    double pTCrxDEBTI14904286 = -605495389;    double pTCrxDEBTI67786521 = -843719220;    double pTCrxDEBTI81651561 = -817182136;    double pTCrxDEBTI92346415 = -551925925;    double pTCrxDEBTI19132605 = -732977963;    double pTCrxDEBTI67646475 = -537789083;    double pTCrxDEBTI22711407 = -442989438;    double pTCrxDEBTI35497819 = -360501171;    double pTCrxDEBTI92700647 = -162684163;    double pTCrxDEBTI12696245 = -567422945;    double pTCrxDEBTI45677651 = -757432745;    double pTCrxDEBTI79305580 = -983240943;    double pTCrxDEBTI76546770 = -536995159;    double pTCrxDEBTI39058653 = -308932087;    double pTCrxDEBTI1677271 = -577127492;    double pTCrxDEBTI14949343 = -661584258;    double pTCrxDEBTI86908805 = -132562601;    double pTCrxDEBTI59678143 = -625201563;    double pTCrxDEBTI34368680 = -660232888;    double pTCrxDEBTI41807924 = -14462596;    double pTCrxDEBTI22498933 = -966820931;    double pTCrxDEBTI60993509 = -142424360;    double pTCrxDEBTI60919505 = -918645055;    double pTCrxDEBTI90529850 = -549416464;    double pTCrxDEBTI82195499 = -115643359;    double pTCrxDEBTI23979144 = -639973326;    double pTCrxDEBTI58278110 = -499651975;    double pTCrxDEBTI86095312 = -297646480;    double pTCrxDEBTI1989315 = -639564267;    double pTCrxDEBTI7476970 = -328082661;    double pTCrxDEBTI18963381 = -32006981;    double pTCrxDEBTI39840615 = -32565560;    double pTCrxDEBTI80300696 = -521496805;    double pTCrxDEBTI23485475 = -852953017;    double pTCrxDEBTI76003645 = -873912454;    double pTCrxDEBTI14074115 = -532465222;    double pTCrxDEBTI36670678 = -318181602;    double pTCrxDEBTI75786768 = -812734437;    double pTCrxDEBTI95490642 = -389664124;     pTCrxDEBTI50774321 = pTCrxDEBTI95092004;     pTCrxDEBTI95092004 = pTCrxDEBTI48804359;     pTCrxDEBTI48804359 = pTCrxDEBTI28888772;     pTCrxDEBTI28888772 = pTCrxDEBTI24254800;     pTCrxDEBTI24254800 = pTCrxDEBTI42319805;     pTCrxDEBTI42319805 = pTCrxDEBTI97732742;     pTCrxDEBTI97732742 = pTCrxDEBTI96978310;     pTCrxDEBTI96978310 = pTCrxDEBTI59358552;     pTCrxDEBTI59358552 = pTCrxDEBTI88716434;     pTCrxDEBTI88716434 = pTCrxDEBTI86927157;     pTCrxDEBTI86927157 = pTCrxDEBTI44658052;     pTCrxDEBTI44658052 = pTCrxDEBTI38821153;     pTCrxDEBTI38821153 = pTCrxDEBTI54989873;     pTCrxDEBTI54989873 = pTCrxDEBTI40227634;     pTCrxDEBTI40227634 = pTCrxDEBTI6162594;     pTCrxDEBTI6162594 = pTCrxDEBTI56906802;     pTCrxDEBTI56906802 = pTCrxDEBTI60671433;     pTCrxDEBTI60671433 = pTCrxDEBTI351262;     pTCrxDEBTI351262 = pTCrxDEBTI29966584;     pTCrxDEBTI29966584 = pTCrxDEBTI83537906;     pTCrxDEBTI83537906 = pTCrxDEBTI33271582;     pTCrxDEBTI33271582 = pTCrxDEBTI55041988;     pTCrxDEBTI55041988 = pTCrxDEBTI74301390;     pTCrxDEBTI74301390 = pTCrxDEBTI57500234;     pTCrxDEBTI57500234 = pTCrxDEBTI23189643;     pTCrxDEBTI23189643 = pTCrxDEBTI42000788;     pTCrxDEBTI42000788 = pTCrxDEBTI82828457;     pTCrxDEBTI82828457 = pTCrxDEBTI29191789;     pTCrxDEBTI29191789 = pTCrxDEBTI77706991;     pTCrxDEBTI77706991 = pTCrxDEBTI96370019;     pTCrxDEBTI96370019 = pTCrxDEBTI67794552;     pTCrxDEBTI67794552 = pTCrxDEBTI77011577;     pTCrxDEBTI77011577 = pTCrxDEBTI16109746;     pTCrxDEBTI16109746 = pTCrxDEBTI19492054;     pTCrxDEBTI19492054 = pTCrxDEBTI47526986;     pTCrxDEBTI47526986 = pTCrxDEBTI93466348;     pTCrxDEBTI93466348 = pTCrxDEBTI11229152;     pTCrxDEBTI11229152 = pTCrxDEBTI81365853;     pTCrxDEBTI81365853 = pTCrxDEBTI23804492;     pTCrxDEBTI23804492 = pTCrxDEBTI90907930;     pTCrxDEBTI90907930 = pTCrxDEBTI81860635;     pTCrxDEBTI81860635 = pTCrxDEBTI18322239;     pTCrxDEBTI18322239 = pTCrxDEBTI68133183;     pTCrxDEBTI68133183 = pTCrxDEBTI14623247;     pTCrxDEBTI14623247 = pTCrxDEBTI23131555;     pTCrxDEBTI23131555 = pTCrxDEBTI81381718;     pTCrxDEBTI81381718 = pTCrxDEBTI19501855;     pTCrxDEBTI19501855 = pTCrxDEBTI21834949;     pTCrxDEBTI21834949 = pTCrxDEBTI68272284;     pTCrxDEBTI68272284 = pTCrxDEBTI87177140;     pTCrxDEBTI87177140 = pTCrxDEBTI14174520;     pTCrxDEBTI14174520 = pTCrxDEBTI43815409;     pTCrxDEBTI43815409 = pTCrxDEBTI18733467;     pTCrxDEBTI18733467 = pTCrxDEBTI30014434;     pTCrxDEBTI30014434 = pTCrxDEBTI17502739;     pTCrxDEBTI17502739 = pTCrxDEBTI40050016;     pTCrxDEBTI40050016 = pTCrxDEBTI74502968;     pTCrxDEBTI74502968 = pTCrxDEBTI71388537;     pTCrxDEBTI71388537 = pTCrxDEBTI1065158;     pTCrxDEBTI1065158 = pTCrxDEBTI319017;     pTCrxDEBTI319017 = pTCrxDEBTI14904286;     pTCrxDEBTI14904286 = pTCrxDEBTI67786521;     pTCrxDEBTI67786521 = pTCrxDEBTI81651561;     pTCrxDEBTI81651561 = pTCrxDEBTI92346415;     pTCrxDEBTI92346415 = pTCrxDEBTI19132605;     pTCrxDEBTI19132605 = pTCrxDEBTI67646475;     pTCrxDEBTI67646475 = pTCrxDEBTI22711407;     pTCrxDEBTI22711407 = pTCrxDEBTI35497819;     pTCrxDEBTI35497819 = pTCrxDEBTI92700647;     pTCrxDEBTI92700647 = pTCrxDEBTI12696245;     pTCrxDEBTI12696245 = pTCrxDEBTI45677651;     pTCrxDEBTI45677651 = pTCrxDEBTI79305580;     pTCrxDEBTI79305580 = pTCrxDEBTI76546770;     pTCrxDEBTI76546770 = pTCrxDEBTI39058653;     pTCrxDEBTI39058653 = pTCrxDEBTI1677271;     pTCrxDEBTI1677271 = pTCrxDEBTI14949343;     pTCrxDEBTI14949343 = pTCrxDEBTI86908805;     pTCrxDEBTI86908805 = pTCrxDEBTI59678143;     pTCrxDEBTI59678143 = pTCrxDEBTI34368680;     pTCrxDEBTI34368680 = pTCrxDEBTI41807924;     pTCrxDEBTI41807924 = pTCrxDEBTI22498933;     pTCrxDEBTI22498933 = pTCrxDEBTI60993509;     pTCrxDEBTI60993509 = pTCrxDEBTI60919505;     pTCrxDEBTI60919505 = pTCrxDEBTI90529850;     pTCrxDEBTI90529850 = pTCrxDEBTI82195499;     pTCrxDEBTI82195499 = pTCrxDEBTI23979144;     pTCrxDEBTI23979144 = pTCrxDEBTI58278110;     pTCrxDEBTI58278110 = pTCrxDEBTI86095312;     pTCrxDEBTI86095312 = pTCrxDEBTI1989315;     pTCrxDEBTI1989315 = pTCrxDEBTI7476970;     pTCrxDEBTI7476970 = pTCrxDEBTI18963381;     pTCrxDEBTI18963381 = pTCrxDEBTI39840615;     pTCrxDEBTI39840615 = pTCrxDEBTI80300696;     pTCrxDEBTI80300696 = pTCrxDEBTI23485475;     pTCrxDEBTI23485475 = pTCrxDEBTI76003645;     pTCrxDEBTI76003645 = pTCrxDEBTI14074115;     pTCrxDEBTI14074115 = pTCrxDEBTI36670678;     pTCrxDEBTI36670678 = pTCrxDEBTI75786768;     pTCrxDEBTI75786768 = pTCrxDEBTI95490642;     pTCrxDEBTI95490642 = pTCrxDEBTI50774321;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void SUYFFaTZrQ67968553() {     double yvYDRBvjnM21425989 = 92186745;    double yvYDRBvjnM63765522 = -226269868;    double yvYDRBvjnM28773278 = -518250066;    double yvYDRBvjnM68820058 = -869661083;    double yvYDRBvjnM45705122 = -172069913;    double yvYDRBvjnM32319568 = -245086766;    double yvYDRBvjnM16111639 = -615217373;    double yvYDRBvjnM53820691 = -606862064;    double yvYDRBvjnM5223607 = -216924376;    double yvYDRBvjnM11682896 = -989386556;    double yvYDRBvjnM29176319 = 93185425;    double yvYDRBvjnM90204623 = -731210471;    double yvYDRBvjnM18648540 = -746463981;    double yvYDRBvjnM38836907 = -393809778;    double yvYDRBvjnM22947653 = -497920434;    double yvYDRBvjnM38691234 = -813962664;    double yvYDRBvjnM50759629 = -316166585;    double yvYDRBvjnM80964928 = -913629018;    double yvYDRBvjnM82059043 = -704849128;    double yvYDRBvjnM88753293 = -330706234;    double yvYDRBvjnM38673661 = -961991073;    double yvYDRBvjnM94667347 = -128233300;    double yvYDRBvjnM56985797 = -65066106;    double yvYDRBvjnM8150928 = -223807360;    double yvYDRBvjnM21624623 = -838444230;    double yvYDRBvjnM53787223 = -67222936;    double yvYDRBvjnM50261574 = -230970379;    double yvYDRBvjnM90610682 = -416070042;    double yvYDRBvjnM61334277 = -710683972;    double yvYDRBvjnM16879087 = -864789983;    double yvYDRBvjnM3442016 = -884798966;    double yvYDRBvjnM12306014 = -467231846;    double yvYDRBvjnM59956938 = -824619205;    double yvYDRBvjnM81474721 = -957467170;    double yvYDRBvjnM97516290 = 20772824;    double yvYDRBvjnM7520245 = -164712817;    double yvYDRBvjnM37257835 = -765149341;    double yvYDRBvjnM64021640 = -433661818;    double yvYDRBvjnM59899142 = -896950517;    double yvYDRBvjnM18907790 = 21725610;    double yvYDRBvjnM50981215 = -863426721;    double yvYDRBvjnM10343627 = -145573510;    double yvYDRBvjnM67289199 = -122117789;    double yvYDRBvjnM74641484 = -436635333;    double yvYDRBvjnM10577306 = -214851286;    double yvYDRBvjnM29537389 = -160643811;    double yvYDRBvjnM7512922 = 29454305;    double yvYDRBvjnM85116065 = -321101969;    double yvYDRBvjnM27276401 = -754285884;    double yvYDRBvjnM76699770 = -780689580;    double yvYDRBvjnM55366464 = -303550571;    double yvYDRBvjnM56046024 = -889941644;    double yvYDRBvjnM98891065 = -426759400;    double yvYDRBvjnM8960772 = -938887118;    double yvYDRBvjnM29294893 = -7231920;    double yvYDRBvjnM26758641 = -779579955;    double yvYDRBvjnM6779725 = -61203762;    double yvYDRBvjnM20622351 = -194442706;    double yvYDRBvjnM47195436 = 68783147;    double yvYDRBvjnM91917898 = -4846978;    double yvYDRBvjnM82057993 = 85883613;    double yvYDRBvjnM25500956 = -99147332;    double yvYDRBvjnM92486414 = -896178093;    double yvYDRBvjnM88344519 = -352134394;    double yvYDRBvjnM8240880 = -4587590;    double yvYDRBvjnM16870305 = -439582729;    double yvYDRBvjnM30247685 = -906591267;    double yvYDRBvjnM37173819 = -788996812;    double yvYDRBvjnM41320616 = -314582602;    double yvYDRBvjnM15427408 = -233207618;    double yvYDRBvjnM1433400 = 51186676;    double yvYDRBvjnM86737988 = -882504767;    double yvYDRBvjnM21065786 = 83321498;    double yvYDRBvjnM63151253 = -626574739;    double yvYDRBvjnM37772079 = -467279513;    double yvYDRBvjnM28330034 = -716417563;    double yvYDRBvjnM27378148 = 93884489;    double yvYDRBvjnM82344313 = -628430774;    double yvYDRBvjnM97573621 = 91043925;    double yvYDRBvjnM92087233 = -577800420;    double yvYDRBvjnM46274302 = 3322759;    double yvYDRBvjnM65145508 = -909868411;    double yvYDRBvjnM63334282 = -661784158;    double yvYDRBvjnM84634506 = -929994393;    double yvYDRBvjnM61512622 = -461239413;    double yvYDRBvjnM47395992 = -994857323;    double yvYDRBvjnM13414948 = 59527553;    double yvYDRBvjnM50996167 = -885732087;    double yvYDRBvjnM52179828 = -850235250;    double yvYDRBvjnM70757649 = -199647222;    double yvYDRBvjnM740520 = -3509055;    double yvYDRBvjnM16635484 = -470706635;    double yvYDRBvjnM16826205 = -402444966;    double yvYDRBvjnM67981244 = -792103539;    double yvYDRBvjnM36849796 = 35841997;    double yvYDRBvjnM25480259 = -664279390;    double yvYDRBvjnM17857213 = -249395418;    double yvYDRBvjnM78944679 = -769983396;    double yvYDRBvjnM66400604 = -332047743;    double yvYDRBvjnM93707000 = 92186745;     yvYDRBvjnM21425989 = yvYDRBvjnM63765522;     yvYDRBvjnM63765522 = yvYDRBvjnM28773278;     yvYDRBvjnM28773278 = yvYDRBvjnM68820058;     yvYDRBvjnM68820058 = yvYDRBvjnM45705122;     yvYDRBvjnM45705122 = yvYDRBvjnM32319568;     yvYDRBvjnM32319568 = yvYDRBvjnM16111639;     yvYDRBvjnM16111639 = yvYDRBvjnM53820691;     yvYDRBvjnM53820691 = yvYDRBvjnM5223607;     yvYDRBvjnM5223607 = yvYDRBvjnM11682896;     yvYDRBvjnM11682896 = yvYDRBvjnM29176319;     yvYDRBvjnM29176319 = yvYDRBvjnM90204623;     yvYDRBvjnM90204623 = yvYDRBvjnM18648540;     yvYDRBvjnM18648540 = yvYDRBvjnM38836907;     yvYDRBvjnM38836907 = yvYDRBvjnM22947653;     yvYDRBvjnM22947653 = yvYDRBvjnM38691234;     yvYDRBvjnM38691234 = yvYDRBvjnM50759629;     yvYDRBvjnM50759629 = yvYDRBvjnM80964928;     yvYDRBvjnM80964928 = yvYDRBvjnM82059043;     yvYDRBvjnM82059043 = yvYDRBvjnM88753293;     yvYDRBvjnM88753293 = yvYDRBvjnM38673661;     yvYDRBvjnM38673661 = yvYDRBvjnM94667347;     yvYDRBvjnM94667347 = yvYDRBvjnM56985797;     yvYDRBvjnM56985797 = yvYDRBvjnM8150928;     yvYDRBvjnM8150928 = yvYDRBvjnM21624623;     yvYDRBvjnM21624623 = yvYDRBvjnM53787223;     yvYDRBvjnM53787223 = yvYDRBvjnM50261574;     yvYDRBvjnM50261574 = yvYDRBvjnM90610682;     yvYDRBvjnM90610682 = yvYDRBvjnM61334277;     yvYDRBvjnM61334277 = yvYDRBvjnM16879087;     yvYDRBvjnM16879087 = yvYDRBvjnM3442016;     yvYDRBvjnM3442016 = yvYDRBvjnM12306014;     yvYDRBvjnM12306014 = yvYDRBvjnM59956938;     yvYDRBvjnM59956938 = yvYDRBvjnM81474721;     yvYDRBvjnM81474721 = yvYDRBvjnM97516290;     yvYDRBvjnM97516290 = yvYDRBvjnM7520245;     yvYDRBvjnM7520245 = yvYDRBvjnM37257835;     yvYDRBvjnM37257835 = yvYDRBvjnM64021640;     yvYDRBvjnM64021640 = yvYDRBvjnM59899142;     yvYDRBvjnM59899142 = yvYDRBvjnM18907790;     yvYDRBvjnM18907790 = yvYDRBvjnM50981215;     yvYDRBvjnM50981215 = yvYDRBvjnM10343627;     yvYDRBvjnM10343627 = yvYDRBvjnM67289199;     yvYDRBvjnM67289199 = yvYDRBvjnM74641484;     yvYDRBvjnM74641484 = yvYDRBvjnM10577306;     yvYDRBvjnM10577306 = yvYDRBvjnM29537389;     yvYDRBvjnM29537389 = yvYDRBvjnM7512922;     yvYDRBvjnM7512922 = yvYDRBvjnM85116065;     yvYDRBvjnM85116065 = yvYDRBvjnM27276401;     yvYDRBvjnM27276401 = yvYDRBvjnM76699770;     yvYDRBvjnM76699770 = yvYDRBvjnM55366464;     yvYDRBvjnM55366464 = yvYDRBvjnM56046024;     yvYDRBvjnM56046024 = yvYDRBvjnM98891065;     yvYDRBvjnM98891065 = yvYDRBvjnM8960772;     yvYDRBvjnM8960772 = yvYDRBvjnM29294893;     yvYDRBvjnM29294893 = yvYDRBvjnM26758641;     yvYDRBvjnM26758641 = yvYDRBvjnM6779725;     yvYDRBvjnM6779725 = yvYDRBvjnM20622351;     yvYDRBvjnM20622351 = yvYDRBvjnM47195436;     yvYDRBvjnM47195436 = yvYDRBvjnM91917898;     yvYDRBvjnM91917898 = yvYDRBvjnM82057993;     yvYDRBvjnM82057993 = yvYDRBvjnM25500956;     yvYDRBvjnM25500956 = yvYDRBvjnM92486414;     yvYDRBvjnM92486414 = yvYDRBvjnM88344519;     yvYDRBvjnM88344519 = yvYDRBvjnM8240880;     yvYDRBvjnM8240880 = yvYDRBvjnM16870305;     yvYDRBvjnM16870305 = yvYDRBvjnM30247685;     yvYDRBvjnM30247685 = yvYDRBvjnM37173819;     yvYDRBvjnM37173819 = yvYDRBvjnM41320616;     yvYDRBvjnM41320616 = yvYDRBvjnM15427408;     yvYDRBvjnM15427408 = yvYDRBvjnM1433400;     yvYDRBvjnM1433400 = yvYDRBvjnM86737988;     yvYDRBvjnM86737988 = yvYDRBvjnM21065786;     yvYDRBvjnM21065786 = yvYDRBvjnM63151253;     yvYDRBvjnM63151253 = yvYDRBvjnM37772079;     yvYDRBvjnM37772079 = yvYDRBvjnM28330034;     yvYDRBvjnM28330034 = yvYDRBvjnM27378148;     yvYDRBvjnM27378148 = yvYDRBvjnM82344313;     yvYDRBvjnM82344313 = yvYDRBvjnM97573621;     yvYDRBvjnM97573621 = yvYDRBvjnM92087233;     yvYDRBvjnM92087233 = yvYDRBvjnM46274302;     yvYDRBvjnM46274302 = yvYDRBvjnM65145508;     yvYDRBvjnM65145508 = yvYDRBvjnM63334282;     yvYDRBvjnM63334282 = yvYDRBvjnM84634506;     yvYDRBvjnM84634506 = yvYDRBvjnM61512622;     yvYDRBvjnM61512622 = yvYDRBvjnM47395992;     yvYDRBvjnM47395992 = yvYDRBvjnM13414948;     yvYDRBvjnM13414948 = yvYDRBvjnM50996167;     yvYDRBvjnM50996167 = yvYDRBvjnM52179828;     yvYDRBvjnM52179828 = yvYDRBvjnM70757649;     yvYDRBvjnM70757649 = yvYDRBvjnM740520;     yvYDRBvjnM740520 = yvYDRBvjnM16635484;     yvYDRBvjnM16635484 = yvYDRBvjnM16826205;     yvYDRBvjnM16826205 = yvYDRBvjnM67981244;     yvYDRBvjnM67981244 = yvYDRBvjnM36849796;     yvYDRBvjnM36849796 = yvYDRBvjnM25480259;     yvYDRBvjnM25480259 = yvYDRBvjnM17857213;     yvYDRBvjnM17857213 = yvYDRBvjnM78944679;     yvYDRBvjnM78944679 = yvYDRBvjnM66400604;     yvYDRBvjnM66400604 = yvYDRBvjnM93707000;     yvYDRBvjnM93707000 = yvYDRBvjnM21425989;}
// Junk Finished
