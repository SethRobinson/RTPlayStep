/*
 *  App.h
 *  Created by Seth Robinson on 3/6/09.
 *  For license info, check the license.txt file that should have come with this.
 *
 */

#pragma once

#include "BaseApp.h"
#include "Manager/WiringPiManager.h"

class GameLogicComponent;

class App: public BaseApp
{
public:
	
	App();
	virtual ~App();
	
	virtual bool Init();
	virtual void Kill();
	virtual void Draw();
	virtual void OnScreenSizeChange();
	virtual void OnEnterBackground();
	virtual void OnEnterForeground();
	virtual bool OnPreInitVideo();
	virtual void Update();
	void OnExitApp(VariantList *pVarList);
		
	//we'll wire these to connect to some signals we care about
	void OnAccel(VariantList *pVList);
	void OnArcadeInput(VariantList *pVList);
	void SetGPIO();
	void UpdateLCD();
	void OnEnergyChanged(int originalEnergy);
	void GPIOThink();
	WiringPiManager *m_pWiringPi;

	bool m_bTestMode = false;
	int m_energy = 0;
	GameLogicComponent *m_pGameLogicComp = NULL;

	bool m_bDidPostInit;
	Surface m_surf; //for testing
	bool m_blinkOn;
	int m_energyTimer;
	int m_ms_perEnergy = 1000;
	int m_beepTimer = 0;
	int m_beepInterval = 2000;
	int m_maxEnergy = 60 * 10;

#ifdef WINAPI
	int m_energyPerMove = 10;
#else
	int m_energyPerMove = 10;
#endif

	bool m_bLeftFootDown = false;
	bool m_bRightFootDown = false;
	bool m_bLastPointWasLeftFoot = false;

};


App * GetApp();
const char * GetAppName();
const char * GetBundlePrefix();
const char * GetBundleName();
