// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItem.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_WaitItemCountsChanged.generated.h"

class UGameItemContainer;


/**
 * Waits for count changes on all items in a container, including new and remove items.
 */
UCLASS()
class GAMEITEMS_API UAsyncAction_WaitItemCountsChanged : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = true))
	static UAsyncAction_WaitItemCountsChanged* WaitForItemCountsChanged(UGameItemContainer* Container);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCountChangedDynDelegate, UGameItem*, Item, int32, NewCount, int32, OldCount);

	/** Called when one or more item slots has changed. */
	UPROPERTY(BlueprintAssignable)
	FOnCountChangedDynDelegate OnCountChanged;

	virtual void Activate() override;

protected:
	void OnItemAdded(UGameItem* Item);
	void OnItemRemoved(UGameItem* Item);
	void OnItemCountChanged(UGameItem* Item, int32 NewCount, int32 OldCount);

protected:
	TWeakObjectPtr<UGameItemContainer> Container;
};
