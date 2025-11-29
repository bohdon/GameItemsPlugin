// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Templates/SubclassOf.h"
#include "GameItemTypes.generated.h"


class UGameItem;
class UGameItemContainer;
class UGameItemContainerDef;
class UGameItemContainerLink;
class UGameItemDef;
struct FGameItemList;
struct FGameItemTagStackContainer;


/**
 * The network execution policy for a game item container.
 * Determines how clients and server have control over updating a container.
 */
UENUM(BlueprintType)
enum class EGameItemContainerNetExecutionPolicy : uint8
{
	/** Execute changes immediately on autonomous clients, and trigger the same changes on the server. */
	LocalPredicted,

	/** Only execute changes on the server and replicate to clients. */
	ServerInitiated,
};


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
		: ItemDef(InItemDef)
		, Count(InCount)
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
		: Tag(InTag)
		, Count(InCount)
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

	UPROPERTY(SaveGame)
	FGameplayTag Tag;

	UPROPERTY(SaveGame)
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
	UPROPERTY(SaveGame)
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
USTRUCT()
struct GAMEITEMS_API FGameItemListEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FGameItemListEntry()
	{
	}

	FGameItemListEntry(UGameItem* InItem, int32 InSlot)
		: Item(InItem)
		, Slot(InSlot)
	{
	}

	// FFastArraySerializerItem
	FString GetDebugString() const;

	/**
	 * The item in this entry. Will always be set once added, since
	 * entries are fully removed when an item is removed from a slot.
	 */
	UPROPERTY()
	TObjectPtr<UGameItem> Item;

	/** The slot index of this entry, since item list order is unstable. */
	UPROPERTY()
	int32 Slot = INDEX_NONE;

	/** The last known slot of this entry, before the latest replicated update. */
	UPROPERTY(NotReplicated)
	int32 LastKnownSlot = INDEX_NONE;
};


/**
 * List of game items for use in a container.
 */
USTRUCT()
struct GAMEITEMS_API FGameItemList : public FFastArraySerializer
{
	GENERATED_BODY()

	FGameItemList()
	{
	}

	// FFastArraySerializer
	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);
	void PostReplicatedReceive(const FPostReplicatedReceiveParameters& Parameters);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FGameItemListEntry, FGameItemList>(Entries, DeltaParms, *this);
	}

	void PostSerialize(const FArchive& Ar);

	/** Add an item/stack to the list at a specific index, expanding the list size as needed. */
	void AddEntryForSlot(UGameItem* Item, int32 Slot);

	/** Remove an item/stack from the list. */
	void RemoveEntry(UGameItem* Item);

	/** Remove an entry from the list for a slot, optionally updating items in higher slots to remove gaps. */
	UGameItem* RemoveEntryForSlot(int32 Slot, bool bCollapseSlots = false);

	/** Return the item in a slot. */
	UGameItem* GetItemInSlot(int32 Slot) const;

	/** Return true if an item exists in a slot. */
	bool HasItemInSlot(int32 Slot) const;

	/** Clear all entries. */
	void Reset();

	/** Swap the slot of two entries. Returns true if any entries were changed. */
	bool SwapEntries(int32 SlotA, int32 SlotB);

	/** Return all items in the list, mapped by slot. */
	void GetAllItems(TMap<int32, UGameItem*>& OutItems) const;

	/** Return slots with items, sorted. */
	void GetAllSlots(TArray<int32>& OutSlots) const;

	/** Return all item entries. Remember that index and order of this array is unstable. */
	FORCEINLINE const TArray<FGameItemListEntry>& GetEntries() const { return Entries; }

public:
	struct GAMEITEMS_API FChange
	{
		FChange()
		{
		}

		FChange(const FGameItemListEntry& InEntry, bool bInIsRemoved)
			: Item(InEntry.Item)
			, Slot(InEntry.Slot)
			, LastKnownSlot(InEntry.LastKnownSlot)
			, bIsRemoved(bInIsRemoved)
		{
		}

		FString GetDebugString() const;

		/** A copy of the entry that was changed. */
		UGameItem* Item = nullptr;

		int32 Slot = INDEX_NONE;

		int32 LastKnownSlot = INDEX_NONE;

		bool bIsRemoved = false;
	};

protected:
	/** Replicated list of items and their stack counts. */
	UPROPERTY()
	TArray<FGameItemListEntry> Entries;

	TArray<FChange> PendingChanges;

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FPostReplicateChangesDelegate, const TArray<FChange>& /*Changes*/);

	FPostReplicateChangesDelegate OnPostReplicateChangesEvent;
};


template <>
struct TStructOpsTypeTraits<FGameItemList> : public TStructOpsTypeTraitsBase2<FGameItemList>
{
	enum
	{
		WithNetDeltaSerializer = true,
		WithPostSerialize = true,
	};
};


/**
 * Defines a container.
 */
USTRUCT(BlueprintType)
struct GAMEITEMS_API FGameItemContainerSpec
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (GameplayTagFilter="GameItemContainerIdTagsCategory"))
	FGameplayTag ContainerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameItemContainerDef> ContainerDef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	bool IsValid() const;
};


/**
 * Defines a container link to add to all matching containers.
 */
USTRUCT(BlueprintType)
struct GAMEITEMS_API FGameItemContainerLinkSpec
{
	GENERATED_BODY()

	/** The container link class to create for each matching container. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContainerLink")
	TSubclassOf<UGameItemContainerLink> ContainerLinkClass;

	/** The container to link with. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContainerLink", meta = (GameplayTagFilter="GameItemContainerIdTagsCategory"))
	FGameplayTag LinkedContainerId;

	/** Apply this link to all containers matching this query. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ContainerLink", meta = (GameplayTagFilter="GameItemContainerTagsCategory"))
	FGameplayTagQuery ContainerQuery;

	bool IsValid() const;
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
	UPROPERTY(SaveGame)
	FGuid Guid;

	/** The item definition class. */
	UPROPERTY(SaveGame)
	TSoftClassPtr<UGameItemDef> ItemDef;

	/** The item's serialized SaveGame properties. */
	UPROPERTY(SaveGame)
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
	UPROPERTY(SaveGame)
	TMap<int32, FGameItemSaveData> ItemList;

	/** The container's serialized SaveGame properties. */
	UPROPERTY(SaveGame)
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
	UPROPERTY(SaveGame)
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
	UPROPERTY(SaveGame)
	TMap<FName, FGameItemContainerCollectionSaveData> PlayerItemData;

	/** Save data for all world item containers. */
	UPROPERTY(SaveGame)
	TMap<FName, FGameItemContainerCollectionSaveData> WorldItemData;
};
