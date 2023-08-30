// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameItemStatics.generated.h"


class UGameItemSubsystem;
/**
 * Static functions for working with game items.
 */
UCLASS()
class GAMEITEMS_API UGameItemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static UGameItemSubsystem* GetItemSubsystemFromContextObject(const UObject* Object);

	/** Find and return an item fragment by class. */
	UFUNCTION(BlueprintCallable, Meta = (WorldContext = "WorldContextObject", DeterminesOutputType = "FragmentClass"), Category = "GameItems")
	static const UGameItemFragment* FindGameItemFragment(const UObject* WorldContextObject, TSubclassOf<UGameItemDef> ItemDef,
	                                                     TSubclassOf<UGameItemFragment> FragmentClass);
};
