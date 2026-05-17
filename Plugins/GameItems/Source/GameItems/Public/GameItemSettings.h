// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "Templates/SubclassOf.h"
#include "UObject/SoftObjectPtr.h"
#include "GameItemSettings.generated.h"

class UGameItemDef;
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

	/** The default expected prefix for item definition assets, used to clean names for debug. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
	FString ItemAssetPrefix = TEXT("ITM_");

	/** Log warnings when using the DefaultContainerId without setting it to valid tag. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
	bool bRequireValidDefaultContainerId;

	/** Game item cheat manager extension class to spawn. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<UGameItemCheatsExtension> ItemCheatsExtensionClass;

	/** Return a clean name for an item definition, stripping _C and ItemAssetPrefix, e.g. ITM_MyItem_C -> "MyItem" */
	FString GetItemDefShortName(const TSubclassOf<UGameItemDef>& ItemDef) const;

	virtual FName GetCategoryName() const override;

	static FGameplayTag GetDefaultContainerId();
};
