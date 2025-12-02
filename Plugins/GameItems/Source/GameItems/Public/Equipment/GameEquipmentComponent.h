// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameEquipmentTypes.h"
#include "Components/ActorComponent.h"
#include "GameEquipmentComponent.generated.h"

class UGameEquipment;
class UGameEquipmentDef;


/**
 * Handles adding and removing game equipment on the owning actor.
 */
UCLASS(Meta = (BlueprintSpawnableComponent))
class GAMEITEMS_API UGameEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGameEquipmentComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void InitializeComponent() override;
	virtual void UninitializeComponent() override;
	virtual void ReadyForReplication() override;
	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;
	virtual void OnRep_IsActive() override;

	/** Create and apply equipment. */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void ApplyEquipment(TSubclassOf<UGameEquipmentDef> EquipmentDef);

	/** Create and apply equipment using a spec, which allows setting dynamic stats for this instance. */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void ApplyEquipmentSpec(const FGameEquipmentSpec& EquipmentSpec);

	/** Remove equipment by definition. */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void RemoveEquipmentByDef(TSubclassOf<UGameEquipmentDef> EquipmentDef);

	/** Remove an equipment instance. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	virtual void RemoveEquipment(UGameEquipment* Equipment);

	/** Remove all applied equipment from the owning actor. */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void RemoveAllEquipment();

	/** Return the first instance of applied equipment by class. */
	UFUNCTION(BlueprintPure, Meta = (DeterminesOutputType = EquipmentClass), Category = "Equipment")
	UGameEquipment* FindEquipment(TSubclassOf<UGameEquipment> EquipmentClass) const;

	template <class T>
	T* FindEquipment()
	{
		static_assert(TIsDerivedFrom<T, UGameEquipment>::IsDerived, TEXT("T must derive from UGameEquipment"));
		return Cast<T>(FindEquipment(T::StaticClass()));
	}

	/** Find applied equipment by definition. */
	UFUNCTION(BlueprintPure, Category = "Equipment")
	UGameEquipment* FindEquipmentByDef(TSubclassOf<UGameEquipmentDef> EquipmentDef) const;

	/** Return all instances of applied equipment by class. */
	UFUNCTION(BlueprintPure, Meta = (DeterminesOutputType = EquipmentClass), Category = "Equipment")
	TArray<UGameEquipment*> FindAllEquipment(TSubclassOf<UGameEquipment> EquipmentClass) const;

	/** Return all equipment instances. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Equipment")
	TArray<UGameEquipment*> GetAllEquipment() const;

	FString GetDebugPrefix() const;

protected:
	UFUNCTION(Server, Reliable)
	void ServerApplyEquipmentSpec(const FGameEquipmentSpec& EquipmentSpec);

	UFUNCTION(Server, Reliable)
	void ServerRemoveEquipmentByDef(TSubclassOf<UGameEquipmentDef> EquipmentDef);

protected:
	virtual void OnEquipmentApplied(UGameEquipment* Equipment);
	virtual void OnEquipmentRemoved(UGameEquipment* Equipment);

	/** Return whether we should apply/remove equipment via Server rpc and/or locally. */
	virtual void GetNetExecutionPlan(bool& bOutExecuteServer, bool& bOutExecuteLocal) const;

	void OnPreReplicatedRemove(FGameEquipmentListEntry& Entry);
	void OnPostReplicatedAdd(FGameEquipmentListEntry& Entry);
	void OnPostReplicatedChange(FGameEquipmentListEntry& Entry);

protected:
	UPROPERTY(Transient, Replicated)
	FGameEquipmentList EquipmentList;
};
