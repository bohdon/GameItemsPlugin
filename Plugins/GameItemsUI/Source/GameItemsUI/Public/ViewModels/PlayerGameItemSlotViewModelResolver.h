// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "View/MVVMViewModelContextResolver.h"
#include "PlayerGameItemSlotViewModelResolver.generated.h"


/**
 * Resolves a view model for a specific slot in a game item container from the owning player.
 */
UCLASS()
class GAMEITEMSUI_API UPlayerGameItemSlotViewModelResolver : public UMVVMViewModelContextResolver
{
	GENERATED_BODY()

public:
	UPlayerGameItemSlotViewModelResolver();

	/** The container id to retrieve from the player. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewModelResolver")
	FGameplayTag ContainerId;

	/** The slot in the container to retrieve. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewModelResolver")
	int32 Slot;

	virtual UObject* CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const override;

	/** Return the actor that has the container to retrieve. */
	virtual AActor* GetContainerActor(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const;
};
