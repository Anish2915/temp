// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/PostProcessVolume.h"
#include "MyPostProcessVolume.generated.h"

/**
 * 
 */
UCLASS()
class FACEFILTER_API AMyPostProcessVolume : public APostProcessVolume
{
	GENERATED_BODY()
	
public:
	AMyPostProcessVolume();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "Post Process")
	UMaterialInterface* PostProcessMaterial;
};
