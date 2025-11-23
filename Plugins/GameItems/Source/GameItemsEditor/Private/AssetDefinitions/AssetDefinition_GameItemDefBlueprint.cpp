// Copyright Bohdon Sayre, All Rights Reserved.


#include "AssetDefinitions/AssetDefinition_GameItemDefBlueprint.h"

#include "GameItemDef.h"
#include "GameItemDefBlueprintFactory.h"


#define LOCTEXT_NAMESPACE "AssetTypeActions"

FText UAssetDefinition_GameItemDefBlueprint::GetAssetDisplayName() const
{
	return LOCTEXT("AssetTypeActions_GameItemDefBlueprint", "Game Item Blueprint");
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_GameItemDefBlueprint::GetAssetCategories() const
{
	static const auto Categories = {EAssetCategoryPaths::Gameplay};
	return Categories;
}

UFactory* UAssetDefinition_GameItemDefBlueprint::GetFactoryForBlueprintType(UBlueprint* InBlueprint) const
{
	UGameItemDefBlueprintFactory* Factory = NewObject<UGameItemDefBlueprintFactory>();
	Factory->ParentClass = TSubclassOf<UGameItemDef>(*InBlueprint->GeneratedClass);
	return Factory;
}

#undef LOCTEXT_NAMESPACE
