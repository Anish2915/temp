// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PixelProcessor.generated.h"

UCLASS()
class FACEFILTER_API APixelProcessor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APixelProcessor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// FOr WebSocket
	void SendFromWebSocket();
	void SetupWebSocket();

	// For UDP
	void SetupUDP();
	void ProcessUDP();
	class FSocket* CreateUDPSocket();
	void ReceiveUDPData(); 

	//For TCP 
	void SetupTCP();
	class FSocket* CreateTCPSocket();
	void ProcessTCP();

	//Extra 
	UTexture2D* CreateTextureFromImageData(const TArray<uint8>& ImageData, int32 Width, int32 Height); 
	void CreateAndSetTextureAtRuntime();

	// Variables 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
	UTextureRenderTarget2D* RenderTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UMyUserWidget> MyWidgetClass;
	
	class UMyUserWidget* MyWidgetInstance;
	bool bIsConnected = false;
	
	//WebSocket 
	TSharedPtr<class IWebSocket> Socket = nullptr;

	//UDP
	class FSocket* UdpSocketRef;

	//TCP
	class FSocket* TCPSocketRef;

	//WebSocket
	TArray<uint8> AccumulatedData; 

	// extra functions
	void ProcessReassembledData(const TArray<uint8>& ReassembledData);
	void SaveImageFromBuffer();
	TArray<uint8> ImageBuffer; // Buffer to hold the received image data  
	bool bIsReceivingImage = false;  


	float AccumlatedTime = 0;
	class FTextureRenderTargetResource* RTResource;
	class TSharedPtr<class IImageWrapper> ImageWrapper;
	class TSharedPtr<class IImageWrapper> ImageWrapper1;

	//UDP
	class TSharedPtr<class FInternetAddr> RemoteAddress;
	class TSharedPtr<class FInternetAddr> RecieveAddress;

	FTimerHandle TimerHandle;

	// cached
	UTexture2D* CachedTexture;
	FTexturePlatformData* PlatformData;  

};
