// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemCollectionInterface.h"
#include "GameItemContainerInterface.h"
#include "Components/ActorComponent.h"
#include "GameItemContainerComponent.generated.h"

class UGameItem;
class UGameItemContainerDef;
class UGameItemContainerLink;
class USaveGame;


/**
 * Defines a container.
 */
USTRUCT(BlueprintType)
struct FGameItemContainerSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (GameplayTagFilter="GameItemContainerIdTagsCategory"))
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

	/** The container link class to create for each matching container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContainerLink")
	TSubclassOf<UGameItemContainerLink> ContainerLinkClass;

	/** The container to link with. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContainerLink", meta = (GameplayTagFilter="GameItemContainerIdTagsCategory"))
	FGameplayTag LinkedContainerId;

	/** Apply this link to all containers matching this query. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContainerLink", meta = (GameplayTagFilter="GameItemContainerTagsCategory"))
	FGameplayTagQuery ContainerQuery;
};


/**
 * Component that provides a collection of game item containers.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMEITEMS_API UGameItemContainerComponent
	: public UActorComponent,
	  public IGameItemContainerInterface,
	  public IGameItemCollectionInterface
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

	/** Whether this container collection should be saved. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame")
	bool bEnableSaveGame;

	/** The id of this container collection for save games. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EditCondition = "bEnableSaveGame"), Category = "SaveGame")
	FName SaveCollectionId;

	/** Should this game item collection be saved to player save data? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Meta = (EditCondition = "bEnableSaveGame"), Category = "SaveGame")
	bool bIsPlayerCollection;

	/** Return true if an item is slotted in a container with any of the given tags. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (GameplayTagFilter = "GameItemContainerTagsCategory"))
	bool IsItemSlotted(UGameItem* Item, FGameplayTagContainer ContainerTags) const;

	/**
	 * Create a new item container.
	 * @return The new container, or null if a container already exists with the same id.
	 */
	UFUNCTION(BlueprintCallable, Meta = (GameplayTagFilter="GameItemContainerIdTagsCategory"))
	UGameItemContainer* CreateContainer(FGameplayTag ContainerId, TSubclassOf<UGameItemContainerDef> ContainerDef = nullptr);

	virtual void InitializeComponent() override;
	virtual void ReadyForReplication() override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	// IGameItemContainerInterface
	virtual TArray<UGameItemContainer*> GetAllItemContainers() const override;
	virtual UGameItemContainer* GetItemContainer(FGameplayTag ContainerId) const override;

	// IGameItemCollectionInterface
	virtual int32 GetTotalMatchingItemCount(const UGameItem* Item) const override;
	virtual int32 GetTotalMatchingItemCountByDef(TSubclassOf<UGameItemDef> ItemDef) const override;

	/** Write all containers and items to a save game. */
	UFUNCTION(BlueprintCallable)
	void CommitSaveGame(USaveGame* SaveGame);

	/** Load all containers and items from a save game. */
	UFUNCTION(BlueprintCallable)
	void LoadSaveGame(USaveGame* SaveGame);

protected:
	UPROPERTY(Transient)
	TMap<FGameplayTag, UGameItemContainer*> Containers;

	/** Create all startup containers. */
	void CreateStartupContainers();

	/** Update all container link rules to assign any containers that aren't set yet. */
	void ResolveContainerLinks();

	void AddContainer(UGameItemContainer* Container);
};
