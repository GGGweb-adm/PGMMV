#include "GuiManager.h"
#include "Lib/Scene.h"

USING_NS_CC;
USING_NS_AGTK;

//--------------------------------------------------------------------------------------------------------------------
// シングルトンのインスタンス
//--------------------------------------------------------------------------------------------------------------------
GuiManager* GuiManager::_guiManager = NULL;

//--------------------------------------------------------------------------------------------------------------------
// コンストラクタ
//--------------------------------------------------------------------------------------------------------------------
GuiManager::GuiManager()
{
	_objectStop = false;
	_gameStop = false;

	_guiList = nullptr;
}

//--------------------------------------------------------------------------------------------------------------------
// デストラクタ
//--------------------------------------------------------------------------------------------------------------------
GuiManager::~GuiManager()
{
	CC_SAFE_RELEASE_NULL(_guiList);
}

//--------------------------------------------------------------------------------------------------------------------
// インスタンスを取得する。
//--------------------------------------------------------------------------------------------------------------------
GuiManager* GuiManager::getInstance()
{
	if (!_guiManager) {
		_guiManager = new GuiManager();
		_guiManager->init();
	}

	return _guiManager;
}

//--------------------------------------------------------------------------------------------------------------------
// シングルトンを破棄する。
//--------------------------------------------------------------------------------------------------------------------
void GuiManager::purge()
{
	if (!_guiManager) {
		return;
	}
	GuiManager *m = _guiManager;
	_guiManager = NULL;
	m->release();
}

//--------------------------------------------------------------------------------------------------------------------
// 初期化
//--------------------------------------------------------------------------------------------------------------------
bool GuiManager::init()
{
	setGuiList(cocos2d::Array::create());

	return true;
}

//--------------------------------------------------------------------------------------------------------------------
// GUIの更新
// @param	delta	前フレームからの経過時間
//--------------------------------------------------------------------------------------------------------------------
void GuiManager::update(float delta)
{
	// オブジェクト停止フラグを初期化
	_objectStop = false;
	// ゲーム停止フラグを初期化
	_gameStop = false;

	if (_guiList == nullptr || _guiList->count() <= 0) {
		return;
	}

	//オブジェクトが持っているゲージUI数を取得する（生成順に優先順位をとるために必要）
	std::map<agtk::Object *, int> prioGaugeUi;
	for (int i = 0; i < _guiList->count(); i++) {
		auto gui = dynamic_cast<Gui *>(_guiList->getObjectAtIndex(i));
		if (gui != nullptr) {
			if (gui->isDelete()) continue;
			auto object = gui->getTargetObject();
			auto gaugeUi = dynamic_cast<agtk::ObjectParameterGaugeUi *>(gui);
			if (object != nullptr && gaugeUi != nullptr) {
				if (prioGaugeUi[object] == 0) {
					prioGaugeUi[object] = 1;
				} else {
					prioGaugeUi[object] = prioGaugeUi[object] + 1;
				}
			}
		}
	}

	for (int i = _guiList->count() - 1; i >= 0; i--) {
		auto gui = dynamic_cast<Gui *>(_guiList->getObjectAtIndex(i));

		if (gui) {
			if (gui->isDelete()) {
				gui->remove();
				_guiList->removeObjectAtIndex(i);
			}
			else {
				auto object = gui->getTargetObject();
				auto gaugeUi = dynamic_cast<agtk::ObjectParameterGaugeUi *>(gui);
				if (object != nullptr && gaugeUi != nullptr) {
					gaugeUi->setAddOrder(prioGaugeUi[object]);
					prioGaugeUi[object] = prioGaugeUi[object] - 1;
				}
				gui->update(delta);

				// オブジェクト停止フラグの更新
				_objectStop |= gui->getObjectStop();
				// ゲーム停止フラグの更新
				_gameStop |= gui->getGameStop();
			}
		}
		else {
			_guiList->removeObjectAtIndex(i);
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------
// オブジェクトに追従するUIを追加する
//--------------------------------------------------------------------------------------------------------------------
void GuiManager::addObjectParameterGui(agtk::Object *object)
{
	cocos2d::DictElement *dl = nullptr;
	auto displayList = object->getObjectData()->getAdditionalDisplayList();

	CCDICT_FOREACH(displayList, dl) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto displayData = static_cast<agtk::data::ObjectAdditionalDisplayData *>(dl->getObject());
#else
		auto displayData = dynamic_cast<agtk::data::ObjectAdditionalDisplayData *>(dl->getObject());
#endif
		Gui *ui = nullptr;

		// 「非表示」にチェックがついている場合、処理しない
		if (displayData->getHide()) {
			continue;
		}

		// テキストを表示する場合
		if (displayData->getShowText()) {
			ui = agtk::ObjectParameterTextUi::create(object, displayData);
		}
		// パラメータを表示する場合
		else {
			// テキスト表示の場合
			if (displayData->getParamDisplayType() == agtk::data::ObjectAdditionalDisplayData::EnumParamDisplayType::kParamDisplayText) {
				ui = agtk::ObjectParameterTextUi::create(object, displayData);
			}
			// ゲージ形式表示の場合
			else {
				ui = agtk::ObjectParameterGaugeUi::create(object, displayData);
			}
		}

		if (ui) {
			_guiList->addObject(ui);
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------
// 実行アクションからメッセージUIを追加する
// @param	object	アクションを発行したオブジェクト
// @param	data	表示するメッセージのデータ
//--------------------------------------------------------------------------------------------------------------------
void GuiManager::addActionCommandMessageGui(agtk::Object* object, agtk::data::ObjectCommandMessageShowData* data)
{
	addActionCommandMessageGui(object, nullptr, data);
}

//--------------------------------------------------------------------------------------------------------------------
// 実行アクションからメッセージUIを追加する
// @param	object		アクションを発行したオブジェクト
// @param	lockObject	ロック対象オブジェクト
// @param	data		表示するメッセージのデータ
//--------------------------------------------------------------------------------------------------------------------
void GuiManager::addActionCommandMessageGui(agtk::Object* object, agtk::Object* lockObject, agtk::data::ObjectCommandMessageShowData* data)
{
	// UIを生成
	Gui *ui = nullptr;
	ui = agtk::ActionCommandMessageTextUi::create(object, lockObject, data);

	if (ui) {
		_guiList->addObject(ui);
	}
}

//--------------------------------------------------------------------------------------------------------------------
// 実行アクションからスクロールするメッセージUIを追加する
// @param	object	アクションを発行したオブジェクト
// @param	data	表示するメッセージのデータ
//--------------------------------------------------------------------------------------------------------------------
void GuiManager::addActionCommandScrollMessageGui(agtk::Object* object, agtk::data::ObjectCommandScrollMessageShowData* data)
{
	addActionCommandScrollMessageGui(object, nullptr, data);
}

//--------------------------------------------------------------------------------------------------------------------
// 実行アクションからスクロールするメッセージUIを追加する
// @param	object	アクションを発行したオブジェクト
// @param	lockObject	ロック対象オブジェクト
// @param	data	表示するメッセージのデータ
//--------------------------------------------------------------------------------------------------------------------
void GuiManager::addActionCommandScrollMessageGui(agtk::Object *object, agtk::Object *lockObject, agtk::data::ObjectCommandScrollMessageShowData * data)
{
	// UIを生成
	Gui *ui = nullptr;
	ui = agtk::ActionCommandScrollMessageTextUi::create(object, lockObject, data);

	if (ui) {
		_guiList->addObject(ui);
	}
}

//--------------------------------------------------------------------------------------------------------------------
// 指定オブジェクトに追従しているUIを削除する
// @param	object	対象のオブジェクト
//--------------------------------------------------------------------------------------------------------------------
void GuiManager::removeGui(agtk::Object *object)
{
	for (int i = _guiList->count() - 1; i >= 0; i--) {
		auto gui = dynamic_cast<Gui *>(_guiList->getObjectAtIndex(i));

		if (gui && object) {
			if (gui->checkRemove(object)) {
				gui->remove();
				_guiList->removeObjectAtIndex(i);
			}
		}
		else {
			_guiList->removeObjectAtIndex(i);
		}
	}
}

//--------------------------------------------------------------------------------------------------------------------
// 全てのUIを削除する
//--------------------------------------------------------------------------------------------------------------------
void GuiManager::clearGui(agtk::Scene *scene)
{
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	auto objectList = scene->getObjectAllReference();
#else
	auto objectList = scene->getObjectAll();
#endif
	for (int i = _guiList->count() - 1; i >= 0; i--) {
		auto gui = dynamic_cast<Gui *>(_guiList->getObjectAtIndex(i));
		if (gui) {
			if (objectList->containsObject(gui->getTargetObject())) {
				gui->remove();
			}
		}
	}

	_guiList->removeAllObjects();
}

//--------------------------------------------------------------------------------------------------------------------
// 指定オブジェクトの表示有無を設定する。
//--------------------------------------------------------------------------------------------------------------------
void GuiManager::setVisibleGui(bool visible, agtk::Object *object)
{
	for (int i = _guiList->count() - 1; i >= 0; i--) {
		auto gui = dynamic_cast<Gui *>(_guiList->getObjectAtIndex(i));
		if (gui != nullptr && gui->getTargetObject() == object) {
			gui->setVisible(visible);
		}
	}
}

int GuiManager::getGuiList(cocos2d::__Array *guiList, agtk::Object *object)
{
	if (guiList == nullptr) {
		return 0;
	}
	for (int i = 0; i < _guiList->count(); i++) {
		auto gui = dynamic_cast<Gui *>(_guiList->getObjectAtIndex(i));
		if (gui != nullptr && gui->getTargetObject() == object) {
			guiList->addObject(gui);
		}
	}
	return guiList->count();
}

// 指定オブジェクトが実行アクション「テキストを表示」を追加したかチェックする。
bool GuiManager::isActionCommandMessageGui(agtk::Object *object)
{
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(_guiList, ref) {
		auto gui = dynamic_cast<ActionCommandMessageTextUi *>(ref);
		if (gui == nullptr) continue;
		if (gui->getTargetObject() == object) {
			return true;
		}
	}
	return false;
}

// 指定オブジェクトが実行アクション「テキストをスクロール表示」を追加したかチェック。
bool GuiManager::isActionCommandScrollMessageGui(agtk::Object *object)
{
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(_guiList, ref) {
		auto gui = dynamic_cast<ActionCommandScrollMessageTextUi *>(ref);
		if (gui == nullptr) continue;
		if (gui->getTargetObject() == object) {
			return true;
		}
	}
	return false;
}

// 指定オブジェクトの「テキストを表示」リソースを取得。
bool GuiManager::getActionCommandMessageGui(agtk::Object *object, cocos2d::__Array *guiList)
{
	if (guiList != nullptr && guiList->count() > 0) {
		guiList->removeAllObjects();
	}
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(_guiList, ref) {
		auto gui = dynamic_cast<ActionCommandMessageTextUi *>(ref);
		if (gui == nullptr) continue;
		if (gui->getTargetObject() == object) {
			if (guiList != nullptr) {
				guiList->addObject(gui);
			}
		}
	}
	return (guiList && guiList->count() > 0) ? true : false;
}

// 指定オブジェクトの「テキストをスクロール表示」リソースを取得。
bool GuiManager::getActionCommandScrollMessageGui(agtk::Object *object, cocos2d::__Array *guiList)
{
	cocos2d::Ref *ref;
	if (guiList != nullptr && guiList->count() > 0) {
		guiList->removeAllObjects();
	}
	CCARRAY_FOREACH(_guiList, ref) {
		auto gui = dynamic_cast<ActionCommandScrollMessageTextUi *>(ref);
		if (gui == nullptr)continue;
		if (gui->getTargetObject() == object) {
			if (guiList != nullptr) {
				guiList->addObject(gui);
			}
		}
	}
	return (guiList && guiList->count() > 0) ? true : false;
}
