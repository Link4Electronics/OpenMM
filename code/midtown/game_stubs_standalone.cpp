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

// Standalone weak stubs for symbols not yet reimplemented.
// Only include the minimum necessary headers to avoid incomplete-type errors.

#include "mmai/aiGoalFollowWayPts.h"
#include "mmai/aiMap.h"
#include "mmai/aiStuck.h"
#include "mmai/aiVehicleMGR.h"
#include "mmcar/carsimcheap.h"
#include "mmcar/skid.h"
#include "mmcityinfo/playercfg.h"
#include "mmgame/player.h"
#include "mminput/input.h"

// --- aiStuck ---
// Constructor (ARTS_IMPORT, no strong def)
__attribute__((weak)) aiStuck::aiStuck() {}

// AddWidgets is the key function (first non-inline virtual, present with ARTS_DEV_BUILD).
// Defining it triggers vtable emission.
__attribute__((weak)) void aiStuck::AddWidgets(Bank* /*arg1*/) {}

// Reset, Update are vtable entries
__attribute__((weak)) void aiStuck::Reset() {}

// --- aiGoalFollowWayPts ---
// GetClass is the key function (first virtual; ~aiGoalFollowWayPts is non-virtual
// because base aiGoal has no virtual destructor).
// Priority and Reset are vtable entries.
__attribute__((weak)) MetaClass* aiGoalFollowWayPts::GetClass() { return nullptr; }
__attribute__((weak)) b32 aiGoalFollowWayPts::Priority() { return {}; }
__attribute__((weak)) void aiGoalFollowWayPts::Reset() {}

// --- aiMap ---
// Constructor (called from static init in aiMap.cpp)
__attribute__((weak)) aiMap::aiMap() {}

// AddWidgets is the key function (first non-inline virtual with ARTS_DEV_BUILD)
__attribute__((weak)) void aiMap::AddWidgets(Bank* /*bank*/) {}

// --- mmWheelCheap ---
// Constructor + Reset (key function for vtable)
__attribute__((weak)) mmWheelCheap::mmWheelCheap() {}
__attribute__((weak)) void mmWheelCheap::Reset() {}
__attribute__((weak)) void mmWheelCheap::Update() {}

// --- aiVehicleActive ---
// Vtable entries (destructor is key function, defined in aiVehicleMGR.cpp)
__attribute__((weak)) void aiVehicleActive::Update() {}
__attribute__((weak)) void aiVehicleActive::PostUpdate() {}
__attribute__((weak)) asBound* aiVehicleActive::GetBound() { return nullptr; }
__attribute__((weak)) asInertialCS* aiVehicleActive::GetICS() { return nullptr; }

// --- mmSkidManager ---
__attribute__((weak)) MetaClass* mmSkidManager::GetClass() { return nullptr; }

// --- mmCtrlCFG ---
__attribute__((weak)) mmCtrlCFG::~mmCtrlCFG() {}

// --- mmPlayerConfig ---
__attribute__((weak)) MetaClass* mmPlayerConfig::GetClass() { return nullptr; }

// --- mmInput ---
__attribute__((weak)) MetaClass* mmInput::GetClass() { return nullptr; }
__attribute__((weak)) void mmInput::AddWidgets(Bank* /*arg1*/) {}

// --- mmPlayer virtual function stubs (for vtable in player.cpp) ---
__attribute__((weak)) MetaClass* mmPlayer::GetClass() { return nullptr; }
__attribute__((weak)) void mmPlayer::Update() {}
__attribute__((weak)) void mmPlayer::Reset() {}
