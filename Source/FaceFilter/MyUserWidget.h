// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class FACEFILTER_API UMyUserWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetMyTexture(class UTexture2D* Texture);

	UPROPERTY(meta = (BindWidget))
	class UImage* ImageWidget;
};
