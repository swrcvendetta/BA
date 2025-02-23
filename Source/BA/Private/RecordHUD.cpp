// Fill out your copyright notice in the Description page of Project Settings.


#include "RecordHUD.h"
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


void ARecordHUD::DrawHUD()
{
    Super::DrawHUD();


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

void ARecordHUD::StartRecording()
{

    bIsRecording = true;

    RecordedStats.Empty();
    CurrentFrameID = 0;
    _World = GetWorld();

    if (_World)
        _World->GetGameViewport()->SetShowStats(false);

    if (CurrentSessionName.Len() <= 0)
        CurrentSessionName = GenerateSessionName();

    
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
    HUDOnRecordingStarted.Broadcast();
}

void ARecordHUD::StopRecording()
{
    bIsRecording = false;
    FrameGrabber->StopCapturingFrames();
    CapturedFrames = FrameGrabber->GetCapturedFrames();
    HUDOnRecordingStopped.Broadcast();
}

void ARecordHUD::SetTargetFrameRate(float FrameRate)
{
    if (FrameRate <= 0 || bIsRecording)
        return;

    TargetFrameRate = FrameRate;
}

void ARecordHUD::RecordingDataToMasterFrame()
{
    FIntPoint size = (CapturedFrames[0].BufferSize.X, CapturedFrames[0].BufferSize.Y);
    MasterFrame = CapturedFrames.Last().ColorBuffer;
    MasterFrameBuffer = CapturedFrames.Last().BufferSize;
    //bCaptureMasterFrame = false;
}

void ARecordHUD::RecordingDataToSetting(FString Key, FString Value)
{
    FSetting setting;
    setting.Key = Key;
    setting.Value = Value;
    //setting.Frame = CapturedFrames[CapturedFrames.Num() - 1].ColorBuffer;
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
    setting.FrameStats = currentStats;
    int32 index = CapturedFrames.Num() - 1;
    if (CapturedFrames.Num() > 1)
        index = CapturedFrames.Num() - 2;
    setting.Frame = CapturedFrames[index].ColorBuffer;
    setting.Quality = ColorSSIM(MasterFrame, setting.Frame);
    Settings.Add(setting);
    HUDOnRecordingDataToSettingFinished.Broadcast();
}

void ARecordHUD::RecordingDataToOptimizedFrame()
{
    OptimizedData.Frame = CapturedFrames.Last().ColorBuffer;
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
    OptimizedData.FrameStats = currentStats;
    OptimizedData.Quality = ColorSSIM(MasterFrame, OptimizedData.Frame);
}



float ARecordHUD::ColorSSIM(TArray<FColor> x, TArray<FColor> y)
{
    return ColorSSIM_Internal(x, y);
}

void ARecordHUD::WriteAllData()
{
    _currentSessionName = GenerateSessionName();

    // Create data dir
    FString SessionFolderPath = FPaths::ProjectDir() + _currentSessionName;
    // Ensure the session folder exists
    if (!FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*SessionFolderPath))
    {
        FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*SessionFolderPath);
        UE_LOG(LogTemp, Log, TEXT("Created path %s"), *SessionFolderPath);
    }

    // Saving Master Frame
    if (!MasterFrame.IsEmpty())
    {
        FString MasterFramePath = SessionFolderPath + TEXT("/MasterFrame.png");
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

    // Saving Optimized Frame
    if (!OptimizedData.Frame.IsEmpty())
    {
        FString OptimizedFramePath = SessionFolderPath + TEXT("/OptimizedFrame.png");
        TArray64<uint8> _PNGData2;
        FImageView ImgView(OptimizedData.Frame.GetData(), _bufferSizeX, _bufferSizeY);
        if (FImageUtils::CompressImage(_PNGData2, TEXT("PNG"), ImgView))
        {
            FFileHelper::SaveArrayToFile(_PNGData2, *OptimizedFramePath);
            UE_LOG(LogTemp, Log, TEXT("Saved masterframe"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to compress masterframe"));
        }

        // Save the stat data as a CSV in the sub-folder
        FString StatFilePath2 = SessionFolderPath + TEXT("/OptimizedStats.csv");
        FString StatOutput2 = TEXT("FrameID,DeltaTime,FrameTime,GameThreadTime,RHITTime,RenderThreadTime,GPUFrameTime,Quality\n");
        StatOutput2 += FString::Printf(
            TEXT("%d,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f\n"),
            OptimizedData.FrameStats.FrameID,
            OptimizedData.FrameStats.DeltaTime,
            OptimizedData.FrameStats.FrameTime,
            OptimizedData.FrameStats.GameThreadTime,
            OptimizedData.FrameStats.RHITTime,
            OptimizedData.FrameStats.RenderThreadTime,
            OptimizedData.FrameStats.GPUFrameTime,
            OptimizedData.Quality
        );
        FFileHelper::SaveStringToFile(StatOutput2, *StatFilePath2);
        UE_LOG(LogTemp, Log, TEXT("Saved stats for setting Optimized Frame at %s"), *StatFilePath2);

        TArray<FSetting> optimizedSettings = GetOptimizedSettings();

        // Save the stat data as a CSV in the sub-folder
        FString OptimizedFilePath = SessionFolderPath + TEXT("/OptimizedSettings.csv");
        FString OptimizedOutput;
        OptimizedOutput.Append(TEXT("Key,Value\n")); // CSV-Header

        for (const FSetting& Setting : optimizedSettings)
        {
            OptimizedOutput.Appendf(
                TEXT("%s,%s\n"),  // Beide Werte sind FString, also beide mit %s
                *Setting.Key,      // FString muss mit * dereferenziert werden
                *Setting.Value     // Auch hier dereferenzieren
            );
        }

        // Datei speichern
        if (FFileHelper::SaveStringToFile(OptimizedOutput, *OptimizedFilePath))
        {
            UE_LOG(LogTemp, Log, TEXT("Saved settings for optimized frame at %s"), *OptimizedFilePath);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to save optimized settings to %s"), *OptimizedFilePath);
        }
    }

    if (Settings.IsEmpty())
        return;

    // Saving setting data
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
}

void ARecordHUD::ClearData()
{
    MasterFrame.Empty();
    OptimizedData.Frame.Empty();
    Settings.Empty();
}

TArray<FSetting> ARecordHUD::GetOptimizedSettings()
{
    float TargetFrameTime = 1000.0f / TargetFrameRate;
    UE_LOG(LogTemp, Log, TEXT("Target Frame Time: %f ms"), TargetFrameTime); // Log the target frame time

    // Access the GameMode to retrieve categories
    ABAGameModeBase* GameMode = GetWorld()->GetAuthGameMode<ABAGameModeBase>();
    if (!GameMode)
    {
        UE_LOG(LogTemp, Error, TEXT("GameMode not found!"));
        return {};
    }

    const TArray<FCategory>& Categories = GameMode->GetCategories();

    // Sort settings by quality (higher quality first)
    Settings.Sort([](const FSetting& A, const FSetting& B)
        {
            return A.Quality > B.Quality;
        });

    UE_LOG(LogTemp, Log, TEXT("Settings sorted by quality"));

    TMap<FString, float> BaseFrameTimes; // Stores the lowest frame time per key

    // Step 1: Determine the base frame load for each key
    for (const FSetting& Setting : Settings)
    {
        if (!BaseFrameTimes.Contains(Setting.Key))
        {
            BaseFrameTimes.Add(Setting.Key, Setting.FrameStats.GPUFrameTime);
            UE_LOG(LogTemp, Log, TEXT("Setting base FrameTime for %s: %f ms"), *Setting.Key, Setting.FrameStats.GPUFrameTime);
        }
        else
        {
            BaseFrameTimes[Setting.Key] = FMath::Min(BaseFrameTimes[Setting.Key], Setting.FrameStats.GPUFrameTime);
            UE_LOG(LogTemp, Log, TEXT("Updating base FrameTime for %s: %f ms"), *Setting.Key, BaseFrameTimes[Setting.Key]);
        }
    }

    TArray<FSetting> OptimalSettings;
    float CurrentTotalFrameTime = 0.0f;
    TArray<FString> AppliedKeys;

    // Step 2: Select the optimal settings
    for (const FSetting& Setting : Settings)
    {
        // Check if Key is a single-digit number (i.e., an index for a category)
        bool bKeyIsIndex = Setting.Key.IsNumeric() && Setting.Value.IsNumeric();

        if (bKeyIsIndex)
        {
            int32 CategoryIndex = FCString::Atoi(*Setting.Key);
            int32 QualityIndex = FCString::Atoi(*Setting.Value);

            UE_LOG(LogTemp, Log, TEXT("Checking category %d, quality %d"), CategoryIndex, QualityIndex);

            // Retrieve settings for this category/quality via GameMode
            TArray<FSetting> CategorySettings = GameMode->GetAllSettingsAtQualityFromCategory(Categories, QualityIndex, CategoryIndex);

            for (const FSetting& CatSetting : CategorySettings)
            {
                float BaseFrameTime = BaseFrameTimes[Setting.Key];
                float AdjustedFrameTime = Setting.FrameStats.GPUFrameTime - BaseFrameTime;

                UE_LOG(LogTemp, Log, TEXT("Category FrameTime for %s at quality %d: %f ms, base: %f ms, adjusted FrameTime: %f ms"),
                    *CatSetting.Key, QualityIndex, Setting.FrameStats.GPUFrameTime, BaseFrameTime, AdjustedFrameTime);

                if (CurrentTotalFrameTime + AdjustedFrameTime <= TargetFrameTime && !AppliedKeys.Contains(CatSetting.Key))
                {
                    OptimalSettings.Add(CatSetting);
                    AppliedKeys.Add(CatSetting.Key);
                    CurrentTotalFrameTime += AdjustedFrameTime;
                    UE_LOG(LogTemp, Log, TEXT("Setting %s selected, total FrameTime now: %f ms"), *Setting.Key, CurrentTotalFrameTime);
                }
            }
        }
        else
        {
            // Standard case: Normal optimization logic for direct settings
            float BaseFrameTime = BaseFrameTimes[Setting.Key];
            float AdjustedFrameTime = Setting.FrameStats.GPUFrameTime - BaseFrameTime;

            UE_LOG(LogTemp, Log, TEXT("Direct setting %s: %f ms, base: %f ms, adjusted FrameTime: %f ms"),
                *Setting.Key, Setting.FrameStats.GPUFrameTime, BaseFrameTime, AdjustedFrameTime);

            if (CurrentTotalFrameTime + AdjustedFrameTime <= TargetFrameTime && !AppliedKeys.Contains(Setting.Key))
            {
                OptimalSettings.Add(Setting);
                AppliedKeys.Add(Setting.Key);
                CurrentTotalFrameTime += AdjustedFrameTime;
                UE_LOG(LogTemp, Log, TEXT("Setting %s selected, total FrameTime now: %f ms"), *Setting.Key, CurrentTotalFrameTime);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Optimized settings found, total FrameTime: %f ms"), CurrentTotalFrameTime);

    return OptimalSettings;
}




void ARecordHUD::BeginPlay()
{
    Super::BeginPlay();

}

void ARecordHUD::BeginDestroy()
{
    Super::BeginDestroy();

    ReleaseFrameGrabber();
}

void ARecordHUD::ReleaseFrameGrabber()
{
    if (FrameGrabber.IsValid())
    {
        //FrameGrabber->StopCapturingFrames();
        FrameGrabber->Shutdown();
        FrameGrabber.Reset();
    }
}

float ARecordHUD::ColorSSIM_Internal(TArray<FColor> x, TArray<FColor> y)
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

float ARecordHUD::SSIM_Internal(TArray<float> x, TArray<float> y)
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



FString ARecordHUD::GenerateSessionName() const
{
    const FDateTime Now = FDateTime::Now();
    return Now.ToString(TEXT("%Y%m%d_%H%M%S"));
}


