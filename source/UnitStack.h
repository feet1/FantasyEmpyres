// Fill out your copyright notice in the Description page of Project Settings.
/**
#pragma once



class PRETENDEMPYRES_API UnitStack
{
public:

	UnitStack(UnitType _type, uint32 _quantity);
	~UnitStack();
	UnitType GetUnitType() const;
	uint32 GetUpkeep() const;
	uint32 GetDamage() const;
	uint32 GetHP() const;
	void ReceiveDamage(uint32 _damage);
	void ModifyStack(int32 _quantity);
	void GrantExperience(uint32 _xp);
	bool IsHero() const;
	void SetUnitType(UnitType type);

private:
	uint32 quantity;
	uint32 experience;
	uint32 hitpointMax;
	uint32 hitpoints;
	UnitType unitType;


};
