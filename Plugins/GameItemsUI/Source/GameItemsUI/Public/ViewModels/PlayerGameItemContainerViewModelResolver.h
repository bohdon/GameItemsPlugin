// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "View/MVVMViewModelContextResolver.h"
#include "PlayerGameItemContainerViewModelResolver.generated.h"


/**
 * Resolves a game item container view model from the owning player.
 */
UCLASS()
class GAMEITEMSUI_API UPlayerGameItemContainerViewModelResolver : public UMVVMViewModelContextResolver
{
	GENERATED_BODY()

public:
	UPlayerGameItemContainerViewModelResolver();

	/** The container id to retrieve from the player. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewModelResolver")
	FGameplayTag ContainerId;

	virtual UObject* CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const override;
};
