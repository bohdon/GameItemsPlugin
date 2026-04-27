// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemTypes.h"
#include "Components/ActorComponent.h"
#include "GameItemControllerComponent.generated.h"

class UGameItemContainer;


/**
 * Component used to handle manipulation of other (usually server owned and controlled) game item containers.
 * UI and other client-side systems can make requests through this to execute on the server (if they have permission).
 * Usually added to player controller, as other clients don't need access.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class GAMEITEMS_API UGameItemControllerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGameItemControllerComponent(const FObjectInitializer& ObjectInitializer);

	virtual FString GetDebugPrefix() const;

	/**
	 * Try handling the move of some items, potentially via RPCs.
	 * @return True if network calls are required and this component will handle them, False to perform the moves via containers as usual. 
	 */
	virtual bool HandleNetMove(const FGameItemMoveSpec& MoveSpec);

	/** Move items from a local-only client container to a server-owned container. */
	void MoveLocalOnlyItemsToServer(const FGameItemMoveSpec& MoveSpec);

	/** Move items from the server to a local-only client container. */
	void MoveServerItemsToLocalOnly(const FGameItemMoveSpec& MoveSpec);

	/** Receive items being sent from a client-only container and move them into ToContainer. */
	UFUNCTION(Server, Reliable)
	void ServerReceiveItems(const TArray<FGameItemSerializedMove>& Moves, const FGameItemContainerPair& Containers, FGameItemsPredictionKey PredictionKey);

	/** Send items being requested from a client-only container. */
	UFUNCTION(Server, Reliable)
	void ServerSendItems(const TArray<FGameItemMove>& Moves, const FGameItemContainerPair& Containers, FGameItemsPredictionKey PredictionKey);

	/** Called from server to accept or reject some predicted changes to this container or its items. */
	UFUNCTION(Client, Reliable)
	void ClientConfirmPredictionKey(const FGameItemsPredictionKey& PredictionKey, bool bAccepted = false);

protected:
	UPROPERTY(Transient)
	TMap<FGameItemsPredictionKey, FGameItemContainerPair> PendingContainers;
};
