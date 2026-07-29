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

define_dummy_symbol(mmcityinfo_playercfg);

#include "playercfg.h"

mmPlayerConfig::mmPlayerConfig()
{}

mmPlayerConfig::~mmPlayerConfig()
{}

void mmPlayerConfig::GetAudio()
{}

void mmPlayerConfig::GetControls()
{}

void mmPlayerConfig::GetGraphics()
{}

void mmPlayerConfig::GetViewSettings()
{}

b32 mmPlayerConfig::Save(char* /*path*/, b32 /*binary*/)
{
    return true;
}

void mmPlayerConfig::DefaultViewSettings()
{
    ViewSettings.IconsState = 1;
    ViewSettings.CameraIndex = 2; // Far Camera
    ViewSettings.HudmapMode = 0;
    ViewSettings.WideView = 0;
    ViewSettings.DashView = 0;
    ViewSettings.EnableMirror = 1;
    ViewSettings.ExternalView = 1; // Speedometer
    ViewSettings.MapRes = 0;
}
