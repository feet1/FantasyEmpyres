// Fill out your copyright notice in the Description page of Project Settings.

#pragma once



/**
 * 
 */
typedef uint64 UnitType;

class PRETENDEMPYRES_API Unit
{
	//Unit Type declarations
public:
	static const uint32 RearmostRank = 4;
	static const UnitType raceMask;
	static const UnitType classMask;
	static const UnitType heroMask;
	static const UnitType levelMask;

	struct Class
	{
		FString noun;
		FString plural;
		FString icon;
		uint32 rank;
		uint32 health;
		uint32 healthperlevel;
		uint32 damage;
		uint32 damageperlevel;
		uint32 recruittime;
		uint32 recruitcost;
		uint32 upkeep;
		uint32 upkeepperlevel;
		uint32 arcane;
		uint32 arcaneperlevel;
		uint32 holy;
		uint32 holyperlevel;
		uint32 nature;
		uint32 natureperlevel;
		bool canbehero;
		bool canberegular;
	};

	struct RacialBonus
	{
		FString classnoun;
		FString noun;
		FString plural;
		float health;
		float healthperlevel;
		float damage;
		float damageperlevel;
		float recruittime;
		float recruitcost;
		float upkeep;
		float upkeepperlevel;
		float arcane;
		float arcaneperlevel;
		float holy;
		float holyperlevel;
		float nature;
		float natureperlevel;
	};

	struct Race
	{
		FString noun;
		FString plural;
		FString adjective;
		FString icon;
		float prevalence;
		TMap<FString /*class noun*/, RacialBonus*> bonuses;
	};

/*	enum UnitRace:uint32
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


	}; */
/*	enum UnitClass:uint32
	{
		//etc			 = 1<<20,
		Fighter          = 1<<21,
		Ranger			 = 1<<22,
		Knight			 = 1<<23,
		Cleric			 = 1<<24,
		Wizard			 = 1<<25,
		Druid			 = 1<<26

	}; */
/*	struct UnitData
	{
		uint32 health; //multiply for stack
		uint32 rank;
		uint32 upkeep; //multiply for stack
		uint32 recruitCost;
		uint32 recruitTime;
		uint32 damage; //multiply for stack
		uint32 armor;
		//UnitClass unitClass;
		//UnitRace unitRace;
		uint32 magicAccumulation;
		uint32 maxLevel;
		uint32 arcane;
		uint32 holy;
		uint32 nature;
		uint32 damagePerLevel;
		uint32 armorPerLevel;
		uint32 rangePerLevel;
		uint32 upkeepPerLevel;
		uint32 healthPerLevel;
		uint32 arcanePerLevel;
		uint32 holyPerLevel;
		uint32 naturePerLevel;
		bool isHero;
		uint32 xPValue;
		uint32 xPValuePerLevel;
		FString raceIcon;
		FString classIcon;
		bool canbehero;
		bool canberegular;
	}; */

private:
	
	struct RaceAndClass
	{
		const Class* aclass;
		const Race* race;
	};


	//per-instance features
public:
	Unit(UnitType _type, uint32 _quantity=1, uint32 _level=0);
	~Unit();
	UnitType GetUnitType() const;
	bool IsHero() const;
	uint32 ModifyHealth(int32 amount);	//returns # units killed
	void GrantExperience(uint32 _quantity);
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
	static TMap<UnitType /* race bits of UnitType*/,Race*> RaceDatabase;
	static TMap<UnitType /* class bits of UnitType*/, Class*> ClassDatabase;
	static RaceAndClass GetRaceAndClass(UnitType _type);
};

