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
		struct FItemDebug
		{
			FString Item;

			int32 ReplicationID = INDEX_NONE;
			GameItems::Debug::ENetworkStatus NetworkStatus = GameItems::Debug::ENetworkStatus::ServerOnly;
		};

		struct FContainerRuleDebug
		{
			FString Rule;

			GameItems::Debug::ENetworkStatus NetworkStatus = GameItems::Debug::ENetworkStatus::ServerOnly;
		};

		struct FContainerDebug
		{
			FString ContainerId;
			FString Owner;
			int32 NumSlots;
			TArray<FContainerRuleDebug> Rules;
			TArray<FItemDebug> Items;

			GameItems::Debug::ENetworkStatus NetworkStatus = GameItems::Debug::ENetworkStatus::ServerOnly;
		};

		TArray<FContainerDebug> Containers;

		void Serialize(FArchive& Ar);
	};

	FRepData DataPack;

	/** Collect all item container debug data for an actor. */
	TArray<FRepData::FContainerDebug> CollectContainerData(APlayerController* OwnerPC, AActor* DebugActor);

	void DrawContainers(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext);

	/** Last expected draw size to be able to draw a border around it next frame */
	float LastDrawDataEndSize = 0.0f;

	bool bShowItems = true;
	bool bShowRules = false;
};

#endif
