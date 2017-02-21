// Fill out your copyright notice in the Description page of Project Settings.

#include "PretendEmpyres.h"
#include "Unit.h"
#include "Army.h"

float Army::DamagePunchThroughThreshhold = 0.5f;
float Army::MinimumDamagePunchedThrough = 0.2f;

Army::Army()
{
}

Army::~Army()
{
	for (auto const& unit : m_unitsByGuid)
	{
		delete unit.Value;
	}
	m_unitsByGuid.Empty();
	m_unitsByUnitType.Empty();
	m_herosByGuid.Empty();
	m_regularsByGuid.Empty();
	m_unitsByRankByGuid.Empty();
}

void Army::AddExperience(uint32 amount)
{
   //calculate the weight of a single regular and hero unit in the army
   uint32 armySize = m_herosByGuid.Num() * Unit::NumberOfRegularsAHeroIsWorth;
   for (auto it = m_regularsByGuid.CreateIterator(); it; ++it)
   {
      armySize += it->Value->Quantity();
   }
   
   uint32 xpPerRegular = FMath::Max(1u, (uint32)(((float)amount / armySize) + 0.5f));

	for (const auto& pair : m_unitsByGuid)
	{
      Unit* unit = pair.Value;
      //XP is divided per unit type. Large stacks of units get much less XP than small stacks?
      uint32 grantedXP = (unit->IsHero()) ? xpPerRegular * Unit::NumberOfRegularsAHeroIsWorth : xpPerRegular * unit->Quantity();
		unit->GrantExperience(grantedXP);
	}
}

bool Army::Attack(Army* defendingArmy)
{
	//returns true if this army won
	uint32 damageToAttacker;
	uint32 XPAwardedToAttacker;
	uint32 XPAwardedToDefender;
	bool defenderRemains = true;
	bool AttackerRemains = true;

	while (defenderRemains&&AttackerRemains)
	{
		defenderRemains = defendingArmy->GetDunked(damageToAttacker, XPAwardedToAttacker, SumDamage());
		m_unawardedXP += XPAwardedToAttacker;
		AttackerRemains = DamageUnits(XPAwardedToDefender, damageToAttacker);
      defendingArmy->m_unawardedXP += XPAwardedToDefender;
	}
	return AttackerRemains;
}

bool Army::GetDunked(uint32& outDamageToAttacker, uint32& outXPForAttacker, uint32 damageToDefender)
{
	outDamageToAttacker = SumDamage();
	bool DefenderRemains = DamageUnits(outXPForAttacker, damageToDefender);
	return DefenderRemains;
}

bool Army::DamageUnits(uint32& outXP, uint32 damage) 
{
	outXP = 0;
	while(m_unitsByGuid.Num() > 0 && damage > 0)
	{
		for (const auto& rank : m_unitsByRankByGuid)
		{
			uint32 unitsInThisRank = 0;
			uint32 totalHealthOfUnitsInRank = 0;
			for (const auto& unit : *(rank.Value))
			{
				unitsInThisRank += unit.Value->Quantity();
				totalHealthOfUnitsInRank += unit.Value->GetTotalHealth();
			}
			//how much damage these mans take
			if (totalHealthOfUnitsInRank * DamagePunchThroughThreshhold < damage)
			{
				uint32 nonPunchthroughDamage = FMath::Min(totalHealthOfUnitsInRank, (uint32)(damage * (1 - MinimumDamagePunchedThrough)));
				check(nonPunchthroughDamage <= damage);
				damage -= nonPunchthroughDamage;
				for (const auto& unit : *(rank.Value))
				{
					float portionOfThisRank = unit.Value->Quantity() / (float)unitsInThisRank;
					int32 damageToThisUnit = portionOfThisRank * nonPunchthroughDamage;
					uint32 unitsKilled = unit.Value->ModifyHealth(-damageToThisUnit);
					if (unit.Value->Quantity() == 0)
					{
						DeleteAndDeregisterUnit(unit.Value);
					}
					outXP += unit.Value->GetXPValue(false) * unitsKilled;
				}
				if (damage == 0)
				{
					return m_unitsByGuid.Num() != 0;
				}

			}
			//only fires if rank has x2 health as dmg & soaks remainder of damage and returns false
			else
			{
				for (const auto& unit : *(rank.Value))
				{
					float portionOfThisRank = unit.Value->Quantity() / (float)unitsInThisRank;
					int32 damageToThisUnit = portionOfThisRank * damage;
					uint32 unitsKilled = unit.Value->ModifyHealth(-damageToThisUnit);
					outXP += unit.Value->GetXPValue(false) * unitsKilled;
					if (unit.Value->Quantity() == 0)
					{
						DeleteAndDeregisterUnit(unit.Value);
					}
				}
				return false;
			} //only fires if rank has x2 health as dmg //soaks remainder of damage and returns false

		}
	}
	//apply fancy damage formula
	check(m_unitsByGuid.Num() == 0)  //find out how the hell there are any units left
	return false;
}

uint32 Army::SumDamage()
{
	uint32 totalDamage = 0;
	for (const auto& unit : m_unitsByGuid)
	{
		totalDamage += unit.Value->GetAttackDamage();
	}
	return totalDamage;
}

//will be called by GUI to move units between your armies
void Army::TransferUnitOut(UnitGUID guid, uint32 quantity, Army* gainingArmy)
{
   Unit* unit = m_unitsByGuid.FindChecked(guid);
   check(unit != nullptr);

   if (unit->IsHero())
   {
      DeregisterUnitWithArmy(unit);
      gainingArmy->RegisterUnitWithArmy(unit);
      return;
   }
   UnitStackType ust = unit->GetUnitStackType();
   uint32 xp = unit->GetCurrentExperience();
   uint32 troopsMoved = RemoveUnits(unit, quantity);
   check(troopsMoved == quantity);
   gainingArmy->AddUnits( ust, troopsMoved, xp );
}


void Army::AddUnits(UnitStackType stackType, uint32 quantity, uint32 xpOfNewUnits)
{
   check((stackType & Unit::heroMask) == 0);//heros can not be added like this
   UnitType type = stackType & ~Unit::levelMask;

   ListOfUnits*& unitList = m_unitsByUnitType.FindOrAdd(type);
   if (unitList == nullptr)
   {
	   unitList = new TDoubleLinkedList<Unit*>();
   }
   Unit* receivingUnit = nullptr;
   for (auto* unitNode = unitList->GetHead(); unitNode != nullptr; unitNode = unitNode->GetNextNode())
   {
      if (unitNode->GetValue()->GetUnitStackType() == stackType)
      {
         receivingUnit = unitNode->GetValue();
         break;
      }
   }
   if (receivingUnit == nullptr)
   {
      receivingUnit = AllocateAndRegisterUnit(stackType, 0);//no dudes yet
   }
   receivingUnit->ModifyQuantity(quantity, xpOfNewUnits);
}


uint32 Army::RemoveUnits(Unit* pUnit, uint32 quantity)
{
   int32 netChange = pUnit->ModifyQuantity(-(int32)quantity);
   check(netChange == -(int32)quantity);

   if (pUnit->Quantity() == 0)
   {
      DeleteAndDeregisterUnit(pUnit);
   }

   return netChange;
}

Unit* Army::AllocateAndRegisterUnit(UnitStackType stackType, uint32 quantity)
{
	//verify it doesn't already exist, allocate unit, add to army containers
	check(GetUnit(stackType) == nullptr);
	Unit* unit = Unit::AllocateNewUnit(stackType, quantity);
	RegisterUnitWithArmy(unit);
	return unit;
}

void Army::DeleteAndDeregisterUnit(Unit* pUnit)
{
   DeregisterUnitWithArmy(pUnit);
   delete pUnit;
}

Unit* Army::GetUnit(UnitGUID guid)
{
   return m_unitsByGuid.FindChecked(guid);
}

void Army::RegisterUnitWithArmy(Unit* unit)
{
   if (unit == nullptr || m_unitsByGuid.Contains(unit->GetGuid()))
   {
      checkf(false, TEXT("Army::RegisterUnitWithArmy was passed a null unit or one that was already registered! (address: %x)\n"), reinterpret_cast<uint64>(unit));
      return;
   }

   uint32 guid = unit->GetGuid();
   UnitType type = unit->GetUnitType();
   uint32 rank = unit->GetRank();

	m_unitsByGuid.Add(guid, unit);
	ListOfUnits*& unitList = m_unitsByUnitType.FindOrAdd(type);
	if (unitList == nullptr)
	{
		unitList = new TDoubleLinkedList<Unit*>();
	}
	unitList->AddHead(unit);

	MapOfUnitGUIDs*& guidList = m_unitsByRankByGuid.FindOrAdd(rank);
	if (guidList == nullptr)
	{
		guidList = new TMap<UnitGUID, Unit*>();
	}
	guidList->Add(guid, unit);

   if (unit->IsHero())
	{
		m_herosByGuid.Add(guid, unit);
	}
	else
	{
		m_regularsByGuid.Add(guid, unit);
	}
}

void Army::DeregisterUnitWithArmy(Unit* unit)
{
   if (unit == nullptr && !m_unitsByGuid.Contains(unit->GetGuid()) )
   {
      checkf(false, TEXT("Army::DeregisterUnitWithArmy is a private function so calling it with an invalid unit means the management of this pointer is somehow broken"));
      return;
   }

   uint32 guid = unit->GetGuid();
   UnitType type = unit->GetUnitType();
   uint32 rank = unit->GetRank();

   // UnitsByGuid
   auto entriesRemoved = m_unitsByGuid.Remove(guid);
   check(entriesRemoved == 1);

   //UnitsByUnitType
   ListOfUnits* unitList = m_unitsByUnitType.FindChecked(type);
   auto* pNode = unitList->FindNode(unit);
   check(pNode != nullptr);
   if (pNode != nullptr)
   {
      unitList->RemoveNode(pNode);
   }

	//UnitsByRankByGuid
	entriesRemoved = m_unitsByRankByGuid.FindChecked(unit->GetRank())->Remove(unit->GetGuid());
	check(entriesRemoved == 1);

	//HerosByGuid or RegularsByGuid
   entriesRemoved = unit->IsHero() ? m_herosByGuid.Remove(guid) : m_regularsByGuid.Remove(guid);
   check(entriesRemoved == 1);
}