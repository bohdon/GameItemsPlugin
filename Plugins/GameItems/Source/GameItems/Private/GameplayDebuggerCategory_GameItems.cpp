// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameplayDebuggerCategory_GameItems.h"

#if WITH_GAMEPLAY_DEBUGGER_MENU

#include "GameItemContainer.h"
#include "GameItemStatics.h"
#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"
#include "Rules/GameItemContainerRule.h"


namespace GameItems::Debug
{
	constexpr int32 ToInt(const ENetworkStatus& Value) { return static_cast<int32>(Value); }

	const TCHAR* BothColor = TEXT("{yellow}");
	const TCHAR* ServerColor = TEXT("{cyan}");
	const TCHAR* LocalColor = TEXT("{green}");
	const TCHAR* DesyncedColor = TEXT("{tan}");

	inline FString ColorNetworkString(ENetworkStatus NetworkStatus, const FStringView DisplayString)
	{
		static const TCHAR* Colors[ToInt(ENetworkStatus::MAX)] = {
			ServerColor,
			LocalColor,
			BothColor,
			DesyncedColor
		};

		return FString::Printf(TEXT("%s%.*s"), Colors[ToInt(NetworkStatus)], DisplayString.Len(), DisplayString.GetData());
	}
}


FGameplayDebuggerCategory_GameItems::FGameplayDebuggerCategory_GameItems()
{
	SetDataPackReplication<FRepData>(&DataPack);

	// hard coded to void importing InputCore
	const FName KeyNameEight("Eight");
	const FName KeyNameNine("Nine");

	BindKeyPress(KeyNameEight, FGameplayDebuggerInputModifier::Shift, this,
	             &FGameplayDebuggerCategory_GameItems::ToggleShowItems, EGameplayDebuggerInputMode::Local);
	BindKeyPress(KeyNameNine, FGameplayDebuggerInputModifier::Shift, this,
	             &FGameplayDebuggerCategory_GameItems::ToggleShowRules, EGameplayDebuggerInputMode::Local);
}

TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_GameItems::MakeInstance()
{
	return MakeShareable(new FGameplayDebuggerCategory_GameItems());
}

void FGameplayDebuggerCategory_GameItems::FRepData::Serialize(FArchive& Ar)
{
	int32 NumContainers = Containers.Num();
	Ar << NumContainers;
	if (Ar.IsLoading())
	{
		Containers.SetNum(NumContainers);
	}

	for (int32 Idx = 0; Idx < NumContainers; ++Idx)
	{
		Ar << Containers[Idx].ContainerId;
		Ar << Containers[Idx].Owner;
		Ar << Containers[Idx].NumSlots;
		Ar << Containers[Idx].NetworkStatus;

		int32 NumRules = Containers[Idx].Rules.Num();
		Ar << NumRules;
		if (Ar.IsLoading())
		{
			Containers[Idx].Rules.SetNum(NumRules);
		}

		for (int32 RuleIdx = 0; RuleIdx < NumRules; ++RuleIdx)
		{
			Ar << Containers[Idx].Rules[RuleIdx].Rule;
			Ar << Containers[Idx].Rules[RuleIdx].NetworkStatus;
		}

		int32 NumItems = Containers[Idx].Items.Num();
		Ar << NumItems;
		if (Ar.IsLoading())
		{
			Containers[Idx].Items.SetNum(NumItems);
		}

		for (int32 ItemIdx = 0; ItemIdx < NumItems; ++ItemIdx)
		{
			Ar << Containers[Idx].Items[ItemIdx].Item;
			Ar << Containers[Idx].Items[ItemIdx].ReplicationID;
			Ar << Containers[Idx].Items[ItemIdx].NetworkStatus;
		}
	}
}

void FGameplayDebuggerCategory_GameItems::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	DataPack.Containers = CollectContainerData(OwnerPC, DebugActor);
}

TArray<FGameplayDebuggerCategory_GameItems::FRepData::FContainerDebug> FGameplayDebuggerCategory_GameItems::CollectContainerData(
	APlayerController* OwnerPC, AActor* DebugActor)
{
	if (!OwnerPC)
	{
		return TArray<FRepData::FContainerDebug>();
	}

	using namespace GameItems::Debug;
	TArray<FRepData::FContainerDebug> Result;

	ENetworkStatus NetworkStatus = OwnerPC->HasAuthority() ? ENetworkStatus::ServerOnly : ENetworkStatus::LocalOnly;

	TArray<UGameItemContainer*> Containers = UGameItemStatics::GetAllItemContainersForActor(DebugActor);
	for (const UGameItemContainer* Container : Containers)
	{
		FRepData::FContainerDebug& ContainerData = Result.AddDefaulted_GetRef();
		ContainerData.ContainerId = Container->ContainerId.ToString();
		ContainerData.NumSlots = Container->GetNumSlots();
		const UActorComponent* OuterComp = Container->GetTypedOuter<UActorComponent>();
		ContainerData.Owner = OuterComp ? OuterComp->GetReadableName() : GetNameSafe(Container->GetOwner());
		ContainerData.NetworkStatus = NetworkStatus;

		// collect rules data
		ContainerData.Rules.Reserve(Container->GetRules().Num());
		for (const UGameItemContainerRule* Rule : Container->GetRules())
		{
			FRepData::FContainerRuleDebug& RuleData = ContainerData.Rules.AddDefaulted_GetRef();
			RuleData.NetworkStatus = NetworkStatus;
			if (!Rule)
			{
				RuleData.Rule = TEXT("(null)");
				continue;
			}
			RuleData.Rule = Rule->GetDebugString();
		}

		// show * before each item, if the container is a child and doesn't store its own items
		const FString ItemPrefix = Container->IsChild() ? TEXT("*") : TEXT("");

		// collect items data
		const FGameItemList& ItemList = Container->GetInternalItemList();
		ContainerData.Items.Reserve(ItemList.GetEntries().Num());
		for (const FGameItemListEntry& Entry : ItemList.GetEntries())
		{
			FRepData::FItemDebug& ItemData = ContainerData.Items.AddDefaulted_GetRef();
			const UGameItem* Item = Entry.Item;
			ItemData.ReplicationID = Entry.ReplicationID;
			ItemData.NetworkStatus = NetworkStatus;
			if (Item)
			{
				ItemData.Item = FString::Printf(TEXT("[%d] %s%s"), Entry.Slot, *ItemPrefix, *Item->GetDebugString());
			}
			else
			{
				ItemData.Item = FString::Printf(TEXT("[%d] (null)"), Entry.Slot);
			}
		}
	}
	return Result;
}

void FGameplayDebuggerCategory_GameItems::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	using namespace GameItems::Debug;

	// header, show toggle hotkeys and active categories
	{
		CanvasContext.CursorX += 200.f;
		CanvasContext.CursorY -= CanvasContext.GetLineHeight();
		const TCHAR* Active = TEXT("{green}");
		const TCHAR* Inactive = TEXT("{grey}");
		CanvasContext.Printf(TEXT("Items [%s%s{white}]\tRules [%s%s{white}]\tLegend:  %sBoth    %sServer    %sLocal    %sDesynced"),
		                     bShowItems ? Active : Inactive, *GetInputHandlerDescription(0),
		                     bShowRules ? Active : Inactive, *GetInputHandlerDescription(1),
		                     BothColor, ServerColor, LocalColor, DesyncedColor);
	}

	if (LastDrawDataEndSize <= 0.0f)
	{
		LastDrawDataEndSize = 100.f;
	}

	const float ThisDrawDataStartPos = CanvasContext.CursorY;

	constexpr FLinearColor BackgroundColor(0.1f, 0.1f, 0.1f, 0.8f);
	const FVector2D BackgroundPos{CanvasContext.CursorX, CanvasContext.CursorY};
	const FVector2D BackgroundSize(CanvasContext.Canvas->SizeX - (2.0f * CanvasContext.CursorX), LastDrawDataEndSize);

	// draw a transparent dark background
	FCanvasTileItem Background(FVector2D(0.0f), BackgroundSize, BackgroundColor);
	Background.BlendMode = SE_BLEND_Translucent;
	CanvasContext.DrawItem(Background, BackgroundPos.X, BackgroundPos.Y);

	// draw all containers
	DrawContainers(OwnerPC, CanvasContext);

	LastDrawDataEndSize = CanvasContext.CursorY - ThisDrawDataStartPos;
}

void FGameplayDebuggerCategory_GameItems::ToggleShowItems()
{
	bShowItems = !bShowItems;
}

void FGameplayDebuggerCategory_GameItems::ToggleShowRules()
{
	bShowRules = !bShowRules;
}

void FGameplayDebuggerCategory_GameItems::DrawContainers(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
	using namespace GameItems::Debug;

	constexpr float Padding = 10.0f;

	// gather local data to compare (or same data if standalone)
	const TArray<FRepData::FContainerDebug> LocalContainersData = OwnerPC->IsNetMode(ENetMode::NM_Standalone)
		? DataPack.Containers
		: CollectContainerData(OwnerPC, FindLocalDebugActor());

	// merge server and local containers and items update network status
	TMap<FString, FRepData::FContainerDebug> AllContainers;
	for (const FRepData::FContainerDebug& RemoteContainerData : DataPack.Containers)
	{
		AllContainers.Emplace(RemoteContainerData.ContainerId, RemoteContainerData);
	}
	for (const FRepData::FContainerDebug& LocalContainerData : LocalContainersData)
	{
		if (FRepData::FContainerDebug* RemoteContainerData = AllContainers.Find(LocalContainerData.ContainerId))
		{
			// found matching container, do basic match check
			const bool bIsContainerMatch = RemoteContainerData->NumSlots == LocalContainerData.NumSlots;
			RemoteContainerData->NetworkStatus = bIsContainerMatch ? ENetworkStatus::Networked : ENetworkStatus::Detached;

			// merge all rules (by array index, not replication ID or anything)
			for (int32 Idx = 0; Idx < RemoteContainerData->Rules.Num(); ++Idx)
			{
				if (LocalContainerData.Rules.IsValidIndex(Idx))
				{
					// make sure they match
					FRepData::FContainerRuleDebug& RemoteRuleData = RemoteContainerData->Rules[Idx];
					const FRepData::FContainerRuleDebug& LocalRuleData = LocalContainerData.Rules[Idx];
					const bool bIsSynced = LocalRuleData.Rule == RemoteRuleData.Rule;
					RemoteRuleData.NetworkStatus = bIsSynced ? ENetworkStatus::Networked : ENetworkStatus::Detached;
					if (!bIsSynced)
					{
						// change displayed data to show local on left, remote on right
						RemoteRuleData.Rule = FString::Printf(TEXT("%s --x-- %s"), *LocalRuleData.Rule, *RemoteRuleData.Rule);
					}
				}
			}

			// include any extra local rules (if there are more than remote)
			for (int32 Idx = RemoteContainerData->Rules.Num(); Idx < LocalContainerData.Rules.Num(); ++Idx)
			{
				RemoteContainerData->Rules.Emplace(LocalContainerData.Rules[Idx]);
			}

			// merge all items
			for (FRepData::FItemDebug& RemoteItemData : RemoteContainerData->Items)
			{
				const FRepData::FItemDebug* LocalItemData = LocalContainerData.Items.FindByPredicate([&](const FRepData::FItemDebug& Other)
				{
					return Other.ReplicationID == RemoteItemData.ReplicationID;
				});

				if (LocalItemData)
				{
					// make sure they match
					const bool bIsSynced = RemoteItemData.Item == LocalItemData->Item;
					RemoteItemData.NetworkStatus = bIsSynced ? ENetworkStatus::Networked : ENetworkStatus::Detached;
					if (!bIsSynced)
					{
						// change displayed data to show local on left, remote on right
						RemoteItemData.Item = FString::Printf(TEXT("%s --x-- %s"), *LocalItemData->Item, *RemoteItemData.Item);
					}
				}
			}

			// include any local items not already in the list
			for (const FRepData::FItemDebug& LocalItemData : LocalContainerData.Items)
			{
				const FRepData::FItemDebug* RemoteItemData = RemoteContainerData->Items.FindByPredicate([&](const FRepData::FItemDebug& Other)
				{
					return Other.ReplicationID == LocalItemData.ReplicationID;
				});

				// copy into the list
				if (!RemoteItemData)
				{
					RemoteContainerData->Items.Emplace(LocalItemData);
				}
			}
		}
		else
		{
			AllContainers.Emplace(LocalContainerData.ContainerId, LocalContainerData);
		}
	}

	CanvasContext.Print(TEXT("Game Item Containers:"));

	// offset up a line so each loop can start a new one
	CanvasContext.CursorY -= CanvasContext.GetLineHeight();
	for (auto& Elem : AllContainers)
	{
		const FRepData::FContainerDebug& ContainerData = Elem.Value;

		CanvasContext.MoveToNewLine();
		CanvasContext.CursorX += Padding;

		float CursorX = CanvasContext.CursorX;
		float CursorY = CanvasContext.CursorY;

		// container id and owner
		const FString ContainerText = ColorNetworkString(ContainerData.NetworkStatus, *ContainerData.ContainerId);
		CanvasContext.PrintfAt(CursorX, CursorY, TEXT("%s [%d/%d] {grey}(%s)"),
		                       *ContainerText, ContainerData.Items.Num(), ContainerData.NumSlots, *ContainerData.Owner);

		// rules
		if (bShowRules)
		{
			for (const FRepData::FContainerRuleDebug& RuleData : ContainerData.Rules)
			{
				CanvasContext.MoveToNewLine();
				CanvasContext.CursorX += Padding * 2;
				CursorX = CanvasContext.CursorX;
				CursorY = CanvasContext.CursorY;

				const FString RuleText = ColorNetworkString(RuleData.NetworkStatus, RuleData.Rule);
				CanvasContext.PrintfAt(CursorX, CursorY, TEXT("+ %s"), *RuleText);
			}
		}

		// items
		if (bShowItems)
		{
			for (const FRepData::FItemDebug& ItemData : ContainerData.Items)
			{
				CanvasContext.MoveToNewLine();
				CanvasContext.CursorX += Padding * 2;

				CursorX = CanvasContext.CursorX;
				CursorY = CanvasContext.CursorY;

				const FString ItemText = ColorNetworkString(ItemData.NetworkStatus, *ItemData.Item);
				CanvasContext.PrintAt(CursorX, CursorY, ItemText);
			}
		}

		CanvasContext.MoveToNewLine();
	}

	if (DataPack.Containers.IsEmpty())
	{
		CanvasContext.MoveToNewLine();
		CanvasContext.CursorX += Padding;
		CanvasContext.Print(TEXT("(none)"));
	}
}

#endif
