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

// Fill VRAM - 12 T-States - out(n),a
void Fill_12(u8 value);

// Fill VRAM - 14 T-States - out(c),a
void Fill_14(u8 value);

// Fill VRAM - 17 T-States - out(n),a; nop
void Fill_17(u8 value);

// Fill VRAM - 18 T-States - otir
void Fill_18(u8 value);

// Fill VRAM - 19 T-States - out(c),a; nop
void Fill_19(u8 value);

// Fill VRAM - 20 T-States - out(n),a; or 0
void Fill_20(u8 value);

// Fill VRAM - 22 T-States - out(n),a; nop; nop
void Fill_22(u8 value);

// Fill VRAM - 29 T-States - out(n),a; or 0; djnz
void Fill_29(u8 value);
