# Requirements

This project is best built in a Windows VM. Linux can be used for some
cross-build workflows, but the normal path for this client is a Windows build
that produces `Main.exe`.

## Recommended VM

- Windows 11 Pro, or Windows 10/11 Pro
- 4 CPU cores minimum
- 8 GB RAM minimum, 16 GB recommended
- 120 GB disk minimum, 200 GB recommended
- 3D acceleration enabled if you want to run the game client inside the VM

## Required Build Tools

- Visual Studio 2022 Community, or Build Tools for Visual Studio 2022
  - Desktop development with C++
  - MSVC v143 - VS 2022 C++ x64/x86 build tools
  - Windows SDK
  - C++ CMake tools for Windows
  - Ninja, if offered by the installer
- .NET 10 SDK
- CMake 3.25 or newer
- Git for Windows

## Recommended Tools

- 7-Zip
- Visual Studio Code or Notepad++
- Windows Terminal
- GitHub Desktop, optional
- Process Explorer or Process Monitor from Sysinternals

## Server And Testing Tools

- Local MU/OpenMU-compatible game server
- SQL Server Express, or the database required by your server
- Wireshark, optional for packet debugging
- TCPView, optional for checking ports such as `44406`, `55901`, and `55902`

## Verify The Install

Open a fresh terminal and run:

```powershell
git --version
cmake --version
dotnet --version
cl
```

Expected results:

- `git` is available.
- `cmake` is available and version 3.25 or newer.
- `dotnet` reports version `10.x`.
- `cl` prints Microsoft C/C++ compiler information.

If `cl` is not available in a normal PowerShell window, use the "x86 Native
Tools Command Prompt for VS 2022" and open the repository from there:

```cmd
cd /d D:\Desarrollo\personal\MuMain
```

## Build And Run

From the repository root:

```powershell
git submodule update --init
cmake --preset windows-x86
cmake --build --preset windows-x86-debug
```

Run the debug client against a local server:

```powershell
.\out\build\windows-x86\src\Debug\Main.exe connect /u127.0.0.1 /p44406
```

For later source changes, usually only rebuild:

```powershell
cmake --build --preset windows-x86-debug
```
