// Fill out your copyright notice in the Description page of Project Settings.


#include "BAHUD.h"
#include "Misc/FileHelper.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/RendererSettings.h"
#include "Math/IntPoint.h"
#include "ImageUtils.h" // For screenshot capture
#include "GameFramework/GameUserSettings.h"
#include "HAL/PlatformFilemanager.h"
#include "ImageUtils.h" // For screenshot capture
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


    // for saving
    /*
    
    if (bIsSavingFrames && SavedFrameIndex < CapturedFrames.Num() && !bSaveAll)
    {
        currentDelay += _World->GetDeltaSeconds();
        if (currentDelay > MaxSaveFrameDelay)
        {
            FString SessionFolderPath = FPaths::ProjectDir() + CurrentSessionName + prefix;

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
    

    if (bSaveAll && Settings.Num() > 0 && !bIsSavingFrames)
    {
        
        currentDelay += _World->GetDeltaSeconds();
        if (currentDelay > MaxSaveFrameDelay)
        {
            FString SessionFolderPath = FPaths::ProjectDir() + _currentSessionName;


            // Ensure the session folder exists
            if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*SessionFolderPath))
            {
                FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*SessionFolderPath);
                UE_LOG(LogTemp, Log, TEXT("Created path %s"), *SessionFolderPath);
            }

            // Saving masterframe
            FString MasterFramePath = SessionFolderPath + TEXT("/MasterFrame.png");
            if (!MasterFrame.IsEmpty())
            {
                TArray64<uint8> _PNGData;
                FImageView ImgView(MasterFrame.GetData(), MasterFrameBuffer.X, MasterFrameBuffer.Y);
                if (FImageUtils::CompressImage(_PNGData, TEXT("PNG"), ImgView))
                {
                    FFileHelper::SaveArrayToFile(_PNGData, *MasterFramePath);
                    UE_LOG(LogTemp, Log, TEXT("Saved masterframe"));
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Failed to compress masterframe"));
                }
            }

            UE_LOG(LogTemp, Log, TEXT("Saving data: %d"), Settings.Num());

            // Iterate over each setting and save the data
            for (const auto& Setting : Settings)
            {
                // Generate a sub-folder with the session name, setting name, and setting value
                FString SubFolderPath = FString::Printf(TEXT("%s/%s_%s"), *SessionFolderPath, *Setting.Key, *Setting.Value);
                if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*SubFolderPath))
                {
                    FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*SubFolderPath);
                    UE_LOG(LogTemp, Log, TEXT("Created sub folder %s"), *SubFolderPath);
                }

                // Save the frame as an image in the sub-folder
                FString ImageFilePath = SubFolderPath + TEXT("/Frame.png");
                if (!Setting.Frame.IsEmpty())
                {
                    TArray64<uint8> PNGData;
                    FImageView ImageView(Setting.Frame.GetData(), _bufferSizeX, _bufferSizeY);
                    if (FImageUtils::CompressImage(PNGData, TEXT("png"), ImageView))
                    {
                        FFileHelper::SaveArrayToFile(PNGData, *ImageFilePath);
                        UE_LOG(LogTemp, Log, TEXT("Saved frame for setting %s=%s at %s"), *Setting.Key, *Setting.Value, *ImageFilePath);
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Failed to compress frame for setting %s=%s"), *Setting.Key, *Setting.Value);
                    }
                }

                // Save the stat data as a CSV in the sub-folder
                FString StatFilePath = SubFolderPath + TEXT("/Stats.csv");
                FString StatOutput = TEXT("FrameID,DeltaTime,FrameTime,GameThreadTime,RHITTime,RenderThreadTime,GPUFrameTime,Quality\n");
                StatOutput += FString::Printf(
                    TEXT("%d,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n"),
                    Setting.FrameStats.FrameID,
                    Setting.FrameStats.DeltaTime,
                    Setting.FrameStats.FrameTime,
                    Setting.FrameStats.GameThreadTime,
                    Setting.FrameStats.RHITTime,
                    Setting.FrameStats.RenderThreadTime,
                    Setting.FrameStats.GPUFrameTime,
                    Setting.Quality
                );
                FFileHelper::SaveStringToFile(StatOutput, *StatFilePath);
                UE_LOG(LogTemp, Log, TEXT("Saved stats for setting %s=%s at %s"), *Setting.Key, *Setting.Value, *StatFilePath);

                Setting.Frame.IsEmpty();
            }
            
            SavedFrameIndex += 1;
            currentDelay = 0.0f;
        }

        if (SavedFrameIndex >= Settings.Num())
        {
            SavedFrameIndex = 0;
            bSaveAll = false;
            currentSetting.Frame.Empty();
            Settings.Empty();
        }
    }
    */


    // for recording

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
    FFrameStatData_internal FrameData;
    FrameData.FrameID = CurrentFrameID;
    FrameData.DeltaTime = _World->GetDeltaSeconds();
    FrameData.FrameTime = StatData->FrameTime;
    FrameData.GameThreadTime = StatData->GameThreadTime;
    FrameData.RHITTime = StatData->RHITTime;
    FrameData.RenderThreadTime = StatData->RenderThreadTime;
    FrameData.GPUFrameTime = StatData->GPUFrameTime[0];
    RecordedStats.Add(FrameData);

    // We always need to record one more frame because stats are from last frame

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
    //SetFrameRate(TargetFrameRate);

    bIsRecording = true;

    RecordedStats.Empty();
    CurrentFrameID = 0;
    _World = GetWorld();

    if (_World)
        _World->GetGameViewport()->SetShowStats(false);

    CurrentSessionName = GenerateSessionName();

    //UGameplayStatics::GetGameInstance(this)->GetEngine()->StartFPSChart(CurrentSessionName, true);

    //removed for now
    // SaveRenderSettingsToFile();

    /* Create a folder for this session if it doesn't exist
    FString SessionFolderPath = FPaths::ProjectDir() + CurrentSessionName + prefix;
    if (!FPaths::DirectoryExists(SessionFolderPath))
    {
        IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
        PlatformFile.CreateDirectory(*SessionFolderPath);
    }
    */
    
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

    /*

    // Capturing Settings and Frame
    if (bCaptureSetting && !bCaptureMasterFrame)
    {
        _bufferSizeX = CapturedFrames[0].BufferSize.X;
        _bufferSizeY = CapturedFrames[0].BufferSize.Y;
        // Process Frames
        FFrameStatData_internal last = RecordedStats.Last();
        FFrameStatData currentStats = {
            currentStats.FrameID = last.FrameID,
            currentStats.DeltaTime = last.DeltaTime,
            currentStats.FrameTime = last.FrameTime,
            currentStats.GameThreadTime = last.GameThreadTime,
            currentStats.GPUFrameTime = last.GPUFrameTime,
            currentStats.RenderThreadTime = last.RenderThreadTime,
            currentStats.RHITTime = last.RHITTime
        };
        currentSetting.FrameStats = currentStats;
        int32 index = CapturedFrames.Num() - 1;
        if (CapturedFrames.Num() > 1)
            index = CapturedFrames.Num() - 2;
        currentSetting.Frame = CapturedFrames[index].ColorBuffer;
        // SSIM here
        float ssim = ColorSSIM(MasterFrame, currentSetting.Frame);
        currentSetting.Quality = ssim;
        Settings.Add(currentSetting);
        bCaptureSetting = false;
        OnRecordingStopped.Broadcast();
    }

    // Capturing Masterframe
    if (bCaptureMasterFrame && !bCaptureSetting)
    {
        FIntPoint size = (CapturedFrames[0].BufferSize.X, CapturedFrames[0].BufferSize.Y);
        MasterFrame = CapturedFrames.Last().ColorBuffer;
        MasterFrameBuffer = CapturedFrames.Last().BufferSize;
        bCaptureMasterFrame = false;
        OnRecordingMasterFrameStopped.Broadcast();
    }
    */
    OnRecordingMasterFrameStopped.Broadcast();

    // ----------
    //SetFrameRate(0.0f);
    //UGameplayStatics::GetGameInstance(this)->GetEngine()->StopFPSChart(GetWorld()->GetMapName());
    //SaveStatsToFile();
    // Log CapturedFrames.Num here
    //UE_LOG(LogTemp, Log, TEXT("Captured Frames: %d"), CapturedFrames.Num());
    //SchreibDenScheis();
    // -------------
}

void ABAHUD::SetTargetFrameRate(float FrameRate)
{
    if (FrameRate <= 0 || bIsRecording)
        return;

    TargetFrameRate = FrameRate;
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

void ABAHUD::SetMaxFrameAmount(int32 FrameAmount)
{
    if (FrameAmount <= 0 || bIsRecording)
        return;

    MaxFrameAmount = FrameAmount;
}

void ABAHUD::SaveAllData()
{
    _currentSessionName = GenerateSessionName();
    bSaveAll = true;
    // generate a sessionname
    // use this sessionname as a foldername
    // for each setting in Settings:
    //      generate a sub folder with sessionname+settingname+settingvalue
    //      write frame here
    //      write statdata her as csv
}

TArray<FSetting> ABAHUD::GetOptimalSettings()
{
    // Target frame time in milliseconds (1000ms / FPS)
    float TargetFrameTime = 1000.0f / TargetFrameRate;

    // Sort settings by quality in descending order (higher quality first)
    Settings.Sort([](const FSetting& A, const FSetting& B)
    {
        return A.Quality > B.Quality; // Higher quality first
    });

    TArray<FSetting> OptimalSettings;
    float CurrentTotalFrameTime = 0.0f;

    // Step 1: Select the highest quality settings within the target frame time
    for (const FSetting& Setting : Settings)
    {
        float SettingFrameTime = Setting.FrameStats.FrameTime; // Using FrameTime as the key metric

        if (CurrentTotalFrameTime + SettingFrameTime <= TargetFrameTime)
        {
            OptimalSettings.Add(Setting);
            CurrentTotalFrameTime += SettingFrameTime;
        }
    }

    return OptimalSettings;
}

void ABAHUD::RecordingDataToMasterFrame()
{
    FIntPoint size = (CapturedFrames[0].BufferSize.X, CapturedFrames[0].BufferSize.Y);
    MasterFrame = CapturedFrames.Last().ColorBuffer;
    MasterFrameBuffer = CapturedFrames.Last().BufferSize;
    //bCaptureMasterFrame = false;
}

void ABAHUD::RecordingDataToSetting(FString Key, FString Value)
{
    FSetting setting;
    setting.Key = Key;
    setting.Value = Value;
    setting.Frame = CapturedFrames[CapturedFrames.Num() - 1].ColorBuffer;
    setting.Quality = ColorSSIM(MasterFrame, setting.Frame);
    Settings.Add(setting);
}

/*
TArray<FColor>& ABAHUD::GetFrameAt(int32 Index)
{
    // TODO: insert return statement here
}

FFrameStatData ABAHUD::GetStatDataAt(int32 Index)
{
    return FFrameStatData();
}
*/

void ABAHUD::RecordSetting(FString Setting, FString Value)
{
    bCaptureSetting = true;
    bCaptureQuality = false;
    bCaptureMasterFrame = false;
    prefix = Setting + TEXT("_") + Value;
    currentSetting.Key = Setting;
    currentSetting.Value = Value;
    //currentSetting.
    currentSetting.Frame.Empty();
    //Settings.Empty();
    StartRecording();
}

void ABAHUD::RecordQuality(FString Category, FString Quality, TArray<FSetting> QualitySettings)
{
    bCaptureSetting = false;
    bCaptureQuality = true;
    bCaptureMasterFrame = false;
    prefix = Category + TEXT("_") + Quality;
    currentSetting.Key = Category;
    currentSetting.Value = Quality;
    currentSetting.Frame.Empty();
    StartRecording();
}

void ABAHUD::RecordMasterFrame()
{
    bCaptureSetting = false;
    bCaptureQuality = false;
    bCaptureMasterFrame = true;
    StartRecording();
}

float ABAHUD::ColorSSIM(TArray<FColor> x, TArray<FColor> y)
{
    return ColorSSIM_Internal(x, y);
}

void ABAHUD::BeginPlay()
{
    Super::BeginPlay();

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

float ABAHUD::ColorSSIM_Internal(TArray<FColor> x, TArray<FColor> y)
{
    // Check if both input arrays have the same size
    if (x.Num() != y.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("SSIM_Internal: Input arrays must have the same size."));
        return 0.0f;
    }

    // Convert FColor arrays to grayscale FLinearColor arrays
    TArray<float> GrayX_R, GrayX_G, GrayX_B;
    TArray<float> GrayY_R, GrayY_G, GrayY_B;

    for (int32 i = 0; i < x.Num(); ++i)
    {
        // Extract RGBA values from FColor
        const FColor& ColorX = x[i];
        const FColor& ColorY = y[i];

        // Add to grayscale arrays
        GrayX_R.Add(FMath::Max(FMath::Min(ColorX.R, (uint8)255), (uint8)0));
        GrayX_G.Add(FMath::Max(FMath::Min(ColorX.G, (uint8)255), (uint8)0));
        GrayX_B.Add(FMath::Max(FMath::Min(ColorX.B, (uint8)255), (uint8)0));

        GrayY_R.Add(FMath::Max(FMath::Min(ColorY.R, (uint8)255), (uint8)0));
        GrayY_G.Add(FMath::Max(FMath::Min(ColorY.G, (uint8)255), (uint8)0));
        GrayY_B.Add(FMath::Max(FMath::Min(ColorY.B, (uint8)255), (uint8)0));
    }

    // Compute SSIM for each channel
    float SSIM_R = SSIM_Internal(GrayX_R, GrayY_R);
    float SSIM_G = SSIM_Internal(GrayX_G, GrayY_G);
    float SSIM_B = SSIM_Internal(GrayX_B, GrayY_B);

    // Calculate the average SSIM
    //float FinalSSIM = SSIM_R * SSIM_R_WEIGHT + SSIM_G * SSIM_G_WEIGHT + SSIM_B * SSIM_B_WEIGHT;
    float FinalSSIM = (SSIM_R + SSIM_G + SSIM_B) / 3.0f;

    return FinalSSIM;
}

float ABAHUD::SSIM_Internal(TArray<float> x, TArray<float> y)
{
    // Check values, might need to calculate for this instance first
    if (_meanX <= -1.0f)
    {
        _meanX = 0.0f;
        for (int i = 0; i < x.Num(); i++)
        {
            _meanX += x[i];
        }
        _meanX = _meanX / ((float)x.Num());
        UE_LOG(LogTemp, Log, TEXT("Statistics: Mean X: %f"), _meanX);
    }

    if (_meanY <= -1.0f)
    {
        _meanY = 0.0f;
        for (int i = 0; i < y.Num(); i++)
        {
            _meanY += y[i];
        }
        _meanY = _meanY / ((float)y.Num());
        UE_LOG(LogTemp, Log, TEXT("Statistics: Mean Y: %f"), _meanY);
    }

    // Variance
    if (_varianceX <= -1.0f)
    {
        _varianceX = 0.0f;
        for (int i = 0; i < x.Num(); i++)
        {
            _varianceX += (x[i] - _meanX) * (x[i] - _meanX);
        }
        _varianceX = _varianceX / ((float)x.Num() - 1.0f);
        UE_LOG(LogTemp, Log, TEXT("Statistics: Variance X: %f"), _varianceX);
    }

    if (_varianceY <= -1.0f)
    {
        _varianceY = 0.0f;
        for (int i = 0; i < y.Num(); i++)
        {
            _varianceY += (y[i] - _meanY) * (y[i] - _meanY);
        }
        _varianceY = _varianceY / ((float)y.Num() - 1.0f);
        UE_LOG(LogTemp, Log, TEXT("Statistics: Variance Y: %f"), _varianceY);
    }

    // co-variance
    if (_covarianceXY <= -1.0f)
    {
        _covarianceXY = 0.0f;
        for (int i = 0; i < x.Num(); i++)
        {
            _covarianceXY += (x[i] - _meanX) * (y[i] - _meanY);
        }
        _covarianceXY = _covarianceXY / ((float(x.Num()) - 1.0f));
        UE_LOG(LogTemp, Log, TEXT("Statistics: CoVariance XY: %f"), _covarianceXY);
    }

    float ssim_value = ((2.0f * _meanX * _meanY + _c1) * (2.0f * _covarianceXY + _c2)) / ((FMath::Pow(_meanX, 2.0f) + FMath::Pow(_meanY, 2.0f) + _c1) * (_varianceX + _varianceY + _c2));
    UE_LOG(LogTemp, Log, TEXT("Statistics: SSIM: %f"), ssim_value);


    // reset SSIM values since we are done with this instance
    _meanX = -1.0f;
    _meanY = -1.0f;
    _varianceX = -1.0f;
    _varianceY = -1.0f;
    _covarianceXY = -1.0f;

    return ssim_value;
}

void ABAHUD::SaveStatsToFile()
{
    FString SavePath = FPaths::ProjectDir() + CurrentSessionName + prefix + TEXT("_Stats.csv");

    FString OutputString = TEXT("FrameID,DeltaTime,FrameTime,GameThreadTime,RHITTime,RenderThreadTime,GPUFrameTime\n");

    for (const FFrameStatData_internal& Data : RecordedStats)
    {
        OutputString += FString::Printf(
            TEXT("%d,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n"),
            Data.FrameID,
            Data.DeltaTime,
            Data.FrameTime,
            Data.GameThreadTime,
            Data.RHITTime,
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


