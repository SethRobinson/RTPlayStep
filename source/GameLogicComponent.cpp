#include "PlatformPrecomp.h"
#include "GameLogicComponent.h"
#include "Entity/EntityUtils.h"
#include "App.h"

GameLogicComponent::GameLogicComponent()
{
	SetName("GameLogic");
}

GameLogicComponent::~GameLogicComponent()
{
}

void GameLogicComponent::SetupStepEnt(Entity *pEnt)
{
	SetScale2DEntity(pEnt, CL_Vec2f(0.7f,0.7f));
	SetAlignmentEntity(pEnt, ALIGNMENT_DOWN_CENTER);
	SetPos2DEntity(pEnt, CL_Vec2f(GetScreenSizeXf() / 2, GetScreenSizeYf()-100) );
}

void GameLogicComponent::SetSlideOverlayPosition()
{
	float percent = (float)GetApp()->m_energy / 100.0f;

	if (percent < 0) percent = 0;
	if (percent > 1) percent = 1;
	percent *= -1;
	
	ZoomToPositionEntity(m_pSlideOverlay, CL_Vec2f(0, -GetScreenSizeYf()*percent), 400);
}

void GameLogicComponent::LowerSlideScreen()
{
	if (m_slideScreenDown) return;

	ZoomToPositionEntity(m_pSlideOverlay, CL_Vec2f(0, 0), 3000);
	m_slideScreenDown = true;
	m_screenDownTimer = GetTick();
}

void GameLogicComponent::RaiseSlideScreen()
{
	if (m_slideScreenDown == false) return;

	ZoomToPositionEntity(m_pSlideOverlay, CL_Vec2f(0,-GetScreenSizeYf()), 3000);
	m_slideScreenDown = false;
	m_screenDownTimer = 0;
}

void GameLogicComponent::OnAdd(Entity *pEnt)
{
	EntityComponent::OnAdd(pEnt);

	LogMsg("GameLogic added");
	GetParent()->GetFunction("OnUpdate")->sig_function.connect(1, boost::bind(&GameLogicComponent::OnUpdate, this, _1));
	GetParent()->GetFunction("OnRender")->sig_function.connect(1, boost::bind(&GameLogicComponent::OnRender, this, _1));

	m_pSlideOverlay = CreateOverlayEntity(GetParent(), "SlideScreen", "interface/blank.rttex", 0,
		-GetScreenSizeYf(), true);
	
	Entity *pTitle = CreateOverlayEntity(m_pSlideOverlay, "Title", "interface/playstep.rttex", GetScreenSizeXf() / 2,
		GetScreenSizeYf() / 3, true);
	SetAlignmentEntity(pTitle, ALIGNMENT_CENTER);

	m_pStep1 = CreateOverlayEntity(m_pSlideOverlay, "Step1", "interface/step1_small.rttex", 0, true);
	m_pStep2 = CreateOverlayEntity(m_pSlideOverlay, "Step2", "interface/step2_small.rttex", 0, true);

	SetupStepEnt(m_pStep1);
	SetupStepEnt(m_pStep2);
	int timerMS = 0;

	LowerSlideScreen();

	//ZoomToPositionFromThisOffsetEntity(pTitle, CL_Vec2f(0, -GetScreenSizeYf() / 2), 1000, INTERPOLATE_SMOOTHSTEP, timerMS);
	GetAudioManager()->Play("audio/intro.wav");

	/*
	timerMS += 4000;
	ZoomToPositionEntityMulti(pTitle, CL_Vec2f(GetScreenSizeXf() / 2, +GetScreenSizeYf() * 2), 1000, INTERPOLATE_SMOOTHSTEP, timerMS);
	timerMS += 1000;
	KillEntity(pTitle, timerMS);

	*/
	Entity *pOverlay = GetParent()->AddEntity(new Entity("Overlay"));
	
	m_pEnergyEnt = CreateTextLabelEntity(pOverlay, "Energy", 10, GetScreenSizeYf() - 50, "0");
	SetupTextEntity(m_pEnergyEnt, FONT_LARGE, 1.0);
	GetApp()->GetFont(FONT_LARGE)->SetSmoothing(false);
	UpdateEnergyText();
	SetTextShadowColor(m_pEnergyEnt, MAKE_RGBA(0, 0, 0, 255));

	m_pDebugEnt = CreateTextLabelEntity(pOverlay, "DebugText", 75, GetScreenSizeYf() - 280, "");
	SetupTextEntity(m_pDebugEnt, FONT_LARGE, 1.0);
	AdvanceStepAnim(); //so both aren't being displayed
}

void GameLogicComponent::UpdateEnergyText()
{
	string color = "";
	if (GetApp()->m_energy < 0)
	{
		color = "`4";
	}
	SetTextEntity(m_pEnergyEnt, color+toString(GetApp()->m_energy));
}

void GameLogicComponent::OnEnergyChanged(int energy, int oldEnergy)
{
	if (energy > oldEnergy)
	{
		AdvanceStepAnim();

		m_blockerManager.RemoveBlocker();
	}
	
	UpdateEnergyText();

	if (GetApp()->m_energy > m_energyToLowerScreen)
	{
		RaiseSlideScreen();
	}
	else
	{
		LowerSlideScreen();
	}
}

int GameLogicComponent::GetSecondsWithScreenDown()
{
	if (m_screenDownTimer == 0) return 0;
	return  ((GetTick() - m_screenDownTimer) / 1000) + 1;
}

void GameLogicComponent::AdvanceStepAnim()
{
	m_showStep1 = !m_showStep1;

	SetVisibleEntity(m_pStep1, m_showStep1);
	SetVisibleEntity(m_pStep2, !m_showStep1);
}

void GameLogicComponent::OnUpdate(VariantList *pVList)
{
	int blocksThatShouldBeShown = 0;
	if (GetApp()->m_energy < 0)
	{
		blocksThatShouldBeShown = 1 + ((-GetApp()->m_energy) / m_secondsBetweenBlocks);
	}
	
	m_blockerManager.SetBlockerCount(blocksThatShouldBeShown);
	
	//LogMsg("Updating..");
		if (m_animTimer < GetTick())
		{
			m_animTimer = GetTick() + 300;
			//m_blockerManager.AddBlocker();
		}

		/*
		SetTextEntity(m_pDebugEnt,
			"Left: " + toString(GetApp()->m_bLeftFootDown) + " Right:" + toString(GetApp()->m_bRightFootDown) + " Last was left: " +
			toString(GetApp()->m_bLastPointWasLeftFoot)
			
		);
		*/
}

	void GameLogicComponent::OnRender(VariantList *pVList)
	{
		/*
		GetApp()->GetFont(FONT_LARGE)->DrawScaled(20, GetScreenSizeYf() - 200, );
		GetApp()->GetFont(FONT_LARGE)->DrawScaled(20, GetScreenSizeYf() - 100, "" + toString(GetApp()->m_energy), 2.0f, MAKE_RGBA(255, 255, 255, 255));
		*/

		m_blockerManager.Render();
	}
