#include "PixelProcessor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/TextureRenderTarget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"

#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "ImageUtils.h"
#include "Modules/ModuleManager.h"
#include "Misc/FileHelper.h"

#include "Networking.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "IPAddress.h"

#include "MyUserWidget.h"

#include "Engine/Texture2D.h"
#include "RenderUtils.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"
#include "Runtime/Engine/Public/ImageUtils.h"

namespace {

    bool ConvertRGBToRGBA(const TArray<uint8>& InRGBArray, TArray<uint8>& OutRGBAArray)
    {
        // Ensure the input array size is a multiple of 3 (RGB).
        int32 NumPixels = InRGBArray.Num() / 3;

        if (float(NumPixels) != float(InRGBArray.Num()) / 3.0)
            return false;

        // Resize the output array to accommodate RGBA (4 bytes per pixel).
        OutRGBAArray.SetNum(NumPixels * 4);

        // Loop through each RGB triplet in the input array.
        for (int32 i = 0, j = 0; i < InRGBArray.Num(); i += 3, j += 4)
        {
            OutRGBAArray[j] = InRGBArray[i];     // R
            OutRGBAArray[j + 1] = InRGBArray[i + 1]; // G
            OutRGBAArray[j + 2] = InRGBArray[i + 2]; // B
            OutRGBAArray[j + 3] = 255;               // A (100%)
        }
        return true;
    }


};


APixelProcessor::APixelProcessor()
{
    PrimaryActorTick.bCanEverTick = true;
}

void APixelProcessor::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    if (World)
    {
        if (MyWidgetClass)
        {
            UMyUserWidget* WidgetInstance = CreateWidget<UMyUserWidget>(World, MyWidgetClass);
            if (WidgetInstance)
            {
                WidgetInstance->AddToViewport();
                UE_LOG(LogTemp, Warning, TEXT("Widget successfully spawned from PixelProcessor"));

                this->MyWidgetInstance = WidgetInstance;
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to spawn widget"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("MyWidgetClass is null"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("World context is null"));
    }

    SetupUDP(); 

    //ReceiveUDPData();
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &APixelProcessor::ReceiveUDPData, 0.01f,true);

    //SetupTCP();
    //SetupWebSocket();

}

// Called every frame
void APixelProcessor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //SendFromWebSocket();
    //ProcessTCP();
    // Run every 1/FPS (e.g. 20 FPS means 50ms)
    ProcessUDP();
    /*AccumlatedTime += DeltaTime;
    if (AccumlatedTime > 0.025)
    {
        AccumlatedTime = 0;
    }*/

}

void APixelProcessor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UdpSocketRef) 
    {
        UdpSocketRef->Close(); // Close the socket 
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(UdpSocketRef); // Destroy the socket 
        UdpSocketRef = nullptr; // Set to nullptr to avoid dangling pointer 
    }

    if (RemoteAddress)
    {
        RemoteAddress.Reset(); // Reset the shared pointer if using one 
    }

    if (RecieveAddress)
    {
        RecieveAddress.Reset(); // Reset the shared pointer if using one 
    }

    Super::EndPlay(EndPlayReason);
}


void APixelProcessor::SendFromWebSocket()
{
    if (!RenderTarget) return;
    if (!RTResource) return;
    if (!bIsConnected) return;
    //if (!TCPSocketRef) return;

    //UE_LOG(LogTemp, Warning, TEXT("Heree"));
    TArray<FColor> PixelData;
    RTResource->ReadPixels(PixelData);

    if (PixelData.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No pixel data read from render target!"));
        return;
    }

    int32 Width = RenderTarget->SizeX; 
    int32 Height = RenderTarget->SizeY; 
    ImageWrapper->SetRaw(PixelData.GetData(), PixelData.Num() * sizeof(FColor), Width, Height, ERGBFormat::BGRA, 8); 

    const TArray64<uint8>& LoadedData = ImageWrapper->GetCompressed(0.4); 

    if (LoadedData.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to compress image data to PNG format!"));
        return;
    }

    if (bIsConnected && Socket.IsValid())
    {
        Socket->Send(LoadedData.GetData(), sizeof(uint8) * LoadedData.Num(), true); 
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("test"));
        UE_LOG(LogTemp, Error, TEXT("WebSocket is not connected or invalid"));
    }

}

void APixelProcessor::SetupWebSocket()
{
    const FString ServerURL = TEXT("ws://127.0.0.1:7890/"); 
    const FString ServerProtocol = TEXT("ws");              

    Socket = FWebSocketsModule::Get().CreateWebSocket(ServerURL, ServerProtocol);

    Socket->OnConnected().AddLambda([this]() -> void {
        UE_LOG(LogTemp, Warning, TEXT("Socket Connected"));
        this->bIsConnected = true;
        });

    Socket->OnRawMessage().AddLambda([this](const void* Data, SIZE_T Size, SIZE_T BytesRemaining) -> void {
        
        /*if (Data == nullptr || Size == 0) {
            UE_LOG(LogTemp, Error, TEXT("Received invalid data: Data is nullptr or Size is 0"));
            return;
        }

        try {
            AccumulatedData.Append(static_cast<const uint8*>(Data), Size);
        }
        catch (...) {
            UE_LOG(LogTemp, Error, TEXT("Failed to append data to AccumulatedData"));
            return;
        }*/

        //UE_LOG(LogTemp, Warning, TEXT("AccumulatedData size: %d"), AccumulatedData.Num());

        //if (BytesRemaining == 0) {
            /*UE_LOG(LogTemp, Warning, TEXT("All data received, total size: %d"), AccumulatedData.Num());
            for (int i = 0; i < 10; ++i) {
                UE_LOG(LogTemp, Warning, TEXT("AccumulatedData[%d]: %d"), i, AccumulatedData[i]);
            }  */ 
            /*FString FilePath1 = FPaths::ProjectDir() + TEXT("ReceivedImage.png");

            FFileHelper::SaveArrayToFile(AccumulatedData, *FilePath1); 
            TArray<uint8> LoadedData ;
            FFileHelper::LoadFileToArray(LoadedData, *FilePath1) */
            
            /*UTexture2D * text = CreateTextureFromImageData(AccumulatedData, 1280, 720);
            MyWidgetInstance->SetMyTexture(text);
            AccumulatedData.Empty();*/
        //}
        
        });

    Socket->OnConnectionError().AddLambda([this](const FString& Error) -> void {
        UE_LOG(LogTemp, Error, TEXT("Socket connection error: %s"), *Error);
        });

    RTResource = RenderTarget->GameThread_GetRenderTargetResource(); 
    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper")); 
    ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG); 
    Socket->Connect();

    //bIsConnected = true; 
}

FSocket* APixelProcessor::CreateUDPSocket()
{
    FSocket* UDPSocket = FUdpSocketBuilder(TEXT("MyUDPSocket")) 
        .AsNonBlocking()
        .AsReusable()
        .WithBroadcast() 
        .WithReceiveBufferSize(64 * 1024 * 1024);

    if (!UDPSocket)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create UDP socket."));
        return nullptr;
    }
     
    return UDPSocket;
}

void APixelProcessor::ReceiveUDPData()
{
    if (!UdpSocketRef || !bIsConnected)
    {
        UE_LOG(LogTemp, Warning, TEXT("Socket not initialized or not connected."));
        return;
    }

    uint32 BufferSize = 65507;
    uint8* DataBuffer = new uint8[65507]; 

    
    int32 BytesReceived = 0;
    while (UdpSocketRef->RecvFrom(DataBuffer, BufferSize, BytesReceived, *RecieveAddress)) 
    {
        //UE_LOG(LogTemp, Warning, TEXT("waiting"));
        if (BytesReceived > 0)
        {
            //UE_LOG(LogTemp, Warning, TEXT("Bytes Received: %d"), BytesReceived); 
            // Check for end-of-message signal by directly comparing the bytes
            if (BytesReceived ==14)
            {
                //bIsReceivingImage = false;
                CreateAndSetTextureAtRuntime();
                ImageBuffer.Empty();  
            }
            else
            {
                // Append the received chunk to the ImageBuffer
                ImageBuffer.Append(DataBuffer, BytesReceived);
                //bIsReceivingImage = true;
            }
        }
    }
}

void APixelProcessor::SetupTCP()
{
    TCPSocketRef = CreateTCPSocket();
    if (!TCPSocketRef)
    {
        UE_LOG(LogTemp, Error, TEXT("tcp connection error"));
    }
    const FString ServerIP = "127.0.0.1";
    const int32 ServerPort = 12345;
    // Create the server address
    TSharedRef<FInternetAddr> ServerAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    bool bIsValid = false;
    ServerAddress->SetIp(*ServerIP, bIsValid);
    ServerAddress->SetPort(ServerPort);

    if (!bIsValid)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid IP address: %s"), *ServerIP);
        TCPSocketRef->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(TCPSocketRef);
    }

    // Connect to the server
    if (!TCPSocketRef->Connect(*ServerAddress))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to connect to server."));
        TCPSocketRef->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(TCPSocketRef);
    }

    RTResource = RenderTarget->GameThread_GetRenderTargetResource(); 
    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper")); 
    ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG); 
    bIsConnected = true; 
}

FSocket* APixelProcessor::CreateTCPSocket() 
{
    FSocket* TCPSocket = FTcpSocketBuilder(TEXT("MyTCPSocket"))
        .AsReusable() 
        .WithSendBufferSize(64 * 1024 * 1024)
        .WithReceiveBufferSize(64 * 1024 * 1024);

    if (!TCPSocket)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create TCP socket."));
        return nullptr;
    }

    return TCPSocket;
}

void APixelProcessor::ProcessTCP()
{
    if (!RenderTarget) return; 
    if (!RTResource) return;
    if (!bIsConnected) return;
    if (!TCPSocketRef) return;

    TArray<FColor> PixelData;
    RTResource->ReadPixels(PixelData);

    if (PixelData.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No pixel data read from render target!"));
        return;
    }

    int32 Width = RenderTarget->SizeX; 
    int32 Height = RenderTarget->SizeY; 
    ImageWrapper->SetRaw(PixelData.GetData(), PixelData.Num() * sizeof(FColor), Width, Height, ERGBFormat::BGRA, 8); 

    const TArray64<uint8>& LoadedData = ImageWrapper->GetCompressed(0.4); 

    if (LoadedData.Num() == 0) 
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to compress image data to PNG format!")); 
        return;
    }

    // Send the compressed image data over TCP
    if (!bIsConnected || !TCPSocketRef) return;

    // Send the size of the data first
    int32 DataSize = LoadedData.Num();
    int32 BytesSentSize = 0;
    bool bSizeSuccess = TCPSocketRef->Send((uint8*)&DataSize, sizeof(DataSize), BytesSentSize);

    if (!bSizeSuccess || BytesSentSize <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to send size of image data over TCP! Sent bytes: %d"), BytesSentSize);
        return;
    }

    // Now send the actual image data
    int32 BytesSent = 0;
    bool bDataSuccess = TCPSocketRef->Send(LoadedData.GetData(), LoadedData.Num(), BytesSent);

    if (!bDataSuccess || BytesSent <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to send image data over TCP! Sent bytes: %d"), BytesSent);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Successfully sent %d bytes of image data."), BytesSent);

    // Optionally, send a termination byte (0xFF) to indicate end of message
    uint8 TerminationByte = 0xFF;
    TCPSocketRef->Send(&TerminationByte, sizeof(TerminationByte), BytesSent);
}

void APixelProcessor::SetupUDP()
{
    UdpSocketRef = CreateUDPSocket();

    // Send address 
    const FString RemoteIP = "127.0.0.1";
    const int32 RemotePort = 12345;
    RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    bool bIsValid = false;
    RemoteAddress->SetIp(*RemoteIP, bIsValid);

    if (!bIsValid)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid IP address: %s"), *RemoteIP);
        UdpSocketRef->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(UdpSocketRef);
        return;
    }
    RemoteAddress->SetPort(RemotePort);

    // Recieve address 
    const int32 RecievePort = 54321;
    RecieveAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    RecieveAddress->SetIp(*RemoteIP, bIsValid);

    if (!bIsValid)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid IP address: %s"), *RemoteIP);
        UdpSocketRef->Close();
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(UdpSocketRef);
        return;
    }
    RecieveAddress->SetPort(RecievePort);

    if (!UdpSocketRef->Bind(*RecieveAddress))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to bind UDP socket."));
    }

    
    RTResource = RenderTarget->GameThread_GetRenderTargetResource();
    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
    bIsConnected = true;


    CachedTexture = UTexture2D::CreateTransient(128, 128, PF_R8G8B8A8); 
    PlatformData = CachedTexture->GetPlatformData();   
    MyWidgetInstance->SetMyTexture(CachedTexture);    
    //IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper")); 
    ImageWrapper1 = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);  
}

UTexture2D* APixelProcessor::CreateTextureFromImageData(const TArray<uint8>& ImageData, int32 Width, int32 Height)
{
    //if (ImageData.Num() == 0)
    //{
    //    UE_LOG(LogTemp, Warning, TEXT("ImageData is empty."));
    //    return nullptr;
    //}

    //UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);

    //if (!Texture)
    //{
    //    UE_LOG(LogTemp, Warning, TEXT("Failed to create UTexture2D."));
    //    return nullptr;
    //}

    //FTexturePlatformData* PlatformData = Texture->GetPlatformData();
    //void* TextureData = PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);

    //IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
    //TSharedPtr<IImageWrapper> ImageWrapper1 = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);


    //if (ImageWrapper1->SetCompressed(ImageData.GetData(), ImageData.Num()))
    //{
    //    TArray<uint8> UncompressedRGBA;
    //    if (ImageWrapper1->GetRaw(ERGBFormat::RGBA, 8, UncompressedRGBA))
    //    {
    //        //TArray<uint8> OutRGBAData;
    //        //ConvertRGBToRGBA(ImageData, OutRGBAData);
    //        FMemory::Memcpy(TextureData, UncompressedRGBA.GetData(), UncompressedRGBA.Num());
    //    }
    //    else
    //    {
    //        UE_LOG(LogTemp, Warning, TEXT("Failed to decompress PNG data."));
    //    }
    //}
    //else
    //{
    //    UE_LOG(LogTemp, Warning, TEXT("Failed to set PNG data in image wrapper."));
    //}

    //PlatformData->Mips[0].BulkData.Unlock();

    //Texture->UpdateResource();

    //return Texture;
    return nullptr;
}

void APixelProcessor::CreateAndSetTextureAtRuntime()
{
    if (ImageBuffer.Num() == 0) return;
    void* TextureData = PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE); 

    if (ImageWrapper1->SetCompressed(ImageBuffer.GetData(), ImageBuffer.Num()))
        {
            TArray<uint8> UncompressedRGBA; 
            if (ImageWrapper1->GetRaw(ERGBFormat::RGBA, 8, UncompressedRGBA)) 
            {
                FMemory::Memcpy(TextureData, UncompressedRGBA.GetData(), UncompressedRGBA.Num());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to decompress JPEG data."));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to set PNG data in image wrapper."));
        }
    PlatformData->Mips[0].BulkData.Unlock(); 
    CachedTexture->UpdateResource();
}

void APixelProcessor::ProcessUDP()
{
    if (!RenderTarget) return;
    if (!RTResource) return;
    if (!bIsConnected) return;
    if (!UdpSocketRef) return;

    TArray<FColor> PixelData;
    RTResource->ReadPixels(PixelData);

    if (PixelData.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No pixel data read from render target!"));
        return;
    }

    int32 Width = RenderTarget->SizeX;
    int32 Height = RenderTarget->SizeY;

    ////////////// only capture face 
    int32 StartX = 440; //660
    int32 StartY = 200; // 300   
    int32 RegionWidth = 128; // 192
    int32 RegionHeight = 128;  // 192 

    TArray<FColor> RegionData;  
    for (int32 y = StartY; y < StartY + RegionHeight; ++y)  
    {
        for (int32 x = StartX; x < StartX + RegionWidth; ++x)  
        {
            RegionData.Add(PixelData[y * Width + x]);  
        }
    }

    if (RegionData.Num() == 0)  
    {
        UE_LOG(LogTemp, Warning, TEXT("No pixel data extracted from the specified region!"));  
        return;
    }



    /////////////////////////

    ImageWrapper->SetRaw(RegionData.GetData(), RegionData.Num() * sizeof(FColor), RegionWidth, RegionHeight, ERGBFormat::BGRA, 8); 
      
    const TArray64<uint8>& LoadedData = ImageWrapper->GetCompressed(0.4);

    if (LoadedData.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to compress image data to PNG format!"));
        return;
    } 

    //UE_LOG(LogTemp, Error, TEXT("OutsideW hile loop %d"), LoadedData.Num());

    const int32 ChunkSize = 65507; 
    int32 Offset = 0;
    int n = 0;
    while (Offset < LoadedData.Num())
    {
        if (n % 3 == 0) {
            FPlatformProcess::Sleep(0.001f);
        }
        n++;
        int32 Size = FMath::Min(ChunkSize, LoadedData.Num() - Offset);
        TArray<uint8> ChunkData;
        ChunkData.AddUninitialized(Size);
        FMemory::Memcpy(ChunkData.GetData(), LoadedData.GetData() + Offset, Size);

        int32 BytesSent = 0;
        //UE_LOG(LogTemp, Error, TEXT("Inside While loop %d"), ChunkData.Num());
        bool bSuccess = UdpSocketRef->SendTo(ChunkData.GetData(), Size, BytesSent, *RemoteAddress);
        if (!bSuccess)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to send image data chunk."));
            break;
        }

        Offset += Size;


    }
    //FPlatformProcess::Sleep(0.001f);

    const FString EndMarker = TEXT("END_OF_MESSAGE");
    TArray<uint8> EndMarkerData;
    EndMarkerData.SetNumUninitialized(EndMarker.Len());
    FMemory::Memcpy(EndMarkerData.GetData(), TCHAR_TO_UTF8(*EndMarker), EndMarker.Len());

    int32 BytesSent = 0;
    bool bSuccess = UdpSocketRef->SendTo(EndMarkerData.GetData(), EndMarkerData.Num(), BytesSent, *RemoteAddress);
    if (!bSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to send end-of-message marker."));
    }
    /*else
    {
        UE_LOG(LogTemp, Log, TEXT("End-of-message marker sent."));
    }*/

    // Log completion
    //UE_LOG(LogTemp, Log, TEXT("Image data sent successfully!"));
    //bIsConnected = false;
}

void APixelProcessor::ProcessReassembledData(const TArray<uint8>& ReassembledData)
{
    UE_LOG(LogTemp, Log, TEXT("Processing reassembled data, size: %d"), ReassembledData.Num()); 
}

void APixelProcessor::SaveImageFromBuffer()
{
    //CreateTextureFromImageData()
    FString FilePath = FPaths::ProjectDir() + TEXT("ReceivedImage.png");  
    if (FFileHelper::SaveArrayToFile(ImageBuffer, *FilePath)) 
    {
        UE_LOG(LogTemp, Log, TEXT("Image saved to %s"), *FilePath); 
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save image to %s"), *FilePath); 
    }
}




// optimization plans
// Try webRTC
// Try Experimental WebSocket implementation
// Use share memory segment (being used by OBS and other video capture software) <--
// Compress the image as jpeg with 40 compression (JPEG smaller than PNG)
// Send only the frame around the character face
// Send the video stream in lower FPS (~20 FPS)