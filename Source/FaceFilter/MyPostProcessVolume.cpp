// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPostProcessVolume.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"

AMyPostProcessVolume::AMyPostProcessVolume()
{
	bEnabled = true;
	BlendWeight = 1.0f;
}

void AMyPostProcessVolume::BeginPlay()
{
    Super::BeginPlay();

    if (PostProcessMaterial)
    {
        // Apply the post-process material
        Settings.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, PostProcessMaterial));
    }
}
