// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "GameItemSettings.generated.h"


/**
 * Defines gameplay settings for the Game Items plugin.
 */
UCLASS(Config = Game, DefaultConfig)
class GAMEITEMS_API UGameItemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** The id to use for default item containers. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
	FGameplayTag DefaultContainerId;

	/** Log warnings when using the DefaultContainerId without setting it to valid tag. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
	bool bRequireValidDefaultContainerId = true;

	virtual FName GetCategoryName() const override;

	static FGameplayTag GetDefaultContainerId();
};
