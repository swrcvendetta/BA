#pragma once

#include "CoreMinimal.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "GameFramework/GameMode.h"
#include "BAGameModeBase.generated.h"


// Struct to hold key-value pairs
USTRUCT(BlueprintType)
struct FFrameStatData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Frame Stats")
	int32 FrameID;

	UPROPERTY(BlueprintReadWrite, Category = "Frame Stats")
	float DeltaTime;

	UPROPERTY(BlueprintReadWrite, Category = "Frame Stats")
	float FrameTime;

	UPROPERTY(BlueprintReadWrite, Category = "Frame Stats")
	float GameThreadTime;

	UPROPERTY(BlueprintReadWrite, Category = "Frame Stats")
	float RHITTime;

	UPROPERTY(BlueprintReadWrite, Category = "Frame Stats")
	float RenderThreadTime;

	UPROPERTY(BlueprintReadWrite, Category = "Frame Stats")
	float GPUFrameTime;
};

// Struct to hold key-value pairs
USTRUCT(BlueprintType)
struct FSetting
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	FString Key;

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	FString Value;

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	TArray<FColor> Frame;

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	FFrameStatData FrameStats;

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	float Quality;
};

// Quality Settings
USTRUCT(BlueprintType)
struct FQuality
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	TArray<FSetting> Settings;
};

// Struct to hold category data
USTRUCT(BlueprintType)
struct FCategory
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	FString Title;

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	bool bEnabled = true;

	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	TArray<FQuality> Qualities;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettingsLoaded, const TArray<FCategory>&, Categories);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSettingsFailed, const FString&, ErrorMessage);



/**
 *
 */
UCLASS()
class BA_API ABAGameModeBase : public AGameMode
{
    GENERATED_BODY()

protected:

    virtual void BeginPlay() override;


	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnSettingsLoaded OnSettingsLoaded;

	UPROPERTY(BlueprintAssignable, Category = "Settings")
	FOnSettingsFailed OnSettingsFailed;

private:
	TArray<FCategory> InternalCategories;

public:

	// Function to read the INI file and parse its contents
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void LoadScalabilitySettings(const FString& FilePath);

	// Expose the categories for Blueprint retrieval if needed
	UFUNCTION(BlueprintCallable, Category = "Settings")
	const TArray<FCategory>& GetCategories() const
	{
		return InternalCategories;
	}

	/**
	 * Enables/disables a category
	 * Returns true if there was a category with the provided index, else false
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	bool ToggleCategoryAt(int32 Index, bool Value)
	{
		if (Index >= InternalCategories.Num())
			return false;

		InternalCategories[Index].bEnabled = Value;
		return true;
	}
};
