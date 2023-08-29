#ifndef LIBCOCOS2D_COCOS_PLATFORM_CCVIBRATION_H
#define LIBCOCOS2D_COCOS_PLATFORM_CCVIBRATION_H

#include "base/CCRef.h"

NS_CC_BEGIN

// コントローラーの振動装置制御
class CC_DLL Vibration
{
public:
	// 振動ファイルを再生する
	//	[file_index]		0から始まる、再生する振動ファイルの番号
	//  [controller_id]		振動する対象となるコントローラーID
	static void playVibrationFile(int file_index, int controller_id);
};

NS_CC_END

#endif // LIBCOCOS2D_COCOS_PLATFORM_CCVIBRATION_H