#include "BAGameModeBase.h"

void ABAGameModeBase::BeginPlay()
{
    Super::BeginPlay();
}

void ABAGameModeBase::LoadScalabilitySettings(const FString& FilePath)
{
    FString FileContent;

    // Attempt to load the file
    if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
    {
        FString ErrorMessage = FString::Printf(TEXT("Failed to load file: %s"), *FilePath);
        UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorMessage);
        OnSettingsFailed.Broadcast(ErrorMessage);
        return;
    }

    TArray<FString> Lines;
    FileContent.ParseIntoArrayLines(Lines);

    InternalCategories.Empty();
    TMap<FString, FCategory> CategoryMap; // Maps category titles to FCategory objects

    FString CurrentCategoryTitle;
    int32 CurrentQualityLevel = -1;

    for (const FString& Line : Lines)
    {

        // Remove inline comments
        int32 CommentIndex;
        FString TrimmedLine = Line;
        if (Line.FindChar(';', CommentIndex))
        {
            TrimmedLine = Line.Left(CommentIndex);
        }

        TrimmedLine = TrimmedLine.TrimStartAndEnd();

        // Ignore comments
        if (TrimmedLine.StartsWith(";") || TrimmedLine.IsEmpty())
        {
            continue;
        }

        // Check for category titles like [Category@Quality]
        if (TrimmedLine.StartsWith("[") && TrimmedLine.EndsWith("]"))
        {
            FString FullTitle = TrimmedLine.Mid(1, TrimmedLine.Len() - 2); // Remove brackets
            FString QualityLevelStr;

            // Split into category and quality level if it matches "Category@Quality"
            if (FullTitle.Split(TEXT("@"), &CurrentCategoryTitle, &QualityLevelStr))
            {
                // Convert quality level to integer
                if (QualityLevelStr.Equals("Cine", ESearchCase::IgnoreCase)) // Cine is a special case
                {
                    CurrentQualityLevel = 4; // Treat "Cine" as level 4 (adjustable)
                }
                else
                {
                    CurrentQualityLevel = FCString::Atoi(*QualityLevelStr);
                }

                // Get or create the category
                FCategory& Category = CategoryMap.FindOrAdd(CurrentCategoryTitle);
                Category.Title = CurrentCategoryTitle;

                // Ensure the Qualities array is large enough
                while (Category.Qualities.Num() <= CurrentQualityLevel)
                {
                    Category.Qualities.Add(FQuality());
                }

                continue;
            }
        }

        // Parse key-value pairs
        FString Key, Value;
        if (TrimmedLine.Split(TEXT("="), &Key, &Value))
        {
            if (!CurrentCategoryTitle.IsEmpty() && CurrentQualityLevel >= 0)
            {
                // Add the setting to the current quality level
                FSetting Setting;
                Setting.Key = Key.TrimStartAndEnd();
                Setting.Value = Value.TrimStartAndEnd();

                FCategory& Category = CategoryMap[CurrentCategoryTitle];
                Category.Qualities[CurrentQualityLevel].Settings.Add(Setting);
            }
        }
    }

    // Move the map contents to the array
    for (auto& Pair : CategoryMap)
    {
        InternalCategories.Add(Pair.Value);
    }

    // Broadcast success
    OnSettingsLoaded.Broadcast(InternalCategories);
}
