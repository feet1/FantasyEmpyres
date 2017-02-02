#include "PretendEmpyres.h"
#include "XmlUnitParser.h"

//see https://answers.unrealengine.com/questions/1859/log-issue-again.html?sort=oldest for details on log category
DEFINE_LOG_CATEGORY_STATIC(PretendEmpyers, All, All);


/////// Macros to support serialization, collapse the #pragma region to clean things up
#pragma region ReadNodeMacros
#define READNODE_FSTRING(_container_, _name_)                   \
   if (FString(ElementName) == FString(TEXT(#_name_)))          \
   {                                                            \
      _container_##->##_name_ = FString(ElementData);           \
      return true;                                              \
   }

#define READNODE_FLOAT(_container_, _name_)                  \
   if (FString(ElementName) == FString(TEXT(#_name_)))       \
   {                                                         \
      _container_##->##_name_ = FCString::Atof(ElementData); \
      return true;                                           \
   }

#define READNODE_BOOL(_container_, _name_)                                       \
   if (FString(ElementName) == FString(TEXT(#_name_)))                           \
   {                                                                             \
      if(FString(ElementData)!=FString(TEXT("true"))||FString(ElementData)!=FString(TEXT("false")))\
      {                                                                          \
         UE_LOG(PretendEmpyers, Warning, TEXT("XmlUnitParser A bool value is neither 'true' nor 'false'. Value:'%s'\n"), ElementData);\
      }                                                                          \
      _container_##->##_name_ = (FString(ElementData)==FString(TEXT("true")));   \
      return true;                                                               \
   }

#define READNODE_INT32(_container_, _name_)                  \
   if (FString(ElementName) == FString(TEXT(#_name_)))       \
   {                                                         \
      _container_##->##_name_ = FCString::Atoi(ElementData); \
      return true;                                           \
   }

#define READNODE_UINT32(_container_, _name_)                 \
   if (FString(ElementName) == FString(TEXT(#_name_)))       \
   {                                                         \
      int tmp____ = FCString::Atoi(ElementData);             \
      if(tmp____<0){UE_LOG(PretendEmpyers, Warning, TEXT("XmlUnitParser A negative integer (%d) is being converted to uint with Abs()."), tmp____);} \
      _container_##->##_name_ = FMath::Abs(tmp____);         \
      return true;                                           \
   }
#pragma endregion

//////////////////////////////////////////////////////////////////////////////////////////////////////////
///////                            START HERE                                                    /////////
///////                                                                                          /////////
///////           For changing how a race/bonus/class is serialized, such as                     /////////
///////               adding/removing a stat, or changing the type of one                        /////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

//==============================================================================================================
// Race serialization (non-racial bonuses)
//  This is called only after a new race has been created in currentRace, called once per non-bonus node under the race node
//==============================================================================================================
bool XmlUnitParser::ProcessElement_RaceStat(const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber)
{
   check(currentRace != nullptr);

   READNODE_FSTRING(currentRace, noun);
   READNODE_FSTRING(currentRace, plural);
   READNODE_FSTRING(currentRace, adjective);
   READNODE_FSTRING(currentRace, icon);
   READNODE_FLOAT(currentRace, prevalence);

   UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser::ProcessElement_RaceStat %s is not a recognized element of a Unit::Race (Line %d)\n"), ElementName, XmlFileLineNumber);
   return false;
}

//==============================================================================================================
// Racial bonus serialization
//  This is called after a new racial bonus is allocated in currentBonus, called once per node under the bonus node
//==============================================================================================================
bool XmlUnitParser::ProcessElement_Modifier(const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber)
{
   check(currentRace != nullptr);
   check(currentModifier != nullptr);

   READNODE_FSTRING(currentModifier, classnoun);
   READNODE_FSTRING(currentModifier, noun);
   READNODE_FSTRING(currentModifier, plural);
   READNODE_FLOAT(currentModifier, health);
   READNODE_FLOAT(currentModifier, healthperlevel);
   READNODE_FLOAT(currentModifier, damage);
   READNODE_FLOAT(currentModifier, damageperlevel);
   READNODE_FLOAT(currentModifier, recruittime);
   READNODE_FLOAT(currentModifier, recruitcost);
   READNODE_FLOAT(currentModifier, upkeep);
   READNODE_FLOAT(currentModifier, upkeepperlevel);
   READNODE_FLOAT(currentModifier, xpvalue);
   READNODE_FLOAT(currentModifier, xpvalueperlevel);
   READNODE_FLOAT(currentModifier, arcane);
   READNODE_FLOAT(currentModifier, arcaneperlevel);
   READNODE_FLOAT(currentModifier, holy);
   READNODE_FLOAT(currentModifier, holyperlevel);
   READNODE_FLOAT(currentModifier, nature);
   READNODE_FLOAT(currentModifier, natureperlevel);

   UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser::ProcessElement_RaceStat %s is not a recognized element of a Unit::RacialBonus (Line %d)\n"), ElementName, XmlFileLineNumber);
   return false;
}

//==============================================================================================================
// Race override value serialization
//  This is called once per override sub-node within an overrides node, which is part of the racial bonus node
// IMPORTANT: This is ultimately serializing a Unit::Class object, so it should MIRROR the serialization of a class
//==============================================================================================================
bool XmlUnitParser::ProcessElement_Override(const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber)
{
   check(currentRace != nullptr);
   check(currentModifier != nullptr);

   READNODE_FSTRING((&currentModifier->classOverride), noun);
   READNODE_FSTRING((&currentModifier->classOverride), plural);
   READNODE_FSTRING((&currentModifier->classOverride), icon);
   READNODE_UINT32((&currentModifier->classOverride), rank);
   READNODE_UINT32((&currentModifier->classOverride), health);
   READNODE_UINT32((&currentModifier->classOverride), healthperlevel);
   READNODE_UINT32((&currentModifier->classOverride), damage);
   READNODE_UINT32((&currentModifier->classOverride), damageperlevel);
   READNODE_UINT32((&currentModifier->classOverride), recruittime);
   READNODE_UINT32((&currentModifier->classOverride), recruitcost);
   READNODE_UINT32((&currentModifier->classOverride), upkeep);
   READNODE_UINT32((&currentModifier->classOverride), upkeepperlevel);
   READNODE_UINT32((&currentModifier->classOverride), xpvalue);
   READNODE_UINT32((&currentModifier->classOverride), xpvalueperlevel);
   READNODE_INT32((&currentModifier->classOverride), arcane);
   READNODE_INT32((&currentModifier->classOverride), arcaneperlevel);
   READNODE_INT32((&currentModifier->classOverride), holy);
   READNODE_INT32((&currentModifier->classOverride), holyperlevel);
   READNODE_INT32((&currentModifier->classOverride), nature);
   READNODE_INT32((&currentModifier->classOverride), natureperlevel);
   READNODE_INT32((&currentModifier->classOverride), ishero);

   UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser::ProcessElement_RaceOverride %s is not a recognized element of a Unit::Class (Line %d)\n"), ElementName, XmlFileLineNumber);
   return false;
}

//==============================================================================================================
// Class serialization
//  This is called after a new class is allocated in currentClass, called once per node under the class node
//==============================================================================================================
bool XmlUnitParser::ProcessElement_Class(const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber)
{
   check(currentClass != nullptr);

   READNODE_FSTRING(currentClass, noun);
   READNODE_FSTRING(currentClass, plural);
   READNODE_FSTRING(currentClass, icon);
   READNODE_UINT32(currentClass, rank);
   READNODE_UINT32(currentClass, health);
   READNODE_UINT32(currentClass, healthperlevel);
   READNODE_UINT32(currentClass, damage);
   READNODE_UINT32(currentClass, damageperlevel);
   READNODE_UINT32(currentClass, recruittime);
   READNODE_UINT32(currentClass, recruitcost);
   READNODE_UINT32(currentClass, upkeep);
   READNODE_UINT32(currentClass, upkeepperlevel);
   READNODE_UINT32(currentClass, xpvalue);
   READNODE_UINT32(currentClass, xpvalueperlevel);
   READNODE_INT32(currentClass, arcane);
   READNODE_INT32(currentClass, arcaneperlevel);
   READNODE_INT32(currentClass, holy);
   READNODE_INT32(currentClass, holyperlevel);
   READNODE_INT32(currentClass, nature);
   READNODE_INT32(currentClass, natureperlevel);
   READNODE_INT32(currentClass, ishero);

   UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser::ProcessElement_RaceStat %s is not a recognized element of a Unit::Class (Line %d)\n"), ElementName, XmlFileLineNumber);
   return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
///////                            STOP HERE                                                     /////////
///////                                                                                          /////////
///////           The rest of the file is the implementation details of serialization            /////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region ImplementationDetails
#undef READNODE_FSTRING
#undef READNODE_FLOAT 
#undef READNODE_BOOL
#undef READNODE_INT32
#undef READNODE_UINT32



#pragma region IFastXmlCallback
/**
* Called after the XML's header is parsed.  This is usually the first call that you'll get back.
*
* @param   ElementData         Optional data for this element, nullptr if none
* @param   XmlFileLineNumber   Line number in the XML file we're on
*
* @return   You should return true to continue processing the file, or false to stop processing immediately.
*/
bool XmlUnitParser::ProcessXmlDeclaration(const TCHAR* ElementData, int32 XmlFileLineNumber)
{
   UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser::ProcessXmlDeclaration ElementData:%s (line %d)\n"), ElementData, XmlFileLineNumber);
   //Verify PretendEmpyres tag
   if (FString(ElementData) == FString(TEXT("pretendempyre")))
   {
      depth = ProcessingDepth::pretendempyre;
      return true;
   }
   UE_LOG(PretendEmpyers, Error, TEXT("XmlUnitParser::ProcessXmlDeclaration '%s' is an invalid top-level node. Mal-formed xml input.\n"), ElementData);
   return false;
}

/**
* Called when a new XML element is encountered, starting a new scope.  You'll receive a call to ProcessClose()
* when this element's scope has ended.
*
* @param   ElementName         The name of the element
* @param   ElementData         Optional data for this element, nullptr if none
* @param   XmlFileLineNumber   The line number in the XML file we're on
*
* @return   You should return true to continue processing the file, or false to stop processing immediately.
*/
bool XmlUnitParser::ProcessElement(const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber)
{
   UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser::ProcessElement ElementName:%s  ElementData:%s (line %d)\n"), ElementName, ElementData, XmlFileLineNumber);
   //Verify either 'Classes' or 'Races' element, set mode accordingly
   bool result = false;
   switch (depth)
   {
   case ProcessingDepth::pretendempyre:
   {
      if (FString(ElementName) == FString(TEXT("races")))
      {
         depth = ProcessingDepth::races;
         result = true;
      }
      else if (FString(ElementName) == FString(TEXT("classes")))
      {
         depth = ProcessingDepth::classes;
         result = true;
      }
      break;
   }
   case ProcessingDepth::races:
   {
      if (FString(ElementName) == FString(TEXT("race")))
      {
         depth = ProcessingDepth::race;
         result = CreateNewRace();
      }
      break;
   }
   case ProcessingDepth::race:
   {
      if (FString(ElementName) == FString(TEXT("modifiers")))
      {
         depth = ProcessingDepth::modifiers;
         result = true;
      }
      else
      {
         result = ProcessElement_RaceStat(ElementName, ElementData, XmlFileLineNumber);
      }
      break;
   }
   case ProcessingDepth::modifiers:
   {
      if (FString(ElementName) == FString(TEXT("modifier")))
      {
         depth = ProcessingDepth::modifier;
         result = CreateNewBonus();
      }
      break;
   }
   case ProcessingDepth::modifier:
   {
      if (FString(ElementName) == FString(TEXT("overrides")))
      {
         depth = ProcessingDepth::overrides;
         result = true;
      }
      else
      {
         result = ProcessElement_Modifier(ElementName, ElementData, XmlFileLineNumber);
      }
      break;
   }
   case ProcessingDepth::overrides:
   {
      result = ProcessElement_Override(ElementName, ElementData, XmlFileLineNumber);
      break;
   }
   case ProcessingDepth::classes:
   {
      if (FString(ElementName) == FString(TEXT("class")))
      {
         depth = ProcessingDepth::aclass;
         result = CreateNewClass();
      }
      break;
   }
   case ProcessingDepth::aclass:
   {
      result = ProcessElement_Class(ElementName, ElementData, XmlFileLineNumber);
   }
   default:
      checkNoEntry();
   }//End Switch(depth)

   if (!result)
   {
      UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser::ProcessElement unexpected node: '%s' : '%s'. Depth: %02u\n"), ElementName, ElementData, (uint32)depth);
   }

   return result;
}

/**
* Called when an XML attribute is encountered for the current scope's element.
*
* @param   AttributeName   The name of the attribute
* @param   AttributeValue   The value of the attribute
*
* @return   You should return true to continue processing the file, or false to stop processing immediately.
*/
bool XmlUnitParser::ProcessAttribute(const TCHAR* AttributeName, const TCHAR* AttributeValue)
{
   UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser::ProcessAttribute AttributeName:%s  AttributeValue:%s\n"), AttributeName, AttributeValue);
   //forward to process race attribute or process classatribute
   bool result = false;
   switch (depth)
   {
   case ProcessingDepth::pretendempyre:   //Fallthrough, these nodes should have no attributes
   case ProcessingDepth::races:           //    |
   case ProcessingDepth::race:            //    |
   case ProcessingDepth::modifiers:       //    |
   case ProcessingDepth::modifier:        //    |
   case ProcessingDepth::overrides:       //    |
   case ProcessingDepth::classes:         //    |
   case ProcessingDepth::aclass:          //    v
      break;
   default:
      checkNoEntry();
   }//End Switch(depth)

   if (!result)
   {
      UE_LOG(PretendEmpyers, Error, TEXT("XmlUnitParser::ProcessAttribute an XML attribute was unexpected for current node.\n'%s'='%s'. Depth: %02u\n"), AttributeName, AttributeValue, (uint32)depth);
   }
   return result;
}


#pragma region ProcessCloseMacros
//Each PROCESS_BASIC_CLOSURE wraps boilerplate code that handles the switch case for going from child to parent depth during a ProcessClose call
#define PROCESS_BASIC_CLOSURE(_node_, _parent_)                                                                                                       \
case ProcessingDepth::##_node_:                                                                                                                       \
   if (FString(Element) != FString(TEXT(#_node_)))                                                                                                    \
   {                                                                                                                                                  \
      UE_LOG(PretendEmpyers, Error, TEXT("XmlUnitParser::ProcessClose '%s' closure expected but '%s' closure was found.\n"), TEXT(#_node_), Element); \
      break;                                                                                                                                          \
   }                                                                                                                                                  \
   depth = ProcessingDepth::##_parent_;                                                                                                               \
   result = true;                                                                                                                                     \
   break
//      ^  missing semi-colon is intentional to force use of the macro to require it

//Each PROCESS_DATA_CLOSURE wraps node closure where current depth has child-data nodes, but also may be the node itsef closing
#define PROCESS_DATA_CLOSURE(_node_, _parent_, _validator_) \
case ProcessingDepth::##_node_:                             \
   if (FString(Element) == FString(TEXT(#_node_)))          \
   {                                                        \
      depth = ProcessingDepth::##_parent_;                  \
      result = _validator_##();                             \
   }                                                        \
   else                                                     \
   {                                                        \
      result = true;                                        \
   }                                                        \
   break
//      ^  missing semicolon is intentional to force use of the macro to require it
#pragma endregion

/**
* Called when an element's scope ends in the XML file
*
* @param   ElementName      Name of the element whose scope closed
*
* @return   You should return true to continue processing the file, or false to stop processing immediately.
*/
bool XmlUnitParser::ProcessClose(const TCHAR* Element)
{
   UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser::ProcessClose::Element:%s\n"), Element);
   bool result = false;
   switch (depth)
   {
      PROCESS_BASIC_CLOSURE(races, pretendempyre);
      PROCESS_BASIC_CLOSURE(modifiers, race);
      PROCESS_BASIC_CLOSURE(classes, pretendempyre);
      PROCESS_BASIC_CLOSURE(overrides, modifier);
      PROCESS_DATA_CLOSURE(race, races, ValidateCurrentRace);
      PROCESS_DATA_CLOSURE(modifier, modifiers, ValidateCurrentModifier);
      PROCESS_DATA_CLOSURE(aclass, classes, ValidateCurrentClass);
      PROCESS_DATA_CLOSURE(pretendempyre, None, ValidateCompletedUnitRoster);
   case ProcessingDepth::None:
      UE_LOG(PretendEmpyers, Error, TEXT("XmlUnitParser::ProcessClose unexpected ProcessClose when already at depth of None\n"), Element);
      break;
   default:
      checkNoEntry();
   }//End Switch(depth)
   return result;
}
#undef PROCESS_DATA_CLOSURE
#undef PROCESS_BASIC_CLOSURE

/**
* Called when a comment is encountered.  This can happen pretty much anywhere in the file.
*
* @param   Comment      The comment text
*/
bool XmlUnitParser::ProcessComment(const TCHAR* Comment)
{
   //UE_LOG(PretendEmpyers, Log, TEXT("ProcessComment::Comment:%s\n"), Comment);
   return true;
}

#pragma endregion Code implementing the IFastXmlCallback interface

TDoubleLinkedList<Unit::Race*>* XmlUnitParser::GetRaces()
{
   return &raceList;
}

TDoubleLinkedList<Unit::Class*>* XmlUnitParser::GetClasses()
{
   return &classList;
}

bool XmlUnitParser::CreateNewClass()
{
   if (currentClass != nullptr)
   {
      checkf(false, TEXT("XmlUnitParser tried to create a new class, but the previous one was still open\n"));
      return false;
   }
   currentClass = new Unit::Class();
   return true;
}
bool XmlUnitParser::CreateNewBonus()
{
   if (currentModifier != nullptr)
   {
      checkf(false, TEXT("XmlUnitParser tried to create a new racial bonus, but the previous one was still open\n"));
      return false;
   }
   currentModifier = new Unit::Modifier();
   return true;
}
bool XmlUnitParser::CreateNewRace()
{
   if (currentRace != nullptr)
   {
      checkf(false, TEXT("XmlUnitParser tried to create a new race, but the previous one was still open\n"));
      return false;
   }
   currentRace = new Unit::Race();
   return true;
}

bool XmlUnitParser::ValidateCurrentRace()
{
   bool result = (currentRace != nullptr
      && currentModifier == nullptr
      && currentRace->noun != TEXT("")
      && currentRace->plural != TEXT("")
      && currentRace->adjective != TEXT("")
      && currentRace->icon != TEXT("")
      && currentRace->noun != unserializedString
      && currentRace->plural != unserializedString
      && currentRace->adjective != unserializedString
      && currentRace->icon != unserializedString
      && currentRace->prevalence != unserializedFloat
      && currentRace->modifiers.Contains(TEXT("any"))
      );

   if (result)
   {
      UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser::ValidateCurrentRace Successfully added race entry for %s\n"), *currentRace->plural);
      //Here's the important part of this function
      raceList.AddHead(currentRace);
      currentRace = nullptr;
   }
   else
   {
      TCHAR* errText = (currentRace == nullptr) ? TEXT("null race") : *currentRace->plural;
      UE_LOG(PretendEmpyers, Error, TEXT("XmlUnitParser::ValidateCurrentRace failed validation for %s. (%d racial bonus entries)\n"), errText, currentRace->modifiers.Num());
   }
   return result;
}

bool XmlUnitParser::ValidateCurrentModifier()
{
   bool result = (currentRace != nullptr
      && currentModifier != nullptr
      && currentModifier->classnoun != TEXT("")
      && currentModifier->noun != TEXT("")
      && currentModifier->plural != TEXT("")
      && currentModifier->classnoun != unserializedString
      && currentModifier->noun != unserializedString
      && currentModifier->plural != unserializedString
      && currentModifier->health != NAN
      && currentModifier->healthperlevel != NAN
      && currentModifier->damage != NAN
      && currentModifier->damageperlevel != NAN
      && currentModifier->recruittime != NAN
      && currentModifier->recruitcost != NAN
      && currentModifier->upkeep != NAN
      && currentModifier->upkeepperlevel != NAN
      && currentModifier->xpvalue != NAN
      && currentModifier->xpvalueperlevel != NAN
      && currentModifier->arcane != NAN
      && currentModifier->arcaneperlevel != NAN
      && currentModifier->holy != NAN
      && currentModifier->holyperlevel != NAN
      && currentModifier->nature != NAN
      && currentModifier->natureperlevel != NAN
      && !(currentRace->modifiers.Contains(currentModifier->classnoun))
      );


   if (result)
   {
      ValidateCurrentOverride();
      if (currentModifier->classnoun == TEXT("any"))
      {
         UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser::ValidateCurrentRacialBonus Successfully added base bonus stats for %s\n"), *currentRace->plural);
      }
      else
      {
         UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser::ValidateCurrentRacialBonus Successfully added racial class override. A %s %s is now a %s\n"), *currentRace->adjective, *currentModifier->classnoun, *currentModifier->noun);
      }
      //Here's the important part of this function
      currentRace->modifiers.Add(currentModifier->classnoun, currentModifier);
      currentModifier = nullptr;
   }
   else
   {
      if (currentRace == nullptr)
      {
         UE_LOG(PretendEmpyers, Error, TEXT("XmlUnitParser::ValidateCurrentRacialBonus failed validation currentRace is a nullptr"));
      }
      else if (currentModifier == nullptr)
      {
         UE_LOG(PretendEmpyers, Error, TEXT("XmlUnitParser::ValidateCurrentRacialBonus failed validation currentBonus is a nullptr"));
      }
      else
      {
         UE_LOG(PretendEmpyers, Error, TEXT("XmlUnitParser::ValidateCurrentRacialBonus failed validation \n %s %s (basic type %s.)"), *currentRace->adjective, *currentModifier->noun, *currentModifier->classnoun);
      }
      
   }
   return result;
}

void XmlUnitParser::ValidateCurrentOverride()
{

   //Validate the stuff that may have been set by the XML, but will be ignored
   // because they are class values that a race can not override directly
   // we wont fail to parse over these, but we will warn in the output log that the source file
   // is trying to do something stupid.
   if (currentModifier->classOverride.noun != unserializedString)
   {
      UE_LOG(PretendEmpyers, Warning, TEXT("XmlUnitParser::ValidateAnOverride The %s override for %s (a type of %s) has 'noun' in the override list\nThis is not allowed and will be ignored.\n"), *currentRace->adjective, *currentModifier->plural, *currentModifier->classnoun);
      currentModifier->classOverride.noun = unserializedString;
   }
   if (currentModifier->classOverride.plural != unserializedString)
   {
      UE_LOG(PretendEmpyers, Warning, TEXT("XmlUnitParser::ValidateAnOverride The %s override for %s (a type of %s) has 'plural' in the override list\nThis is not allowed and will be ignored.\n"), *currentRace->adjective, *currentModifier->plural, *currentModifier->classnoun);
      currentModifier->classOverride.plural = unserializedString;
   }
   if (currentModifier->classOverride.icon != unserializedString)
   {
      UE_LOG(PretendEmpyers, Warning, TEXT("XmlUnitParser::ValidateAnOverride The %s override for %s (a type of %s) has 'icon' in the override list\nThis is not allowed and will be ignored.\n"), *currentRace->adjective, *currentModifier->plural, *currentModifier->classnoun);
      currentModifier->classOverride.icon = unserializedString;
   }
   if (currentModifier->classOverride.ishero != unserializedInt32)
   {
      UE_LOG(PretendEmpyers, Warning, TEXT("XmlUnitParser::ValidateAnOverride The %s override for %s (a type of %s) has 'ishero' in the override list\nThis is not allowed and will be ignored.\n"), *currentRace->adjective, *currentModifier->plural, *currentModifier->classnoun);
      currentModifier->classOverride.ishero = unserializedInt32;
   }
   if (currentModifier->classOverride.rank != unserializedUint32)
   {
      UE_LOG(PretendEmpyers, Warning, TEXT("XmlUnitParser::ValidateAnOverride The %s override for %s (a type of %s) has 'rank' in the override list\nThis is not allowed and will be ignored.\n"), *currentRace->adjective, *currentModifier->plural, *currentModifier->classnoun);
      currentModifier->classOverride.ishero = unserializedUint32;
   }
}

bool XmlUnitParser::ValidateCurrentClass()
{
   bool result = (currentClass != nullptr
      && currentClass->noun != TEXT("")
      && currentClass->plural != TEXT("")
      && currentClass->icon != TEXT("")
      && currentClass->health > 0
      && currentClass->recruittime > 0
      && currentClass->rank != unserializedUint32
      && currentClass->health != unserializedUint32
      && currentClass->healthperlevel != unserializedUint32
      && currentClass->damage != unserializedUint32
      && currentClass->damageperlevel != unserializedUint32
      && currentClass->recruittime != unserializedUint32
      && currentClass->recruitcost != unserializedUint32
      && currentClass->upkeep != unserializedUint32
      && currentClass->upkeepperlevel != unserializedUint32
      && currentClass->xpvalue != unserializedUint32
      && currentClass->xpvalueperlevel != unserializedUint32
      && currentClass->arcane != unserializedInt32
      && currentClass->arcaneperlevel != unserializedInt32
      && currentClass->holy != unserializedInt32
      && currentClass->holyperlevel != unserializedInt32
      && currentClass->nature != unserializedInt32
      && currentClass->natureperlevel != unserializedInt32
      && currentClass->ishero != unserializedInt32
      && currentClass->rank <= Unit::RearmostRank
      );

   if (result)
   {
      UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser::ValidateCurrentClass Successfully added class entry for %s\n"), *currentClass->plural);
      //Here's the important part of this function
      classList.AddHead(currentClass);
      currentClass = nullptr;
   }
   else
   {
      TCHAR* errText = (currentClass == nullptr) ? TEXT("null class") : *currentClass->plural;
      UE_LOG(PretendEmpyers, Error, TEXT("XmlUnitParser::ValidateCurrentClass failed validation for %s\n"), errText);
   }
   return result;
}

bool XmlUnitParser::ValidateCompletedUnitRoster()
{
   bool result = (currentClass == nullptr
      && currentRace == nullptr
      && currentModifier == nullptr
      && raceList.Num() > 0
      && classList.Num() > 0
      );

   //loop through the races and validate that any racial modifiers that specialize a class references a class that actually exists
   for (auto raceNode = raceList.GetHead(); raceNode != nullptr; raceNode = raceNode->GetNextNode())
   {
      //Iterate the modifiers in this race and make sure it has a matching class
      for (auto modifierIterator = raceNode->GetValue()->modifiers.CreateIterator(); !modifierIterator; ++modifierIterator)
      {
         //Skip the "any" modifier
         if (modifierIterator->Value->classnoun == FString(TEXT("any")))
         {
            continue;
         }
         bool foundBaseClass = false;
         //Search through available classes for a match to this racial specialization
         for (auto classNode = classList.GetHead(); classNode != nullptr; classNode = classNode->GetNextNode())
         {
            if (classNode->GetValue()->noun == modifierIterator->Value->classnoun)
            {
               //we found the class that matches this specialModifier
               foundBaseClass = true;
               break;
            }
         }
         
         if (foundBaseClass)
         {
            continue; //Proceed to next specialization
         }
         //At this point we have not found the baseClass of an existing racial specialization
         UE_LOG(PretendEmpyers, Warning, TEXT("XmlUnitParser::ValidateCompletedUnitRoster() %s %s are declared to be a class modifier for the %s class, but the %s class was not serialized.\n"), *raceNode->GetValue()->adjective, *modifierIterator->Value->plural, *modifierIterator->Value->classnoun, *modifierIterator->Value->classnoun);
         result = false;
      }
   }

   if (result)
   {
      UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser Successfully serialized %d classes, and %d races\n"), classList.Num(), raceList.Num());
   }
   else
   {
      UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser completed in an invalid state.\nCurrent pointers all null: %s\nSerialized %d classes, and %d races\n"), ((currentClass == nullptr&& currentRace == nullptr&& currentModifier == nullptr) ? TEXT("True") : TEXT("False")), classList.Num(), raceList.Num());
   }
   return result;

}
#pragma endregion
