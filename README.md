# VATT - VRAM Access Timing Tester
VATT is a tool to test access VRAM timng on MSX.

## Why?

Writing or reading too fast in VRAM can fail under certain conditions and generate graphic bugs. 
Until now, the reference in terms of VRAM access time was those defined on the MSX Assembly Page (http://map.grauw.nl/articles/vdp_tut.php#vramtiming). 
The problem was that these numbers did not always match the observations. 
So I created this tool to allow the MSX community to have a way to objectively analyze the actual timing under different conditions.

You can find the results here: https://aoineko.org/msxgl/index.php?title=VRAM_access_timing

## How?

VATT uses a simple method to test access times:
- It writes 256 characters (empty circles) slowly (29 t-states between each writing, which is known to be reliable even in the worst case scenario).
- On top of that, it writes 256 characters (filled circle) with different assembly code more or less fast (from 12 to 29 t-states).
- It reads slowly (more than 29 t-states) the 256 characters and counts the number of full circles. If there are less than 256, it means that some writing has been lost because of lack of time.
- Repeat these steps N times (between 1 and 128) and calculate an average, a minimum and a maximum.

And voila! we can test these different writing speeds in a multitude of contexts: each screen mode, activation or not of sprites, activation or not of display, and that, for each generation of MSX VDP.

## What?

All the following combinations can be tested.

Screen modes:
- Text mode 1 (Screen 0, 40 columns)
- Graphic mode 1 (Screen 1)
- Graphic mode 2 (Screen 2)
- Multi-color mode (Screen 3)
- Text mode 2 (Screen 0, 80 columns)
- Graphic mode 3 (Screen 4)
- Graphic mode 4 (Screen 5)
- Graphic mode 5 (Screen 6)
- Graphic mode 6 (Screen 7)
- Graphic mode 7 (Screen 8)
- Graphic mode 7 + YJK + YAE (Screen 10)
- Graphic mode 7 + YJK (Screen 12)

Fill instructions:
- 12 TS - `out (n),a`
- 14 TS - `out (c),a`
- 17 TS - `out (n),a; nop`
- 18 TS - `outi`
- 19 TS - `out (c),a; nop`
- 20 TS - `out (n),a; cp (hl)`
- 21 TS - `out (c),a; inc de`
- 22 TS - `out (c),a; cp (hl)`
- 23 TS - `otir`
- 24 TS - `out (n),a; inc (hl)`
- 25 TS - `outi; inc de`
- 26 TS - `out (n),a; djnz`
- 27 TS - `out (n),a; cp (hl); inc de`
- 28 TS - `out (n),a; cp (hl); cp (hl)`
- 29 TS - `outi; ld i,a`
- 30 TS - `out (c),a; cp (hl); cp (hl)`

Sprites:
- Enable (32 sprites displayed on screen during testing)
- Hide (use magic number for Y coordinate to hide all sprite)
- Disable (for MSX2 and above)

Display:
- Enable (normal)
- Disable (screen is blank )

 <p xmlns:cc="http://creativecommons.org/ns#" >This work is licensed under <a href="https://creativecommons.org/licenses/by-sa/4.0/?ref=chooser-v1" target="_blank" rel="license noopener noreferrer" style="display:inline-block;">CC BY-SA 4.0<img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/cc.svg?ref=chooser-v1" alt=""><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/by.svg?ref=chooser-v1" alt=""><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/sa.svg?ref=chooser-v1" alt=""></a></p> 
