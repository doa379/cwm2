# cwm2

cwm2 X11 WM

Introducing a next-gen Unix'y WM for X11.

This projects aims to explore "new paradigms" in managing window clients 
under the X-Window System. You compile the config a la dwm style. If you
are familiar with dwm then this Wm should come fairly naturally. This 
project aims to address a number of ideas that have been festering with
dwm over the last decade or so.

As far as Wms, the representation (and corresponding usage) of the desktop 
reached a milestone of a reference design several decades ago now. A
typical windowing desktop is a reasonably optimised way of organising your
information on a 2D platter. There is no shortage of Wms, and you can easily 
find a Wm from the plethora of choices available, and it will meet your 
requirements and work flow off the bat.

So why yet another Wm?

Presented here are a combination of features that make this Wm unique. The
motivation is not to replicate what already exists. The following are a 
genuinely unique combination of features. The codebase has been founded 
from first principles. Most importantly, the idea here has been to cast the 
project in such a way that it is open to extensibility. Herein is the highly 
modular aspect of the project's design. The project then has to be managable 
such that the user is not inhibited or curtailed from making modifications 
to the project at any later point in time. The project has to be simple 
(even dumb), clean, easy to control, yet useful. So where are the "new 
paradigms" I was referring to? We will come to those in due course. The 
design choices are kept natural and straighforward. We don't conform to any 
corporate or vested interests.

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

CC, Make, lib-X11, lib-Xinerama

Distribution: Run make in the project's root directory. This produces the 
executable cwm2.bin which is all you need to run the Wm (no shared/dynamic 
libs required apart from the pre-installed system libs above).

Release: TBC

Target(s): Any UNIX w/ X-Window System Ver. 11, Rel. 7

(C) 2023-2026 doa379
