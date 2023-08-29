#include "Portal.h"
#include "Lib/Collision.h"
#include "Lib/Scene.h"
#include "Manager/GameManager.h"
#include "Manager/InputManager.h"
#include "Manager/AudioManager.h"
#include "Manager/DebugManager.h"
#include "Manager/EffectManager.h"
#include "External/collision/CollisionComponent.hpp"
#include "External/collision/CollisionUtils.hpp"

NS_AGTK_BEGIN
//-------------------------------------------------------------------------------------------------------------------
// !ポータルタッチデータクラス
//-------------------------------------------------------------------------------------------------------------------
/**
* コンストラクタ
*/
PortalTouchedData::PortalTouchedData()
{
	this->_object = nullptr;
	this->_appearPosition = Vec2::ZERO;
	this->_visibleDelay300 = 0;
	this->_needObjectMoveEffect = true;
#ifdef USE_ACT2_5389
#else
	this->_childrenObjectList = nullptr;
#endif
	this->_particleGroupBackupList = nullptr;
	this->_effectBackupList = nullptr;
}

/**
* デストラクタ
*/
PortalTouchedData::~PortalTouchedData()
{
	CC_SAFE_RELEASE(_object);
#ifdef USE_ACT2_5389
#else
	CC_SAFE_RELEASE(_childrenObjectList);
#endif
	CC_SAFE_RELEASE(_particleGroupBackupList);
	CC_SAFE_RELEASE(_effectBackupList);
}

/**
* 初期化
* @param	object			ポータルに触れたオブジェクト
* @param	appearPosition	出現させる位置
* @param	sceneLayerId	出現させるシーンレイヤーID
* @param	visibleDlay300	出現させるまでのディレイ(300 = 1.0f)秒
* @return					初期化の成否
*/
bool PortalTouchedData::init(agtk::Object * object, cocos2d::Vec2 appearPosition, int sceneLayerId, int visibleDlay300)
{
	this->setObject(object);
	this->setAppearPosition(appearPosition);
	this->setsceneLayerId(sceneLayerId);
	this->setVisibleDelay300(visibleDlay300);
	//子オブジェクト
#ifdef USE_ACT2_5389
#else
	auto childrenObjectList = cocos2d::__Array::create();
	this->setChildrenObjectList(childrenObjectList);
	if (object->getChildrenObjectList()->count() > 0) {
		childrenObjectList->addObjectsFromArray(object->getChildrenObjectList());
	}
#endif
	auto particleManager = ParticleManager::getInstance();
	this->setParticleGroupBackupList(particleManager->getParticleGroupBackupList(object));
	auto effectManager = EffectManager::getInstance();
	this->setEffectBackupList(effectManager->getEffectBackupList(object));
	return true;
}

cocos2d::__Array *PortalTouchedData::getParticleGroupBackupList()
{
	return _particleGroupBackupList;
}

cocos2d::__Array *PortalTouchedData::getEffectBackupList()
{
	return _effectBackupList;
}

//-------------------------------------------------------------------------------------------------------------------
// !ポータル
//-------------------------------------------------------------------------------------------------------------------
/**
* コンストラクタ
*/
Portal::Portal()
{
	this->_moveSettingData = nullptr;
	this->_touchedObjectIdList = nullptr;

	this->_elapsedTime = 0;
	this->_startCountElapsedTime = false;
	this->_needMovePortal = false;

	this->_rePortalDirectionBit = 0;
}

/**
* デストラクタ
*/
Portal::~Portal()
{
	CC_SAFE_RELEASE_NULL(_touchedObjectIdList);
	CC_SAFE_RELEASE_NULL(_moveSettingData);
}

/**
* 初期化
* @param	id					ポータルID
* @param	idx					ポータルIDX
* @param	type				ポータルタイプ(0:A, 1:B)
* @param	areaSettingDataList	エリア設定データリスト
* @param	movableList			移動可否リスト
* @param	moveSettingData		エリア移動データ
*/
bool Portal::init(int id, int idx, int type, cocos2d::__Array * areaSettingDataList, cocos2d::__Array * movableList, agtk::data::MoveSettingData * moveSettingData, agtk::data::SceneData * sceneData)
{
	_id = id;
	// ポータルタイプ保持
	this->setPortalType(type);

	// 移動可否フラグ設定
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto value = static_cast<cocos2d::Bool *>(movableList->getObjectAtIndex(type))->getValue();
#else
	auto value = dynamic_cast<cocos2d::Bool *>(movableList->getObjectAtIndex(type))->getValue();
#endif
	this->setIsMovable(value);

	// 移動元エリア設定データ取得
	auto p1 = areaSettingDataList->getObjectAtIndex(type);
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto fromAreaSettingData = static_cast<agtk::data::AreaSettingData *>(p1);
#else
	auto fromAreaSettingData = dynamic_cast<agtk::data::AreaSettingData *>(p1);
#endif

	Size fromPortalSize = Size(fromAreaSettingData->getWidth(), fromAreaSettingData->getHeight());
	Vec2 fromProtalPos = Vec2(fromAreaSettingData->getX(), fromAreaSettingData->getY());
	fromProtalPos = Scene::getPositionCocos2dFromScene(fromProtalPos, sceneData);
	fromProtalPos.y -= fromAreaSettingData->getHeight();
	this->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
	this->setPosition(fromProtalPos);
	this->setContentSize(fromPortalSize);

	// シーンレイヤーID保持
	this->setSceneLayerId(fromAreaSettingData->getLayerIndex() + 1);

	// 出現位置合わせフラグ保持
	this->setKeepHorzPosition(fromAreaSettingData->getKeepHorzPosition());
	this->setKeepVertPosition(fromAreaSettingData->getKeepVertPosition());

	// ポータルのユニーク名を設定
	this->setName(StringUtils::format("potal_%d_%d_%d", fromAreaSettingData->getSceneId(), fromAreaSettingData->getLayerIndex() + 1, idx));

	// 再移動用方向ビット値設定
	this->setRePortalDirectionBit(fromAreaSettingData->getRePortalDirectionBit());

	// 移動先のポータルタイプを算出
	int moveTargetPortalIdx = (type + 1) % agtk::data::TransitionPortalData::EnumPortalType::MAX;

	// 移動先エリア設定データ取得
	auto p2 = areaSettingDataList->getObjectAtIndex(moveTargetPortalIdx);
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto toAreaSettingData = static_cast<agtk::data::AreaSettingData *>(p2);
#else
	auto toAreaSettingData = dynamic_cast<agtk::data::AreaSettingData *>(p2);
#endif

	// 移動先のシーンID設定
	this->setMoveToSceneId(toAreaSettingData->getSceneId());
	//ACT2-4790 シーンレイヤーが設定なしの場合は、移動先シーンIDも設定なしにする。
	if (toAreaSettingData->getLayerIndex() < 0) {
		this->setMoveToSceneId(-1);
	}

	// 移動先シーンと移動元シーンは同一か？
	this->setIsSameScene(this->getMoveToSceneId() == fromAreaSettingData->getSceneId());

	// 移動先のシーンレイヤーID設定
	this->setMoveToSceneLayerId(toAreaSettingData->getLayerIndex() + 1);

	// 移動先ポータルの位置とサイズ取得
	Size toProtalSize = Size(toAreaSettingData->getWidth(), toAreaSettingData->getHeight());
	Vec2 toPortalPos = Vec2(toAreaSettingData->getX(), toAreaSettingData->getY());
	this->setMoveToPortalPosition(toPortalPos);
	this->setMoveToPortalSize(toProtalSize);

	// 移動設定データ保持
	this->setMoveSettingData(moveSettingData);

	cocos2d::__Array *arr = cocos2d::__Array::create();
	this->setTouchedObjectIdList(arr);

	bool isDebugShow = DebugManager::getInstance()->getShowPortalFlag();
	this->showDebugVisible(isDebugShow);

	return true;
}

/**
* 更新
* @param	dt	前フレームからの経過時間
*/
void Portal::update(float dt)
{
	_lastTouchObjectInstanceIdList = _touchObjectInstanceIdList;
	_touchObjectInstanceIdList.clear();

	// 経過時間計測開始がONの場合
	if (_startCountElapsedTime) {
		_elapsedTime += dt;
		
		// 移動開始時間を超える場合
		if (_elapsedTime * 300 >= _moveSettingData->getPreMoveConditionDuration300()) {
			_needMovePortal = true;
		}
	}

	// ポータル移動が要求されている場合
	if (_needMovePortal) {

		auto gameManager = GameManager::getInstance();

		// 移動先シーンが別シーンの場合
		if (!_isSameScene) {
			// 遷移先シーンを設定
			gameManager->setNextSceneId(_moveToSceneId);
		}
		else {
			//同じシーンの場合
			auto currentScene = gameManager->getCurrentScene();
			if (currentScene != nullptr) {
				//次シーンID=0で、現在シーンIDと移動先シーンIDが同じ場合。
				if (gameManager->getNextSceneId() == 0 && currentScene->getSceneData()->getId() == _moveToSceneId) {
					gameManager->setNextSceneId(_moveToSceneId);
				}
			}
		}

		// ポータル移動フラグをON
		gameManager->setIsPortalMoving(true);

		// シーン変更要求フラグ設定
		gameManager->setNeedSceneChange(!_isSameScene);

		// 遷移用データを設定
		gameManager->setPortalData(_moveSettingData);

		// 触れたプレイヤーの順に表示するフラグがOFFの場合
		if (!_moveSettingData->getPostMoveShowPlayersInTouchedOrder()) {

			// ワーク変数
			auto playerMax = gameManager->getProjectData()->getPlayerCount();
			auto newList = cocos2d::__Array::create();

			// プレイヤー1から順に表示するよう _touchedObjectIdList を入れ替え
			for (int playerId = 1; playerId <= playerMax; playerId++) {

				cocos2d::Ref * ref = nullptr;
				CCARRAY_FOREACH(_touchedObjectIdList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto data = static_cast<PortalTouchedData *>(ref);
#else
					auto data = dynamic_cast<PortalTouchedData *>(ref);
#endif
					auto obj = data->getObject();

					if (playerId == obj->getPlayerId()) {
						newList->addObject(data);
						break;
					}
				}
			}

			// 入れ替え
			if (newList->count() > 0) {
				_touchedObjectIdList->removeAllObjects();
				this->setTouchedObjectIdList(newList);
			}
		}

		// ポータルに触れたプレイヤーリストを設定
		gameManager->setPortalTouchedPlayerList(_touchedObjectIdList);

		// プレイヤーの接続オブジェクトを設定。
		if(_touchedObjectIdList->count() > 0) {
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(_touchedObjectIdList, ref) {
				auto portal = dynamic_cast<PortalTouchedData *>(ref);
				auto obj = portal->getObject();
				auto connectObjectList = obj->getConnectObjectList();
				cocos2d::Ref *ref2;
				std::vector<agtk::Object::ConnectObjectLoadList> list;
				CCARRAY_FOREACH(connectObjectList, ref2) {
					auto connectObject = dynamic_cast<agtk::ConnectObject *>(ref2);
					int instanceId = connectObject->getInstanceId();
					int settingId = connectObject->getObjectConnectSettingData()->getId();
					agtk::Object::ConnectObjectLoadList data;
					data.instanceId = instanceId;
					data.settingId = settingId;
					data.actionId = connectObject->getCurrentObjectAction()->getId();
					list.push_back(data);
				}
				obj->setConnectObjectPortalLoadList(list);
			}
		}
#ifdef FIX_ACT2_4774
		gameManager->setTransitionPortalId(_id);
#endif

		// 遷移開始を要求
		gameManager->setNeedPortalMove(true);

		// ポータル移動演出がスライド連結系の場合
		if (_moveSettingData->getPreMoveEffect() >= agtk::data::EnumMoveEffect::kMoveEffectSlideUpLink) {
			// ポータル移動の為に一旦オブジェクトを非表示
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(_touchedObjectIdList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto p = static_cast<PortalTouchedData *>(ref);
#else
				auto p = dynamic_cast<PortalTouchedData *>(ref);
#endif
				auto player = p->getObject()->getPlayer();
				if (player) {
					player->setVisible(false);
				}
				p->setsrcObjectPos(agtk::Scene::getPositionCocos2dFromScene(p->getObject()->getPosition()));
			}
		}

		reset();
	}
}

/**
* 移動開始条件の「スイッチ、変数が変化」チェック
* @param	condisionList	チェックする条件リスト
* @return					条件を満たしたか？
*/
bool Portal::checkSwichAndVariablesCondision(cocos2d::__Array *condisionList)
{
	auto gameManager = GameManager::getInstance();
	bool isSwitchVariableChanged = true;

	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(condisionList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto conditionData = static_cast<agtk::data::SwitchVariableConditionData *>(ref);
#else
		auto conditionData = dynamic_cast<agtk::data::SwitchVariableConditionData *>(ref);
#endif
		// スイッチが変化の場合
		if (conditionData->getSwtch()) {
			// スイッチデータリスト取得
			cocos2d::__Array *switchDataList = cocos2d::__Array::create();
			gameManager->getSwitchVariableDataList(conditionData->getSwitchQualifierId(), conditionData->getSwitchObjectId(), conditionData->getSwitchId(), true, switchDataList);

			// スイッチデータリストを回す
			cocos2d::Ref *ref4 = nullptr;
			bool ret = false;
			CCARRAY_FOREACH(switchDataList, ref4) {
				// スイッチのON/OFFの状態による条件チェック
				auto switchData = dynamic_cast<agtk::data::PlaySwitchData *>(ref4);
				ret |= (nullptr != switchData && gameManager->checkSwitchCondition(conditionData->getSwitchValue(), switchData->getValue(), switchData->isState()));
			}

			isSwitchVariableChanged &= ret;
		}
		// 変数が変化の場合
		else {
			// 変数データリスト取得
			cocos2d::__Array *variableDataList = cocos2d::__Array::create();
			gameManager->getSwitchVariableDataList(conditionData->getVariableQualifierId(), conditionData->getVariableObjectId(), conditionData->getVariableId(), false, variableDataList);

			// 変数データリストを回す
			cocos2d::Ref *ref4 = nullptr;
			bool ret = false;
			CCARRAY_FOREACH(variableDataList, ref4) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto variableData = static_cast<agtk::data::PlayVariableData *>(ref4);
#else
				auto variableData = dynamic_cast<agtk::data::PlayVariableData *>(ref4);
#endif
				double srcValue = variableData->getValue();
				double compareValue = 0;

				ret |= gameManager->checkVariableCondition(
					conditionData->getCompareValueType(),
					srcValue,
					conditionData->getCompareOperator(),
					conditionData->getComparedValue(),
					conditionData->getComparedVariableQualifierId(),
					conditionData->getComparedVariableObjectId(),
					conditionData->getComparedVariableId());
			}

			isSwitchVariableChanged &= ret;
		}
	}

	return isSwitchVariableChanged;
}

/**
* 動作用メンバのリセット
*/
void Portal::reset()
{
	_elapsedTime = 0;
	_startCountElapsedTime = false;
	_needMovePortal = false;
}

/**
* ポータルに触れたオブジェクト追加
* @param	object	ポータルに触れたオブジェクト
*/
bool Portal::addTouchObject(agtk::Object *object)
{
	// このポータルから移動は不可 or レイヤーが異なる or プレイヤータイプでない or 操作オブジェクトでない場合
	if (!_isMovable || object->getLayerId() != _sceneLayerId || !object->getObjectData()->isGroupPlayer() || !object->getObjectData()->getOperatable()) {
		return false;
	}

	// 子オブジェクトの場合。
	if (object->getOwnParentObject()) {
		return false;
	}

	// 既に登録済みかチェック
	auto instanceId = object->getInstanceId();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(_touchedObjectIdList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<PortalTouchedData *>(ref);
#else
		auto p = dynamic_cast<PortalTouchedData *>(ref);
#endif
		if (p->getObject()->getInstanceId() == instanceId) {
			return false;
		}
	}

	// ワーク変数
	auto gameManager = GameManager::getInstance();
	auto sceneLayer = object->getSceneLayer();
	auto scene = sceneLayer->getScene();

	// -------------------------------------------------
	// ▼プレイヤーが触れた判定の条件チェック
	// -------------------------------------------------
	// 入力操作がされたがON　かつ　キー入力がなされていない場合
	if (_moveSettingData->getPreMoveKeyInput() && !InputManager::getInstance()->isPressed(_moveSettingData->getPreMoveOperationKeyId())) {
		return false;
	}

	//スイッチ、変数が変化がONの場合
	if (_moveSettingData->getPreMoveTouchedSwitchVariableCondition()) {
		
		// チェック
		bool isSwitchVariableChanged = checkSwichAndVariablesCondision(_moveSettingData->getPreMoveTouchedSwitchVariableConditionList());
		
		// スイッチ、変数の変化チェックが全て満たされなかった場合
		if (!isSwitchVariableChanged) {
			return false;
		}
	}

	// ポータルに触れたデータを生成
	createTouchedData(object, object->getPosition());

	// -------------------------------------------------
	// ▼プレイヤーが触れた時の設定
	// -------------------------------------------------
	// スイッチ、変数を変更がONで、かつ直前に触れていなかった場合
	if (_moveSettingData->getPreMoveChangeSwitchVariable() && std::find(_lastTouchObjectInstanceIdList.begin(), _lastTouchObjectInstanceIdList.end(), instanceId) == _lastTouchObjectInstanceIdList.end()) {
		// スイッチ、変数を変更
		gameManager->calcSwichVariableChange(_moveSettingData->getPreMoveSwitchVariableAssignList(), GameManager::kPlacePortal, getId(), getPortalType(), 0);
		// 変数・スイッチ変更時のオブジェクトに対して変更処理。
		GameManager::getInstance()->updateObjectVariableAndSwitch();
	}
	_touchObjectInstanceIdList.push_back(instanceId);

	// -------------------------------------------------
	// ▼移動開始条件チェック(触れた系)
	// -------------------------------------------------
	// シーンに存在するプレイヤー総数を取得
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	auto objectList = gameManager->getCurrentScene()->getObjectAllReference(sceneLayer->getType());
#else
	auto objectList = gameManager->getCurrentScene()->getObjectAll(sceneLayer->getType());
#endif
	int playerNum = 0;
	CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto obj = static_cast<agtk::Object *>(ref);
#else
		auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
		auto objData = obj->getObjectData();
		if (objData->isGroupPlayer() && objData->getOperatable() && !obj->getOwnParentObject()) {
			playerNum++;
		}
	}

	switch (_moveSettingData->getPreMoveConditionType())
	{
		// --------------------------------------------------------
		// プレイヤーが一人でも触れた
		// --------------------------------------------------------
	case agtk::data::MoveSettingData::kConditionTypeAnyPlayerTouched:
		_needMovePortal |= (_touchedObjectIdList->count() > 0);
		if (_needMovePortal && !_moveSettingData->getPostMoveOnlyTransportPlayersTouching()) {
			// 残りの触れていないプレイヤーを強制的に触れた扱いにする
			// ※遷移先座標は触れたプレイヤーと同一になる
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto obj = static_cast<agtk::Object *>(ref);
#else
				auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
				auto objData = obj->getObjectData();
				// プレイヤータイプ かつ 操作可能 かつ 登録済みのオブジェクトでない かつ 子オブジェクトではない
				if (objData->isGroupPlayer() && objData->getOperatable() && obj->getInstanceId() != object->getInstanceId() && !obj->getOwnParentObject()) {
					createTouchedData(obj, object->getPosition());
				}
			}
		}
		break;
		// --------------------------------------------------------
		// 全てのプレイヤーが触れた
		// --------------------------------------------------------
	case agtk::data::MoveSettingData::kConditionTypeAllPlayersTouched:
		_needMovePortal |= (_touchedObjectIdList->count() == playerNum);
		break;
		// --------------------------------------------------------
		// プレイヤーが一人でも触れて一定時間が経過した
		// --------------------------------------------------------
	case agtk::data::MoveSettingData::kConditionTypeAnyPlayerTouchedAndDurationPassed:
		_startCountElapsedTime = (_touchedObjectIdList->count() > 0);
		break;
		// --------------------------------------------------------
		// 全てのプレイヤーが触れて一定時間が経過した
		// --------------------------------------------------------
	case agtk::data::MoveSettingData::kConditionTypeAllPlayersTouchedAndDurationPassed:
		_startCountElapsedTime = (_touchedObjectIdList->count() == playerNum);
		break;
	}

	// 移動要請が ON または 時間経過カウント開始が ON の場合
	if (_needMovePortal || _startCountElapsedTime) {
		// スイッチ、変数が変化がONの場合
		if (_moveSettingData->getPreMoveSwitchVariableCondition()) {

			// スイッチ、変数が変化していない場合
			if (!checkSwichAndVariablesCondision(_moveSettingData->getPreMoveSwitchVariableConditionList())) {

				// 要請フラグをOFF
				_needMovePortal = false;
				_startCountElapsedTime = false;

				// 触れたオブジェクトリストを初期化
				_touchedObjectIdList->removeAllObjects();

				return false;
			}
		}
	}
	return true;
}

/**
* ポータルに触れたデータの生成
* @param	object			触れたオブジェクト
* @param	objectPosition	触れたオブジェクトの座標
*/
void Portal::createTouchedData(agtk::Object *object, cocos2d::Vec2 objectPosition)
{
	// 触れたオブジェクトとポータルとの位置の割合を算出
	// ※デフォルトは中心
	// ※Y軸の算出はcocos2dの座標軸とSceneの座標軸が違うため注意
	cocos2d::Vec2 rate = Vec2(0.5f, 0.5f);
	cocos2d::Vec2 myPos = Scene::getPositionSceneFromCocos2d(getPosition());
	myPos.y -= getContentSize().height;

	// X方向で位置を合わせる場合
	if (_keepHorzPosition) {
		rate.x = (objectPosition.x - myPos.x) / getContentSize().width;
	}
	//Y方向で位置を合わせる場合
	if (_keepVertPosition) {
		rate.y = (objectPosition.y - myPos.y) / getContentSize().height;
	}

	// 出現位置算出
	auto pos = Vec2(_moveToPortalPosition.x + _moveToPortalSize.width * rate.x, _moveToPortalPosition.y + _moveToPortalSize.height * rate.y);
	// ACT2-5126 再移動有効な横幅が違う場合に、計算結果に細かい少数点が入り
	// Object::getPortalMoveDispBit()での処理で移動していると判定されてしまうので
	// 小数点を切り捨てる
	pos = Vec2((int)pos.x, (int)pos.y);

	// 出現までの時間を設定
	int visibleDelay300 = 0;

	// 表示間隔設定がある場合
	if (_moveSettingData->getPostMoveShowPlayersInTouchedOrder()) {
		visibleDelay300 = _moveSettingData->getPostMoveShowDuration300();
	}

	// 触れたオブジェクトを登録
	auto touchedData = PortalTouchedData::create(object, pos, _moveToSceneLayerId, visibleDelay300);

	// 移動元ポータルの中心座標(cocos2座標)を設定
	touchedData->setSrcPortalCenterPos(this->getPosition() + this->getContentSize() * 0.5f);

	// 移動先ポータルの中心座標(scene座標)を設定 ※座標変換を行わないのはシーンが異なる可能性がある為
	touchedData->setDstPortalCenterPos(_moveToPortalPosition + _moveToPortalSize * 0.5f);

	// 触れたプレイヤーを無効にする場合
	if (_moveSettingData->getPreMoveInvalidateTouchedPlayer()) {

		// プレイヤーを無効化
		object->setDisabled(true);

		// 表示も消す場合
		if (!_moveSettingData->getPreMoveKeepDisplay()) {
			// 表示をOFF
			object->setVisible(false);

			// オブジェクト移動演出OFF(表示されない為)
			touchedData->setNeedObjectMoveEffect(false);
		}
	}

	_touchedObjectIdList->addObject(touchedData);
}

/**
* デバッグ表示
* @param	isShow	表示のON/OFF
*/
void Portal::showDebugVisible(bool isShow)
{
#ifdef USE_PREVIEW
	auto debugView = this->getChildByName("debugVisible");

	if (!debugView && isShow) {
		Rect r = Rect(0, 0, getContentSize().width, getContentSize().height);
		auto s = Sprite::create();
		s->setAnchorPoint(Vec2::ANCHOR_BOTTOM_LEFT);
		s->setTextureRect(r);
		s->setColor(Color3B::RED);
		s->setOpacity(100);
		this->addChild(s, 1, "debugVisible");
	}
	else if (debugView) {
		debugView->setVisible(isShow);
	}
#endif // USE_PREVIEW
}

NS_AGTK_END
