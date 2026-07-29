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

define_dummy_symbol(mmcityinfo_cityinfo);

#include "cityinfo.h"

#include "data7/str.h"
#include "stream/stream.h"

mmCityInfo::mmCityInfo() = default;
mmCityInfo::~mmCityInfo() = default;

b32 mmCityInfo::Load(char* path)
{
    Ptr<Stream> file {arts_fopen(path, "r")};

    if (!file)
    {
        Loaded = false;
        return false;
    }

    // .city file format:
    // LocalizedName=<string>
    // MapName=<string>
    // RaceDir=<string>
    // BlitzCount=<int>
    // CircuitCount=<int>
    // CheckpointCount=<int>
    // BlitzNames=<pipe-separated list>
    // CircuitNames=<pipe-separated list>
    // CheckpointNames=<pipe-separated list>
    char blitz_names[512] {};
    char checkpoint_names[512] {};
    char circuit_names[512] {};

    b32 success = true;

    success = success && arts_fscanf(file.get(), "LocalizedName=%[^\r]", LocalizedName) > 0;
    success = success && arts_fscanf(file.get(), "MapName=%s", MapName) > 0;
    success = success && arts_fscanf(file.get(), "RaceDir=%s", RaceDir) > 0;
    success = success && arts_fscanf(file.get(), "BlitzCount=%d", &BlitzCount) > 0;
    success = success && arts_fscanf(file.get(), "CircuitCount=%d", &CircuitCount) > 0;
    success = success && arts_fscanf(file.get(), "CheckpointCount=%d", &CheckpointCount) > 0;
    success = success && arts_fscanf(file.get(), "BlitzNames=%[^\r]", blitz_names) > 0;
    success = success && arts_fscanf(file.get(), "CircuitNames=%[^\r]", circuit_names) > 0;
    success = success && arts_fscanf(file.get(), "CheckpointNames=%[^\r]", checkpoint_names) > 0;

    Loaded = success;

    file = nullptr;

    if (!Loaded)
        return false;

    // Process pipe-separated name strings, counting substrings
    if (BlitzCount)
    {
        string temp(blitz_names);
        BlitzCount = temp.NumSubStrings();
        BlitzNames = ConstString(arts_strdup(blitz_names));
    }

    if (CircuitCount)
    {
        string temp(circuit_names);
        CircuitCount = temp.NumSubStrings();
        CircuitNames = ConstString(arts_strdup(circuit_names));
    }

    if (CheckpointCount)
    {
        string temp(checkpoint_names);
        CheckpointCount = temp.NumSubStrings();
        CheckpointNames = ConstString(arts_strdup(checkpoint_names));
    }

    return true;
}