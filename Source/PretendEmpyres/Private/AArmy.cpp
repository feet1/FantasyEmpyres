// Fill out your copyright notice in the Description page of Project Settings.

#include "PretendEmpyres.h"
#include "AArmy.h"
#include "Army.h"

// Sets default values
AArmy::AArmy()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
   m_army = new Army();

}

AArmy::~AArmy()
{
   delete m_army;
   m_army = nullptr;
}

// Called when the game starts or when spawned
void AArmy::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AArmy::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

