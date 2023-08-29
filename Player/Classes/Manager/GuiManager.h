#ifndef __GUI_MANAGER_H__
#define __GUI_MANAGER_H__

#include "cocos2d.h"
#include "Lib/Macros.h"
#include "Lib/Gui.h"

USING_NS_CC;

class AGTKPLAYER_API GuiManager : public cocos2d::Ref
{
private:
	GuiManager();
	static GuiManager *_guiManager;

public:
	virtual ~GuiManager();
	static GuiManager* getInstance();
	static void purge();
	bool init();

	void update(float delta);

	// オブジェクトに追従するUIを追加する
	void addObjectParameterGui(agtk::Object *object); 
	// 実行アクションからメッセージUIを追加する
	void addActionCommandMessageGui(agtk::Object *object, agtk::data::ObjectCommandMessageShowData* data);  
	// 実行アクションからメッセージUIを追加する（ロック対象指定あり）
	void addActionCommandMessageGui(agtk::Object *object, agtk::Object* lockObject, agtk::data::ObjectCommandMessageShowData* data);	
	// 実行アクションからスクロールするメッセージUIを追加する
	void addActionCommandScrollMessageGui(agtk::Object *object, agtk::data::ObjectCommandScrollMessageShowData* data);	
	// 実行アクションからスクロールするメッセージUIを追加する（ロック対象指定あり）
	void addActionCommandScrollMessageGui(agtk::Object *object, agtk::Object* lockObject, agtk::data::ObjectCommandScrollMessageShowData* data);

	// 指定オブジェクトに追従しているUIを削除する
	void removeGui(agtk::Object *object); 
	// 全てのUIを削除
	void clearGui(agtk::Scene *scene); 
	// 指定オブジェクトの表示有無を設定する。
	void setVisibleGui(bool visible, agtk::Object *object);
	// 指定オブジェクトのGUIを取得する。
	int getGuiList(cocos2d::__Array *, agtk::Object *object);

public:
	// 指定オブジェクトが実行アクション「テキストを表示」を追加したかチェックする。
	bool isActionCommandMessageGui(agtk::Object *object);
	// 指定オブジェクトが実行アクション「テキストをスクロール表示」を追加したかチェック。
	bool isActionCommandScrollMessageGui(agtk::Object *object);
	// 指定オブジェクトの「テキストを表示」リソースを取得。
	bool getActionCommandMessageGui(agtk::Object *object, cocos2d::__Array *guiList);
	// 指定オブジェクトの「テキストをスクロール表示」リソースを取得。
	bool getActionCommandScrollMessageGui(agtk::Object *object, cocos2d::__Array *guiList);

private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _guiList, GuiList);

	CC_SYNTHESIZE(bool, _objectStop, ObjectStop);	// メッセージ表示によるオブジェクト動作の停止が発生するか？
	CC_SYNTHESIZE(bool, _gameStop, GameStop);		// メッセージ表示によるゲーム動作停止が発生するか？
};

#endif //__GUI_MANAGER_H__
