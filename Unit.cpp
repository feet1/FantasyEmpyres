// Fill out your copyright notice in the Description page of Project Settings.

#include "PretendEmpyres.h"
#include "FastXML.h"
#include "XmlUnitParser.h"
#include "Unit.h"


namespace
{
	uint32 GetNewGuid()
	{
		static uint32 nextGuid = 0;
		return nextGuid++;
	}
}

//magic numbers
uint32 Unit::ExperiencePerLevel = 1000; 
float Unit::UnitsLostToStarvationPercent = 0.5f;
float Unit::FightingPenaltyWhileHungry = 0.5f;
float Unit::HowMuchHarderYouGetHitWhileHungry = 0.1f;
//[level:5][race:20][class:38][isHero:1]
const UnitType Unit::raceMask = 0x7ffff8000000000;
const UnitType Unit::classMask = 0x7fffffffe;
const UnitType Unit::heroMask = 0x1;
const UnitType Unit::levelMask = 0xf800000000000000;
TMap<UnitType /* race bits of UnitType*/, Unit::Race*> Unit::RaceDatabase;
TMap<UnitType /* class bits of UnitType*/, Unit::Class*> Unit::ClassDatabase;



Unit::Unit(UnitType _type, uint32 _quantity, uint32 _level)
	    :type(_type)
		,quantity(_quantity)
		,level(_level)
		,health(1)
		,experience(0)
		,hunger(0.0f)
		,guid(GetNewGuid())
{
	if (level == 0)
	{
		level = type >> 27;
	}
	type = (type << 5) >> 5;
//TODO	UnitData* found = UnitDatabase.Find(type);
//TODO	checkf(found != nullptr, TEXT("Unit::Unit() was passed a UnitType (%u) which was not found in the database!"), _type);
//TODO	health = found->health + found->healthPerLevel*level;

}

Unit::~Unit()
{

}

UnitType Unit::GetUnitType() const
{
	return type & (level << 27); //add in level
}

bool Unit::IsHero() const
{
//TODO	UnitData* found = UnitDatabase.Find(type);
//TODO	checkf(found != nullptr, TEXT("Unit::IsHero() instance function was passed a UnitType (%u) which was not fount in ye ol database!"), type);
//TODO	return found->isHero;
	return true; //fuck
}

uint32 Unit::ModifyHealth(int32 amount)
{/*
	UnitData* data = UnitDatabase.Find(type);
	checkf(data != nullptr, TEXT("Unit::AddHealth() failed to locate it's UnitData in the database. Stored type was (%u)\n"), type);
	uint32 maxHealth = data->health + data->healthPerLevel * level;
	if (amount < 0)
	{
		amount += (hunger * HowMuchHarderYouGetHitWhileHungry * amount);
	}
	if (amount >= 0)
	{
		health = FMath::Min(maxHealth, health + amount);
	}
	else if (health > -amount)
	{
		health += amount;
	}
	else
	{
		amount += health;
		uint32 unitsKilled = FMath::Min(quantity, 1 + FMath::Abs(amount) / maxHealth);
		amount += (unitsKilled - 1) * maxHealth;
		health = maxHealth + amount;
		ModifyQuantity(-unitsKilled);
		return unitsKilled;
	}*/
	return 0;
}

void Unit::GrantExperience(uint32 _quantity)
{
	/*
	UnitData* data = UnitDatabase.Find(type);
	checkf(data != nullptr, TEXT("Unit::GrantExperience() failed to locate it's UnitData in the database. Stored type was (%u)\n"), type);
	//manage levels in hear
	experience += quantity;
	while(experience > ExperiencePerLevel && level < data->maxLevel)
	{
		++level;
		experience -= ExperiencePerLevel;
	}*/
}

int32 Unit::ModifyQuantity(int32 _quantity)
{
	checkf(-_quantity <= (int32)quantity, TEXT("Unit::AddQuantity() was called with a negative value (%d) which was in excess of existing stack, (%u), fool!"), _quantity, quantity);
	if (_quantity<0 && -_quantity>(int32)quantity)
	{
		int32 retQuantity = -(int32)quantity;
		quantity = 0;
		return retQuantity;
	}
	else
	{
		quantity += _quantity;
		return _quantity;
	}
}

uint32 Unit::Quantity() const
{
	return quantity;
}

uint32 Unit::GetMagicAccumulation() const
{
	/*
	UnitData* found = UnitDatabase.Find(type);
	checkf(found != nullptr, TEXT("Unit::GetMagicAccumulation() was passed a UnitType (%u) which was not found in the database!"), type);
	uint32 returnMagic = found->magicAccumulation*found->magicAccumulationPerLevel*level*quantity;
	return returnMagic;
	*/return 0; //retarded
}

uint32 Unit::GetAttackDamage() const
{
	/*
	UnitData* data = UnitDatabase.Find(type);
	checkf(data != nullptr, TEXT("Unit::GetBattleData() failed to locate it's UnitData in the database. Stored type was (%u)\n"), type);
	float hungerEffect = 1 - hunger * FightingPenaltyWhileHungry;
	return hungerEffect * (data->damage + data->damagePerLevel * level) * quantity;
	*/return 3; //derp
	}

float Unit::GetHunger() const
{
	return hunger;
}

uint32 Unit::GetUpkeep() const
{
	/*
	UnitData* data = UnitDatabase.Find(type);
	checkf(data != nullptr, TEXT("Unit::GetUpkeep() failed to locate it's UnitData in the database. Stored type was (%u)\n"), type);
	uint32 _upkeep = (data->upkeep + (level * data->upkeepPerLevel))*quantity;
	return _upkeep;*/return 34;
}

uint32 Unit::FeedUnit(uint32 foodstuffs)
{
	uint32 needs = GetUpkeep();
	checkf(needs != 0, TEXT("Dividing by zero eh? Unit::FeedUnit() got a zero back from Unit::GetUpkeep() for UnitType %d"), type);
	float percentNeedsMet = FMath::Min(1.0f, (float)foodstuffs / needs);
	if (percentNeedsMet == 1.0f)
	{
		hunger /= 2;
	}
	else
	{
		hunger = FMath::Lerp(hunger, 1.0f - percentNeedsMet, 0.33f);
	}

	if (hunger > 0.0f)
	{
		if (hunger < 0.05f)
		{
			hunger = 0.0f;
		}
		else if (hunger > 0.9f)
		{
			hunger = 1.0f;
			
			int32 UnitsStarvedToDeath = quantity * (1.0f - percentNeedsMet) * UnitsLostToStarvationPercent;
			ModifyQuantity(-UnitsStarvedToDeath);
			//TODO: Broadcast and event for the system to handle that a "your shit is starving to death m'lord"
		}
		else if (percentNeedsMet != 1.0f)
		{
			//TODO: Occur once per turn: Your units are hungry m'lord. Hungry units fight worse, and eventually will starve to death.
			//TODO: Notify UI that this unit is insufficiently fed
		}
	}
	uint32 foodLeftover = foodstuffs - FMath::Min(foodstuffs, needs);
	return foodLeftover;
}

uint32 Unit::GetTotalHealth() const
{
	/*
	UnitData* data = UnitDatabase.Find(type);
	checkf(data != nullptr, TEXT("Unit::GetTotalHealth() failed to locate it's UnitData in the database. Stored type was (%u)\n"), type);
	uint32 _totalHealth = (data->health + (level * data->healthPerLevel)) * (quantity - 1) + health;
	return _totalHealth;*/return 42;
}

uint32 Unit::GetGuid() const
{
	return guid;
}

uint32 Unit::GetXPValue(bool wholeStack) const
{
	/*
	UnitData* data = UnitDatabase.Find(type);
	checkf(data != nullptr, TEXT("Unit::GetXPValue() failed to locate it's UnitData in the database. Stored type was (%u)\n"), type);
	if (data == nullptr)
	{
		return 0;
	}
	return (data->XPValue + level * data->XPValuePerLevel) * (wholeStack ? quantity : 1);
*/ return -1;
}

uint32 Unit::GetRank() const
{
	/*
	UnitData* data = UnitDatabase.Find(type);
	checkf(data != nullptr, TEXT("Unit::GetRank() failed to locate it's UnitData in the database. Stored type was (%u)\n"), type);
	if (data == nullptr)
	{
		return 0;
	}
	return data->rank;*/return 54;
}


bool Unit::IsHero(UnitType _type)
{
	/*
	_type = (_type << 5) >> 5; //shift out level so database lookup works
	UnitData* foundType = UnitDatabase.Find(_type);
	checkf(foundType != nullptr, TEXT("Unit::IsHero() was passed a UnitType (%u) which was not found in the database!"), _type);
	return foundType->isHero;*/return false;//always false... always.
} 

Unit::RaceAndClass Unit::GetRaceAndClass(UnitType _type)
{
	//[level:5][race:20][class:38][isHero:1]
	RaceAndClass returnValue;
	uint32 race = _type&raceMask;
	uint32 aclass = _type&classMask;
	returnValue.aclass = ClassDatabase.FindChecked(aclass);
	returnValue.race = RaceDatabase.FindChecked(race);
	return returnValue;
}

void Unit::InitializeUnitDatabase()
{
	//deserialize all the THINGS
	//use XML thing to serialize the thing
	//populate the static UnitDatabases
	XmlUnitParser parser;
	FText errorMessage;
	int32 errorLineNumber;
	FFastXml::ParseXmlFile(&parser, TEXT("D:\file.xml"), nullptr, nullptr, false, false, errorMessage, errorLineNumber);
	
}