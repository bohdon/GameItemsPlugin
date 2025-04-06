// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "GameItemCheatsExtension.generated.h"

class UGameItemContainer;
class UGameItemDef;


UCLASS()
class UGameItemCheatsExtension : public UCheatManagerExtension
{
	GENERATED_BODY()

public:
	/** List all loaded game item definition classes. */
	UFUNCTION(Exec)
	void ItemList();

	/** Create and add a new item to an actor's default container. If no actor is given, gives the item to the player pawn. */
	UFUNCTION(Exec, meta = (Keywords = "Give"))
	void ItemAdd(FString ItemDefName, int32 Count = 1, AActor* Actor = nullptr);

	/** Remove an item from an actor. If no actor is given, removes the item from the player pawn. */
	UFUNCTION(Exec)
	void ItemRemove(FString ItemDefName, int32 Count = 1, AActor* Actor = nullptr);

protected:
	/** Resolve the actor to use when modifying items. */
	virtual AActor* ResolveActor(AActor* Actor);

	/** Find an item definition class by name, from the list of items in a container. */
	virtual TSubclassOf<UGameItemDef> FindItemDefInContainer(UGameItemContainer* Container, const FString& ItemDefName, bool bLogWarning = true) const;

	/** Find an item definition class by name. Returns the first matching result. */
	virtual TSubclassOf<UGameItemDef> FindItemDef(const FString& ItemDefName, bool bLogWarning = true) const;

	/** Find an unloaded blueprint item definition by name. */
	virtual TSubclassOf<UGameItemDef> FindBlueprintItemDef(const FString& ItemDefName) const;

	virtual void GetAllLoadedItemDefs(TArray<TSubclassOf<UGameItemDef>>& OutItemDefs) const;
};
