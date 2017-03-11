// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class HexCoordinate {};
#include "Unit.h"

/**
 * 
 */
class PRETENDEMPYRES_API Army
{
public:
	static float DamagePunchThroughThreshhold;
	static float MinimumDamagePunchedThrough;

public:
	Army();
	~Army();

	void AddExperience(uint32 amount);
	bool Attack(Army* defendingArmy);

private:
	
	bool GetDunked(uint32& outDamage, uint32& outExperience, uint32 damage);
	bool DamageUnits(uint32& outXP, uint32 damage);
	uint32 SumDamage();
   Unit* GetUnit(UnitGUID guid);
   void RegisterUnitWithArmy(Unit* unit);
	void DeregisterUnitWithArmy(Unit* unit);

private: // Internal calls for this army only
	void TransferUnitOut(UnitGUID guid, uint32 quantity, Army* gainingArmy);
	void AddUnits(UnitStackType unitType, uint32 quantity, uint32 xpOfNewUnits);
   uint32 RemoveUnits(Unit* pUnit, uint32 quantity);
   Unit* AllocateAndRegisterUnit(UnitStackType type, uint32 quantity);
   void DeleteAndDeregisterUnit(Unit* pUnit);

private:
	typedef TDoubleLinkedList<Unit*> ListOfUnits;
	typedef TMap<UnitGUID, Unit*> MapOfUnitGUIDs;
	TMap<UnitGUID, Unit*> m_unitsByGuid;
	TMap<UnitType, ListOfUnits* > m_unitsByUnitType;
	TMap<uint32 /*rank*/, MapOfUnitGUIDs* > m_unitsByRankByGuid;
	TMap<UnitGUID, Unit*> m_herosByGuid;
	TMap<UnitGUID, Unit*> m_regularsByGuid;
	Unit* m_leadUnit;
	uint32 m_unawardedXP;

	Army(const Army&);
};
