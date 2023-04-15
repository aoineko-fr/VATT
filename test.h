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

// Set VRAM address to write in
void SetWriteVRAM(u16 dest) __PRESERVES(b, c, d, e, iyl, iyh);

// Set VRAM address to read from
void SetReadVRAM(u16 dest) __PRESERVES(b, c, d, e, iyl, iyh);

// test VRAM - 12 T-States - out(n),a
void Test_12(u8 value);

// test VRAM - 14 T-States - out(c),a
void Test_14(u8 value);

// test VRAM - 17 T-States - out(n),a; nop
void Test_17(u8 value);

// test VRAM - 18 T-States - outi
void Test_18(u8 value);

// test VRAM - 19 T-States - out (c),a; nop
void Test_19(u8 value);

// test VRAM - 20 T-States - out (n),a; cp (hl)
void Test_20(u8 value);

// test VRAM - 21 T-States - out (c),a; inc de
void Test_21(u8 value);

// test VRAM - 22 T-States - out (c),a; cp (hl)
void Test_22(u8 value);

// test VRAM - 23 T-States - otir
void Test_23(u8 value);

// test VRAM - 24 T-States - out (n),a; inc (hl)
void Test_24(u8 value);

// test VRAM - 25 T-States - outi; inc de
void Test_25(u8 value);

// test VRAM - 26 T-States - out (n),a; djnz
void Test_26(u8 value);

// test VRAM - 27 T-States - out (n),a; cp (hl); inc de
void Test_27(u8 value);

// test VRAM - 28 T-States - out (n),a; cp (hl) x 2
void Test_28(u8 value);

// test VRAM - 29 T-States - outi; jp nz
void Test_29(u8 value);

// test VRAM - 30 T-States - out (c),a; cp (hl) x 2
void Test_30(u8 value);

// test VRAM - 31 T-States - out(n),a; nop; djnz
void Test_31(u8 value);