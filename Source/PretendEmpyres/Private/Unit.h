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
   static const uint32 HeroMaxLevel = 30;
   static const uint32 RegularMaxLevel = 4;
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
      uint32 xpvalue;
      uint32 xpvalueperlevel;
		int32 arcane;
		int32 arcaneperlevel;
		int32 holy;
		int32 holyperlevel;
		int32 nature;
		int32 natureperlevel;
		bool canbehero;
		bool canberegular;
	};

	struct RacialBonus
	{
		FString classnoun;
		FString noun;
		FString plural;
      float xpvalue;
      float xpvalueperlevel;
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

private:

	//pre-calculated cached values
	struct RaceAndClass  
	{
      FString classNoun;
      FString classPlural;
      FString classIcon;
      FString raceNoun;
      FString racePlural;
      FString raceIcon;
      FString raceAdjective;
      float racePrevalence;
      uint32 rank;
      uint32 health;
      uint32 healthperlevel;
      uint32 damage;
      uint32 damageperlevel;
      uint32 recruittime;
      uint32 recruitcost;
      uint32 upkeep;
      uint32 upkeepperlevel;
      uint32 xpvalue;
      uint32 xpvalueperlevel;
      int32 arcane;
      int32 arcaneperlevel;
      int32 holy;
      int32 holyperlevel;
      int32 nature;
      int32 natureperlevel;
      bool canbehero;
      bool canberegular;
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
	void GetMagicAccumulation(int32& outArcane, int32& outHoly, int32& outNature) const;
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
	UnitType m_type;
	uint32 m_quantity;
	uint32 m_level;
	uint32 m_health;
	uint32 m_experience;
	float m_hunger;
	uint32 m_guid;

	//non-instance features
public:
	
	static void InitializeUnitDatabase();
	static bool IsHero(UnitType);
   static const RaceAndClass& GetRaceAndClass(UnitType type);
	static uint32 ExperiencePerLevel;
	static float UnitsLostToStarvationPercent;
	static float FightingPenaltyWhileHungry;
	static float HowMuchHarderYouGetHitWhileHungry;
	static TMap<uint32/*guid*/, Unit*> UnitDatabaseByGuid;
private:
	static TMap<UnitType /* race bits of UnitType*/,Race*> RaceDatabase;
	static TMap<UnitType /* class bits of UnitType*/, Class*> ClassDatabase;
   static TMap<UnitType, RaceAndClass> ____RaceAndClassCache____UseGetRaceAndClassFunction_Instead_Stupid;
};

