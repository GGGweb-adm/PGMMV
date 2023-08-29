How to make an open source version of Players of Pixel Game Maker MV.

Preparation:
First install Visual Studio 2022 Community.
If you use Visual Studio 2022 instead of Community, change "C:\Program Files\Microsoft Visual Studio 2022″ in build_player.bat.

To build:
Open the Installer folder.
Run build_player.bat.
If successful, three folders will be created in the Installer folder: player, runtime, and debugMenuRuntime.
*If you need to rebuild libchipmunk.lib, see ../Player/cocos2d/external/win32-specific/chipmunk/libchipmunk/readme_e.txt.
*Do not include multi-byte characters such as Japanese in the build path name.

How to replace:
Open the installation folder of the Steam version of Pixel Game Maker MV. (We call it the PGM folder.)

- Replace the player used in the test play.
Overwrite the Installer/player folder with the PGM/player-win folder.

- Replace the player used in "Build Game" ("Enable Debugging Function" is OFF).
Extract PGM/resource/runtime.zip. (PGM/resource/runtime folder will be created.)
Overwrite the Installer/runtime folder with the PGM/resource/runtime folder.
Zip the PGM/resource/runtime folder and overwrite it with PGM/resource/runtime.zip.

- Replace the player used in "Build Game" ("Enable Debug Function Function" is ON).
Extract PGM/resource/debugMenuRuntime.zip. (The folder PGM/resource/debugMenuRuntime will be created.)
Overwrite the Installer/debugMenuRuntime folder with the PGM/resource/runtime debugMenuRuntime.
Zip the PGM/resource/debugMenuRuntime folder and overwrite it with PGM/resource/debugMenuRuntime.zip.

Caution:
The open source version does not support encryption builds. Encrypted games will not start.

That's it.