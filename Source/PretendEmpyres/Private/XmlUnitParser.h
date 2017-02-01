#pragma once
<<<<<<< HEAD
#include "Unit.h"

class Unit::Race;
class Unit::Class;
=======
#include "FastXml.h"
#include "Unit.h"
>>>>>>> origin/master

class PRETENDEMPYRES_API XmlUnitParser : public IFastXmlCallback
{
public:
	virtual bool ProcessXmlDeclaration(const TCHAR* ElementData, int32 XmlFileLineNumber) override;

	virtual bool ProcessElement(const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber) override;

	virtual bool ProcessAttribute(const TCHAR* AttributeName, const TCHAR* AttributeValue) override;

	virtual bool ProcessClose(const TCHAR* Element) override;

	virtual bool ProcessComment(const TCHAR* Comment) override;

	TDoubleLinkedList<Unit::Race*>* GetRaces();
	TDoubleLinkedList<Unit::Class*>* GetClasses();

private:

	bool CreateNewClass();
	bool CreateNewBonus();
	bool CreateNewRace();

	bool ProcessElement_RaceStat(const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber);
	bool ProcessElement_RaceBonus(const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber);
	bool ProcessElement_Class(const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber);

	bool ValidateCurrentRace();
	bool ValidateCurrentRacialBonus();
	bool ValidateCurrentClass();
	bool ValidateCompletedUnitRoster();

private:
	TDoubleLinkedList<Unit::Race*> raceList;
	TDoubleLinkedList<Unit::Class*> classList;
	Unit::Race* currentRace;
	Unit::Class* currentClass;
	Unit::RacialBonus* currentBonus;

private:
	enum ProcessingDepth    //    ProcessingDepth Schema:
	{                       //
		None,                //  None
		pretendempyre,       //  |--PretendEmpyre
		races,               //     |--Races
		race,                //     |  |--Race
		bonuses,             //     |     |--Bonuses
		bonus,               //     |        |--Bonus
		classes,             //     |--Classes
		aclass                //       |--Class
	};
	ProcessingDepth depth;
};