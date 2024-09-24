// Fill out your copyright notice in the Description page of Project Settings.


#include "MyUserWidget.h"
#include "Components/Image.h"
void UMyUserWidget::SetMyTexture(UTexture2D* Texture)
{
    if (ImageWidget && Texture)
    {
        ImageWidget->SetBrushFromTexture(Texture);
    }
}
