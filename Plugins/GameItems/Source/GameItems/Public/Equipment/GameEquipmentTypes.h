// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Templates/SubclassOf.h"
#include "GameEquipmentTypes.generated.h"

class UGameEquipmentComponent;
class UGameEquipment;
struct FGameEquipmentList;


/**
 * Determines how equipment actors should be spawned.
 */
UENUM(BlueprintType)
enum class EGameEquipmentActorSpawnPolicy : uint8
{
	/** Spawn the equipment on the local owner only. */
	LocalOnly,

	/** Spawn the equipment on the server, and replicate to clients. */
	ServerInitiated,
};


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

	/** How the equipment actor should be spawned in networked games. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EGameEquipmentActorSpawnPolicy NetSpawnPolicy = EGameEquipmentActorSpawnPolicy::ServerInitiated;
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

	FGameEquipmentListEntry(UGameEquipment* InEquipment)
		: Equipment(InEquipment)
	{
	}

	// FFastArraySerializerItem
	FString GetDebugString() const;

	/** The equipment instance. */
	UPROPERTY()
	TObjectPtr<UGameEquipment> Equipment = nullptr;
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
	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FGameEquipmentListEntry, FGameEquipmentList>(Entries, DeltaParms, *this);
	}

	void AddEntry(UGameEquipment* Equipment);
	void RemoveEntry(UGameEquipment* Equipment);

	const TArray<FGameEquipmentListEntry>& GetEntries() const { return Entries; }

protected:
	/** Replicated list of equipment entries */
	UPROPERTY()
	TArray<FGameEquipmentListEntry> Entries;

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FGameEquipmentListReplicateDelegate, FGameEquipmentListEntry& /*Entry*/);

	FGameEquipmentListReplicateDelegate OnPreReplicatedRemoveEvent;
	FGameEquipmentListReplicateDelegate OnPostReplicatedAddEvent;
	FGameEquipmentListReplicateDelegate OnPostReplicatedChangeEvent;
};

template <>
struct TStructOpsTypeTraits<FGameEquipmentList> : public TStructOpsTypeTraitsBase2<FGameEquipmentList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
