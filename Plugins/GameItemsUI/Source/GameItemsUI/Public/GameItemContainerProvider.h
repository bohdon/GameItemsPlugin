// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "GameItemContainerProvider.generated.h"

class UGameItemContainer;
class UMVVMView;
class UUserWidget;


/** Context passed to a UGameItemViewContainerProvider. */
USTRUCT(BlueprintType)
struct FGameItemViewContext
{
	GENERATED_BODY()

	/** The user widget requesting the container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<const UUserWidget> UserWidget;

	/** Additional context tags. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer ContextTags;
};


/**
 * Provides a game item container, for use in view model resolvers or similar situations.
 */
UCLASS(BlueprintType, Blueprintable, Abstract, Const)
class GAMEITEMSUI_API UGameItemContainerProvider : public UObject
{
	GENERATED_BODY()

public:
	/** Return the relevant item container. */
	UFUNCTION(BlueprintNativeEvent)
	UGameItemContainer* ProvideContainer(const FGameplayTag& ContainerId, const FGameItemViewContext& Context) const;
};


/**
 * Provides a game item container from the widget's owning player.
 * Checks the player pawn, player state, and player controller in that order.
 */
UCLASS(DisplayName = "Player")
class UGameItemContainerProvider_Player : public UGameItemContainerProvider
{
	GENERATED_BODY()

public:
	virtual UGameItemContainer* ProvideContainer_Implementation(const FGameplayTag& ContainerId, const FGameItemViewContext& Context) const override;
};
