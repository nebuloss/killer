# killer
The goal of this project is to stop the Windows compositor (dwm.exe) to improve game performances by using exclusive fullscreen.
We cannot disable the compositor properly since Windows 8.
The program provide a simple window to disable/enable the compositor.

it kills explorer.exe and dwm.exe dynamically. It also suspends winlogon.exe

This program gathers some useful functions. The Windows API documentation is not complete so it take a lot of time to grab relevant informations.
This is an alternative to Sysinternal's program called "psuspend.exe"


**__Key binding__**

Escape / Alt F4 to quit -> it will automatically restart all suspended processes.

click on the buttons to suspend or restart processes.

**__Issues__**

This program requires elevated privileges to work properly. It tested it on Windows 10 LTSB (1607)
