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

	virtual void UninitializeComponent() override;
	virtual void ReadyForReplication() override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	/** Apply equipment to the owning actor. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	virtual UGameEquipment* ApplyEquipment(TSubclassOf<UGameEquipmentDef> EquipmentDef, UObject* Instigator = nullptr);

	/** Remove equipment from the owning actor. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	virtual void RemoveEquipment(UGameEquipment* Equipment);

	/** Remove all applied equipment from the owning actor. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Equipment")
	virtual void RemoveAllEquipment();

	/** Return the first instance of applied equipment by class. */
	UFUNCTION(BlueprintPure, Meta = (DeterminesOutputType = EquipmentClass), Category = "Equipment")
	UGameEquipment* FindEquipment(TSubclassOf<UGameEquipment> EquipmentClass) const;

	/** Return all instances of applied equipment by class. */
	UFUNCTION(BlueprintPure, Meta = (DeterminesOutputType = EquipmentClass), Category = "Equipment")
	TArray<UGameEquipment*> FindAllEquipment(TSubclassOf<UGameEquipment> EquipmentClass) const;

	template <class T>
	T* FindEquipment()
	{
		return (T*)FindEquipment(T::StaticClass());
	}

protected:
	UPROPERTY(Replicated)
	FGameEquipmentList EquipmentList;
};
