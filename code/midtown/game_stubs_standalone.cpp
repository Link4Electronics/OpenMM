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

#include "mmai/aiGoalChase.h"
#include "mmai/aiGoalFollowWayPts.h"
#include "mmai/aiGoalRandomDrive.h"
#include "mmai/aiMap.h"
#include "mmai/aiStuck.h"
#include "mmai/aiVehicleMGR.h"
#include "mmai/aiVehicleSpline.h"
#include "mmcar/carsimcheap.h"
#include "mmcar/force.h"
#include "mmcar/skid.h"
#include "mmcity/cullcity.h"
#include "mmcity/inst.h"
#include "mmcityinfo/playercfg.h"
#include "mmgame/player.h"
#include "mminput/input.h"
#include "mmphysics/inertia.h"

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

// --- Stubs for PR ports (aiVehiclePolice::Init, mmForce::Update, facades) ---

// aiGoalChase: ctor/dtor + vtable entries (referenced by aiVehiclePolice::Init)
__attribute__((weak)) aiGoalChase::aiGoalChase(aiVehiclePolice* arg1, aiRailSet* arg2, mmCar** arg3, i16* arg4, Vector3* arg5, i16* arg6) {}
__attribute__((weak)) aiGoalChase::~aiGoalChase() {}
__attribute__((weak)) b32 aiGoalChase::Context() { return {}; }
__attribute__((weak)) void aiGoalChase::Init() {}
__attribute__((weak)) void aiGoalChase::Reset() {}
__attribute__((weak)) void aiGoalChase::Update() {}

// aiGoalRandomDrive: ctor + key functions for vtable emission
__attribute__((weak)) aiGoalRandomDrive::aiGoalRandomDrive(aiRailSet* arg1, aiVehicleSpline* arg2) {}
__attribute__((weak)) b32 aiGoalRandomDrive::Context() { return {}; }
__attribute__((weak)) void aiGoalRandomDrive::Dump() {}
__attribute__((weak)) void aiGoalRandomDrive::Init() {}
__attribute__((weak)) b32 aiGoalRandomDrive::Priority() { return {}; }
__attribute__((weak)) void aiGoalRandomDrive::Reset() {}
__attribute__((weak)) void aiGoalRandomDrive::Update() {}

// aiVehicleSpline
__attribute__((weak)) void aiVehicleSpline::Init(char* /*arg1*/, i32 /*arg2*/) {}

// asInertialCS (used by mmForce::Update)
__attribute__((weak)) void asInertialCS::ApplyForce(const Vector3& /*arg1*/) {}

// mmForce: GetClass is the key function for the vtable
__attribute__((weak)) void mmForce::AddWidgets(Bank* /*arg1*/) {}
__attribute__((weak)) void mmForce::DeclareFields() {}
__attribute__((weak)) MetaClass* mmForce::GetClass() { return nullptr; }

// mmBuildingInstance / mmMatrixInstance / mmShearInstance / mmStaticInstance / mmUpperInstance / mmFacadeInstance
// (referenced by mmCullCity::AddInstance and facade code)
__attribute__((weak)) mmBuildingInstance::mmBuildingInstance() {}
__attribute__((weak)) i32 mmBuildingInstance::Init(char* /*arg1*/, Vector3& /*arg2*/, Vector3& /*arg3*/, Vector3& /*arg4*/) { return {}; }
__attribute__((weak)) MetaClass* mmFacadeInstance::GetClass() { return nullptr; }
__attribute__((weak)) i32 mmMatrixInstance::Init(aconst char* /*arg1*/, Vector3& /*arg2*/, Vector3& /*arg3*/, Vector3& /*arg4*/, i32 /*arg5*/, aconst char* /*arg6*/) { return {}; }
__attribute__((weak)) void ARTS_FASTCALL mmShearInstance::Draw(i32 /*arg1*/) {}
__attribute__((weak)) MetaClass* mmShearInstance::GetClass() { return nullptr; }
__attribute__((weak)) mmShearInstance::mmShearInstance() {}
__attribute__((weak)) i32 mmShearInstance::Init(aconst char* /*arg1*/, Vector3& /*arg2*/, Vector3& /*arg3*/, f32 /*arg4*/, i32 /*arg5*/, aconst char* /*arg6*/) { return {}; }
__attribute__((weak)) f32 ARTS_FASTCALL mmShearInstance::GetScale() { return {}; }
__attribute__((weak)) usize mmShearInstance::SizeOf() { return {}; }
__attribute__((weak)) void mmShearInstance::DeclareFields() {}
__attribute__((weak)) void ARTS_FASTCALL mmStaticInstance::Draw(i32 /*arg1*/) {}
__attribute__((weak)) MetaClass* mmStaticInstance::GetClass() { return nullptr; }
__attribute__((weak)) mmStaticInstance::mmStaticInstance() {}
__attribute__((weak)) void mmStaticInstance::AddWidgets(Bank* /*arg1*/) {}
__attribute__((weak)) void mmStaticInstance::DeclareFields() {}
__attribute__((weak)) void ARTS_FASTCALL mmUpperInstance::Draw(i32 /*arg1*/) {}
__attribute__((weak)) mmYInstance::mmYInstance() {}

// Data globals (original values from game.asm .data)
__attribute__((weak)) i32 EnableFacadeSideClipping {};
__attribute__((weak)) i32 facadeTriCount {};
__attribute__((weak)) f32 YDownForceMinHeight {30.0f};  // 0x41F00000
__attribute__((weak)) f32 YDownForceMaxHeight {50.0f};  // 0x42480000
__attribute__((weak)) f32 YDownForceMin {60.0f};        // 0x42700000
__attribute__((weak)) f32 YDownForceMax {90.0f};        // 0x42B40000
