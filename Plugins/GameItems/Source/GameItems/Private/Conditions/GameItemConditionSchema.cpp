// Copyright Bohdon Sayre, All Rights Reserved.


#include "Conditions/GameItemConditionSchema.h"

#include "GameItem.h"
#include "GameItemContainer.h"
#include "GameItemSubsystem.h"
#include "Conditions/GameItemConditionBase.h"
#include "GameFramework/Actor.h"


UGameItemConditionSchema::UGameItemConditionSchema(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TargetActorRef = AddContextDataDesc(TEXT("TargetActor"), AActor::StaticClass(), EWorldConditionContextDataType::Dynamic);
	TargetContainerRef = AddContextDataDesc(TEXT("TargetContainer"), UGameItemContainer::StaticClass(), EWorldConditionContextDataType::Dynamic);
	TargetItemRef = AddContextDataDesc(TEXT("TargetItem"), UGameItem::StaticClass(), EWorldConditionContextDataType::Dynamic);
	SubsystemRef = AddContextDataDesc(TEXT("Subsystem"), UGameItemSubsystem::StaticClass(), EWorldConditionContextDataType::Persistent);
}

bool UGameItemConditionSchema::IsStructAllowed(const UScriptStruct* InScriptStruct) const
{
	check(InScriptStruct);
	return Super::IsStructAllowed(InScriptStruct)
		|| InScriptStruct->IsChildOf(TBaseStructure<FWorldConditionCommonBase>::Get())
		|| InScriptStruct->IsChildOf(TBaseStructure<FWorldConditionCommonActorBase>::Get())
		|| InScriptStruct->IsChildOf(TBaseStructure<FGameItemConditionBase>::Get());
}
