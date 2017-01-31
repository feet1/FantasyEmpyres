#include "PretendEmpyres.h"
#include "FastXml.h"
#include "XmlParser.h"
#include "Unit.h"
#include "XmlUnitParser.h"

//see https://answers.unrealengine.com/questions/1859/log-issue-again.html?sort=oldest for details on log category
DEFINE_LOG_CATEGORY_STATIC(PretendEmpyers, All, All);

#pragma region IFastXmlCallback
/**
* Called after the XML's header is parsed.  This is usually the first call that you'll get back.
*
* @param	ElementData			Optional data for this element, nullptr if none
* @param	XmlFileLineNumber	Line number in the XML file we're on
*
* @return	You should return true to continue processing the file, or false to stop processing immediately.
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
* @param	ElementName			The name of the element
* @param	ElementData			Optional data for this element, nullptr if none
* @param	XmlFileLineNumber	The line number in the XML file we're on
*
* @return	You should return true to continue processing the file, or false to stop processing immediately.
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
		if (FString(ElementName) == FString(TEXT("bonuses")))
		{
			depth = ProcessingDepth::bonuses;
			result = true;
		}
		else
		{
			result = ProcessElement_RaceStat(ElementName, ElementData, XmlFileLineNumber);
		}
		break;
	}
	case ProcessingDepth::bonuses:
	{
		if (FString(ElementName) == FString(TEXT("bonus")))
		{
			depth = ProcessingDepth::bonus;
			result = CreateNewBonus();
		}
		break;
	}
	case ProcessingDepth::bonus:
	{
		result = ProcessElement_RaceBonus(ElementName, ElementData, XmlFileLineNumber);
		break;
	}
	case ProcessingDepth::classes:
	{
		if (FString(ElementName) == FString(TEXT("aclass")))
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
* @param	AttributeName	The name of the attribute
* @param	AttributeValue	The value of the attribute
*
* @return	You should return true to continue processing the file, or false to stop processing immediately.
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
	case ProcessingDepth::bonuses:         //    |
	case ProcessingDepth::bonus:           //    |
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
//      ^  missing semi-colan is intentional to force use of the macro to require it

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
* @param	ElementName		Name of the element whose scope closed
*
* @return	You should return true to continue processing the file, or false to stop processing immediately.
*/
bool XmlUnitParser::ProcessClose(const TCHAR* Element)
{
	UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser::ProcessClose::Element:%s\n"), Element);
	bool result = false;
	switch (depth)
	{
		PROCESS_BASIC_CLOSURE(races, pretendempyre);
		PROCESS_BASIC_CLOSURE(bonuses, race);
		PROCESS_BASIC_CLOSURE(classes, pretendempyre);
		PROCESS_DATA_CLOSURE(race, races, ValidateCurrentRace);
		PROCESS_DATA_CLOSURE(bonus, bonuses, ValidateCurrentRacialBonus);
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
* @param	Comment		The comment text
*/
bool XmlUnitParser::ProcessComment(const TCHAR* Comment)
{
	UE_LOG(PretendEmpyers, Log, TEXT("ProcessComment::Comment:%s\n"), Comment);
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
	if (currentBonus != nullptr)
	{
		checkf(false, TEXT("XmlUnitParser tried to create a new racial bonus, but the previous one was still open\n"));
		return false;
	}
	currentBonus = new Unit::RacialBonus();
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

//This is called only after a new race has been created in currentRace, called once per non-bonus node under the race node
bool XmlUnitParser::ProcessElement_RaceStat(const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber)
{
	check(currentRace != nullptr);

	READNODE_FSTRING(currentRace, noun);
	READNODE_FSTRING(currentRace, plural);
	READNODE_FSTRING(currentRace, adjective);
	READNODE_FSTRING(currentRace, icon);
	READNODE_FLOAT(currentRace, prevalence);

	UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser::ProcessElement_RaceStat %s is not a recognized element of a UnitRace (Line %d)\n"), ElementName, XmlFileLineNumber);
	return false;
}

//This is called after a new racial bonus is allocated in currentBonus, called once per node under the bonus node
bool XmlUnitParser::ProcessElement_RaceBonus(const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber)
{
	check(currentRace != nullptr);
	check(currentBonus != nullptr);

	READNODE_FSTRING(currentBonus, classnoun);
	READNODE_FSTRING(currentBonus, noun);
	READNODE_FSTRING(currentBonus, plural);
	READNODE_FLOAT(currentBonus, health);
	READNODE_FLOAT(currentBonus, healthperlevel);
	READNODE_FLOAT(currentBonus, damage);
	READNODE_FLOAT(currentBonus, damageperlevel);
	READNODE_FLOAT(currentBonus, recruittime);
	READNODE_FLOAT(currentBonus, recruittime);
	READNODE_FLOAT(currentBonus, upkeep);
	READNODE_FLOAT(currentBonus, upkeepperlevel);
	READNODE_FLOAT(currentBonus, arcane);
	READNODE_FLOAT(currentBonus, arcaneperlevel);
	READNODE_FLOAT(currentBonus, holy);
	READNODE_FLOAT(currentBonus, holyperlevel);
	READNODE_FLOAT(currentBonus, nature);
	READNODE_FLOAT(currentBonus, natureperlevel);

	UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser::ProcessElement_RaceStat %s is not a recognized element of a UnitRace (Line %d)\n"), ElementName, XmlFileLineNumber);
	return false;
}

//This is called after a new class is allocaed in currentClass, called once per node under the class node
bool XmlUnitParser::ProcessElement_Class(const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber)
{
	check(currentClass != nullptr);

	READNODE_FSTRING(currentClass, noun);
	READNODE_FSTRING(currentClass, plural);
	READNODE_FSTRING(currentClass, icon);
	READNODE_BOOL(currentClass, canbehero);
	READNODE_BOOL(currentClass, canberegular);
	READNODE_UINT32(currentClass, rank);
	READNODE_UINT32(currentClass, health);
	READNODE_UINT32(currentClass, healthperlevel);
	READNODE_UINT32(currentClass, damage);
	READNODE_UINT32(currentClass, damageperlevel);
	READNODE_UINT32(currentClass, recruittime);
	READNODE_UINT32(currentClass, recruitcost);
	READNODE_UINT32(currentClass, upkeep);
	READNODE_UINT32(currentClass, upkeepperlevel);
	READNODE_UINT32(currentClass, arcane);
	READNODE_UINT32(currentClass, arcaneperlevel);
	READNODE_UINT32(currentClass, holy);
	READNODE_UINT32(currentClass, holyperlevel);
	READNODE_UINT32(currentClass, nature);
	READNODE_UINT32(currentClass, natureperlevel);

	UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser::ProcessElement_RaceStat %s is not a recognized element of a UnitRace (Line %d)\n"), ElementName, XmlFileLineNumber);
	return false;
}

bool XmlUnitParser::ValidateCurrentRace()
{
	bool result = (currentRace != nullptr
		&& currentBonus == nullptr
		&& currentRace->noun != TEXT("")
		&& currentRace->plural != TEXT("")
		&& currentRace->adjective != TEXT("")
		&& currentRace->icon != TEXT("")
		&& currentRace->bonuses.Contains(TEXT("any"))
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
		UE_LOG(PretendEmpyers, Error, TEXT("XmlUnitParser::ValidateCurrentRace failed validation for %s. (%d racial bonus entries)\n"), errText, currentRace->bonuses.Num());
	}
	return result;
}

bool XmlUnitParser::ValidateCurrentRacialBonus()
{
	bool result = (currentRace != nullptr
		&& currentBonus != nullptr
		&& currentBonus->classnoun != TEXT("")
		&& currentBonus->noun != TEXT("")
		&& currentBonus->plural != TEXT("")
		&& !(currentRace->bonuses.Contains(currentBonus->classnoun))
		);

	if (result)
	{
		if (currentBonus->classnoun == TEXT("any"))
		{
			UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser::ValidateCurrentRacialBonus Successfully added base bonus stats for %s\n"), *currentRace->plural);
		}
		else
		{
			UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser::ValidateCurrentRacialBonus Successfully added racial class override. A %s %s is now a %s\n"), *currentRace->adjective, *currentBonus->classnoun, *currentBonus->noun);
		}
		//Here's the important part of this function
		currentRace->bonuses.Add(currentBonus->classnoun, currentBonus);
		currentBonus = nullptr;
	}
	else
	{
		if (currentRace == nullptr)
		{
			UE_LOG(PretendEmpyers, Error, TEXT("XmlUnitParser::ValidateCurrentRacialBonus failed validation currentRace is a nullptr"));
		}
		else if (currentBonus == nullptr)
		{
			UE_LOG(PretendEmpyers, Error, TEXT("XmlUnitParser::ValidateCurrentRacialBonus failed validation currentBonus is a nullptr"));
		}
		else
		{
			UE_LOG(PretendEmpyers, Error, TEXT("XmlUnitParser::ValidateCurrentRacialBonus failed validation \n %s %s (basic type %s.)"), *currentRace->adjective, *currentBonus->noun, *currentBonus->classnoun);
		}
		
	}
	return result;
}

bool XmlUnitParser::ValidateCurrentClass()
{
	bool result = (currentClass != nullptr
		&& currentClass->noun != TEXT("")
		&& currentClass->plural != TEXT("")
		&& currentClass->icon != TEXT("")
		&& (currentClass->canbehero || currentClass->canberegular)
		&& currentClass->health > 0
		&& currentClass->recruittime > 0
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
		&& currentBonus == nullptr
		&& raceList.Num() > 0
		&& classList.Num() > 0
		);
	if (result)
	{
		UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser Successfully serialized %d classes, and %d races\n"), classList.Num(), raceList.Num());
	}
	else
	{
		UE_LOG(PretendEmpyers, Log, TEXT("XmlUnitParser completed in an invalid state.\nCurrent pointers all null: %s\nSerialized %d classes, and %d races\n"), ((currentClass == nullptr&& currentRace == nullptr&& currentBonus == nullptr) ? TEXT("True") : TEXT("False")), classList.Num(), raceList.Num());
	}
	return result;

}
