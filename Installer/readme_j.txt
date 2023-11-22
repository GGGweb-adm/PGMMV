アクションゲームツクールMVのオープンソース版Playerのビルド方法です。

準備：
あらかじめ、Visual Sudio 2022 Communityをインストールしておいてください。“Desktop development with C++”のワークロードのインストールも必要です。
Communityでない、Visual Studio 2022を使用される場合は、build_player.bat内の、"C:\Program Files\Microsoft Visual Studio\2022"配下を指定するパスを変更してください。

ビルド方法：
Installerフォルダを開きます。
build_player.batを実行します。
成功すると、Installerフォルダ内に、player, runtime, debugMenuRuntimeの3つのフォルダが作成されます。
E:\workfolder\homework\agtk-branch\Installer
※libchipmunk.libを再ビルドする必要があれば、../Player/cocos2d/external/win32-specific/chipmunk/libchipmunk/readme_j.txtを参照してください。
※ビルドパス名には、日本語などのマルチバイト文字を含めないようにしてください。

差し替え方法：
Steam版のアクションゲームツクールMVのインストールフォルダを開きます。(PGMフォルダと呼びます。)

・テストプレイで使用するPlayerを差し替える
Installer/playerフォルダ配下を、PGM/player-winフォルダ内に上書きコピーします。

・「ゲームをビルド」で使用するPlayer（「デバッグ機能を有効化」がOFF）を差し替える。
PGM/resource/runtime.zipを展開します。(PGM/resource/runtimeフォルダが作成されます。）
Installer/runtimeフォルダ配下を、PGM/resource/runtimeフォルダ内に上書きコピーします。
PGM/resource/runtimeフォルダをZIP圧縮して、PGM/resource/runtime.zipに上書きコピーします。

・「ゲームをビルド」で使用するPlayer（「デバッグ機能を有効化」がON）を差し替える。
PGM/resource/debugMenuRuntime.zipを展開します。(PGM/resource/debugMenuRuntimeフォルダが作成されます。）
Installer/debugMenuRuntimeフォルダ配下を、PGM/resource/debugMenuRuntimeフォルダ内に上書きコピーします。
PGM/resource/debugMenuRuntimeフォルダをZIP圧縮して、PGM/resource/debugMenuRuntime.zipに上書きコピーします。

注意：
オープンソース版は、暗号化ビルドに対応していません。暗号化ビルドしたゲームは起動しません。

以上
