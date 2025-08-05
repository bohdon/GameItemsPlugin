// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemContainerRule.h"
#include "GameplayTagContainer.h"
#include "GameItemContainerLink.generated.h"

class UGameItemContainer;


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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Link", meta = (GameplayTagFilter="GameItemContainerIdTagsCategory"))
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
