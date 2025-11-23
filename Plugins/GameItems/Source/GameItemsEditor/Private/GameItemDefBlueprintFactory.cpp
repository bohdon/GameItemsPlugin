// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemDefBlueprintFactory.h"

#include "GameItemDef.h"
#include "GameItemDefBlueprint.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/MessageDialog.h"


#define LOCTEXT_NAMESPACE "GameItemDefBlueprintFactory"


UGameItemDefBlueprintFactory::UGameItemDefBlueprintFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UGameItemDefBlueprint::StaticClass();
	ParentClass = UGameItemDef::StaticClass();
}

UObject* UGameItemDefBlueprintFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName,
                                                        EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	check(InClass && InClass->IsChildOf<UGameItemDefBlueprint>());

	if (!FKismetEditorUtilities::CanCreateBlueprintOfClass(ParentClass) || !ParentClass->IsChildOf<UGameItemDef>())
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			FText::Format(
				LOCTEXT("InvalidParentClassMessage", "Unable to create GameItemDef Blueprint with parent class '{0}'."),
				FText::FromString(GetNameSafe(ParentClass))));
		return nullptr;
	}

	UGameItemDefBlueprint* Blueprint = CastChecked<UGameItemDefBlueprint>(FKismetEditorUtilities::CreateBlueprint(
			ParentClass,
			InParent,
			InName,
			BPTYPE_Normal,
			UGameItemDefBlueprint::StaticClass(),
			UBlueprintGeneratedClass::StaticClass(),
			CallingContext)
	);

	return Blueprint;
}

UObject* UGameItemDefBlueprintFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName,
                                                        EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return FactoryCreateNew(InClass, InParent, InName, Flags, Context, Warn, NAME_None);
}

#undef LOCTEXT_NAMESPACE
