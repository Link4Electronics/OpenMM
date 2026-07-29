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

define_dummy_symbol(mmphysics_joint3dof);

#include "joint3dof.h"

#include "inertia.h"

void Joint3Dof::BreakJoint()
{
    JointFlags |= JOINT_FLAG_BROKEN;
}

void Joint3Dof::InitJoint3Dof(asInertialCS* arg1, const Vector3& arg2, asInertialCS* arg3, const Vector3& arg4)
{
    ICS1 = arg1;
    Offset1 = arg2;
    ICS2 = arg3;
    Offset2 = arg4;

    if (ICS1 && ICS2)
    {
        Orientation1 = ICS1->Matrix;
        Orientation2 = ICS2->Matrix;
        Position = ICS2->Matrix.m3 - ICS1->Matrix.m3;
        JointFlags = JOINT_FLAG_LIMIT;
    }

    if (ICS1)
        ICS1->Joint = this;
    if (ICS2)
        ICS2->Joint = this;
}

void Joint3Dof::Reset()
{
    JointFlags = 0;
    Position = Vector3(0.0f, 0.0f, 0.0f);
}

void Joint3Dof::UnbreakJoint()
{
    JointFlags &= ~JOINT_FLAG_BROKEN;
}

void Joint3Dof::MoveICS()
{
    // TODO: Why was LinearPush applied here?
    // It is already applied in Joint3Dof::Update.
    // if (!(JointFlags & JOINT_FLAG_BROKEN))
    // {
    //     ICS1->ApplyPush(ICS2->LinearPush);
    //     ICS2->LinearPush = ICS1->LinearPush;
    // }

    ICS1->MoveICS();
    ICS2->MoveICS();
}
