// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "View/MVVMViewModelContextResolver.h"
#include "GameItemViewModelResolver.generated.h"

class UVM_GameItemContainer;
class UGameItemContainer;
class UGameItemContainerProvider;


/**
 * Base class for a view model resolver related to game items.
 */
UCLASS(Abstract)
class GAMEITEMSUI_API UGameItemViewModelResolverBase : public UMVVMViewModelContextResolver
{
	GENERATED_BODY()

public:
	/** Get or create a view model for the relevant container. */
	virtual UVM_GameItemContainer* GetItemContainerViewModel(const UUserWidget* UserWidget, const UMVVMView* View) const;

	/** Return the relevant game item container. */
	virtual UGameItemContainer* GetItemContainer(const UUserWidget* UserWidget, const UMVVMView* View) const;
};


/**
 * Resolves a game item container view model using a provider.
 * Implement custom provider classes to retrieve containers from different actors in the world.
 */
UCLASS()
class GAMEITEMSUI_API UVMR_GameItemContainer : public UGameItemViewModelResolverBase
{
	GENERATED_BODY()

public:
	UVMR_GameItemContainer();

	/** The item container provider to use. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewModelResolver")
	TSubclassOf<UGameItemContainerProvider> Provider;

	/** The container id. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewModelResolver")
	FGameplayTag ContainerId;

	virtual UObject* CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const override;
	virtual UGameItemContainer* GetItemContainer(const UUserWidget* UserWidget, const UMVVMView* View) const override;
};


/**
 * Resolves a view model for a specific slot in a game item container using a provider.
 * Implement custom provider classes to retrieve containers from different actors in the world.
 */
UCLASS()
class GAMEITEMSUI_API UVMR_GameItemSlot : public UGameItemViewModelResolverBase
{
	GENERATED_BODY()

public:
	UVMR_GameItemSlot();

	/** The item container provider to use. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewModelResolver")
	TSubclassOf<UGameItemContainerProvider> Provider;

	/** The container id. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewModelResolver")
	FGameplayTag ContainerId;

	/** The slot in the container to retrieve. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ViewModelResolver")
	int32 Slot;

	virtual UObject* CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const override;
	virtual UGameItemContainer* GetItemContainer(const UUserWidget* UserWidget, const UMVVMView* View) const override;
};
