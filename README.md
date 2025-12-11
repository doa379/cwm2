# cwm2

cwm2 X11 WM

Introducing a next-gen Unix'y WM for X11.

This projects aims to explore new paradigms in managing window clients 
under the X-Windows system.

Features in a nut-shell:

* No relation to cwm, but motivated by dopenbox-wm.
* Completely new project designed & impl. from scratch.
* Borrows and improves the user experience from dwm.
* A highly modularised, "object-oriented" & extensible design.
* An integrated user-interface that uses native X-protocols.
* Integrated blocked memory management.
* A safe implementation in C.
* Tiny demands on system resources.
* Transparent performance.
* Tiling windows.
* Articulated multi-monitor support.
* Dynamic workspaces.
* Open to ricing without using patches.

Build prerequisites/deps:

CC, Make, lib-X11, lib-Xinerama, dbus-1

Distribution: The ./build.sh script produces the executable cwm2.bin 
which is all you need to run the wm (no shared/dynamic libs required 
apart from those pre-installed above). 

Release: TBC

Target(s): BSD UNIX, Linux

(C) 2023-2025 doa379
