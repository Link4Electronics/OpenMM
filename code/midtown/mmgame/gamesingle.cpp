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

define_dummy_symbol(mmgame_gamesingle);

#include "gamesingle.h"

#include "agi/pipeline.h"
#include "agiworld/meshlight.h"
#include "arts7/lamp.h"
#include "data7/metaclass.h"
#include "mmai/aiMap.h"
#include "mmai/aiaudiomanager.h"
#include "mmaudio/manager.h"
#include "mmaudio/mmvoicecommentary.h"
#include "mmcity/cullcity.h"
#include "mmcityinfo/racedata.h"
#include "mmcityinfo/state.h"
#include "mminput/input.h"
#include "player.h"
#include "popup.h"
#include "waypoints.h"

#ifndef ARTS_STANDALONE
i32 mmGameSingle::OppNumCheck[MaxOpponents] {};
i16 mmGameSingle::OppFinishPositions[MaxOpponents] {};
#endif

__attribute__((weak)) mmGameSingle::mmGameSingle() {}
__attribute__((weak)) mmGameSingle::~mmGameSingle() {}

MetaClass* mmGameSingle::GetClass()
{
    return nullptr;
}

mmWaypoints* mmGameSingle::GetWaypoints()
{
    return Waypoints;
}

void mmGameSingle::Update()
{
    agiBeginCones();
    mmGame::Update();
}

__attribute__((weak)) void mmGameSingle::InitMyPlayer() {}
__attribute__((weak)) void mmGameSingle::InitOtherPlayers() {}
__attribute__((weak)) void mmGameSingle::InitGameObjects() {}
__attribute__((weak)) void mmGameSingle::InitHUD() {}
__attribute__((weak)) void mmGameSingle::UpdateGameInput(i32) {}
__attribute__((weak)) void mmGameSingle::UpdateGame() {}
__attribute__((weak)) void mmGameSingle::NextRace() {}
__attribute__((weak)) void mmGameSingle::HitWaterHandler() {}
__attribute__((weak)) void mmGameSingle::UpdateScore() {}
__attribute__((weak)) void mmGameSingle::DisableRacers() {}
__attribute__((weak)) void mmGameSingle::EnableRacers() {}
__attribute__((weak)) void mmGameSingle::FinishMessage(i32, i32) {}
__attribute__((weak)) i32 mmGameSingle::ProgressCheck(i32, i32) { return 0; }
__attribute__((weak)) i32 mmGameSingle::RegisterFinish() { return 0; }
__attribute__((weak)) void mmGameSingle::DeclareFields() {}
#ifdef ARTS_DEV_BUILD
__attribute__((weak)) void mmGameSingle::AddWidgets(Bank*) {}
#endif

__attribute__((weak)) void agiBeginCones() {}

// mmGame inherited virtuals (needed for vtable)
__attribute__((weak)) void mmGame::InitGameStrings() {}
__attribute__((weak)) void mmGame::DropThruCityHandler() {}
__attribute__((weak)) void mmGame::CollideAIOpponents() {}
__attribute__((weak)) i32 mmGame::GetCDTrack(i16) { return 0; }
__attribute__((weak)) void mmGame::PlayerSetState() {}
__attribute__((weak)) void mmGame::SetIconsState() {}
__attribute__((weak)) void mmGame::UpdateSteeringBrakes() {}
__attribute__((weak)) i32 mmGame::CalculateRaceScore(i32, i32) { return 0; }
__attribute__((weak)) void mmGame::RespawnXYZ(Vector3&, f32&) {}

b32 mmGameSingle::Init()
{
    if (!mmGame::Init())
        return false;

    RaceData = arnew mmRaceData();
    RaceData->Load(arts_formatf<128>("%s\\mmracedata", RaceDir));

    GameState = 0;

#ifndef ARTS_STANDALONE
    std::memset(&OldOppNumCheck, 0xAA, sizeof(OldOppNumCheck));
    std::memset(&OldOppFinishPositions, 0xAA, sizeof(OldOppFinishPositions));
#endif

    for (usize i = 0; i < MaxOpponents; ++i)
    {
        OppNumCheck[i] = 1;
        OppFinishPositions[i] = 0;
    }

    NumFinished = 0;

    AddChild(Player.get());

    if (HasAIMap)
        AddChild(&AIMAP);

    AddChild(pCullCity.get());
    AddChild(&Icons);

    if (Waypoints)
        AddChild(Waypoints);

    AddChild(&Player->HudMap);
    AddChild(&Player->Hud);

    LampCS->AddChild(Lamp.get());
    AddChild(LampCS.get());

    AddChild(&FooBar);
    AddChild(Popup.get());

    return true;
}

void mmGameSingle::Reset()
{
    AudMgr()->Reset();
    InWater = false;

    if (MMSTATE.HasMidtownCD)
        AudMgr()->StopCD();

    if (AiAudMgr())
        AiAudMgr()->LoadCopVoice();

    GameInput()->Reset();

    GameStateWait = 0.0f;
    GameState = 0;

    for (usize i = 0; i < MaxOpponents; ++i)
    {
        OppNumCheck[i] = 1;
        OppFinishPositions[i] = 0;
    }

    NumFinished = 0;

    if (MMSTATE.GameMode == mmGameMode::Checkpoint)
        DisableRacers();

    mmGame::Reset();

    if (MMSTATE.GameMode == mmGameMode::Cruise)
    {
        if (VoiceCommentary)
            VoiceCommentary->PlayRoam();
    }
    else
    {
        Player->SetPreRaceCam();

        if (MMSTATE.HasMidtownCD)
            AudMgr()->PlayCDTrack(GetCDTrack(4), 1);

        if (VoiceCommentary)
            VoiceCommentary->PlayPreRace();

        Player->Hud.StopTimers();
    }
}

void mmGameSingle::UpdateDebugKeyInput(i32 /*arg1*/)
{}