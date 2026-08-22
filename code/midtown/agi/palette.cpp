/*
    Open1560 - An Open Source Re-Implementation of Midtown Madness 1 Beta
    Copyright (C) 2020 Brick

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

define_dummy_symbol(agi_palette);

#include "palette.h"

#include "agi/rgba.h"

// FUNCTION: MIDTOWN 0x0053DFB0
agiPalette::agiPalette()
{}

// FUNCTION: MIDTOWN 0x0053E2C0
u32 agiPalette::FindColor(agiRgba /*arg1*/)
{
    return 0;
}

// FUNCTION: MIDTOWN 0x0053E230
u32 agiPalette::GetColor(agiRgba /*arg1*/)
{
    return 0;
}

// FUNCTION: MIDTOWN 0x0053DFD0
void agiPalette::Kill()
{
    ColorCount = 0;
    ChangeCount = 0;
}
