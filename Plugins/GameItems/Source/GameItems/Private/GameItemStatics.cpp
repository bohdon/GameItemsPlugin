// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemStatics.h"

#include "GameItemSubsystem.h"


UGameItemSubsystem* UGameItemStatics::GetItemSubsystemFromContextObject(const UObject* Object)
{
	if (const UWorld* World = GEngine->GetWorldFromContextObject(Object, EGetWorldErrorMode::LogAndReturnNull))
	{
		return World->GetGameInstance()->GetSubsystem<UGameItemSubsystem>();
	}
	return nullptr;
}

const UGameItemFragment* UGameItemStatics::FindGameItemFragment(const UObject* WorldContextObject, TSubclassOf<UGameItemDef> ItemDef,
                                                        TSubclassOf<UGameItemFragment> FragmentClass)
{
	const UGameItemSubsystem* ItemSubsystem = GetItemSubsystemFromContextObject(WorldContextObject);
	if (!ItemSubsystem)
	{
		return nullptr;
	}

	return ItemSubsystem->FindFragment(ItemDef, FragmentClass);
}
