// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemTypes.h"
#include "Engine/EngineTypes.h"
#include "UObject/Object.h"
#include "GameItemContainerRule.generated.h"

class UGameItem;
class UGameItemContainer;


/**
 * Defines conditions and stock limitations for a game item container.
 * Rules can be stateful and are instantiated and replicated within the container they apply to.
 */
UCLASS(BlueprintType, Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class GAMEITEMS_API UGameItemContainerRule : public UObject
{
	GENERATED_BODY()

public:
	UGameItemContainerRule();

	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack) override;

	/** Return the net role of the owning actor. */
	ENetRole GetNetRole() const;

	/** Return the owning container. */
	UFUNCTION(BlueprintPure, Category = "GameItems")
	FORCEINLINE UGameItemContainer* GetContainer() const;

	/** Return true if this rule makes the container a child of another container, such that it cannot store its own items. */
	UFUNCTION(BlueprintNativeEvent)
	bool IsChild() const;

	/** Return true if an item is allowed in the container. */
	UFUNCTION(BlueprintNativeEvent)
	bool CanContainItem(const UGameItem* Item) const;

	/** Return true if an item is allowed in the container by definition. */
	UFUNCTION(BlueprintNativeEvent)
	bool CanContainItemByDef(TSubclassOf<UGameItemDef> ItemDef) const;

	/** Return the maximum allowed count for an item, or -1 if unlimited. */
	UFUNCTION(BlueprintNativeEvent)
	int32 GetItemMaxCount(const UGameItem* Item) const;

	/** Return the maximum allowed count for a single stack of an item, or -1 if unlimited. */
	UFUNCTION(BlueprintNativeEvent)
	int32 GetItemStackMaxCount(const UGameItem* Item) const;

	virtual UWorld* GetWorld() const override;

	virtual FString GetDebugString() const;
};
