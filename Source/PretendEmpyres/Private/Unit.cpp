#include "PretendEmpyres.h"
#include "FastXML.h"
#include "XmlUnitParser.h"
#include "Unit.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Static Constants (Magic Numbers and Such)
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma region StaticConstants

//We split the bits of UnitType into [Level][Race][Class][ishero].
// Class gets however many bits are left after the others are assigned
// Since a hero 'knight' is a different class for the purpose of stats,
// we have a mask that accounts for the hero bit (fullClassMask) whereas
// the "classMask" represents the part of the class ignorant of the hero bit
// such as to see if x regulars are the same type of unit as y hero

const UnitType allBitsSet = 0xFFFFFFFFFFFFFFFF;// just used to instantiate the mask values, not part of Unit
const UnitType bitsInByte = 8;//just need to multiply by sizeof() to get bit counts
const UnitType Unit::raceBits = 24;
const UnitType Unit::heroBits = 1;
const UnitStackType Unit::levelBits = 5;
const UnitType Unit::classBits = sizeof(UnitStackType) * bitsInByte - Unit::levelBits - Unit::raceBits - Unit::heroBits; //class gets all remaining bits
const UnitType Unit::Unit::raceMask = (allBitsSet << (sizeof(UnitStackType) * bitsInByte - Unit::raceBits)) >> Unit::levelBits;
const UnitType Unit::classMask = (allBitsSet << (sizeof(UnitStackType) * bitsInByte - Unit::classBits)) >> (Unit::levelBits + Unit::raceBits);
const UnitType Unit::heroMask = allBitsSet >> (sizeof(UnitStackType) * bitsInByte - Unit::heroBits);
const UnitStackType Unit::levelMask = allBitsSet << (sizeof(UnitStackType) * bitsInByte - Unit::levelBits);

const uint32 Unit::RearmostRank = 4;
const uint32 Unit::HeroMaxLevel = 30;
const uint32 Unit::RegularMaxLevel = 4;
const uint32 Unit::ExperiencePerLevel = 1000;
const uint32 Unit::NumberOfRegularsAHeroIsWorth = 50;
const float Unit::UnitsLostToStarvationPercent = 0.5f;
const float Unit::FightingPenaltyWhileHungry = 0.5f;
const float Unit::HowMuchHarderYouGetHitWhileHungry = 0.1f;


//some functionality really only needed within this CPP so it's hidden in an anonymous namespace
namespace /* ANONYMOUS */
{
   UnitGUID GetNewGuid()
   {
      static UnitGUID nextGuid = 0;
      return ++nextGuid;
   }
   UnitStackType GetNewRaceID()
   {
      static UnitStackType nextId = 0;
      static UnitStackType maxRaceId = Unit::raceMask >> (Unit::classBits + Unit::heroBits);
      checkf(nextId < maxRaceId, TEXT("FATAL: Too many races were serialized to fit in %u bits, max value %u\n"), Unit::raceBits, maxRaceId);
      return ++nextId << (Unit::classBits + Unit::heroBits);
   }
   UnitStackType GetNewClassID()
   {
      static UnitStackType nextId = 1;
      static UnitStackType maxClassId = Unit::classMask >> Unit::heroBits;
      checkf(nextId < maxClassId, TEXT("FATAL: Too many classes were serialized to fit in %u bits, max value %u\n"), Unit::classBits, maxClassId);
      return ++nextId << Unit::heroBits;
   }
}
#pragma endregion

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Static Containers (Unit Databases)
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
TMap<UnitStackType /* race bits of UnitType*/, Unit::Race*> Unit::RaceDatabase;
TMap<UnitStackType /* class bits of UnitType*/, Unit::Class*> Unit::ClassDatabase;
TMap<UnitStackType, Unit::RaceAndClass> Unit::_______RaceAndClassCache_______;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Default constructor, which is private (not callable directly).
*    Call the AllocateNewUnit() static method to actually create units
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
Unit::Unit()
{
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Destructor, no dynamic memory owned by instances of this class, so no work to be done.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
Unit::~Unit()
{

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Retrieves the UnitType as it will be used by entities external to the Unit. The level is NOT in
*        this value to allow such operations as determining variety (dwarven fighter)
*
*     return:     returns the full unit type, including the level bits
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
UnitStackType Unit::GetUnitStackType() const
{
   checkf((m_type & levelMask) == 0, TEXT("GetUnitType found an issue with Unit::type: The level should not be stored in type."));
   return (m_type & ~levelMask) | (((UnitStackType)m_level) << (classBits + raceBits + heroBits));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Retrieves the UnitStackType as it will be used by entities external to the Unit. The level is encoded into
*        this value to allow such operations as comparing two units stacking (level 2 dwarven fighter vs level 3 dwarven fighter?)
*
*     return:     returns the full unit type, including the level bits
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
UnitStackType Unit::GetUnitType() const
{
   checkf((m_type & levelMask) == 0, TEXT("GetUnitType found an issue with Unit::type: The level should not be stored in type."));
   return (m_type & ~levelMask);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Retrieves the heroic state of the unit.
*
*     return:     true if this is a hero unit, false otherwise.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
bool Unit::IsHero() const
{
   return (heroMask & m_type) != 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Adds or removes health from the unit stack. This function can handle dealing damage in excess of the health 1 single unit can
*        handle, and will properly spill over on the stack until sufficiently many units in the stack have perished.
*
*     amount:     The hit points healed if positive, or damage dealt if negative.
*
*     return:     Quantity of units lost as a result. 0 for healing, [0 to Quantity] for damage.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint32 Unit::ModifyHealth(int32 amount)
{
   const RaceAndClass& rac = GetRaceAndClass(m_type);
   uint32 maxHealth = rac.health + rac.healthperlevel * m_level;
   uint32 absAmount = (uint32)FMath::Abs(amount);

   if (amount >= 0)
   {
      //Heal the unit!
      m_health = FMath::Min(maxHealth, m_health + absAmount);
   }
   else
   {
      //Damage the unit!

      //increase damage based on hunger effects
      amount += (int32)(m_hunger * HowMuchHarderYouGetHitWhileHungry * amount);

      //soakable damage we handle up front
      if (m_health > absAmount)
      {
         m_health -= absAmount;
      }
      else
      {
         //this unit is dead, remove it's health from amount needing to be dealt
         absAmount -= m_health; 

         //The units that will die include the dude above, plus however many full-health units the rest of the damage can kill...
         uint32 unitsKilled = FMath::Min(m_quantity, 1 + absAmount / maxHealth);

         if (unitsKilled >= m_quantity)
         {
            //TODO: Fire event: Unit destroyed
            return m_quantity;
         }

         //the remainder damage that wasn't enough to kill a full dude
         absAmount %= maxHealth;

         //The current "top" unit's health is reduced by the remainder
         m_health = maxHealth - absAmount;

         //killing units requires a negative quantity
         ModifyQuantity(-(int32)unitsKilled);

         return unitsKilled;
      }
   }
   return 0;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Awards the unit stack with experience, and properly levels up the stack if enough XP is awarded.
*        If a unit reaches their maximum level, their XP total will be set to 0 and remain there.
*
*     amount:     The amount of XP to grant the unit with
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void Unit::GrantExperience(uint32 amount)
{
   uint32 maxLevel = (m_type & heroMask) != 0 ? HeroMaxLevel : RegularMaxLevel;

   m_experience += FMath::Max(1u, (uint32)(((float)amount / m_quantity) + 0.5f));

   while(m_experience >= ExperiencePerLevel && m_level < maxLevel)
   {
      ++m_level;
      m_experience -= ExperiencePerLevel;
   }

   if (m_level == maxLevel)
   {
      m_experience = 0;
   }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Changes the number of units in this stack. Positive to add, negative to remove.
*
*   quantity:     The number of units to add if positive, or remove if negative.
*
*     return:     The net change to the stack (how many units were actually added or subtracted)
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 Unit::ModifyQuantity(int32 quantity, uint32 xpOfNewUnits)
{
   int32 originalCount = (int32)m_quantity;
   checkf(quantity >= 0 || originalCount >= FMath::Abs(quantity), TEXT("Unit::AddQuantity() was called with a negative value (%d) in excess of stack size (%u)\n"), quantity, m_quantity);
   
   if (quantity > 0)
   {
      //When adding new units to the stack, we take a weighted average of the experience of each group
      //to figure out how much XP the new stack should have in the most balanced way possible
      uint32 oldUnitCount = m_quantity;
      m_quantity += (uint32)quantity;
      float oldUnitRatio = oldUnitCount / m_quantity;
      float newUnitRatio = 1 - oldUnitRatio;
      m_experience = m_experience * oldUnitRatio + xpOfNewUnits * newUnitRatio;
   }
   else if (originalCount <= FMath::Abs(quantity))
   {
      m_quantity = 0;
      //Unit has died!
      //TODO: Fire event: Unit destroyed
      return -originalCount;
   }
   else
   {
      m_quantity = (uint32)(originalCount + quantity);
   }
   return quantity;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Retrieves the number of units in this stack
*
*     return:     Number of units in the stack
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint32 Unit::Quantity() const
{
   return m_quantity;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Provides the caller's out-arguments with the amount of magical power that this unit stack will generate in one turn.
*        Note that it is possible for this amount to be negative, implying a form of magic based upkeep.
*
*    outArcane:   Output of the total amount of Arcane magic supplied/consumed by this unit stack (quantity included).
*
*      outHoly:   Output of the total amount of Holy magic supplied/consumed by this unit stack (quantity included).
*
*    outNature:   Output of the total amount of Nature magic supplied/consumed by this unit stack (quantity included).
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void Unit::GetMagicAccumulation(int32& outArcane, int32& outHoly, int32& outNature) const
{
   const RaceAndClass& rac = GetRaceAndClass(m_type);
   outArcane = rac.arcane + rac.arcaneperlevel * m_level;
   outHoly = rac.holy + rac.holyperlevel * m_level;
   outNature = rac.nature + rac.natureperlevel * m_level;

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Retrieves the amount of damage the unit stack (quantity included) will deal in combat.
*
*     return:     Attack damage of the unit stack (quantity included)
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint32 Unit::GetAttackDamage() const
{
   const RaceAndClass& rac = GetRaceAndClass(m_type);
   float hungerEffect = 1 - m_hunger * FightingPenaltyWhileHungry;
   return hungerEffect * (rac.damage + rac.damageperlevel * m_level) * m_quantity;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Retrieves the current state of inefficiency of the unit suffers from due to lack of proper upkeep, turn over turn.
*
*     return:     The hunger of the unit, from 0.0 (well fed) to 1.0 (complete starvation)
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
float Unit::GetHunger() const
{
   return m_hunger;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Retrieves the amount of upkeep (food) the unit stack (quantity included) consumes per turn. 
*
*     return:     Amount of upkeep (food) the unit stack (quantity included) will consume per turn.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint32 Unit::GetUpkeep() const
{
   const RaceAndClass& rac = GetRaceAndClass(m_type);
   uint32 upkeep = (rac.upkeep + (m_level * rac.upkeepperlevel)) * m_quantity;
   return upkeep;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Provide food supplies to the unit stack (pays their upkeep). The unit will consume up to their total upkeep value and 
*        return the unused supplies. If they are under-supplied, then a level of hunger will be assigned to the unit which 
*        which affect their performance. Hunger LERPs toward starvation/well-fed over time
*
*    foodstuffs:  The total amount of 'supply' being offered to the unit this game-turn.
*
*     return:     The input foodstuffs that the unit did not require. This should be returned to the caller's supply stores.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
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

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Retrieves the amount of health the unit stack (quantity included).
*
*     return:     Current health of the unit stack, if they are damaged, this will be reflected.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint32 Unit::GetTotalHealth() const
{
   
   const RaceAndClass& rac = GetRaceAndClass(m_type);
   uint32 totalHealth = (rac.health + (m_level * rac.healthperlevel)) * (m_quantity - 1) + m_health;
   return totalHealth;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    A unit's guid is a way to uniquely identify this particular unit apart from others that a player may own of the same type
*
*     return:     The guid of this unit
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
UnitGUID Unit::GetGuid() const
{
   return m_guid;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Retrieves the amount of XP a unit of this type and level is worth, and optionally, how much the whole unit stack is worth
*
*    wholeStack:  if true, the XP value returned will reflect the sum total XP value of all units (quantity) in this stack
*
*     return:     The amount of XP the unit is worth when killed*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint32 Unit::GetXPValue(bool wholeStack) const
{
   const RaceAndClass& rac = GetRaceAndClass(m_type);
   return (rac.xpvalue + m_level * rac.xpvalueperlevel) * (wholeStack ? m_quantity : 1);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Retrieves the rank (aka row order in combat) of this unit
*
*     return:     The rank of the unit*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint32 Unit::GetRank() const
{
   const RaceAndClass& rac = GetRaceAndClass(m_type);
   return rac.rank;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Retrieves the current amount of experience the unit has
*
*     return:     The Experience total for the units
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
uint32 Unit::GetCurrentExperience() const
{
   return m_experience;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*     Local helper function, called from GetRaceAndClass to calculate the value of a stat, which takes into account
*       that the classes base stat may be overridden (or not) in a racial definition
*
*  orideValue:   the value stored in the racial override for this stat
*  classValue:   the value stored in the class for this stat
*      scalar:   the modifier value from the race which is multiplied by one of the above terms.
* unserializedValue: A special value which the race/class stats are initialized with, which represents that the data has not
*                    been changed through serialization, so shouldn't be used.
*
*        return:   the properly calculated and cast stat
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
template <typename T>
T GetOverridableStat(const T& orideValue, const T& classValue, const float& scalar, const T& unserializedValue)
{
   return (T)(((orideValue != unserializedValue) ? orideValue : classValue) * scalar);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *    Static method which will look up a unit's stat page, which accounts both for class and racial modifiers as one whole.
 *          It uses lazy calculation and caching. Therefore, the race/class calculations are done for each race/class on the
 *          first lookup, and then stored in the cache for future requests of the same race/class.
 *
 *       type:    The identifier of the unit whose stats you wish to look up. This is resilient to merged in level values.
 *
 *     return:    The cached universal data about this race/class/hero combination. 
 *                To avoid unnecessary copies, you should capture this return value as a 'const Unit::RaceAndClass &'
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
const Unit::RaceAndClass& Unit::GetRaceAndClass(UnitType type)
{
   //Only Race, Class and Hero bits are used, so we discard level
   type &= ~levelMask;
   RaceAndClass* cachedValuep = _______RaceAndClassCache_______.Find(type);
   if (cachedValuep == nullptr)
   {
      Race* racep = RaceDatabase.FindChecked(type & raceMask);
      Class* classp = ClassDatabase.FindChecked(type & (heroMask | classMask));
      //lookup for specific racial version of this class
      //if the class is not found, use generic racial bonus "any"
      Modifier** bonuspp = racep->modifiers.Find(classp->noun);
      Modifier* bonusp;
      if(bonuspp == nullptr)
      {
         bonusp = racep->modifiers.FindChecked(TEXT("any"));
      }
      else
      {
         bonusp = *bonuspp;
      }
      cachedValuep = &_______RaceAndClassCache_______.FindOrAdd(type);
      cachedValuep->classNoun = classp->noun;
      cachedValuep->classPlural = classp->plural;
      cachedValuep->classIcon = classp->icon;
      cachedValuep->raceNoun = racep->noun;
      cachedValuep->racePlural = racep->plural;
      cachedValuep->raceIcon = racep->icon;
      cachedValuep->raceAdjective = racep->adjective;
      cachedValuep->racePrevalence = racep->prevalence;
      cachedValuep->rank = classp->rank;
      cachedValuep->ishero = (classp->ishero == 1);

      //These can be overridden by the race, so we must test each one to see if it was
      //A helper function does the math since racial overrides can make it pretty messy
      Unit::Class* oride = &bonusp->classOverride;
      cachedValuep->health = GetOverridableStat(oride->health, classp->health, bonusp->health, unserializedUint32);
      cachedValuep->healthperlevel = GetOverridableStat(oride->healthperlevel, classp->healthperlevel, bonusp->healthperlevel, unserializedUint32);

      cachedValuep->damage = GetOverridableStat(oride->damage, classp->damage, bonusp->damage, unserializedUint32);
      cachedValuep->damageperlevel = GetOverridableStat(oride->damageperlevel, classp->damageperlevel, bonusp->damageperlevel, unserializedUint32);

      cachedValuep->recruittime = GetOverridableStat(oride->recruittime, classp->recruittime, bonusp->recruittime, unserializedUint32);
      cachedValuep->recruitcost = GetOverridableStat(oride->recruitcost, classp->recruitcost, bonusp->recruitcost, unserializedUint32);

      cachedValuep->upkeep = GetOverridableStat(oride->upkeep, classp->upkeep, bonusp->upkeep, unserializedUint32);
      cachedValuep->upkeepperlevel = GetOverridableStat(oride->upkeepperlevel, classp->upkeepperlevel, bonusp->upkeepperlevel, unserializedUint32);

      cachedValuep->xpvalue = GetOverridableStat(oride->xpvalue, classp->xpvalue, bonusp->xpvalue, unserializedUint32);
      cachedValuep->xpvalueperlevel = GetOverridableStat(oride->xpvalueperlevel, classp->xpvalueperlevel, bonusp->xpvalueperlevel, unserializedUint32);

      cachedValuep->arcane = GetOverridableStat(oride->arcane, classp->arcane, bonusp->arcane, unserializedInt32);
      cachedValuep->arcaneperlevel = GetOverridableStat(oride->arcaneperlevel, classp->arcaneperlevel, bonusp->arcaneperlevel, unserializedInt32);

      cachedValuep->holy = GetOverridableStat(oride->holy, classp->holy, bonusp->holy, unserializedInt32);
      cachedValuep->holyperlevel = GetOverridableStat(oride->holyperlevel, classp->holyperlevel, bonusp->holyperlevel, unserializedInt32);

      cachedValuep->nature = GetOverridableStat(oride->nature, classp->nature, bonusp->nature, unserializedInt32);
      cachedValuep->natureperlevel = GetOverridableStat(oride->natureperlevel, classp->natureperlevel, bonusp->natureperlevel, unserializedInt32);
   }
   return *cachedValuep;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Static method which loads the xml file containing all unit definitions (race, class, hero etc) 
*        This must be executed prior to using anything else, since it forms the basis for all the data a Unit will need.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void Unit::InitializeUnitDatabase()
{
   static bool initialized = false;
   check(!initialized);
   initialized = true;

   XmlUnitParser parser;
   FText errorMessage;
   int32 errorLineNumber;
   FFastXml::ParseXmlFile(&parser, *FPaths::Combine(*FPaths::GameContentDir(), TEXT("/content/unitdatabase.xml")), nullptr, nullptr, false, false, errorMessage, errorLineNumber);
   
   //Add serialized races to race database (take ownership of the pointers)
   for (auto node = parser.GetRaces()->GetHead(); node != nullptr; node = node->GetNextNode())
   {
      RaceDatabase.Add(GetNewRaceID(), node->GetValue());
   }
   
   //Add serialized classes to class database (take ownership of the pointers)
   for (auto node = parser.GetClasses()->GetHead(); node != nullptr; node = node->GetNextNode())
   {
      ClassDatabase.Add(GetNewClassID(), node->GetValue());
   }

   check(errorLineNumber == 0);
   check(errorMessage.EqualTo(FText()));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    Static constructor, also known as a factory method
*        Allocates a new Unit and ensures proper construction is performed before returning the pointer to the caller
*
*       type:    The unit type to create, which must exist in the race and class databases. The level bits are ignored unless
*                the 'level' variable is 0
*   quantity:    The number of UnitType dudes this unit stack should represent. Hero's must pass 1, regulars must pass  >= 1
*      level:    This set the level of the new unit. If 0, the new unit's level will be extracted from unitType.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
Unit* Unit::AllocateNewUnit(UnitStackType type, uint32 quantity, uint32 level)
{
   //if level is not provided, extract level from the unit type
   if (level == 0)
   {
      level = type >> (raceBits + classBits + heroBits);
      checkf(level > 0, TEXT("Unit::AllocateNewUnit was passed a unit type without an encoded level, and was also passed 0 in the level parameter. One of these must have a valid level to initialize the unit with.\n"));
      if (level == 0)
      {
         level = 1;
      }
   }
   type &= ~levelMask;
   uint32 allowedQuantity = (type & heroMask) != 0 ? 1 : quantity;
   check(allowedQuantity == quantity);
   UnitStackType stackType = type & ~levelMask;
   const RaceAndClass& rac = GetRaceAndClass(stackType);

   Unit* unit = new Unit();
   unit->m_type = type;
   unit->m_quantity = allowedQuantity;
   unit->m_level = level;
   unit->m_health = rac.health + rac.healthperlevel * level;
   unit->m_experience = 0;
   unit->m_hunger = 0;
   unit->m_guid = GetNewGuid();
   checkf(unit->m_guid < (uint32)-1, TEXT("Unit GUIDs ran out of unique values"));
   return unit;
}

