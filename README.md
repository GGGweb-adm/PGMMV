# Pixel Game Maker MV Player Open Source Edition
日本語のREADMEは[こちら](README_JP.md)です。

## Introduction

"Pixel Game Maker MV Player Open Source Edition" is an open source version of the player application need to run games and software made with "Pixel Game Maker MV" (PGMMV) for Windows.

## What is this project?

This project is a project aimed at improving the PGMMV player application with support from the community and game creators.
Improvements developed by this project will be distributed to all PGMMV users by Gotcha Gotcha Games.
The current version of the player used for this project is 1.0.6.5, which does not reflect the fixes made in 1.0.6.6 or later.

## How to join

- This project uses English as the main language, though Japanese is also allowed.
- This project follows Github Flow rules. Please prepare a new branch for any Pull Requests (PR). Once the PR is approved it must be merged to master branch. http://scottchacon.com/2011/08/31/github-flow.html
- This project requires use of "Visual Studio 2022 Community" with C++ components.
- The version of "cocos2d-x" used by this project is 3.17.1.
- Project discussions will be carried out on the official Pixel Game Maker MV Discord channel: https://discord.com/invite/FtRVRkx5tP

## Roadmap

Initially this project aims to fix outstanding bugs.
Contributors are allowed to create a customized version of the player that improves performance for their game, but for the time being only bug fix PRs will be accepted.
Contributors will have their names added to the "Pixel Game Maker MV" developer credits

## How to build

Please review the included readme: [readme_e.txt](/Installer/readme_e.txt)

## How to use cutomized version of the player

From November 21, 2023, it will be possible to use a customized Player component via the “1.0.6-Osplayer-beta” branch on Steam.
After enabling this branch, you will have the option to specify versions to use for the Player, Runtime, and Debugging Runtime components.

### Player: This is the version of the Player component used when performing a test play via the PGMMV editor.
The default version is the normal Player component installed with PGMMV’s editor at (install folder)/player-win/player.exe.
### Runtime: This is the version of the Player component packaged with a game during the build process, and thus is the version people who play your game will use.
The default version is the one included with a PGMMV install: (install folder)/resource/runtime.zip
### Debugging Runtime: A version of the Runtime used when a game is built with the debug flag enabled.
The default version is the one included with a PGMMV install: (install folder)/resource/debugMenuRuntime.zip


## License

This content is released under the MIT License.
[LICENSE](LICENSE)
