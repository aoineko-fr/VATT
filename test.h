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

// Test VRAM - 12 T-States - out(n),a
void Test_12(u8 value) __PRESERVES(b, c, d, e, h, l, iyl, iyh);

// Test VRAM - 14 T-States - out(c),a
void Test_14(u8 value) __PRESERVES(b, d, e, h, l, iyl, iyh);

// Test VRAM - 17 T-States - out(n),a; nop
void Test_17(u8 value) __PRESERVES(b, c, d, e, h, l, iyl, iyh);

// Test VRAM - 18 T-States - outi
void Test_18(u8 value) __PRESERVES(iyl, iyh);

// Test VRAM - 19 T-States - out(c),a; nop
void Test_19(u8 value) __PRESERVES(b, d, e, h, l, iyl, iyh);

// Test VRAM - 20 T-States - out(n),a; or 0
void Test_20(u8 value) __PRESERVES(b, c, d, e, h, l, iyl, iyh);

// Test VRAM - 22 T-States - out(n),a; nop; nop
void Test_22(u8 value) __PRESERVES(b, c, d, e, h, l, iyl, iyh);

// Test VRAM - 29 T-States - outi; jp
void Test_29(u8 value) __PRESERVES(iyl, iyh);

// Test VRAM - 31 T-States - out(n),a; nop; djnz
void Test_31(u8 value) __PRESERVES(c, d, e, h, l, iyl, iyh);