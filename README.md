# killer

![screenshot](screenshot.png)

The goal of this project is to stop the Windows compositor (dwm.exe) to improve game performances by using exclusive fullscreen.
We cannot disable the compositor properly since Windows 8.
The program provide a simple window to disable/enable the compositor.

it kills explorer.exe and dwm.exe dynamically. It also suspends winlogon.exe

This program gathers some useful functions. The Windows API documentation is not complete so it take a lot of time to grab relevant informations.
This is an alternative to Sysinternal's program called "psuspend.exe"

**__Key binding__**

Escape / Alt F4 to quit -> it will automatically restart all suspended processes.

click on the buttons to suspend or restart processes.

*I made my tests on Windows 10 22H2*

# Build

Install `mingw` and `make` using chocolatey for example.

Then compile the C program using `make` command.
