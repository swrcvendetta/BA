// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Engine/GameEngine.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "FrameGrabber.h"
#include "GameFramework/Actor.h"
#include "ImageCoreClasses.h"
#include <BAGameModeBase.h>
#include "BAHUD.generated.h"

class FFrameGrabber;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFrameSaved, int32, Current, int32, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRecordingStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRecordingStopped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRecordingMasterFrameStopped);

/**
 * 
 */
UCLASS()
class BA_API ABAHUD : public AHUD
{
	GENERATED_BODY()
	
	void DrawHUD() override;

public:

    // The multicast event
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnFrameSaved OnFrameSaved;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnRecordingStarted OnRecordingStarted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnRecordingStopped OnRecordingStopped;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnRecordingMasterFrameStopped OnRecordingMasterFrameStopped;

	UFUNCTION(BlueprintCallable, Category = "Measuring")
	bool RecordStats();

    UFUNCTION(BlueprintCallable, Category = "Measuring")
    void StartRecording();

    UFUNCTION(BlueprintCallable, Category = "Measuring")
    void StopRecording();

    UFUNCTION(BlueprintCallable, Category = "Measuring")
    void SetTargetFrameRate(float FrameRate);

    UFUNCTION(BlueprintCallable, Category = "Measuring")
    void SetFrameRate(float FrameRate);

    UFUNCTION(BlueprintCallable, Category = "Measuring")
    void SetMaxFrameAmount(int32 FrameAmount);

    UFUNCTION(BlueprintCallable, Category = "Measuring")
    void SaveAllData();

    UFUNCTION(BlueprintCallable, Category = "Measuring")
    TArray<FSetting> GetOptimalSettings();

    // apply settings in BP I'd say

    /*
    UFUNCTION(BlueprintCallable, Category = "Measuring")
    TArray<FColor>& GetFrameAt(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Measuring")
    FFrameStatData GetStatDataAt(int32 Index);
    */

    UFUNCTION(BlueprintCallable, Category = "Measuring")
    void RecordSetting(FString Setting, FString Value);

    UFUNCTION(BlueprintCallable, Category = "Measuring")
    void RecordMasterFrame();

    UFUNCTION(BlueprintCallable, Category = "Measuring")
    float ColorSSIM(TArray<FColor> x, TArray<FColor> y);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Measuring")
    float SSIM_R_WEIGHT = 0.3333333333333333f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Measuring")
    float SSIM_G_WEIGHT = 0.3333333333333333f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Measuring")
    float SSIM_B_WEIGHT = 0.3333333333333333f;

protected:

	virtual void BeginPlay() override;

    virtual void BeginDestroy() override;

    virtual void ReleaseFrameGrabber();

    virtual float ColorSSIM_Internal(TArray<FColor> x, TArray<FColor> y);

    virtual float SSIM_Internal(TArray<float> x, TArray<float> y);

private:

	bool bIsRecording = false;

	UWorld* _World;
	
    // Store recorded stats
    struct FFrameStatData_internal
    {
        int32 FrameID;
        float DeltaTime;
        float FrameTime;
        float GameThreadTime;
        float RHITTime;
        float RenderThreadTime;
        float GPUFrameTime;
    };

    TArray<FFrameStatData_internal> RecordedStats;
    int32 CurrentFrameID = 0;

    FString CurrentSessionName;
    FString prefix;

    void SaveStatsToFile();
    void SaveRenderSettingsToFile();

    FString GenerateSessionName() const;

    float TargetFrameRate = 30.0f;

    int32 MaxFrameAmount = 300;

    TSharedPtr<FFrameGrabber> FrameGrabber;
    TArray<FCapturedFrameData> CapturedFrames;

    void SchreibDenScheis();

    bool bIsSavingFrames = false;
    int32 SavedFrameIndex = 0;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Measuring")
    float MaxSaveFrameDelay = 0.5f;

private:

    float currentDelay = 0.0f;

    TArray<FSetting> Settings;
    FSetting currentSetting;

    bool bCaptureSetting = false;
    bool bCaptureMasterFrame = false;

    bool bSaveAll = false;
    FString _currentSessionName;
    int32 _bufferSizeX;
    int32 _bufferSizeY;

    TArray<FColor> MasterFrame;
    FIntPoint MasterFrameBuffer;

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Measuring", meta = (ClampMin="0", ClampMax="1", UIMin = "0", UIMax="1"))
    float k1 = 0.01f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Measuring", meta = (ClampMin = "0", ClampMax = "1", UIMin = "0", UIMax = "1"))
    float k2 = 0.03f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Measuring", meta = (ClampMin = "0", ClampMax = "255", UIMin = "0", UIMax = "255"))
    float L = 255.0f;

private:

    // SSIM

    float _meanX = -1.0f;
    float _meanY = -1.0f;
    float _varianceX = -1.0f;
    float _varianceY = -1.0f;
    float _covarianceXY = -1.0f;
    float _c1 = (k1 * L) * (k1 * L);
    float _c2 = (k2 * L) * (k2 * L);
};
