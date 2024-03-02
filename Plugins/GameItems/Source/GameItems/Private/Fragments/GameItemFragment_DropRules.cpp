// Copyright Bohdon Sayre, All Rights Reserved.


#include "Fragments/GameItemFragment_DropRules.h"

#include "Conditions/GameItemConditionSchema.h"


UGameItemFragment_DropRules::UGameItemFragment_DropRules(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	  Rate(1.f)
{
	Condition.SetSchemaClass(UGameItemConditionSchema::StaticClass());
}
