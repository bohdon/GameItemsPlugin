// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "GameItemDefBlueprintFactory.generated.h"

class UGameItemDef;


UCLASS()
class GAMEITEMSEDITOR_API UGameItemDefBlueprintFactory : public UFactory
{
	GENERATED_BODY()

public:
	UGameItemDefBlueprintFactory(const FObjectInitializer& ObjectInitializer);

	/** The parent class of the created blueprint */
	UPROPERTY(EditAnywhere, Category = "GameItemDefBlueprintFactory")
	TSubclassOf<UGameItemDef> ParentClass;

	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext) override;
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
