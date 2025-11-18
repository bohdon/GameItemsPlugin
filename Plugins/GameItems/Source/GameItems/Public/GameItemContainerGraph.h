// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemTypes.h"
#include "Engine/DataAsset.h"
#include "GameItemContainerGraph.generated.h"


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
