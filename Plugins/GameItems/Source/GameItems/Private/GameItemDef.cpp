// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemDef.h"

#include "GameItem.h"

#if WITH_EDITOR
#include "Fragments/GameItemFragment_UIData.h"
#endif


UGameItemDef::UGameItemDef(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ItemClass = UGameItem::StaticClass();
}

const UGameItemFragment* UGameItemDef::FindFragment(TSubclassOf<UGameItemFragment> FragmentClass) const
{
	return FindFragmentInternal(FragmentClass);
}

UGameItemFragment* UGameItemDef::FindFragmentInternal(TSubclassOf<UGameItemFragment> FragmentClass) const
{
	if (FragmentClass)
	{
		for (UGameItemFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}
	return nullptr;
}

#if WITH_EDITOR
TOptional<FSlateBrush> UGameItemDef::GetEditorIcon() const
{
	const UGameItemFragment_UIData* UIData = FindFragment<UGameItemFragment_UIData>();
	return (UIData && UIData->Icon) ? UIData->Icon->Brush : TOptional<FSlateBrush>();
}
#endif
