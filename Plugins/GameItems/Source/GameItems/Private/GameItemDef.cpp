// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemDef.h"


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
