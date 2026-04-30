// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemControllerComponent.h"

#include "GameItemContainer.h"
#include "GameItemsModule.h"
#include "GameItemStatics.h"
#include "GameItemSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"


UGameItemControllerComponent::UGameItemControllerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetIsReplicatedByDefault(true);
}

FString UGameItemControllerComponent::GetDebugPrefix() const
{
	return FString::Printf(TEXT("%s[%s]"), *UGameItemStatics::GetNetDebugPrefix(this), *GetReadableName());
}

void UGameItemControllerComponent::MoveSwapOrStackItem(UGameItemContainer* From, UGameItem* Item, UGameItemContainer* To, int32 ToSlot, bool bAllowPartial)
{
	// early out if moving to same slot
	if (From && To && From->GetItemSlot(Item) == ToSlot)
	{
		return;
	}

	// check if the move requires network handling
	if (GetWorld()->GetNetMode() == NM_Client)
	{
		const FGameItemMoveSpec MoveSpec(From, To, {{Item, ToSlot}});
		if (HandleNetMove(MoveSpec))
		{
			// will be handled via net RPCs
			return;
		}
	}

	UGameItemSubsystem* ItemsSubsystem = UGameItemSubsystem::Get(this);
	ItemsSubsystem->MoveSwapOrStackItem(From, Item, To, ToSlot, bAllowPartial);
}

void UGameItemControllerComponent::MoveItem(UGameItemContainer* From, UGameItemContainer* To, UGameItem* Item, bool bAllowPartial)
{
	MoveItems(From, To, {Item}, bAllowPartial);
}

void UGameItemControllerComponent::MoveItems(UGameItemContainer* From, UGameItemContainer* To, TArray<UGameItem*> Items, bool bAllowPartial)
{
	if (From == To)
	{
		return;
	}

	// check if the move requires network handling
	if (GetWorld()->GetNetMode() == NM_Client)
	{
		TArray<FGameItemMove> Moves;
		Moves.Reserve(Items.Num());
		for (UGameItem* Item : Items)
		{
			Moves.Emplace(Item, -1);
		}

		// check for network move
		const FGameItemMoveSpec MoveSpec(From, To, Moves);
		if (HandleNetMove(MoveSpec))
		{
			// will be handled via net RPCs
			return;
		}
	}

	UGameItemSubsystem* ItemsSubsystem = UGameItemSubsystem::Get(this);
	ItemsSubsystem->MoveItems(From, To, Items, bAllowPartial);
}

void UGameItemControllerComponent::MoveAllItems(UGameItemContainer* From, UGameItemContainer* To, bool bAllowPartial)
{
	if (!From || !To)
	{
		return;
	}

	// TODO: optimize
	const TMap<int32, UGameItem*> ItemsBySlot = From->GetAllItems();
	TArray<UGameItem*> Items;
	ItemsBySlot.GenerateValueArray(Items);
	MoveItems(From, To, Items, bAllowPartial);
}

bool UGameItemControllerComponent::HandleNetMove(const FGameItemMoveSpec& MoveSpec)
{
	if (!ensure(MoveSpec.IsValid()))
	{
		return false;
	}

	if (GetWorld()->GetNetMode() == NM_Standalone)
	{
		return false;
	}

	UGameItemContainer* From = MoveSpec.Containers.From;
	UGameItemContainer* To = MoveSpec.Containers.To;

	const bool bFromItemsOnServer = From->ItemsExistOnServer();
	const bool bToItemsOnServer = To->ItemsExistOnServer();

	if (GetWorld()->GetNetMode() == NM_Client)
	{
		const bool bFromOwned = From->IsLocallyControlled();
		const bool bToOwned = To->IsLocallyControlled();

		// handle client sending items to server
		if (!bFromItemsOnServer && bToItemsOnServer)
		{
			// no way this client has visibility of the items if not the owner
			check(bFromOwned);

			UE_LOG(LogGameItems, Verbose, TEXT("%s[%hs] Client sending unreplicated items to server: %s -> %s"),
				*UGameItemStatics::GetNetDebugPrefix(From), __func__, *From->GetReadableName(), *To->GetReadableName());

			MoveClientItemsToServer(MoveSpec);
			return true;
		}

		// handle client taking items from server
		if (bFromItemsOnServer && !bToItemsOnServer)
		{
			// must be a replicated item the client can see
			check(From->IsReplicated());
			check(bToOwned);

			UE_LOG(LogGameItems, Verbose, TEXT("%s[%hs] Client taking replicated server items: %s -> %s"),
				*UGameItemStatics::GetNetDebugPrefix(From), __func__, *From->GetReadableName(), *To->GetReadableName());

			MoveServerItemsToClient(MoveSpec);
			return true;
		}

		// handle client moving items around on server-owned containers
		if (bFromItemsOnServer && bToItemsOnServer && (!bFromOwned || !bToOwned))
		{
			MoveServerItems(MoveSpec);
			return true;
		}
	}
	else
	{
		// we're on server

		// handle sending items from server to client
		if (bFromItemsOnServer && !bToItemsOnServer)
		{
			if (From->IsReplicated())
			{
				// server sending replicated item to client
				UE_LOG(LogGameItems, Verbose, TEXT("%s[%hs] Server sending replicated item to client: %s -> %s"),
					*UGameItemStatics::GetNetDebugPrefix(From), __func__, *From->GetReadableName(), *To->GetReadableName());

				// tell client to take the known item by reference
				ensureMsgf(false, TEXT("Sending server initiated to client-only is unsupported"));
				return true;
			}
			else
			{
				// server sending non-replicated item to client 
				UE_LOG(LogGameItems, Verbose, TEXT("%s[%hs] Server sending unreplicated item to client: %s -> %s"),
					*UGameItemStatics::GetNetDebugPrefix(From), __func__, *From->GetReadableName(), *To->GetReadableName());

				// serialize to save data and recreate on client
				ensureMsgf(false, TEXT("Sending server initiated to client-only is unsupported"));
				return true;
			}
		}
	}

	return false;
}

void UGameItemControllerComponent::MoveClientItemsToServer(const FGameItemMoveSpec& MoveSpec)
{
	// - generate a prediction key, and 'remove' the items locally (don't fully remove them or free up slots, to make rollback easy)
	// - serialize to save data and send items to server, where it recreates the items in ToContainer
	// - server acks with the prediction...
	//   - if accepted, commits the "real" removal of items on this client
	//   - if rejected, unmarks th pending removal

	if (!MoveSpec.Containers.IsValid() || MoveSpec.Moves.IsEmpty())
	{
		return;
	}

	UGameItemContainer* From = MoveSpec.Containers.From;
	UGameItemContainer* To = MoveSpec.Containers.From;

	const FGameItemsPredictionKey PredictionKey = FGameItemsPredictionKey::CreateNewClientPredictionKey(GetOwner());

	TArray<FGameItemSerializedMove> ServerMoves;
	for (const FGameItemMove& Move : MoveSpec.Moves)
	{
		UGameItem* Item = Move.Item;
		if (!ensure(Item))
		{
			continue;
		}

		if (Item->HasPendingNetChange())
		{
			UE_LOG(LogGameItems, Warning, TEXT("Item already pending net changes: %s"),
				*Item->GetDebugString());

			// item already waiting on some predicted action, leave it alone
			continue;
		}

		// mark item as pending-remove from this container
		UE_LOG(LogGameItems, VeryVerbose, TEXT("%s [%hs] Marking for remove: %s"),
			*From->GetDebugPrefix(), __func__, *Item->GetDebugString());

		Item->MarkPendingRemove(From, PredictionKey);

		// TODO: show as predicted add in target container for UI
		// ToContainer->AddPendingItem(Item, TargetSlot);

		// serialize for server recreation
		ServerMoves.Emplace(Item, Move.TargetSlot);
	}

	if (ServerMoves.IsEmpty())
	{
		// nothing to send
		UE_LOG(LogGameItems, Verbose, TEXT("%s [%hs] All items invalid or already pending net change"),
			*GetDebugPrefix(), __func__);
		return;
	}

	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s [%hs] Moving %d items to %s (Key: %s)"),
		*GetDebugPrefix(), __func__, ServerMoves.Num(), *To->GetReadableName(), *PredictionKey.ToString());

	// send the items and await confirmation
	PredictionContainerMap.Emplace(PredictionKey, MoveSpec.Containers);
	ServerReceiveItems(ServerMoves, MoveSpec.Containers, PredictionKey);
}

void UGameItemControllerComponent::MoveServerItemsToClient(const FGameItemMoveSpec& MoveSpec)
{
	// - generate a prediction key, and 'add' the items locally (in a pending state for easy rollback)
	// - request the server send the items (which just involves removing them)
	// - server acks with the prediction...
	//   - if accepted, commits the "real" adding of items on this client
	//   - if rejected, unmarks the pending add

	if (!MoveSpec.IsValid())
	{
		return;
	}

	const FGameItemsPredictionKey PredictionKey = FGameItemsPredictionKey::CreateNewClientPredictionKey(GetOwner());

	TArray<FGameItemMove> ServerMoves;
	for (const FGameItemMove& Move : MoveSpec.Moves)
	{
		UGameItem* Item = Move.Item;
		if (!ensure(Item))
		{
			continue;
		}

		if (Item->HasPendingNetChange())
		{
			UE_LOG(LogGameItems, Warning, TEXT("Item already pending net changes: %s"),
				*Item->GetDebugString());

			// item already waiting on some predicted action, leave it alone
			continue;
		}

		UE_LOG(LogGameItems, VeryVerbose, TEXT("%s [%hs] Marking for remove: %s"),
			*GetDebugPrefix(), __func__, *Item->GetDebugString());
		Item->MarkPendingRemove(MoveSpec.Containers.From, PredictionKey);

		// mark item as pending add in this container
		// AddPendingItem(Item, TargetSlot);

		// no serialization when client takes items from server, they already exist on both sides
		ServerMoves.Emplace(Move);
	}

	MoveSpec.Containers.To->PendingAddExistingItems.Emplace(PredictionKey, ServerMoves);

	if (ServerMoves.IsEmpty())
	{
		// nothing to receive
		UE_LOG(LogGameItems, Verbose, TEXT("%s [%hs] All items invalid or already pending net change"),
			*GetDebugPrefix(), __func__);
		return;
	}

	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s [%hs] Moving %d items from %s -> %s (Key: %s)"),
		*GetDebugPrefix(), __func__, ServerMoves.Num(),
		*MoveSpec.Containers.From->GetReadableName(),
		*MoveSpec.Containers.To->GetReadableName(),
		*PredictionKey.ToString());

	// request the items and await confirmation
	PredictionContainerMap.Emplace(PredictionKey, MoveSpec.Containers);
	ServerSendItems(ServerMoves, MoveSpec.Containers, PredictionKey);
}

void UGameItemControllerComponent::MoveServerItems(const FGameItemMoveSpec& MoveSpec)
{
	// - generate a prediction key, and mark the items as pending move
	// - request the server perform the moves
	// - server acks the prediction...
	//   - if accepted, waits for replication and clears the pending moves
	//   - if rejected, unmarks the pending move

	if (!MoveSpec.IsValid())
	{
		return;
	}

	const FGameItemsPredictionKey PredictionKey = FGameItemsPredictionKey::CreateNewClientPredictionKey(GetOwner());

	TArray<FGameItemMove> ServerMoves;
	for (const FGameItemMove& Move : MoveSpec.Moves)
	{
		UGameItem* Item = Move.Item;
		if (!ensure(Item))
		{
			continue;
		}

		if (Item->HasPendingNetChange())
		{
			UE_LOG(LogGameItems, Warning, TEXT("Item already pending net changes: %s"),
				*Item->GetDebugString());

			// item already waiting on some predicted action, leave it alone
			continue;
		}

		UE_LOG(LogGameItems, VeryVerbose, TEXT("%s [%hs] Marking pending move: %s"),
			*GetDebugPrefix(), __func__, *Item->GetDebugString());
		Item->MarkPendingMove(PredictionKey);

		// TODO: mark both from and to slots as pending net change, so containers don't allow them to be used
		// mark item as pending add in this container
		// AddPendingItem(Item, TargetSlot);

		// no serialization when for server-to-server moves, all items already replicated
		ServerMoves.Emplace(Move);
	}

	// request the move and await confirmation
	PredictionContainerMap.Emplace(PredictionKey, MoveSpec.Containers);
	ServerMoveItems(ServerMoves, MoveSpec.Containers, PredictionKey);
}

void UGameItemControllerComponent::ServerReceiveItems_Implementation(
	const TArray<FGameItemSerializedMove>& Moves,
	const FGameItemContainerPair& Containers,
	FGameItemsPredictionKey PredictionKey)
{
	if (!Containers.IsValid())
	{
		UE_LOG(LogGameItems, Warning, TEXT("%s [ServerReceiveItems] Cant move items, invalid containers (From: %s, To: %s) (Key: %s)"),
			*GetDebugPrefix(), *GetNameSafe(Containers.From), *GetNameSafe(Containers.To), *PredictionKey.ToString());

		ClientConfirmPredictionKey(PredictionKey, false);
		return;
	}

	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s [ServerReceiveItems] Receiving %d items moving from %s -> %s (Key: %s)"),
		*GetDebugPrefix(), Moves.Num(), *Containers.From->GetReadableName(), *Containers.To->GetReadableName(), *PredictionKey.ToString());

	UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::Get(this);

	bool bSuccess = true;

	// recreate the items in the target container
	for (const FGameItemSerializedMove& Move : Moves)
	{
		const FGameItemSaveData& ItemData = Move.ItemData;
		const int32 TargetSlot = Move.TargetSlot;

		if (UGameItem* NewItem = ItemSubsystem->CreateItemFromSaveData(Containers.To->GetItemOuter(), ItemData))
		{
			UE_LOG(LogGameItems, VeryVerbose, TEXT("%s [ServerReceiveItems] Recreated item %s (Key: %s)"),
				*GetDebugPrefix(), *NewItem->GetDebugString(), *PredictionKey.ToString());

			// TODO: test ALL items up front before adding any! or mark added items with a prediction key for rollback
			// make sure the item will add successfully
			const FGameItemContainerAddPlan Plan = Containers.To->CheckAddItem(NewItem, TargetSlot, Containers.From);
			if (!Plan.bWillAddFullAmount)
			{
				bSuccess = false;
				break;
			}

			Containers.To->AddItem(NewItem, TargetSlot);
		}
		else
		{
			// this should cancel the operation, but for now allow partial failure
			UE_LOG(LogGameItems, Error, TEXT("%s [ServerReceiveItems] Failed to recreate client item: %s (Key: %s)"),
				*GetDebugPrefix(), *ItemData.ToString(), *PredictionKey.ToString());
		}
	}

	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s [ServerReceiveItems] Calling ClientConfirmPredictionKey %s (Key: %s)"),
		*GetDebugPrefix(), bSuccess ? TEXT("Accepted") : TEXT("Rejected"), *PredictionKey.ToString());

	ClientConfirmPredictionKey(PredictionKey, bSuccess);
}

void UGameItemControllerComponent::ServerSendItems_Implementation(
	const TArray<FGameItemMove>& Moves,
	const FGameItemContainerPair& Containers,
	FGameItemsPredictionKey PredictionKey)
{
	if (!Containers.IsValid())
	{
		UE_LOG(LogGameItems, Warning, TEXT("%s [ServerSendItems] Cant move items, invalid containers (From: %s, To: %s) (Key: %s)"),
			*GetDebugPrefix(), *GetNameSafe(Containers.From), *GetNameSafe(Containers.To), *PredictionKey.ToString());

		ClientConfirmPredictionKey(PredictionKey, false);
		return;
	}

	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s [ServerSendItems] Sending %d items moving from %s -> %s (Key: %s)"),
		*GetDebugPrefix(), Moves.Num(), *Containers.From->GetReadableName(), *Containers.To->GetReadableName(), *PredictionKey.ToString());

	bool bSuccess = true;

	for (const FGameItemMove& Move : Moves)
	{
		UGameItem* Item = Move.Item;
		if (!Item)
		{
			bSuccess = false;
			break;
		}

		// all we have to do on server is remove and confirm, the client will add their local-only items
		Containers.From->RemoveItem(Item);
	}

	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s [ServerSendItems] Calling ClientConfirmPredictionKey %s (Key: %s)"),
		*GetDebugPrefix(), bSuccess ? TEXT("Accepted") : TEXT("Rejected"), *PredictionKey.ToString());

	ClientConfirmPredictionKey(PredictionKey, bSuccess);
}

void UGameItemControllerComponent::ServerMoveItems_Implementation(
	const TArray<FGameItemMove>& Moves,
	const FGameItemContainerPair& Containers,
	FGameItemsPredictionKey PredictionKey)
{
	if (!Containers.IsValid())
	{
		UE_LOG(LogGameItems, Warning, TEXT("%s [ServerMoveItems] Cant move items, invalid containers (From: %s, To: %s) (Key: %s)"),
			*GetDebugPrefix(), *GetNameSafe(Containers.From), *GetNameSafe(Containers.To), *PredictionKey.ToString());

		ClientConfirmPredictionKey(PredictionKey, false);
		return;
	}

	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s [ServerMoveItems] Moving %d items from %s -> %s (Key: %s)"),
		*GetDebugPrefix(), Moves.Num(), *Containers.From->GetReadableName(), *Containers.To->GetReadableName(), *PredictionKey.ToString());

	bool bSuccess = true;

	for (const FGameItemMove& Move : Moves)
	{
		UGameItem* Item = Move.Item;
		if (!Item || !Containers.From->Contains(Item))
		{
			bSuccess = false;
			break;
		}

		// perform the full move, client will just receive replicated results
		UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::Get(this);
		if (!ItemSubsystem->MoveSwapOrStackItem(Containers.From, Item, Containers.To, Move.TargetSlot))
		{
			bSuccess = false;
			break;
		}
	}

	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s [ServerMoveItems] Calling ClientConfirmPredictionKey %s (Key: %s)"),
		*GetDebugPrefix(), bSuccess ? TEXT("Accepted") : TEXT("Rejected"), *PredictionKey.ToString());

	ClientConfirmPredictionKey(PredictionKey, bSuccess);
}

void UGameItemControllerComponent::ClientConfirmPredictionKey_Implementation(
	const FGameItemsPredictionKey& PredictionKey,
	bool bAccepted)
{
	if (!ensure(PredictionKey.IsValidKey()))
	{
		return;
	}

	UE_LOG(LogGameItems, VeryVerbose, TEXT("%s [ClientConfirmPredictionKey]: %s (Key: %s)"),
		*GetDebugPrefix(), bAccepted ? TEXT("Accepted") : TEXT("Rejected"), *PredictionKey.ToString());

	const FGameItemContainerPair* AffectedPair = PredictionContainerMap.Find(PredictionKey);
	if (!ensure(AffectedPair))
	{
		return;
	}

	// update both containers
	if (AffectedPair->From)
	{
		AffectedPair->From->ConfirmPredictionKey(PredictionKey, bAccepted);
	}
	if (AffectedPair->To)
	{
		AffectedPair->To->ConfirmPredictionKey(PredictionKey, bAccepted);
	}

	PredictionContainerMap.Remove(PredictionKey);
}
