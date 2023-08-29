// Fill out your copyright notice in the Description page of Project Settings.


#include "Fragments/GameItemFragment_TagStats.h"

#include "GameItem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameItemFragment_TagStats)


void UGameItemFragment_TagStats::OnItemCreated(UGameItem* Item) const
{
	for (const auto& KeyValue : DefaultStats)
	{
		Item->AddTagStat(KeyValue.Key, KeyValue.Value);
	}
}
