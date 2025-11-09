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
 * Defines limitations for the quantity of an item.
 */
USTRUCT(BlueprintType)
struct GAMEITEMS_API FGameItemCountLimit
{
	GENERATED_BODY()

	FGameItemCountLimit()
	{
	}

	/** Limit the count of this item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (InlineEditConditionToggle), Category = "GameItem")
	bool bLimitCount = false;

	/** The maximum quantity allowed for the item. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (EditCondition="bLimitCount", ClampMin = 0), Category = "GameItem")
	int32 MaxCount = 1;

	/** Return the max count, or default value if not limited. */
	int32 GetMaxCount(int32 Default = MAX_int32) const;
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

	FGameItemDefStack(const TSubclassOf<UGameItemDef>& InItemDef, const int32& InCount)
		: ItemDef(InItemDef), Count(InCount)
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

	// FFastArraySerializerItem
	FString GetDebugString() const;

	bool operator==(const FGameItemTagStack& Other) const
	{
		return Tag == Other.Tag && Count == Other.Count;
	}

	bool operator!=(const FGameItemTagStack& Other) const
	{
		return Tag != Other.Tag || Count != Other.Count;
	}

	UPROPERTY()
	FGameplayTag Tag;

	UPROPERTY()
	int32 Count = 0;
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

	// FFastArraySerializerItem
	FString GetDebugString() const;

	/** The item in this entry. */
	UPROPERTY()
	TObjectPtr<UGameItem> Item = nullptr;
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

	void PostSerialize(const FArchive& Ar);

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

	/** Return all item entries. */
	FORCEINLINE const TArray<FGameItemListEntry>& GetEntries() const { return Entries; }

	/** Clear all entries. */
	void Reset();

	/** Swap the location of two entries, expanding the array size if needed. */
	void SwapEntries(int32 IndexA, int32 IndexB);

	void GetAllItems(TArray<UGameItem*>& OutItems) const;

	DECLARE_MULTICAST_DELEGATE_ThreeParams(FGameItemListNewOrRemovedDelegate, FGameItemListEntry& /*Entry*/, int32 /*Slot*/, bool /*bAdded*/);

	/** Called when any item is added or removed. */
	FGameItemListNewOrRemovedDelegate OnItemAddedOrRemovedEvent;

protected:
	/** Replicated list of items and their stack counts. */
	UPROPERTY()
	TArray<FGameItemListEntry> Entries;
};


template <>
struct TStructOpsTypeTraits<FGameItemList> : public TStructOpsTypeTraitsBase2<FGameItemList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};


/**
 * Save data for a single game item.
 */
USTRUCT()
struct GAMEITEMS_API FGameItemSaveData
{
	GENERATED_BODY()

	FGameItemSaveData();

	/** Create save data from an item. */
	FGameItemSaveData(UGameItem* InItem);

	/** Create save data using only a guid, pointing to an item in a parent container. */
	FGameItemSaveData(const FGuid& InGuid);

	/** The unique id of this item. */
	UPROPERTY()
	FGuid Guid;

	/** The item definition class. */
	UPROPERTY()
	TSoftClassPtr<UGameItemDef> ItemDef;

	/** The item's serialized SaveGame properties. */
	UPROPERTY()
	TArray<uint8> ByteData;
};


/**
 * Save data for a single game item container.
 */
USTRUCT()
struct GAMEITEMS_API FGameItemContainerSaveData
{
	GENERATED_BODY()

	/** All items in the container by slot */
	UPROPERTY()
	TMap<int32, FGameItemSaveData> ItemList;

	/** The container's serialized SaveGame properties. */
	UPROPERTY()
	TArray<uint8> ByteData;
};


/**
 * Save data for a collection of related containers.
 */
USTRUCT()
struct GAMEITEMS_API FGameItemContainerCollectionSaveData
{
	GENERATED_BODY()

	/** Save data for all containers by id. */
	UPROPERTY()
	TMap<FGameplayTag, FGameItemContainerSaveData> Containers;
};


/**
 * Contains all save data needed to store both player and world game
 * items and containers. Player data is stored separately to avoid possible
 * conflicts with world data.
 */
USTRUCT()
struct GAMEITEMS_API FPlayerAndWorldGameItemSaveData
{
	GENERATED_BODY()

	/** Save data for all the player's item collections. */
	UPROPERTY()
	TMap<FName, FGameItemContainerCollectionSaveData> PlayerItemData;

	/** Save data for all world item containers. */
	UPROPERTY()
	TMap<FName, FGameItemContainerCollectionSaveData> WorldItemData;
};
