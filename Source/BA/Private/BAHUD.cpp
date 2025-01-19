// Fill out your copyright notice in the Description page of Project Settings.


#include "BAHUD.h"
#include "Misc/FileHelper.h"
#include "Engine/Engine.h"
#include "Editor.h"
#include "Engine/GameViewportClient.h"
#include "Engine/RendererSettings.h"
#include "Math/IntPoint.h"
#include "ImageUtils.h" // For screenshot capture
#include "GameFramework/GameUserSettings.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Engine/World.h"
#include "Kismet/KismetRenderingLibrary.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "IAssetViewport.h"
#endif
#include <Kismet/GameplayStatics.h>


void ABAHUD::DrawHUD()
{
    Super::DrawHUD();

    /*
    static float cpua = 0.0;

    // CPU/RAM/GPU auslesen
    float CpuUsage = ABAGameModeBase::StaticClass()->GetDefaultObject<ABAGameModeBase>()->GetCpuUsage();
    float RamUsage = ABAGameModeBase::StaticClass()->GetDefaultObject<ABAGameModeBase>()->GetRamUsage();
    //float GpuUsage = ABAGameModeBase::StaticClass()->GetDefaultObject<ABAGameModeBase>()->GetGpuUsage();

    //GetWorld()->Exec(nullptr, TEXT("stat unit"));

    float GpuUsage = GetWorld()->GetGameViewport()->GetStatUnitData()->GPUFrameTime[0];
    CpuUsage = GetWorld()->GetGameViewport()->GetStatUnitData()->FrameTime;
    float x = GetWorld()->GetGameViewport()->GetStatUnitData()->GameThreadTime;
    float y = GetWorld()->GetGameViewport()->GetStatUnitData()->RenderThreadTime;

    // Optional: Anzeige der Systeminfos
    FString Info = FString::Printf(TEXT("CPU: %f%%"), CpuUsage);
    DrawText(Info, FLinearColor::White, 50, 50);
    Info = FString::Printf(TEXT("RAM: %f%%"), RamUsage);
    DrawText(Info, FLinearColor::White, 50, 80);
    Info = FString::Printf(TEXT("GPU: %f%%"), GpuUsage);
    DrawText(Info, FLinearColor::White, 50, 110);
    Info = FString::Printf(TEXT("GPU: %f%%"), x);
    DrawText(Info, FLinearColor::White, 50, 140);
    Info = FString::Printf(TEXT("GPU: %f%%"), y);
    DrawText(Info, FLinearColor::White, 50, 170);

    */

    if (bIsSavingFrames && SavedFrameIndex < CapturedFrames.Num())
    {
        currentDelay += _World->GetDeltaSeconds();
        if (currentDelay > MaxSaveFrameDelay)
        {
            FString SessionFolderPath = FPaths::ProjectDir() + CurrentSessionName;

            const FCapturedFrameData& FrameData = CapturedFrames[SavedFrameIndex];

            // Generate a unique filename for each frame
            FString FileName = FString::Printf(TEXT("Frame_%d.png"), SavedFrameIndex);
            FString FilePath = SessionFolderPath + TEXT("/") + FileName;

            // Convert raw pixel data to an image
            TArray64<uint8> PNGData; // Use uint8 for image data

            // Create FImageView using a pointer to the raw pixel data (FColor) and the dimensions
            FImageView ImageView(FrameData.ColorBuffer.GetData(), FrameData.BufferSize.X, FrameData.BufferSize.Y);

            // Compress the image to PNG
            if (FImageUtils::CompressImage(PNGData, TEXT("png"), ImageView))
            {
                // Write the PNG data to a file
                if (FFileHelper::SaveArrayToFile(PNGData, *FilePath))
                {
                    UE_LOG(LogTemp, Log, TEXT("Saved frame %d as PNG at %s"), SavedFrameIndex, *FilePath);
                    OnFrameSaved.Broadcast(SavedFrameIndex, CapturedFrames.Num());
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Failed to save frame %d as PNG."), SavedFrameIndex);
                }
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to compress frame %d to PNG."), SavedFrameIndex);
            }
            SavedFrameIndex += 1;
            currentDelay = 0.0f;
        }

        if (SavedFrameIndex >= CapturedFrames.Num())
        {
            SavedFrameIndex = 0;
            bIsSavingFrames = false;
        }
    }

    if (!bIsRecording)
        return;

    if (!_World)
        return;

    _World->GetGameViewport()->SetShowStats(false);

    const FStatUnitData* StatData = _World->GetGameViewport()->GetStatUnitData();
    if (!StatData)
        return;

    _World->GetGameViewport()->SetShowStats(false);

    // Record the current frame's stats
    FFrameStatData FrameData;
    FrameData.FrameID = CurrentFrameID;
    FrameData.DeltaTime = _World->GetDeltaSeconds();
    FrameData.GameThreadTime = StatData->GameThreadTime;
    FrameData.RenderThreadTime = StatData->RenderThreadTime;
    FrameData.GPUFrameTime = StatData->GPUFrameTime[0];
    RecordedStats.Add(FrameData);

    FrameGrabber->CaptureThisFrame(FFramePayloadPtr());

    if (CurrentFrameID > MaxFrameAmount)
    {
        StopRecording();
    }

    CurrentFrameID++;
}

bool ABAHUD::RecordStats()
{
    bIsRecording = !bIsRecording;
    if (bIsRecording)
    {
        StartRecording();
    }
    else
    {
        StopRecording();
    }
    return bIsRecording;
}

void ABAHUD::StartRecording()
{
    SetFrameRate(30.0f);

    bIsRecording = true;

    RecordedStats.Empty();
    CurrentFrameID = 0;
    _World = GetWorld();

    if (_World)
        _World->GetGameViewport()->SetShowStats(false);

    CurrentSessionName = GenerateSessionName();

    //UGameplayStatics::GetGameInstance(this)->GetEngine()->StartFPSChart(CurrentSessionName, true);

    SaveRenderSettingsToFile();

    // Create a folder for this session if it doesn't exist
    FString SessionFolderPath = FPaths::ProjectDir() + CurrentSessionName;
    if (!FPaths::DirectoryExists(SessionFolderPath))
    {
        IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
        PlatformFile.CreateDirectory(*SessionFolderPath);
    }
    
    // ---------------------------------------
    
    TSharedPtr<FSceneViewport> SceneViewport;

    // Get SceneViewport
    // ( quoted from FRemoteSessionHost::OnCreateChannels() )
#if WITH_EDITOR
    if (GIsEditor)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::PIE)
            {
                FSlatePlayInEditorInfo* SlatePlayInEditorSession = GEditor->SlatePlayInEditorMap.Find(Context.ContextHandle);
                if (SlatePlayInEditorSession)
                {
                    if (SlatePlayInEditorSession->DestinationSlateViewport.IsValid())
                    {
                        TSharedPtr<IAssetViewport> DestinationLevelViewport = SlatePlayInEditorSession->DestinationSlateViewport.Pin();
                        SceneViewport = DestinationLevelViewport->GetSharedActiveViewport();
                    }
                    else if (SlatePlayInEditorSession->SlatePlayInEditorWindowViewport.IsValid())
                    {
                        SceneViewport = SlatePlayInEditorSession->SlatePlayInEditorWindowViewport;
                    }
                }
            }
        }
    }
    else
#endif
    {
        UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
        SceneViewport = GameEngine->SceneViewport;
    }
    if (!SceneViewport.IsValid())
    {
        return;
    }

    // Capture Start
    FrameGrabber = MakeShareable(new FFrameGrabber(SceneViewport.ToSharedRef(), SceneViewport->GetSize()));
    FrameGrabber->StartCapturingFrames();
    OnRecordingStarted.Broadcast();
}

void ABAHUD::StopRecording()
{
    bIsRecording = false;
    FrameGrabber->StopCapturingFrames();
    CapturedFrames = FrameGrabber->GetCapturedFrames();
    SetFrameRate(0.0f);
    //UGameplayStatics::GetGameInstance(this)->GetEngine()->StopFPSChart(GetWorld()->GetMapName());
    SaveStatsToFile();
    // Log CapturedFrames.Num here
    //UE_LOG(LogTemp, Log, TEXT("Captured Frames: %d"), CapturedFrames.Num());
    SchreibDenScheis();
    OnRecordingStopped.Broadcast();
}

void ABAHUD::SetFrameRate(float FrameRate)
{
    // reset FPS
    UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();
    if (UserSettings)
    {
        UserSettings->SetFrameRateLimit(FrameRate); // Set FPS limit to 30
        UserSettings->ApplySettings(false);    // Apply without restarting
    }
}

void ABAHUD::BeginPlay()
{
    Super::BeginPlay();

    SetupRenderTarget();
}

void ABAHUD::BeginDestroy()
{
    Super::BeginDestroy();

    ReleaseFrameGrabber();
}

void ABAHUD::ReleaseFrameGrabber()
{
    if (FrameGrabber.IsValid())
    {
        //FrameGrabber->StopCapturingFrames();
        FrameGrabber->Shutdown();
        FrameGrabber.Reset();
    }
}

void ABAHUD::SaveStatsToFile()
{
    FString SavePath = FPaths::ProjectDir() + CurrentSessionName + TEXT("_Stats.csv");

    FString OutputString = TEXT("FrameID,DeltaTime,FrameTime,GameThreadTime,RenderThreadTime,GPUFrameTime\n");

    for (const FFrameStatData& Data : RecordedStats)
    {
        OutputString += FString::Printf(
            TEXT("%d,%.6f,%.6f,%.6f,%.6f,%.6f\n"),
            Data.FrameID,
            Data.DeltaTime,
            Data.FrameTime,
            Data.GameThreadTime,
            Data.RenderThreadTime,
            Data.GPUFrameTime
        );
    }

    FFileHelper::SaveStringToFile(OutputString, *SavePath);
}

void ABAHUD::SaveRenderSettingsToFile()
{
    FString SavePath = FPaths::ProjectDir() + CurrentSessionName + TEXT("_RenderSettings.txt");

    // Retrieve shading path using runtime console variable
    FString ShadingPath;
    if (GEngine && GEngine->GameViewport)
    {
        int32 ForwardShading = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ForwardShading"))->GetInt();
        ShadingPath = ForwardShading == 1 ? TEXT("Forward") : TEXT("Deferred");
    }

    // Retrieve screen resolution
    int32 ScreenWidth = 0, ScreenHeight = 0;
    if (GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
    {
        FIntPoint ViewportSize = GEngine->GameViewport->Viewport->GetSizeXY();
        ScreenWidth = ViewportSize.X;
        ScreenHeight = ViewportSize.Y;
    }

    // Retrieve anti-aliasing settings
    FString AntiAliasing;
    FString AASettingsDetails;
    int32 MSAA_Samples = 0; // Move this initialization outside the switch

    const URendererSettings* RendererSettings = GetDefault<URendererSettings>();
    if (RendererSettings)
    {
        switch (RendererSettings->DefaultFeatureAntiAliasing)
        {
        case AAM_None:
            AntiAliasing = TEXT("None");
            break;
        case AAM_FXAA:
            AntiAliasing = TEXT("FXAA");
            break;
        case AAM_TemporalAA:
            AntiAliasing = TEXT("TAA");
            break;
        case AAM_MSAA:
            AntiAliasing = TEXT("MSAA");
            // Check the number of MSAA samples
            MSAA_Samples = IConsoleManager::Get().FindConsoleVariable(TEXT("r.MSAACount"))->GetInt();
            AASettingsDetails = FString::Printf(TEXT("Samples: %dx"), MSAA_Samples); // Add MSAA sample count
            break;
        default:
            AntiAliasing = TEXT("Unknown");
            break;
        }
    }

    // Write to file
    FString OutputString = FString::Printf(
        TEXT("ShadingPath: %s\nResolution: %dx%d\nAntiAliasing: %s %s\n"),
        *ShadingPath,
        ScreenWidth,
        ScreenHeight,
        *AntiAliasing,
        *AASettingsDetails // Append MSAA sub-settings if applicable
    );

    FFileHelper::SaveStringToFile(OutputString, *SavePath);
}


FString ABAHUD::GenerateSessionName() const
{
    const FDateTime Now = FDateTime::Now();
    return Now.ToString(TEXT("%Y%m%d_%H%M%S"));
}

void ABAHUD::SetupRenderTarget()
{
    if (!RenderTarget)
    {
        RenderTarget = NewObject<UTextureRenderTarget2D>();
        RenderTarget->InitAutoFormat(ResolutionWidth, ResolutionHeight);
        RenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
        RenderTarget->bAutoGenerateMips = false;
        RenderTarget->AddToRoot();
    }

    if (!SceneCaptureComponent)
    {
        // Attach a Scene Capture Component
        SceneCaptureComponent = NewObject<USceneCaptureComponent2D>(this);
        SceneCaptureComponent->TextureTarget = RenderTarget;
        SceneCaptureComponent->CaptureSource = SCS_FinalColorLDR;
        SceneCaptureComponent->ShowFlags.SetPostProcessing(true); // Enable post-processing
        SceneCaptureComponent->ShowFlags.SetAntiAliasing(true);   // Enable AA
        SceneCaptureComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        SceneCaptureComponent->RegisterComponent();
    }
}

void ABAHUD::SchreibDenScheis()
{
    // Validate if there are captured frames
    if (CapturedFrames.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No frames captured to write."));
        return;
    }

    bIsSavingFrames = true;

    // Print the number of captured frames in the log
    /* 
    //UE_LOG(LogTemp, Log, TEXT("Captured Frames: %d"), CapturedFrames.Num());

    // Get the session folder path
    FString SessionFolderPath = FPaths::ProjectDir() + CurrentSessionName;

    // Ensure the directory exists before attempting to save the files
    if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*SessionFolderPath))
    {
        if (FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*SessionFolderPath))
        {
            UE_LOG(LogTemp, Log, TEXT("Created session folder at %s"), *SessionFolderPath);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to create session folder at %s"), *SessionFolderPath);
            return;
        }
    }

    // Loop through each captured frame and write it as a PNG
    for (int32 FrameIndex = 0; FrameIndex < CapturedFrames.Num(); ++FrameIndex)
    {
        const FCapturedFrameData& FrameData = CapturedFrames[FrameIndex];

        // Generate a unique filename for each frame
        FString FileName = FString::Printf(TEXT("Frame_%d.png"), FrameIndex);
        FString FilePath = SessionFolderPath + TEXT("/") + FileName;

        // Convert raw pixel data to an image
        TArray64<uint8> PNGData; // Use uint8 for image data

        // Create FImageView using a pointer to the raw pixel data (FColor) and the dimensions
        FImageView ImageView(FrameData.ColorBuffer.GetData(), FrameData.BufferSize.X, FrameData.BufferSize.Y);

        // Compress the image to PNG
        if (FImageUtils::CompressImage(PNGData, TEXT("png"), ImageView))
        {
            // Write the PNG data to a file
            if (FFileHelper::SaveArrayToFile(PNGData, *FilePath))
            {
                UE_LOG(LogTemp, Log, TEXT("Saved frame %d as PNG at %s"), FrameIndex, *FilePath);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to save frame %d as PNG."), FrameIndex);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to compress frame %d to PNG."), FrameIndex);
        }
    }
    */
}


