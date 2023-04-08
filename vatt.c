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

//=============================================================================
// DEFINES
//=============================================================================

// Version
#define APP_VERSION "0.4"

// Library's logo
#define MSX_GL "\x01\x02\x03\x04\x05\x06"

// VRAM access counter
#define FILL_COUNT					256

// Fill function callback
typedef void (*cbFill)(u8);

//
struct FillTime
{
	const c8* Code;
	const c8* Text;
	cbFill    Function;
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

// VRAM fill functions prototypes
void Fill_12(u8 value);				// Fill VRAM - 12 T-States - out(n),a
void Fill_14(u8 value);				// Fill VRAM - 14 T-States - out(c),a
void Fill_17(u8 value);				// Fill VRAM - 17 T-States - out(n),a; nop
void Fill_19(u8 value);				// Fill VRAM - 19 T-States - out(c),a; nop
void Fill_20(u8 value);				// Fill VRAM - 20 T-States - out(n),a; or 0
void Fill_22(u8 value);				// Fill VRAM - 22 T-States - out(n),a; nop; nop
void Fill_29(u8 value);				// Fill VRAM - 29 T-States - out(n),a; or 0; djnz

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
const struct FillTime g_Time[] =
{
	{ "12ts", "12 TS - out(n),a",             Fill_12 },
	{ "14ts", "14 TS - out(c),a",             Fill_14 },
	{ "17ts", "17 TS - out(n),a; nop",        Fill_17 },
	{ "19ts", "19 TS - out(c),a; nop",        Fill_19 },
	{ "20ts", "20 TS - out(n),a; or 0",       Fill_20 },
	{ "22ts", "22 TS - out(n),a; nop; nop",   Fill_22 },
	{ "29ts", "29 TS - out(n),a; or 0; djnz", Fill_29 },
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

// Menu main page
const MenuItem g_MenuMain[] =
{
	{ "Mode",     MENU_ITEM_ACTION, MenuAction_Mode,  0 },
	{ "Timing",   MENU_ITEM_ACTION, MenuAction_Time,  0 },
	{ "Sprite",   MENU_ITEM_BOOL,   &g_DisplaySprite, 0 },
	{ "Screen",   MENU_ITEM_BOOL,   &g_DisplayScreen, 0 },
	{ "Count",    MENU_ITEM_ACTION, MenuAction_Count, 0 },
	{ "Test",     MENU_ITEM_ACTION, MenuAction_Test,  0 },
	{ "Test All", MENU_ITEM_ACTION, MenuAction_Test,  1 },
	{ "Report",   MENU_ITEM_ACTION, MenuAction_Test,  2 },
	{ "Reset",    MENU_ITEM_ACTION, MenuAction_Test,  3 },
};

// Menu pages configuration
const Menu g_Menus[] =
{
	{ NULL, g_MenuMain, numberof(g_MenuMain), NULL },
};

// States data
const FSM_State State_Menu =	{ 0, State_Menu_Begin,		State_Menu_Update,		NULL };
const FSM_State State_Report =	{ 0, State_Report_Begin,	State_Report_Update,	NULL };

//                                             T1 G1 G2 MC T2 G3 G4 G5 G6 G7 10 12
const u8 g_ModeLimitMSX1[numberof(g_Mode)] = { 0, 6, 6, 1, 9, 9, 9, 9, 9, 9, 9, 9 };
const u8 g_ModeLimitMSX2[numberof(g_Mode)] = { 4, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2 };

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
u8   g_SelectMode;					// Selected Screen mode
u8   g_SelectTime;					// Selected access time
bool g_DisplaySprite;				// Display sprite
bool g_DisplayScreen;				// Blank the screen
u16  g_DestAddr;					// VRAM destination address
u8   g_ModeNum;						// Number of available modes (depend of VDP version)
u8   g_IterationCount;				// Iteration counter
const u8* g_ModeLimit;				// Speed limit for each screen mdoe

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
	// case 0:  return "MSX 1";
	// case 1:  return "MSX 2";
	// case 2:  return "MSX 2+";
	// case 3:  return "MSX turbo R";
	case 0:  return "TMS9918";
	case 1:  return "V9938";
	case 2:  return "V9958";
	case 3:  return "V9958";
	}
	return "Unknow";
}

//-----------------------------------------------------------------------------
// Get VDP version
const c8* GetVDPVersion()
{
	switch(g_VDP)
	{
	case VDP_VERSION_TMS9918A: return "TMS9918";
	case VDP_VERSION_V9938:    return "V9938";
	case VDP_VERSION_V9958:    return "V9958";
	}
	return "Unknow";
}

//-----------------------------------------------------------------------------
// Dummy assembler function
void SetWriteVRAM(u16 dest)
{
	dest; // HL
	__asm
		// Setup destination address (LSB)
		ld		a, l
		out		(P_VDP_ADDR), a			// RegPort = (dest & 0x00FF);

		// Setup destination address (MSB)
		ld		a, h
		and		a, #0x3F				// reset 2 MSB bits
		or		a, #F_VDP_WRIT			// add write flag
		out		(P_VDP_ADDR), a			// RegPort = ((dest >> 8) & 0x3F) + F_VDP_WRIT;
	__endasm;
}

//-----------------------------------------------------------------------------
// Fill VRAM - 12 T-States - out(n),a
void Fill_12(u8 value)
{
	value; // A
	__asm
		.rept FILL_COUNT
		out		(P_VDP_DATA), a			// 12 ts
		.endm
	__endasm;
}

//-----------------------------------------------------------------------------
// Fill VRAM - 14 T-States - out(c),a
void Fill_14(u8 value)
{
	value; // A
	__asm
		ld		c, #P_VDP_DATA
		.rept FILL_COUNT
		out		(c), a					// 14 ts
		.endm
	__endasm;
}

//-----------------------------------------------------------------------------
// Fill VRAM - 17 T-States - out(n),a; nop
void Fill_17(u8 value)
{
	value; // A
	__asm
		.rept FILL_COUNT
		out		(P_VDP_DATA), a			// 12 ts
		nop								//  5 ts
		.endm
	__endasm;
}

//-----------------------------------------------------------------------------
// Fill VRAM - 19 T-States - out(c),a; nop
void Fill_19(u8 value)
{
	value; // A
	__asm
		ld		c, #P_VDP_DATA
		.rept FILL_COUNT
		out		(c), a					// 14 ts
		nop								//  5 ts
		.endm
	__endasm;
}

//-----------------------------------------------------------------------------
// Fill VRAM - 20 T-States - out(n),a; or 0
void Fill_20(u8 value)
{
	value; // A
	__asm
		.rept FILL_COUNT
		out		(P_VDP_DATA), a			// 12 ts
		or		#0						//  8 ts
		.endm
	__endasm;
}

//-----------------------------------------------------------------------------
// Fill VRAM - 22 T-States - out(n),a; nop; nop
void Fill_22(u8 value)
{
	value; // A
	__asm
		.rept FILL_COUNT
		out		(P_VDP_DATA), a			// 12 ts
		nop								//  5 ts
		nop								//  5 ts
		.endm
	__endasm;
}

//-----------------------------------------------------------------------------
// Fill VRAM - 29 T-States - out(n),a; or 0; djnz
void Fill_29(u8 value)
{
	value; // A
	__asm
		ld		b, #0
	fill29:
		out		(P_VDP_DATA), a			// 12 ts
		or		#0						//  8 ts
		djnz	fill29					//  9 ts
	__endasm;
}

//-----------------------------------------------------------------------------
// Test a given screen mode with a given fill function
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
	u16 total = 0;
	u8 min = 255;
	u8 max = 0;
	u8 itNum = 1 << g_IterationCount; // Compute iteration number

	for(u8 j = 0; j < itNum; ++j)
	{
		// Write reference
		DisableInterrupt();
		u16 addr = g_DestAddr;
		SetWriteVRAM(addr);
		Fill_29(0x09);

		// Test the given writing function
		SetWriteVRAM(addr);
		g_Time[time].Function(0x0A);
		EnableInterrupt();

		// Check result
		u16 count = 0;
		for(u16 i = 0; i < 256; ++i)
			if(VDP_Peek_16K(addr++) == 0x0A)
				count++;

		// Compute percentage and store total, min and max
		u8 percent = (u8)((count * 100) / 256);
		total += percent;
		if(percent < min)
			min = percent;
		if(percent > max)
			max = percent;
	}
	total >>= g_IterationCount; // Scale down the total to get the average

	g_ReportAve[time][mode] = (u8)total;
	g_ReportMin[time][mode] = min;
	g_ReportMax[time][mode] = max;

	if(g_VDP == VDP_VERSION_TMS9918A)
		VDP_SetSpritePositionY(0, VDP_SPRITE_DISABLE_SM1);
	else
		VDP_EnableSprite(FALSE);
	VDP_EnableDisplay(TRUE);

	VDP_SetMode(VDP_MODE_SCREEN0); // Restore Screen mode 0
	VDP_SetColor(COLOR_MERGE(COLOR_WHITE, COLOR_BLACK));

	Print_SetPosition(28, 23);
	Print_DrawFormat("Result: %i", total);
	Print_DrawText("%  ");
}

//-----------------------------------------------------------------------------
// Test all screen mode and fill function
void TestAll()
{
	for(u8 j = 0; j < g_ModeNum; ++j)
		for(u8 i = 0; i < numberof(g_Time); ++i)
			Test(j, i);

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
	Fill_29(' ');
}

//-----------------------------------------------------------------------------
// 
void DisplayHeader()
{
	Print_Clear();
	Print_SetPosition(0, 0);
	Print_DrawText(MSX_GL" VRAM Access Timing Tester "APP_VERSION);
	Print_DrawLineH(0, 1, 40);
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
		g_SelectMode = (g_SelectMode + 1) % g_ModeNum;
		break;
	case MENU_ACTION_DEC:
		g_SelectMode = (g_SelectMode + (g_ModeNum - 1)) % g_ModeNum;
		break;
	}

	return g_Mode[g_SelectMode].Text;
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
		g_SelectTime = (g_SelectTime + 1) % numberof(g_Time);
		break;
	case MENU_ACTION_DEC:
		g_SelectTime = (g_SelectTime + (numberof(g_Time) - 1)) % numberof(g_Time);
		break;
	}

	return g_Time[g_SelectTime].Text;
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
		{
			Test(g_SelectMode, g_SelectTime);
			break;
		}
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

//=============================================================================
// STATES
//=============================================================================

//-----------------------------------------------------------------------------
//
void State_Menu_Begin()
{
	DisplayHeader();

	// System information (use interslot access for MSX-DOS)
	u8 biosReadPort  = Bios_InterSlotRead(g_MNROM, 0x0006);
	u8 biosWritePort = Bios_InterSlotRead(g_MNROM, 0x0007);
	u8 biosVersion   = Bios_InterSlotRead(g_MNROM, 0x002B);
	u8 biosNumber    = Bios_InterSlotRead(g_MNROM, 0x002D);
	// Display BIOS information
	Print_SetPosition(1, 2);
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
	Print_SetPosition(1, 3);
	Print_DrawFormat("Detect: %s %iHz\n", GetVDPVersion(), VDP_GetFrequency() == VDP_FREQ_50HZ ? 50 : 60);
	Print_DrawLineH(0, 4, 40);

	// Menu
	Menu_Initialize(g_Menus);
	Menu_DrawPage(0);

	Print_DrawLineH(0, 16, 40);
}

//-----------------------------------------------------------------------------
//
void State_Menu_Update()
{
	Menu_Update();

	if(Keyboard_IsKeyPressed(KEY_RET))
		Test(g_SelectMode, g_SelectTime);

	if(Keyboard_IsKeyPressed(KEY_R))
	{
		g_CurResult = &g_ReportAve;
		FSM_SetState(&State_Report);
	}

	if(Keyboard_IsKeyPressed(KEY_T))
		TestAll();
}

//-----------------------------------------------------------------------------
//
void State_Report_Begin()
{
	DisplayHeader();
	Print_SetPosition(1, 2);
	Print_DrawFormat("Count:%sx256 Sprite:%c Screen:%c", g_IterationText[g_IterationCount], g_DisplaySprite ? 0x0C : 0x0B, g_DisplayScreen ? 0x0C : 0x0B);
	Print_DrawLineH(0, 3, 40);

	// Table
	for(u8 i = 0; i < numberof(g_Time); ++i)
	{
		Print_SetPosition(4 + i * 5, 5);
		Print_DrawText(g_Time[i].Code);
	}
	for(u8 j = 0; j < numberof(g_Mode); ++j)
	{
		u8 x = 0;
		u8 y = 7 + j;
		Print_SetPosition(x, y);
		Print_DrawText(g_Mode[j].Code);
		x += 3;

		for(u8 i = 0; i < numberof(g_Time); ++i)
		{
			Print_SetPosition(x, y);
			if(i == g_ModeLimit[j])
				Print_DrawChar(0x16);
			else
				Print_DrawChar(0x1D);
			x += 1;
			Print_SetPosition(x, y);
			u8 percentage = (*g_CurResult)[i][j];
			if(percentage == 0xFF)
				Print_DrawText(" \x7\x7");
			else if(percentage == 100)
				Print_DrawText(" OK");
			else
			{
				Print_DrawInt(percentage);
				Print_DrawChar('%');
			}
			x += 4;
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
void main()
{
	// Initialize screen
	g_VDP = VDP_GetVersion(); // must be called before VDP_SetMode
	VDP_SetMode(VDP_MODE_SCREEN0);
	VDP_ClearVRAM();

	// Initialize font
	Print_SetTextFont(g_Font_MGL_Sample6, 0);
	Print_SetColor(COLOR_WHITE, COLOR_BLACK);

	// Initialize sprite data
	VDP_SetSpriteFlag(VDP_SPRITE_SIZE_16 | VDP_SPRITE_SCALE_2);
	VDP_WriteVRAM_16K(g_Font_MGL_Sample6 + 4, 0x3800, 256*8);
	VDP_WriteVRAM_16K((const u8*)g_SpriteAttr, 0x1A00, 32*4);
	VDP_FillVRAM_16K(COLOR_LIGHT_RED, 0x1800, 0x200);

	// Initialize variables
	g_SelectMode = 0;
	g_SelectTime = 0;
	g_DisplaySprite = TRUE;
	g_DisplayScreen = TRUE;
	g_IterationCount = 4;
	g_DestAddr = VDP_GetLayoutTable() + (40 * 17);
	switch(g_VDP)
	{
	case VDP_VERSION_TMS9918A:
		g_ModeNum = 4;
		g_ModeLimit = g_ModeLimitMSX1;
		break;

	case VDP_VERSION_V9938:
		g_ModeNum = 10;
		g_ModeLimit = g_ModeLimitMSX2;
		break;

	case VDP_VERSION_V9958:
		g_ModeNum = numberof(g_Mode);
		g_ModeLimit = g_ModeLimitMSX2;
		break;
	}
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