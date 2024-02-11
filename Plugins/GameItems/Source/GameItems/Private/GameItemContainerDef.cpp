// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainerDef.h"

#include "GameItemContainer.h"


UGameItemContainerDef::UGameItemContainerDef(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	  ContainerClass(UGameItemContainer::StaticClass()),
	  bLimitSlots(false),
	  SlotCount(0),
	  bAutoStack(false)
{
}

#if WITH_EDITOR
void UGameItemContainerDef::PreSave(FObjectPreSaveContext SaveContext)
{
	// cleanup invalid rules
	Rules.RemoveAll([](const UGameItemContainerRule* Rule)
	{
		return Rule == nullptr;
	});

	UObject::PreSave(SaveContext);
}
#endif
