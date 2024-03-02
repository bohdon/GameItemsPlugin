// Copyright Bohdon Sayre, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WorldConditionSchema.h"
#include "GameItemConditionSchema.generated.h"


UCLASS()
class GAMEITEMS_API UGameItemConditionSchema : public UWorldConditionSchema
{
	GENERATED_BODY()

public:
	UGameItemConditionSchema(const FObjectInitializer& ObjectInitializer);

	/** Context data reference for accessing a target actor. */
	FWorldConditionContextDataRef GetTargetActorRef() const { return TargetActorRef; }

	/** Context data reference for accessing a target container. */
	FWorldConditionContextDataRef GetTargetContainerRef() const { return TargetContainerRef; }

	/** Context data reference for accessing a target item. */
	FWorldConditionContextDataRef GetTargetItemRef() const { return TargetItemRef; }

	/** Context data reference for accessing the GameItemSubsystem. */
	FWorldConditionContextDataRef GetSubsystemRef() const { return SubsystemRef; }

	virtual bool IsStructAllowed(const UScriptStruct* InScriptStruct) const override;

private:
	FWorldConditionContextDataRef TargetActorRef;
	FWorldConditionContextDataRef TargetContainerRef;
	FWorldConditionContextDataRef TargetItemRef;
	FWorldConditionContextDataRef SubsystemRef;
};
