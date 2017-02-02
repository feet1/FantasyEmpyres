#pragma once
#include "FastXml.h"
#include "Unit.h"

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
   bool ProcessElement_Modifier(const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber);
   bool ProcessElement_Override(const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber);
   bool ProcessElement_Class(const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber);

   bool ValidateCurrentRace();
   bool ValidateCurrentModifier();
   void ValidateCurrentOverride();
   bool ValidateCurrentClass();
   bool ValidateCompletedUnitRoster();

private:
   TDoubleLinkedList<Unit::Race*> raceList;
   TDoubleLinkedList<Unit::Class*> classList;
   Unit::Race* currentRace;
   Unit::Class* currentClass;
   Unit::Modifier* currentModifier;

private:
   enum ProcessingDepth    //    ProcessingDepth Schema:
   {                       //
      None,                //  None
      pretendempyre,       //  |--PretendEmpyre
      races,               //     |--Races
      race,                //     |  |--Race
      modifiers,           //     |     |--Bonuses
      modifier,            //     |       |--Bonus
      overrides,           //     |       |--Overrides
      classes,             //     |--Classes
      aclass               //       |--Class
   };
   ProcessingDepth depth;
};