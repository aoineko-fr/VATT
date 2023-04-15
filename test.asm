;; ____________________________
;; ██▀▀█▀▀██▀▀▀▀▀▀▀█▀▀█        │   ▄▄▄                ▄▄      
;; ██  ▀  █▄  ▀██▄ ▀ ▄█ ▄▀▀ █  │  ▀█▄  ▄▀██ ▄█▄█ ██▀▄ ██  ▄███
;; █  █ █  ▀▀  ▄█  █  █ ▀▄█ █▄ │  ▄▄█▀ ▀▄██ ██ █ ██▀  ▀█▄ ▀█▄▄
;; ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀────────┘                 ▀▀
;;  VRAM access timing sample
;;─────────────────────────────────────────────────────────────────────────────
.module	test

;;=============================================================================
;; DEFINES
;;=============================================================================

TEST_COUNT	= 256
P_VDP_0		= 0x98
P_VDP_DATA	= P_VDP_0
P_VDP_1		= 0x99
P_VDP_REG	= P_VDP_1
P_VDP_ADDR	= P_VDP_1
P_VDP_STAT	= P_VDP_1
F_VDP_REG	= 0x80 ;; VDP register write port (bit 7=1 in second write)
F_VDP_VRAM	= 0x00 ;; VRAM address register (bit 7=0 in second write)
F_VDP_WRIT	= 0x40 ;; bit 6: read/write access (1=write)
F_VDP_READ	= 0x00 ;; bit 6: read/write access (0=read)
RAM_BUFFER  = 0xE000

;;=============================================================================
;; FUNCTIONS
;;=============================================================================

;;-----------------------------------------------------------------------------
;; Set VRAM address to write in
;; void SetWriteVRAM(u16 dest -> HL)
;;-----------------------------------------------------------------------------
_SetWriteVRAM::

	;; Setup destination address (LSB)
	ld		a, l
	out		(P_VDP_ADDR), a			;; RegPort = (dest & 0x00FF);

	;; Setup destination address (MSB)
	ld		a, h
	and		a, #0x3F				;; reset 2 MSB bits
	or		a, #F_VDP_WRIT			;; add write flag
	out		(P_VDP_ADDR), a			;; RegPort = ((dest >> 8) & 0x3F) + F_VDP_WRIT;
	ret

;;-----------------------------------------------------------------------------
;; Set VRAM address to write in
;; void SetReadVRAM(u16 dest -> HL)
;;-----------------------------------------------------------------------------
_SetReadVRAM::

	;; Setup destination address (LSB)
	ld		a, l
	out		(P_VDP_ADDR), a			;; RegPort = (dest & 0x00FF);

	;; Setup destination address (MSB)
	ld		a, h
	and		a, #0x3F				;; reset 2 MSB bits
	; or		a, #F_VDP_READ			;; add read flag
	out		(P_VDP_ADDR), a			;; RegPort = ((dest >> 8) & 0x3F) + F_VDP_WRIT;
	ret

;;-----------------------------------------------------------------------------
;; Create buffer in RAM filled with register A's value
CreateBuffer:
	ld		hl, #RAM_BUFFER
	ld		de, #RAM_BUFFER+1
	ld		(hl), a
	ld		bc, #TEST_COUNT-1
	ldir
	ret

;;-----------------------------------------------------------------------------
;; test VRAM - 12 T-States - out(n),a
;; void test_12(u8 value -> A)
;;-----------------------------------------------------------------------------
_Test_12::
	.rept	TEST_COUNT
	out		(P_VDP_DATA), a			;; 12 ts	| 2 B
	.endm
	ret

;;-----------------------------------------------------------------------------
;; test VRAM - 14 T-States - out(c),a
;; void test_14(u8 value -> A)
;;-----------------------------------------------------------------------------
_Test_14::
	ld		c, #P_VDP_DATA
	.rept	TEST_COUNT
	out		(c), a					;; 14 ts	| 2 B
	.endm
	ret

;;-----------------------------------------------------------------------------
;; test VRAM - 17 T-States - out(n),a; nop
;; void test_17(u8 value -> A)
;;-----------------------------------------------------------------------------
_Test_17::
	.rept	TEST_COUNT
	out		(P_VDP_DATA), a			;; 12 ts	| 2 B
	nop								;;  5 ts	| 1 B
	.endm
	ret

;;-----------------------------------------------------------------------------
;; test VRAM - 18 T-States - outi
;; void test_18(u8 value -> A)
;;-----------------------------------------------------------------------------
_Test_18::
	;; Create buffer in RAM
	call	CreateBuffer
	;; Copy to VRAM
	ld		hl, #RAM_BUFFER
	ld		c, #P_VDP_DATA
	.rept	TEST_COUNT
	outi							;; 18 ts	| 2 B
	.endm
	ret

;;-----------------------------------------------------------------------------
;; test VRAM - 19 T-States - out (c),a; nop
;; void test_19(u8 value -> A)
;;-----------------------------------------------------------------------------
_Test_19::
	ld		c, #P_VDP_DATA
	.rept	TEST_COUNT
	out		(c), a					;; 14 ts	| 2 B
	nop								;;  5 ts	| 1 B
	.endm
	ret

;;-----------------------------------------------------------------------------
;; test VRAM - 20 T-States - out (n),a; cp (hl)
;; void test_20(u8 value -> A)
;;-----------------------------------------------------------------------------
_Test_20::
	.rept	TEST_COUNT
	out		(P_VDP_DATA), a			;; 12 ts	| 2 B
	cp		(hl)					;;  8 ts	| 1 B
	.endm
	ret

;;-----------------------------------------------------------------------------
;; test VRAM - 21 T-States - out (c),a; inc de
;; void test_21(u8 value -> A)
;;-----------------------------------------------------------------------------
_Test_21::
	ld		c, #P_VDP_DATA
	.rept	TEST_COUNT
	out		(c), a					;; 14 ts	| 2 B
	inc		de						;;  7 ts	| 1 B
	.endm
	ret

;;-----------------------------------------------------------------------------
;; test VRAM - 22 T-States - out (c),a; cp (hl)
;; void test_22(u8 value -> A)
;;-----------------------------------------------------------------------------
_Test_22::
	ld		c, #P_VDP_DATA
	.rept	TEST_COUNT
	out		(c), a					;; 14 ts	| 2 B
	cp		(hl)					;;  8 ts	| 1 B
	.endm
	ret

;;-----------------------------------------------------------------------------
;; test VRAM - 23 T-States - otir
;; void test_23(u8 value -> A)
;;-----------------------------------------------------------------------------
_Test_23::
	;; Create buffer in RAM
	call	CreateBuffer
	;; Copy to VRAM
	ld		hl, #RAM_BUFFER
	ld		b, #TEST_COUNT
	ld		c, #P_VDP_DATA
	otir							;; 23 ts	| 2 B
	ret

;;-----------------------------------------------------------------------------
;; test VRAM - 24 T-States - out (n),a; inc (hl)
;; void test_24(u8 value -> A)
;;-----------------------------------------------------------------------------
_Test_24::
	ld		hl, #RAM_BUFFER
	.rept	TEST_COUNT
	out		(P_VDP_DATA), a			;; 12 ts	| 2 B
	inc		(hl)					;; 12 ts	| 1 B
	.endm
	ret

;;-----------------------------------------------------------------------------
;; test VRAM - 25 T-States - outi; inc de
;; void test_25(u8 value -> A)
;;-----------------------------------------------------------------------------
_Test_25::
	;; Create buffer in RAM
	call	CreateBuffer
	;; Copy to VRAM
	ld		hl, #RAM_BUFFER
	ld		c, #P_VDP_DATA
	.rept	TEST_COUNT
	outi							;; 18 ts	| 2 B
	inc		de						;;  7 ts	| 1 B
	.endm
	ret

;;-----------------------------------------------------------------------------
;; test VRAM - 26 T-States - out (n),a; djnz
;; void test_26(u8 value -> A)
;;-----------------------------------------------------------------------------
_Test_26::
	ld		b, #TEST_COUNT
test26:
	out		(P_VDP_DATA), a			;; 12 ts	| 2 B
	djnz	test26					;; 14 ts
	ret

;;-----------------------------------------------------------------------------
;; test VRAM - 27 T-States - out (n),a; cp (hl); inc de
;; void test_27(u8 value -> A)
;;-----------------------------------------------------------------------------
_Test_27::
	.rept	TEST_COUNT
	out		(P_VDP_DATA), a			;; 12 ts	| 2 B
	cp		(hl)					;;  8 ts	| 1 B
	inc		de						;;  7 ts	| 1 B
	.endm
	ret

;;-----------------------------------------------------------------------------
;; test VRAM - 28 T-States - out (n),a; cp (hl) x 2
;; void test_28(u8 value -> A)
;;-----------------------------------------------------------------------------
_Test_28::
	.rept	TEST_COUNT
	out		(P_VDP_DATA), a			;; 12 ts	| 2 B
	cp		(hl)					;;  8 ts	| 1 B
	cp		(hl)					;;  8 ts	| 1 B
	.endm
	ret

;;-----------------------------------------------------------------------------
;; test VRAM - 29 T-States - outi; jp nz
;; void test_29(u8 value -> A)
;;-----------------------------------------------------------------------------
_Test_29::
	;; Create buffer in RAM
	call	CreateBuffer
	;; Copy to VRAM
	ld		hl, #RAM_BUFFER
	ld		b, #TEST_COUNT
	ld		c, #P_VDP_DATA
test29:
	outi							;; 18 ts
	jp		nz, test29				;; 11 ts
	ret

;;-----------------------------------------------------------------------------
;; test VRAM - 30 T-States - out (c),a; cp (hl) x 2
;; void test_30(u8 value -> A)
;;-----------------------------------------------------------------------------
_Test_30::
	ld		c, #P_VDP_DATA
	.rept	TEST_COUNT
	out		(c), a					;; 14 ts	| 2 B
	cp		(hl)					;;  8 ts	| 1 B
	cp		(hl)					;;  8 ts	| 1 B
	.endm
	ret

;;-----------------------------------------------------------------------------
;; test VRAM - 31 T-States - out(n),a; nop; djnz
;; void test_31(u8 value -> A)
;;-----------------------------------------------------------------------------
_Test_31::
	ld		b, #TEST_COUNT
test31:
	out		(P_VDP_DATA), a			;; 12 ts	| 2 B
	nop								;;  5 ts	| 1 B
	djnz	test31					;; 14 ts
	ret
