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

define_dummy_symbol(mmgame_player);

#include "player.h"

#include "arts7/linear.h"
#include "arts7/sim.h"
#include "mmcity/cullcity.h"
#include "vector7/vector3.h"

void mmPlayer::AfterLoad()
{}

void mmPlayer::BeforeSave()
{}

static const f32 RegenFrameRate = 30.0f;
static const f32 PlayerRegenRate = RegenFrameRate * 0.0005f;

void mmPlayer::UpdateRegen()
{
    if (Car.Sim.ICS.GetVelocity().Mag2() > 25.0f)
    {
        if (f32 damage = Car.Sim.CurrentDamage; damage > 0.0f)
        {
            Car.Sim.CurrentDamage =
                std::max<f32>(0.0f, damage - (Car.Sim.MaxDamageScaled * PlayerRegenRate * Sim()->GetUpdateDelta()));
        }
    }
}

void mmPlayer::EnableRegen(i32 enable)
{
    RegenEnabled = enable;
}

void mmPlayer::Init(char* car_name, char* map_name, mmGame* game)
{
    SetName(car_name);
    AfterLoad();

    Car.Init(car_name, CAR_TYPE_PLAYER, 0);

    Camera.SetView(0.87266f, 1.33333f, 1.0f, 1500.0f);

    MirrorMatrix.Identity();
    MirrorMatrix.Rotate(Vector3(0.0f, 0.0f, 1.0f), ARTS_PI);

    CullCity()->RenderWeb.MirrorMatrix = &MirrorMatrix;
    CullCity()->RenderWeb.SetMirrorPos(0.625f, 0.875f, 0.375f, 0.125f, 1.0f);

    field_30 = 0;
    XCamIndex = 3;
    XCamStart = 3;
    XCamCount = 3;

    NearCam.Init(&Car, "bumper");
    FarCam.Init(&Car, "hood");
    IndCam.Init(&Car, "far");
    PovCam.Init(&Car, "pov");
    DashCam.Init(&Car, "dash");

    PolarCam1.Init(&Car);
    PolarCam2.Init(&Car);
    PointCam.Init(&Car);
    AiCam.Init(&Car);
    PreCam.Init(&Car);
    PostCam.Init(&Car);

    CarCams[0] = &NearCam;
    CarCams[1] = &PovCam;
    CarCams[2] = &FarCam;
    CarCams[3] = &PolarCam1;
    CarCams[4] = &PolarCam2;
    CarCams[5] = &AiCam;

    Hud.Init(car_name, this);
    HudMap.Init(&Camera, &Car.Sim.ICS.World, &Hud, game, map_name);

    Reset();
}

i32 mmPlayer::IsMaxDamaged()
{
    return Car.Sim.CurrentDamage >= Car.Sim.MaxDamageScaled;
}

i32 mmPlayer::IsPOV()
{
    return CameraMode == 0 || CameraMode == 1;
}

void mmPlayer::ResetDamage()
{
    Car.Sim.CurrentDamage = 0.0f;
}

void mmPlayer::SetCamInterest(asInertialCS* /*arg1*/)
{}

void mmPlayer::SetPreRaceCam()
{
    WantPreRaceCam = true;
}

void mmPlayer::SetWideFOV(b32 wide)
{
    WideFov = wide;
}

void mmPlayer::ToggleCam()
{
    if (++CamIndex >= XCamCount)
        CamIndex = 0;
}

void mmPlayer::ToggleDash()
{
    if (Hud.IsDashActive())
        Hud.DeactivateDash();
    else
        Hud.ActivateDash();
}

void mmPlayer::ToggleExternalView()
{}

void mmPlayer::ToggleWideFOV()
{
    SetWideFOV(!WideFov);
}
