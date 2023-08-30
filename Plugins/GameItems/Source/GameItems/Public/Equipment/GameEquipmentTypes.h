// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "UObject/Object.h"
#include "GameEquipmentTypes.generated.h"

class UGameEquipmentComponent;
class UGameEquipment;
struct FGameEquipmentList;


/**
 * Defines necessary info for spawning an actor from equipment.
 */
USTRUCT(BlueprintType)
struct FGameEquipmentActorSpawnInfo
{
	GENERATED_BODY()

	FGameEquipmentActorSpawnInfo()
	{
	}

	/** The actor class to spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ActorClass;

	/** The socket to attach to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AttachSocket;

	/** The relative transform to set for the actor when attached. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform AttachTransform;
};


/**
 * Represents a single instance of applied equipment.
 */
USTRUCT(BlueprintType)
struct FGameEquipmentListEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FGameEquipmentListEntry()
	{
	}

	FString ToDebugString() const;

	TObjectPtr<UGameEquipment> GetEquipment() const { return Equipment; }

private:
	/** The equipment instance. */
	UPROPERTY()
	TObjectPtr<UGameEquipment> Equipment = nullptr;

	friend FGameEquipmentList;
};


/**
 * List of equipment instances.
 */
USTRUCT(BlueprintType)
struct FGameEquipmentList : public FFastArraySerializer
{
	GENERATED_BODY()

	FGameEquipmentList()
	{
	}

	// FFastArraySerializer
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FGameEquipmentListEntry, FGameEquipmentList>(Entries, DeltaParms, *this);
	}

	void AddEntry(UGameEquipment* Equipment);
	void RemoveEntry(UGameEquipment* Equipment);

private:
	/** Replicated list of equipment entries */
	UPROPERTY()
	TArray<FGameEquipmentListEntry> Entries;

	friend UGameEquipmentComponent;
};

template <>
struct TStructOpsTypeTraits<FGameEquipmentList> : public TStructOpsTypeTraitsBase2<FGameEquipmentList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
