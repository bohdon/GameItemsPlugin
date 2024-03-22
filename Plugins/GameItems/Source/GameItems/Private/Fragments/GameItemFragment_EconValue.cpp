// Copyright Bohdon Sayre, All Rights Reserved.


#include "Fragments/GameItemFragment_EconValue.h"


int32 UGameItemFragment_EconValue::GetCountForValue(float TargetValue, int32 MinCount) const
{
	return FMath::Max(FMath::RoundFromZero(TargetValue / EconValue), MinCount);
}
