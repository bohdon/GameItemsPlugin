// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameItemDefBlueprint.h"
#include "Script/AssetDefinition_Blueprint.h"
#include "AssetDefinition_GameItemDefBlueprint.generated.h"


UCLASS()
class GAMEITEMSEDITOR_API UAssetDefinition_GameItemDefBlueprint : public UAssetDefinition_Blueprint
{
	GENERATED_BODY()

public:
	// UAssetDefinition
	virtual FText GetAssetDisplayName() const override;
	virtual FLinearColor GetAssetColor() const override { return FLinearColor(FColor(150, 120, 100)); }
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return UGameItemDefBlueprint::StaticClass(); }
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;

	// UAssetDefinition_Blueprint
	virtual UFactory* GetFactoryForBlueprintType(UBlueprint* InBlueprint) const override;
	
};
