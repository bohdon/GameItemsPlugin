// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Templates/SubclassOf.h"
#include "GameItemTypes.generated.h"


class UGameItem;
class UGameItemContainer;
class UGameItemDef;
struct FGameItemList;
struct FGameItemTagStackContainer;


/**
 * Defines limitations for the quantity and stacking of an item.
 */
USTRUCT(BlueprintType)
struct FGameItemStockRules
{
	GENERATED_BODY()

	FGameItemStockRules()
	{
	}

	/** Limit the maximum count of this item that can be in a container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameItem")
	bool bLimitCount = false;

	/** The total maximum quantity allowed for the item in a container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EditCondition="bLimitCount", ClampMin = 0), Category = "GameItem")
	int32 MaxCount = 1;

	/** Limit the maximum count for a single stack of the item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameItem")
	bool bLimitStackCount = false;

	/** The maximum count for a single stack of the item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EditCondition="bLimitStackCount", ClampMin = 1), Category = "GameItem")
	int32 StackMaxCount = 1;
};


/**
 * Represents a game item definition and quantity, e.g. for use
 * when creating new items or defining default container inventories.
 */
USTRUCT(BlueprintType)
struct GAMEITEMS_API FGameItemDefStack
{
	GENERATED_BODY()

	FGameItemDefStack()
	{
	}

	/** The item definition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameItemDef> ItemDef = nullptr;

	/** The quantity of the item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 1;
};


/**
 * Represents a single gameplay tag and a count.
 */
USTRUCT(BlueprintType)
struct GAMEITEMS_API FGameItemTagStack : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FGameItemTagStack()
	{
	}

	FGameItemTagStack(FGameplayTag InTag, int32 InCount)
		: Tag(InTag),
		  Count(InCount)
	{
	}

	FString ToDebugString() const;

	bool operator==(const FGameItemTagStack& Other) const
	{
		return Tag == Other.Tag && Count == Other.Count;
	}

	bool operator!=(const FGameItemTagStack& Other) const
	{
		return Tag != Other.Tag || Count != Other.Count;
	}

private:
	UPROPERTY()
	FGameplayTag Tag;

	UPROPERTY()
	int32 Count = 0;

	friend FGameItemTagStackContainer;
};


/**
 * Container of game item tag stacks, designed for fast replication.
 */
USTRUCT(BlueprintType)
struct GAMEITEMS_API FGameItemTagStackContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	FGameItemTagStackContainer()
	{
	}

	/** Add stacks to a tag. */
	void AddStack(FGameplayTag Tag, int32 DeltaCount);

	/** Remove stacks from a tag. */
	void RemoveStack(FGameplayTag Tag, int32 DeltaCount);

	/** Return the stack count for a tag, or 0 if the tag is not present. */
	int32 GetStackCount(FGameplayTag Tag) const
	{
		return StackCountMap.FindRef(Tag);
	}

	/** Return true if there is at least one stack of a tag. */
	bool ContainsTag(FGameplayTag Tag) const
	{
		return StackCountMap.Contains(Tag);
	}

	// FFastArraySerializer
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FastArrayDeltaSerialize<FGameItemTagStack, FGameItemTagStackContainer>(Stacks, DeltaParms, *this);
	}

	void PostSerialize(const FArchive& Ar);

	FString ToDebugString() const;

	bool operator ==(const FGameItemTagStackContainer& Other) const
	{
		return Stacks == Other.Stacks;
	}

	bool operator !=(const FGameItemTagStackContainer& Other) const
	{
		return !(*this == Other);
	}

private:
	/** Replicated array of gameplay tag stacks. */
	UPROPERTY()
	TArray<FGameItemTagStack> Stacks;

	/** Cached map of stack counts by tag, for faster lookup. */
	TMap<FGameplayTag, int32> StackCountMap;
};

template <>
struct TStructOpsTypeTraits<FGameItemTagStackContainer> : public TStructOpsTypeTraitsBase2<FGameItemTagStackContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
		WithPostSerialize = true,
	};
};


/**
 * A single item or item stack in a list of items.
 */
USTRUCT(BlueprintType)
struct GAMEITEMS_API FGameItemListEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FGameItemListEntry()
	{
	}

	FString ToDebugString() const;

	UGameItem* GetItem() const { return Item; }

private:
	/** The item in this entry. */
	UPROPERTY()
	TObjectPtr<UGameItem> Item = nullptr;

	friend FGameItemList;
};


/**
 * List of game items for use in a container.
 */
USTRUCT(BlueprintType)
struct GAMEITEMS_API FGameItemList : public FFastArraySerializer
{
	GENERATED_BODY()

	FGameItemList()
	{
	}

	// FFastArraySerializer
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FGameItemListEntry, FGameItemList>(Entries, DeltaParms, *this);
	}

	/** Add an item/stack to the list. */
	void AddEntry(UGameItem* Item);

	/** Add an item/stack to the list at a specific index, expanding the list size as needed. */
	void AddEntryAt(UGameItem* Item, int32 Index);

	/** Remove an item/stack from the list. */
	void RemoveEntry(UGameItem* Item);

	/**
	 * Remove an item/stack from a specific index in the list.
	 * @param bPreserveIndices Simply null out the item instead of removing it from the list, which would affect all item indices following it.
	 */
	UGameItem* RemoveEntryAt(int32 Index, bool bPreserveIndices = false);

	void GetAllItems(TArray<UGameItem*>& OutItems) const;

	DECLARE_MULTICAST_DELEGATE_ThreeParams(FGameItemListChangedDelegate, FGameItemListEntry& /*Entry*/, int32 /*NewCount*/, int32 /*OldCount*/);

	/** Called when any item is added or removed. */
	FGameItemListChangedDelegate OnListChangedEvent;

private:
	/** Replicated list of items and their stack counts. */
	UPROPERTY()
	TArray<FGameItemListEntry> Entries;

	friend UGameItemContainer;
};

template <>
struct TStructOpsTypeTraits<FGameItemList> : public TStructOpsTypeTraitsBase2<FGameItemList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
