//  ***************************************************************
//  GameLogicComponent - Creation date: 02/05/2019
//  -------------------------------------------------------------
//  Robinson Technologies Copyright (C) 2019 - All Rights Reserved
//
//  ***************************************************************
//  Programmer(s):  Seth A. Robinson (seth@rtsoft.com)
//  ***************************************************************

#ifndef GameLogicComponent_h__
#define GameLogicComponent_h__

#include "Entity/Component.h"
#include "Entity/Entity.h"

#include "BlockerManager.h"

class GameLogicComponent : public EntityComponent
{
public:
	GameLogicComponent();
	virtual ~GameLogicComponent();

	void SetupStepEnt(Entity *pEnt);
	void SetSlideOverlayPosition();
	void LowerSlideScreen();
	void RaiseSlideScreen();
	virtual void OnAdd(Entity *pEnt);

	void UpdateEnergyText();
	void OnEnergyChanged(int energy, int oldEnergy);
	int GetSecondsWithScreenDown();
	void AdvanceStepAnim();
	void OnUpdate(VariantList *pVList);


	void OnRender(VariantList *pVList);
	int m_energyToLowerScreen = -100;

private:

	bool m_slideScreenDown = false;
	Entity *m_pStep1 = NULL;
	Entity *m_pStep2 = NULL;

	unsigned int m_animTimer = 0;
	bool m_showStep1 = true;
	Entity *m_pEnergyEnt = NULL;
	Entity *m_pDebugEnt = NULL;
	Entity *m_pSlideOverlay = NULL;
	BlockerManager m_blockerManager;
	int m_secondsBetweenBlocks = 10;
	unsigned int m_screenDownTimer = 0;
};

#endif // GameLogicComponent_h__