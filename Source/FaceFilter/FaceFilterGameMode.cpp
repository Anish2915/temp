// Copyright Epic Games, Inc. All Rights Reserved.

#include "FaceFilterGameMode.h"
#include "FaceFilterCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFaceFilterGameMode::AFaceFilterGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
