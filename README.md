# アクションゲームツクールMV オープンソース版プレーヤー / Pixel Game Maker MV Player Open Source Edition

## はじめに / Introduction

"アクションゲームツクールMV オープンソース版プレーヤー"は、"アクションゲームツクールMV"製のゲームソフトウェアをWindows PC上で動作させるためのプレーヤーアプリケーションです。

"Pixel Game Maker MV Player Open Source Edition" is player application for running game software made with "Pixel Game Maker MV" on Windows.

## プロジェクト概要 / What is this project?

本プロジェクトは、コミュニティやクリエイターの力によって"アクションゲームツクールMV"のプレーヤーアプリケーションを改良することを目的としたプロジェクトです。
本プロジェクトにて開発されたプレーヤーの修正は、Gotcha Gotcha Gamesを通して"アクションゲームツクールMV"の利用者へと配信されます。
本プロジェクトで公開されるプレーヤーのバージョンは1.0.6.5となります。1.0.6.6以降の修正は反映されていないことにご留意ください。

This project is a project aimed at improving "Pixel Game Maker MV player application" better by the community and supporting many game creators.
The core script developed by this project is widely distributed to PGMMV Maker users through Gotcha Gotcha Games.
Please note that the version of the player released in this project is 1.0.6.5, which does not reflect the fixes made in 1.0.6.6 or later.

## 参加方法 / How to join

- 本プロジェクトでは、日本語と英語を主要言語として利用します。
- ワークフローはGithub Flowです。PRを送るときは、新しいfeatureブランチを用意して、このリポジトリのmasterブランチに送る必要があります。http://scottchacon.com/2011/08/31/github-flow.html
- 本プロジェクトでは、"Visual Studio 2022 Community"を利用します。C++のコンポーネントのインストールが必要です。
- 本プロジェクトで利用している"cocos2d-x"のバージョンは、3.17.1です。
- 本プロジェクトに関する議論の補助には、Pixel Game Maker MV公式Discordチャンネルを利用します。https://discord.com/invite/FtRVRkx5tP

- This project uses English as the main language.
- The workflow is Github Flow. When sending PR, prepare a new feature branch and send it to the master branch of this repository. http://scottchacon.com/2011/08/31/github-flow.html
- This project uses "Visual Studio 2022 Community" with C++ componets.
- The version of "cocos2d-x" used in this project is 3.17.1.
- The official Pixel Game Maker MV Discord channel will be used to assist in discussions. https://discord.com/invite/FtRVRkx5tP

## ロードマップ / Roadmap

本プロジェクトは第一段階として、バグ修正を目的とします。
自作のゲーム用のカスタマイズプレーヤーの制作や、新機能の追加等をお楽しみいただくことはできますが、PRは基本的にはバグの修正に関わるものを中心に受け付けます。
コミット頂いたバグ修正はレビュー後、『アクションゲームツクールMV』製品版のプレーヤーに反映します。その際スタッフクレジットにコントリビューターとして掲載させていただきます。

This project aims to fix bugs as a first step.
You can use a customized version of the player that improves performance for your game, but PR will only be accepted related to bug fixes.
Contributors will have their names added to the "Pixel Game Maker MV" developer credits

## ビルド方法 / How to build

Installerフォルダ内の
[readme_j.txt](/Installer/readme_j.txt)
をご確認ください。

Please check [readme_e.txt](/Installer/readme_e.txt)

## カスタマイズプレーヤーの利用方法 / How to use cutomized version of the player
2023/11/21現在、カスタマイズされたプレーヤーアプリケーションをご利用いただくためには、
[1.0.6-OSplayer-beta]ブランチをご利用いただく必要がございます。
※こちらのブランチは1.0.6.8 相当となります。
本ブランチでは、[ツール]>[オプション]より、プレーヤーを指定することが可能です。

# プレーヤー：エディター上でテストプレイを行う際に使用するプレーヤーです。
デフォルトのplayerはアクションゲームツクールMVインストールフォルダ内、player-win>player.exeとなります。
# ランタイム：ゲームをビルドする際に同梱されるプレーヤーです。
デフォルトのplayerはアクションゲームツクールMVインストールフォルダ内、resource>runtime.zipとなります。
# デバッグ用ランタイム：[デバッグ機能を有効化]した状態でゲームをビルドする際に同梱されるプレーヤーです。
デフォルトのplayerはアクションゲームツクールMVインストールフォルダ内、resource>debugMenuRuntime.zipとなります。

From November 21, 2023, it will be possible to use a customized Player component via the “1.0.6-Osplayer-beta” branch on Steam.
After enabling this branch, you will have the option to specify versions to use for the Player, Runtine, and Debugging Runtime components.

# Player: This is the version of the Player component used when performing a test play via the PGMMV editor.
The default version is the normal Player component installed with PGMMV’s editor at (install folder)/player-win/player.exe.
# Runtime: This is the version of the Player component packaged with a game during the build process, and thus is the version people who play your game will use.
The default version is the one included with a PGMMV install: (install folder)/resource/runtime.zip
# Debugging Runtime: A version of the Runtime used when a game is built with the debug flag enabled.
The default version is the one included with a PGMMV install: (install folder)/resource/debugMenuRuntime.zip

## ライセンス / License

MITライセンス (http://opensource.org/licenses/MIT) の下で配布されます。
This content is released under the MIT License.
[LICENSE](LICENSE)
