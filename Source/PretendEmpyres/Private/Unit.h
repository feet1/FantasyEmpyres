#pragma once

// A UnitType is a special identifier that encodes the level, race, class and hero status of a unit into an integer
// to simplify unit comparisons, among other things
// We split the bits of UnitType into [Level][Race][Class][ishero]
typedef uint64 UnitType;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*    CLASS Unit
*
*   The unit class encapsulates all behavior related to an individual "soldier" in the player (or computer) army.
*   The Unit is typically a member of an Army object (Army.h), and not directly controlled, but rather through the Army.
*   Units Race and Class definitions are serialized by the XmlUnitParser.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
class PRETENDEMPYRES_API Unit
{
   //// Type Definitions
   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
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
      bool ishero;
   };

private:
   friend class XmlUnitParser; //Allows parser to serialize Class/Modifier/Race definitions
   struct Class
   {
      //default constructor explicitly sets all values to special 'uninitialized' values to allow serialization to check for mistakes
      Class()
         : noun(unserializedString)
         , plural(unserializedString)
         , icon(unserializedString)
         , rank(unserializedUint32)
         , health(unserializedUint32)
         , healthperlevel(unserializedUint32)
         , damage(unserializedUint32)
         , damageperlevel(unserializedUint32)
         , recruittime(unserializedUint32)
         , recruitcost(unserializedUint32)
         , upkeep(unserializedUint32)
         , upkeepperlevel(unserializedUint32)
         , xpvalue(unserializedUint32)
         , xpvalueperlevel(unserializedUint32)
         , arcane(unserializedInt32)
         , arcaneperlevel(unserializedInt32)
         , holy(unserializedInt32)
         , holyperlevel(unserializedInt32)
         , nature(unserializedInt32)
         , natureperlevel(unserializedInt32)
         , ishero(unserializedInt32)
      { }
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
      int32 ishero; //this is a bool stored as int to allow us to detect if it is being specified in the XML or not
   };
   struct Modifier
   {
      //default constructor explicitly sets all values to special 'uninitialized' values to allow serialization to check for mistakes
      Modifier() 
         : classnoun(unserializedString)
         , noun(unserializedString)
         , plural(unserializedString)
         , health(unserializedFloat)
         , healthperlevel(unserializedFloat)
         , damage(unserializedFloat)
         , damageperlevel(unserializedFloat)
         , recruittime(unserializedFloat)
         , recruitcost(unserializedFloat)
         , upkeep(unserializedFloat)
         , upkeepperlevel(unserializedFloat)
         , arcane(unserializedFloat)
         , arcaneperlevel(unserializedFloat)
         , holy(unserializedFloat)
         , holyperlevel(unserializedFloat)
         , nature(unserializedFloat)
         , natureperlevel(unserializedFloat)
      { }
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
      float xpvalue;
      float xpvalueperlevel;
      float arcane;
      float arcaneperlevel;
      float holy;
      float holyperlevel;
      float nature;
      float natureperlevel;
      Class classOverride;
   };
   struct Race
   {
      //default constructor explicitly sets all values to special 'uninitialized' values to allow serialization to check for mistakes
      Race() 
         : noun(unserializedString)
         , plural(unserializedString)
         , adjective(unserializedString)
         , icon(unserializedString)
         , prevalence(unserializedFloat)
      { }
      FString noun;
      FString plural;
      FString adjective;
      FString icon;
      float prevalence;
      TMap<FString /*class noun*/, Modifier*> modifiers;
   };

   //// Instance Definitions
   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
   ~Unit();
   UnitType GetUnitType() const;
   bool IsHero() const;
   uint32 ModifyHealth(int32 amount);   //returns # units killed
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

private:
      Unit();              //private default CTOR, call AllocateNewUnit to make units
      Unit(const Unit&);   //unimplemented copy CTOR

//// Static Definitions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
   static const UnitType raceBits;
   static const UnitType classBits;
   static const UnitType heroBits;
   static const UnitType levelBits;
   static const UnitType raceMask;
   static const UnitType classMask;
   static const UnitType heroMask;
   static const UnitType fullClassMask;
   static const UnitType levelMask;
   static const uint32 RearmostRank;
   static const uint32 HeroMaxLevel;
   static const uint32 RegularMaxLevel;
   static const uint32 ExperiencePerLevel;
   static const float UnitsLostToStarvationPercent;
   static const float FightingPenaltyWhileHungry;
   static const float HowMuchHarderYouGetHitWhileHungry;

   static const RaceAndClass& GetRaceAndClass(UnitType type);
   static void InitializeUnitDatabase();
   static Unit* AllocateNewUnit(UnitType _type, uint32 _quantity = 1, uint32 _level = 0);

private:
   static TMap<UnitType /* race bits of UnitType*/,Race*> RaceDatabase;
   static TMap<UnitType /* class bits of UnitType*/, Class*> ClassDatabase;
   static TMap<UnitType, RaceAndClass> _______RaceAndClassCache_______;
};

