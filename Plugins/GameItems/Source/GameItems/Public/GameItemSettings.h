// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "Templates/SubclassOf.h"
#include "GameItemSettings.generated.h"

class UGameItemCheatsExtension;


/**
 * Defines gameplay settings for the Game Items plugin.
 */
UCLASS(Config = Game, DefaultConfig)
class GAMEITEMS_API UGameItemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGameItemSettings();

	/** The id to use for default item containers. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, meta = (GameplayTagFilter="GameItemContainerIdTagsCategory"))
	FGameplayTag DefaultContainerId;

	/** Log warnings when using the DefaultContainerId without setting it to valid tag. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
	bool bRequireValidDefaultContainerId;

	/** Game item cheat manager extension class to spawn. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UGameItemCheatsExtension> ItemCheatsExtensionClass;

	virtual FName GetCategoryName() const override;

	static FGameplayTag GetDefaultContainerId();
};
