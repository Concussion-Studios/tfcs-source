Team Fortress Classic Source
=====

[![Build status](https://ci.appveyor.com/api/projects/status/l1ukwkp2qqn87p81/branch/master?svg=true?style=for-the-badge)](https://ci.appveyor.com/project/xXxToxicBlueDustxXx/tfcs-source-3kx9a/branch/master) ![Discord](https://img.shields.io/discord/637821719708434473?color=7289DA&label=DIscord%20Server&logoColor=2C2F33&style=for-the-badge)

This is the codebase for Team Fortress Classic: Source. Developers feel free to upload your changes here.
This modification runs off of the Source Engine. It will always be free.

You can find a link to the playable build on the latest release's page: https://github.com/Concussion-Studios/tfcs-source/releases/latest

## Dependencies

### Windows
* [Visual Studio 2013 with Update 5](https://visualstudio.microsoft.com/vs/older-downloads/)

### macOS
* [Xcode 5.0.2](https://developer.apple.com/downloads/more)

### Linux
* GCC 4.8
* [Steam Client Runtime](http://media.steampowered.com/client/runtime/steam-runtime-sdk_latest.tar.xz)

## Building

Compiling process is the same as for Source SDK 2013. Instructions for building Source SDK 2013 can be found here: https://developer.valvesoftware.com/wiki/Source_SDK_2013

Assets that need to be used with compiled binaries: https://github.com/Concussion-Studios/tfcsource

## Installing:

### Client:

1. Go to the Tools section in your Steam Library and install Source SDK Base 2013 Multiplayer. 

2. Download the asset package and extract its contents to <Steam>\steamapps\sourcemods.

3. Restart Steam. now the mod should appear in your Steam Library.

4. Put your compiled binaries into "bin" directory.

NOTE: If you're on Linux or Mac, Steam client currently has a bug where it doesn't attach -steam parameter when running Source mods like it's supposed to. You'll need to manually add -steam parameter to the mod in your Steam Library.

### Server:

1. These instructions assume you know how to host a dedicated server for TFCs and/or other Source games. If you don't, refer to these articles:

   * https://developer.valvesoftware.com/wiki/SteamCMD

2. Use SteamCMD to download app 244310 (Source SDK Base 2013 Dedicated Server).

3. Download the asset package and extract its contents to where you installed Source SDK Base 2013 Dedicated Server.

4. If you're on Linux, go to <server_install_folder>/bin and make copies of the files as follows:

   * soundemittersystem_srv.so -> soundemittersystem.so

   * scenefilecache_srv.so -> scenefilecache.so
   
5. Put your compiled binaries into "bin" directory.