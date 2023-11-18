<h1 align="center">「🧊」 About Frosty</h1>

<p align="center"><img src="Assets/banner.png"></p>

Frosty is a sophisticated rootkit malware developed specifically for Windows operating systems, with an emphasis on its compatibility and effectiveness with Windows 10.

A notable feature of Frosty is its use of Microsoft's Detours library. This strategic choice allows the malware to engage the Windows Native API (NTAPI), the application programming interfaces essential to the operating system's functions. Manipulating NTAPI allows Frosty to bypass conventional security measures and avoid detection by traditional methods.

## Features

* Hide Processes
* Hide Services
* Hide Directory
* AMSI Bypass

## Install

### Requirements

* Operating System: Windows 10.
* Software: Microsoft Visual Studio 2022.
* Tools, Compilers: MSVC v143 Build Tools, C++ Build Insights, MSBuild.exe.

### Commands

```
git clone https://github.com/MrEmpy/Frosty.git
cd Frosty
notepad config.h # customize the macros
.\builder.ps1 (x64 or x86)
ls build/
```

Execute the `build/Deployer.exe` file to automatically deploy the rootkit to the machine and execute `build/Uninstall.exe` on the machine to uninstall the Frosty rootkit.

## PoC Video

[![](https://img.youtube.com/vi/Ji12eh6LR78/0.jpg)](https://www.youtube.com/watch?v=Ji12eh6LR78)

## Note

The Frosty rootkit is in its early development phase, so we are in beta. We welcome feedback on the rootkit so we can improve it.

This is one of my ambitious projects, as much as it is a ring 3 rootkit, it was a project in which I learned more internally about the Windows operating system. So if you find any bugs, please report them in `issues` so we can reach a final point and fix them.

## Changelog (v0.2b)

* Added suffix that allow user to customize their settings.
* New macro in `config.h` called `RK_SERVICE_DESCRIPTION` used to set the service description (if you want to change it).
* Hook the `AmsiScanBuffer` function from `amsi.dll` for evasion.
