// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MVVMViewModelBase.h"
#include "VM_GameItemTagStat.generated.h"

class UGameItem;


/**
 * View model for a single tag stat of a game item.
 */
UCLASS()
class GAMEITEMSUI_API UVM_GameItemTagStat : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetItemAndTag(UGameItem* NewItem, FGameplayTag NewTag);

	/** Return the current item. */
	UGameItem* GetItem() const { return Item; }

	void SetTag(const FGameplayTag& NewTag);

	UFUNCTION(BlueprintPure, FieldNotify)
	int32 GetValue() const;

protected:
	void OnTagStatChanged(const FGameplayTag& ChangedTag, int32 NewValue, int32 OldValue);

protected:
	/** The game item. */
	UPROPERTY(BlueprintReadWrite, Getter, FieldNotify, Meta = (AllowPrivateAccess))
	TObjectPtr<UGameItem> Item;

	UPROPERTY(BlueprintReadWrite, Setter, FieldNotify);
	FGameplayTag Tag;
};
