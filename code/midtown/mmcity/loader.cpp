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

define_dummy_symbol(mmcity_loader);

#include "loader.h"

#include "agi/bitmap.h"
#include "agi/pipeline.h"
#include "arts7/cullmgr.h"
#include "eventq7/event.h"
#include "localize/localize.h"
#include "mmeffects/mmtext.h"

#include "data7/printer.h"

static mem::cmd_param PARAM_loadingscreen {"loadingscreen", "Show loading screens"};

mmLoader::mmLoader()
{
    // Sub-objects are default-initialized (zero from memset in caller)
    // asCullable constructor sets vtable, then we set mmLoader vtable below

    // vtable pointer is handled by C++ compiler

    Current = this;
}

mmLoader::~mmLoader()
{
    Current = nullptr;
}

void mmLoader::Init(aconst char* underlay_name, f32 bar_x, f32 bar_y)
{
    Current = this;

    if (!myFont)
        myFont = mmText::CreateFont("Gill Sans MT", 20);

    bar_x_ = UI_XPos + std::lround(UI_Width * bar_x);
    bar_y_ = UI_YPos + std::lround(UI_Height * bar_y);

    camera_.SetUnderlay(underlay_name);

    bar_inactive_ = Pipe()->GetBitmap("pbar_inact", 1.0f, 1.0f, 0);
    bar_active_ = Pipe()->GetBitmap("pbar_act", 1.0f, 1.0f, 0);

    task_text_.Init(0.25f, 0.85f, 0.5f, 0.0729f, 2, BITMAP_TRANSPARENT);
    task_text_.AddText(myFont, LOC_TEXT(""), MM_TEXT_CENTER, 0.0f, 0.0f);
    task_text_.AddText(myFont, LOC_TEXT(""), MM_TEXT_CENTER, 0.0f, 0.075f);

    intro_text_.Init(0.25f, 0.07f, 0.5f, 0.2f, 1, 0);
    intro_text_.AddText(myFont, LOC_TEXT(""), MM_TEXT_PADDING | MM_TEXT_WORDBREAK, 0.0f, 0.0f);

    text_node3_.Init(0.25f, 0.30f, 0.5f, 0.2f, 1, 0);
    text_node3_.AddText(myFont, LOC_TEXT(""), MM_TEXT_PADDING | MM_TEXT_WORDBREAK, 0.0f, 0.0f);

    Update();
}

void mmLoader::Cull()
{
    if (!PARAM_loadingscreen.get_or(true))
        return;

    // Render progress bar (track + fill clipped by task percent).
    // The camera draws its underlay via CullMgr's camera pass.
    if (bar_inactive_)
    {
        Pipe()->CopyClippedBitmap(bar_x_, bar_y_, bar_inactive_.get(), 0, 0,
            bar_inactive_->GetWidth(), bar_inactive_->GetHeight());
    }

    if (bar_active_ && current_task_percent_ > 0.0f)
    {
        const i32 width = static_cast<i32>(bar_active_->GetWidth() *
            std::clamp(current_task_percent_, 0.0f, 1.0f));

        if (width > 0)
        {
            Pipe()->CopyClippedBitmap(bar_x_, bar_y_, bar_active_.get(), 0, 0,
                width, bar_active_->GetHeight());
        }
    }

    // Render task text
    task_text_.Cull();
    intro_text_.Cull();
    text_node3_.Cull();
}

void mmLoader::SetIntroText(LocString* text)
{
    text_node3_.SetString(0, text);
    Update();
}

void mmLoader::BeginTask(LocString* text, f32 percent)
{
    if (percent >= 0.0f)
    {
        if (percent > 1.0f)
            percent = 1.0f;

        current_task_percent_ = percent;
        task_start_time_ = timer_.Time();
    }

    task_text_.SetString(0, text);
    Update();
}

void mmLoader::EndTask(f32 percent)
{
    if (percent >= 0.0f)
    {
        if (percent > 1.0f)
            percent = 1.0f;

        current_task_percent_ = percent;
        task_start_time_ = timer_.Time();
    }

    task_text_.SetString(0, LOC_TEXT(""));
    task_text_.SetString(1, LOC_TEXT(""));
    Update();

    task_percent_ = 0;
}

void mmLoader::Reset()
{
    task_start_percent_ = 0.0f;
    current_task_percent_ = 0.0f;
    task_start_time_ = 0.0f;
    timer_.Reset();
}

void mmLoader::Update()
{
    eqEventHandler::SuperQ->Update();

    if (!PARAM_loadingscreen.get_or(true))
    {
        return;
    }

    camera_.Update();

    // Declare the loader camera so CullMgr pumps frames while the city
    // loads (without it, num_cameras_ == 0 and nothing renders).
    CullMgr()->DeclareCamera(&camera_);
    CullMgr()->DeclareCullable2D(this);

#ifndef ARTS_FINAL
    // if ((current_task_percent_ == 1.0f) && (static_cast<i32>(timer_.Time()) % 2))
    {
        task_text_.Update();
    }
#endif

    CullMgr()->Update();
}
