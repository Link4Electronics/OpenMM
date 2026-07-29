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

define_dummy_symbol(mmaudio_manager);

#include "manager.h"

ulong AudManager::GetNum3DHalBufs()
{
    return 0;
}

void AudManager::Reset()
{}

void AudManager::ZeroVolAllSounds()
{}

// ?Init@AudManager@@QAEXHIPADFF@Z
void AudManager::Init(i32 /*max_sounds*/, u32 /*flags*/, char* /*device*/, i16 /*sfx*/, i16 /*cd*/)
{}

// ?PlayCDTrack@AudManager@@QAEHHH@Z
i32 AudManager::PlayCDTrack(i32 /*track*/, b32 /*restart*/)
{
    return 0;
}

// ?PlayCDTrack@AudManager@@QAEHHEEEH@Z
i32 AudManager::PlayCDTrack(i32 /*track*/, u8 /*minute*/, u8 /*second*/, u8 /*frame*/, b32 /*restart*/)
{
    return 0;
}

// ?GetVoiceCommentaryPtr@AudManager@@QAEPAVmmVoiceCommentary@@XZ
mmVoiceCommentary* AudManager::GetVoiceCommentaryPtr()
{
    return nullptr;
}

AudManager::AudManager()
{}

AudManager::~AudManager()
{}

void AudManager::AlwaysEAX(u32 /*arg1*/)
{}

void AudManager::AssignCDVolume(f32 /*arg1*/)
{}

void AudManager::AssignWaveVolume(f32 /*arg1*/)
{}

u8 AudManager::CheckCDFile(aconst char* /*arg1*/)
{
    return 0;
}

void AudManager::DeallocateUIADF()
{}

void AudManager::Disable(i16 /*sfx_mode*/, i16 /*cd_mode*/)
{}

u32 AudManager::EAXEnabled()
{
    return 0;
}

void AudManager::Enable(char* /*arg1*/, i16 /*arg2*/, i16 /*arg3*/)
{}

void AudManager::SetBitDepthAndSampleRate(i32 /*arg1*/, ulong /*arg2*/)
{}

void AudManager::SetCDPlayMode(u8 /*arg1*/)
{}

i32 AudManager::SetEAXPreset(EAX_REVERBPROPERTIES* /*arg1*/)
{
    return 0;
}

void AudManager::SetNumChannels(i32 /*arg1*/)
{}

void AudManager::SetVoiceCommentaryPtr(mmVoiceCommentary* /*arg1*/)
{}

void AudManager::StopCD()
{}

MetaClass* AudManager::GetClass()
{
    return nullptr;
}

void AudManager::Update()
{}

void AudManager::UpdatePaused()
{}

void AudManager::AddWidgets(Bank* /*arg1*/)
{}
