// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "UnitContainer.h"
#include "Army.h"



/**
 * 
 */
typedef uint32 UnitType;

class PRETENDEMPYRES_API Unit
{
	//Unit Type declarations
public:
	enum UnitRace:uint32
	{
		Human             = 1<<0,
		Elf               = 1<<1,
		Dwarf			  = 1<<2,
		Orc				  = 1<<3,
		Halfling		  = 1<<4,
		Gnome			  = 1<<5,
		Goblin			  = 1<<6,
		Skeleton		  = 1<<7,
		Vampire			  = 1<<8,
		Lizardfolk		  = 1<<9,
		//etc			  = 1<<,


	};
	enum UnitClass:uint32
	{
		//etc			 = 1<<20,
		Fighter          = 1<<21,
		Ranger			 = 1<<22,
		Knight			 = 1<<23,
		Cleric			 = 1<<24,
		Wizard			 = 1<<25,
		Druid			 = 1<<26

	};
	enum MagicType
	{
		None,
		Wizard,
		Cleric,
		Druid
	};
	struct UnitData
	{
		uint32 health; //multiply for stack
		uint32 rank;
		uint32 upkeep; //multiply for stack
		uint32 recruitCost;
		uint32 damage; //multiply for stack
		uint32 range;
		uint32 armor;
		UnitClass unitClass;
		UnitRace unitRace;
		uint32 magicAccumulation;
		uint32 maxLevel;
		uint32 damagePerLevel;
		uint32 armorPerLevel;
		uint32 rangePerLevel;
		uint32 upkeepPerLevel;
		uint32 healthPerLevel;
		uint32 magicAccumulationPerLevel;
		MagicType magicType;
		bool isHero;
		uint32 XPValue;
		uint32 XPValuePerLevel;
	};


	//per-instance features
public:
	Unit(Army*const _army, UnitType _type, uint32 _quantity=1, uint32 _level=0);
	~Unit();
	UnitType GetUnitType() const;
	bool IsHero() const;
	//returns # units killed
	uint32 ModifyHealth(int32 amount);
	void GrantExperience(uint32 quantity);
	int32 ModifyQuantity(int32 _quantity);
	uint32 Quantity() const;
	uint32 GetMagicAccumulation() const;
	uint32 GetAttackDamage() const;
	float GetHunger() const;
	uint32 GetUpkeep() const;
	//feeds the unit, returns remainder
	uint32 FeedUnit(uint32 foodstuffs);
	uint32 GetTotalHealth() const;
	uint32 GetGuid() const;
	uint32 GetXPValue(bool wholeStack) const;
	uint32 GetRank() const;
	

private:
	UnitType type;
	Army* owningArmy;
	uint32 quantity;
	uint32 level;
	uint32 health;
	uint32 experience;
	float hunger;
	uint32 guid;

	//non-instance features
public:
	
	static void InitializeUnitDatabase();
	static bool IsHero(UnitType);
	static uint32 ExperiencePerLevel;
	static float UnitsLostToStarvationPercent;
	static float FightingPenaltyWhileHungry;
	static float HowMuchHarderYouGetHitWhileHungry;
	static TMap<uint32/*guid*/, Unit*> UnitDatabaseByGuid;
private:
	static TMap<UnitType,UnitData> UnitDatabase;

};

