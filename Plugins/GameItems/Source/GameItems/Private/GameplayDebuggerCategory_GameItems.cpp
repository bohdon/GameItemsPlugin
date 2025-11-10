// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameplayDebuggerCategory_GameItems.h"

#if WITH_GAMEPLAY_DEBUGGER_MENU

#include "GameItemContainer.h"
#include "GameItemStatics.h"
#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"


namespace GameItems::Debug
{
	constexpr int32 ToInt(const ENetworkStatus& Value) { return static_cast<int32>(Value); }

	const TCHAR* BothColor = TEXT("{yellow}");
	const TCHAR* ServerColor = TEXT("{cyan}");
	const TCHAR* LocalColor = TEXT("{green}");
	const TCHAR* NonReplicatedColor = TEXT("{violetred}");

	inline FString ColorNetworkString(ENetworkStatus NetworkStatus, const FStringView DisplayString)
	{
		static const TCHAR* Colors[ToInt(ENetworkStatus::MAX)] = {
			ServerColor,
			LocalColor,
			BothColor,
			NonReplicatedColor
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

		int32 NumItems = Containers[Idx].Items.Num();
		Ar << NumItems;
		if (Ar.IsLoading())
		{
			Containers[Idx].Items.SetNum(NumItems);
		}

		for (int32 ItemIdx = 0; ItemIdx < NumItems; ++ItemIdx)
		{
			Ar << Containers[Idx].Items[ItemIdx].Slot;
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

TArray<FGameplayDebuggerCategory_GameItems::FRepData::FGameItemContainerDebug> FGameplayDebuggerCategory_GameItems::CollectContainerData(
	APlayerController* OwnerPC, AActor* DebugActor)
{
	if (!OwnerPC)
	{
		return TArray<FRepData::FGameItemContainerDebug>();
	}

	using namespace GameItems::Debug;
	TArray<FRepData::FGameItemContainerDebug> Result;

	ENetworkStatus NetworkStatus = OwnerPC->HasAuthority() ? ENetworkStatus::ServerOnly : ENetworkStatus::LocalOnly;

	TArray<UGameItemContainer*> Containers = UGameItemStatics::GetAllItemContainersForActor(DebugActor);
	for (const UGameItemContainer* Container : Containers)
	{
		FRepData::FGameItemContainerDebug& ContainerData = Result.AddDefaulted_GetRef();
		ContainerData.ContainerId = Container->ContainerId.ToString();
		ContainerData.NumSlots = Container->GetNumSlots();
		const UActorComponent* OuterComp = Container->GetTypedOuter<UActorComponent>();
		ContainerData.Owner = OuterComp ? OuterComp->GetReadableName() : GetNameSafe(Container->GetOwner());
		ContainerData.NetworkStatus = NetworkStatus;

		const FGameItemList& ItemList = Container->GetInternalItemList();

		ContainerData.Items.Reserve(ItemList.GetEntries().Num());
		for (const FGameItemListEntry& Entry : ItemList.GetEntries())
		{
			FRepData::FGameItemDebug& ItemData = ContainerData.Items.AddDefaulted_GetRef();
			ItemData.Slot = Entry.Slot;

			const UGameItem* Item = Entry.Item;
			if (!Item)
			{
				ItemData.Item = TEXT("(invalid)");
				continue;
			}
			ItemData.Item = Item->ToDebugString();
			ItemData.ReplicationID = Entry.ReplicationID;
			ItemData.NetworkStatus = NetworkStatus;
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
		CanvasContext.Printf(TEXT("Items [%s%s{white}]\tRules [%s%s{white}]\tLegend:  %sBoth    %sServer    %sLocal"),
		                     bShowItems ? Active : Inactive, *GetInputHandlerDescription(0),
		                     bShowRules ? Active : Inactive, *GetInputHandlerDescription(1),
		                     BothColor, ServerColor, LocalColor);
	}

	if (LastDrawDataEndSize <= 0.0f)
	{
		LastDrawDataEndSize = 100.f;
	}

	float ThisDrawDataStartPos = CanvasContext.CursorY;

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
	const TArray<FRepData::FGameItemContainerDebug> LocalContainersData = OwnerPC->IsNetMode(ENetMode::NM_Standalone)
		                                                                      ? DataPack.Containers
		                                                                      : CollectContainerData(OwnerPC, FindLocalDebugActor());

	// merge server and local containers and items update network status
	TMap<FString, FRepData::FGameItemContainerDebug> AllContainers;
	for (const FRepData::FGameItemContainerDebug& RemoteContainerData : DataPack.Containers)
	{
		AllContainers.Emplace(RemoteContainerData.ContainerId, RemoteContainerData);
	}
	for (const FRepData::FGameItemContainerDebug& LocalContainerData : LocalContainersData)
	{
		if (FRepData::FGameItemContainerDebug* RemoteContainerData = AllContainers.Find(LocalContainerData.ContainerId))
		{
			// found matching container, do basic match check
			const bool bIsContainerMatch = RemoteContainerData->NumSlots == LocalContainerData.NumSlots;
			RemoteContainerData->NetworkStatus = bIsContainerMatch ? ENetworkStatus::Networked : ENetworkStatus::Detached;

			// merge all items
			for (FRepData::FGameItemDebug& RemoteItemData : RemoteContainerData->Items)
			{
				const FRepData::FGameItemDebug* LocalItemData = LocalContainerData.Items.FindByPredicate([&](const FRepData::FGameItemDebug& Other)
				{
					return Other.ReplicationID == RemoteItemData.ReplicationID;
				});

				if (LocalItemData)
				{
					// make sure they match
					const bool bIsItemMatch = RemoteItemData.Slot == LocalItemData->Slot && RemoteItemData.Item == LocalItemData->Item;
					RemoteItemData.NetworkStatus = bIsItemMatch ? ENetworkStatus::Networked : ENetworkStatus::Detached;
				}
			}

			// include any local items not already in the list
			for (const FRepData::FGameItemDebug& LocalItemData : LocalContainerData.Items)
			{
				const FRepData::FGameItemDebug* RemoteItemData = LocalContainerData.Items.FindByPredicate([&](const FRepData::FGameItemDebug& Other)
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
		const FRepData::FGameItemContainerDebug& ContainerData = Elem.Value;

		CanvasContext.MoveToNewLine();
		CanvasContext.CursorX += Padding;

		float CursorX = CanvasContext.CursorX;
		float CursorY = CanvasContext.CursorY;

		// container id and owner
		const FString ContainerName = ColorNetworkString(ContainerData.NetworkStatus, *ContainerData.ContainerId);
		CanvasContext.PrintfAt(CursorX, CursorY, TEXT("%s [%d/%d] {grey}(%s)"),
		                       *ContainerName, ContainerData.Items.Num(), ContainerData.NumSlots, *ContainerData.Owner);

		for (const FRepData::FGameItemDebug& ItemData : ContainerData.Items)
		{
			CanvasContext.MoveToNewLine();
			CanvasContext.CursorX += Padding * 2;

			CursorX = CanvasContext.CursorX;
			CursorY = CanvasContext.CursorY;

			// item slot and debug name
			const FString ItemName = ColorNetworkString(ItemData.NetworkStatus, *ItemData.Item);
			CanvasContext.PrintfAt(CursorX, CursorY, TEXT("[%d] %s"), ItemData.Slot, *ItemName);
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
