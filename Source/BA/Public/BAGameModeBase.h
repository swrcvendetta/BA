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

	// Expose the categories for Blueprint retrieval if needed
	UFUNCTION(BlueprintCallable, Category = "Settings")
	TArray<FSetting> GetAllSettingsAtQuality(const TArray<FCategory>& Categories, int32 Index) const
	{
		TArray<FSetting> Settings;

		for (const FCategory& Category : Categories)
		{
			if (Category.bEnabled && Category.Qualities.IsValidIndex(Index))
			{
				Settings.Append(Category.Qualities[Index].Settings);
			}
		}

		return Settings; // Returning by value is safe here
	}

	// Expose the categories for Blueprint retrieval if needed
	UFUNCTION(BlueprintCallable, Category = "Settings")
	TArray<FSetting> GetAllSettingsAtQualityFromCategory(const TArray<FCategory>& Categories, int32 QualityIndex, int32 CategoryIndex) const
	{
		TArray<FSetting> Settings;

		// Prüfen, ob CategoryIndex gültig ist
		if (!Categories.IsValidIndex(CategoryIndex))
		{
			UE_LOG(LogTemp, Error, TEXT("Ungültiger CategoryIndex: %d"), CategoryIndex);
			return Settings;
		}

		const FCategory& Category = Categories[CategoryIndex];

		// Prüfen, ob die Qualität in der Kategorie existiert und die Kategorie aktiv ist
		if (!Category.Qualities.IsValidIndex(QualityIndex) || !Category.bEnabled)
		{
			UE_LOG(LogTemp, Error, TEXT("Ungültiger QualityIndex: %d oder Kategorie deaktiviert"), QualityIndex);
			return Settings;
		}

		// Die Einstellungen der gewünschten Qualitätsstufe hinzufügen
		const FQuality& Quality = Category.Qualities[QualityIndex];
		Settings.Append(Quality.Settings);

		return Settings;
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
