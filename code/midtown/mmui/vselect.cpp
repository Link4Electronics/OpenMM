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

define_dummy_symbol(mmui_vselect);

#include "vselect.h"

#include <cstring>

#include "agi/pipeline.h"
#include "agi/viewport.h"
#include "arts7/sim.h"
#include "arts7/camera.h"
#include "arts7/dof.h"
#include "core/primitives.h"
#include "eventq7/event.h"
#include "mmcityinfo/playerdata.h"
#include "mmcityinfo/state.h"
#include "mmcityinfo/vehlist.h"
#include "mmeffects/vehform.h"
#include "mmui/vehicle.h"
#include "mmwidget/manager.h"
#include "mmwidget/menu.h"
#include "memory/stub.h"

void* arts_operator_new(std::size_t size);
void arts_operator_delete(void* ptr);

VehicleSelectBase::VehicleSelectBase(i32 arg1)
    : UIMenu(arg1)
{
    p_b_state_ = &b_state_;
    b_state_ = 0;
    std::memset(gap90, 0, sizeof(gap90));
}

VehicleSelectBase::~VehicleSelectBase()
{}

void VehicleSelectBase::Reset()
{}

void VehicleSelectBase::PostSetup()
{}

static void LoadCarMesh(VehicleSelectBase* base, i32 car)
{
    mmVehicleForm* forms = base->GetVehicleFormArray();
    if (!forms || forms[car].HasMesh())
        return;

    mmVehInfo* info = VehicleListPtr ? VehicleListPtr->GetVehicleInfo(car) : nullptr;
    if (!info || !info->IsValid())
        return;

    forms[car].SetShape(info->BaseName, const_cast<char*>("BODY"), const_cast<char*>("SHADOW"), nullptr);
}

void VehicleSelectBase::PreSetup()
{
    mmVehicleForm* forms = GetVehicleFormArray();
    i32 car = CurrentCar();

    // Safety: guard against uninitialized forms
    if (!forms)
    {
        return;
    }

    // Update locked label if difficulty changed (affects which vehicles are locked/unlocked)
    i32 stored_diff = *reinterpret_cast<i32*>(&gap90[0xD4]);
    i32 cur_diff = static_cast<i32>(MMCURRPLAYER.Difficulty);
    if (stored_diff != cur_diff)
    {
        *reinterpret_cast<i32*>(&gap90[0xD4]) = cur_diff;
        SetLockedLabel();
    }

    // Activate current car's DofCS node (set NODE_FLAG_ACTIVE)
    asDofCS* dofcs = GetDofCSArray();
    if (dofcs)
    {
        dofcs[car].SetNodeFlag(NODE_FLAG_ACTIVE);
    }

    // Load mesh for current car if not already loaded
    LoadCarMesh(this, car);

    // Set viewport on camera
    if (asCamera* camera = MenuMgr()->GetCamera())
    {
        f32 vx = *reinterpret_cast<f32*>(&gap90[0x74]);
        f32 vy = *reinterpret_cast<f32*>(&gap90[0x7C]);
        f32 vw = *reinterpret_cast<f32*>(&gap90[0x80]);
        f32 vh = *reinterpret_cast<f32*>(&gap90[0x88]);
        camera->SetViewport(vx, vy, vw, vh, 0);
    }
}

void VehicleSelectBase::Update()
{
    UIMenu::Update();

    mmVehicleForm* forms = GetVehicleFormArray();
    asDofCS* dofcs = GetDofCSArray();
    if (!forms || !dofcs)
    {
        return;
    }

    i32 car = CurrentCar();
    i32 count = VehicleListPtr ? VehicleListPtr->NumVehicles : 0;
    if (car < 0 || car >= count)
    {
        return;
    }

    if (!forms[car].HasMesh())
        return;

    dofcs[car].Update();
    dofcs[car].World.m3 = Vector3(0.0f, 40.0f, -250.0f);

    Sim()->PushFrame(&dofcs[car]);
    forms[car].Update();
    Sim()->PopFrame();

    // Set up camera matrices on the menu camera's viewport
    if (asCamera* camera = MenuMgr()->GetCamera())
    {
        agiViewParameters& params = camera->GetViewport()->GetParams();

        Vector3 eye(0.0f, 45.0f, -230.0f);
        Vector3 target(0.0f, 40.0f, -250.0f);
        Vector3 fwd = ~(target - eye);

        Vector3 cam_right = ~(fwd % Vector3(0.0f, 1.0f, 0.0f));
        Vector3 up = cam_right % fwd;

        params.Camera.m0 = cam_right;
        params.Camera.m1 = up;
        params.Camera.m2 = -fwd;
        params.Camera.m3 = eye;

        params.View.m0 = Vector3(cam_right.x, up.x, -fwd.x);
        params.View.m1 = Vector3(cam_right.y, up.y, -fwd.y);
        params.View.m2 = Vector3(cam_right.z, up.z, -fwd.z);
        params.View.m3 = Vector3(-(cam_right ^ eye), -(up ^ eye), fwd ^ eye);

        params.SetWorld(dofcs[car].World);
    }

    forms[car].SetColor(0xFFFFFFFF);
}

void VehicleSelectBase::InitCarSelection(i32 mode, f32 x, f32 y, f32 w, f32 h)
{
    // Guard against double initialization
    if (GetDofCSArray() != nullptr)
    {
        return;
    }

    i32 count = VehicleListPtr ? VehicleListPtr->NumVehicles : 0;
    f32 view_right = (x + w) + 0.05f;                                  // original: (x+w) - (-0.05)
    *reinterpret_cast<f32*>(&gap90[0x74]) = x;                         // ViewX at 0x104
    *reinterpret_cast<f32*>(&gap90[0x78]) = view_right;                 // ViewRight at 0x108
    *reinterpret_cast<f32*>(&gap90[0x7C]) = y;                         // ViewY at 0x10C
    *reinterpret_cast<f32*>(&gap90[0x80]) = w;                         // ViewWidth at 0x110
    *reinterpret_cast<f32*>(&gap90[0x84]) = 0.95f - view_right;        // ViewBottomMargin at 0x114
    *reinterpret_cast<f32*>(&gap90[0x88]) = h;                         // ViewHeight at 0x118
    *reinterpret_cast<f32*>(&gap90[0x8C]) = x;                         // ViewX dup at 0x11C
    *reinterpret_cast<f32*>(&gap90[0x94]) = 0.05f;                     // StepSize at 0x124
    *reinterpret_cast<f32*>(&gap90[0x90]) = 1.0f - (y + h);            // RightOffset at 0x120

    // Reset state
    SetCurrentCar(0);
    *reinterpret_cast<i32*>(&gap90[0x0C]) = 0;  // SomeIndex (still i32, safe in gap90)

    if (count <= 0)
        return;

    // Allocate raw memory for asDofCS array (header + count * sizeof(asDofCS))
    {
        char* mem = static_cast<char*>(arts_operator_new(4 + count * sizeof(asDofCS)));
        if (mem)
        {
            *reinterpret_cast<i32*>(mem) = count;
            asDofCS* dofcs = reinterpret_cast<asDofCS*>(mem + 4);
            for (i32 i = 0; i < count; ++i)
                new (&dofcs[i]) asDofCS();
            SetDofCSArray(dofcs);
        }
    }

    // Allocate raw memory for mmVehicleForm array (header + count * sizeof(mmVehicleForm))
    {
        char* mem = static_cast<char*>(arts_operator_new(4 + count * sizeof(mmVehicleForm)));
        if (mem)
        {
            *reinterpret_cast<i32*>(mem) = count;
            mmVehicleForm* forms = reinterpret_cast<mmVehicleForm*>(mem + 4);
            for (i32 i = 0; i < count; ++i)
                new (&forms[i]) mmVehicleForm();
            SetVehicleFormArray(forms);
        }
    }

    // Allocate int data arrays
    SetTopSpeedArray(static_cast<i32*>(arts_operator_new(count * sizeof(i32))));
    SetExtraArray(static_cast<i32*>(arts_operator_new(count * sizeof(i32))));

    // Create prev/next vehicle buttons
    f32 button_y = 1.0f - (y + h) + h;
    AddBMButton(IDC_VEHICLE_PREV, "roller_up", x, button_y, 3);
    AddBMButton(IDC_VEHICLE_NEXT, "roller_down", x + w, button_y, 3);

    SetFocusWidget(-1);
}

char* VehicleSelectBase::GetCarTitle(i32 index, char* buffer, i16 arg3, string* arg4)
{
    mmVehInfo* info = VehicleListPtr ? VehicleListPtr->GetVehicleInfo(index) : nullptr;
    if (info)
    {
        arts_strcpy(buffer, sizeof(buffer), info->Description);
    }
    else
    {
        *buffer = '\0';
    }
    return buffer;
}

void VehicleSelectBase::IncCar()
{
    i32 count = VehicleListPtr ? VehicleListPtr->NumVehicles : 0;
    i32 current = CurrentCar();
    if (current + 1 < count)
    {
        SetCurrentCar(current + 1);
        PreSetup();
    }
}

void VehicleSelectBase::DecCar()
{
    i32 current = CurrentCar();
    if (current > 0)
    {
        SetCurrentCar(current - 1);
        PreSetup();
    }
}

void VehicleSelectBase::IncColor()
{}

void VehicleSelectBase::DecColor()
{}

void VehicleSelectBase::ColorCB()
{}

void VehicleSelectBase::SetLockedLabel()
{}

void VehicleSelectBase::SetPick(i32 arg1, i16 arg2)
{
    CarMod(arg1);

    i32 car = CurrentCar();
    i32 count = VehicleListPtr ? VehicleListPtr->NumVehicles : 0;

    if (count <= 0 || car < 0 || car >= count)
        return;

    mmVehicleForm* forms = GetVehicleFormArray();
    mmVehInfo* info = VehicleListPtr ? VehicleListPtr->GetVehicleInfo(car) : nullptr;

    if (forms && info && info->IsValid())
    {
        forms[car].SetShape(info->BaseName, const_cast<char*>("BODY"), const_cast<char*>("SHADOW"), nullptr);
    }

    asDofCS* dofcs = GetDofCSArray();

    if (dofcs)
    {
        for (i32 i = 0; i < count; ++i)
            *reinterpret_cast<i32*>(reinterpret_cast<char*>(&dofcs[i]) + 0x18) &= ~1;

        *reinterpret_cast<i32*>(reinterpret_cast<char*>(&dofcs[car]) + 0x18) |= 1;
    }

    MMSTATE.CurrentCar = car;

    FillStats();
}

void VehicleSelectBase::TDPickCB()
{}

void VehicleSelectBase::AllSetCar(char* arg1, i32 arg2)
{}

void VehicleSelectBase::AssignVehicleStats(i32 arg1, f32 arg2, f32 arg3, f32 arg4, f32 arg5)
{}

void VehicleSelectBase::CarMod(i32& arg1)
{}

i32 VehicleSelectBase::CurrentVehicleIsLocked()
{
    return 0;
}

void VehicleSelectBase::FillStats()
{}

i32 VehicleSelectBase::LoadStats(char* arg1)
{
    return 0;
}

void VehicleSelectBase::SetLastUnlockedVehicle()
{}

void VehicleSelectBase::SetShowcaseFlag()
{}
