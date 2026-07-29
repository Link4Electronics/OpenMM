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

define_dummy_symbol(vector7_matrix44);

#include "matrix44.h"
#include "matrix34.h"

// ??0Matrix44@@QAE@ABVMatrix34@@@Z
Matrix44::Matrix44(const Matrix34& arg1)
{
    m0.Set(arg1.m0.x, arg1.m0.y, arg1.m0.z, 0.0f);
    m1.Set(arg1.m1.x, arg1.m1.y, arg1.m1.z, 0.0f);
    m2.Set(arg1.m2.x, arg1.m2.y, arg1.m2.z, 0.0f);
    m3.Set(arg1.m3.x, arg1.m3.y, arg1.m3.z, 1.0f);
}

// ??0Matrix44@@QAE@ABV0@@Z
Matrix44::Matrix44(const Matrix44& arg1)
    : m0(arg1.m0)
    , m1(arg1.m1)
    , m2(arg1.m2)
    , m3(arg1.m3)
{}

void Matrix44::Dot(const Matrix44& lhs, const Matrix44& rhs)
{
    *this = {lhs.m0 ^ rhs, lhs.m1 ^ rhs, lhs.m2 ^ rhs, lhs.m3 ^ rhs};
}
