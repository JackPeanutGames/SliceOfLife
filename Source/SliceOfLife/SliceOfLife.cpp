// Copyright Epic Games, Inc. All Rights Reserved.

#include "SliceOfLife.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, SliceOfLife, "SliceOfLife" );

// Console variable to toggle hitbox debug draws across game
static TAutoConsoleVariable<int32> CVarSliceOfLifeShowHitboxes(
    TEXT("SliceOfLife.ShowHitboxes"),
    0,
    TEXT("Toggle debug visualization of hit/hurt boxes. 0=Off, 1=On"),
    ECVF_Cheat
);