// Fill out your copyright notice in the Description page of Project Settings.

#include "PretendEmpyres.h"
#include "Nation.h"

namespace
{
	uint32 GetNewNation_ID()
	{
		static uint32 nextNation_ID = 0;
		return nextNation_ID++;
	}
}

Nation::Nation(FString name = "")
	:nation_ID(GetNation_ID())
	, nationName(name)
{
}

Nation::~Nation()
{
}

uint32 Nation::GetNation_ID()
{
	return nation_ID;
}