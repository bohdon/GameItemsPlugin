// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "GameplayTagContainer.h"
#include "GameItemTypes.generated.h"


struct FGameItemTagStackContainer;


/**
 * Represents a single gameplay tag and a count.
 */
USTRUCT(BlueprintType)
struct GAMEITEMS_API FGameItemTagStack : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FGameItemTagStack()
		: Count(0)
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
	friend FGameItemTagStackContainer;

	UPROPERTY()
	FGameplayTag Tag;

	UPROPERTY()
	int32 Count;
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

	// FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	// End of FFastArraySerializer contract

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
