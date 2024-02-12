// Copyright Bohdon Sayre, All Rights Reserved.


#include "GameItemContainerRule.h"

#include "GameItem.h"
#include "GameItemContainer.h"
#include "GameItemContainerComponent.h"
#include "GameItemDef.h"
#include "GameItemsModule.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemContainerRule)


// UGameItemContainerStockRule
// ---------------------------

UGameItemContainerRule::UGameItemContainerRule()
{
}

void UGameItemContainerRule::Initialize()
{
	Container = GetTypedOuter<UGameItemContainer>();
}

void UGameItemContainerRule::Uninitialize()
{
}

UWorld* UGameItemContainerRule::GetWorld() const
{
	return GetOuter() ? GetOuter()->GetWorld() : nullptr;
}

bool UGameItemContainerRule::CanContainItem_Implementation(const UGameItem* Item) const
{
	return true;
}

int32 UGameItemContainerRule::GetItemMaxCount_Implementation(const UGameItem* Item) const
{
	return -1;
}

int32 UGameItemContainerRule::GetItemStackMaxCount_Implementation(const UGameItem* Item) const
{
	return -1;
}


// UGameItemContainerRule_TagRequirements
// --------------------------------------

bool UGameItemContainerRule_TagRequirements::CanContainItem_Implementation(const UGameItem* Item) const
{
	const FGameplayTagContainer ItemTags = Item->GetItemDefCDO()->OwnedTags;
	return ItemTags.HasAll(RequireTags) && !ItemTags.HasAny(IgnoreTags) && (Query.IsEmpty() || Query.Matches(ItemTags));
}


// UGameItemContainerStockRule_Simple
// ----------------------------------

UGameItemContainerStockRule_Simple::UGameItemContainerStockRule_Simple()
{
}

int32 UGameItemContainerStockRule_Simple::GetItemMaxCount_Implementation(const UGameItem* Item) const
{
	return StockRules.bLimitCount ? StockRules.MaxCount : -1;
}

int32 UGameItemContainerStockRule_Simple::GetItemStackMaxCount_Implementation(const UGameItem* Item) const
{
	return StockRules.bLimitStackCount ? StockRules.StackMaxCount : -1;
}


// UGameItemContainerStockRule_Tags
// --------------------------------

UGameItemContainerStockRule_Tags::UGameItemContainerStockRule_Tags()
{
}

FGameItemStockRules UGameItemContainerStockRule_Tags::GetStockRulesForItem(const UGameItem* Item) const
{
	const UGameItemDef* ItemDefCDO = Item ? Item->GetItemDefCDO() : nullptr;
	if (!ItemDefCDO)
	{
		return FGameItemStockRules();
	}

	for (FGameplayTag ItemTag : ItemDefCDO->OwnedTags)
	{
		if (StockRules.Contains(ItemTag))
		{
			return StockRules[ItemTag];
		}
	}
	return FGameItemStockRules();
}

int32 UGameItemContainerStockRule_Tags::GetItemMaxCount_Implementation(const UGameItem* Item) const
{
	const FGameItemStockRules ItemStockRules = GetStockRulesForItem(Item);
	return ItemStockRules.bLimitCount ? ItemStockRules.MaxCount : -1;
}

int32 UGameItemContainerStockRule_Tags::GetItemStackMaxCount_Implementation(const UGameItem* Item) const
{
	const FGameItemStockRules ItemStockRules = GetStockRulesForItem(Item);
	return ItemStockRules.bLimitStackCount ? ItemStockRules.StackMaxCount : -1;
}


// UGameItemContainerRule_AutoSlot
// -------------------------------

void UGameItemContainerRule_AutoSlot::Initialize()
{
	Super::Initialize();

	Container->OnItemAddedEvent.AddUObject(this, &UGameItemContainerRule_AutoSlot::OnItemAdded);
}

void UGameItemContainerRule_AutoSlot::Uninitialize()
{
	Super::Uninitialize();

	Container->OnItemAddedEvent.RemoveAll(this);
}

bool UGameItemContainerRule_AutoSlot::ShouldAutoSlot_Implementation(UGameItem* Item) const
{
	return true;
}

void UGameItemContainerRule_AutoSlot::OnItemAdded(UGameItem* Item)
{
	if (!ShouldAutoSlot(Item))
	{
		return;
	}

	const UGameItemContainerComponent* ContainerComp = Container->GetTypedOuter<UGameItemContainerComponent>();
	if (!ContainerComp)
	{
		UE_LOG(LogGameItems, Error, TEXT("UGameItemContainerRule_AutoSlot requires a Container that is part of a UGameItemContainerComponent"));
		return;
	}

	ContainerComp->TryAutoSlotItem(Item, ContextTags);
}
