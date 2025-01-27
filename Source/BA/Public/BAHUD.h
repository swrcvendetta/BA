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

    UFUNCTION(BlueprintCallable, Category = "Measuing")
    void SaveAllData();

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

protected:

	virtual void BeginPlay() override;

    virtual void BeginDestroy() override;

    virtual void ReleaseFrameGrabber();

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
};
