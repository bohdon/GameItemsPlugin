// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemCheatsExtension.h"

#include "GameItemContainer.h"
#include "GameItemDef.h"
#include "GameItemSubsystem.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GeometryCollection/Facades/CollectionPositionTargetFacade.h"


void UGameItemCheatsExtension::ItemList()
{
	UE_LOG(LogConsoleResponse, Log, TEXT("Loaded Game Item Definitions:"));

	TArray<TSubclassOf<UGameItemDef>> AllItemDefs;
	GetAllLoadedItemDefs(AllItemDefs);
	for (const TSubclassOf<UGameItemDef>& ItemDef : AllItemDefs)
	{
		UE_LOG(LogConsoleResponse, Log, TEXT("  %s"), *ItemDef->GetName());
	}
}

void UGameItemCheatsExtension::ItemAdd(FString ItemDefName, int32 Count, AActor* Actor)
{
	const TSubclassOf<UGameItemDef> ItemDef = FindItemDef(ItemDefName);
	if (!ItemDef)
	{
		return;
	}

	Actor = ResolveActor(Actor);
	if (!Actor)
	{
		return;
	}

	UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(this);
	UGameItemContainer* Container = ItemSubsystem->GetDefaultContainerForActor(Actor);
	if (!Container)
	{
		return;
	}

	UE_LOG(LogConsoleResponse, Log, TEXT("Adding %d %s to %s"),
	       Count, *ItemDef->GetName(), *Container->GetReadableName());
	ItemSubsystem->CreateItemInContainer(Container, ItemDef, Count);
}

void UGameItemCheatsExtension::ItemRemove(FString ItemDefName, int32 Count, AActor* Actor)
{
	Actor = ResolveActor(Actor);
	if (!Actor)
	{
		return;
	}

	const UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(this);
	UGameItemContainer* Container = ItemSubsystem->GetDefaultContainerForActor(Actor);
	if (!Container)
	{
		return;
	}

	const TSubclassOf<UGameItemDef> ItemDef = FindItemDefInContainer(Container, ItemDefName);
	if (!ItemDef)
	{
		return;
	}

	UE_LOG(LogConsoleResponse, Log, TEXT("Removing %d %s from %s"),
	       Count, *ItemDef->GetName(), *Container->GetReadableName());
	Container->RemoveItemsByDef(ItemDef, Count);
}

AActor* UGameItemCheatsExtension::ResolveActor(AActor* Actor)
{
	return Actor ? Actor : GetOuterUCheatManager()->GetPlayerController()->GetPawn();
}

TSubclassOf<UGameItemDef> UGameItemCheatsExtension::FindItemDefInContainer(UGameItemContainer* Container, const FString& ItemDefName, bool bLogWarning) const
{
	TArray<UGameItem*> AllItems = Container->GetAllItems();
	for (const UGameItem* Item : AllItems)
	{
		if (!Item || !Item->GetItemDef())
		{
			continue;
		}
		if (Item->GetItemDef()->GetName().Contains(ItemDefName))
		{
			return Item->GetItemDef();
		}
	}

	if (bLogWarning)
	{
		UE_LOG(LogConsoleResponse, Warning, TEXT("ItemDef not found in %s: '%s'"),
		       *Container->GetReadableName(), *ItemDefName);
	}
	return nullptr;
}

TSubclassOf<UGameItemDef> UGameItemCheatsExtension::FindItemDef(const FString& ItemDefName, bool bLogWarning) const
{
	if (ItemDefName.IsEmpty() || ItemDefName.Contains(TEXT(" ")) || ItemDefName.Contains(TEXT(".")))
	{
		return nullptr;
	}

	// check native classes and loaded assets first
	TArray<TSubclassOf<UGameItemDef>> LoadedItemDefs;
	GetAllLoadedItemDefs(LoadedItemDefs);

	for (const TSubclassOf<UGameItemDef>& ItemDef : LoadedItemDefs)
	{
		if (ItemDef->GetName().Contains(ItemDefName))
		{
			return ItemDef;
		}
	}

	// try the asset registry for blueprints that match the requirements
	if (TSubclassOf<UGameItemDef> ItemDef = FindBlueprintItemDef(ItemDefName))
	{
		return ItemDef;
	}

	if (bLogWarning)
	{
		UE_LOG(LogConsoleResponse, Warning, TEXT("ItemDef not found: '%s'"), *ItemDefName);
	}
	return nullptr;
}

void UGameItemCheatsExtension::GetAllLoadedItemDefs(TArray<TSubclassOf<UGameItemDef>>& OutItemDefs) const
{
	TArray<UClass*> Classes;
	GetDerivedClasses(UGameItemDef::StaticClass(), Classes);
	Classes.RemoveAll([](const UClass* InClass)
	{
		const FString Name = InClass->GetName();
		return InClass->HasAnyClassFlags(CLASS_Abstract) ||
			Name.StartsWith(TEXT("SKEL_")) ||
			Name.StartsWith(TEXT("Default_SKEL_")) ||
			Name.StartsWith(TEXT("REINST_"));
	});

	OutItemDefs.Reset(Classes.Num());
	for (UClass* Class : Classes)
	{
		if (TSubclassOf<UGameItemDef> ItemDefClass = Class)
		{
			OutItemDefs.Emplace(ItemDefClass);
		}
	}
}

TSubclassOf<UGameItemDef> UGameItemCheatsExtension::FindBlueprintItemDef(const FString& ItemDefName) const
{
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	if (AssetRegistry.IsLoadingAssets())
	{
		AssetRegistry.SearchAllAssets(true);
	}

	FARFilter Filter;
	Filter.bRecursiveClasses = true;
	Filter.ClassPaths.Add(UBlueprintCore::StaticClass()->GetClassPathName());

	UClass* FoundClass = nullptr;
	AssetRegistry.EnumerateAssets(Filter, [&FoundClass, ItemDefName](const FAssetData& AssetData)
	{
		if (AssetData.AssetName.ToString().Contains(ItemDefName))
		{
			if (const UBlueprint* BP = Cast<UBlueprint>(AssetData.GetAsset()))
			{
				if (BP->GeneratedClass->IsChildOf<UGameItemDef>() &&
					!BP->GeneratedClass->HasAnyClassFlags(CLASS_Abstract))
				{
					FoundClass = BP->GeneratedClass;
					return false;
				}
			}
		}
		return true;
	});

	return FoundClass;
}
