// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemContainerInterface.h"
#include "Components/ActorComponent.h"
#include "GameItemContainerComponent.generated.h"

class UGameItem;
class UGameItemAutoSlotRule;
class UGameItemContainerDef;
class UGameItemContainerLink;


/**
 * Defines a container.
 */
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
 * Defines a container link to add to all matching containers.
 */
USTRUCT(BlueprintType)
struct FGameItemContainerLinkSpec
{
	GENERATED_BODY()

	/** Apply this link to all containers matching this query. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContainerLink")
	FGameplayTagQuery ContainerQuery;

	/** The linked container id. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContainerLink")
	FGameplayTag LinkedContainerId;

	/** The container link class to create for each matching container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContainerLink")
	TSubclassOf<UGameItemContainerLink> ContainerLinkClass;
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

	/** The container links to add on any containers created by this component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (TitleProperty = "{LinkedContainerId} {ContainerLinkClass}"), Category = "GameItems")
	TArray<FGameItemContainerLinkSpec> ContainerLinks;

	/** Logic for determining how to auto-slot items, in order of priority. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "GameItems")
	TArray<TObjectPtr<UGameItemAutoSlotRule>> AutoSlotRules;

	/** Attempt to auto-slot an item into a container using the AutoSlotRules. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	TArray<UGameItem*> TryAutoSlotItem(UGameItem* Item, FGameplayTagContainer ContextTags) const;

	/** Return true if an item is slotted in a container with any of the given tags. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	bool IsItemSlotted(UGameItem* Item, FGameplayTagContainer ContainerTags) const;

	/**
	 * Create a new item container.
	 * @return The new container, or null if a container already exists with the same id.
	 */
	UFUNCTION(BlueprintCallable, Meta = (DeterminesOutputType = "ContainerClass"))
	UGameItemContainer* CreateContainer(FGameplayTag ContainerId, TSubclassOf<UGameItemContainerDef> ContainerDef = nullptr);

	/** Return all containers that are a child of a container. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	TArray<UGameItemContainer*> GetChildContainers(UGameItemContainer* ParentContainer) const;

	/**
	 * Find and return a child container best suited to receive an item.
	 * This is useful for example to find an equip slot.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	UGameItemContainer* GetBestChildContainerForItem(UGameItemContainer* ParentContainer, UGameItem* Item, FGameplayTagContainer ContextTags) const;

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

	/** Update all container link rules to assign any containers that aren't set yet. */
	void ResolveContainerLinks();

	void AddContainer(UGameItemContainer* Container);
};
