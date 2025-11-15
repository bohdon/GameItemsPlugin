// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "GameItemContainerGraph.generated.h"

class UGameItemContainerDef;
class UGameItemContainerLink;


/**
 * Defines a container.
 */
USTRUCT(BlueprintType)
struct GAMEITEMS_API FGameItemContainerSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (GameplayTagFilter="GameItemContainerIdTagsCategory"))
	FGameplayTag ContainerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameItemContainerDef> ContainerDef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;
	
	FORCEINLINE bool IsValid() const;
};


/**
 * Defines a container link to add to all matching containers.
 */
USTRUCT(BlueprintType)
struct GAMEITEMS_API FGameItemContainerLinkSpec
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

	FORCEINLINE bool IsValid() const;
};


/**
 * Defines a set of containers and their relationships / links.
 */
UCLASS(BlueprintType, Blueprintable)
class GAMEITEMS_API UGameItemContainerGraph : public UDataAsset
{
	GENERATED_BODY()

public:
	UGameItemContainerGraph();

	/**
	 * The definitions of containers that should be created for this graph.
	 * Containers may be created by other means, and can still be referenced in this graph using tags.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (TitleProperty = "{ContainerId}"), Category = "GameItems")
	TArray<FGameItemContainerSpec> Containers;

	/** The links between any containers, establishing parent/child relationships or other rules. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (TitleProperty = "{LinkedContainerId} {ContainerLinkClass}"), Category = "GameItems")
	TArray<FGameItemContainerLinkSpec> Links;
};
