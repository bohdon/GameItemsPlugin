// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemContainerInterface.h"
#include "Components/ActorComponent.h"
#include "GameItemContainerComponent.generated.h"

class UGameItem;
class UGameItemAutoSlotRule;
class UGameItemContainerDef;


USTRUCT(BlueprintType)
struct FGameItemContainerSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ContainerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameItemContainerDef> ContainerDef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;
};


/**
 * Component that provides any number of game item containers.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMEITEMS_API UGameItemContainerComponent : public UActorComponent,
                                                  public IGameItemContainerInterface
{
	GENERATED_BODY()

public:
	UGameItemContainerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/** The definitions for all additional containers to create at startup. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (TitleProperty = "{ContainerId}"), Category = "GameItems")
	TArray<FGameItemContainerSpec> StartupContainers;

	/** Logic for determining how to auto-slot items, in order of priority. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "GameItems")
	TArray<TObjectPtr<UGameItemAutoSlotRule>> AutoSlotRules;

	/** Attempt to auto-slot an item into a container using the AutoSlotRules. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	TArray<UGameItem*> TryAutoSlotItem(UGameItem* Item, FGameplayTagContainer ContextTags) const;

	/**
	 * Create a new item container.
	 * @return The new container, or null if a container already exists with the same id.
	 */
	UFUNCTION(BlueprintCallable, Meta = (DeterminesOutputType = "ContainerClass"))
	UGameItemContainer* CreateContainer(FGameplayTag ContainerId, TSubclassOf<UGameItemContainerDef> ContainerDef = nullptr);

	virtual void PostLoad() override;
	virtual void InitializeComponent() override;
	virtual void ReadyForReplication() override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	// IGameItemContainerInterface
	virtual TArray<UGameItemContainer*> GetAllItemContainers() const override;
	virtual UGameItemContainer* GetItemContainer(FGameplayTag ContainerId) const override;

protected:
	UPROPERTY(Transient)
	TMap<FGameplayTag, UGameItemContainer*> Containers;

	/** Create all startup containers. */
	void CreateStartupContainers();

	void AddContainer(UGameItemContainer* Container);
};
