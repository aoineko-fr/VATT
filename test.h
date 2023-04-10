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

//=============================================================================
// FUNCTIONS
//=============================================================================

// Dummy assembler function
void SetWriteVRAM(u16 dest);

// Test VRAM - 12 T-States - out(n),a
void Test_12(u8 value);

// Test VRAM - 14 T-States - out(c),a
void Test_14(u8 value);

// Test VRAM - 17 T-States - out(n),a; nop
void Test_17(u8 value);

// Test VRAM - 18 T-States - outi
void Test_18(u8 value);

// Test VRAM - 19 T-States - out(c),a; nop
void Test_19(u8 value);

// Test VRAM - 20 T-States - out(n),a; or 0
void Test_20(u8 value);

// Test VRAM - 22 T-States - out(n),a; nop; nop
void Test_22(u8 value);

// Test VRAM - 29 T-States - outi; jp
void Test_29(u8 value);

// Test VRAM - 29 T-States - out(n),a; nop; djnz
void Test_31(u8 value);

// Test VRAM - 29 T-States - out(n),a; or 0; djnz
void Test_34(u8 value);
