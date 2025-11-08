// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "Templates/SubclassOf.h"
#include "GameItemsEditorSubsystem.generated.h"

class UGameItemDef;
class UGameItemFragment;


/**
 * Editor tools for working with and authoring Game Items.
 */
UCLASS()
class GAMEITEMSEDITOR_API UGameItemsEditorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	/** Return the selected items from the content browser. */
	UFUNCTION(BlueprintCallable, Category = "GameItems|Editor")
	TArray<TSubclassOf<UGameItemDef>> GetSelectedItems();

	/** Find or add a fragment to an item definition. */
	UFUNCTION(BlueprintCallable, Category = "GameItems|Editor", meta = (DeterminesOutputType = "FragmentClass"))
	UGameItemFragment* FindOrAddFragment(TSubclassOf<UGameItemDef> ItemDef, TSubclassOf<UGameItemFragment> FragmentClass);

	/** Find or add a fragment to an item definition. */
	template <class T>
	T* FindOrAddFragment(TSubclassOf<UGameItemDef> ItemDef)
	{
		static_assert(TIsDerivedFrom<T, UGameItemFragment>::IsDerived, TEXT("T must derive from UGameItemFragment"));
		return Cast<T>(FindOrAddFragment(ItemDef, T::StaticClass()));
	}
};
