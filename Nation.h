// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * 
 */
class PRETENDEMPYRES_API Nation
{
public:
	Nation(FString name);
	~Nation();

	uint32 GetNation_ID();

private:
	uint32 nation_ID;
	FString nationName;
};
