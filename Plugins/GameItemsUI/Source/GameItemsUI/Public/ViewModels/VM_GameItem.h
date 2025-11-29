// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MVVMViewModelBase.h"
#include "VM_GameItem.generated.h"

class UGameItem;
class UVM_GameItemTagStat;


/**
 * A view-model for a UGameItem.
 */
UCLASS()
class GAMEITEMSUI_API UVM_GameItem : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	UVM_GameItem();

	/** Set the game item for this view model .*/
	void SetItem(UGameItem* NewItem);

	/** Return the current item. */
	UGameItem* GetItem() const { return Item; }

	/** Return the quantity of the item. */
	UFUNCTION(BlueprintPure, FieldNotify)
	int32 GetCount() const;

	/** Return view models for all tag stats of this item. */
	UFUNCTION(BlueprintPure, FieldNotify)
	TArray<UVM_GameItemTagStat*> GetAllTagStatViewModels() const;

	/** Return a view model for a tag stat of this item. */
	UFUNCTION(BlueprintPure)
	UVM_GameItemTagStat* GetTagStatViewModel(FGameplayTag Tag) const;

	/** Return true if count is above 1. */
	UFUNCTION(BlueprintPure, FieldNotify)
	bool HasMultiple() const;

	/** Return the display name of the item. */
	UFUNCTION(BlueprintPure, FieldNotify)
	FText GetDisplayName() const;

protected:
	/** The game item. */
	UPROPERTY(BlueprintReadWrite, Setter, Getter, FieldNotify, Meta = (AllowPrivateAccess))
	TObjectPtr<UGameItem> Item;

	/** The cached view models for each tag stat. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UVM_GameItemTagStat>> TagStatViewModels;

	void OnCountChanged(int32 NewCount, int32 OldCount);
};
