#ifndef PLAYER_CLASSES_MANAGER_OUTPUTMANAGER_H
#define PLAYER_CLASSES_MANAGER_OUTPUTMANAGER_H

#include <cocos2d.h>
#include <Lib/Macros.h>

// 出力マネージャー
class AGTKPLAYER_API OutputManager :
	public cocos2d::Ref
{
private:
	OutputManager();
	static OutputManager* output_manager_;
public:
	virtual ~OutputManager();
	static OutputManager* getInstance();
	static void purge();
public:
	// 振動ファイルを再生する
	//	[file_index]		0から始まる、再生する振動ファイルの番号
	//  [controller_id]		振動する対象となるコントローラーID(-1...全てのコントローラー)
	void playVibrationFile(int file_index, int controller_id = -1);
};

#endif // PLAYER_CLASSES_MANAGER_OUTPUTMANAGER_H