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

define_dummy_symbol(mmphysics_inertia);

#include "inertia.h"

void asInertialCS::ApplyImpulse(const Vector3& impulse, const Vector3& point)
{
    LinearImpulse += impulse;
    AngularImpulse += point % impulse;
    NumImpulses++;
}

Vector3 asInertialCS::GetVelocity(const Vector3* arg1)
{
    if (arg1)
        return LinearVelocity + (AngularVelocity % *arg1);
    return LinearVelocity;
}

void asInertialCS::MoveICS()
{
    if (State == ICS_STATE_OFF)
        return;

    LinearVelocity = LinearMomentum * InvMass;
    AngularVelocity = Vector3(
        AngularMomentum.x * InvInertia.x,
        AngularMomentum.y * InvInertia.y,
        AngularMomentum.z * InvInertia.z);

    if (LimitAngVelocity)
    {
        f32 ang_speed = AngularVelocity.Mag();
        if (ang_speed > MaxAngVelocity)
            AngularVelocity *= MaxAngVelocity / ang_speed;
    }

    Matrix.m3 += LinearVelocity * Time;

    f32 ang_speed = AngularVelocity.Mag();
    if (ang_speed > 0.0f)
        Matrix.Rotate(AngularVelocity / ang_speed, ang_speed * Time);

    asLinearCS::Update();

    LinearForce = Vector3(0, 0, 0);
    AngularTorque = Vector3(0, 0, 0);

    if (NumImpulses > 0)
    {
        LinearMomentum += LinearImpulse;
        AngularMomentum += AngularImpulse;
        LinearImpulse = Vector3(0, 0, 0);
        AngularImpulse = Vector3(0, 0, 0);
        NumImpulses = 0;
    }

    Vel2 = LinearVelocity.Mag2();
    AngVel2 = AngularVelocity.Mag2();
    State = (Vel2 + AngVel2 > 0.01f) ? ICS_STATE_AWAKE : ICS_STATE_ASLEEP;
}

void asInertialCS::Zero()
{
    LinearMomentum = Vector3(0, 0, 0);
    AngularMomentum = Vector3(0, 0, 0);
    LinearVelocity = Vector3(0, 0, 0);
    AngularVelocity = Vector3(0, 0, 0);
    FrameVelocity = Vector3(0, 0, 0);
    LinearForce = Vector3(0, 0, 0);
    AngularTorque = Vector3(0, 0, 0);
    LinearImpulse = Vector3(0, 0, 0);
    AngularImpulse = Vector3(0, 0, 0);
    LinearPush = Vector3(0, 0, 0);
    TurnForce = Vector3(0, 0, 0);
    FramePush = Vector3(0, 0, 0);
    NumImpulses = 0;
    Vel2 = 0.1f;
    AngVel2 = 0.1f;
}

void asInertialCS::FileIO(MiniParser* /*arg1*/)
{}

asInertialCS::asInertialCS()
{
    Global = true;
    SetDensity(1.0f, 1.0f, 1.0f, 1.0f);
    Zero();
}

void asInertialCS::SetDensity(f32 size_x, f32 size_y, f32 size_z, f32 density)
{
    SetMass(size_x, size_y, size_z, size_x * size_y * size_z * density * 1000.0f);
}

void asInertialCS::SetMass(f32 size_x, f32 size_y, f32 size_z, f32 mass)
{
    Size = Vector3(size_x, size_y, size_z);
    Mass = mass;
    InvMass = 1.0f / mass;

    size_x *= size_x;
    size_y *= size_y;
    size_z *= size_z;
    Inertia = Vector3(size_y + size_z, size_x + size_z, size_x + size_y) * (mass / 12.0f);
    InvInertia = 1.0f / Inertia;
}
