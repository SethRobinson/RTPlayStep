/*
 *  App.cpp
 *  Created by Seth Robinson on 3/6/09.
 *  For license info, check the license.txt file that should have come with this.
 *
 */ 
#include "PlatformPrecomp.h"
#include "App.h"

#include "Entity/CustomInputComponent.h" //used for the back button (android)
#include "Entity/FocusInputComponent.h" //needed to let the input component see input messages
#include "Entity/ArcadeInputComponent.h" 
#include "Entity/EntityUtils.h"
#include "GameLogicComponent.h"
#include "util/TextScanner.h"

const bool G_ENABLE_LCD = false;
const int PIN_HALL1 = 0;
const int PIN_HALL2 = 2;


MessageManager g_messageManager;
MessageManager * GetMessageManager() {return &g_messageManager;}

FileManager g_fileManager;
FileManager * GetFileManager() {return &g_fileManager;}

#include "Audio/AudioManagerSDL.h"
AudioManagerSDL g_audioManager; //sound in windows/WebOS/Linux/html5

AudioManager * GetAudioManager(){return &g_audioManager;}

App *g_pApp = NULL;

string RunLinuxShell(string command)
{

	string temp;

	temp = "\r\nRunning " + command + " ...\r\n";

#ifndef WINAPI

	system(command.c_str());
	return temp;

#else
	return "Doesn't work in windows, can't run " + command;

#endif
}

void EnableTV(bool bOn)
{

	return;
	if (!bOn)
	{
		//RunLinuxShell("tvservice -o");

		RunLinuxShell("./uhubctl -a off -p 2");
	}
	else
	{
		//RunLinuxShell("tvservice -p; fbset -depth 8; fbset -depth 16");
		RunLinuxShell("./uhubctl -a on -p 2");
	}
}


BaseApp * GetBaseApp() 
{
	if (!g_pApp)
	{
		g_pApp = new App;
	}
	return g_pApp;
}

App * GetApp() 
{
	assert(g_pApp && "GetBaseApp must be called used first");
	return g_pApp;
}

App::App()
{
	m_blinkOn = false;
	m_bDidPostInit = false;
	m_energy = -100;

	if (m_bTestMode)
	{
		m_energy = 10;
	}

	m_energyTimer = 0;
	
}

App::~App()
{
	//EnableTV(true);
}

bool App::Init()
{
	m_pWiringPi = new WiringPiManager();

	if (m_bInitted)
	{
		return true;
	}

	if (!BaseApp::Init()) return false;

	if (GetEmulatedPlatformID() == PLATFORM_ID_IOS || GetEmulatedPlatformID() == PLATFORM_ID_WEBOS)
	{
		//SetLockedLandscape( true); //if we don't allow portrait mode for this game
		//SetManualRotationMode(true); //don't use manual, it may be faster (33% on a 3GS) but we want iOS's smooth rotations
	}

	LogMsg("The Save path is %s", GetSavePath().c_str());
	LogMsg("Region string is %s", GetRegionString().c_str());

#ifdef _DEBUG
	LogMsg("Built in debug mode");
#endif
#ifndef C_NO_ZLIB
	//fonts need zlib to decompress.  When porting a new platform I define C_NO_ZLIB and add zlib support later sometimes
	if (!GetFont(FONT_SMALL)->Load("interface/font_trajan.rtfont")) return false;

	//GetFont(FONT_SMALL)->SetSmoothing(false);

	if (!GetFont(FONT_LARGE)->Load("interface/font_trajan_big.rtfont"))
	{
		LogMsg("Can't load font 2");
		return false;
	}

#endif

	if (m_bTestMode)
	{
		GetBaseApp()->SetFPSVisible(true);
	}


	LogMsg("Reading config.txt");

	TextScanner ts;
	if (ts.LoadFile("config.txt"))
	{
		m_maxEnergy = StringToInt(ts.GetParmString("max_energy", 1));
		m_energyTimer = StringToInt(ts.GetParmString("energy_timer", 1));
		m_energyPerMove = StringToInt(ts.GetParmString("energy_per_move", 1));
	}
	else
	{
		LogMsg("Couldn't find config.txt");
	}

	return true;
}

void App::Kill()
{
	SAFE_DELETE(m_pWiringPi);
	BaseApp::Kill();
}

void App::OnExitApp(VariantList *pVarList)
{
	LogMsg("Exiting the app");
	OSMessage o;
	o.m_type = OSMessage::MESSAGE_FINISH_APP;
	GetBaseApp()->AddOSMessage(o);
}

#define kFilteringFactor 0.1f
#define C_DELAY_BETWEEN_SHAKES_MS 500

//testing accelerometer readings. To enable the test, search below for "ACCELTEST"
//Note: You'll need to look at the  debug log to see the output. (For android, run PhoneLog.bat from RTPlayStep/android)
void App::OnAccel(VariantList *pVList)
{
	
	if ( int(pVList->m_variant[0].GetFloat()) != MESSAGE_TYPE_GUI_ACCELEROMETER) return;

	CL_Vec3f v = pVList->m_variant[1].GetVector3();

	LogMsg("Accel: %s", PrintVector3(v).c_str());

	v.x = v.x * kFilteringFactor + v.x * (1.0f - kFilteringFactor);
	v.y = v.y * kFilteringFactor + v.y * (1.0f - kFilteringFactor);
	v.z = v.z * kFilteringFactor + v.z * (1.0f - kFilteringFactor);

	// Compute values for the three axes of the acceleromater
	float x = v.x - v.x;
	float y = v.y - v.x;
	float z = v.z - v.x;

	//Compute the intensity of the current acceleration 
	if (sqrt(x * x + y * y + z * z) > 2.0f)
	{
		Entity *pEnt = GetEntityRoot()->GetEntityByName("jumble");
		if (pEnt)
		{
			//GetAudioManager()->Play("audio/click.wav");
            VariantList vList(CL_Vec2f(), pEnt);
			pEnt->GetFunction("OnButtonSelected")->sig_function(&vList);
		}
		LogMsg("Shake!");
	}
}

//test for arcade keys.  To enable this test, search for TRACKBALL/ARCADETEST: below and uncomment the stuff under it.
//Note: You'll need to look at the debug log to see the output.  (For android, run PhoneLog.bat from RTPlayStep/android)

void App::OnArcadeInput(VariantList *pVList)
{

	int vKey = pVList->Get(0).GetUINT32();
	eVirtualKeyInfo keyInfo = (eVirtualKeyInfo) pVList->Get(1).GetUINT32();
	
	string pressed;

	switch (keyInfo)
	{
		case VIRTUAL_KEY_PRESS:
			pressed = "pressed";
			break;

		case VIRTUAL_KEY_RELEASE:
			pressed = "released";
			break;

		default:
			LogMsg("OnArcadeInput> Bad value of %d", keyInfo);
	}
	
	string keyName = "unknown";

	m_bLeftFootDown = false;
	m_bRightFootDown = false;

	switch (vKey)
	{
		case VIRTUAL_KEY_DIR_LEFT:
			keyName = "Left";
			m_bLeftFootDown = true;
			break;

		case VIRTUAL_KEY_DIR_RIGHT:
			keyName = "Right";
			m_bRightFootDown = true;
			break;

		case VIRTUAL_KEY_DIR_UP:
		{
			keyName = "Up";
			int oldEnergy = m_energy;
			m_energy++;
			OnEnergyChanged(oldEnergy);
		}
			break;

		case VIRTUAL_KEY_DIR_DOWN:
		{
			keyName = "Down";
			int oldEnergy = m_energy;
			m_energy--;
			OnEnergyChanged(oldEnergy);
		}
			break;
	}
	
	//LogMsg("Arcade input: Hit %d (%s) (%s)", vKey, keyName.c_str(), pressed.c_str());
}

void AppInputRawKeyboard(VariantList *pVList)
{
	char key = (char) pVList->Get(0).GetUINT32();
	bool bDown = pVList->Get(1).GetUINT32() != 0;
	LogMsg("Raw key %c (%d)",key, (int)bDown);
}

void AppInput(VariantList *pVList)
{

	//0 = message type, 1 = parent coordinate offset, 2 is fingerID
	eMessageType msgType = eMessageType( int(pVList->Get(0).GetFloat()));
	CL_Vec2f pt = pVList->Get(1).GetVector2();
	//pt += GetAlignmentOffset(*m_pSize2d, eAlignment(*m_pAlignment));

	uint32 fingerID = 0;
	if ( msgType != MESSAGE_TYPE_GUI_CHAR && pVList->Get(2).GetType() == Variant::TYPE_UINT32)
	{
		fingerID = pVList->Get(2).GetUINT32();
	}

	CL_Vec2f vLastTouchPt = GetBaseApp()->GetTouch(fingerID)->GetLastPos();

	switch (msgType)
	{
	case MESSAGE_TYPE_GUI_CLICK_START:
		LogMsg("Touch start: X: %.2f YL %.2f (Finger %d)", pt.x, pt.y, fingerID);
		break;
	case MESSAGE_TYPE_GUI_CLICK_MOVE:
		LogMsg("Touch move: X: %.2f YL %.2f (Finger %d)", pt.x, pt.y, fingerID);
		break;

	case MESSAGE_TYPE_GUI_CLICK_MOVE_RAW:
		//LogMsg("Touch raw move: X: %.2f YL %.2f (Finger %d)", pt.x, pt.y, fingerID);
		break;
	case MESSAGE_TYPE_GUI_CLICK_END:
		LogMsg("Touch end: X: %.2f YL %.2f (Finger %d)", pt.x, pt.y, fingerID);
		break;

	case MESSAGE_TYPE_GUI_CHAR:
		char key = (char)pVList->Get(2).GetUINT32();
		LogMsg("Hit key %c (%d)", key, (int)key);
		break;

	}	
}

void App::Update()
{
	
	//game can think here.  The baseApp::Update() will run Update() on all entities, if any are added.  The only one
	//we use in this example is one that is watching for the Back (android) or Escape key to quit that we setup earlier.

	BaseApp::Update();

	if (!m_bDidPostInit)
	{
		//stuff I want loaded during the first "Update"
		m_bDidPostInit = true;
		
		//for android, so the back key (or escape on windows) will quit out of the game
		Entity *pEnt = GetEntityRoot()->AddEntity(new Entity);
		EntityComponent *pComp = pEnt->AddComponent(new CustomInputComponent);
		//tell the component which key has to be hit for it to be activated
		pComp->GetVar("keycode")->Set(uint32(VIRTUAL_KEY_BACK));
		//attach our function so it is called when the back key is hit
		pComp->GetFunction("OnActivated")->sig_function.connect(1, boost::bind(&App::OnExitApp, this, _1));

		//nothing will happen unless we give it input focus
		pEnt->AddComponent(new FocusInputComponent);

		//ACCELTEST:  To test the accelerometer uncomment below: (will print values to the debug output)
		//SetAccelerometerUpdateHz(25); //default is 0, disabled
		//GetBaseApp()->m_sig_accel.connect(1, boost::bind(&App::OnAccel, this, _1));

		//TRACKBALL/ARCADETEST: Uncomment below to see log messages on trackball/key movement input
		pComp = pEnt->AddComponent(new ArcadeInputComponent);
		GetBaseApp()->m_sig_arcade_input.connect(1, boost::bind(&App::OnArcadeInput, this, _1));
	
		//these arrow keys will be triggered by the keyboard, if applicable
		AddKeyBinding(pComp, "Left", VIRTUAL_KEY_DIR_LEFT, VIRTUAL_KEY_DIR_LEFT);
		AddKeyBinding(pComp, "Right", VIRTUAL_KEY_DIR_RIGHT, VIRTUAL_KEY_DIR_RIGHT);
		AddKeyBinding(pComp, "Up", VIRTUAL_KEY_DIR_UP, VIRTUAL_KEY_DIR_UP);
		AddKeyBinding(pComp, "Down", VIRTUAL_KEY_DIR_DOWN, VIRTUAL_KEY_DIR_DOWN);
		AddKeyBinding(pComp, "Fire", VIRTUAL_KEY_CONTROL, VIRTUAL_KEY_GAME_FIRE);

		//INPUT TEST - wire up input to some functions to manually handle.  AppInput will use LogMsg to
		//send them to the log.  (Each device has a way to view a debug log in real-time)
		GetBaseApp()->m_sig_input.connect(&AppInput);

		//this one gives raw up and down of keyboard events, where the one above only gives
		//MESSAGE_TYPE_GUI_CHAR which is just the down and includes keyboard repeats from
		//holding the key
		//GetBaseApp()->m_sig_raw_keyboard.connect(&AppInputRawKeyboard);
		
		/*
		//file handling test, if TextScanner.h is included at the top..

		TextScanner t;
		t.m_lines.push_back("Testing 123");
		t.m_lines.push_back("Heck yeah!");
		t.m_lines.push_back("Whoopsopsop!");

		LogMsg("Saving file...");
		t.SaveFile("temp.txt");


		TextScanner b;
		b.LoadFile("temp.txt");
		b.DumpToLog();
		*/

		SetGPIO();

		AddFocusIfNeeded(GetEntityRoot());

		Entity *pGameLogicEnt = GetEntityRoot()->AddEntity(new Entity("GameLogic"));
		
		
		m_pGameLogicComp = (GameLogicComponent*)pGameLogicEnt->AddComponent(new GameLogicComponent());


	}

	//game is thinking. 
	GPIOThink();
}

void App::Draw()
{

	//Use this to prepare for raw GL calls
	PrepareForGL();
#ifdef _DEBUG
	//LogMsg("Doing draw");
#endif
	glClearColor(0,1,0,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	CLEAR_GL_ERRORS() //honestly I don't know why I get a 0x0502 GL error when doing the FIRST gl action that requires a context with emscripten only

	/*
	//draw our game stuff
	DrawFilledRect(10.0f,10.0f,GetScreenSizeXf()/3,GetScreenSizeYf()/3, MAKE_RGBA(255,255,0,255));
	DrawFilledRect(0,0,64,64, MAKE_RGBA(0,255,0,100));

	//after our 2d rect call above, we need to prepare for raw GL again. (it keeps it in ortho mode if we don't for speed)
	PrepareForGL();
	RenderSpinningTriangle();
	//RenderGLTriangle();
	//let's blit a bmp, but first load it if needed

	if (!m_surf.IsLoaded())
	{
		m_surf.LoadFile("interface/test.bmp");
	}

	m_surf.Bind();

	//RenderTexturedGLTriangle();
	//RenderTexturedGLTriangleWithDrawElements();

	//blit the logo with the Y mirrored
	//rtRect texRect = rtRect(0, m_surf.GetHeight(), m_surf.GetWidth(), 0);
	//rtRect destRect = rtRect(0,0, m_surf.GetWidth(), m_surf.GetHeight());
	//m_surf.BlitEx(destRect, texRect);

	//make the logo spin like a wheel, whee!
	//m_surf.BlitEx(destRect, texRect, MAKE_RGBA(255,255,255,255) , 180*SinGamePulseByMS(3000), CL_Vec2f(m_surf.GetWidth()/2,m_surf.GetHeight()/2));

	//blit it normally
	m_surf.Blit(0, 0);
	//m_surf.Blit(100, 100);

	m_surf.BlitScaled(100, 200, CL_Vec2f(1,1), ALIGNMENT_CENTER, MAKE_RGBA(255,255,255,255), SinGamePulseByMS(3000)*360);

	m_surf.BlitRotated(400, 200, CL_Vec2f(0.2f,0.2f), ALIGNMENT_CENTER, MAKE_RGBA(255,255,255,255), SinGamePulseByMS(4000)*360,
		CL_Vec2f(20,-20), NULL);
		*/

	//GetFont(FONT_SMALL)->Draw(0,0, "test");
	
	
	//the base handles actually drawing the GUI stuff over everything else, if applicable
	BaseApp::Draw();

	
}

void App::OnScreenSizeChange()
{
	BaseApp::OnScreenSizeChange();
}

void App::OnEnterBackground()
{
	//save your game stuff here, as on some devices (Android <cough>) we never get another notification of quitting.
	LogMsg("Entered background");
	BaseApp::OnEnterBackground();
}

void App::OnEnterForeground()
{
	LogMsg("Entered foreground");
	BaseApp::OnEnterForeground();
}

const char * GetAppName() {return "RTPlayStep";}

//the stuff below is for android/webos builds.  Your app needs to be named like this.

//note: these are put into vars like this to be compatible with my command-line parsing stuff that grabs the vars

const char * GetBundlePrefix()
{
	const char * bundlePrefix = "com.rtsoft.";
	return bundlePrefix;
}

const char * GetBundleName()
{
	const char * bundleName = "RTPlayStep";
	return bundleName;
}

bool App::OnPreInitVideo()
{
	//only called for desktop systems
	//override in App.* if you want to do something here.  You'd have to
	//extern these vars from main.cpp to change them...

	//SetEmulatedPlatformID(PLATFORM_ID_WINDOWS);

	SetPrimaryScreenSize(1920, 1080);
	SetupScreenInfo(1920, 1080, ORIENTATION_DONT_CARE);
	return true; //no error
}

const int POWER_PIN = 29;

void App::SetGPIO()
{
	LogMsg("Setting GPIO");
	if (wiringPiSetup() == -1) //using WPi numbering.  See chart at the end, look for "wPi".  That chart is the 40 pins. Works with Pi2
	{
		LogMsg("Wiring PI failure.  You need to run it with root privileges for the GPIO stuff to work. (put sudo first!)");
	}
	else
	{
		LogMsg("Initted Wiring PI");
	}

	pinMode(PIN_HALL1, INPUT);
	pullUpDnControl(PIN_HALL1, PUD_UP);

	pinMode(PIN_HALL2, INPUT);
	pullUpDnControl(PIN_HALL2, PUD_UP);

	//pinMode(POWER_PIN, OUTPUT);
	if (G_ENABLE_LCD)
	{
		m_pWiringPi->SetupLCD(7);
		UpdateLCD();
	}
}

void App::UpdateLCD()
{
	if (G_ENABLE_LCD)
	{
		m_pWiringPi->LCDClear();
		m_pWiringPi->LCDPrint(toString(m_energy));
	}
}

void App::OnEnergyChanged(int originalEnergy)
{
	m_pGameLogicComp->OnEnergyChanged(m_energy, originalEnergy);
	UpdateLCD();
	EnableTV(m_energy > 0);
}

void App::GPIOThink()
{
	int originalEnergy = m_energy;

	if (m_energyTimer < GetTick())
	{
		m_energyTimer = m_ms_perEnergy + GetTick();
	//	if (m_energy > 0)
		{
			//if (m_pGameLogicComp->GetSecondsWithScreenDown() == 0 || m_bTestMode == true)
			{
				m_energy--;
			}
			
		}
	}

	if (m_energy > m_maxEnergy) m_energy = m_maxEnergy;

	if (m_energy < m_pGameLogicComp->m_energyToLowerScreen)
	{
		m_energy = m_pGameLogicComp->m_energyToLowerScreen;
	}


	if (m_pGameLogicComp->GetSecondsWithScreenDown() > 60)
	{
		//m_energy = 0;
	}

	/*
	if (digitalRead(AF_UP) == LOW)
	{
		m_energy++;
	}

	if (digitalRead(AF_DOWN) == LOW)
	{
		if (m_energy > 0)
			m_energy--;
	}

	*/

	//will be set to 0 if close to the magnet
	
#ifndef WINAPI
	m_bLeftFootDown = digitalRead(PIN_HALL1) == 0;
	m_bRightFootDown = digitalRead(PIN_HALL2) == 0;

//	LogMsg("Read %d and %d", digitalRead(PIN_HALL1), digitalRead(PIN_HALL2));
#endif
	
	bool bAwardPoint = false;

	if (m_bLastPointWasLeftFoot)
	{
		//only care if right is down now
		if (m_bRightFootDown)
		{
			m_bLastPointWasLeftFoot = !m_bLastPointWasLeftFoot;
			bAwardPoint = true;
		}
	}
	else
	{
		if (m_bLeftFootDown == true)
		{
			m_bLastPointWasLeftFoot = !m_bLastPointWasLeftFoot;
			bAwardPoint = true;
		}
	}

	if (bAwardPoint)
	{
		GetAudioManager()->Play("audio/blip1.wav");

		m_energy += m_energyPerMove;
	}

	if (originalEnergy != m_energy)
	{
		OnEnergyChanged(originalEnergy);
	}

	if (m_energy < 10 && m_energy != 0)
	{
		//enable beeping
		if (m_beepInterval < GetTick())
		{
		//	GetAudioManager()->Play("audio/buzzer_blip.wav");

			int beepDelay = 100 * m_energy;
			if (beepDelay < 100)
			{
				beepDelay = 100;
			}

			m_beepInterval = GetTick() + beepDelay;
		}
	}
}

//pin layouts
/*

+-----+-----+---------+------+---+--B Plus--+---+------+---------+-----+-----+
| BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
+-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
|     |     |    3.3v |      |   |  1 || 2  |   |      | 5v      |     |     |
|   2 |   8 |   SDA.1 |   IN | 1 |  3 || 4  |   |      | 5V      |     |     |
|   3 |   9 |   SCL.1 |   IN | 1 |  5 || 6  |   |      | 0v      |     |     |
|   4 |   7 | GPIO. 7 |   IN | 0 |  7 || 8  | 1 | ALT0 | TxD     | 15  | 14  |
|     |     |      0v |      |   |  9 || 10 | 1 | ALT0 | RxD     | 16  | 15  |
|  17 |   0 | GPIO. 0 |  OUT | 0 | 11 || 12 | 0 | IN   | GPIO. 1 | 1   | 18  |
|  27 |   2 | GPIO. 2 |   IN | 0 | 13 || 14 |   |      | 0v      |     |     |
|  22 |   3 | GPIO. 3 |   IN | 0 | 15 || 16 | 0 | IN   | GPIO. 4 | 4   | 23  |
|     |     |    3.3v |      |   | 17 || 18 | 0 | IN   | GPIO. 5 | 5   | 24  |
|  10 |  12 |    MOSI |   IN | 0 | 19 || 20 |   |      | 0v      |     |     |
|   9 |  13 |    MISO |   IN | 0 | 21 || 22 | 0 | IN   | GPIO. 6 | 6   | 25  |
|  11 |  14 |    SCLK |   IN | 0 | 23 || 24 | 0 | IN   | CE0     | 10  | 8   |
|     |     |      0v |      |   | 25 || 26 | 0 | IN   | CE1     | 11  | 7   |
|   0 |  30 |   SDA.0 |   IN | 0 | 27 || 28 | 0 | IN   | SCL.0   | 31  | 1   |
|   5 |  21 | GPIO.21 |   IN | 0 | 29 || 30 |   |      | 0v      |     |     |
|   6 |  22 | GPIO.22 |   IN | 0 | 31 || 32 | 0 | IN   | GPIO.26 | 26  | 12  |
|  13 |  23 | GPIO.23 |   IN | 0 | 33 || 34 |   |      | 0v      |     |     |
|  19 |  24 | GPIO.24 |   IN | 0 | 35 || 36 | 1 | OUT  | GPIO.27 | 27  | 16  |
|  26 |  25 | GPIO.25 |   IN | 0 | 37 || 38 | 0 | IN   | GPIO.28 | 28  | 20  |
|     |     |      0v |      |   | 39 || 40 | 0 | IN   | GPIO.29 | 29  | 21  |
+-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
| BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
+-----+-----+---------+------+---+--B Plus--+---+------+---------+-----+-----+
*/