// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemTypes.h"
#include "Components/ActorComponent.h"
#include "GameItemControllerComponent.generated.h"

class UGameItemContainer;


/**
 * Component used to provide full network support, usually added to the player controller.
 * 
 * Allows clients to control server-owned containers via requests made using RPCs,
 * or automatically fallback to UGameItemSubsystem for locally-controlled containers.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class GAMEITEMS_API UGameItemControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGameItemControllerComponent(const FObjectInitializer& ObjectInitializer);

	virtual FString GetDebugPrefix() const;

	/** Move an item from one slot to another, swapping or stacking as needed. */
	UFUNCTION(BlueprintCallable)
	virtual void MoveSwapOrStackItem(UGameItemContainer* From, UGameItem* Item, UGameItemContainer* To, int32 ToSlot, bool bAllowPartial = true);

	/** Move an item from one container to another. */
	UFUNCTION(BlueprintCallable)
	virtual void MoveItem(UGameItemContainer* From, UGameItemContainer* To, UGameItem* Item, bool bAllowPartial = true);

	/** Move a set of items from one container to another. */
	UFUNCTION(BlueprintCallable)
	virtual void MoveItems(UGameItemContainer* From, UGameItemContainer* To, TArray<UGameItem*> Items, bool bAllowPartial = true);

	/** Move all items from one container to another. */
	UFUNCTION(BlueprintCallable)
	virtual void MoveAllItems(UGameItemContainer* From, UGameItemContainer* To, bool bAllowPartial = true);

	/**
	 * Try handling the move of some items, potentially via RPCs.
	 * @return True if network calls are required and this component will handle them, False to perform the moves via containers as usual. 
	 */
	virtual bool HandleNetMove(const FGameItemMoveSpec& MoveSpec);

public:
	/** Move items from a local-only client container to a server-owned container. */
	void MoveClientItemsToServer(const FGameItemMoveSpec& MoveSpec);

	/** Move items from the server to a local-only client container. */
	void MoveServerItemsToClient(const FGameItemMoveSpec& MoveSpec);

	/** Move items around between server-owned containers. */
	void MoveServerItems(const FGameItemMoveSpec& MoveSpec);

public:
	/** Receive items being sent from a client-only container and move them into ToContainer. */
	UFUNCTION(Server, Reliable)
	void ServerReceiveItems(const TArray<FGameItemSerializedMove>& Moves, const FGameItemContainerPair& Containers, FGameItemsPredictionKey PredictionKey);

	/** Send items being requested from a client-only container. */
	UFUNCTION(Server, Reliable)
	void ServerSendItems(const TArray<FGameItemMove>& Moves, const FGameItemContainerPair& Containers, FGameItemsPredictionKey PredictionKey);

	/** Move items around between server-owned containers. */
	UFUNCTION(Server, Reliable)
	void ServerMoveItems(const TArray<FGameItemMove>& Moves, const FGameItemContainerPair& Containers, FGameItemsPredictionKey PredictionKey);

	/** Called from server to accept or reject some predicted changes to this container or its items. */
	UFUNCTION(Client, Reliable)
	void ClientConfirmPredictionKey(const FGameItemsPredictionKey& PredictionKey, bool bAccepted = false);

protected:
	/** Map of containers involved in any actions for each prediction key. */
	UPROPERTY(Transient)
	TMap<FGameItemsPredictionKey, FGameItemContainerPair> PredictionContainerMap;
};
