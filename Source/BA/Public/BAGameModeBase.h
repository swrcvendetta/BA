#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BAGameModeBase.generated.h"

/**
 *
 */
UCLASS()
class BA_API ABAGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "System Info")
    float GetRamUsage() const;

    UFUNCTION(BlueprintCallable, Category = "System Info")
    float GetCpuUsage() const;

    UFUNCTION(BlueprintCallable, Category = "System Info")
    float GetGpuUsage() const;

protected:

    virtual void BeginPlay() override;
};
