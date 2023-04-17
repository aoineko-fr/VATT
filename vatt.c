// ____________________________
// ██▀▀█▀▀██▀▀▀▀▀▀▀█▀▀█        │   ▄▄▄                ▄▄      
// ██  ▀  █▄  ▀██▄ ▀ ▄█ ▄▀▀ █  │  ▀█▄  ▄▀██ ▄█▄█ ██▀▄ ██  ▄███
// █  █ █  ▀▀  ▄█  █  █ ▀▄█ █▄ │  ▄▄█▀ ▀▄██ ██ █ ██▀  ▀█▄ ▀█▄▄
// ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀────────┘                 ▀▀
//  VRAM access timing sample
//─────────────────────────────────────────────────────────────────────────────

//=============================================================================
// INCLUDES
//=============================================================================
#include "msxgl.h"
#include "vdp_reg.h"
#include "game_menu.h"
#include "fsm.h"
#include "test.h"
#include "ascii.h"

//=============================================================================
// DEFINES
//=============================================================================

// Version
#define APP_VERSION "0.11"

// Library's logo
#define MSX_GL "\x01\x02\x03\x04\x05\x06"

// VRAM access counter
#define TEST_COUNT					256

// Test function callback
typedef void (*cbTest)(u8);

//
struct TestTime
{
	cbTest    Function;				// Test function pointer
	u8        Time;					// I/O access interval
	const c8* Text;					// Description
};

//
struct ScreenMode
{
	const c8* Code;
	const c8* Scr;
	const c8* Text;
	u8        Mode;
};

// Menu functions prototypes
const c8* MenuAction_Mode(u8 op, i8 value);
const c8* MenuAction_Time(u8 op, i8 value);
const c8* MenuAction_Count(u8 op, i8 value);
const c8* MenuAction_Test(u8 op, i8 value);
const c8* MenuAction_Name(u8 op, i8 value);
void MenuInit_Main();
void MenuInit_Sub();

// State functions prototypes
void State_Menu_Begin();
void State_Menu_Update();
void State_Report_Begin();
void State_Report_Update();

//=============================================================================
// READ-ONLY DATA
//=============================================================================

// Fonts data
#include "font/font_mgl_sample6.h"

// Test speed
const struct TestTime g_Time[] =
{
	{ Test_12, 12, "12: out(n),a"               },
	{ Test_14, 14, "14: out(c),a"               },
	{ Test_17, 17, "17: out(n),a; nop"          },
	{ Test_18, 18, "18: outi"                   },
	{ Test_19, 19, "19: out(c),a; nop"          },
	{ Test_20, 20, "20: out(n),a; cp(hl)"       },
	{ Test_21, 21, "21: out(c),a; inc de"       },
	{ Test_22, 22, "22: out(c),a; cp(hl)"       },
	{ Test_23, 23, "23: otir"                   },
	{ Test_24, 24, "24: out(n),a; inc(hl)"      },
	{ Test_25, 25, "25: outi; inc de"           },
	{ Test_26, 26, "26: out(n),a; djnz"         },
	{ Test_27, 27, "27: out(n),a;cp(hl);inc de" },
	{ Test_28, 28, "28: out(n),a; cp(hl) x 2"   },
	{ Test_29, 29, "29: outi; jp nz"            },
	{ Test_30, 30, "30: out(c),a; cp(hl) x 2"   },
	// { Test_31, 31, "31 TS - out(n),a; nop; djnz"  },
};

// Screen modes
const struct ScreenMode g_Mode[] =
{
	{ "T1",  "Sc0", "Text mode 1 (SC0, 40 col)", VDP_MODE_TEXT1 },
	{ "G1",  "Sc1", "Graphic mode 1 (SC1)",      VDP_MODE_GRAPHIC1 },
	{ "G2",  "Sc2", "Graphic mode 2 (SC2)",      VDP_MODE_GRAPHIC2 },
	{ "MC",  "Sc3", "Multi-color mode (SC3)",    VDP_MODE_MULTICOLOR },
	{ "T2",  "W80", "Text mode 2 (SC0, 80 col)", VDP_MODE_TEXT2 },
	{ "G3",  "Sc4", "Graphic mode 3 (SC4)",      VDP_MODE_GRAPHIC3 },
	{ "G4",  "Sc5", "Graphic mode 4 (SC5)",      VDP_MODE_GRAPHIC4 },
	{ "G5",  "Sc6", "Graphic mode 5 (SC6)",      VDP_MODE_GRAPHIC5 },
	{ "G6",  "Sc7", "Graphic mode 6 (SC7)",      VDP_MODE_GRAPHIC6 },
	{ "G7",  "Sc8", "Graphic mode 7 (SC8)",      VDP_MODE_GRAPHIC7 },
	{ "YAE", "S10", "GM7 + YJK + YAE (SC10)",    VDP_MODE_SCREEN10 },
	{ "YJK", "S12", "GM7 + YJK (SC12)",          VDP_MODE_SCREEN12 },
};

enum MENU_IDS
{
	MENU_MAIN = 0,
	MENU_OPTIONS,
	MENU_MODES,
	MENU_TIMINGS,
};

// Menu main page
const MenuItem g_MenuMain[] =
{
	{ "Mode",     MENU_ITEM_ACTION, MenuAction_Mode,  0 },
	{ "Timing",   MENU_ITEM_ACTION, MenuAction_Time,  0 },
	{ "Options>", MENU_ITEM_GOTO,   NULL,             MENU_OPTIONS },
	{ NULL,       MENU_ITEM_EMPTY,  NULL,             0 },
	{ "Test",     MENU_ITEM_ACTION, MenuAction_Test,  0 },
	{ "Test All", MENU_ITEM_ACTION, MenuAction_Test,  1 },
	{ "Report",   MENU_ITEM_ACTION, MenuAction_Test,  2 },
	{ "Reset",    MENU_ITEM_ACTION, MenuAction_Test,  3 },
};

// Options menu
const MenuItem g_MenuOption[] =
{
	{ "Name",     MENU_ITEM_ACTION, MenuAction_Name,  0 },
	{ "Modes>",   MENU_ITEM_GOTO,   NULL,             MENU_MODES },
	{ "Timings>", MENU_ITEM_GOTO,   NULL,             MENU_TIMINGS },
	{ "Sprite",   MENU_ITEM_BOOL,   &g_DisplaySprite, 0 },
	{ "Screen",   MENU_ITEM_BOOL,   &g_DisplayScreen, 0 },
	{ "Count",    MENU_ITEM_ACTION, MenuAction_Count, 0 },
	{ "Waits",    MENU_ITEM_INT,    &g_TimeOffset,    0 },
	{ NULL,       MENU_ITEM_EMPTY,  NULL,             0 },
	{ "<Back",    MENU_ITEM_GOTO,   NULL,             MENU_MAIN },
};

// Screen mode selection menu
const MenuItem g_MenuMode[] =
{
	{ "T1",       MENU_ITEM_BOOL,   &g_SelectModes[0],  0 },
	{ "G1",       MENU_ITEM_BOOL,   &g_SelectModes[1],  0 },
	{ "G2",       MENU_ITEM_BOOL,   &g_SelectModes[2],  0 },
	{ "MC",       MENU_ITEM_BOOL,   &g_SelectModes[3],  0 },
	{ "T2",       MENU_ITEM_BOOL,   &g_SelectModes[4],  0 },
	{ "G3",       MENU_ITEM_BOOL,   &g_SelectModes[5],  0 },
	{ "G4",       MENU_ITEM_BOOL,   &g_SelectModes[6],  0 },
	{ "G5",       MENU_ITEM_BOOL,   &g_SelectModes[7],  0 },
	{ "G6",       MENU_ITEM_BOOL,   &g_SelectModes[8],  0 },
	{ "G7",       MENU_ITEM_BOOL,   &g_SelectModes[9],  0 },
	{ "YAE",      MENU_ITEM_BOOL,   &g_SelectModes[10], 0 },
	{ "YJK",      MENU_ITEM_BOOL,   &g_SelectModes[11], 0 },
	{ NULL,       MENU_ITEM_EMPTY,  NULL,               0 },
	{ "<Back",    MENU_ITEM_GOTO,   NULL,               MENU_OPTIONS },
};

// Screen mode selection menu
const MenuItem g_MenuTime[] =
{
	{ "12 TS",    MENU_ITEM_BOOL,   &g_SelectTimes[0],  0 },
	{ "14 TS",    MENU_ITEM_BOOL,   &g_SelectTimes[1],  0 },
	{ "17 TS",    MENU_ITEM_BOOL,   &g_SelectTimes[2],  0 },
	{ "18 TS",    MENU_ITEM_BOOL,   &g_SelectTimes[3],  0 },
	{ "19 TS",    MENU_ITEM_BOOL,   &g_SelectTimes[4],  0 },
	{ "20 TS",    MENU_ITEM_BOOL,   &g_SelectTimes[5],  0 },
	{ "21 TS",    MENU_ITEM_BOOL,   &g_SelectTimes[6],  0 },
	{ "22 TS",    MENU_ITEM_BOOL,   &g_SelectTimes[7],  0 },
	{ "23 TS",    MENU_ITEM_BOOL,   &g_SelectTimes[8],  0 },
	{ "24 TS",    MENU_ITEM_BOOL,   &g_SelectTimes[9],  0 },
	{ "25 TS",    MENU_ITEM_BOOL,   &g_SelectTimes[10], 0 },
	{ "26 TS",    MENU_ITEM_BOOL,   &g_SelectTimes[11], 0 },
	{ "27 TS",    MENU_ITEM_BOOL,   &g_SelectTimes[12], 0 },
	{ "28 TS",    MENU_ITEM_BOOL,   &g_SelectTimes[13], 0 },
	{ "29 TS",    MENU_ITEM_BOOL,   &g_SelectTimes[14], 0 },
	{ "30 TS",    MENU_ITEM_BOOL,   &g_SelectTimes[15], 0 },
	// { "31 TS",    MENU_ITEM_BOOL,   &g_SelectTimes[16], 0 },
	{ NULL,       MENU_ITEM_EMPTY,  NULL,               0 },
	{ "<Back",    MENU_ITEM_GOTO,   NULL,               MENU_OPTIONS },
};

// Menu pages configuration
const Menu g_Menus[] =
{
	{ NULL, g_MenuMain,   numberof(g_MenuMain),   MenuInit_Main }, // MENU_MAIN
	{ NULL, g_MenuOption, numberof(g_MenuOption), MenuInit_Sub  }, // MENU_OPTIONS
	{ NULL, g_MenuMode,   numberof(g_MenuMode),   MenuInit_Sub  }, // MENU_MODES
	{ NULL, g_MenuTime,   numberof(g_MenuTime),   MenuInit_Sub  }, // MENU_TIMINGS
};

// States data
const FSM_State State_Menu =	{ 0, State_Menu_Begin,   State_Menu_Update,   NULL };
const FSM_State State_Report =	{ 0, State_Report_Begin, State_Report_Update, NULL };

//                                             T1  G1  G2  MC  T2  G3  G4  G5  G6  G7  YAE YJK
const u8 g_ModeLimitMSX1[numberof(g_Mode)] = { 12, 29, 29, 13, -1, -1, -1, -1, -1, -1, -1, -1 };
const u8 g_ModeLimitMSX2[numberof(g_Mode)] = { 20, 15, 15, 15, 20, 15, 15, 15, 15, 15, 15, 15 };

// Is default test function for MSX1 machine
const bool g_DefaultTestMSX1[numberof(g_Time)] =
{
	TRUE,	// 12 TS - out(n),a
	TRUE,	// 14 TS - out(c),a
	TRUE,	// 17 TS - out(n),a; nop
	TRUE,	// 18 TS - outi
	FALSE,	// 19 TS - out(c),a; nop
	FALSE,	// 20 TS - out(n),a; cp(hl)
	FALSE,	// 21 TS - out(c),a; inc de
	FALSE,	// 22 TS - out(c),a; cp(hl)
	FALSE,	// 23 TS - otir
	FALSE,	// 24 TS - out(n),a; inc(hl)
	FALSE,	// 25 TS - outi; inc de
	TRUE,	// 26 TS - out(n),a; djnz
	TRUE,	// 27 TS - out(n),a;cp();inc de
	TRUE,	// 28 TS - out(n),a; cp(hl) x 2
	TRUE,	// 29 TS - outi; jp nz
	TRUE,	// 30 TS - out(c),a; cp(hl) x 2
	// FALSE,	// 31 TS - out(n),a; nop; djnz
};

// Is default test function for MSX2 machine
const bool g_DefaultTestMSX2[numberof(g_Time)] =
{
	TRUE,	// 12 TS - out(n),a
	TRUE,	// 14 TS - out(c),a
	TRUE,	// 17 TS - out(n),a; nop
	TRUE,	// 18 TS - outi
	TRUE,	// 19 TS - out(c),a; nop
	TRUE,	// 20 TS - out(n),a; cp(hl)
	TRUE,	// 21 TS - out(c),a; inc de
	TRUE,	// 22 TS - out(c),a; cp(hl)
	TRUE,	// 23 TS - otir
	FALSE,	// 24 TS - out(n),a; inc(hl)
	FALSE,	// 25 TS - outi; inc de
	FALSE,	// 26 TS - out(n),a; djnz
	FALSE,	// 27 TS - out(n),a;cp();inc de
	FALSE,	// 28 TS - out(n),a; cp(hl) x 2
	FALSE,	// 29 TS - outi; jp nz
	FALSE,	// 30 TS - out(c),a; cp(hl) x 2
	// FALSE,	// 31 TS - out(n),a; nop; djnz
};

// Iteration counter
const c8* g_IterationText[] = { "1", "2", "4", "8", "16", "32", "64", "128" };

// Sprite attribute
const struct VDP_Sprite g_SpriteAttr[32] = 
{
	{   0,   0, 'V', COLOR_LIGHT_RED },
	{   6,   8, 'A', COLOR_LIGHT_RED },
	{  12,  16, 'T', COLOR_LIGHT_RED },
	{  18,  24, 'T', COLOR_LIGHT_RED },
	{  24,  32, '-', COLOR_LIGHT_RED },
	{  30,  40, '1', COLOR_LIGHT_RED },
	{  36,  48, 'T', COLOR_LIGHT_RED },
	{  42,  56, '-', COLOR_LIGHT_RED },
	{  48,  64, '1', COLOR_LIGHT_RED },
	{  54,  72, '2', COLOR_LIGHT_RED },
	{  60,  80, 'T', COLOR_LIGHT_RED },
	{  66,  88, 'S', COLOR_LIGHT_RED },
	{  72,  96, 'T', COLOR_LIGHT_RED },
	{  78, 104, 'E', COLOR_LIGHT_RED },
	{  84, 112, 'S', COLOR_LIGHT_RED },
	{  90, 120, 'T', COLOR_LIGHT_RED },
	{  96, 128, 'V', COLOR_LIGHT_RED },
	{ 102, 136, 'A', COLOR_LIGHT_RED },
	{ 108, 144, 'T', COLOR_LIGHT_RED },
	{ 114, 152, 'T', COLOR_LIGHT_RED },
	{ 120, 160, '-', COLOR_LIGHT_RED },
	{ 126, 168, '1', COLOR_LIGHT_RED },
	{ 132, 176, 'T', COLOR_LIGHT_RED },
	{ 138, 184, '-', COLOR_LIGHT_RED },
	{ 144, 192, '1', COLOR_LIGHT_RED },
	{ 150, 200, '2', COLOR_LIGHT_RED },
	{ 156, 208, 'T', COLOR_LIGHT_RED },
	{ 162, 216, 'S', COLOR_LIGHT_RED },
	{ 168, 224, 'T', COLOR_LIGHT_RED },
	{ 174, 232, 'E', COLOR_LIGHT_RED },
	{ 180, 240, 'S', COLOR_LIGHT_RED },
	{ 186, 248, 'T', COLOR_LIGHT_RED },
};

//=============================================================================
// MEMORY DATA
//=============================================================================

u8   g_VDP;							// Detected VDP version
u8   g_CurMode;						// Current Screen mode
u8   g_CurTime;						// Selected access time
bool g_DisplaySprite;				// Display sprite
bool g_DisplayScreen;				// Blank the screen
u16  g_DestAddr;					// VRAM destination address
u8   g_ModeNum;						// Number of available modes (depend of VDP version)
u8   g_IterationCount;				// Iteration counter
i8   g_TimeOffset;					// Iteration counter
const u8* g_ModeLimit;				// Speed limit for each screen mdoe
bool g_SelectTimes[numberof(g_Time)]; // 
bool g_SelectModes[numberof(g_Mode)]; //

u16  g_TestTotal;					// 
u16  g_TestMin;						// 
u16  g_TestMax;						// 

c8 g_StringBuffer[64];
const c8* g_MachineName;
//-----------------------------------------------------------------------------
// Report table
//-----------------------------------------------------------------------------
typedef u8 ResultTable[numberof(g_Time)][numberof(g_Mode)];

struct Result
{
	const c8*   Name;
	ResultTable Table;
};

ResultTable g_ReportAve;
ResultTable g_ReportMin;
ResultTable g_ReportMax;
ResultTable* g_CurResult;			// Index of the result to display

//=============================================================================
// FUNCTIONS
//=============================================================================

//-----------------------------------------------------------------------------
// Get MSX version
const c8* GetMSXVersion(u8 ver)
{
	switch(ver)
	{
	case 0: // MSX 1
		return "TMS9918";
	case 1: // MSX 2
		return "V9938";
	case 2: // MSX 2+
	case 3: // MSX turbo R
		return "V9958";
	}
	return "Unknow";
}

//-----------------------------------------------------------------------------
// Get VDP version
const c8* GetVDPVersion()
{
	switch(g_VDP)
	{
	case VDP_VERSION_TMS9918A:
		return "TMS9918";
	case VDP_VERSION_V9938:
		return "V9938";
	case VDP_VERSION_V9958:
		return "V9958";
	}
	return "Unknow";
}

//-----------------------------------------------------------------------------
// Get next character from user
c8 GetCharacter()
{
	__asm
		push	ix
		ld		ix, #R_CHGET
		ld		iy, #0x0000
		call	R_CALSLT
		pop		ix
	__endasm;
}

//-----------------------------------------------------------------------------
// Test a given screen mode with a given test function
const c8* GetStringAt(u8 x, u8 y)
{
	Print_SetPosition(x, y);
	c8* ptr = g_StringBuffer;
	c8 chr = 0;
	while(chr != ASCII_RETURN)
	{
		chr = GetCharacter();
		if((chr == ASCII_BS) && (ptr > g_StringBuffer))
		{
			Print_Backspace(1);
			ptr--;
		}

		if((chr == ASCII_SPACE) && (ptr == g_StringBuffer))
			continue;

		if((chr >= ASCII_SPACE) && (chr <= '~'))
		{
			Print_DrawChar(chr);
			*ptr = chr;
			ptr++;
		}
	}
	*ptr = 0;
	return g_StringBuffer;
}

//-----------------------------------------------------------------------------
// Test a given screen mode with a given test function
void Test(u8 mode, u8 time)
{
	VDP_SetMode(g_Mode[mode].Mode); // Set selected screen mode
	VDP_SetColor(COLOR_MERGE(COLOR_LIGHT_RED, COLOR_DARK_RED));
	VDP_SetSpritePatternTable(0x3800);
	VDP_SetSpriteAttributeTable(0x1A00);

	// MSX 1
	if(g_VDP == VDP_VERSION_TMS9918A)
		VDP_SetSpritePositionY(0, g_DisplaySprite ? 0 : VDP_SPRITE_DISABLE_SM1);
	// MSX 2/2+/turbo R
	else
	{
		VDP_RegWrite(14, 0);
		VDP_EnableSprite(g_DisplaySprite);
	}
	VDP_EnableDisplay(g_DisplayScreen);

	// Init value
	g_TestTotal = 0; // Max 
	g_TestMin = TEST_COUNT;
	g_TestMax = 0;
	u8 itNum = 1 << g_IterationCount; // Compute iteration number
	cbTest cb = g_Time[time].Function;

	for(u8 j = 0; j < itNum; ++j)
	{
		// Write reference
		DisableInterrupt();
		SetWriteVRAM(g_DestAddr);
		Test_31(0x09);

		// Test the given writing function
		SetWriteVRAM(g_DestAddr);
		cb(0x0A);
		EnableInterrupt();

		// Check result
		u16 addr = g_DestAddr;
		u16 count = 0;
		for(u16 i = 0; i < TEST_COUNT; ++i)
			if(VDP_Peek_16K(addr++) == 0x0A)
				count++;

		// Store total, min and max
		g_TestTotal += count;
		if(count < g_TestMin)
			g_TestMin = count;
		if(count > g_TestMax)
			g_TestMax = count;
	}

	g_TestTotal = (u16)((u32)g_TestTotal * 100 / TEST_COUNT) >> g_IterationCount; // Scale down the total to get the average

	g_ReportAve[time][mode] = (u8)g_TestTotal;
	g_ReportMin[time][mode] = (u8)(100 * g_TestMin / TEST_COUNT);
	g_ReportMax[time][mode] = (u8)(100 * g_TestMax / TEST_COUNT);

	if(g_VDP == VDP_VERSION_TMS9918A)
		VDP_SetSpritePositionY(0, VDP_SPRITE_DISABLE_SM1);
	else
		VDP_EnableSprite(FALSE);
	VDP_EnableDisplay(TRUE);

	VDP_SetMode(VDP_MODE_SCREEN0); // Restore Screen mode 0
	VDP_SetColor(COLOR_MERGE(COLOR_WHITE, COLOR_BLACK));

	Print_SetPosition(28, 23);
	Print_DrawFormat("Result: %i", g_TestTotal);
	Print_DrawText("%  ");
}

//-----------------------------------------------------------------------------
// Test all screen mode and test function
void TestAll()
{
	for(u8 m = 0; m < g_ModeNum; ++m)
	{
		if(!g_SelectModes[m])
			continue;
		for(u8 t = 0; t < numberof(g_Time); ++t)
		{
			if(!g_SelectTimes[t])
				continue;
			Test(m, t);
		}
	}

	g_CurResult = &g_ReportAve;
	FSM_SetState(&State_Report);
}

//-----------------------------------------------------------------------------
// 
void Reset()
{
	Mem_Set(0xFF, g_ReportAve, numberof(g_Time) * numberof(g_Mode));
	Mem_Set(0xFF, g_ReportMin, numberof(g_Time) * numberof(g_Mode));
	Mem_Set(0xFF, g_ReportMax, numberof(g_Time) * numberof(g_Mode));
	SetWriteVRAM(g_DestAddr);
	Test_31(' ');
}

//-----------------------------------------------------------------------------
// 
void DisplayHeader()
{
	Print_Clear();
	Print_SetPosition(0, 0);
	Print_DrawText(MSX_GL" VRAM Access Timing Tester "APP_VERSION);
	Print_DrawLineH(0, 1, 40);

	Print_SetPosition(1, 2);
	Print_DrawFormat("Machine:%s", g_MachineName);

	// System information (use interslot access for MSX-DOS)
	u8 biosReadPort  = Bios_InterSlotRead(g_MNROM, 0x0006);
	u8 biosWritePort = Bios_InterSlotRead(g_MNROM, 0x0007);
	u8 biosVersion   = Bios_InterSlotRead(g_MNROM, 0x002B);
	u8 biosNumber    = Bios_InterSlotRead(g_MNROM, 0x002D);
	// Display BIOS information
	Print_SetPosition(1, 3);
	Print_DrawFormat("BIOS:   %s %iHz", GetMSXVersion(biosNumber), (biosVersion & 0x80) ? 50 : 60);
	switch(GET_VRAM_SIZE())
	{
		case 0: Print_DrawText(" 16KB"); break;
		case 1: Print_DrawText(" 64KB"); break;
		case 2: Print_DrawText(" 128KB"); break;
		case 3: Print_DrawText(" 192KB"); break;
	}
	Print_DrawFormat(" R/W %2xh/%2xh", biosReadPort, biosWritePort);
	// Display detected information
	Print_SetPosition(1, 4);
	Print_DrawFormat("Detect: %s", GetVDPVersion());
	if(g_VDP > VDP_VERSION_TMS9918A)
		Print_DrawFormat(" %iHz", VDP_GetFrequency() == VDP_FREQ_50HZ ? 50 : 60);
	Print_Return();
}

//=============================================================================
// MENU ACTIONS
//=============================================================================

//-----------------------------------------------------------------------------
// 
const c8* MenuAction_Mode(u8 op, i8 value)
{
	value;

	switch(op)
	{
	case MENU_ACTION_SET:
	case MENU_ACTION_INC:
		g_CurMode = (g_CurMode + 1) % g_ModeNum;
		break;

	case MENU_ACTION_DEC:
		g_CurMode = (g_CurMode + (g_ModeNum - 1)) % g_ModeNum;
		break;
	}

	return g_Mode[g_CurMode].Text;
}

//-----------------------------------------------------------------------------
// 
const c8* MenuAction_Time(u8 op, i8 value)
{
	value;

	switch(op)
	{
	case MENU_ACTION_SET:
	case MENU_ACTION_INC:
		g_CurTime = (g_CurTime + 1) % numberof(g_Time);
		break;

	case MENU_ACTION_DEC:
		g_CurTime = (g_CurTime + (numberof(g_Time) - 1)) % numberof(g_Time);
		break;
	}

	return g_Time[g_CurTime].Text;
}

//-----------------------------------------------------------------------------
// 
const c8* MenuAction_Name(u8 op, i8 value)
{
	value;

	if(op == MENU_ACTION_SET)
	{
		Print_DrawCharXAt(12, 6, ' ', 40 - 12);
		g_MachineName = GetStringAt(12, 6);
	}

	return g_MachineName;
}


//-----------------------------------------------------------------------------
// 
const c8* MenuAction_Count(u8 op, i8 value)
{
	value;

	switch(op)
	{
	case MENU_ACTION_SET:
	case MENU_ACTION_INC:
		g_IterationCount = (g_IterationCount + 1) % numberof(g_IterationText);
		break;

	case MENU_ACTION_DEC:
		g_IterationCount = (g_IterationCount + (numberof(g_IterationText) - 1)) % numberof(g_IterationText);
		break;
	}

	return g_IterationText[g_IterationCount];
}

//-----------------------------------------------------------------------------
// 
const c8* MenuAction_Test(u8 op, i8 value)
{
	if(op == MENU_ACTION_SET)
	{
		switch(value)
		{
		case 0: // Test current config
			Test(g_CurMode, g_CurTime);
			break;

		case 1: // Test all config
			TestAll();
			break;

		case 2: // Report
			g_CurResult = &g_ReportAve;
			FSM_SetState(&State_Report);
			break;

		case 3: // Reset
			Reset();
			break;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// 
void MenuInit_Sub()
{
	Print_Clear();
	DisplayHeader();
	Print_DrawLineH(0, 5, 40);
}

//-----------------------------------------------------------------------------
// 
void MenuInit_Main()
{
	MenuInit_Sub();
	Print_DrawLineH(0, 16, 40);
}

//=============================================================================
// STATES
//=============================================================================

//-----------------------------------------------------------------------------
//
void State_Menu_Begin()
{
	// DisplayHeader();
	// Print_DrawLineH(0, 5, 40);

	// Menu
	Menu_Initialize(g_Menus);
	Menu_DrawPage(0);

	// Print_DrawLineH(0, 16, 40);
}

//-----------------------------------------------------------------------------
//
void State_Menu_Update()
{
	Menu_Update();

	if(Keyboard_IsKeyPressed(KEY_T))
		Test(g_CurMode, g_CurTime);

	if(Keyboard_IsKeyPressed(KEY_E))
		TestAll();

	if(Keyboard_IsKeyPressed(KEY_R))
	{
		g_CurResult = &g_ReportAve;
		FSM_SetState(&State_Report);
	}
}

//-----------------------------------------------------------------------------
//
void State_Report_Begin()
{
	DisplayHeader();
	Print_SetPosition(1, 5);
	Print_DrawFormat("Count:%sx%i Sprite:%c Screen:%c", g_IterationText[g_IterationCount], TEST_COUNT, g_DisplaySprite ? 0x0C : 0x0B, g_DisplayScreen ? 0x0C : 0x0B);

	u8 x = 4;
	u8 y = 7;
	u8 col = 0;
	// Table
	for(u8 t = 0; t < numberof(g_Time); ++t)
	{
		if(!g_SelectTimes[t]) // Skip unselected test functions
			continue;
		if(++col > 9)
			break;

		Print_SetPosition(x, y);
		Print_DrawFormat("%it", g_Time[t].Time + g_TimeOffset);
		x += 4;
	}
	for(u8 m = 0; m < numberof(g_Mode); ++m)
	{
		x = 0;
		y = 9 + m;
		Print_SetPosition(x, y);
		Print_DrawText(g_Mode[m].Code);
		x += 3;

		col = 0;
		for(u8 t = 0; t < numberof(g_Time); ++t)
		{
			if(!g_SelectTimes[t]) // Skip unselected test functions
				continue;
			if(++col > 9)
				break;

			Print_SetPosition(x, y);
			if((g_Time[t].Time + g_TimeOffset >= g_ModeLimit[m]) && ((t == 0) || (g_Time[t - 1].Time + g_TimeOffset < g_ModeLimit[m])))
				Print_DrawChar(0x16);
			else
				Print_DrawChar(0x1D);
			x += 1;
			Print_SetPosition(x, y);
			u8 percentage = (*g_CurResult)[t][m];
			if(percentage == 0xFF)
				Print_DrawText("\x7\x7\x7");
			else if(percentage == 100)
				Print_DrawText("OK");
			else
			{
				Print_DrawInt(percentage);
				Print_DrawChar('%');
			}
			x += 3;
		}
		Print_SetPosition(x, y);
		Print_DrawChar(0x1D);
	}
	Print_DrawLineH(0, 22, 40);
	Print_SetPosition(1, 23);
	Print_DrawText("F1:Average  F2:Min  F3:Max  \x83:Back");
}

//-----------------------------------------------------------------------------
//
void State_Report_Update()
{
	if(Keyboard_IsKeyPressed(KEY_F1))
	{
		g_CurResult = &g_ReportAve;
		State_Report_Begin();
	}
	else if(Keyboard_IsKeyPressed(KEY_F2))
	{
		g_CurResult = &g_ReportMin;
		State_Report_Begin();
	}
	else if(Keyboard_IsKeyPressed(KEY_F3))
	{
		g_CurResult = &g_ReportMax;
		State_Report_Begin();
	}

	if(Keyboard_IsKeyPressed(KEY_SPACE))
		FSM_SetState(&State_Menu);
}

//=============================================================================
// MAIN LOOP
//=============================================================================

//-----------------------------------------------------------------------------
/// Program entry point
u8 main(u8 argc, const c8** argv)
{
	argc; argv;

	// Initialize screen
	g_VDP = VDP_GetVersion(); // must be called before VDP_SetMode
	VDP_SetMode(VDP_MODE_SCREEN0);
	VDP_ClearVRAM();
	if(g_VDP >= VDP_VERSION_V9958)
		VDP_RegWriteBak(25, 0); // Reset MSX2+ R#25 register (to work around wrong MSX2+ BIOS for Omega)

	// Initialize font
	Print_SetTextFont(g_Font_MGL_Sample6, 0);
	Print_SetColor(COLOR_WHITE, COLOR_BLACK);

	// Get machine name
#if (TARGET_TYPE == TYPE_DOS)
	if(argc > 0)
	{
		g_MachineName = argv[0];
	}
	else
#endif
	{
		Print_DrawText("Machine:\n\n(press \x84 to validate)");
		g_MachineName = GetStringAt(9, 0);
	}

	// Initialize sprite data
	VDP_SetSpriteFlag(VDP_SPRITE_SIZE_16 | VDP_SPRITE_SCALE_2);
	VDP_WriteVRAM_16K(g_Font_MGL_Sample6 + 4, 0x3800, 256*8);
	VDP_WriteVRAM_16K((const u8*)g_SpriteAttr, 0x1A00, 32*4);
	VDP_FillVRAM_16K(COLOR_LIGHT_RED, 0x1800, 0x200);

	// Initialize variables
	g_DisplaySprite = TRUE;
	g_DisplayScreen = TRUE;
	g_IterationCount = 4;
	g_TimeOffset = 0;
	g_DestAddr = VDP_GetLayoutTable() + (40 * 17);
	const bool* defaultTime;
	switch(g_VDP)
	{
	case VDP_VERSION_TMS9918A:
		g_ModeNum = 4;
		g_ModeLimit = g_ModeLimitMSX1;
		defaultTime = g_DefaultTestMSX1;
		break;

	case VDP_VERSION_V9938:
		g_ModeNum = 10;
		g_ModeLimit = g_ModeLimitMSX2;
		defaultTime = g_DefaultTestMSX2;
		break;

	case VDP_VERSION_V9958:
		g_ModeNum = numberof(g_Mode);
		g_ModeLimit = g_ModeLimitMSX2;
		defaultTime = g_DefaultTestMSX2;
		break;
	}
	g_CurMode = 0;
	for(u8 i = 0; i < numberof(g_Time); ++i)
		g_SelectTimes[i] = defaultTime[i];
	g_CurTime = 0;
	for(u8 i = 0; i < numberof(g_Mode); ++i)
		g_SelectModes[i] = TRUE;

	Reset();
	FSM_SetState(&State_Menu);

	u8 count = 0;
	while(!Keyboard_IsKeyPressed(KEY_ESC))
	{
		// Wait V-Blank
		Halt();

		// Update states
		FSM_Update();

		// Sign of life
		Print_SetPosition(39, 0);
		Print_DrawChar(176 + (count++ / 2 % 8));
	}

	Bios_Exit(0);
}