// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemContainerRule.h"
#include "GameplayTagContainer.h"
#include "GameItemContainerLink.generated.h"

class UGameItem;
class UGameItemContainer;
class UGameItemContainerLink;


/**
 * Links are container rules that create dependencies and automation between
 * related containers. Examples include preventing an item from being equipping
 * unless it exists in the main inventory, or auto-equipping items when picked-up.
 */
UCLASS(BlueprintType, Blueprintable, Abstract)
class GAMEITEMS_API UGameItemContainerLink : public UGameItemContainerRule
{
	GENERATED_BODY()

public:
	/** The id of the other container to link to. */
	UPROPERTY(Transient, BlueprintReadWrite)
	FGameplayTag LinkedContainerId;

	/** Return the linked container. */
	UGameItemContainer* GetLinkedContainer() const { return LinkedContainer; }

	/** Set the container that this one is linked to. */
	UFUNCTION(BlueprintCallable)
	void SetLinkedContainer(UGameItemContainer* NewContainer);

	/** Called when the linked container has changed. */
	virtual void OnLinkedContainerChanged(UGameItemContainer* NewContainer, UGameItemContainer* OldContainer);

protected:
	/** The other container that is linked. */
	UPROPERTY(Transient, BlueprintReadOnly, Meta = (AllowPrivateAccess = true))
	TObjectPtr<UGameItemContainer> LinkedContainer;
};


/**
 * Requires an item exist in the linked container before it can be added to this one.
 */
UCLASS(DisplayName = "Link Parent")
class UGameItemContainerLink_Parent : public UGameItemContainerLink
{
	GENERATED_BODY()

public:
	virtual void OnLinkedContainerChanged(UGameItemContainer* NewContainer, UGameItemContainer* OldContainer) override;
	virtual bool CanContainItem_Implementation(const UGameItem* Item) const override;

protected:
	void OnLinkedItemRemoved(UGameItem* Item);
};


/**
 * Attempts to automatically slot an item to this container when it is added to the linked container.
 */
UCLASS(DisplayName = "Link Auto-Slot")
class UGameItemContainerLink_AutoSlot : public UGameItemContainerLink
{
	GENERATED_BODY()

public:
	/** Context tags passed to the auto slot calls. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Slot")
	FGameplayTagContainer ContextTags;
};
