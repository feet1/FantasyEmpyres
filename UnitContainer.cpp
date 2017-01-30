// Fill out your copyright notice in the Description page of Project Settings.

#include "PretendEmpyres.h"
#include "Unit.h"
//using statement for TLinkedList

UnitContainer::UnitContainer()
{
	food = 1.0f;
	stack = new UnitStack[30/*number of possible level/race/class combinations*/];
	xp = new unsigned[30 /*number of poss lvl/rc/cl combos*/];
	list = new TlinkedList<HeroUnit>();
	numberOfUnits = 0;
}

UnitContainer::~UnitContainer()
{
}

int UnitContainer::CostToFeed()
{
	return 0; //calculate the cost of feeding a unit based on Unit_Stack total
}
void UnitContainer::ModifyUnit(UnitType _type, int _unitChange) //add or subtract with signed int
{
	if (_unitChange > 0)
	{
		if (UnitStack[_type] == 0)
		{
			++numberOfUnits;
		}
		UnitStack[_type] += _unitChange;
	}
	else if (_unitChange < 0)
	{
		if (-_unitChange < UnitStack[_type]) //verify the unit stack isn't going negative
		{
			if (-_unitChange == UnitStack[_type])
			{
				--numberOfUnits;
			}
			UnitStack[_type] += _unitChange;
		}
	}
	//else THROW error
}
void UnitContainer::RemoveHero(*HeroUnit _hero);
{
	if (list.IsLinked(_hero))
	{
		list.Unlink(_hero);
	}
	//else THROW
}
void UnitContainer::AddHero(HeroUnit*_hero)
{
	if (!heroList.IsLinked(_hero))
	{
		heroList.LinkHead(_hero);
	}
	//else THROW
}
void UnitContainer::AddExperience(unsigned _xp);
{
	unsigned _portion = _xp / numberOfUnits;
	//iterate through units in the UnitStack that aren't 0 and add _portion
	//use TIterator to iterate through the hero list
	auto it =
		for (myIterator = Titerator.begin(); iterator != list.end(); iterator++)
		{
			if (iterator == _hero)
			{
				*iterator.AddExperience(_portion);
				return;
			}
		}
	//THROW error
}
void UnitContainer::ModifyFood(float _change)
{
	food += _change;
}