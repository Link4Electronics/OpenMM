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

define_dummy_symbol(mmcar_transmission);

#include "transmission.h"

void mmTransmission::Automatic(i32 arg1)
{
    IsAutomatic = arg1 != 0;
}

i32 mmTransmission::Downshift()
{
    if (CurrentGear > 1)
    {
        CurrentGear--;
        GearChanged = true;
    }
    return CurrentGear;
}

i32 mmTransmission::GetCurrentGear()
{
    return CurrentGear;
}

i32 mmTransmission::SetCurrentGear(i32 arg1)
{
    CurrentGear = arg1;
    GearChanged = true;
    return CurrentGear;
}

void mmTransmission::SetDrive()
{
    CurrentGear = 2;
    InPark = false;
    GearChanged = true;
}

void mmTransmission::SetReverse()
{
    CurrentGear = 0;
    InPark = false;
    GearChanged = true;
}

i32 mmTransmission::Upshift()
{
    if (CurrentGear < NumGears)
    {
        CurrentGear++;
        GearChanged = true;
    }
    return CurrentGear;
}
