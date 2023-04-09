;; ____________________________
;; ██▀▀█▀▀██▀▀▀▀▀▀▀█▀▀█        │   ▄▄▄                ▄▄      
;; ██  ▀  █▄  ▀██▄ ▀ ▄█ ▄▀▀ █  │  ▀█▄  ▄▀██ ▄█▄█ ██▀▄ ██  ▄███
;; █  █ █  ▀▀  ▄█  █  █ ▀▄█ █▄ │  ▄▄█▀ ▀▄██ ██ █ ██▀  ▀█▄ ▀█▄▄
;; ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀────────┘                 ▀▀
;;  VRAM access timing sample
;;─────────────────────────────────────────────────────────────────────────────

;;=============================================================================
;; DEFINES
;;=============================================================================

FILL_COUNT	= 256
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

;;=============================================================================
;; FUNCTIONS
;;=============================================================================

;;-----------------------------------------------------------------------------
;; Dummy assembler function
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
;; Fill VRAM - 12 T-States - out(n),a
;; void Fill_12(u8 value -> A)
;;-----------------------------------------------------------------------------
_Fill_12::
	.rept	FILL_COUNT
	out		(P_VDP_DATA), a			;; 12 ts
	.endm
	ret

;;-----------------------------------------------------------------------------
;; Fill VRAM - 14 T-States - out(c),a
;; void Fill_14(u8 value -> A)
;;-----------------------------------------------------------------------------
_Fill_14::
	ld		c, #P_VDP_DATA
	.rept	FILL_COUNT
	out		(c), a					;; 14 ts
	.endm
	ret

;;-----------------------------------------------------------------------------
;; Fill VRAM - 17 T-States - out(n),a; nop
;; void Fill_17(u8 value -> A)
;;-----------------------------------------------------------------------------
_Fill_17::
	.rept	FILL_COUNT
	out		(P_VDP_DATA), a			;; 12 ts
	nop								;;  5 ts
	.endm
	ret

;;-----------------------------------------------------------------------------
;; Fill VRAM - 18 T-States - outi
;; void Fill_18(u8 value -> A)
;;-----------------------------------------------------------------------------
_Fill_18::
	;; Create buffer in RAM
	ld		hl, #0xE000
	ld		de, #0xE001
	ld		(hl), a
	ld		bc, #FILL_COUNT-1
	;; Copy to VRAM
	ld		c, #P_VDP_DATA
	.rept	FILL_COUNT
	outi
	.endm
	ret

;;-----------------------------------------------------------------------------
;; Fill VRAM - 19 T-States - out(c),a; nop
;; void Fill_19(u8 value -> A)
;;-----------------------------------------------------------------------------
_Fill_19::
	ld		c, #P_VDP_DATA
	.rept	FILL_COUNT
	out		(c), a					;; 14 ts
	nop								;;  5 ts
	.endm
	ret

;;-----------------------------------------------------------------------------
;; Fill VRAM - 20 T-States - out(n),a; or 0
;; void Fill_20(u8 value -> A)
;;-----------------------------------------------------------------------------
_Fill_20::
	.rept	FILL_COUNT
	out		(P_VDP_DATA), a			;; 12 ts
	or		#0						;;  8 ts
	.endm
	ret

;;-----------------------------------------------------------------------------
;; Fill VRAM - 22 T-States - out(n),a; nop; nop
;; void Fill_22(u8 value -> A)
;;-----------------------------------------------------------------------------
_Fill_22::
	.rept	FILL_COUNT
	out		(P_VDP_DATA), a			;; 12 ts
	nop								;;  5 ts
	nop								;;  5 ts
	.endm
	ret

;;-----------------------------------------------------------------------------
;; Fill VRAM - 29 T-States - out(n),a; or 0; djnz
;; void Fill_29(u8 value -> A)
;;-----------------------------------------------------------------------------
_Fill_29::
	ld		b, #0
fill29:
	out		(P_VDP_DATA), a			;; 12 ts
	or		#0						;;  8 ts
	djnz	fill29					;;  9 ts
	ret