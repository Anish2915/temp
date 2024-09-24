// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FaceFilterGameMode.generated.h"

UCLASS(minimalapi)
class AFaceFilterGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFaceFilterGameMode();

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "UI")
	class UMyUserWidget* GlobalWidgetInstance;
};



