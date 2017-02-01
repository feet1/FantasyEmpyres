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
TMap<UnitType, Unit::RaceAndClass> Unit::____RaceAndClassCache____UseGetRaceAndClassFunction_Instead_Stupid;



Unit::Unit(UnitType _type, uint32 _quantity, uint32 _level)
	    :m_type(_type)
		,m_quantity(_quantity)
		,m_level(_level)
		,m_health(1)
		,m_experience(0)
		,m_hunger(0.0f)
		,m_guid(GetNewGuid())
{
	if (m_level == 0)
	{
		m_level = m_type >> 27;
	}
	m_type = (m_type << 5) >> 5;
}

Unit::~Unit()
{

}

UnitType Unit::GetUnitType() const
{
	return m_type & ~levelMask;
}

bool Unit::IsHero() const
{
   return (levelMask & m_type) != 0;
}

uint32 Unit::ModifyHealth(int32 amount)
{
   const RaceAndClass& raceAndClass = GetRaceAndClass(m_type);
   uint32 maxHealth = raceAndClass.health + raceAndClass.healthperlevel * m_level;

	if (amount >= 0)
	{
		m_health = FMath::Min(maxHealth, m_health + amount);
	}
   else
   {
      amount += (m_hunger * HowMuchHarderYouGetHitWhileHungry * amount);
	   if (m_health > (uint32)FMath::Abs(amount))
	   {
		   m_health -= (uint32)FMath::Abs(amount);
	   }
	   else
	   {
		   amount += (int32)m_health;
		   uint32 unitsKilled = FMath::Min(m_quantity, 1 + FMath::Abs(amount) / maxHealth);
		   amount += (int32)((unitsKilled - 1) * maxHealth);
		   m_health = maxHealth - FMath::Abs(amount);
		   ModifyQuantity(-(int32)unitsKilled);
		   return unitsKilled;
	   }
   }
	return 0;
}

void Unit::GrantExperience(uint32 quantity)
{
	m_experience += quantity;
   uint32 maxLevel = ((bool) (m_type & heroMask)) ? HeroMaxLevel : RegularMaxLevel;
	while(m_experience > ExperiencePerLevel && m_level < maxLevel)
	{
		++m_level;
		m_experience -= ExperiencePerLevel;
	}
}

int32 Unit::ModifyQuantity(int32 _quantity)
{
	checkf(-_quantity <= (int32)m_quantity, TEXT("Unit::AddQuantity() was called with a negative value (%d) which was in excess of existing stack, (%u), fool!"), _quantity, m_quantity);
	if (_quantity<0 && -_quantity>(int32)m_quantity)
	{
		int32 retQuantity = -(int32)m_quantity;
		m_quantity = 0;
		return retQuantity;
	}
	else
	{
		m_quantity += _quantity;
		return _quantity;
	}
}

uint32 Unit::Quantity() const
{
	return m_quantity;
}

void Unit::GetMagicAccumulation(int32& outArcane, int32& outHoly, int32& outNature) const
{
   const RaceAndClass& rac = GetRaceAndClass(m_type);
   outArcane = rac.arcane + rac.arcaneperlevel * m_level;
   outHoly = rac.holy + rac.holyperlevel * m_level;
   outNature = rac.nature + rac.natureperlevel * m_level;

}

uint32 Unit::GetAttackDamage() const
{
   const RaceAndClass& rac = GetRaceAndClass(m_type);
	float hungerEffect = 1 - m_hunger * FightingPenaltyWhileHungry;
	return hungerEffect * (rac.damage + rac.damageperlevel * m_level) * m_quantity;
}

float Unit::GetHunger() const
{
	return m_hunger;
}

uint32 Unit::GetUpkeep() const
{
   const RaceAndClass& rac = GetRaceAndClass(m_type);
   uint32 upkeep = (rac.upkeep + (m_level * rac.upkeepperlevel)) * m_quantity;
	return upkeep;
}

uint32 Unit::FeedUnit(uint32 foodstuffs)
{
	uint32 needs = GetUpkeep();
	if(needs == 0)
   {
      return foodstuffs;
   }
   float percentNeedsMet = FMath::Min(1.0f, (float)foodstuffs / needs);
	if (percentNeedsMet == 1.0f)
	{
		m_hunger /= 2;
	}
	else
	{
		m_hunger = FMath::Lerp(m_hunger, 1.0f - percentNeedsMet, 0.33f);
	}

	if (m_hunger > 0.0f)
	{
		if (m_hunger < 0.05f)
		{
			m_hunger = 0.0f;
		}
		else if (m_hunger > 0.9f)
		{
			m_hunger = 1.0f;
			
			int32 UnitsStarvedToDeath = m_quantity * (1.0f - percentNeedsMet) * UnitsLostToStarvationPercent;
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
	
   const RaceAndClass& rac = GetRaceAndClass(m_type);
   uint32 totalHealth = (rac.health + (m_level * rac.healthperlevel)) * (m_quantity - 1) + m_health;
	return totalHealth;
}

uint32 Unit::GetGuid() const
{
	return m_guid;
}

uint32 Unit::GetXPValue(bool wholeStack) const
{
   const RaceAndClass& rac = GetRaceAndClass(m_type);
   return (rac.xpvalue + m_level * rac.xpvalueperlevel) * (wholeStack ? m_quantity : 1);
}

uint32 Unit::GetRank() const
{
   const RaceAndClass& rac = GetRaceAndClass(m_type);
	return rac.rank;
}

const Unit::RaceAndClass& Unit::GetRaceAndClass(UnitType type)
{
	//[level:5][race:20][class:38][isHero:1]
   type &= ~levelMask;
   RaceAndClass* cachedValuep = ____RaceAndClassCache____UseGetRaceAndClassFunction_Instead_Stupid.Find(type);
   if (cachedValuep == nullptr)
   {
      Race* racep = RaceDatabase.FindChecked(type & raceMask);
      Class* classp = ClassDatabase.FindChecked(type & classMask);
      //lookup for specific racial version of this class
      //if the class is not found, use generic racial bonus "any"
      RacialBonus** bonuspp = racep->bonuses.Find(classp->noun);
      RacialBonus* bonusp;
      if(bonuspp == nullptr)
      {
         bonusp = racep->bonuses.FindChecked(TEXT("any"));
      }
      else
      {
         bonusp = *bonuspp;
      }
      cachedValuep = &____RaceAndClassCache____UseGetRaceAndClassFunction_Instead_Stupid.FindOrAdd(type);
      cachedValuep->classNoun = classp->noun;
      cachedValuep->classPlural = classp->plural;
      cachedValuep->classIcon = classp->icon;
      cachedValuep->raceNoun = racep->noun;
      cachedValuep->racePlural = racep->plural;
      cachedValuep->raceIcon = racep->icon;
      cachedValuep->raceAdjective = racep->adjective;
      cachedValuep->racePrevalence = racep->prevalence;
      cachedValuep->rank = classp->rank;
      cachedValuep->health = (uint32)(classp->health * bonusp->health);
      cachedValuep->healthperlevel = (uint32)(classp->healthperlevel * bonusp->healthperlevel);
      cachedValuep->damage = (uint32)(classp->damage * bonusp->damage);
      cachedValuep->damageperlevel = (uint32)(classp->damageperlevel * bonusp->damageperlevel);
      cachedValuep->recruittime = (uint32)(classp->recruittime * bonusp->recruittime);
      cachedValuep->recruitcost = (uint32)(classp->recruitcost * bonusp->recruitcost);
      cachedValuep->upkeep = (uint32)(classp->upkeep * bonusp->upkeep);
      cachedValuep->upkeepperlevel = (uint32)(classp->upkeepperlevel * bonusp->upkeepperlevel);
      cachedValuep->upkeepperlevel = (uint32)(classp->xpvalue * bonusp->xpvalue);
      cachedValuep->upkeepperlevel = (uint32)(classp->xpvalueperlevel * bonusp->xpvalueperlevel);
      cachedValuep->arcane = (uint32)(classp->arcane * bonusp->arcane);
      cachedValuep->arcaneperlevel = (uint32)(classp->arcaneperlevel * bonusp->arcaneperlevel);
      cachedValuep->holy = (uint32)(classp->holy * bonusp->holy);
      cachedValuep->holyperlevel = (uint32)(classp->holyperlevel * bonusp->holyperlevel);
      cachedValuep->nature = (uint32)(classp->nature * bonusp->nature);
      cachedValuep->natureperlevel = (uint32)(classp->natureperlevel * bonusp->natureperlevel);
      cachedValuep->canbehero = classp->canbehero;
      cachedValuep->canberegular = classp->canberegular;
   }
	return *cachedValuep;
}

void Unit::InitializeUnitDatabase()
{
	//deserialize all the THINGS
	//use XML thing to serialize the thing
	//populate the static UnitDatabases
	// initialize RaceAndClassCache
	XmlUnitParser parser;
	FText errorMessage;
	int32 errorLineNumber;
	FFastXml::ParseXmlFile(&parser, *FPaths::Combine(*FPaths::GameContentDir(), TEXT("/content/unitdatabase.xml")), nullptr, nullptr, false, false, errorMessage, errorLineNumber);
	
}