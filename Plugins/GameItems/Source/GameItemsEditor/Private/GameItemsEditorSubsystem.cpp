// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemsEditorSubsystem.h"

#include "ContentBrowserModule.h"
#include "GameItemDef.h"
#include "GameItemsEditorModule.h"
#include "IContentBrowserSingleton.h"
#include "Engine/Blueprint.h"


TArray<TSubclassOf<UGameItemDef>> UGameItemsEditorSubsystem::GetSelectedItems()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetData> SelectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

	TArray<TSubclassOf<UGameItemDef>> Result;
	for (FAssetData& AssetData : SelectedAssets)
	{
		const UClass* NativeClass = UBlueprint::GetBlueprintParentClassFromAssetTags(AssetData);
		if (!NativeClass || !NativeClass->IsChildOf<UGameItemDef>())
		{
			continue;
		}

		// load blueprint
		if (const UBlueprint* Blueprint = Cast<UBlueprint>(AssetData.GetAsset()))
		{
			if (TSubclassOf<UGameItemDef> ItemDef = Cast<UClass>(Blueprint->GeneratedClass))
			{
				Result.Add(ItemDef);
			}
		}
	}

	return Result;
}

UGameItemFragment* UGameItemsEditorSubsystem::FindMutableFragment(TSubclassOf<UGameItemDef> ItemDef, TSubclassOf<UGameItemFragment> FragmentClass)
{
	if (!ItemDef)
	{
		return nullptr;
	}
	UGameItemDef* ItemDefCDO = ItemDef->GetDefaultObject<UGameItemDef>();
	for (const TObjectPtr<UGameItemFragment>& Fragment : ItemDefCDO->Fragments)
	{
		if (Fragment->IsA(FragmentClass))
		{
			return Fragment;
		}
	}
	return nullptr;
}

UGameItemFragment* UGameItemsEditorSubsystem::FindOrAddFragment(TSubclassOf<UGameItemDef> ItemDef, TSubclassOf<UGameItemFragment> FragmentClass)
{
	if (!ItemDef)
	{
		return nullptr;
	}
	if (!ItemDef->ClassGeneratedBy)
	{
		UE_LOG(LogGameItemsEditor, Error, TEXT("[%hs] Can't edit native classes"), __func__);
		return nullptr;
	}

	UGameItemDef* ItemDefCDO = ItemDef->GetDefaultObject<UGameItemDef>();
	for (const TObjectPtr<UGameItemFragment>& Fragment : ItemDefCDO->Fragments)
	{
		if (Fragment->IsA(FragmentClass))
		{
			return Fragment;
		}
	}

	// create a new fragment
	UGameItemFragment* Fragment = NewObject<UGameItemFragment>(ItemDefCDO, FragmentClass, NAME_None, RF_Transactional);
	ItemDefCDO->Modify();
	ItemDefCDO->Fragments.Add(Fragment);

	return Fragment;
}
