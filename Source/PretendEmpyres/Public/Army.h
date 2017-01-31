// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class HexCoordinate {};

/**
 * 
 */
class PRETENDEMPYRES_API Army
{
public:
	Army(FString& _armyName, const int32 _identity);
	~Army();
	void AddExperience(uint32 amount);
	bool BeginAttack(Army* opponent);
	void AddRegulars(UnitType unitType, uint32 quantity);

protected:
	
	bool GetDunked(uint32& outDamage, uint32& outExperience, uint32 damage);


private:
	TMap<uint32, Unit*> UnitsByGuid;
	TMap<UnitType, TDoubleLinkedList<Unit*>* > UnitsByUnitType;
	TMap<uint32, Unit*> HerosByGuid;
	TMap<uint32, Unit*> RegularsByGuid;
	TMap<uint32 /*rank*/, TMap<uint32, Unit*>> UnitsByRankByGuid;
	Unit* Leader;
	HexCoordinate location;
	HexCoordinate destination;
	FString Name;
	uint32 Player_ID;
	uint32 unawardedXP;
	FString armyName;
	bool DamageUnits(uint32& outXP, uint32 damage);
	uint32 SumDamage();
	void DeleteUnit(uint32 guid);
	uint32 RemoveRegulars(uint32 guid, uint32 quantity);
	void TransferUnit(uint32 _guid, uint32 _quantity, Army* gainingArmy);
	void TransferHero(Unit* hero, Army* gainingArmy);
	Unit* AllocateUnit(UnitType _type, uint32 _quantity);
	Unit* GetUnitPointerByUnitType(UnitType type);
	void RegisterUnitWithArmy(Unit* unit);
	void DeregisterUnitWithArmy(Unit* unit);
public:
	static float DamagePunchThroughThreshhold;
	static float MinimumDamagePunchedThrough;
};
