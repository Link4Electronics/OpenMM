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

define_dummy_symbol(agi_viewport);

#include "viewport.h"

#include <cmath>

#include "pipeline.h"
#include "vector7/matrix34.h"

// FUNCTION: MIDTOWN 0x005392D0
agiViewParameters::agiViewParameters()
{
    X = 0.0f;
    Y = 0.0f;
    Width = 1.0f;
    Height = 1.0f;
    Fov = 1.0f;
    Aspect = 1.0f;
    Near = 0.1f;
    Far = 1000.0f;
    DepthScale = 1.0f;
    Orthographic = false;
}

// FUNCTION: MIDTOWN 0x00539340
void agiViewParameters::Perspective(f32 fov, f32 aspect, f32 near, f32 far)
{
    f32 half_fov_rad = fov * (ARTS_PI / 360.0f);
    f32 cot = 1.0f / std::tan(half_fov_rad);

    Fov = fov;
    Aspect = aspect;
    Near = near;
    Far = far;

    ProjX = cot;
    ProjY = cot / aspect;
    ProjZZ = far / (far - near);
    ProjZW = -near * far / (far - near);
    ProjXZ = 0.0f;
    ProjYZ = 0.0f;

    DepthScale = 1.0f / (far - near);

    Orthographic = false;

    ++MtxSerial;
}

void agiViewParameters::SetWorld(const Matrix34& world)
{
    World = world;
    ModelView.Dot(View, World);
    ++MtxSerial;
}

agiViewport::agiViewport(agiPipeline* pipe)
    : agiRefreshable(pipe)
    , field_144_(pipe_->GetDword38())
{}

agiViewport::~agiViewport()
{
    if (this == Active)
        Active = nullptr;
}

// FUNCTION: MIDTOWN 0x005399A0
void agiViewport::SetWorld(aconst Matrix34& world)
{
    params_.SetWorld(world);
}

// FUNCTION: MIDTOWN 0x00539970
f32 agiViewport::Aspect()
{
    if (state_)
    {
        agiPipeline* pipe = Pipe();

        return (pipe->GetWidth() * params_.Width) / (pipe->GetHeight() * params_.Height);
    }

    return params_.Width / params_.Height;
}

// FUNCTION: MIDTOWN 0x00539AD0
aconst char* agiViewport::GetName()
{
    static char buffer[128]; // FIXME: Static buffer
    arts_sprintf(buffer, "Viewport '%p'", this);
    return buffer;
}
