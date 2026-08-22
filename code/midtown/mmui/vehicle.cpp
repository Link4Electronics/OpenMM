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

define_dummy_symbol(mmui_vehicle);

#include "vehicle.h"

#include "mmwidget/manager.h"

Vehicle::Vehicle(i32 arg1)
    : VehicleSelectBase(arg1)
{
    AddBMButton(IDC_VEHICLE_DRIVE, "vehi_play"_xconst, 0.55f, 0.88f, 5);

    InitCarSelection(1, 34.0f / 640.0f, 175.0f / 480.0f, 290.0f / 640.0f, 216.0f / 480.0f);
}

void Vehicle::PreSetup()
{
    AssignBackground("veh_back");

    // Original calls InitCarSelection once from constructor, but at that point
    // VehicleListPtr may not be loaded yet. Call here as well as a safety net
    // (idempotent guard inside InitCarSelection prevents double allocation).
    InitCarSelection(1, 34.0f / 640.0f, 175.0f / 480.0f, 290.0f / 640.0f, 216.0f / 480.0f);

    VehicleSelectBase::PreSetup();
}

void Vehicle::PostSetup()
{
    VehicleSelectBase::PostSetup();
}

void Vehicle::SetSubMenu(i32 /*arg1*/)
{}

void Vehicle::SetSubMenuButtons()
{}
