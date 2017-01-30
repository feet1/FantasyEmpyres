// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
class UnitType {};
class HeroUnit {};
/**
 * 
 */
class PRETENDEMPYRES_API UnitContainer
{
public:
	UnitContainer();
	~UnitContainer();
	UINT32 CostToFeed();
	unsigned numberOfUnits;
	void ModifyUnit(UnitType, int); //add or subtract with signed int
	void RemoveHero(HeroUnit*);
	void AddHero(HeroUnit*);
	void AddExperience(unsigned);
	void ModifyFood(float);
	UnitStack[] stack; //[UnitType]
	TLinkedList<HeroUnit*> heroList;
	float food;
	void AddExperience(unsigned); //adds experience to the unit either evenly or weighted based on level / hero vs regular
								  //possibly add something that indicates army strength, change icons, etc

};
