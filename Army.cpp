// Fill out your copyright notice in the Description page of Project Settings.

#include "PretendEmpyres.h"
#include "Unit.h"

float Army::DamagePunchThroughThreshhold = 0.5f;
float Army::MinimumDamagePunchedThrough = 0.2f;

Army::Army(FString& _armyName, const int32 _identity)
{
	armyName = "Army"; //eventually build automatic army names based on an incrementing variable based on the team, i.e. "Selastria First Legion"

}

Army::~Army()
{
	for (auto const& unit : UnitsByGuid)
	{
		delete unit.Value;
	}
	UnitsByGuid.Empty();
	UnitsByUnitType.Empty();
	HerosByGuid.Empty();
	RegularsByGuid.Empty();
	UnitsByRankByGuid.Empty();
}

void Army::AddExperience(uint32 amount)
{
	for (const auto& unit : UnitsByGuid)
	{
		uint32 grantedXP = amount / UnitsByGuid.Num();
		unit.Value->GrantExperience(grantedXP);
	}
}

bool Army::BeginAttack(Army* defender) 
{
	//returns true if this army won
	uint32 damageToAttacker;
	uint32 XPAwardedToAttacker;
	uint32 XPAwardedToDefender;
	bool defenderRemains = true;
	bool AttackerRemains = true;
	while (defenderRemains&&AttackerRemains)
	{
		defenderRemains = defender->GetDunked(damageToAttacker, XPAwardedToAttacker, SumDamage());
		unawardedXP += XPAwardedToAttacker;
		AttackerRemains = DamageUnits(XPAwardedToDefender, damageToAttacker);
		defender->unawardedXP += XPAwardedToDefender;
	}
	//if we ain't ded, give attackorz xp
	return AttackerRemains;

}

void Army::AddRegulars(UnitType unitType, uint32 quantity)
{
	Unit* unit = GetUnitPointerByUnitType(unitType);
	if (unit == nullptr)
	{
		AllocateUnit(unitType, quantity);
	}
	else
	{
		unit->ModifyQuantity(quantity);
	}
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
	while(UnitsByGuid.Num() > 0 && damage > 0)
	{
		for (const auto& rank : UnitsByRankByGuid)
		{
			uint32 unitsInThisRank = 0;
			uint32 totalHealthOfUnitsInRank = 0;
			for (const auto& unit : rank.Value)
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
				for (const auto& unit : rank.Value)
				{
					float portionOfThisRank = unit.Value->Quantity() / (float)unitsInThisRank;
					int32 damageToThisUnit = portionOfThisRank * nonPunchthroughDamage;
					uint32 unitsKilled = unit.Value->ModifyHealth(-damageToThisUnit);
					if (unit.Value->Quantity() == 0)
					{
						DeleteUnit(unit.Value->GetGuid());
					}
					outXP += unit.Value->GetXPValue(false) * unitsKilled;
				}
				if (damage == 0)
				{
					return UnitsByGuid.Num() != 0;
				}

			}
			//only fires if rank has x2 health as dmg & soaks remainder of damage and returns false
			else
			{
				for (const auto& unit : rank.Value)
				{
					float portionOfThisRank = unit.Value->Quantity() / (float)unitsInThisRank;
					int32 damageToThisUnit = portionOfThisRank * damage;
					uint32 unitsKilled = unit.Value->ModifyHealth(-damageToThisUnit);
					outXP += unit.Value->GetXPValue(false) * unitsKilled;
					if (unit.Value->Quantity() == 0)
					{
						DeleteUnit(unit.Value->GetGuid());
					}
				}
				return false;
			} //only fires if rank has x2 health as dmg //soaks remainder of damage and returns false

		}
	}
	//apply fancy damage formula
	check(UnitsByGuid.Num() == 0)  //find out how the hell there are any units left
	return false;
}

uint32 Army::SumDamage()
{
	uint32 totalDamage = 0;
	for (const auto& unit : UnitsByGuid)
	{
		totalDamage += unit.Value->GetAttackDamage();
	}
	return totalDamage;
}

void Army::DeleteUnit(uint32 guid)
{
	RemoveRegulars(guid, 0);
}
uint32 Army::RemoveRegulars(uint32 guid, uint32 quantity)
{
	Unit* unit = UnitsByGuid.FindChecked(guid);
	uint32 qty = unit->Quantity();
	if (quantity >= qty||quantity==0)
	{
		//remove from UnitsByGuid
		UnitsByGuid.Remove(guid);
		//remove from UnitsByUnitType
		TDoubleLinkedList<Unit*>* list = UnitsByUnitType.Find(unit->GetUnitType());
		check(list != nullptr);
		if (list != nullptr)
		{
			TDoubleLinkedList<Unit*>::TDoubleLinkedListNode* node = list->FindNode(unit);
			check(node != nullptr);
			if (node != nullptr)
			{
				list->RemoveNode(node);
			}
		}
		//remove from UnitsByRankByGuid
		TMap<uint32, Unit*>* map = UnitsByRankByGuid.Find(unit->GetRank());
		check(map != nullptr);
		if (map != nullptr)
		{
			uint32 count = map->Remove(guid);
			check(count != 0);
		}

		//remove from either HerosByGuid or RegularsByGuid
		TMap<uint32, Unit*> &HerosOrRegularsByGuid = (unit->IsHero()) ? HerosByGuid:RegularsByGuid;
		HerosOrRegularsByGuid.Remove(guid);
		delete unit;
		//remove every single place 
		return qty;
	}
	else
	{
		int32 amountRemoved = -(unit->ModifyQuantity(-quantity));
		check(amountRemoved > 0);
		return (uint32)amountRemoved;
	}

}

//will be called by GUI to move units between your armies
void Army::TransferUnit(uint32 _guid, uint32 _quantity, Army* gainingArmy)
{
	if (true) //(destination=={0,0,0}, unit has not moved this turn)
	{
		Unit* unit = UnitsByGuid.FindChecked(_guid);
		
		if (unit->IsHero())
		{
			TransferHero(unit, gainingArmy);
			return;
		}
		uint32 troopsSent = 0;
		uint32 troopsRecieved = 0;
		//xfer units by qty
		UnitType unitType = unit->GetUnitType();
		gainingArmy->AddRegulars(unitType, _quantity);
		troopsSent = RemoveRegulars(_guid, _quantity);
		check(troopsSent == troopsRecieved);
	}
}

void Unit::TransferHero(Unit* hero, Army* gainingArmy)
{
	//gainingArmy->
}

Unit* Army::AllocateUnit(UnitType _type, uint32 _quantity)
{
	//verify it doesn't already exist, allocate unit, add to army containers
	check(GetUnitPointerByUnitType(_type) == nullptr);
	Unit* unit = new Unit(this, _type, _quantity);
	RegisterUnitWithArmy(unit);
	return unit;
}


Unit* Army::GetUnitPointerByUnitType(UnitType type)
{
	TDoubleLinkedList<Unit*>* list = UnitsByUnitType.Find(type);
	TDoubleLinkedList<Unit*>::TDoubleLinkedListNode* node = list->GetHead();
	while (node != nullptr)
	{
		Unit* unitPtr = node->GetValue();
		if (unitPtr->GetUnitType() == type)
		{
			return node->GetValue();
		}
		node = node->GetNextNode();
	}
	return nullptr;
}

void Army::RegisterUnitWithArmy(Unit* unit)
{
	UnitsByGuid.Add(unit->GetGuid(), unit);
	UnitsByUnitType.FindChecked(unit->GetUnitType()).AddHead(unit);
	if (unit->IsHero())
	{
		HerosByGuid.Add(unit->GetGuid(), unit);
	}
	else
	{
		RegularsByGuid.Add(unit->GetGuid(), unit);
	}
	UnitsByRankByGuid.Find(unit->GetRank())->Add(unit->GetGuid(), unit);
}
void Army::DeregisterUnitWithArmy(Unit* unit)
{
		//remove from UnitsByGuid
		UnitsByGuid.Remove(unit->GetGuid());
		//remove from UnitsByUnitType
		TDoubleLinkedList<Unit*>* list = UnitsByUnitType.Find(unit->GetUnitType());
		check(list != nullptr);
		if (list != nullptr)
		{
			TDoubleLinkedList<Unit*>::TDoubleLinkedListNode* node = list->FindNode(unit);
			check(node != nullptr);
			if (node != nullptr)
			{
				list->RemoveNode(node);
			}
		}
		//remove from UnitsByRankByGuid
		TMap<uint32, Unit*>* map = UnitsByRankByGuid.Find(unit->GetRank());
		check(map != nullptr);
		if (map != nullptr)
		{
			uint32 count = map->Remove(guid);
			check(count != 0);
		}

		//remove from either HerosByGuid or RegularsByGuid
		TMap<uint32, Unit*> &HerosOrRegularsByGuid = (unit->IsHero()) ? HerosByGuid : RegularsByGuid;
		HerosOrRegularsByGuid.Remove(guid);
		delete unit;
		//remove every single place 
		return qty;
	}
	else
	{
		int32 amountRemoved = -(unit->ModifyQuantity(-quantity));
		check(amountRemoved > 0);
		return (uint32)amountRemoved;
	}


}