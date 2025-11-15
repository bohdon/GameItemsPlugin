// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemCollectionInterface.h"
#include "GameItemContainerGraph.h"
#include "GameItemContainerInterface.h"
#include "Components/ActorComponent.h"
#include "Rules/GameItemContainerRule.h"
#include "GameItemContainerComponent.generated.h"

class UGameItemContainer;
class UGameItemContainerDef;
class UGameItemDef;
class USaveGame;


/**
 * Represents an applied container link, with optional source object.
 */
USTRUCT()
struct FActiveGameItemContainerLink
{
	GENERATED_BODY()

	FActiveGameItemContainerLink()
	{
	}

	explicit FActiveGameItemContainerLink(const FGameItemContainerLinkSpec& InLinkSpec, const UObject* InSourceObject = nullptr)
		: LinkSpec(InLinkSpec)
		, SourceObject(InSourceObject)
	{
	}

	/** The container link class to create for each matching container. */
	UPROPERTY()
	FGameItemContainerLinkSpec LinkSpec;

	/** The optional source object that created this link. */
	UPROPERTY(NotReplicated)
	TObjectPtr<const UObject> SourceObject;
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

	/**
	 * The default set of containers and links to create.
	 * Additional graphs can be added at runtime to easily add more groups of related containers.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameItems")
	TArray<TObjectPtr<const UGameItemContainerGraph>> DefaultContainerGraphs;

private:
	/** DEPRECATED: Use DefaultContainerGraph instead. */
	UPROPERTY(EditAnywhere, Meta = (TitleProperty = "{ContainerId}", DeprecatedProperty), Category = "GameItems")
	TArray<FGameItemContainerSpec> StartupContainers;

	/** DEPRECATED: Use DefaultContainerGraph instead. */
	UPROPERTY(EditAnywhere, Meta = (TitleProperty = "{LinkedContainerId} {ContainerLinkClass}", DeprecatedProperty), Category = "GameItems")
	TArray<FGameItemContainerLinkSpec> ContainerLinks;

public:

	/** Whether this container collection should be saved. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame")
	bool bEnableSaveGame = false;

	/** The id of this container collection for save games. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EditCondition = "bEnableSaveGame"), Category = "SaveGame")
	FName SaveCollectionId;

	/** Should this game item collection be saved to player save data? */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Meta = (EditCondition = "bEnableSaveGame"), Category = "SaveGame")
	bool bIsPlayerCollection = false;

	/** Return true if an item is slotted in a container with any of the given tags. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "GameItems", meta = (GameplayTagFilter = "GameItemContainerTagsCategory"))
	bool IsItemSlotted(UGameItem* Item, FGameplayTagContainer ContainerTags) const;

	/**
	 * Add a new container graph, creating all the containers it defines, and setting up links
	 * between containers (both old and new).
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems")
	void AddContainerGraph(const UGameItemContainerGraph* Graph);

	/**
	 * Create a new item container.
	 * @return The new container, or null if a container already exists with the same id.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems")
	UGameItemContainer* CreateContainer(const FGameItemContainerSpec& ContainerSpec, bool bResolveLinks = true);

	/**
	 * Apply a new link between containers. Updates any pre-existing containers, and will apply to any containers created in the future.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems", Meta = (GameplayTagFilter="GameItemContainerIdTagsCategory"))
	void CreateContainerLink(const FGameItemContainerLinkSpec& LinkSpec, const UObject* SourceObject = nullptr, bool bResolveLinks = true);

	/**
	 * Update all container link rules to assign any containers that aren't set yet.
	 * Should be called after creating all related containers and links.
	 * @param bForce If true, re-resolve linked containers even if they are already set.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems")
	void ResolveAllContainerLinks(bool bForce = false);

	/** Create and add the default items for any newly created containers. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GameItems")
	void CreateDefaultItems(bool bForce = false);


	// IGameItemContainerInterface
	virtual TArray<UGameItemContainer*> GetAllItemContainers() const override;
	virtual UGameItemContainer* GetItemContainer(FGameplayTag ContainerId) const override;

	// IGameItemCollectionInterface
	virtual int32 GetTotalMatchingItemCount(const UGameItem* Item) const override;
	virtual int32 GetTotalMatchingItemCountByDef(TSubclassOf<UGameItemDef> ItemDef) const override;

	/** Write all containers and items to a save game. */
	UFUNCTION(BlueprintCallable, Category = "GameItems")
	void CommitSaveGame(USaveGame* SaveGame);

	/** Load all containers and items from a save game. */
	UFUNCTION(BlueprintCallable, Category = "GameItems")
	void LoadSaveGame(USaveGame* SaveGame);

	virtual void PostLoad() override;
	virtual void InitializeComponent() override;
	virtual void ReadyForReplication() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** The currently applied container graphs. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<const UGameItemContainerGraph>> Graphs;

	/** All containers that have been created on this component. */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Containers)
	TArray<TObjectPtr<UGameItemContainer>> Containers;

	UFUNCTION()
	void OnRep_Containers(const TArray<UGameItemContainer*>& PreviousContainers);

	/** Links that have been created to connect different containers via rules. */
	UPROPERTY(Transient, Replicated)
	TArray<FActiveGameItemContainerLink> Links;

	/** Non-replicated map of containers by id, for faster lookup. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<UGameItemContainer>> ContainerMap;

	/** Register a newly created container. */
	void AddContainer(UGameItemContainer* Container);

	/** Called when a new container is added. */
	virtual void OnContainerAdded(UGameItemContainer* Container);

	/** Called when a container is removed. */
	virtual void OnContainerRemoved(UGameItemContainer* Container);

	/** Add new link rules to a container. */
	void AddMatchingLinkRulesToContainer(UGameItemContainer* Container, const TArray<FActiveGameItemContainerLink>& InLinks);

	/** Add a new link rule to a container, if the query matches. */
	void AddLinkRuleToContainer(UGameItemContainer* Container, const FActiveGameItemContainerLink& Link);

	virtual void OnItemAdded(UGameItem* GameItem);

	virtual void OnItemRemoved(UGameItem* GameItem);

	virtual void OnRuleAdded(UGameItemContainerRule* Rule);

	virtual void OnRuleRemoved(UGameItemContainerRule* Rule);

	FString GetNetDebugString() const;
};
