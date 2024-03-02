// Copyright Bohdon Sayre, All Rights Reserved.


#include "Equipment/GameItemFragment_Equipment.h"

#include "Conditions/GameItemConditionSchema.h"


UGameItemFragment_Equipment::UGameItemFragment_Equipment(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Condition.SetSchemaClass(UGameItemConditionSchema::StaticClass());
}
