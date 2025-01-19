// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Engine/GameEngine.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "FrameGrabber.h"
#include "GameFramework/Actor.h"
#include "BAHUD.generated.h"

class FFrameGrabber;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFrameSaved, int32, Current, int32, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRecordingStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRecordingStopped);

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

	UFUNCTION(BlueprintCallable, Category = "Measuring")
	bool RecordStats();

    UFUNCTION(BlueprintCallable, Category = "Measuring")
    void StartRecording();

    UFUNCTION(BlueprintCallable, Category = "Measuring")
    void StopRecording();

    UFUNCTION(BlueprintCallable, Category = "Measuring")
    void SetFrameRate(float FrameRate);

protected:

	virtual void BeginPlay() override;

    virtual void BeginDestroy() override;

    virtual void ReleaseFrameGrabber();

private:

	bool bIsRecording = false;

	UWorld* _World;
	
    // Store recorded stats
    struct FFrameStatData
    {
        int32 FrameID;
        float DeltaTime;
        float FrameTime;
        float GameThreadTime;
        float RenderThreadTime;
        float GPUFrameTime;
    };

    TArray<FFrameStatData> RecordedStats;
    int32 CurrentFrameID = 0;

    FString CurrentSessionName;

    void SaveStatsToFile();
    void SaveRenderSettingsToFile();

    FString GenerateSessionName() const;

    int32 MaxFrameAmount = 300;
    int32 ResolutionWidth = 1920;
    int32 ResolutionHeight = 1080;

    UPROPERTY()
    UTextureRenderTarget2D* RenderTarget; // Render target for capturing frames

    UPROPERTY()
    USceneCaptureComponent2D* SceneCaptureComponent;

    void SetupRenderTarget();

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
};
