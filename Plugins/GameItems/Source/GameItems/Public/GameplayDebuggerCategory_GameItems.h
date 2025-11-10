// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#if WITH_GAMEPLAY_DEBUGGER
#include "GameplayDebuggerCategory.h"

namespace GameItems::Debug
{
	enum class ENetworkStatus : uint8
	{
		ServerOnly,
		LocalOnly,
		Networked,
		Detached,
		MAX
	};
}

class GAMEITEMS_API FGameplayDebuggerCategory_GameItems : public FGameplayDebuggerCategory
{
public:
	FGameplayDebuggerCategory_GameItems();

	static TSharedRef<FGameplayDebuggerCategory> MakeInstance();

	virtual void CollectData(APlayerController* OwnerPC, AActor* DebugActor) override;
	virtual void DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext) override;

	void ToggleShowItems();
	void ToggleShowRules();

protected:
	/** The possible states of a replicated game item or container. */

	struct FRepData
	{
		struct FGameItemDebug
		{
			int32 Slot;
			FString Item;

			int32 ReplicationID = INDEX_NONE;
			GameItems::Debug::ENetworkStatus NetworkStatus = GameItems::Debug::ENetworkStatus::ServerOnly;
		};

		struct FGameItemContainerDebug
		{
			FString ContainerId;
			FString Owner;
			int32 NumSlots;
			TArray<FGameItemDebug> Items;

			GameItems::Debug::ENetworkStatus NetworkStatus = GameItems::Debug::ENetworkStatus::ServerOnly;
		};

		TArray<FGameItemContainerDebug> Containers;

		void Serialize(FArchive& Ar);
	};

	FRepData DataPack;

	/** Collect all item container debug data for an actor. */
	TArray<FRepData::FGameItemContainerDebug> CollectContainerData(APlayerController* OwnerPC, AActor* DebugActor);

	void DrawContainers(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext);

	/** Last expected draw size to be able to draw a border around it next frame */
	float LastDrawDataEndSize = 0.0f;

	bool bShowItems = true;
	bool bShowRules = true;
};

#endif
