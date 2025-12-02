// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameEquipmentTypes.h"
#include "GameItemTypes.h"
#include "Templates/SubclassOf.h"
#include "UObject/Object.h"
#include "GameEquipment.generated.h"

class UGameEquipmentComponent;
class UGameEquipmentDef;


/**
 * Represents gear or other gameplay functionality that gets instanced and applied to an actor.
 */
UCLASS(BlueprintType, Blueprintable)
class GAMEITEMS_API UGameEquipment : public UObject
{
	GENERATED_BODY()

public:
	UGameEquipment(const FObjectInitializer& ObjectInitializer);

	virtual UWorld* GetWorld() const override;
	virtual bool IsSupportedForNetworking() const override { return true; }

	virtual FString GetReadableName() const;

	/** Set the spec of this equipment, which contains it's definition and other stats. */
	void SetEquipmentSpec(const FGameEquipmentSpec& InSpec);

	const FGameEquipmentSpec& GetEquipmentSpec() const { return EquipmentSpec; }

	/** Return the definition of this equipment. */
	TSubclassOf<UGameEquipmentDef> GetEquipmentDef() const { return EquipmentSpec.EquipmentDef; }

	/** Return the class default object for the definition of this equipment. */
	const UGameEquipmentDef* GetEquipmentDefCDO() const;

	/** Return the equipment definition class default object cast to a type. */
	template <class T>
	const T* GetEquipmentDefCDO() const
	{
		return Cast<T>(GetEquipmentDefCDO());
	}

	const FGameItemTagStackContainer& GetTagStats() const { return EquipmentSpec.TagStats; }

	/** Return the owning equipment component. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	virtual UGameEquipmentComponent* GetOwner() const;

	/** Return the owning Actor of this equipment. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	virtual AActor* GetOwningActor() const;

	/** Return all actors that this equipment has spawned. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Equipment")
	TArray<AActor*> GetSpawnedActors() const;

	/** Return the first spawned actor of a specific class. */
	UFUNCTION(BlueprintPure, Meta = (DeterminesOutputType = "ActorClass"), Category = "Equipment")
	AActor* GetSpawnedActorOfClass(TSubclassOf<AActor> ActorClass) const;

	/** Activate this equipment (if not already). */
	virtual void Equip();

	/** Deactivate this equipment. */
	virtual void Unequip();

	/** Return the component that should be used as the attach parent for spawned actors. */
	virtual USceneComponent* GetTargetAttachComponent() const;

protected:
	/** The specification used to create this equipment. */
	UPROPERTY(Replicated, BlueprintReadOnly, meta = (AllowPrivateAccess))
	FGameEquipmentSpec EquipmentSpec;

	/** The actors this equipment has spawned. */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_SpawnedActors)
	TArray<TObjectPtr<AActor>> SpawnedActors;

	UFUNCTION()
	void OnRep_SpawnedActors();

	/** The unreplicated actors this equipment has spawned. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> LocalSpawnedActors;

	/** Is this equipment currently active? True after calling Equip. */
	bool bIsEquipped = false;

	/** Called when the equipment has been applied to an actor. */
	virtual void OnEquipped();

	/** Called when the equipment is removed from an actor. */
	virtual void OnUnequipped();

	/** Called when the equipment is added to an actor. */
	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "OnEquipped"), Category = "Equipment")
	void OnEquipped_BP();

	/** Called when the equipment has been removed from an actor. */
	UFUNCTION(BlueprintImplementableEvent, Meta = (DisplayName = "OnUnequipped"), Category = "Equipment")
	void OnUnequipped_BP();

	/** Spawn all actors for this equipment. */
	virtual void SpawnEquipmentActors();

	/** Destroy all actors that this equipment has spawned. */
	virtual void DestroyEquipmentActors();
};
