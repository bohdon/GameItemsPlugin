// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemSet.h"

#include "GameItemContainer.h"
#include "GameItemContainerComponent.h"
#include "GameItemContainerInterface.h"
#include "GameItemDef.h"
#include "GameItemsModule.h"
#include "GameItemSubsystem.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/Blueprint.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"


// UGameItemSetAutoFillRule
// ------------------------

void UGameItemSetAutoFill::FillSet_Implementation(UGameItemSet* ItemSet) const
{
	ItemSet->Items.Reset();

	if (!GetPackage())
	{
		return;
	}

	const IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry")).Get();

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	for (const FDirectoryPath& Directory : SearchDirectories)
	{
		if (!Directory.Path.IsEmpty())
		{
			Filter.PackagePaths.Add(*Directory.Path);
		}
	}
	if (bSearchCurrentDirectory)
	{
		const FName OwnerPackagePath = FName(*FPackageName::GetLongPackagePath(GetPackage()->GetName()));
		Filter.PackagePaths.Add(OwnerPackagePath);
	}
	Filter.bRecursiveClasses = true;
	Filter.ClassPaths.Add(UBlueprintCore::StaticClass()->GetClassPathName());

	if (Filter.PackagePaths.IsEmpty())
	{
		// no paths to search
		return;
	}

	TArray<FAssetData> Assets;
	AssetRegistry.GetAssets(Filter, Assets);

	TArray<TSubclassOf<UGameItemDef>> ItemDefs;
	for (const FAssetData& AssetData : Assets)
	{
		// filter blueprints for item definition classes and query
		if (const UBlueprint* Blueprint = Cast<UBlueprint>(AssetData.GetAsset()))
		{
			if (Blueprint->GeneratedClass->IsChildOf<UGameItemDef>())
			{
				TSubclassOf<UGameItemDef> ItemDef = TSubclassOf<UGameItemDef>(Blueprint->GeneratedClass);
				if (ShouldIncludeItem(ItemDef))
				{
					ItemDefs.AddUnique(ItemDef);
				}
			}
		}
	}

	for (const TSubclassOf<UGameItemDef>& ItemDef : ItemDefs)
	{
		ItemSet->Items.Add(FGameItemDefStack(ItemDef, 1));
	}
}

bool UGameItemSetAutoFill::ShouldIncludeItem_Implementation(TSubclassOf<UGameItemDef> ItemDef) const
{
	if (!ItemDef || ItemDef->HasAnyClassFlags(CLASS_Abstract))
	{
		return false;
	}

	const UGameItemDef* ItemDefCDO = GetDefault<UGameItemDef>(ItemDef);

	if (!ItemDefCDO->OwnedTags.HasAll(RequireTags) || ItemDefCDO->OwnedTags.HasAny(IgnoreTags))
	{
		return false;
	}

	if (!TagQuery.IsEmpty() && !TagQuery.Matches(ItemDefCDO->OwnedTags))
	{
		return false;
	}

	for (const TSubclassOf<UGameItemFragment>& FragmentClass : RequireFragments)
	{
		if (!ItemDefCDO->FindFragment(FragmentClass))
		{
			return false;
		}
	}

	return true;
}


// UGameItemSet
// ------------

void UGameItemSet::AddToContainer(UGameItemContainer* Container) const
{
	if (!Container)
	{
		return;
	}

	UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(Container);
	if (!ItemSubsystem)
	{
		return;
	}

	for (const FGameItemDefStack& Item : Items)
	{
		ItemSubsystem->CreateItemInContainer(Container, Item.ItemDef, Item.Count);
	}
}

void UGameItemSet::AddToDefaultContainers(TScriptInterface<IGameItemContainerInterface> ContainerInterface) const
{
	UGameItemSubsystem* ItemSubsystem = UGameItemSubsystem::GetGameItemSubsystem(ContainerInterface.GetObject());
	if (!ItemSubsystem)
	{
		return;
	}

	for (const FGameItemDefStack& Item : Items)
	{
		if (UGameItemContainer* Container = ContainerInterface->GetDefaultContainerForItem(Item.ItemDef))
		{
			ItemSubsystem->CreateItemInContainer(Container, Item.ItemDef, Item.Count);
		}
#if !NO_LOGGING
		else if (UE_LOG_ACTIVE(LogGameItems, Verbose))
		{
			const UGameItemContainerComponent* ContainerComp = Cast<UGameItemContainerComponent>(ContainerInterface.GetObject());
			const FString ContainerName = ContainerComp ? ContainerComp->GetReadableName() : ContainerInterface.GetObject()->GetPathName();
			UE_LOG(LogGameItems, Verbose, TEXT("[%s] Couldn't add %s to %s, no matching container found"),
			       *GetName(), *Item.ItemDef->GetName(), *ContainerName);
		}
#endif
	}
}


#if WITH_EDITOR
void UGameItemSet::PreSave(FObjectPreSaveContext SaveContext)
{
	if (AutoFillRule)
	{
		AutoFillRule->FillSet(this);
	}

	Super::PreSave(SaveContext);
}

void UGameItemSet::PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UGameItemSet, AutoFillRule))
	{
		if (AutoFillRule)
		{
			// don't auto-update when editing the tag query, it may crash when not compiled
			auto* NextNode = PropertyChangedEvent.PropertyChain.GetActiveNode()->GetNextNode();
			if (!NextNode || NextNode->GetValue()->GetName() != GET_MEMBER_NAME_CHECKED(UGameItemSetAutoFill, TagQuery))
			{
				AutoFillRule->FillSet(this);
			}
		}
		else
		{
			Items.Reset();
		}
	}
}
#endif
