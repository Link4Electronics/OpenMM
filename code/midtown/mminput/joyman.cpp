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

define_dummy_symbol(mminput_joyman);

#include "joyman.h"

#include "arts7/sim.h"
#include "eventq7/event.h"
#include "eventq7/keys.h"

#include "input.h"
#include "joystick.h"

mmJoyMan::mmJoyMan()
{}

mmJoyMan::~mmJoyMan()
{}

i32 mmJoyMan::FFPlay(i32 /*arg1*/)
{
    return 0;
}

i32 mmJoyMan::FFSetValues(i32 /*arg1*/, f32 /*arg2*/, f32 /*arg3*/)
{
    return 0;
}

i32 mmJoyMan::HasCoolie()
{
    return 0;
}

i32 mmJoyMan::ToggleEnabled(i32 /*arg1*/)
{
    return 0;
}

i32 mmJoyMan::ButtonToBit(i32 button)
{
    if (button >= mmJoyInput::JButton1 && button <= mmJoyInput::JButton12)
        return 1u << (button - mmJoyInput::JButton1);

    if (button >= mmJoyInput::JButton13 && button <= mmJoyInput::JButton32)
        return 1u << (button - mmJoyInput::JButton13 + 12);

    return 0;
}

i32 mmJoyMan::GetOneButton(ilong flags)
{
    for (i32 i = 0; i < 32; ++i)
    {
        if ((flags >> i) & 1)
            return i + 1;
    }

    return -1;
}

void mmJoyMan::Update()
{
    for (i32 i = 0; i < NumSticks; ++i)
        Sticks[i].Update();
}

static mem::cmd_param PARAM_joynav {"joynav", "Enable using the joystick for menu navigation"};

void mmJoyMan::GenerateEvents()
{
    if (!PARAM_joynav.get_or(true))
        return;

    // TODO: Store state in mmJoystick
    static u32 prev_buttton = 0;
    static u32 prev_axes = 0;
    static bool use_mouse = false;

    u32 buttons = 0;
    u32 axes = 0;

    auto sq = eqEventHandler::SuperQ;

    for (i32 i = 0; i < NumSticks; ++i)
    {
        const auto& stick = Sticks[i];
        buttons |= stick.ButtonFlags;

        static const f32 deadzone = 0.25f;
        static const f32 exp = 2.0f;
        static const f32 speed = 1200.0f;

        if (f32 x = std::abs(stick.XAxis.Value); x > deadzone)
        {
            f32 xrel = std::copysign(std::pow(x, exp), stick.XAxis.Value) * Sim()->GetUpdateDelta() * speed;
            sq->WarpMouse(sq->GetMouseX() + xrel, sq->GetMouseY(), true);
            use_mouse = true;
        }

        if (f32 y = std::abs(stick.YAxis.Value); y > deadzone)
        {
            f32 yrel = std::copysign(std::pow(y, exp), stick.YAxis.Value) * Sim()->GetUpdateDelta() * speed;
            sq->WarpMouse(sq->GetMouseX(), sq->GetMouseY() + yrel, true);
            use_mouse = true;
        }

        if (stick.PovAxis.Value == 0.25f)
            axes |= 1;
        else if (stick.PovAxis.Value == 0.75f)
            axes |= 2;
        else if (stick.PovAxis.Value == 1.00f)
            axes |= 4;
        else if (stick.PovAxis.Value == 0.50f)
            axes |= 8;
    }

    if (u32 changed = buttons ^ std::exchange(prev_buttton, buttons))
    {
        const auto button_button = [&](mmJoyInput button, i32 btn) {
            if (u32 bit = ButtonToBit(button); changed & bit)
                sq->SendMousePress(nullptr, btn, !!(buttons & bit));
        };

        const auto button_key = [&](mmJoyInput button, i32 vkey) {
            if (u32 bit = ButtonToBit(button); changed & bit)
                sq->SendKeyPress(nullptr, (buttons & bit) ? EQ_KMOD_DOWN : 0, vkey, 0);
        };

        if (use_mouse)
            button_button(mmJoyInput::JButton1, EQ_BUTTON_LEFT);
        else
            button_key(mmJoyInput::JButton1, EQ_VK_GAMEPAD_A);

        button_key(mmJoyInput::JButton2, EQ_VK_GAMEPAD_B);
        button_key(mmJoyInput::JButton7, EQ_VK_GAMEPAD_VIEW);
        button_key(mmJoyInput::JButton8, EQ_VK_GAMEPAD_MENU);
    }

    if (u32 changed = axes ^ std::exchange(prev_axes, axes))
    {
        const auto axis_key = [&](i32 axis, i32 vkey) {
            if (changed & axis)
                sq->SendKeyPress(nullptr, (axes & axis) ? EQ_KMOD_DOWN : 0, vkey, 0);
        };

        axis_key(1, EQ_VK_GAMEPAD_DPAD_LEFT);
        axis_key(2, EQ_VK_GAMEPAD_DPAD_RIGHT);
        axis_key(4, EQ_VK_GAMEPAD_DPAD_UP);
        axis_key(8, EQ_VK_GAMEPAD_DPAD_DOWN);
        use_mouse = false;
    }
}
