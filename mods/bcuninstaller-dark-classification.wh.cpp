// ==WindhawkMod==
// @id              bcuninstaller-dark-classification
// @name            BCUninstaller Dark Classification
// @description     Converts BCUninstaller classification colors to dark colors.
// @version         1.0.0
// @author          moobd3
// @github          https://github.com/moobd3
// @include         BCUninstaller.exe
// @compilerOptions -lgdiplus
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# BCUninstaller Dark Classification

Darkens the bright classification highlight colors used by
[Bulk Crap Uninstaller](https://github.com/Klocman/Bulk-Crap-Uninstaller)
to make them better suited for dark mode.

## What it does

BCUninstaller uses light pastel colors to visually classify
installed applications.

This mod converts those bright colors into darker equivalents
while keeping the different classification colors visually
distinct.

### Color mapping

- Light Cyan → Dark Cyan
- Light Green → Dark Green
- Light Blue → Dark Blue
- Light Red/Pink → Dark Red
- Light Yellow → Dark Yellow
- Light Purple → Dark Purple

## Compatibility

The mod targets `BCUninstaller.exe` only.

It hooks GDI+ solid-brush color creation and changes the
classification colors before they are rendered.

No BCUninstaller files are modified.

## Installation

Install the mod through Windhawk and restart BCUninstaller.

## Credits

Created by @moobd3 with AI-assisted development.
*/
// ==/WindhawkModReadme==

#include <windows.h>
#include <gdiplus.h>

using namespace Gdiplus;

// ============================================================
// GDI+ function types
// ============================================================

using GdipCreateSolidFill_t =
    decltype(&DllExports::GdipCreateSolidFill);

using GdipSetSolidFillColor_t =
    decltype(&DllExports::GdipSetSolidFillColor);

static GdipCreateSolidFill_t
    GdipCreateSolidFill_Original = nullptr;

static GdipSetSolidFillColor_t
    GdipSetSolidFillColor_Original = nullptr;

// ============================================================
// Convert BCUninstaller classification colors
// ============================================================

static ARGB DarkenClassificationColor(ARGB color)
{
    BYTE alpha = (BYTE)((color >> 24) & 0xFF);
    BYTE red   = (BYTE)((color >> 16) & 0xFF);
    BYTE green = (BYTE)((color >> 8) & 0xFF);
    BYTE blue  = (BYTE)(color & 0xFF);

    // ========================================================
    // Exact BCUninstaller classification colors
    // ========================================================

    // Light Cyan
    // #A3FFFF
    if (red == 0xA3 &&
        green == 0xFF &&
        blue == 0xFF)
    {
        return Color::MakeARGB(
            alpha,
            0x16,
            0x4E,
            0x63
        );
    }

    // Light Green
    // #CCFFCC
    if (red == 0xCC &&
        green == 0xFF &&
        blue == 0xCC)
    {
        return Color::MakeARGB(
            alpha,
            0x1B,
            0x4D,
            0x2A
        );
    }

    // Light Blue
    // #BBDDFF
    if (red == 0xBB &&
        green == 0xDD &&
        blue == 0xFF)
    {
        return Color::MakeARGB(
            alpha,
            0x1F,
            0x3F,
            0x63
        );
    }

    // ========================================================
    // Other pastel classification colors
    // ========================================================

    // Light Cyan / Aqua
    if (red >= 0x90 &&
        green >= 0xE0 &&
        blue >= 0xE0 &&
        red < 0xD0)
    {
        return Color::MakeARGB(
            alpha,
            0x16,
            0x4E,
            0x63
        );
    }

    // Light Green
    if (red >= 0xB0 &&
        green >= 0xE0 &&
        blue >= 0xB0 &&
        blue < 0xE0)
    {
        return Color::MakeARGB(
            alpha,
            0x1B,
            0x4D,
            0x2A
        );
    }

    // Light Blue
    if (red >= 0xA0 &&
        green >= 0xC0 &&
        blue >= 0xE0)
    {
        return Color::MakeARGB(
            alpha,
            0x1F,
            0x3F,
            0x63
        );
    }

    // Light Red / Pink
    if (red >= 0xE0 &&
        green >= 0xA0 &&
        blue >= 0xA0)
    {
        return Color::MakeARGB(
            alpha,
            0x62,
            0x24,
            0x2E
        );
    }

    // Light Yellow
    if (red >= 0xE0 &&
        green >= 0xD0 &&
        blue <= 0xB0)
    {
        return Color::MakeARGB(
            alpha,
            0x5A,
            0x4A,
            0x1F
        );
    }

    // Light Purple
    if (red >= 0xC0 &&
        blue >= 0xC0 &&
        green <= 0xD0)
    {
        return Color::MakeARGB(
            alpha,
            0x4A,
            0x2F,
            0x5A
        );
    }

    // Not a classification color.
    return color;
}

// ============================================================
// GdipCreateSolidFill hook
// ============================================================

GpStatus WINAPI GdipCreateSolidFill_Hook(
    ARGB color,
    GpSolidFill** brush)
{
    ARGB newColor =
        DarkenClassificationColor(color);

    if (newColor != color)
    {
        Wh_Log(
            L"BCUninstaller classification color: "
            L"%08X -> %08X",
            color,
            newColor
        );
    }

    return GdipCreateSolidFill_Original(
        newColor,
        brush
    );
}

// ============================================================
// GdipSetSolidFillColor hook
// ============================================================

GpStatus WINAPI GdipSetSolidFillColor_Hook(
    GpSolidFill* brush,
    ARGB color)
{
    ARGB newColor =
        DarkenClassificationColor(color);

    if (newColor != color)
    {
        Wh_Log(
            L"BCUninstaller classification color update: "
            L"%08X -> %08X",
            color,
            newColor
        );
    }

    return GdipSetSolidFillColor_Original(
        brush,
        newColor
    );
}

// ============================================================
// Windhawk initialization
// ============================================================

BOOL Wh_ModInit()
{
    Wh_Log(
        L"BCUninstaller Dark Classification: Init"
    );

    HMODULE gdiplus =
        LoadLibraryW(L"gdiplus.dll");

    if (!gdiplus)
    {
        Wh_Log(
            L"Failed to load gdiplus.dll"
        );

        return FALSE;
    }

    // --------------------------------------------------------
    // Find GdipCreateSolidFill
    // --------------------------------------------------------

    FARPROC createSolidFill =
        GetProcAddress(
            gdiplus,
            "GdipCreateSolidFill"
        );

    if (!createSolidFill)
    {
        Wh_Log(
            L"GdipCreateSolidFill not found"
        );

        return FALSE;
    }

    // --------------------------------------------------------
    // Find GdipSetSolidFillColor
    // --------------------------------------------------------

    FARPROC setSolidFillColor =
        GetProcAddress(
            gdiplus,
            "GdipSetSolidFillColor"
        );

    if (!setSolidFillColor)
    {
        Wh_Log(
            L"GdipSetSolidFillColor not found"
        );

        return FALSE;
    }

    // --------------------------------------------------------
    // Hook GdipCreateSolidFill
    // --------------------------------------------------------

    if (!Wh_SetFunctionHook(
            reinterpret_cast<void*>(
                createSolidFill),
            reinterpret_cast<void*>(
                GdipCreateSolidFill_Hook),
            reinterpret_cast<void**>(
                &GdipCreateSolidFill_Original)))
    {
        Wh_Log(
            L"Failed to hook GdipCreateSolidFill"
        );

        return FALSE;
    }

    // --------------------------------------------------------
    // Hook GdipSetSolidFillColor
    // --------------------------------------------------------

    if (!Wh_SetFunctionHook(
            reinterpret_cast<void*>(
                setSolidFillColor),
            reinterpret_cast<void*>(
                GdipSetSolidFillColor_Hook),
            reinterpret_cast<void**>(
                &GdipSetSolidFillColor_Original)))
    {
        Wh_Log(
            L"Failed to hook GdipSetSolidFillColor"
        );

        return FALSE;
    }

    Wh_Log(
        L"BCUninstaller Dark Classification: Ready"
    );

    return TRUE;
}

// ============================================================
// Windhawk uninitialization
// ============================================================

void Wh_ModUninit()
{
    Wh_Log(
        L"BCUninstaller Dark Classification: Uninit"
    );
}
