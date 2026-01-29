# cwm2

## cwm2 X11 WM

Introducing a next-gen Unix'y Window Manager for X11.

This projects aims to explore "new paradigms" in managing window clients under 
the X-Window System. You compile the config a la dwm style. If you are familiar 
with dwm then this Wm should come fairly naturally. This project aims to address 
a number of ideas that have been festering with dwm over the last decade or so.

As far as Wms, the representation (and corresponding usage) of the desktop 
reached a milestone of a reference design several decades ago now. A typical 
windowing desktop is a reasonably optimised way of organising your information 
on a 2D platter. There is no shortage of Wms, and you can easily find a Wm from 
the plethora of choices available, and it will meet your requirements and work 
flow off the bat.

## So why yet another Wm?

Presented here are a combination of features that make this Wm unique. The 
motivation is not to replicate what already exists. The following are a 
genuinely unique combination of features. The codebase has been founded from 
first principles. Most importantly, the idea here has been to cast the project 
in such a way that it is relatively easy to extend. Herein is the highly modular 
aspect of the project's design. The project has to be manageable such that the 
user is not inhibited or curtailed from making modifications to it at any future 
point in time. The project has to be simple (even dumb), clean, under control, 
yet useful. So where are the "new paradigms" I was referring to? We will come to 
those in due course. The design choices are kept natural and straightforward. We 
don't conform to any corporate or vested interests.

## Features in a nut-shell:

* No relation to cwm, but inspired by dopenbox-wm in the tradition of twm, the 
canonical Wm for the X-Window System.

* Completely new project imagined, designed & impl. from scratch.

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

* Slop free.

* Woke free.

* Cult free.

## Build prerequisites/deps:

```CC, Make, lib-X11, lib-Xinerama```

Distribution: Run ```make``` in the project's root directory. This produces the 
executable ```cwm2.bin``` which is all you need to run the Wm (no shared/dynamic 
libs required apart from the pre-installed system libs above).

## Release: rc2.

## Target(s): Any UNIX w/ X-Window System Ver. 11.

## User's Guide:

This overarching philosophy of this Wm is it works on the principle of 
selections. You select a group of objects (namely Clients/Windows) and then 
apply the desired operation on the group. The selection expires once an 
operation is complete. Operations are typically arrangements. It is possible to 
conceivably implement any number of operations. The default arrangement for 
clients is a floating arrangement.

The ```config.h``` file (in the root directory) initialises the program. The 
config is not entirely bomb-proof -- as such invalid values could result in 
errors. The essential settings are implemented in the ```calls.c``` file. It is 
recommended that you leave ```calls.c``` intact for reference, but instead add 
your own extensions to the ```usercalls.c``` file instead.

You can pin simple apps to the tray.

The panel shows all clients across all workspaces. It is easy to keep track of 
clients. Clients are easily switchable.

Can be entirely keyboard driven.

### Call options:

  ```.cmd  = "<...>"``` Fork a command
  
  ```.call = calls_wk<...>``` Switch to a wk (between 1, 12)
  
  ```.call = calls_cli_last``` Switch to last client
  
  ```.call = calls_cli<...>``` Switch to a cli (between 1, 12)
  
  ```.call = calls_cli_next``` Switch to next client
  
  ```.call = calls_cli_prev``` Switch to prev client
  
  ```.call = calls_cli_raise_toggle``` Raise a client
  
  ```.call = calls_arrange_toggle``` Toggle client arrangement
  
  ```.call = calls_cli_mode_toggle``` Toggle client mode
  
  ```.call = calls_sel_toggle``` Select current client
  
  ```.call = calls_sel_clear``` Clear client selection
  
  ```.call = calls_cli_last``` Switch to last focused client
  
  ```.call = calls_wk_prev``` Switch to prev wk
  
  ```.call = calls_wk_next``` Switch to next wk
  
  ```.call = calls_wk_map``` Add a wk
  
  ```.call = calls_wk_unmap``` Remove current wk
  
  ```.call = calls_kill``` Close current client
  
  ```.call = calls_cli_wk_prev_move``` Move client to prev wk
  
  ```.call = calls_cli_wk_next_move``` Move client to next wk
  
  ```.call = calls_cli_wk<...>_move``` Move client to a wk (between 1, 12)
  
  ```.call = calls_quit``` Exit the program

The theme here is simplicity for use and maintenance, transparency, low 
maintenance, low overheads. It may be simple, but still be fun to use, and sharp 
for the job.

(C) 2023-2026 doa379
