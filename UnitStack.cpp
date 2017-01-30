// Fill out your copyright notice in the Description page of Project Settings.
/*
#include "PretendEmpyres.h"
#include "Unit.h"


UnitStack::UnitStack(UnitType _type=0, uint32 _quantity=0, _unit=void)
	:quantity(_quantity),experience(0),unitType(_type)
{

}
UnitStack::~UnitStack()
{

}
UnitType UnitStack::GetUnitType() const
{

}
uint32 UnitStack::GetUpkeep() const
{

}
uint32 UnitStack::GetDamage() const
{

}
uint32 UnitStack::GetHP() const
{

}
void UnitStack::ReceiveDamage(uint32 _damage)
{

}
void UnitStack::ModifyStack(int32 _quantity)
{
	if (_quantity < 0)
	{
		if (!_quantity > quantity)
		{
			//THROW error
			return;
		}
	}
	quantity += _quantity;
	
}
void UnitStack::GrantExperience(uint32 _xp)
{
	experience += _xp;
}
bool UnitStack::IsHero() const
{

}


void UnitStack::SetUnitType(UnitType type)
{
	//todo WARN if existing type is NOT zero
}