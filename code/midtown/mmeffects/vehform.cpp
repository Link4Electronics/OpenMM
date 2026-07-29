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

define_dummy_symbol(mmeffects_vehform);

#include "vehform.h"

#include "agi/viewport.h"
#include "agi/rsys.h"
#include "agi/texdef.h"
#include "agi/vertex.h"
#include "agiworld/getmesh.h"
#include "agiworld/meshlight.h"
#include "agiworld/meshset.h"
#include "agiworld/quality.h"
#include "agiworld/texsheet.h"
#include "agiworld/texsort.h"
#include "arts7/camera.h"
#include "arts7/cullmgr.h"
#include "arts7/sim.h"
#include "mmcity/cullcity.h"
#include "stream/fsystem.h"

#include "agigl/glcontext.h"

#include <unistd.h>

static mem::cmd_param PARAM_menu_refl {"menurefl"};

mmVehicleForm::mmVehicleForm()
    : color_pointer(&color_index_)
{
    if (SphMapTex)
    {
        SphMapTex->AddRef();
    }
    else
    {
        if (agiRQ.SphMap && PARAM_menu_refl.get_or<bool>(true))
        {
            t_mmEnvSetup* env = &mmEnvSetup[1][0];

            SphMapTex = as_raw GetPackedTexture(xconst(env->SphereMap), 0);

            if (SphMapTex)
                SphMapTex->Tex.Props |= agiTexProp::AlphaGlow;
        }
    }
}

mmVehicleForm::~mmVehicleForm()
{
    if (SphMapTex)
    {
        if (SphMapTex->Release() == 0)
            SphMapTex = nullptr;
    }
}

void mmVehicleForm::Update()
{
    if (vehicle_mesh_)
    {
        CullMgr()->DeclareCullable(this);
    }
}

void (*mmVehicleForm::Lighter)(u8*, u32*, u32*, agiMeshSet*) = agiMeshLighterTriple;

// Strong definitions override the 8-byte weak stubs in game_stubs.S (Vector3 is 12 bytes).
Vector3 agiMeshLighterAmbient {};
Vector3 agiMeshLighterSun {};
Vector3 agiMeshLighterSunColor {};
Vector3 agiMeshLighterFill1 {};
Vector3 agiMeshLighterFill1Color {};
Vector3 agiMeshLighterFill2 {};
Vector3 agiMeshLighterFill2Color {};

static void InitVehicleLighting()
{
    agiMeshLighterAmbient = Vector3(0.4f, 0.4f, 0.45f);

    agiMeshLighterSun = ~Vector3(0.3f, 0.5f, -0.8f);
    agiMeshLighterSunColor = Vector3(1.0f, 0.9f, 0.8f);

    agiMeshLighterFill1 = ~Vector3(-0.6f, 0.4f, 0.7f);
    agiMeshLighterFill1Color = Vector3(0.5f, 0.5f, 0.55f);

    agiMeshLighterFill2 = ~Vector3(0.8f, 0.2f, 0.5f);
    agiMeshLighterFill2Color = Vector3(0.25f, 0.25f, 0.3f);
}

static const char* WHEEL_GROUPS[mmVehicleForm::MAX_WHEELS] = {
    "WHL0", "WHL1", "WHL2", "WHL3"
};

void mmVehicleForm::SetShape(char* name, char* group, char* arg3, Vector3* offset)
{
    Displayf("DBG SetShape: name=%s group=%s arg3=%s", name, group, arg3 ? arg3 : "(null)");

    char tsh_path[64];
    arts_sprintf(tsh_path, "mtl/%s.TSH", name);
    TEXSHEET.Load(tsh_path);

    Displayf("DBG SetShape: calling GetMeshSet(\"%s\", \"%s\", %p, 0x107)", name, group, (void*)offset);

    vehicle_mesh_ = GetMeshSet(name, group, offset, 0x107);

    Displayf("DBG SetShape: vehicle_mesh_=%p offset=%p", (void*)vehicle_mesh_, (void*)offset);

    if (arg3)
        shadow_mesh_ = GetMeshSet(name, arg3, offset, 0x107);

    for (i32 i = 0; i < MAX_WHEELS; ++i)
    {
        wheel_meshes_[i] = GetMeshSet(name, const_cast<char*>(WHEEL_GROUPS[i]), offset, 0x107);
        Displayf("DBG SetShape: wheel_mesh_[%d]=%p", i, (void*)wheel_meshes_[i]);
        if (!wheel_meshes_[i])
            Displayf("DBG SetShape: wheel_mesh_[%d] FAILED to load!", i);
    }
}

void mmVehicleForm::Cull()
{
    Displayf("DBG mmVehicleForm::Cull entered");

    static bool once = (InitVehicleLighting(), true);
    (void)once;

    auto old_cull = agiCurState.SetCullMode(agiCullMode::None);

    auto RenderMesh = [&](agiMeshSet* mesh, bool lit)
    {
        if (!mesh)
            return;

        if (mesh->LockIfResident())
        {
            if (lit)
            {
                mesh->Geometry(MESH_DRAW_CLIP, mesh->Vertices, mesh->Planes);
                mesh->DrawLit(Lighter, MESH_DRAW_CLIP, nullptr);
            }
            else
            {
                mesh->Draw(MESH_DRAW_CLIP);
            }
            mesh->Unlock();
        }
        else
        {
            mesh->PageIn();
        }
    };

    RenderMesh(vehicle_mesh_, true);

    for (i32 i = 0; i < MAX_WHEELS; ++i)
        RenderMesh(wheel_meshes_[i], true);

    // Shadow: use vertex colors with semi-transparency
    if (shadow_mesh_)
    {
        if (shadow_mesh_->LockIfResident())
        {
            if (shadow_mesh_->Colors)
                shadow_mesh_->DrawColor(0x88FFFFFF, MESH_DRAW_CLIP);
            else
                shadow_mesh_->DrawColor(0x55000000, MESH_DRAW_CLIP);
            shadow_mesh_->Unlock();
        }
        else
        {
            shadow_mesh_->PageIn();
        }
    }

    agiCurState.SetCullMode(old_cull);
}
