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

//  Initialize test context
void InitializeTest();	

// Set VRAM address to write in
extern void SetWriteVRAM(u16 dest) __PRESERVES(b, c, d, e, iyl, iyh);

// Set VRAM address to read from
extern void SetReadVRAM(u16 dest) __PRESERVES(b, c, d, e, iyl, iyh);

// test VRAM - 12 T-States - out(n),a
extern void Test_12(u8 value);

// test VRAM - 14 T-States - out(c),a
extern void Test_14(u8 value);

// test VRAM - 17 T-States - out(n),a; nop
extern void Test_17(u8 value);

// test VRAM - 18 T-States - outi
extern void Test_18(u8 value);

// test VRAM - 19 T-States - out (c),a; nop
extern void Test_19(u8 value);

// test VRAM - 20 T-States - out (n),a; cp (hl)
extern void Test_20(u8 value);

// test VRAM - 21 T-States - out (c),a; inc de
extern void Test_21(u8 value);

// test VRAM - 22 T-States - out (c),a; cp (hl)
extern void Test_22(u8 value);

// test VRAM - 23 T-States - otir
extern void Test_23(u8 value);

// test VRAM - 24 T-States - out (n),a; inc (hl)
extern void Test_24(u8 value);

// test VRAM - 25 T-States - outi; inc de
extern void Test_25(u8 value);

// test VRAM - 26 T-States - out (n),a; djnz
extern void Test_26(u8 value);

// test VRAM - 27 T-States - out (n),a; cp (hl); inc de
extern void Test_27(u8 value);

// test VRAM - 28 T-States - out (n),a; cp (hl) x 2
extern void Test_28(u8 value);

// test VRAM - 29 T-States - outi; jp nz
extern void Test_29(u8 value);

// test VRAM - 30 T-States - out (c),a; cp (hl) x 2
extern void Test_30(u8 value);

// test VRAM - 31 T-States - out(n),a; nop; djnz
extern void Test_31(u8 value);