/**
 * @brief オブジェクトコマンド
 */
#include "ObjectCommand.h"
#include "Data/ObjectActionLinkConditionData.h"
#include "Lib/Common.h"
#include "Lib/Collision.h"
#include "Lib/Object.h"
#include "Lib/Particle.h"
#include "Lib/Bullet.h"
#include "Lib/BaseLayer.h"
#include "Lib/Slope.h"
#include "Lib/Scene.h"
#include "Lib/PhysicsObject.h"
#include "Lib/ViewportLight.h"
#include "Manager/AudioManager.h"
#include "Manager/DebugManager.h"
#include "Manager/GameManager.h"
#include "Manager/ParticleManager.h"
#include "Manager/EffectManager.h"
#include "Manager/PrimitiveManager.h"
#include "Manager/DebugManager.h"
#include "Manager/BulletManager.h"
#include "Manager/GuiManager.h"
#include "Manager/ImageManager.h"
#include "Manager/MovieManager.h"
#include "External/collision/CollisionComponent.hpp"
#include "External/collision/CollisionUtils.hpp"
#include "JavascriptManager.h"
#include "scripting/js-bindings/manual/ScriptingCore.h"
#include <regex>

NS_AGTK_BEGIN
namespace
{
	//スコープ内でファイルセーブスイッチの変更を無効にする者
	class DisableChangingFileSaveSwitchInScope {
	public:
		DisableChangingFileSaveSwitchInScope(ObjectAction* oa) :_objAction(oa) {
			if (_objAction && _objAction->getDisableChangingFileSaveSwitchNextExecOtherAction()) 
			{
				_objAction->setDisableChangingFileSaveSwitchNextExecOtherAction(false);
				auto data = getSwitch();
				_prevReadOnly = data->getReadOnly();
				data->setReadOnly(true);
			}
			else
			{
				_objAction = nullptr;
			}
		}
		~DisableChangingFileSaveSwitchInScope() {
			if (_objAction)
			{
				getSwitch()->setReadOnly(_prevReadOnly);
			}
		}
	private:
		agtk::data::PlaySwitchData* getSwitch() 
		{
			return GameManager::getInstance()->getPlayData()->getCommonSwitchData(agtk::data::kProjectSystemSwitchSaveFile);
		}
	private:
		ObjectAction* _objAction;
		bool          _prevReadOnly;
	};
}

ObjectAction::ObjectAction()
{
	_object = nullptr;
	_objectData = nullptr;
	_objectActionData = nullptr;
	_objectActionLinkData = nullptr;
	_nextObjectActionLinkList = nullptr;
	_objCommandList = nullptr;
	_duration = 0.0f;
	_waitDuration300 = 0;
	_ignored = false;
	_isCommon = false;
	_commonActionSettingId = -1;
	_preActionId = -1;
	_hasHoldCommandList = false;
	_actionObjectLockOnceMore = false;
	_disableChangingFileSaveSwitchNextExecOtherAction = false;
	_dispDirection = 0;
	_objectFilterEffectCommandData = nullptr;
}

ObjectAction::~ObjectAction()
{
	//	CC_SAFE_RELEASE_NULL(_object);
	CC_SAFE_RELEASE_NULL(_objectData);
	CC_SAFE_RELEASE_NULL(_objectActionData);
	CC_SAFE_RELEASE_NULL(_objectActionLinkData);
	CC_SAFE_RELEASE_NULL(_nextObjectActionLinkList);
	CC_SAFE_RELEASE_NULL(_objCommandList);
	CC_SAFE_RELEASE_NULL(_objectFilterEffectCommandData);
}

agtk::data::ObjectData *ObjectAction::getObjectData()
{
	return _object->getObjectData();
}

agtk::Scene *ObjectAction::getScene()
{
	return _object->getSceneLayer()->getScene();
}

agtk::SceneLayer *ObjectAction::getSceneLayer()
{
	return _object->getSceneLayer();
}

bool compareObjectActionLinkData(agtk::data::ObjectActionLinkData *p1, agtk::data::ObjectActionLinkData *p2)
{
	return p1->getPriority() > p2->getPriority();
}

/**
 * 初期化
 * @param	objectActionData		オブジェクトアクション設定データ
 * @param	object					オブジェクトアクションを設定するオブジェクト
 * @param	isCommon				コモンアクションデータか？
 * @param	commonActionSettingId	コモンアクションセッティングID
 */
bool ObjectAction::init(agtk::data::ObjectActionData *objectActionData, agtk::Object *object, bool isCommon, int commonActionSettingId)
{
	CC_ASSERT(object);

	// 必要なデータを保持
	_object = object;
	this->setIsCommon(isCommon);
	this->setCommonActionSettingId(commonActionSettingId);
	this->setHasHoldCommandList(false);
	this->setObjectData(object->getObjectData());
	this->setObjectActionData(objectActionData);

	// その他実行アクションリスト生成
	auto objCommandList = cocos2d::__Array::create();
	this->setObjCommandList(objCommandList);

	// セットアップ
	this->setup();

	return true;
}


/**
* アクションリンクの登録
* @param	commonActionLinkList	コモンアクションへのリンクリスト
*/
void ObjectAction::registActionLink(cocos2d::__Array* commonActionLinkList)
{
	// ワーク変数
	bool isCommon = this->getIsCommon();
	auto objectData = this->getObjectData();
	auto objectActionData = this->getObjectActionData();
	std::vector<agtk::data::ObjectActionLinkData *> tmpActionLinkList;
	auto nextObjectActionLinkList = cocos2d::__Dictionary::create();

	// -----------------------------------------------------------
	// コモンアクションの場合
	if (isCommon) {
		cocos2d::DictElement *el = nullptr;
		auto list = objectData->getCommonActionSettingList();
		CCDICT_FOREACH(list, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::ObjectCommonActionSettingData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::ObjectCommonActionSettingData *>(el->getObject());
#endif
			if (p->getObjAction()->getId() == objectActionData->getId()) {
				// このアクションが起点となるアクションリンクリストを生成
				auto nextActionLink = p->getObjActionLink2();
				cocos2d::__Array *arr = cocos2d::__Array::create();
				arr->addObject(nextActionLink);
				nextObjectActionLinkList->setObject(arr, nextActionLink->getPriority());
				break;
			}
		}
	}
	// -----------------------------------------------------------
	// 通常アクションの場合
	else {

		// コモンアクションへのリンクリストがある場合
		if (commonActionLinkList->count() > 0) {
			// このアクションが起点となるアクションリンクリストにコモンアクションへのリンクを追加
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(commonActionLinkList, ref) {
				auto actionLinkData = dynamic_cast<agtk::data::ObjectActionLinkData *>(ref);
				tmpActionLinkList.push_back(actionLinkData);
			}
		}

		// このアクションが起点となるアクションリンクリストを生成
		auto actionLinkList = objectData->getActionLinkList();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(actionLinkList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto actionLinkData = static_cast<agtk::data::ObjectActionLinkData *>(el->getObject());
			auto id = static_cast<cocos2d::Integer *>(actionLinkData->getActionIdPair()->getObjectAtIndex(0));
#else
			auto actionLinkData = dynamic_cast<agtk::data::ObjectActionLinkData *>(el->getObject());
			auto id = dynamic_cast<cocos2d::Integer *>(actionLinkData->getActionIdPair()->getObjectAtIndex(0));
#endif
			if (objectActionData->getId() == id->getValue()) {
				tmpActionLinkList.push_back(actionLinkData);
			}
		}

		// アクションリンクリストをソート(priority順)
		std::sort(tmpActionLinkList.begin(), tmpActionLinkList.end(), compareObjectActionLinkData);

		// 次のアクションへのリンクリスト(key:priority, value:Array of actionLinkData)
		for (unsigned int i = 0; i < tmpActionLinkList.size(); i++) {
			auto actionLinkData = tmpActionLinkList[i];

			cocos2d::__Array *arr = nullptr;
			auto next_action_list = nextObjectActionLinkList->objectForKey(actionLinkData->getPriority());

			if (next_action_list == nullptr) {
				arr = cocos2d::__Array::create();
				arr->addObject(actionLinkData);
				nextObjectActionLinkList->setObject(arr, actionLinkData->getPriority());
			}
			else {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				arr = static_cast<cocos2d::__Array *>(next_action_list);
#else
				arr = dynamic_cast<cocos2d::__Array *>(next_action_list);
#endif
				arr->addObject(actionLinkData);
			}
		}
	}

	this->setNextObjectActionLinkList(nextObjectActionLinkList);
}

/**
* アクションリンク条件チェック
* @return		次のアクションID
* @note		戻り値が -1 の場合は無効か継続
*/
int ObjectAction::checkActionLinkCondition()
{
	// 次のアクションID
	int nextActionId = -1;

	// 無効の場合
	if (this->getIgnored()) {
		//無効
		return nextActionId;
	}

	// UI表示によるオブジェクト停止が発生した場合、アクションの切り替えを発生させない
	if (GuiManager::getInstance()->getObjectStop()) {
		return nextActionId;
	}

	// ----------------------------------------
	// アクションリンクの条件チェック
	// ----------------------------------------
	auto nextObjectActionLinkList = this->getNextObjectActionLinkList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(nextObjectActionLinkList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto arr = static_cast<cocos2d::__Array *>(el->getObject());
#else
		auto arr = dynamic_cast<cocos2d::__Array *>(el->getObject());
#endif
		cocos2d::Ref *ref = nullptr;
		bool result = false;
		CCARRAY_FOREACH(arr, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto objectActionLinkData = static_cast<agtk::data::ObjectActionLinkData *>(ref);
#else
			auto objectActionLinkData = dynamic_cast<agtk::data::ObjectActionLinkData *>(ref);
#endif

			// 「入力に関する条件設定」のチェック
			bool retInputCondition = this->checkInputCondition(objectActionLinkData);

			// 「その他条件設定」のチェック
			int retLinkCondition = this->checkLinkCondition(objectActionLinkData);

			// 「アクションを切り替える条件」で分岐
			switch (objectActionLinkData->getChangeConditionType()) {
				// -----------------------------------------------------------
				// すべての条件が満たされていたら切り替え
				// -----------------------------------------------------------
			case agtk::data::ObjectActionLinkData::kAllConditionsSatisfied:
				if (retLinkCondition < 0) {
					result = retInputCondition;
				}
				else {
					if (retInputCondition == false && (objectActionLinkData->getNoInput() == false && objectActionLinkData->getUseInput() == false)) {
						//「何も入力されなかった」と「入力操作がされた」がOFFの場合は「その他の条件」のみの条件が反映される。
						result = (retLinkCondition > 0);
					}
					else {
						result = (retInputCondition && retLinkCondition);
					}
				}
				break;

				// -----------------------------------------------------------
				// いずれかの条件が満たされていたら切り替え
				// -----------------------------------------------------------
			case agtk::data::ObjectActionLinkData::kAnyConditionSatisfied:
				if (retLinkCondition < 0) {
					result = retInputCondition;
				}
				else {
					result = (retInputCondition || retLinkCondition);
				}
				break;

				// -----------------------------------------------------------
				// アクションが最後まで実行されたら
				// -----------------------------------------------------------
			case agtk::data::ObjectActionLinkData::kActionFinished: {
				auto basePlayer = _object->getBasePlayer();
				if (basePlayer == nullptr) {
					//ゼロフレームのアニメーションとして、遷移を進める。
					result = true;
				}
				else {
					auto animationMotion = basePlayer->getCurrentAnimationMotion();
					result = animationMotion->isAllAnimationFinished();
				}
				break; }

				// -----------------------------------------------------------
				// 強制的に自動切り替え
				// -----------------------------------------------------------
			case agtk::data::ObjectActionLinkData::kAlways:
				result = true;
				break;
			}

			// リンク条件が満たされている場合
			if (result) {
				this->setObjectActionLinkData(objectActionLinkData);
				auto objectData = this->getObjectData();
				nextActionId = this->getIsCommon() ? this->getPreActionID() : objectActionLinkData->getNextActionId();
				if (nextActionId == -1) {
					nextActionId = objectData->getInitialActionId();
				}
				auto nextObjectActionData = objectData->getActionData(nextActionId);
#ifdef USE_PREVIEW
				//リンク条件を満たしたリンクID情報をEditorに送信する。
				auto scene = this->getScene();
				if (scene->getPreviewInstanceId() == this->_object->getInstanceId()) {
					auto commonActionSettingId = -1;
					if (this->getIsCommon()) {
						commonActionSettingId = this->getCommonActionSettingId();
					}
					else {
						auto nextObjectAction = _object->getObjectAction(nextActionId);
						if (nextObjectAction->getIsCommon()) {
							commonActionSettingId = nextObjectAction->getCommonActionSettingId();
						}
					}
					auto message = cocos2d::String::createWithFormat("object feedbackActionInfo { \"linkId\": %d, \"objectId\": %d, \"instanceId\": %d, \"commonActionSettingId\": %d }", objectActionLinkData->getId(), this->getObjectData()->getId(), _object->getInstanceId(), commonActionSettingId);
					auto gameManager = GameManager::getInstance();
					auto ws = gameManager->getWebSocket();
					if (ws) {
						ws->send(message->getCString());
					}
				}
#endif
				// コモンアクションから戻るアクションが削除された場合の対処
				if (nullptr == nextObjectActionData) {
					return -1;
				}

				if (nextObjectActionData->getJumpable()) {
					_object->setJumpAction();
				}

				// リンク条件が満たされたのでリンクチェック終了
				break;
			}
		}

		// リンク条件が満たされている場合
		if (result) {
			// 以降のリンクチェックは行わない
			break;
		}
	}

	return nextActionId;
}

/**
 * 更新
 * @param	dt	前フレームからの経過時間
 */
void ObjectAction::update(float dt)
{
	// 無効の場合
	if (this->getIgnored()) {
		//無効
		return;
	}

	// 時間を経過
	_duration += dt;

	_waitDuration300 -= (int)(dt * 300);
	if (_waitDuration300 < 0) _waitDuration300 = 0;

	// 「その他の実行アクション」を実行
	this->updateOtherExecAction();
}

/**
 * その他実行アクションの処理
 */
void ObjectAction::updateOtherExecAction()
{
	DisableChangingFileSaveSwitchInScope disableChangingSaveSwitch(this);

	// 保留となったコマンドリストがあるフラグリセット
	this->setHasHoldCommandList(false);

	// その他の実行アクションリスト取得
	auto objCommandList = this->getObjCommandList();

	bool isLoopBreak = false;

	auto storeCommandList = cocos2d::__Array::create();
	while (objCommandList->count() && _waitDuration300 == 0 && !this->getHasHoldCommandList() && !isLoopBreak) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto commandData = static_cast<agtk::data::ObjectCommandData *>(objCommandList->getObjectAtIndex(0));
#else
		auto commandData = dynamic_cast<agtk::data::ObjectCommandData *>(objCommandList->getObjectAtIndex(0));
#endif
		if (commandData->getIgnored()) {
			//無効。
			objCommandList->removeObject(commandData);
			continue;
		}
		//継続処理フラグ。
		bool bContinuation = false;

		// コマンド別処理
		if (commandData->getCommandType() >= agtk::data::ObjectCommandData::kCustomHead) {
			// カスタム
			auto commandBehavior = this->execActionCustom(commandData);
			if (commandBehavior == kCommandBehaviorLoop) {
				bContinuation = true;
			} else if (commandBehavior == kCommandBehaviorBlock) {
				this->setHasHoldCommandList(true);
				bContinuation = true;
			} else if (commandBehavior == kCommandBehaviorBreak) {
				// 以降のアクションは行わない
				isLoopBreak = true;
			}
		} else
		switch (commandData->getCommandType())
		{
			// テンプレート移動の設定
			case agtk::data::ObjectCommandData::kTemplateMove: this->execActionTemplateMove(commandData); break;
			// オブジェクトをロック(1)
			case agtk::data::ObjectCommandData::kObjectLock: bContinuation = this->execActionObjectLock(commandData); break;
			// オブジェクトを生成(2)
			case agtk::data::ObjectCommandData::kObjectCreate: this->execActionObjectCreate(commandData); break;
			// オブジェクトを変更(3)
			case agtk::data::ObjectCommandData::kObjectChange: this->execActionObjectChange(commandData); break;
			// オブジェクトを移動させる(4)
			case agtk::data::ObjectCommandData::kObjectMove: this->execActionObjectMove(commandData); break;
			// オブジェクトを押す・引く(5)
			case agtk::data::ObjectCommandData::kObjectPushPull: bContinuation = this->execActionObjectPushPull(commandData); break;
			// 
			case agtk::data::ObjectCommandData::kParentChildChange:
				CC_ASSERT(0);
				break;
			// レイヤーを移動(7)
			case agtk::data::ObjectCommandData::kLayerMove: this->execActionLayerMove(commandData); break;
			// 攻撃の設定(8)
			case agtk::data::ObjectCommandData::kAttackSetting: this->execActionAttackSetting(commandData); break;
			// 弾を発射(9)
			case agtk::data::ObjectCommandData::kBulletFire: this->execActionBulletFire(commandData); break;
			// オブジェクトを消滅する(10)
			case agtk::data::ObjectCommandData::kDisappear: this->execActionDisappear(commandData); break;
			// 消滅状態のオブジェクトを復活させる(11)
			case agtk::data::ObjectCommandData::kDisappearObjectRecover: this->execActionDisappearObjectRecover(commandData); break;
			// オブジェクトを無効にする(12)
			case agtk::data::ObjectCommandData::kDisable: this->execActionDisable(commandData); break;
			// 無効状態のオブジェクトを有効にする(13)
			case agtk::data::ObjectCommandData::kDisableObjectEnable: this->execActionDisableObjectEnable(commandData); break;
			// オブジェクトにフィルター効果を設定(17)
			case agtk::data::ObjectCommandData::kObjectFilterEffect: this->execActionObjectFilterEffect(commandData); break;
			// オブジェクトにフィルター効果を削除(18)
			case agtk::data::ObjectCommandData::kObjectFilterEffectRemove: this->execActionObjectFilterEffectRemove(commandData); break;
			// シーンに画面効果を設定(19)
			case agtk::data::ObjectCommandData::kSceneEffect: this->execActionSceneEffect(commandData); break;
			// シーンに画面効果を削除(20)
			case agtk::data::ObjectCommandData::kSceneEffectRemove: this->execActionSceneEffectRemove(commandData); break;
			// 重力効果を変更する(21)
			case agtk::data::ObjectCommandData::kSceneGravityChange: this->execActionSceneGravityChange(commandData); break;
			// シーンを回転・反転(23)
			case agtk::data::ObjectCommandData::kSceneRotateFlip: this->execActionSceneRotateFlip(commandData); break;
			// カメラの表示領域を変更する(24)
			case agtk::data::ObjectCommandData::kCameraAreaChange: this->execActionCameraAreaChange(commandData); break;
			// 音の再生(25)
			case agtk::data::ObjectCommandData::kSoundPlay: this->execActionSoundPlay(commandData); break;
			// テキストを表示(26)
			case agtk::data::ObjectCommandData::kMessageShow: this->execActionMessageShow(commandData); break;
			// スクロールメッセージを設定(27)
			case agtk::data::ObjectCommandData::kScrollMessageShow: this->execActionScrollMessageShow(commandData); break;
			// エフェクトの表示(28)
			case agtk::data::ObjectCommandData::kEffectShow: this->execActionEffectShow(commandData); break;
			// 動画を再生(29)
			case agtk::data::ObjectCommandData::kMovieShow: this->execActionMovieShow(commandData); break;
			// 画像を表示(30)
			case agtk::data::ObjectCommandData::kImageShow: this->execActionImageShow(commandData); break;
			// スイッチ・変数を変更(31)
			case agtk::data::ObjectCommandData::kSwitchVariableChange: this->execActionSwitchVariableChange(commandData); break;
			// スイッチを初期値に戻す(32)
			case agtk::data::ObjectCommandData::kSwitchVariableReset: this->execActionSwitchVariableReset(commandData); break;
			// ゲームスピードを変更(33)
			case agtk::data::ObjectCommandData::kGameSpeedChange: this->execActionGameSpeedChange(commandData); break;
			// ウェイトを入れる(34)
			case agtk::data::ObjectCommandData::kWait:
				this->execActionWait(commandData);
				this->setHasHoldCommandList(true);
				break;
			// シーン終了(35)
			case agtk::data::ObjectCommandData::kSceneTerminate: this->execActionSceneTerminate(commandData); break;
			// 移動方向を指定して移動(38)
			case agtk::data::ObjectCommandData::kDirectionMove: this->execActionDirectionMove(commandData); break;
			// 前後移動と旋回(39)
			case agtk::data::ObjectCommandData::kForthBackMoveTurn: this->execActionForthBackMoveTurn(commandData); break;
			// オブジェクトのアクション実行(40)
			case agtk::data::ObjectCommandData::kActionExec:
				// オブジェクトのアクションが自分自身を含む場合
				if (this->execActionActionExec(commandData)) {
					// 以降のアクションは行わない
					isLoopBreak = true;
				}
				break;
			// パーティクルの表示(41)
			case agtk::data::ObjectCommandData::kParticleShow: this->execActionParticleShow(commandData); break;
			// タイマー(42)
			case agtk::data::ObjectCommandData::kTimer: this->execActionTimer(commandData); break;
			// 画面振動(43)
			case agtk::data::ObjectCommandData::kSceneShake: this->execActionSceneShake(commandData); break;
			// エフェクトを非表示(44)
			case agtk::data::ObjectCommandData::kEffectRemove: this->execActionEffectRemove(commandData); break;
			// パーティクルを非表示(45)
			case agtk::data::ObjectCommandData::kParticleRemove: this->execActionParticleRemove(commandData); break;
			// レイヤーの表示OFF(46)
			case agtk::data::ObjectCommandData::kLayerHide: this->execActionLayerHide(commandData); break;
			// レイヤーの表示ON(47)
			case agtk::data::ObjectCommandData::kLayerShow: this->execActionLayerShow(commandData); break;
			// レイヤーの動作OFF(48)
			case agtk::data::ObjectCommandData::kLayerDisable: this->execActionLayerDisable(commandData); break;
			// レイヤーの動作ON(49)
			case agtk::data::ObjectCommandData::kLayerEnable: this->execActionLayerEnable(commandData); break;
			// スクリプトを記述して実行(50)
			case agtk::data::ObjectCommandData::kScriptEvaluate: {
				auto commandBehavior = this->execActionScriptEvaluate(commandData);
				if (commandBehavior == kCommandBehaviorLoop) {
					bContinuation = true;
				} else if (commandBehavior == kCommandBehaviorBlock) {
					this->setHasHoldCommandList(true);
					bContinuation = true;
				} else if (commandBehavior == kCommandBehaviorBreak) {
					// 以降のアクションは行わない
					isLoopBreak = true;
				}
				break;
			}
			// 音の停止(51)
			case agtk::data::ObjectCommandData::kSoundStop: this->execActionSoundStop(commandData); break;
			// メニュー画面を表示(52)
			case agtk::data::ObjectCommandData::kMenuShow: this->execActionMenuShow(commandData); break;
			// メニュー画面を非表示(53)
			case agtk::data::ObjectCommandData::kMenuHide: this->execActionMenuHide(commandData); break;
			// 表示方向と同じ方へ移動(54)
			case agtk::data::ObjectCommandData::kDisplayDirectionMove: this->execActionDisplayDirectionMove(commandData); break;
			// ファイルをロード(55)
			case agtk::data::ObjectCommandData::kFileLoad: this->execActionFileLoad(commandData); break;
			// 音の再生位置を保存(56)
			case agtk::data::ObjectCommandData::kSoundPositionRemember: this->execActionSoundPositionRemember(commandData); break;
			// ロックを解除(57)
			case agtk::data::ObjectCommandData::kObjectUnlock: this->execActionObjectUnlock(commandData); break;
			// アニメーションの素材セットを変更(58)
			case agtk::data::ObjectCommandData::kResourceSetChange: this->execActionResourceSetChange(commandData); break;
			// データベースの値反映(59)
			case agtk::data::ObjectCommandData::kDatabaseReflect: this->execActionDatabaseReflect(commandData); break;
			// #AGTK-NX Nintendo Switch 振動コントローラーアクション(60)
			case agtk::data::ObjectCommandData::kNXVibrateController:
				#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
				#endif
				break;
			// #AGTK-NX Nintendo Switch コントローラー接続設定表示アクション(61)
			case agtk::data::ObjectCommandData::kNXShowControllerApplet:
				#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
				#endif
				break;

			// その他不正なコマンド
			default:CC_ASSERT(0);
		}

		//実行アクションログ記録
		this->logExecAction(commandData);

		if (bContinuation) {
			storeCommandList->addObject(commandData);
		}
		objCommandList->removeObject(commandData);
	}

	//継続アクションを再登録する。
	if (storeCommandList->count() > 0) {
		int index = 0;
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(storeCommandList, ref) {
			objCommandList->insertObject(ref, index++);
		}
	}
}

void ObjectAction::logExecAction(agtk::data::ObjectCommandData * commandData)
{
	auto debugManager = DebugManager::getInstance();
	const char *objectName = nullptr;
	if (_object->getScenePartObjectData()) {
		objectName = _object->getScenePartObjectData()->getName();
	}
	else {
		objectName = _object->getObjectData()->getName();
	}
	auto objectActionData = _object->getCurrentObjectAction()->getObjectActionData();
	debugManager->getDebugExecuteLogWindow()->addLog(
		"[%s->%s]:%s",
		objectName, 
		objectActionData->getName(),
		GameManager::tr(agtk::data::ObjectCommandData::getCommandTypeName(commandData->getCommandType()))
	);
}

void ObjectAction::execActionTemplateMove(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandTemplateMoveData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandTemplateMoveData *>(commandData);
#endif
	CC_ASSERT(cmd);
	auto templateMove = _object->getObjectTemplateMove();
	templateMove->start(cmd);
}

/**
 * オブジェクトをロック
 * @param	commandData	コマンドデータ
 */
bool ObjectAction::execActionObjectLock(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandObjectLockData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandObjectLockData *>(commandData);
#endif
	CC_ASSERT(cmd);
	auto scene = this->getScene();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	auto objectAll = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
	auto objectAll = scene->getObjectAll(this->getSceneLayer()->getType());
#endif
	CC_ASSERT(objectAll);

#if 0//ACT2-2404 「ロック元が移動するとロックが解除される」の修正。
	// 前にロックしている場合は、ロックを解除する
	{
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectAll, ref) {
			auto object = dynamic_cast<agtk::Object *>(ref);
			auto playObjectData = object->getPlayObjectData();

			playObjectData->removeLocking(_object->getInstanceId());
		}
	}
#endif

	//ロック対象のオブジェクトを取得する。
	auto objectList = cocos2d::__Array::create();

	// 触れたオブジェクトをロックする場合
	if (cmd->getLockTouchedObject()) {
		// 壁判定で接触しているオブジェクトリストを生成
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		auto list = _object->getLeftWallObjectList();
		for (int i = 0; i < (int)list->size(); i++) {
			objectList->addObject((*list)[i]);
		}
		list = _object->getRightWallObjectList();
		for (int i = 0; i < (int)list->size(); i++) {
			objectList->addObject((*list)[i]);
		}
		list = _object->getUpWallObjectList();
		for (int i = 0; i < (int)list->size(); i++) {
			objectList->addObject((*list)[i]);
		}
		list = _object->getDownWallObjectList();
		for (int i = 0; i < (int)list->size(); i++) {
			objectList->addObject((*list)[i]);
		}
		//objectList->addObjectsFromArray(_object->getLeftWallObjectList());
		//objectList->addObjectsFromArray(_object->getRightWallObjectList());
		//objectList->addObjectsFromArray(_object->getUpWallObjectList());
		//objectList->addObjectsFromArray(_object->getDownWallObjectList());
#else
		objectList->addObjectsFromArray(_object->getLeftWallObjectList());
		objectList->addObjectsFromArray(_object->getRightWallObjectList());
		objectList->addObjectsFromArray(_object->getUpWallObjectList());
		objectList->addObjectsFromArray(_object->getDownWallObjectList());
#endif
	}

	//視界・照明に入ったオブジェクトをロックする場合
	if (cmd->getLockViewportLightObject()) {
		cocos2d::DictElement *el = nullptr;

		auto layerId = _object->getLayerId();
		auto viewportLightSceneLayer = scene->getViewportLight()->getViewportLightSceneLayer(layerId);
		auto viewportLightObject = viewportLightSceneLayer->getViewportLightObject(_object);

		auto viewLightList = _object->getObjectData()->getViewportLightSettingList();
		CCDICT_FOREACH(viewLightList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto viewLight = static_cast<agtk::data::ObjectViewportLightSettingData *>(el->getObject());
#else
			auto viewLight = dynamic_cast<agtk::data::ObjectViewportLightSettingData *>(el->getObject());
#endif
			auto viewportLightSprite = viewportLightObject->getViewportLightSprite(el->getIntKey());
			auto rotation = viewportLightSprite->getRotation();

			// 視界・照明がONの場合
			auto isViewLight = viewportLightSprite->checkSwitch();
			if (isViewLight) {
				// 中心位置の設定
				cocos2d::Vec2 pos;
				switch (viewLight->getPositionType()) {
					// このオブジェクトの中心
				case agtk::data::ObjectViewportLightSettingData::kPositionCenter: {
					pos = _object->getCenterPosition();
				}break;
					// このオブジェクトの足元
				case agtk::data::ObjectViewportLightSettingData::kPositionFoot: {
					pos = _object->getCenterPosition();
					pos.y += _object->getContentSize().height * 0.5f;
				} break;
					// 接続点を仕様
				case agtk::data::ObjectViewportLightSettingData::kPositionUseConnection: {
					cocos2d::Rect rect = _object->getAreaRect(viewLight->getConnectionId());
					pos = rect.origin;
				}break;

				default: CC_ASSERT(0);
				}

				pos.x += viewLight->getAdjustX();
				pos.y += viewLight->getAdjustY();

				float sx = viewLight->getScaleX() * 0.01f;
				float sy = viewLight->getScaleY() * 0.01f;

				float rad = viewLight->getRadius();

				float angle = agtk::Scene::getAngleMathFromCocos2d(rotation) + agtk::Scene::getAngleMathFromScene(viewLight->getRotation());
				int dispDirectionId = _object->getDispDirection();
				auto areaDirection = agtk::GetDirectionFromMoveDirectionId(dispDirectionId);
				float arcAngle = viewLight->getAngleRange();

				// 視野・照明の当たり判定を生成
				pos = agtk::Scene::getPositionSceneFromCocos2d(pos);
				auto hitShape1 = agtk::PolygonShape::createFan(pos, rad, angle, arcAngle, sx, sy);

#if 0
				// 視界・照明範囲の可視化
				{
					auto tpos = pos;// agtk::Scene::getPositionSceneFromCocos2d(pos);
					auto primitiveManager = PrimitiveManager::getInstance();
					auto rectangle = PolygonShape::createFan(tpos, rad, angle, arcAngle, sx, sy);
					auto polyArea = primitiveManager->createPolygon(0, 0, rectangle->_vertices, rectangle->_segments, cocos2d::Color4F(1, 1, 0, 1), cocos2d::Color4F(1, 1, 0, 0.3f));
					primitiveManager->setTimer(polyArea, 0.2f);
					_object->addChild(polyArea);
				}
#endif

				// 視野・照明に接触しているオブジェクトを取得する
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(objectAll, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = static_cast<agtk::Object *>(ref);
#else
					auto object = dynamic_cast<agtk::Object *>(ref);
#endif

					// 自分自身と同じオブジェクトは触れたオブジェクトの対象に入れない
					if (object->getInstanceId() == _object->getInstanceId()) { continue; }
					// 既にロック対象になっているオブジェクトは判定を飛ばす
					if (objectList->containsObject(object)) { continue; }

					// ACT-6093
					if (layerId != object->getLayerId()) {
						if (!cmd->getLockViewportLightOfAcrossLayerObject()) {
							continue;
						}
					}

					auto objRect = object->getRect();
					objRect = agtk::Scene::getRectSceneFromCocos2d(objRect);
					auto hitShape2 = agtk::PolygonShape::createRectangle(objRect, object->getRotation());

#if 0
					{
						// オブジェクトの当たり範囲の可視化
						auto primitiveManager = PrimitiveManager::getInstance();
						auto objRectangle = PolygonShape::createRectangle(objRect, 0.0f);
						auto polyObj = primitiveManager->createPolygon(0, 0, objRectangle->_vertices, objRectangle->_segments, cocos2d::Color4F(1, 1, 1, 1), cocos2d::Color4F(1, 1, 1, 0.5));
						primitiveManager->setTimer(polyObj, 2.0f);
						object->addChild(polyObj);
					}
#endif

					// 検索中のオブジェクトと当たらなかった場合は対象に入れない
					if (!agtk::PolygonShape::intersectsFunPolygonShape(hitShape1, hitShape2)) { continue; }

					objectList->addObject(object);
				}
			}
		}
	}

	//画面内のオブジェクトをロックする場合
	if (cmd->getLockObjectOnScreen()) {
		auto camera = scene->getCamera();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectAll, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			bool bWithinCamera = camera->isPositionScreenWithinCamera(
				cocos2d::Rect(
					object->getPosition().x, object->getPosition().y,
					object->getContentSize().width, object->getContentSize().height
				)
			);
			if (bWithinCamera) {
				if (!objectList->containsObject(object)) {
					objectList->addObject(object);
				}
			}
		}
	}

	// 触れたオブジェクトをロックする場合
	bool bContinuation = false;
	if (cmd->getLockObjectTouchedByThisObjectAttack()) {
		// 壁判定で接触しているオブジェクトリストを生成
		auto attackObjectList = _object->getAttackObjectList();
		if (attackObjectList && attackObjectList->count() > 0) {
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(attackObjectList, ref) {
				auto obj = dynamic_cast<agtk::Object *>(ref);
				objectList->addObject(obj);
			}
		}
	}

	// 何もチェックを付けていない場合
	if (!cmd->getLockTouchedObject() &&
		!cmd->getLockViewportLightObject() &&
		!cmd->getLockObjectOnScreen() &&
		!cmd->getLockObjectTouchedByThisObjectAttack()) {
		// 現在、シーン内にいる全てのオブジェクトを対象とする
		objectList->addObjectsFromArray(objectAll);
	}

	//オブジェクトの種類で指定
	switch (cmd->getObjectType()) {
		// -----------------------------------------------
		// オブジェクトの種類で指定
		// -----------------------------------------------
		case agtk::data::ObjectCommandData::kObjectByGroup:
		{
			if (cmd->getObjectGroup() != agtk::data::ObjectCommandData::kObjectTypeAll) {
				for (int i = objectList->count() - 1; i >= 0; i--) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = static_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#else
					auto object = dynamic_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#endif
					if (cmd->getObjectGroup() != object->getObjectData()->getGroup() ) {
						objectList->removeObjectAtIndex(i); // 対象じゃないものを除外
					}
				}
			}
			break;
		}

		// -----------------------------------------------
		// オブジェクトで指定
		// -----------------------------------------------
		case agtk::data::ObjectCommandData::kObjectById:
		{
			// オブジェクトID別
			switch (cmd->getObjectId())
			{
				// -----------------------------------------------
				// 自身のオブジェクト
				// -----------------------------------------------
				//case agtk::data::ObjectCommandData::kSelfObject:
				//{
				//	// 自分以外のオブジェクトを対象リストから省く
				//	for (int i = objectList->count() - 1; i >= 0; i--) {
				//		auto object = dynamic_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
				//		if (object->getId() != _object->getId()) {
				//			objectList->removeObjectAtIndex(i);
				//		}
				//	}
				//} break;

				// -----------------------------------------------
				// 自身以外のオブジェクト
				// -----------------------------------------------
				//case agtk::data::ObjectCommandData::kOtherThanSelfObject:
				//{
				//	// 自分のオブジェクトを対象リストから省く
				//	for (int i = objectList->count() - 1; i >= 0; i--) {
				//		auto object = dynamic_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
				//		if (object->getId() == _object->getId()) {
				//			objectList->removeObjectAtIndex(i);
				//		}
				//	}
				//} break;

				// -----------------------------------------------
				// 設定無し
				// -----------------------------------------------
				case -1: {
					return false;
				}

				// -----------------------------------------------
				// 指定のオブジェクト
				// -----------------------------------------------
				default:
				{
					// 指定のオブジェクト以外は対象リストから省く
					for (int i = objectList->count() - 1; i >= 0; i--) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#else
						auto object = dynamic_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#endif
						if (object->getObjectData()->getId() != cmd->getObjectId()) {
							objectList->removeObjectAtIndex(i);
						}
					}
				} break;
			}
			break;
		}
		// -----------------------------------------------
		// 指定しない
		// -----------------------------------------------
		case agtk::data::ObjectCommandData::kObjectNone:
		{
			// 何もしない
		} break;
		default:CC_ASSERT(0);
	}

	{

		// 変数の条件チェック
		std::function<bool(double, double, int)> checkVariable = [&](double val, double compVal, int op) {
			bool src_isnan = std::isnan(val);
			bool comp_isnan = std::isnan(compVal);
			if (src_isnan || comp_isnan) {
				//非数を含む比較は特別処理。
				switch (op) {
					// -------------------------------------------------
					// <
					// -------------------------------------------------
				case agtk::data::ObjectCommandData::kOperatorLess: return false;

					// -------------------------------------------------
					// <=
					// -------------------------------------------------
				case agtk::data::ObjectCommandData::kOperatorLessEqual: return (src_isnan && comp_isnan);

					// -------------------------------------------------
					// =
					// -------------------------------------------------
				case agtk::data::ObjectCommandData::kOperatorEqual: return (src_isnan && comp_isnan);

					// -------------------------------------------------
					// >=
					// -------------------------------------------------
				case agtk::data::ObjectCommandData::kOperatorGreaterEqual: return (src_isnan && comp_isnan);

					// -------------------------------------------------
					// >
					// -------------------------------------------------
				case agtk::data::ObjectCommandData::kOperatorGreater: return false;
					// -------------------------------------------------
					// !=
					// -------------------------------------------------
				case agtk::data::ObjectCommandData::kOperatorNotEqual: return (src_isnan != comp_isnan);
				}
			}
			else {
				switch (op) {
					// -------------------------------------------------
					// <
					// -------------------------------------------------
				case agtk::data::ObjectCommandData::kOperatorLess: return (val < compVal);

					// -------------------------------------------------
					// <=
					// -------------------------------------------------
				case agtk::data::ObjectCommandData::kOperatorLessEqual: return (val <= compVal);

					// -------------------------------------------------
					// =
					// -------------------------------------------------
				case agtk::data::ObjectCommandData::kOperatorEqual: return (val == compVal);

					// -------------------------------------------------
					// >=
					// -------------------------------------------------
				case agtk::data::ObjectCommandData::kOperatorGreaterEqual: return (val >= compVal);

					// -------------------------------------------------
					// >
					// -------------------------------------------------
				case agtk::data::ObjectCommandData::kOperatorGreater: return (val > compVal);
					// -------------------------------------------------
					// !=
					// -------------------------------------------------
				case agtk::data::ObjectCommandData::kOperatorNotEqual: return (val != compVal);
				}
			}

			return false;
		};

		auto projectPlayData = GameManager::getInstance()->getPlayData();
		
		// ロック対象の変数・スイッチで指定
		switch (cmd->getUseType()) {
			// -------------------------------------------------
			// スイッチを条件に設定
			// -------------------------------------------------
			case agtk::data::ObjectCommandObjectLockData::kUseSwitch: {
				if (cmd->getSwitchId() == -1) break;//指定しない場合。
				for (int i = objectList->count() - 1; i >= 0; i--) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = static_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#else
					auto object = dynamic_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#endif

					auto switchData = object->getPlayObjectData()->getSwitchData(cmd->getSwitchId());
					if (switchData) {
						// スイッチの状態で分岐
						switch (cmd->getSwitchCondition()) {

							// -------------------------------------------------
							// スイッチがON
							// -------------------------------------------------
							case agtk::data::ObjectCommandObjectLockData::kSwitchConditionOn: {
								// スイッチがOFFの場合は対象リストから外す
								if (switchData->getValue() != true) { 
									objectList->removeObjectAtIndex(i);
								}
							} break;

							// -------------------------------------------------
							// スイッチがOFF
							// -------------------------------------------------
							case agtk::data::ObjectCommandObjectLockData::kSwitchConditionOff:
							{
								// スイッチがONの場合は処理しない
								if (switchData->getValue() != false) {
									objectList->removeObjectAtIndex(i);
								}
							} break;

							// -------------------------------------------------
							// スイッチがOFFからONになった
							// -------------------------------------------------
							case agtk::data::ObjectCommandObjectLockData::kSwitchConditionOnFromOff:
							{
								// スイッチがONからOFFになった場合は処理しない
								if (switchData->isState() != agtk::data::PlaySwitchData::kStateOnFromOff) { 
									objectList->removeObjectAtIndex(i);
								}
							} break;

							// -------------------------------------------------
							// スイッチがONからOFFになった
							// -------------------------------------------------
							case agtk::data::ObjectCommandObjectLockData::kSwitchConditionOffFromOn:
							{
								// スイッチがOFFからONになった場合は処理しない
								if (switchData->isState() != agtk::data::PlaySwitchData::kStateOffFromOn) {
									objectList->removeObjectAtIndex(i);
								}
							} break;
						}
					}
					// 指定のスイッチデータが見当たらない場合は対象対象リストから外す
					else {
						objectList->removeObjectAtIndex(i);
					}
				}
			} break;

			// -------------------------------------------------
			// 変数を条件に設定
			// -------------------------------------------------
			case agtk::data::ObjectCommandObjectLockData::kUseVariable: {
				CC_ASSERT(cmd->getCompareVariableQualifierId() == agtk::data::ObjectCommandData::kQualifierSingle);
				if (cmd->getVariableId() == -1) break;//指定しない場合。
				bool findValue = true;
				double compareValue = 0.0f;
				switch (cmd->getCompareValueType()) {
				case agtk::data::ObjectCommandObjectLockData::kVariableCompareValue://定数
					compareValue = cmd->getCompareValue();
					break;
				case agtk::data::ObjectCommandObjectLockData::kVariableCompareVariable: {//他の変数
#if 1
					if (!GameManager::getInstance()->getVariableValue(cmd->getCompareVariableObjectId(), cmd->getCompareVariableQualifierId(), cmd->getCompareVariableId(), compareValue, _object)) {
						findValue = false;
						break;
					}
#else
					if (cmd->getCompareVariableObjectId() == agtk::data::ObjectCommandData::kSelfObject) {//自身のオブジェクト
						auto compareVariableData = _object->getPlayObjectData()->getVariableData(cmd->getCompareVariableId());
						compareValue = compareVariableData->getValue();
					}
					else if (cmd->getCompareVariableObjectId() == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {//Resources共通
						auto projectPlayData = GameManager::getInstance()->getPlayData();
						auto compareVariableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, cmd->getCompareVariableId());
						compareValue = compareVariableData->getValue();
					}
					else if (cmd->getCompareVariableObjectId() > 0) {
						auto projectPlayData = GameManager::getInstance()->getPlayData();
						auto singleInstanceData = projectPlayData->getVariableData(cmd->getCompareVariableObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID);
						auto object = this->getTargetObjectInstanceId((int)singleInstanceData->getValue());
						if (object == nullptr) {
							//比較元のオブジェクトが存在しない事で比較不能のため、比較対象オブジェクトを破棄する。
							objectList->removeAllObjects();
							break;
						}
						auto compareVariableData = object->getPlayObjectData()->getVariableData(cmd->getCompareVariableId());
						compareValue = compareVariableData->getValue();
					}
					else {
						//エラー
						CC_ASSERT(0);
					}
#endif
					break; }
				case agtk::data::ObjectCommandObjectLockData::kVariableCompareNaN://数値以外（非数）
					compareValue = NAN;
					break;
				default:CC_ASSERT(0);
				}
				
				if (!findValue) {
					break;
				}

				for (int i = objectList->count() - 1; i >= 0; i--) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = static_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#else
					auto object = dynamic_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#endif
					auto variableData = object->getPlayObjectData()->getVariableData(cmd->getVariableId());
					if (variableData != nullptr) {
						if (!checkVariable(variableData->getValue(), compareValue, cmd->getCompareVariableOperator())) {
							objectList->removeObjectAtIndex(i);
						}
					}
					else {
						objectList->removeObjectAtIndex(i);
					}
				}
			} break;

			// -------------------------------------------------
			// 指定しない
			// -------------------------------------------------
			case agtk::data::ObjectCommandObjectLockData::kUseNone:
			{
				// 何もしない
			} break;
		}
	}
	// 誰にロックされているかを設定
	{
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			auto playObjectData = object->getPlayObjectData();
			playObjectData->setObjectId(object->getObjectData()->getId());
			playObjectData->setLockTarget(true);//ロック対象ON
			playObjectData->addLocking(_object->getInstanceId());
		}
	}
	return bContinuation;
}

/**
 * オブジェクトの生成
 * @param	commandData	コマンドデータ
 * @note	オブジェクトを生成します
 */
void ObjectAction::execActionObjectCreate(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandObjectCreateData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandObjectCreateData *>(commandData);
#endif
	CC_ASSERT(cmd);

	//オブジェクト生成確率
	double r = ((double)rand() / RAND_MAX) * 100.0;

	// 生成出来る率でない場合
	if (r > cmd->getProbability()) {
		// 何もしない
		return;
	}

	if (cmd->getObjectId() < 0) {
		//設定無し。
		return;
	}

	int connectId = cmd->getConnectId();
	if (cmd->getUseConnect() && cmd->getCreatePosition() == agtk::data::ObjectCommandObjectCreateData::kPositionCenter && connectId >= 0) {
		_object->getExecActionObjectCreateList()->addObject(cmd);
	}
	else {
		_object->execActionObjectCreate(commandData);
	}
}

/**
* オブジェクトの生成
* @param	commandData	コマンドデータ
* @note	オブジェクトを生成します
*/
#if 0
" arg1: <int: オブジェクトID>
 arg2: <int : 生成位置X>
 arg3 : <int : 生成位置Y>
 arg4 : <int : レイヤーID>
ret: <int: インスタンスID>"
#endif
int ObjectAction::execActionObjectCreateForScript(int objectId, int x, int y, int layerId)
{
	// ワーク変数
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneLayer = scene->getSceneLayer(layerId);
	auto viewportLightSceneLayer = scene->getViewportLight()->getViewportLightSceneLayer(layerId);
	cocos2d::__Array *objectList = nullptr;

	//オブジェクト位置
	cocos2d::Vec2 pos(x, y);

#if defined(USE_RUNTIME)
	auto projectData = GameManager::getInstance()->getProjectData();
	if (projectData->getObjectData(objectId)->getTestplayOnly()) {
		return -1;
	}
#endif

	//オブジェクト生成
	auto newObject = agtk::Object::create(
		sceneLayer,
		objectId,
		-1,//initialActionId
		pos,
		cocos2d::Vec2(1, 1),//scale
		0,//rotation
		-1//向き
		, -1, -1, -1
	);

	//オブジェクトをレイヤーに追加。
	sceneLayer->addCollisionDetaction(newObject);
	newObject->setId(sceneLayer->publishObjectId());
	newObject->getPlayObjectData()->setInstanceId(scene->getObjectInstanceId(newObject));
	newObject->getPlayObjectData()->setInstanceCount(scene->incrementObjectInstanceCount(newObject->getObjectData()->getId()));
	scene->updateObjectInstanceCount(newObject->getObjectData()->getId());
	newObject->setLayerId(layerId);
	newObject->setPhysicsBitMask(layerId, scene->getSceneData()->getId());
	auto newObjectData = newObject->getObjectData();
	if (newObjectData->getViewportLightSettingFlag() && newObjectData->getViewportLightSettingList()->count() > 0) {
		auto viewportLightObject = ViewportLightObject::create(newObject, scene->getViewportLight(), sceneLayer);
		viewportLightSceneLayer->getViewportLightObjectList()->addObject(viewportLightObject);
	}
	sceneLayer->addObject(newObject);

	// オブジェクトに紐付いた物理オブジェクトを生成
	sceneLayer->createPhysicsObjectWithObject(newObject);

	//デバッグ表示
	if (DebugManager::getInstance()->getDebugForDevelopment()) {
		auto primitiveManager = PrimitiveManager::getInstance();
		//生成オブジェクト
		auto objRect = newObject->getRect();
		objRect = agtk::Scene::getRectSceneFromCocos2d(objRect);
		auto objRectangle = PolygonShape::createRectangle(objRect, 0.0f);
		auto polyObj = primitiveManager->createPolygon(0, 0, objRectangle->_vertices, objRectangle->_segments, cocos2d::Color4F(1, 1, 1, 1), cocos2d::Color4F(1, 1, 1, 0.5));
		primitiveManager->setTimer(polyObj, 2.0f);
		newObject->addChild(polyObj);
	}

	return newObject->getInstanceId();
}


#ifdef USE_PREVIEW
/**
* オブジェクトの生成(プレビュー用)
* @param	objectId	生成するオブジェクトのID
* @ret	インスタンスID
* @note	オブジェクトを生成します
*/
int ObjectAction::callObjectCreate(int objectId, int actionId, int layerId, cocos2d::Vec2 pos)
{
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto sceneLayer = scene->getSceneLayer(layerId);
	auto viewportLightSceneLayer = scene->getViewportLight()->getViewportLightSceneLayer(layerId);

	int moveDirection = -1;

	//オブジェクト生成
	auto newObject = agtk::Object::create(
		sceneLayer,
		objectId,
		actionId,//initialActionId
		pos,
		cocos2d::Vec2(1, 1),//scale
		0,//rotation
		moveDirection//向き
		, -1, -1, -1
	);

	//オブジェクトをレイヤーに追加。
	sceneLayer->addCollisionDetaction(newObject);
	newObject->setId(sceneLayer->publishObjectId());
	newObject->getPlayObjectData()->setInstanceId(scene->getObjectInstanceId(newObject));
	newObject->getPlayObjectData()->setInstanceCount(scene->incrementObjectInstanceCount(newObject->getObjectData()->getId()));
	scene->updateObjectInstanceCount(newObject->getObjectData()->getId());
	newObject->setLayerId(layerId);
	auto newObjectData = newObject->getObjectData();
	if (newObjectData->getViewportLightSettingFlag() && newObjectData->getViewportLightSettingList()->count() > 0) {
		auto viewportLightObject = ViewportLightObject::create(newObject, scene->getViewportLight(), sceneLayer);
		viewportLightSceneLayer->getViewportLightObjectList()->addObject(viewportLightObject);
	}
	sceneLayer->addObject(newObject);

	//デバッグ表示
	if (DebugManager::getInstance()->getDebugForDevelopment()) {
		auto primitiveManager = PrimitiveManager::getInstance();
		//生成オブジェクト
		auto objRect = newObject->getRect();
		objRect = agtk::Scene::getRectSceneFromCocos2d(objRect);
		auto objRectangle = PolygonShape::createRectangle(objRect, 0.0f);
		auto polyObj = primitiveManager->createPolygon(0, 0, objRectangle->_vertices, objRectangle->_segments, cocos2d::Color4F(1, 1, 1, 1), cocos2d::Color4F(1, 1, 1, 0.5));
		primitiveManager->setTimer(polyObj, 2.0f);
		newObject->addChild(polyObj);
	}

	return newObject->getInstanceId();
}
#endif

ObjectAction *ObjectAction::createForScript()
{
	return new ObjectAction();
}


/**
* オブジェクトを変更
* @param	commandData	コマンドデータ
*/
void ObjectAction::execActionObjectChange(agtk::data::ObjectCommandData *commandData)
{
#if defined(AGTK_DEBUG)
	CCLOG("** 変更元オブジェクト:Variable **");
	auto list1 = _object->getPlayObjectData()->getVariableList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(list1, el) { dynamic_cast<agtk::data::PlayVariableData*>(el->getObject())->dump(); }
	CCLOG("** 変更元オブジェクト:Switch **");
	auto list2 = _object->getPlayObjectData()->getSwitchList();
	CCDICT_FOREACH(list2, el) { dynamic_cast<agtk::data::PlaySwitchData*>(el->getObject())->dump(); }
	CCLOG("------------------------");
#endif

	// オブジェクトを変更コマンドデータにキャスト
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandObjectChangeData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandObjectChangeData *>(commandData);
#endif
	CC_ASSERT(cmd);

	// 「設定無し」の場合は「消滅」扱い
	if (cmd->getObjectId() == TARGET_NONE) {
		_object->setLateRemove(true);
		return;
	}

	//オブジェクト生成確率
	double r = ((double)rand() / RAND_MAX) * 100.0;
	if (r > cmd->getProbability()) {
		return;
	}

	// 現在のシーンとシーンレイヤとオブジェクトデータを取得
	auto scene = this->getScene();
	auto sceneLayer = this->getSceneLayer();
	auto viewportLightSceneLayer = scene->getViewportLight()->getViewportLightSceneLayer(_object->getLayerId());
	auto objectData = _object->getObjectData();

	// 指定オブジェクトを元に生成位置を割り出す
	std::function<bool(agtk::Object *, cocos2d::Vec2 &)> calcObjectPosition = [&](agtk::Object *object, cocos2d::Vec2 &pos) {
		int connectId = cmd->getConnectId();
		if (cmd->getUseConnect() && cmd->getCreatePosition() == agtk::data::ObjectCommandObjectCreateData::kPositionCenter && connectId >= 0) {
			//接続点を使用
			agtk::Vertex4 vertex4;
			if (object->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, connectId, vertex4)) {
				auto sceneData = object->getSceneData();
				pos = Scene::getPositionSceneFromCocos2d(vertex4.addr()[0], sceneData);
			}
			else {
				return false;
			}
		}
		// オブジェクトの中心点の場合
		else {
			pos = object->getCenterPosition();
		}
		pos += Object::getSceneLayerScrollDiff(sceneLayer, object->getSceneLayer());
		return true;
	};

	// 変更後のオブジェクトの生成位置
	cocos2d::Vec2 pos;
	switch (cmd->getCreatePosition()) {
		// ----------------------------------------------------
		// このオブジェクトの中心
		// ----------------------------------------------------
		case agtk::data::ObjectCommandObjectCreateData::kPositionCenter:
		{
			CCLOG("このオブジェクトの中心");
			if (calcObjectPosition(_object, pos) == false) {
				return;
			}
			break;
		}
		// ----------------------------------------------------
		// ロックしたオブジェクトの中心
		// ----------------------------------------------------
		case agtk::data::ObjectCommandObjectCreateData::kPositionLockObjectCenter:
		{
			CCLOG("ロックしたオブジェクトの中心");

			// ロックしたオブジェクトを取得
			auto obj = scene->getObjectLocked(_object->getInstanceId());

			// ロックしたオブジェクトがある場合
			if (nullptr != obj) {
				// 位置を取得
				if (calcObjectPosition(obj, pos) == false) {
					return;
				}
			}
			// ロックしたオブジェクトが存在しない場合
			else {
				// 自身の位置を取得
				if (calcObjectPosition(_object, pos) == false) {
					return;
				}
			}

			break;
		}
		// ----------------------------------------------------
		// 不正なID
		// ----------------------------------------------------
		default:
		{
			CCLOG("** 不正な生成位置: %d", cmd->getCreatePosition());
			CC_ASSERT(0);
		}
	}

	//位置を調整
	pos.x += cmd->getAdjustX();
	pos.y += cmd->getAdjustY();

	CCLOG("表示位置: (%f, %f)", pos.x, pos.y);

#if defined(USE_RUNTIME)
	auto projectData = GameManager::getInstance()->getProjectData();
	if (projectData->getObjectData(cmd->getObjectId())->getTestplayOnly()) {
		return;
	}
#endif
	int directionId = this->getDispDirection();

	//オブジェクト生成
	auto newObject = agtk::Object::create(
		sceneLayer,
		cmd->getObjectId(),
		cmd->getActionId(),//initialActionId
		pos,
		cocos2d::Vec2(1, 1),//scale
		0,//rotation
		directionId,
		-1, -1, -1
	);

	if (directionId > 0) {
		//※agtk::Object::createメソッドのdispDirection（向き）に直接値を入れると移動方向まで設定されるため、移動方向をZEROとする。
		newObject->getObjectMovement()->setDirection(cocos2d::Vec2::ZERO);
	}

	//オブジェクトをレイヤーに追加。
	sceneLayer->addCollisionDetaction(newObject);
	newObject->setId(sceneLayer->publishObjectId());
	if (newObject->getObjectData()->getId() == _object->getObjectData()->getId()) {
		newObject->getPlayObjectData()->setInstanceId(_object->getInstanceId());
	} else {
		newObject->getPlayObjectData()->setInstanceId(scene->getObjectInstanceId(newObject));
	}
	newObject->setLayerId(_object->getLayerId());
	newObject->setPhysicsBitMask(_object->getLayerId(), scene->getSceneData()->getId());
	auto newObjectData = newObject->getObjectData();
	if (newObjectData->getViewportLightSettingFlag() && newObjectData->getViewportLightSettingList()->count() > 0) {
		auto viewportLightObject = ViewportLightObject::create(newObject, scene->getViewportLight(), sceneLayer);
		viewportLightSceneLayer->getViewportLightObjectList()->addObject(viewportLightObject);
	}
	sceneLayer->insertObject(newObject, _object);

	// オブジェクトに紐付いた物理オブジェクトを生成
	sceneLayer->createPhysicsObjectWithObject(newObject);

	//デバッグ表示
	if (DebugManager::getInstance()->getDebugForDevelopment()) {
		auto primitiveManager = PrimitiveManager::getInstance();
		//生成オブジェクト
		auto objRect = newObject->getRect();
		objRect = agtk::Scene::getRectSceneFromCocos2d(objRect);
		auto objRectangle = PolygonShape::createRectangle(objRect, 0.0f);
		auto polyObj = primitiveManager->createPolygon(0, 0, objRectangle->_vertices, objRectangle->_segments, cocos2d::Color4F(1, 1, 1, 1), cocos2d::Color4F(1, 1, 1, 0.5));
		primitiveManager->setTimer(polyObj, 2.0f);
		newObject->addChild(polyObj);
		//破棄オブジェクト
		objRect = _object->getRect();
		objRect = agtk::Scene::getRectSceneFromCocos2d(objRect);
		objRectangle = PolygonShape::createRectangle(objRect, 0.0f);
		polyObj = primitiveManager->createPolygon(0, 0, objRectangle->_vertices, objRectangle->_segments, cocos2d::Color4F(1, 0, 0, 1), cocos2d::Color4F(1, 0, 0, 0.5));
		primitiveManager->setTimer(polyObj, 2.0f);
		newObject->addChild(polyObj);
	}

	//オブジェクトの変数を引き継ぎ
	if (cmd->getTakeOverVariables()) {
		newObject->getPlayObjectData()->takeOverVariableList(_object->getPlayObjectData());

		//※引き継いだ際に、前回の位置を設定し直すため、こちらで再度、新しい位置を設定する。
		auto playObjectData = newObject->getPlayObjectData();
		auto variableX = playObjectData->getVariableData(agtk::data::kObjectSystemVariableX);
		auto variableY = playObjectData->getVariableData(agtk::data::kObjectSystemVariableY);
		variableX->setExternalValue(pos.x);
		variableY->setExternalValue(pos.y);
	}

	//オブジェクトのスイッチを引き継ぎ
	if (cmd->getTakeOverSwitches()) {
		newObject->getPlayObjectData()->takeOverSwitchList(_object->getPlayObjectData());
		//データを調整する。
		auto playData = GameManager::getInstance()->getPlayData();
		if (playData) {
			playData->adjustData();
		}
	}

	// オブジェクトの親子関係を引き継ぐ
	if (cmd->getTakeOverParentChild()) {
		// 変更前の親インスタンスIDを変更後オブジェクトに保存
		newObject->getPlayObjectData()->setParentObjectInstanceId(_object->getPlayObjectData()->getParentObjectInstanceId());
		auto parentObject = this->getTargetObjectInstanceId(_object->getPlayObjectData()->getParentObjectInstanceId());
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
		if (parentObject > 0) {
#endif
			parentObject->addChildObject(newObject, _object->_parentFollowPosOffset, _object->_parentFollowConnectId);
		}

		auto childrenList = cocos2d::__Array::create();
		childrenList->addObjectsFromArray(_object->getChildrenObjectList());
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(childrenList, ref) {
			auto connectObj = dynamic_cast<agtk::ConnectObject *>(ref);
			if (connectObj) {
				// オブジェクトの接続で親子関係がある場合
				// 親子関係判定
				if (connectObj->getPlayObjectData()->getParentObjectInstanceId() == _object->getPlayObjectData()->getInstanceId()) {
					// 親インスタンスIDを変更後オブジェクトに設定
					connectObj->getPlayObjectData()->setParentObjectInstanceId(newObject->getPlayObjectData()->getInstanceId());
				}
				else {
					// 接続を引き継がない
					continue;
				}
				// 接続の引継ぎ
				auto connectObjectList = _object->getConnectObjectList();
				auto newSettingList = newObject->getObjectData()->getConnectSettingList();
				auto settingData = connectObj->getObjectConnectSettingData();
				cocos2d::DictElement *el = nullptr;
				CCDICT_FOREACH(newSettingList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto newSettingData = static_cast<agtk::data::ObjectConnectSettingData *>(el->getObject());
#else
					auto newSettingData = dynamic_cast<agtk::data::ObjectConnectSettingData *>(el->getObject());
#endif
					// 接続するオブジェクトが同じでなければ接続を引き継がない
					if (newSettingData->getObjectId() != connectObj->getPlayObjectData()->getObjectId()) {
						continue;
					}

					//connectObj->setOwnParentObject(newObject);					// ownParentObjectを変更後オブジェクトに設定 ※addChildObjectで変更するのでコメント
					connectObj->changeConnectBaseObject(newObject);					// connectBaseObjectを変更後オブジェクトに設定
					connectObj->changeObjectConnectSettingData(newSettingData);		// オブジェクトの接続設定を変更後オブジェクトに設定
																					// 接続状態更新
					if (newSettingData->getObjectSwitch()) {
						// 「このオブジェクトのスイッチ」にチェックが入っている場合
						if (newSettingData->getObjectSwitchId() == -1) {
							// 「無し」設定時は常にアクティブ
							connectObj->setIsConnecting(true);
						}
						else if (newSettingData->getObjectSwitchId() != settingData->getObjectSwitchId()) {
							// オブジェクト固有のスイッチが違う場合は更新
							auto switchData = newObject->getPlayObjectData()->getSwitchData(newSettingData->getObjectSwitchId());
							if (switchData != nullptr) {
								connectObj->setIsConnecting(switchData->getValue());
							}
						}
						// 同じスイッチの場合はそのまま引き継ぎ
					}
					else {
						// 「システム共通」にチェックが入っている場合
						if (newSettingData->getSystemSwitchId() == -1) {
							// 「無し」設定時は常にアクティブ
							connectObj->setIsConnecting(true);
						}
						else if (newSettingData->getSystemSwitchId() != settingData->getSystemSwitchId()) {
							// システム共通のスイッチが違う場合は更新
							auto switchData = GameManager::getInstance()->getPlayData()->getCommonSwitchData(newSettingData->getSystemSwitchId());
							if (switchData != nullptr) {
								connectObj->setIsConnecting(switchData->getValue());
							}
						}
						// 同じスイッチの場合はそのまま引き継ぎ
					}

					if (connectObj->getIsConnecting()) {
						// 子オブジェクトの座標を位置調整
						switch (newSettingData->getPositionType()) {
							// このオブジェクトの中心
						case agtk::data::ObjectConnectSettingData::EnumPositionType::kPositionCenter: {
							pos = newObject->getCenterPosition();
							break; }
																									  // このオブジェクトの足元
						case agtk::data::ObjectConnectSettingData::EnumPositionType::kPositionFoot: {
							pos = newObject->getFootPosition();
							break; }
																									// 接続点を使用
						case agtk::data::ObjectConnectSettingData::EnumPositionType::kPositionUseConnection: {
							int connectId = newSettingData->getConnectionId();
							pos = newObject->getCenterPosition();
							if (connectId > 0) {
								agtk::Vertex4 vertex4;
								if (newObject->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, connectId, vertex4)) {
									pos = Scene::getPositionSceneFromCocos2d(vertex4.addr()[0], newObject->getSceneData());
								}
							}
							break; }
						}
						connectObj->setPosition(pos);

						// リストの更新
						connectObjectList->removeObject(connectObj);
						newObject->getConnectObjectList()->addObject(connectObj);
						//newObject->addChildObject(dynamic_cast<agtk::Object *>(connectObj), pos, newSettingData->getConnectionId());
						newObject->addChildObject(connectObj, cocos2d::Vec2(newSettingData->getAdjustX(), newSettingData->getAdjustY()), newSettingData->getConnectionId());
					}
					break;
				}
			}
			else {
				// オブジェクトの生成で親子関係がある場合
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto obj = static_cast<agtk::Object *>(ref);
#else
				auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
				newObject->addChildObject(obj, obj->_parentFollowPosOffset, obj->_parentFollowConnectId);
				obj->getPlayObjectData()->setParentObjectInstanceId(newObject->getPlayObjectData()->getInstanceId());
			}
		}
	}

	// ヒットストップ中に変更された場合、現在のヒットストップ情報を差し替え後オブジェクトにセット
	if (_object->getDioExecuting()) {
		newObject->setDioExecuting(_object->getDioExecuting());
		newObject->setDioFrame(_object->getDioFrame());
		newObject->setDioGameSpeed(_object->getDioGameSpeed());
		newObject->setDioEffectDuration(_object->getDioEffectDuration());
		newObject->setDioParent(_object->getDioParent());
		newObject->setDioChild(_object->getDioChild());
	}

	//表示方向変数値を更新。
	newObject->updateDisplayDirectionVariable();

	//デバッグ用ウインドウが立ち上がっている場合。
	auto debugManager = DebugManager::getInstance();
	if (debugManager->getShowDebugObjectInfoWindow()) {
		auto objectInfoWindow = debugManager->getObjectInfoWindow(_object);
		if (objectInfoWindow != nullptr) {
			debugManager->createObjectInfoWindow(newObject, objectInfoWindow->getPosition());
			debugManager->removeObjectInfoWindow(_object);
		}
	}

	// このコマンド以降のコマンドを破棄
	_object->getCurrentObjectAction()->getObjCommandList()->removeAllObjects();

	// 変更前のオブジェクト消滅（※復活しないように復活条件無視する）
	_object->removeSelf(false, true);

#if defined(AGTK_DEBUG)
	CCLOG("** 変更後オブジェクト:Variable **");
	auto list3 = newObject->getPlayObjectData()->getVariableList();
	CCDICT_FOREACH(list3, el) { dynamic_cast<agtk::data::PlayVariableData*>(el->getObject())->dump(); }
	CCLOG("** 変更後オブジェクト:Switch **");
	auto list4 = newObject->getPlayObjectData()->getSwitchList();
	CCDICT_FOREACH(list4, el) { dynamic_cast<agtk::data::PlaySwitchData*>(el->getObject())->dump(); }
	CCLOG("------------------------");
#endif
}

void ObjectAction::execActionObjectMove(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandObjectMoveData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandObjectMoveData *>(commandData);
#endif
	CC_ASSERT(cmd);
	//移動対象のオブジェクトを指定。
	cocos2d::__Array *objectList = nullptr;
	switch (cmd->getTargettingType()) {
	case agtk::data::ObjectCommandObjectMoveData::kTargettingByGroup://オブジェクトの種類で指定
		objectList = this->getTargetObjectByGroup(cmd->getTargetObjectGroup());
		break;
	case agtk::data::ObjectCommandObjectMoveData::kTargettingById: {//オブジェクトで指定
		auto targetObjectId = cmd->getTargetObjectId();
		if (targetObjectId == agtk::data::ObjectCommandData::kSelfObject) {//自身のオブジェクト
			objectList = cocos2d::__Array::create();
			objectList->addObject(_object);
		} else
		if (targetObjectId == agtk::data::ObjectCommandData::kOtherThanSelfObject) {//自身以外のオブジェクト
			objectList = cocos2d::__Array::create();
			auto scene = this->getScene();
			auto objectAllList = scene->getObjectAll(this->getSceneLayer()->getType());
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(objectAllList, ref) {
				auto object = dynamic_cast<agtk::Object *>(ref);
				if (object == _object) continue;
				objectList->addObject(object);
			}
		} else 
		switch (cmd->getTargetQualifierId()) {
		case agtk::data::ObjectCommandData::kQualifierSingle: {//単体
			objectList = cocos2d::__Array::create();
			agtk::Object *object = nullptr;
			auto projectPlayData = GameManager::getInstance()->getPlayData();
			auto singleInstanceData = projectPlayData->getVariableData(targetObjectId, agtk::data::kObjectSystemVariableSingleInstanceID);
			if (singleInstanceData != nullptr) {
				object = this->getTargetObjectInstanceId((int)singleInstanceData->getValue());
			}
			if (object) {
				objectList->addObject(object);
			}
			break; }
		case agtk::data::ObjectCommandData::kQualifierWhole: {//全体
			objectList = this->getTargetObjectById(cmd->getTargetObjectId());
			break; }
		default: {
			if (cmd->getTargetQualifierId() >= 0) {
				objectList = cocos2d::__Array::create();
				auto objectList2 = this->getTargetObjectById(cmd->getTargetObjectId());
				cocos2d::Ref *ref2;
				CCARRAY_FOREACH(objectList2, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto obj = static_cast<agtk::Object *>(ref2);
#else
					auto obj = dynamic_cast<agtk::Object *>(ref2);
#endif
					if (obj->getInstanceId() == cmd->getTargetQualifierId()) {
						objectList->addObject(obj);
					}
				}
			} else {
				CC_ASSERT(0);
			}
			break; }
		}
		break; }
	case agtk::data::ObjectCommandObjectMoveData::kTargettingTouched: {//このオブジェクトに触れたオブジェクト
		if (!_object->getCollisionWallWallChecked()) {
			_object->updateCollisionWallWallList();
		}
		objectList = cocos2d::__Array::create();
		cocos2d::Ref *ref;
		auto collisionWallWallList = _object->getCollisionWallWallList();
		CCARRAY_FOREACH(collisionWallWallList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			if (cmd->getExcludeObjectGroupBit() & object->getObjectData()->getGroupBit()) { continue; }
			objectList->addObject(object);
		}
		break; }
	case agtk::data::ObjectCommandObjectMoveData::kTargettingLocked://このオブジェクトがロックしたオブジェクト
		objectList = this->getTargetObjectLocked();
		break;
	default:CC_ASSERT(0);
	}

	auto primitiveManager = PrimitiveManager::getInstance();
	auto scene = this->getScene();

	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object = static_cast<agtk::Object *>(ref);
#else
		auto object = dynamic_cast<agtk::Object *>(ref);
#endif
		//移動方法
		cocos2d::Vec2 startPos = agtk::Scene::getPositionCocos2dFromScene(object->getPosition());
		cocos2d::Vec2 endPos = startPos;
		cocos2d::Vec2 mv;

		cocos2d::Vec2 direction = cocos2d::Vec2::ZERO;
#ifdef FIX_ACT2_5237
		bool followCameraMoving = false;
#endif
		//移動方向と位置フレームあたりの移動量で、到達するように。※ただし最終位置は正確な位置に。
		switch (cmd->getMoveType()) {
		case agtk::data::ObjectCommandObjectMoveData::kMoveWithDirection: {//方向を指定して移動
			mv = agtk::GetDirectionFromDegrees(cmd->getAngle());
			endPos = mv * cmd->getMoveDistance() + startPos;

			direction = _object->directionCorrection(mv);
			break; }
		case agtk::data::ObjectCommandObjectMoveData::kMoveToPosition: {//座標を指定して移動
			endPos = agtk::Scene::getPositionCocos2dFromScene(cocos2d::Vec2(cmd->getPosX(), cmd->getPosY()));
#ifdef FIX_ACT2_5237
			if (cmd->getMoveInDisplayCoordinates()) {
				followCameraMoving = cmd->getFollowCameraMoving();
				auto gameManager = GameManager::getInstance();
				auto projectData = gameManager->getProjectData();
				auto screenWidth = projectData->getScreenWidth();
				auto screenHeight = projectData->getScreenHeight();
				endPos = scene->getCamera()->getPosition() + cocos2d::Vec2(-screenWidth / 2, screenHeight / 2) + cocos2d::Vec2(cmd->getPosX(), -cmd->getPosY());
			}
#endif
			mv = (endPos - startPos).getNormalized();

			direction = _object->directionCorrection(mv);
			break; }
		case agtk::data::ObjectCommandObjectMoveData::kMoveToObjectCenter:
		case agtk::data::ObjectCommandObjectMoveData::kMoveToObjectOrigin:
		case agtk::data::ObjectCommandObjectMoveData::kMoveToObjectConnectionPoint: {//指定したオブジェクトへ移動
			agtk::Object *object2 = nullptr;
			auto projectPlayData = GameManager::getInstance()->getPlayData();
			if (cmd->getCenterObjectId() == agtk::data::ObjectCommandData::kSelfObject) {//自身のオブジェクト
				object2 = this->getTargetObjectInstanceId(_object->getInstanceId());
			}
			else if (cmd->getCenterObjectId() == -1) {//設定無し。
				return;
			}
			else {
				if (cmd->getCenterQualifierId() == agtk::data::ObjectCommandData::kQualifierSingle) {//単体
					auto singleInstanceData = projectPlayData->getVariableData(cmd->getCenterObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID);
					if (singleInstanceData) {
						object2 = this->getTargetObjectInstanceId((int)singleInstanceData->getValue());
					}
				}
				else if (cmd->getCenterQualifierId() >= 0) {
					auto objectList2 = this->getTargetObjectById(cmd->getCenterObjectId());
					cocos2d::Ref *ref2;
					CCARRAY_FOREACH(objectList2, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto obj = static_cast<agtk::Object *>(ref2);
#else
						auto obj = dynamic_cast<agtk::Object *>(ref2);
#endif
						if (obj->getInstanceId() == cmd->getCenterQualifierId()) {
							object2 = obj;
							break;
						}
					}
				}
				else {//全体
					CC_ASSERT(0);
				}
			}
			if (object2) {
				cocos2d::Vec2 position = object2->getCenterPosition();	//オブジェクトの中心をデフォルトに設定。
				if (cmd->getMoveType() == agtk::data::ObjectCommandObjectMoveData::kMoveToObjectCenter) {
				}
				else if (cmd->getMoveType() == agtk::data::ObjectCommandObjectMoveData::kMoveToObjectOrigin) {
					position = object2->getPosition();
				}
				else if (cmd->getMoveType() == agtk::data::ObjectCommandObjectMoveData::kMoveToObjectConnectionPoint) {
					int connectId = cmd->getConnectId();
					if (connectId >= 0) {
						//接続点を使用
						agtk::Vertex4 vertex4;
						if (object2->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, connectId, vertex4)) {
							//メニュー用オブジェクトかどうかで座標の返し方を変える
							if (object2->getInstanceId() > agtk::data::SceneData::kMenuSceneId) {
								//メニューシーンを取得、メニューシーンの高さを参照する
								position = cocos2d::Vec2(vertex4.addr()[0].x, GameManager::getInstance()->getProjectData()->getMenuSceneData()->getLimitAreaHeight() - vertex4.addr()[0].y);
							}
							else {
								position = Scene::getPositionSceneFromCocos2d(vertex4.addr()[0]);
							}
						}
					}
				}
				position += cocos2d::Vec2(cmd->getCenterAdjustX(), cmd->getCenterAdjustY());
				position += Object::getSceneLayerScrollDiff(object->getSceneLayer(), object2->getSceneLayer());
				endPos = agtk::Scene::getPositionCocos2dFromScene(position);
				mv = (endPos - startPos).getNormalized();

				direction = _object->directionCorrection(mv);
			}
			break; }
		default:CC_ASSERT(0);
		}

		//移動スピード指定
		float speed = 0.0f;
		float seconds = 1.0f;
		//対象オブジェクトの基本移動パラメータを使用
		if (cmd->getUseObjectParameter()) {
			bool movePosition = (cmd->getMoveType() != agtk::data::ObjectCommandObjectMoveData::kMoveWithDirection);
			auto objectMovement = object->getObjectMovement();
			objectMovement->startForceMoveParam(startPos, endPos, cmd->getChangeMoveSpeed(), cmd->getFinalGridMagnet(), movePosition, direction);
			seconds = objectMovement->getForceMove()->getMaxSeconds();
		}
		//指定位置までの移動が完了する時間を設定（秒）
		else {
			float length = (endPos - startPos).getLength();
			speed = length / ((float)cmd->getMoveDuration300() / 300.0f) / 60.0f;
			seconds = (float)cmd->getMoveDuration300() / 300;
			object->getObjectMovement()->startForceMoveTime(startPos, endPos, speed, cmd->getFinalGridMagnet());
// #AGTK-NX #AGTK-WIN
#ifdef USE_30FPS
			object->getObjectMovement()->getForceMove()->setWarpMoved(cmd->getMoveDuration300() <= 5);
#endif
		}
#ifdef FIX_ACT2_5237
		if (followCameraMoving) {
			auto objectMovement = object->getObjectMovement();
			auto forceMove = objectMovement->getForceMove();
			forceMove->setFollowCameraMoving(true);
			forceMove->setTargetPosInCamera(cocos2d::Vec2(cmd->getPosX(), cmd->getPosY()));
		}
#endif

		//デバッグ表示。
		if (DebugManager::getInstance()->getDebugForDevelopment()) {
			auto sceneLayer = object->getSceneLayer();

			auto sec = seconds < 2 ? seconds : 2;
			auto objCircle = PolygonShape::createCircle(endPos, 5);
			auto polyObj = primitiveManager->createPolygon(0, 0, objCircle->_vertices, objCircle->_segments, cocos2d::Color4F(0, 0, 1, 1), cocos2d::Color4F(0, 0, 1, 0.5));
			primitiveManager->setTimer(polyObj, sec);
			sceneLayer->addChild(polyObj);
			polyObj = primitiveManager->createLine(startPos.x, startPos.y, endPos.x, endPos.y, cocos2d::Color4F(0, 0, 1, 0.5));
			primitiveManager->setTimer(polyObj, sec);
			sceneLayer->addChild(polyObj);
		}

		//オブジェクト
		bool bSetupInertia = false;
		if (cmd->getFitDispDirToMoveDir()) {//表示方向を移動方向に合わせる
			float angle = agtk::GetDegreeFromVector(mv);
			int directionId = GetMoveDirectionId(angle);
			int actionId = object->getCurrentObjectAction()->getId();
			object->playAction(actionId, directionId);
			bSetupInertia = true;
		}
		if (cmd->getDispWhileMoving() == false) {//移動中オブジェクトを表示しない（表示:true,非表示:false）
			object->getObjectVisible()->start(false, seconds);
#ifdef FIX_ACT2_5401
#else
			object->getObjectVisible()->getObjectDamageInvincible()->start();
#endif
		}
		if (cmd->getStopActionWhileMoving()) {//移動中オブジェクトのアクションを停止。
			object->pauseAction(seconds);
		}
		if (cmd->getStopAnimWhileMoving()) {//移動中オブジェクトのアニメーションを停止。
			object->pauseAnimation(seconds);
		}
	}
}

bool ObjectAction::execActionObjectPushPull(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandObjectPushPullData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandObjectPushPullData *>(commandData);
#endif
	CC_ASSERT(cmd);

	//対象のオブジェクトを指定
	cocos2d::__Array *objectList = nullptr;
	switch (cmd->getTargettingType()) {
	case agtk::data::ObjectCommandObjectPushPullData::kTargettingByGroup://オブジェクトの種類で指定
		objectList = this->getTargetObjectByGroup(cmd->getTargetObjectGroup());
		break;
	case agtk::data::ObjectCommandObjectPushPullData::kTargettingById: {//オブジェクトで指定
		auto targetObjectId = cmd->getTargetObjectId();
		if (targetObjectId == agtk::data::ObjectCommandData::kOtherThanSelfObject) {//自身以外のオブジェクト
			objectList = cocos2d::__Array::create();
			auto scene = this->getScene();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
			auto objectAllList = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
			auto objectAllList = scene->getObjectAll(this->getSceneLayer()->getType());
#endif
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(objectAllList, ref) {
				auto object = dynamic_cast<agtk::Object *>(ref);
				if (object == _object) continue;
				objectList->addObject(object);
			}
		} else 
		switch (cmd->getTargetQualifierId()) {
		case agtk::data::ObjectCommandData::kQualifierSingle: {//単体
			objectList = cocos2d::__Array::create();
			auto projectPlayData = GameManager::getInstance()->getPlayData();
			auto singleInstanceData = projectPlayData->getVariableData(targetObjectId, agtk::data::kObjectSystemVariableSingleInstanceID);
			if (singleInstanceData) {
				auto object = this->getTargetObjectInstanceId((int)singleInstanceData->getValue());
				if (object) {
					objectList->addObject(object);
				}
			}
			break; }
		case agtk::data::ObjectCommandData::kQualifierWhole: {//全体
			objectList = this->getTargetObjectById(targetObjectId);
			break; }
		default: {
			if (cmd->getTargetQualifierId() >= 0) {
				objectList = cocos2d::__Array::create();
				auto objectList2 = this->getTargetObjectById(targetObjectId);
				cocos2d::Ref *ref2;
				CCARRAY_FOREACH(objectList2, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = static_cast<agtk::Object *>(ref2);
#else
					auto object = dynamic_cast<agtk::Object *>(ref2);
#endif
					if (object->getInstanceId() == cmd->getTargetQualifierId()) {
						objectList->addObject(object);
					}
				}
			} else {
				CC_ASSERT(0);
			}
			break; }
		}
		break; }
	case agtk::data::ObjectCommandObjectPushPullData::kTargettingTouched: {//このオブジェクトに触れたオブジェクト
		if (!_object->getCollisionWallWallChecked()) {
			_object->updateCollisionWallWallList();
		}
		objectList = cocos2d::__Array::create();
		cocos2d::Ref *ref;
		auto collisionWallWallList = _object->getCollisionWallWallList();
		CCARRAY_FOREACH(collisionWallWallList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			if (cmd->getExcludeObjectGroupBit() & object->getObjectData()->getGroupBit()) { continue; }
			objectList->addObject(object);
		}
		break; }
	case agtk::data::ObjectCommandObjectPushPullData::kTargettingLocked://このオブジェクトがロックしたオブジェクト
		objectList = this->getTargetObjectLocked();
		break;
	default:CC_ASSERT(0);
	}

	cocos2d::Vec2 centerPos = agtk::Scene::getPositionCocos2dFromScene(_object->getCenterPosition());
	if (cmd->getEffectRangeBaseConnect() && cmd->getEffectRangeBaseConnectId() != -1) {
		agtk::Vertex4 v4;
		if (!_object->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, cmd->getEffectRangeBaseConnectId(), v4)) {
			CC_ASSERT(0);
			return false;
		}
		centerPos = v4[0];
	}

	auto primitiveManager = PrimitiveManager::getInstance();

	//効果範囲の可視化。
	float duration = 0.5f;
	float areaAngle = agtk::Scene::getAngleMathFromScene(cmd->getAngle());
	if (cmd->getDirectionType()) {//このオブジェクトの表示方向
		int dispDirectionId = _object->getDispDirection();
		auto areaDirection = agtk::GetDirectionFromMoveDirectionId(dispDirectionId);
		areaAngle = agtk::GetDegreeFromVector(areaDirection);
		areaAngle = 90.0f - areaAngle;
	}

	std::set<int> *effectedObjectIdSet = nullptr;
	if (cmd->getOneTimeEffect()) {
		if (_pushPullCommandIdMapEffectedObjectIdSet.find(cmd->getId()) == _pushPullCommandIdMapEffectedObjectIdSet.end()) {
			_pushPullCommandIdMapEffectedObjectIdSet.insert(std::make_pair(cmd->getId(), std::set<int>()));
		}
		effectedObjectIdSet = &_pushPullCommandIdMapEffectedObjectIdSet[cmd->getId()];
	}
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object = static_cast<agtk::Object *>(ref);
#else
		auto object = dynamic_cast<agtk::Object *>(ref);
#endif

		if (_object == object) continue;
		if (effectedObjectIdSet && effectedObjectIdSet->find(object->getId()) != effectedObjectIdSet->end()) {
			//効果適用済み。
			continue;
		}

		//範囲の方向
		cocos2d::Vec2 areaDirection;
		switch (cmd->getDirectionType()) {
		case agtk::data::ObjectCommandObjectPushPullData::kDirectionAngle: {//角度を指定
			areaDirection = agtk::GetDirectionFromDegrees(cmd->getAngle());
			break; }
		case agtk::data::ObjectCommandObjectPushPullData::kDirectionObjectDisp: {//このオブジェクトの表示方向
			int dispDirectionId = _object->getDispDirection();
			areaDirection = agtk::GetDirectionFromMoveDirectionId(dispDirectionId);
			break; }
		case agtk::data::ObjectCommandObjectPushPullData::kDirectionObjectConnect: {//このオブジェクトの接続点
			agtk::Vertex4 connectPosition;
			if (cmd->getConnectId() < 0 || _object->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, cmd->getConnectId(), connectPosition) == false) {
				return false;
			}
			auto myself = connectPosition.p0;
			auto target = agtk::Scene::getPositionSceneFromCocos2d(object->getCenterPosition());
			areaDirection = (target - myself).getNormalized();
			break; }
		case agtk::data::ObjectCommandObjectPushPullData::kDirectionObject: {//このオブジェクトの方向
			auto myself = agtk::Scene::getPositionCocos2dFromScene(_object->getCenterPosition());
			auto target = agtk::Scene::getPositionCocos2dFromScene(object->getCenterPosition());
			areaDirection = (target - myself).getNormalized();
			break; }
		default:CC_ASSERT(0);
		}

		//効果の方向
		cocos2d::Vec2 effectDirection;
		cocos2d::Vec2 centerPosition ;
		switch (cmd->getEffectDirectionType()) {
		case agtk::data::ObjectCommandObjectPushPullData::kEffectDirectionAngle: {//角度指定
			effectDirection = agtk::GetDirectionFromDegrees(cmd->getEffectDirection());
			break; }
		case agtk::data::ObjectCommandObjectPushPullData::kEffectDirectionObjectDisp: {//このオブジェクトの表示方向
			int dispDirectionId = _object->getDispDirection();
			effectDirection = agtk::GetDirectionFromMoveDirectionId(dispDirectionId);
			break; }
		case agtk::data::ObjectCommandObjectPushPullData::kEffectDirectionObjectConnect: {//このオブジェクトの接続点
			agtk::Vertex4 connectPosition;
			if (cmd->getConnectId() < 0 || _object->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, cmd->getConnectId(), connectPosition) == false) {
				return false;
			}
			auto target = agtk::Scene::getPositionSceneFromCocos2d(object->getCenterPosition());
			effectDirection = (target - connectPosition.p0).getNormalized();
			break; }
		default:CC_ASSERT(0);
		}

		//タイル判定、当たり判定
		bool ret = false;
		float maxLength = 0.0f;
		if (cmd->getRectangle()) {//矩形
			//効果範囲
			maxLength = cmd->getRectangleDistance();
			cocos2d::Vec2 centerPoint = cocos2d::Vec2(0, cmd->getRectangleHeight() * 0.5f);
			auto rectangle = PolygonShape::createRectangleCenterPoint(centerPos, cocos2d::Size(maxLength, cmd->getRectangleHeight()), centerPoint, areaAngle);//矩形
			//オブジェクト範囲
			auto objRect = object->getRect();
			objRect = agtk::Scene::getRectCocos2dFromScene(objRect);
			float effectAngle = agtk::GetDegreeFromVector(effectDirection);
			auto objRectangle = PolygonShape::createRectangle(objRect, 0.0f);
			//あたり判定。
			bool bCheck = PolygonShape::intersectsPolygonShape(rectangle, objRectangle);
			if (bCheck == false) {
				goto lSkip;
			}

			//あたり判定表示。
			if (DebugManager::getInstance()->getDebugForDevelopment()) {
				auto polyObj = primitiveManager->createPolygon(0, 0, objRectangle->_vertices, objRectangle->_segments, cocos2d::Color4F(0.7f, 0.0f, 0.7f, 1));
				primitiveManager->setTimer(polyObj, duration);
				_object->addChild(polyObj, _object->getLocalZOrder() + 1);
			}
			ret = true;
		}
		//円形
		else {
			maxLength = cmd->getCircleDistance();
			auto fan = PolygonShape::createFan(centerPos, cmd->getCircleDistance(), areaAngle, cmd->getArcAngle());
			//オブジェクト範囲
			auto objRect = object->getRect();
			objRect = agtk::Scene::getRectCocos2dFromScene(objRect);
			float effectAngle = agtk::GetDegreeFromVector(effectDirection);
			auto objRectangle = PolygonShape::createRectangle(objRect, 0.0f);
			//あたり判定。
			bool bCheck = PolygonShape::intersectsFunPolygonShape(fan, objRectangle);
			if (bCheck == false) {
				goto lSkip;
			}

			//あたり判定表示。
			if (DebugManager::getInstance()->getDebugForDevelopment()) {
				auto polyObj = primitiveManager->createPolygon(0, 0, objRectangle->_vertices, objRectangle->_segments, cocos2d::Color4F(0.7f, 0.0f, 0.7f, 1));
				primitiveManager->setTimer(polyObj, duration);
				_object->addChild(polyObj, _object->getLocalZOrder() + 1);
			}
			ret = true;
		}

	lSkip:;
		if (ret == false) {
			continue;
		}
		auto objectMovement = object->getObjectMovement();
		if (cmd->getOneTimeEffect() == false && objectMovement->getForceMove()->isContinueAction() == false) {
			continue;
		}
		if (effectedObjectIdSet){
			//効果を受けたことを記録。
			effectedObjectIdSet->insert(object->getId());
		}

		auto objCenterPos = agtk::Scene::getPositionCocos2dFromScene(object->getCenterPosition());
		float length = (centerPos - objCenterPos).getLength();
		if (length > maxLength) length = maxLength;

		//効果の範囲
		float effectValue = cmd->getEffectValue();
		if (cmd->getDistanceEffect()) {
			effectValue = effectValue * (AGTK_LINEAR_INTERPOLATE(cmd->getNearValue(), cmd->getFarValue(), maxLength, length) * 0.01f);
		}
		//効果方向と力の表示。
		if (DebugManager::getInstance()->getDebugForDevelopment()) {
			cocos2d::Vec2 pos2 = centerPos + effectDirection * cmd->getEffectValue();
			auto line = primitiveManager->createLine(centerPos.x, centerPos.y, pos2.x, pos2.y, cocos2d::Color4F::YELLOW);
			primitiveManager->setTimer(line, duration);
			_object->addChild(line);
		}

		objectMovement->startForceMovePushPull(objCenterPos, objCenterPos + effectDirection * effectValue);
	}

	if (DebugManager::getInstance()->getDebugForDevelopment()) {
		if (cmd->getRectangle()) {//矩形
			cocos2d::Vec2 centerPoint = cocos2d::Vec2(0, cmd->getRectangleHeight() * 0.5f);
			auto rectangle = PolygonShape::createRectangleCenterPoint(centerPos, cocos2d::Size(cmd->getRectangleDistance(), cmd->getRectangleHeight()), centerPoint, areaAngle);
			auto polyArea = primitiveManager->createPolygon(0, 0, rectangle->_vertices, rectangle->_segments, cocos2d::Color4F(1, 1, 0, 1), cocos2d::Color4F(1, 1, 0, 0.3f));
			primitiveManager->setTimer(polyArea, duration);
			_object->addChild(polyArea);
		}
		else {//円形
			auto fan = PolygonShape::createFan(centerPos, cmd->getCircleDistance(), areaAngle, cmd->getArcAngle());
			auto polyArea = primitiveManager->createPolygon(0, 0, fan->_vertices, fan->_segments, cocos2d::Color4F(1, 1, 0, 1), cocos2d::Color4F(1, 1, 0, 0.3f));
			primitiveManager->setTimer(polyArea, duration);
			_object->addChild(polyArea);
		}
	}

	return true;
}

void ObjectAction::execActionLayerMove(agtk::data::ObjectCommandData *commandData)
{
	// ※レイヤー移動でコリジョンコンポーネントがおかしくなるので入れ替える
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandLayerMoveData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandLayerMoveData *>(commandData);
#endif
	CC_ASSERT(cmd);
	if (cmd->getLayerIndex() == -1) {
		//設定無し。
		return;
	}
	auto scene = GameManager::getInstance()->getCurrentScene();
	int nowLayerId = _object->getLayerId();
	int nextLayerId = cmd->getLayerIndex() + 1;
	if (nowLayerId == nextLayerId) {
		//same layer.
		return;
	}
	auto nextSceneLayer = scene->getSceneLayer(nextLayerId);
	if (nextSceneLayer == nullptr) {
		//指定レイヤーが無い場合。
		return;
	}
	auto nowSceneLayer = _object->getSceneLayer();

	// ACT2-5130 追従時でターゲットが同じ場合はカメラ更新フラグを立てる
	bool isCameraUpdate = false;
	auto camera = scene->getCamera();
	if (camera->getTargetType() == agtk::Camera::CameraTargetType::kCameraTargetObject) {
		if (_object == camera->getTargetObject()) {
			isCameraUpdate = true;
		}
	}

	// ACT2-6062 視野の生成の有無
	bool isViewportLightObject = false;
	auto viewportLightSceneLayer = scene->getViewportLight()->getViewportLightSceneLayer(_object->getLayerId());
	auto viewportLightObject = viewportLightSceneLayer->getViewportLightObject(_object);
	if (viewportLightObject != nullptr) {
		isViewportLightObject = true;
	}
	
	auto oldInstanceId = _object->getInstanceId();
	//detach now layer.
	ParticleManager::getInstance()->addRemoveParticlesOfFollowed(_object, agtk::data::ObjectCommandParticleRemoveData::ALL_PARTICLE, false);
	_object->setRemoveLayerMoveFlag(true);
	_object->removeSelf(false, true, agtk::Object::kRemoveOptionKeepChildObjectBit | agtk::Object::kRemoveOptionKeepConnectObjectBit | agtk::Object::kRemoveOptionKeepOwnParentObjectBit);
	_object->setRemoveLayerMoveFlag(false);

	//attach next layer.
	nextSceneLayer->getObjectList()->addObject(_object);
	_object->setLayerId(nextLayerId);
	_object->setSceneLayer(nextSceneLayer);

	nextSceneLayer->addCollisionDetaction(_object);
	_object->setupPhysicsBody(true);
	_object->setPhysicsBitMask(nextLayerId, scene->getSceneData()->getId());
	// オブジェクトに紐付いた物理オブジェクトの生成
	nextSceneLayer->createPhysicsObjectWithObject(_object);
	
	// ACT2-6062 視野の生成
	if (isViewportLightObject) {
		auto newViewportLightSceneLayer = scene->getViewportLight()->getViewportLightSceneLayer(_object->getLayerId());
		auto viewportLightObject = ViewportLightObject::create(_object, scene->getViewportLight(), nextSceneLayer);
		newViewportLightSceneLayer->getViewportLightObjectList()->addObject(viewportLightObject);
	}

	_object->addPosition(nowSceneLayer->getPosition() - nextSceneLayer->getPosition());
// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_1
	nextSceneLayer->addObject(_object);
#endif

	ParticleManager::getInstance()->addRemoveParticlesOfFollowed(_object, agtk::data::ObjectCommandParticleRemoveData::ALL_PARTICLE, true);

	// インスタンスIDを再設定。
	auto playObjectData = _object->getPlayObjectData();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	playObjectData->setInstanceId(scene->getObjectInstanceId(_object));
	nextSceneLayer->addObject(_object);
#else
	playObjectData->setInstanceId(scene->getObjectInstanceId(_object));
#endif

	// 子のインスタンスIDを再設定
	auto  childrenList = _object->getChildrenObjectList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(childrenList, ref) {
		auto connectObj = dynamic_cast<agtk::ConnectObject *>(ref);
		if (connectObj) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto connectObj = static_cast<agtk::ConnectObject *>(ref);
#else
			auto connectObj = dynamic_cast<agtk::ConnectObject *>(ref);
#endif
			auto connectObjLayerId = connectObj->getLayerId();
			if (connectObjLayerId == nextLayerId) {
				// レイヤー移動先に接続オブジェクトがある場合は、表示の優先度を再設定
				auto settingData = connectObj->getObjectConnectSettingData();
				_object->addConnectObjectDispPriority(connectObj, settingData->getLowerPriority());
			}
			connectObj->getPlayObjectData()->setParentObjectInstanceId(_object->getPlayObjectData()->getInstanceId());
		}
		else {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto obj = static_cast<agtk::Object *>(ref);
#else
			auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
			obj->getPlayObjectData()->setParentObjectInstanceId(_object->getPlayObjectData()->getInstanceId());
		}
	}

	// カメラ更新が必要な場合、リセット時にターゲットがnullptrになるので再設定し更新する
	if (isCameraUpdate) {
		camera->setTargetObject(_object);
		camera->update(0);
	}

	//ロックしていた場合の再設定）
	auto objectLockedList = scene->getObjectAllLocked(oldInstanceId, agtk::SceneLayer::kTypeAll);
	if (objectLockedList->count() > 0) {
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(objectLockedList, ref) {
			auto obj = dynamic_cast<agtk::Object *>(ref);
			auto playObjectData = obj->getPlayObjectData();
			if (playObjectData->isLocked(oldInstanceId)) {
				playObjectData->removeLocking(oldInstanceId);
				playObjectData->addLocking(_object->getInstanceId());
			}
		}
	}
}

/**
 * 攻撃の設定
 * @param	commandData	コマンドデータ
 */
void ObjectAction::execActionAttackSetting(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandAttackSettingData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandAttackSettingData *>(commandData);
#endif
	CC_ASSERT(cmd);

	auto playData = _object->getPlayObjectData();
	auto objectData = _object->getObjectData();

	// -----------------------------------------
	// 攻撃力の変更
	// -----------------------------------------
	_object->setVariableAttackRate(cmd->getAttackChange());

	// -----------------------------------------
	// 攻撃判定の変更
	// -----------------------------------------
	// 全プレイヤーに当たる攻撃判定を変更
	if (cmd->getHitObjectFlag()) {
		playData->setHitObjectGroupBit( cmd->getObjectGroupBit() );
	}

	// タイルに当たる攻撃判定を変更
	playData->setHitTileGroupBit(cmd->getHitTileFlag() ? cmd->getTileGroupBit() : 0);

	// -----------------------------------------
	// 攻撃属性値の変更
	// -----------------------------------------
	auto attributeType = cmd->getAttributeType();

	// 属性値の設定がある場合
	if (attributeType != agtk::data::ObjectCommandAttackSettingData::AttributeType::kNone) {
		// 攻撃属性値を変更
		auto attributeValue = attributeType == agtk::data::ObjectCommandAttackSettingData::AttributeType::kPreset ? cmd->getAttributePresetId() : cmd->getAttributeValue();
		auto variableData = playData->getVariableData(agtk::data::EnumObjectSystemVariable::kObjectSystemVariableAttackAttribute);
		variableData->setValue(attributeValue);
	}
}

void ObjectAction::execActionBulletFire(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandBulletFireData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandBulletFireData *>(commandData);
#endif
	CC_ASSERT(cmd);
	if(cmd->getBulletId() < 0) {
		//設定無し。
		return;
	}
	BulletManager::getInstance()->createBullet(_object, cmd->getBulletId(), cmd->getConnectId());
}

void ObjectAction::execActionDisappear(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandDisappearData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandDisappearData *>(commandData);
#endif
	CC_ASSERT(cmd);
	//オブジェクト消滅
	CC_ASSERT(_object);
	//_object->removeSelf();

	// このタイミングで消滅処理を行うと
	// オブジェクト同士の接触によるアクション切り替えなどに支障をきたすので
	// lateUpdateで消滅を行う
	_object->setLateRemove(true);
}

/**
* 消滅状態のオブジェクトを復活させる
* @param	commandData	コマンドデータ
*/
void ObjectAction::execActionDisappearObjectRecover(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandDisappearObjectRecoverData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandDisappearObjectRecoverData *>(commandData);
#endif
	CC_ASSERT(cmd);
	//! 2018.03.13 オブジェクトを指定する形に機能変更
	auto scene = this->getScene();
	int targetObjectId = cmd->getObjectId();

	// 「設定無し」の場合は何もしない
	if (targetObjectId == TARGET_NONE) {
		return;
	}

	// シーンレイヤー毎に復活を行う
	auto sceneLayerList = scene->getSceneLayerList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(sceneLayerList, el) {
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
		CC_ASSERT(sceneLayer);

		if (sceneLayer != nullptr) {
			sceneLayer->reappearObjectByAction(targetObjectId);
		}
	}
}

void ObjectAction::execActionDisable(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandDisableData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandDisableData *>(commandData);
#endif
	CC_ASSERT(cmd);
	//無効
	_object->setDisabled(true);
	//非表示にする。
	_object->setVisible(false);	
	// パーティクルを削除
	_object->removeParticles();
}

/**
* 無効状態のオブジェクトを有効にする
* @param	commandData	コマンドデータ
*/
void ObjectAction::execActionDisableObjectEnable(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandDisableObjectEnableData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandDisableObjectEnableData *>(commandData);
#endif
	CC_ASSERT(cmd);

	//! 2018.03.13 オブジェクトを指定する形に機能変更
	auto scene = this->getScene();
	int targetObjectId = cmd->getObjectId();

	// 「設定無し」の場合は何もしない
	if (targetObjectId == TARGET_NONE) {
		return;
	}

	// シーンレイヤー毎に有効化を行う
	auto sceneLayerList = scene->getSceneLayerList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(sceneLayerList, el) {
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
		CC_ASSERT(sceneLayer);

		if (sceneLayer != nullptr) {
			sceneLayer->enableObjectByAction(targetObjectId);
		}
	}
}

void ObjectAction::execActionObjectFilterEffect(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandObjectFilterEffectData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandObjectFilterEffectData *>(commandData);
#endif
	CC_ASSERT(cmd);
	auto player = _object->getPlayer();

	//オブジェクトのフィルター効果コマンドを保持する。
	this->setObjectFilterEffectCommandData(commandData);

	// モーションが「設定無し」等でプレイヤーが存在しない場合
	if (player == nullptr) {
		// 何もしない
		return;
	}

	auto filterEffect = cmd->getFilterEffect();
	float seconds = (float)filterEffect->getDuration300() / 300.0f;
	switch (filterEffect->getEffectType()) {
	case agtk::data::FilterEffect::kEffectNoise://ノイズ
		player->setShader(agtk::Shader::kShaderNoisy, (float)filterEffect->getNoise() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectMosaic://モザイク
		player->setShader(agtk::Shader::kShaderMosaic, (float)filterEffect->getMosaic() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectMonochrome://モノクロ
		player->setShader(agtk::Shader::kShaderColorGray, (float)filterEffect->getMonochrome() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectSepia://セピア
		player->setShader(agtk::Shader::kShaderColorSepia, (float)filterEffect->getSepia() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectNegaPosiReverse://ネガ反転
		player->setShader(agtk::Shader::kShaderColorNega, (float)filterEffect->getNegaPosiReverse() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectDefocus://ぼかし
		player->setShader(agtk::Shader::kShaderBlur, (float)filterEffect->getDefocus() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectChromaticAberration://色収差
		player->setShader(agtk::Shader::kShaderColorChromaticAberration, (float)filterEffect->getChromaticAberration() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectDarkness://暗闇
		player->setShader(agtk::Shader::kShaderColorDark, (float)filterEffect->getDarkness() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectTransparency://透明
		player->setShader(agtk::Shader::kShaderTransparency, (float)filterEffect->getTransparency() / 100.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectBlink://点滅
		_object->getObjectVisible()->startBlink(filterEffect->getBlinkInterval300() / 300.0f, seconds);
		break;
	case agtk::data::FilterEffect::kEffectDispImage://画像表示
		if (cmd->getFilterEffect()->getImageId() >= 0) {
			player->setExecActionSprite(cmd->getFilterEffect()->getImageId(), 255 * (100 - filterEffect->getImageTransparency()) / 100, seconds);
		}
		break;
	case agtk::data::FilterEffect::kEffectFillColor: {//指定色で塗る
		player->setShader(agtk::Shader::kShaderColorRgba, 1.0f, seconds);
		player->getShader(agtk::Shader::kShaderColorRgba)->setShaderRgbaColor(cocos2d::Color4B(
			filterEffect->getFillR(),
			filterEffect->getFillG(),
			filterEffect->getFillB(),
			filterEffect->getFillA()
		));
		break; }
	default:CC_ASSERT(0);
	}
}

void ObjectAction::execActionObjectFilterEffect()
{
	auto commandData = this->getObjectFilterEffectCommandData();
	if (commandData == nullptr) {
		return;
	}
	this->execActionObjectFilterEffect(commandData);
}

void ObjectAction::execActionObjectFilterEffectRemove(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandObjectFilterEffectRemoveData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandObjectFilterEffectRemoveData *>(commandData);
#endif
	CC_ASSERT(cmd);
	auto player = _object->getPlayer();

	//オブジェクトのフィルター効果コマンドを破棄する。
	this->setObjectFilterEffectCommandData(nullptr);

	// モーションが「設定無し」等でプレイヤーが存在しない場合
	if (player == nullptr) {
		// 何もしない
		return;
	}

	unsigned int bit = cmd->getRemoveBit();
	float seconds = (float)cmd->getDuration300() / 300.0f;

	if (bit & (1 << agtk::data::FilterEffect::kEffectNoise)) {//ノイズ
		player->removeShader(agtk::Shader::kShaderNoisy, seconds);
	}
	if (bit & (1 << agtk::data::FilterEffect::kEffectMosaic)) {//モザイク
		player->removeShader(agtk::Shader::kShaderMosaic, seconds);
	}
	if (bit & (1 << agtk::data::FilterEffect::kEffectMonochrome)) {//モノクロ
		player->removeShader(agtk::Shader::kShaderColorGray, seconds);
	}
	if (bit & (1 << agtk::data::FilterEffect::kEffectSepia)) {//セピア
		player->removeShader(agtk::Shader::kShaderColorSepia, seconds);
	}
	if (bit & (1 << agtk::data::FilterEffect::kEffectNegaPosiReverse)) {//ネガ反転
		player->removeShader(agtk::Shader::kShaderColorNega, seconds);
	}
	if (bit & (1 << agtk::data::FilterEffect::kEffectDefocus)) {//ぼかし
		player->removeShader(agtk::Shader::kShaderBlur, seconds);
	}
	if (bit & (1 << agtk::data::FilterEffect::kEffectChromaticAberration)) {//色収差
		player->removeShader(agtk::Shader::kShaderColorChromaticAberration, seconds);
	}
	if (bit & (1 << agtk::data::FilterEffect::kEffectDarkness)) {//暗闇
		player->removeShader(agtk::Shader::kShaderColorDark, seconds);
	}
	if (bit & (1 << agtk::data::FilterEffect::kEffectTransparency)) {//透明
		player->removeShader(agtk::Shader::kShaderTransparency, seconds);
	}
	if (bit & (1 << agtk::data::FilterEffect::kEffectBlink)) {//点滅
		_object->getObjectVisible()->endBlink(seconds);
	}
	if (bit & (1 << agtk::data::FilterEffect::kEffectDispImage)) {//画像表示
		player->removeExecActionSprite(seconds);
	}
	if (bit & (1 << agtk::data::FilterEffect::kEffectFillColor)) {//指定色で塗る
		player->removeShader(agtk::Shader::kShaderColorRgba, seconds);
	}
}

void ObjectAction::execActionSceneEffect(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandSceneEffectData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandSceneEffectData *>(commandData);
#endif
	CC_ASSERT(cmd);
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto projectData = GameManager::getInstance()->getProjectData();

	std::function<void(int)> setFilterEffect = [&](int layerId) {
		auto filterEffect = cmd->getFilterEffect();
		float seconds = (float)filterEffect->getDuration300() / 300.0f;
		switch (filterEffect->getEffectType()) {
		case agtk::data::FilterEffect::kEffectNoise: {//ノイズ
			scene->setShader(layerId, agtk::Shader::kShaderNoisy, (float)filterEffect->getNoise() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectMosaic: {//モザイク
			scene->setShader(layerId, agtk::Shader::kShaderMosaic, (float)filterEffect->getMosaic() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectMonochrome: {//モノクロ
			scene->setShader(layerId, agtk::Shader::kShaderColorGray, (float)filterEffect->getMonochrome() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectSepia: {//セピア
			scene->setShader(layerId, agtk::Shader::kShaderColorSepia, (float)filterEffect->getSepia() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectNegaPosiReverse: {//ネガ反転
			scene->setShader(layerId, agtk::Shader::kShaderColorNega, (float)filterEffect->getNegaPosiReverse() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectDefocus: {//ぼかし
			scene->setShader(layerId, agtk::Shader::kShaderBlur, (float)filterEffect->getDefocus() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectChromaticAberration: {//色収差
			scene->setShader(layerId, agtk::Shader::kShaderColorChromaticAberration, (float)filterEffect->getChromaticAberration() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectDarkness: {//暗闇
			 //背景 or 最前面 or 最前面(メニュー含む)
			if (layerId == agtk::data::SceneData::kBackgroundLayerId ||
				layerId == agtk::data::SceneData::kTopMostLayerId ||
				layerId == agtk::data::SceneData::kTopMostWithMenuLayerId) {
				scene->setShader(layerId, agtk::Shader::kShaderColorDark, (float)filterEffect->getDarkness() / 100.0f, seconds);
			}
			else {//レイヤー
				scene->setShader(layerId, agtk::Shader::kShaderColorDarkMask, (float)filterEffect->getDarkness() / 100.0f, seconds);
			}
			break; }
		case agtk::data::FilterEffect::kEffectTransparency: {//透明
			scene->setShader(layerId, agtk::Shader::kShaderTransparency, (float)filterEffect->getTransparency() / 100.0f, seconds);
			break; }
		case agtk::data::FilterEffect::kEffectBlink: {//点滅
			//点滅はオブジェクトのみに実装。
			break; }
		case agtk::data::FilterEffect::kEffectDispImage: {//画像表示

			std::function<void(agtk::data::FilterEffect *, agtk::RenderTextureCtrl *)> setImageFilterEffect = [&](agtk::data::FilterEffect *filterEffect, agtk::RenderTextureCtrl *renderTextureCtrl) {
				auto projectData = GameManager::getInstance()->getProjectData();
				auto imageData = projectData->getImageData(filterEffect->getImageId());
				auto sceneData = scene->getSceneData();
				cocos2d::Size texSizeDef;

				// create texture2d
				auto texture2d = CreateTexture2D(imageData->getFilename(), (filterEffect->getImagePlacement() == agtk::data::FilterEffect::kPlacementTiling), nullptr, &texSizeDef.width, &texSizeDef.height);
				if (filterEffect->getImagePlacement() == agtk::data::FilterEffect::kPlacementTiling) {
					//タイリング
					Texture2D::TexParams tRepeatParams;
					tRepeatParams.magFilter = GL_LINEAR;
					tRepeatParams.minFilter = GL_LINEAR;
					tRepeatParams.wrapS = GL_REPEAT;
					tRepeatParams.wrapT = GL_REPEAT;
					texture2d->setTexParameters(tRepeatParams);
				}
				else {
					texture2d->setAliasTexParameters();
				}

				auto sceneSize = projectData->getSceneSize(sceneData);
				auto shader = renderTextureCtrl->addShader(agtk::Shader::kShaderImage, sceneSize, (float)(100.0f - filterEffect->getImageTransparency()) / 100.0f, seconds);
				shader->setMaskTexture(texture2d);
				// set imgPlacement
				int imgPlacement = filterEffect->getImagePlacement();
				auto programState = shader->getProgramState();
				programState->setUniformInt("imgPlacement", imgPlacement);
				// set resolution
				float width = sceneSize.width;
				float height = sceneSize.height;
				programState->setUniformVec2("resolution", cocos2d::Vec2(width, height));
				// set imgResolution
				auto imgResolution = (imgPlacement == 2) ? Vec2(texture2d->getContentSize().width, texture2d->getContentSize().height) : Vec2(width, height);
				programState->setUniformVec2("imgResolution", imgResolution);
				// set imgSizeRate
				auto imgSizeRate = cocos2d::Vec2(texSizeDef.width / texture2d->getContentSize().width, texSizeDef.height / texture2d->getContentSize().height);
				programState->setUniformVec2("imgSizeRate", imgSizeRate);
				// set sxy
				float imgSourceWidth = texSizeDef.width;
				float imgSourceHeight = texSizeDef.height;
				auto sxy = (imgPlacement == 3) ? ((width / imgSourceWidth <= height / imgSourceHeight) ? Vec2(1, height / imgSourceHeight * imgSourceWidth / width) : Vec2(width / imgSourceWidth * imgSourceHeight / height, 1)) : (imgSourceWidth > 0 && imgSourceHeight > 0 ? Vec2(width / imgSourceWidth, height / imgSourceHeight) : Vec2(1, 1));
				programState->setUniformVec2("sxy", sxy);
				// set imgXy
				auto imgXy = Vec2((1 - sxy.x) / 2, (1 - sxy.y) / 2);
				programState->setUniformVec2("imgXy", imgXy);
				// set FilterEffect
				shader->setUserData(filterEffect);
			};

			cocos2d::Vec2 pos = Scene::getPositionCocos2dFromScene(_object->getCenterPosition());
			auto sceneData = scene->getSceneData();
			if (layerId == agtk::data::SceneData::kBackgroundLayerId) {//背景
				auto sceneBackground = scene->getSceneBackground();
				//配置方法:「このオブジェクトの中心」の場合
				if (filterEffect->getImagePlacement() == agtk::data::FilterEffect::kPlacementObjectCenter) {
					sceneBackground->createSceneSprite(filterEffect->getImageId(), 255 * (100 - filterEffect->getImageTransparency()) / 100, pos, seconds);
				}
				else {
					auto renderTextureCtrl = sceneBackground->getRenderTexture();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
					if (renderTextureCtrl == false) {
#else
#endif
						sceneBackground->createObjectCommandRenderTexture();
						renderTextureCtrl = sceneBackground->getRenderTexture();
					}
					setImageFilterEffect(filterEffect, renderTextureCtrl);
				}
			}
			else if (layerId == agtk::data::SceneData::kTopMostLayerId) {//最前面
				auto sceneTopMost = scene->getSceneTopMost();
				//配置方法:「このオブジェクトの中心」の場合
				if (filterEffect->getImagePlacement() == agtk::data::FilterEffect::kPlacementObjectCenter) {
					sceneTopMost->createSceneSprite(filterEffect->getImageId(), 255 * (100 - filterEffect->getImageTransparency()) / 100, pos, seconds);
				}
				else {
					auto renderTextureCtrl = sceneTopMost->getRenderTexture();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
					if (renderTextureCtrl == false) {
#else
#endif
						sceneTopMost->createObjectCommandRenderTexture();
						renderTextureCtrl = sceneTopMost->getRenderTexture();
					}
					setImageFilterEffect(filterEffect, renderTextureCtrl);
				}
			}
			else if (layerId == agtk::data::SceneData::kTopMostWithMenuLayerId) {//最前面(メニュー含む)
				auto sceneTopMost = scene->getSceneTopMost();
				//配置方法:「このオブジェクトの中心」の場合
				if (filterEffect->getImagePlacement() == agtk::data::FilterEffect::kPlacementObjectCenter) {
					sceneTopMost->createSceneSprite(filterEffect->getImageId(), 255 * (100 - filterEffect->getImageTransparency()) / 100, pos, seconds);
				}
				else {
					auto withMenuRenderTextureCtrl = sceneTopMost->getWithMenuRenderTexture();
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
					if (withMenuRenderTextureCtrl == false) {
#else
#endif
						sceneTopMost->createObjectCommandWithMenuRenderTexture();
						withMenuRenderTextureCtrl = sceneTopMost->getWithMenuRenderTexture();
					}
					setImageFilterEffect(filterEffect, withMenuRenderTextureCtrl);
				}
			}
			else {
				//配置方法:「このオブジェクトの中心」の場合
				if (filterEffect->getImagePlacement() == agtk::data::FilterEffect::kPlacementObjectCenter) {
					auto sceneLayer = scene->getSceneLayer(layerId);
					if (sceneLayer) {
						if (filterEffect->getImageId() >= 0) {
							sceneLayer->createSceneSprite(filterEffect->getImageId(), 255 * (100 - filterEffect->getImageTransparency()) / 100, pos, seconds);
						}
					}
				}
				//配置方法：「このオブジェクトの中心」以外
				else {
					auto sceneLayer = scene->getSceneLayer(layerId);
					if (sceneLayer) {
						auto rtCtrl = sceneLayer->getRenderTexture();
						if (rtCtrl) {
							setImageFilterEffect(filterEffect, rtCtrl);
						}
					}
				}
			}
			break; }
		case agtk::data::FilterEffect::kEffectFillColor: {//指定色で塗る
			auto color = cocos2d::Color4B(filterEffect->getFillR(), filterEffect->getFillG(), filterEffect->getFillB(), filterEffect->getFillA());
			scene->setShader(layerId, agtk::Shader::kShaderColorRgba, 1.0f, seconds);
			// 背景
			if (layerId == agtk::data::SceneData::kBackgroundLayerId) {
				auto sceneBackground = scene->getSceneBackground();
				if (sceneBackground->getRenderTexture()) {
					auto shader = sceneBackground->getShader(agtk::Shader::kShaderColorRgba);
					shader->setShaderRgbaColor(color);
				}
			}
			//最前面
			else if (layerId == agtk::data::SceneData::kTopMostLayerId) {
				auto sceneTopMost = scene->getSceneTopMost();
				if (sceneTopMost->getRenderTexture()) {
					auto shader = sceneTopMost->getShader(agtk::Shader::kShaderColorRgba);
					shader->setShaderRgbaColor(color);
				}
			}
			//最前面(メニュー含む)
			else if (layerId == agtk::data::SceneData::kTopMostWithMenuLayerId) {
				auto sceneTopMost = scene->getSceneTopMost();
				if (sceneTopMost->getWithMenuRenderTexture()) {
					auto shader = sceneTopMost->getWithMenuShader(agtk::Shader::kShaderColorRgba);
					shader->setShaderRgbaColor(color);
				}
			}
			else {
				auto sceneLayer = scene->getSceneLayer(layerId);
				if (sceneLayer) {
					if (sceneLayer->getRenderTexture()) {
						auto shader = sceneLayer->getShader(agtk::Shader::kShaderColorRgba);
						shader->setShaderRgbaColor(color);
					}
				}
			}
			break; }
		default:CC_ASSERT(0);
		}
	};

	int layerId = cmd->getLayerIndex();
	if (layerId == agtk::data::SceneFilterEffectData::kLayerIndexAllSceneLayers) {//全てのシーンレイヤー
		// 背景
		setFilterEffect(agtk::data::SceneData::kBackgroundLayerId);
		//レイヤー
		auto sceneLayerList = scene->getSceneLayerList();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
			auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
			setFilterEffect(sceneLayer->getLayerId());
		}
		return;
	}
	else if (layerId == agtk::data::SceneFilterEffectData::kLayerIndexBackground) {//背景
		layerId = agtk::data::SceneData::kBackgroundLayerId;
	}
	else if (layerId == agtk::data::SceneFilterEffectData::kLayerIndexTopMost) {//最前面
		layerId = agtk::data::SceneData::kTopMostLayerId;
	}
	else if (layerId == agtk::data::SceneFilterEffectData::kLayerIndexTopMostWithMenu) {//最前面(メニュー含む)
		layerId = agtk::data::SceneData::kTopMostWithMenuLayerId;
	}
	else {
		layerId += 1;
	}

	setFilterEffect(layerId);
}

void ObjectAction::execActionSceneEffectRemove(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandSceneEffectRemoveData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandSceneEffectRemoveData *>(commandData);
#endif
	CC_ASSERT(cmd);
	auto scene = GameManager::getInstance()->getCurrentScene();
	float seconds = (float)cmd->getDuration300() / 300.0f;

	std::function<void(int)> removeFilterEffect = [&](int layerId) {
		unsigned int bit = cmd->getRemoveBit();
		if (bit & (1 << agtk::data::FilterEffect::kEffectNoise)) {//ノイズ
			scene->removeShader(layerId, agtk::Shader::kShaderNoisy, seconds);
		}
		if (bit & (1 << agtk::data::FilterEffect::kEffectMosaic)) {//モザイク
			scene->removeShader(layerId, agtk::Shader::kShaderMosaic, seconds);
		}
		if (bit & (1 << agtk::data::FilterEffect::kEffectMonochrome)) {//モノクロ
			scene->removeShader(layerId, agtk::Shader::kShaderColorGray, seconds);
		}
		if (bit & (1 << agtk::data::FilterEffect::kEffectSepia)) {//セピア
			scene->removeShader(layerId, agtk::Shader::kShaderColorSepia, seconds);
		}
		if (bit & (1 << agtk::data::FilterEffect::kEffectNegaPosiReverse)) {//ネガ反転
			scene->removeShader(layerId, agtk::Shader::kShaderColorNega, seconds);
		}
		if (bit & (1 << agtk::data::FilterEffect::kEffectDefocus)) {//ぼかし
			scene->removeShader(layerId, agtk::Shader::kShaderBlur, seconds);
		}
		if (bit & (1 << agtk::data::FilterEffect::kEffectChromaticAberration)) {//色収差
			scene->removeShader(layerId, agtk::Shader::kShaderColorChromaticAberration, seconds);
		}
		if (bit & (1 << agtk::data::FilterEffect::kEffectDarkness)) {//暗闇
			//背景 or 最前面 or 最前面(メニュー含む)
			if (layerId == agtk::data::SceneData::kBackgroundLayerId ||
				layerId == agtk::data::SceneData::kTopMostLayerId ||
				layerId == agtk::data::SceneData::kTopMostWithMenuLayerId) { 
				scene->removeShader(layerId, agtk::Shader::kShaderColorDark, seconds);
			}
			else {//レイヤー
				scene->removeShader(layerId, agtk::Shader::kShaderColorDarkMask, seconds);
			}
		}
		if (bit & (1 << agtk::data::FilterEffect::kEffectTransparency)) {//透明
			scene->removeShader(layerId, agtk::Shader::kShaderTransparency, seconds);
		}
		// agtk::data::FilterEffect::kEffectBlink, 点滅はオブジェクトのみに実装。
		if (bit & (1 << agtk::data::FilterEffect::kEffectDispImage)) {//画像表示
			if (layerId == agtk::data::SceneData::kBackgroundLayerId) {//背景
				auto sceneBackground = scene->getSceneBackground();

				sceneBackground->removeSceneSprite(seconds);

				if (sceneBackground->getRenderTexture()) {
					sceneBackground->removeShader(agtk::Shader::kShaderImage, seconds);
				}
			}
			else if (layerId == agtk::data::SceneData::kTopMostLayerId) {//最前面
				auto sceneTopMost = scene->getSceneTopMost();

				sceneTopMost->removeSceneSprite(seconds);

				if (sceneTopMost->getRenderTexture()) {
					sceneTopMost->removeShader(agtk::Shader::kShaderImage, seconds);
				}
			}
			else if (layerId == agtk::data::SceneData::kTopMostWithMenuLayerId) {//最前面(メニュー含む)
				auto sceneTopMost = scene->getSceneTopMost();

				sceneTopMost->removeSceneSprite(seconds);

				if (sceneTopMost->getWithMenuRenderTexture()) {
					sceneTopMost->removeWithMenuShader(agtk::Shader::kShaderImage, seconds);
				}
			}
			else {
				auto sceneLayer = scene->getSceneLayer(layerId);

				if (sceneLayer) {
					sceneLayer->removeSceneSprite(seconds);

					if (sceneLayer->getRenderTexture()) {
						sceneLayer->removeShader(agtk::Shader::kShaderImage, seconds);
					}
				}
			}
		}
		if (bit & (1 << agtk::data::FilterEffect::kEffectFillColor)) {//指定色で塗る
			scene->removeShader(layerId, agtk::Shader::kShaderColorRgba, seconds);
		}
	};

	int layerId = cmd->getLayerIndex();
	if (layerId == agtk::data::SceneFilterEffectData::kLayerIndexAllSceneLayers) {//全てのシーンレイヤー
		//レイヤー
		auto sceneLayerList = scene->getSceneLayerList();
		cocos2d::DictElement *el = nullptr;
		CCDICT_FOREACH(sceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
			auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
			removeFilterEffect(sceneLayer->getLayerId());
		}
		//背景
		removeFilterEffect(agtk::data::SceneData::kBackgroundLayerId);
		return;
	}
	else if (layerId == agtk::data::SceneFilterEffectData::kLayerIndexBackground) {//背景
		layerId = agtk::data::SceneData::kBackgroundLayerId;
	}
	else if (layerId == agtk::data::SceneFilterEffectData::kLayerIndexTopMost) {//最前面
		layerId = agtk::data::SceneData::kTopMostLayerId;
	}
	else if (layerId == agtk::data::SceneFilterEffectData::kLayerIndexTopMostWithMenu) {//最前面(メニュー含む)
		layerId = agtk::data::SceneData::kTopMostWithMenuLayerId;
	}
	else {
		layerId += 1;
	}

	removeFilterEffect(layerId);
}

void ObjectAction::execActionSceneGravityChange(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandSceneGravityChangeData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandSceneGravityChangeData *>(commandData);
#endif
	CC_ASSERT(cmd);
	auto scene = this->getScene();
	CC_ASSERT(scene);
	scene->getGravity()->set(
		cmd->getGravity(),//重力（％）
		cmd->getDirection(),//方向
		cmd->getDuration300(),//効果時間
		cmd->getDurationUnlimited()//時間制限なし
	);
}

/**
* シーンを回転・反転させる
* @param	commandData	コマンドデータ
*/
void ObjectAction::execActionSceneRotateFlip(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandSceneRotateFlipData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandSceneRotateFlipData *>(commandData);
#endif
	CC_ASSERT(cmd);

	auto scene = this->getScene();
	auto camera = scene->getCamera();
	auto cameraNode = camera->getCamera();
	if (!cameraNode) {
		//※ACT2-1764 cameraオブジェクトがない場合は、一度更新する。
		camera->update(0);
		cameraNode = camera->getCamera();
	}
	bool isReset = (cmd->getType() == agtk::data::ObjectCommandSceneRotateFlipData::kTypeReset);
	bool isAbsolute = cmd->getAbsoluteRotation();

	//完了までの時間
	float duration = (float)cmd->getDuration300() / 300.0f;

	// ---------------------------------------------------------
	// カメラの回転とレイヤーの反転の設定
	// ---------------------------------------------------------
	Vec2 anchorPoint = Vec2::ANCHOR_MIDDLE;
	bool isRotationActive = false;// 回転が有効
	bool isFlipXActive = false;// X軸反転が有効
	bool isFlipYActive = false;// Y軸反転が有効

	// リセットの場合
	if (isReset) {
		// 回転されている場合
		if (camera->getCameraRotationZ()->getValue() != 0) {
			// アンカーポイントを設定
			cameraNode->setAnchorPoint(anchorPoint);
			camera->getCameraRotationZ()->setValue(0, duration);
			isRotationActive = true;
		}

		// 左右反転している場合
		if (camera->getCameraRotationY()->getValue() != 0) {
			cameraNode->setAnchorPoint(anchorPoint);
			camera->getCameraRotationY()->setValue(0, duration);
			isFlipXActive = true;
		}
		// 上下反転している場合
		if (camera->getCameraRotationX()->getValue() != 0) {
			cameraNode->setAnchorPoint(anchorPoint);
			camera->getCameraRotationX()->setValue(0, duration);
			isFlipYActive = true;
		}
	}
	// リセットでない場合
	else {
		// 回転の場合
		if (cmd->getRotationFlag()) {
			auto cameraRotationZ = camera->getCameraRotationZ();

			// カメラの回転動作が待機中の場合
			if (cameraRotationZ->getState() == agtk::EventTimer::EnumState::kStateIdle) {
				// アンカーポイントを設定
				cameraNode->setAnchorPoint(anchorPoint);

				//※ACT2-1765 回転が反時計回りを時計回りに変更。

				// 絶対座標の場合
				if (isAbsolute) {
					cameraRotationZ->setValue(-cmd->getRotation(), duration);
				}
				// 相対座標の場合
				else {
					cameraRotationZ->addValue(-cmd->getRotation(), duration);
				}
				isRotationActive = true;
			}
		}

		// 左右反転かつ左右反転中でない場合
		if (cmd->getFlipX() && camera->getCameraRotationY()->getState() == agtk::EventTimer::EnumState::kStateIdle) {
			cameraNode->setAnchorPoint(anchorPoint);
			auto value = camera->getCameraRotationY()->getValue() == 0 ? 180 : 0;
			camera->getCameraRotationY()->setValue(value, duration);
			isFlipXActive = true;
		}
		// 上下反転かつ上下反転中でない場合
		if (cmd->getFlipY() && camera->getCameraRotationX()->getState() == agtk::EventTimer::EnumState::kStateIdle) {
			cameraNode->setAnchorPoint(anchorPoint);
			auto value = camera->getCameraRotationX()->getValue() == 0 ? 180 : 0;
			camera->getCameraRotationX()->setValue(value, duration);
			isFlipYActive = true;
		}
	}

#if 0
	// ---------------------------------------------------------
	// 回転・反転の対象外の指定
	// ---------------------------------------------------------
	// プレイヤー・エネミーオブジェクトの対象外設定
	auto objectAll = scene->getObjectAll(this->getSceneLayer()->getType());
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(objectAll, ref) {
		auto object = dynamic_cast<agtk::Object *>(ref);
		auto objAngle = object->getRotation();
		auto type = object->getObjectData()->getType();
		Vector<FiniteTimeAction*> actionList;

		// プレイヤーオブジェクト or エネミーオブジェクトで回転・反転の対象外 or リセットの場合
		if ((cmd->getPlayerObject() && type == agtk::data::ObjectData::kObjTypePlayer) ||
			(cmd->getEnemyObject() && type == agtk::data::ObjectData::kObjTypeEnemy) ||
			isReset) {

			// 反映時間が 0 の場合
			if (duration == 0)
			{
				// cocos2d の action の仕様上 0 では動作しないので違和感のない極短時間の値に置換え
				duration = 0.0001f;
			}

			// 回転が実行される場合
			if (isRotationActive) {
				// リセットの場合
				if (isReset) {
					actionList.pushBack(TargetedAction::create(object->getPlayer()->getCenterNode(), RotateTo::create(duration, 0)));
				}
				else {
					actionList.pushBack(TargetedAction::create(object->getPlayer()->getCenterNode(), RotateBy::create(duration, -cmd->getRotation())));
				}
			}

			// 上下左右反転が実行される場合
			if (isFlipXActive && isFlipYActive) {
				auto scaleX = isReset ? 1.0f : object->getScaleX();
				auto scaleY = isReset ? 1.0f : object->getScaleY();
				actionList.pushBack(TargetedAction::create(object->getPlayer(), ScaleTo::create(duration, -scaleX, -scaleY)));
			}
			//左右反転のみが実行される場合
			else if (isFlipXActive) {
				auto scaleX = isReset ? 1.0f : object->getScaleX();
				auto scaleY = object->getScaleY();
				actionList.pushBack(TargetedAction::create(object->getPlayer(), ScaleTo::create(duration, -scaleX, scaleY)));
			}
			// 上下反転のみが実行される場合
			else if (isFlipYActive) {
				auto scaleX = object->getScaleX();
				auto scaleY = isReset ? 1.0f : object->getScaleY();
				actionList.pushBack(TargetedAction::create(object->getPlayer(), ScaleTo::create(duration, scaleX , -scaleY)));
			}

			if (actionList.size() > 0) {
				object->runAction(Spawn::create(actionList));
			}
		}
	}

	// 重力方向は回転・反転の対象外 or リセットの場合
	if (cmd->getGravityDirection() || isReset) {
		
		// 現在の重力方向
		auto sceneGravity = scene->getGravity();
		float gravityAngle = sceneGravity->getRotation();

		// リセットの場合
		if (isReset) {
			// 初期設定値の重力方向角度に設定
			gravityAngle = scene->getSceneData()->getGravityDirection();
		}
		// リセットでない場合
		else {
			// 回転が実行される場合
			if (isRotationActive) {
				gravityAngle -= cmd->getRotation();
			}

			//左右反転が実行される場合
			if (isFlipXActive) {
				gravityAngle = 360.0f - gravityAngle;
			}

			// 上下反転が実行される場合
			if (isFlipYActive) {
				gravityAngle = 180.0f - gravityAngle;
			}
		}

		// 回転・左右反転・上下反転のいずれかが実行される場合
		if (isRotationActive || isFlipXActive || isFlipYActive) {
			// 回転・反転の処理が終わってから重力を変更
			scene->runAction(Sequence::create(
					DelayTime::create(duration),
					CallFunc::create([sceneGravity, gravityAngle]() {
					sceneGravity->getTimerRotation()->start(sceneGravity->getRotation(), gravityAngle, 0.01f);
				}),
				nullptr
				));
		}
	}
#endif
}

/**
* カメラの表示領域を変更する
* @param	commandData	コマンドデータ
* @note		2018/05/09 ロック対象に関する仕様が削除
*/
void ObjectAction::execActionCameraAreaChange(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandCameraAreaChangeData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandCameraAreaChangeData *>(commandData);
#endif
	CC_ASSERT(cmd);
	
	// 2018.05.08 ACT2-1588 変更基準点をロック対象にする仕様が削除
	auto scene = this->getScene();
	auto camera = scene->getCamera();
	auto scaleXY = Vec2(cmd->getXRate(), cmd->getYRate());

	// 拡縮率が同一の場合は何もしない
#ifdef FIX_ACT2_4471
	if (scaleXY == camera->getCommandScale()->getValue()) {
#else
	if (scaleXY == camera->getZoom()) {
#endif
		return;
	}

	// ズーム動作開始
#ifdef FIX_ACT2_4471
	camera->setCommandZoom(scaleXY, ((float)cmd->getDuration300() / 300.0f));
#else
	camera->setZoom(camera->getTargetPosition(), scaleXY, ((float)cmd->getDuration300() / 300.0f));
#endif
}

void ObjectAction::execActionSoundPlay(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandSoundPlayData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandSoundPlayData *>(commandData);
#endif
	CC_ASSERT(cmd);

	// 再生位置を指定された場合は変数から取得
	double _currentTime = -1.0f;
	if (cmd->getSpecifyAudioPosition()) {
		GameManager::getInstance()->getVariableValue(cmd->getAudioPositionVariableObjectId(), cmd->getAudioPositionVariableQualifierId(), cmd->getAudioPositionVariableId(), _currentTime, _object);
	}

	float fadeSeconds = cmd->getFadein() ? (float)cmd->getDuration300() / 300.0f : 0.0f;
	switch (cmd->getSoundType()) {
	case agtk::data::ObjectCommandSoundPlayData::kSoundSe: {
		if (_object != nullptr) {
			_object->playSeObject(cmd->getSeId(), cmd->getLoop(), cmd->getVolume(), cmd->getPan(), cmd->getPitch(), fadeSeconds, (float)_currentTime);
		}
		break; }
	case agtk::data::ObjectCommandSoundPlayData::kSoundVoice: {
		if (_object != nullptr) {
			_object->playVoiceObject(cmd->getVoiceId(), cmd->getLoop(), cmd->getVolume(), cmd->getPan(), cmd->getPitch(), fadeSeconds, (float)_currentTime);
		}
		break; }
	case agtk::data::ObjectCommandSoundPlayData::kSoundBgm: {
		if (_object != nullptr) {
			_object->playBgmObject(cmd->getBgmId(), cmd->getLoop(), cmd->getVolume(), cmd->getPan(), cmd->getPitch(), fadeSeconds, (float)_currentTime);
		}
		break; }
	}
}

/**
* テキストを表示
* @param	commandData	コマンドデータ
*/
void ObjectAction::execActionMessageShow(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandMessageShowData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandMessageShowData *>(commandData);
#endif
	CC_ASSERT(cmd);

	switch (cmd->getPositionType()) {
		// このオブジェクトがロックしたオブジェクトの中心
		case agtk::data::ObjectCommandMessageShowData::kPositionLockObjectCenter: {
			auto scene = this->getScene();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
			auto objectAll = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
			auto objectAll = scene->getObjectAll(this->getSceneLayer()->getType());
#endif
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectAll, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object*>(ref);
#else
				auto object = dynamic_cast<agtk::Object*>(ref);
#endif
				auto playObjectData = object->getPlayObjectData();

				if (playObjectData->isLocked(_object->getInstanceId())) {
					// GUI管理にメッセージを追加
					GuiManager::getInstance()->addActionCommandMessageGui(_object, object, cmd);
				}
			}
		} break;

		default: {
			// GUI管理にメッセージを追加
			GuiManager::getInstance()->addActionCommandMessageGui(_object, cmd);
		} break;
	}
}

/**
* スクロールメッセージを設定
* @param	commandData	コマンドデータ
*/
void ObjectAction::execActionScrollMessageShow(agtk::data::ObjectCommandData *commandData) 
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandScrollMessageShowData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandScrollMessageShowData *>(commandData);
#endif
	CC_ASSERT(cmd);

	switch (cmd->getPositionType()) {
		// このオブジェクトがロックしたオブジェクトの中心
		case agtk::data::ObjectCommandScrollMessageShowData::kPositionLockObjectCenter: {
			auto scene = this->getScene();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
			auto objectAll = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
			auto objectAll = scene->getObjectAll(this->getSceneLayer()->getType());
#endif

			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectAll, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object*>(ref);
#else
				auto object = dynamic_cast<agtk::Object*>(ref);
#endif
				auto playObjectData = object->getPlayObjectData();

				if (playObjectData->isLocked(_object->getInstanceId())) {
					// GUI管理にスクロールするメッセージを追加
					GuiManager::getInstance()->addActionCommandScrollMessageGui(_object, object, cmd);
				}
			}

		} break;

		default: {
			// GUI管理にスクロールするメッセージを追加
			GuiManager::getInstance()->addActionCommandScrollMessageGui(_object, cmd);
		} break;
	}
}

void ObjectAction::execActionEffectShow(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandEffectShowData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandEffectShowData *>(commandData);
#endif
	CC_ASSERT(cmd);

	auto projectData = GameManager::getInstance()->getProjectData();
	CC_ASSERT(projectData);

	if (cmd->getEffectId() < 0) {
		//設定無し。
		return;
	}

	// アニメーションデータ取得
	auto animationData = projectData->getAnimationData(cmd->getEffectId());

	// エフェクトアニメーションのみ対象
	if (animationData->getType() == agtk::data::AnimationData::kEffect) {

		// 表示時間
		int duration300 = (!cmd->getDurationUnlimited()) ? cmd->getDuration300() : -1;

		// 調整位置
		cocos2d::Vec2 offset = cocos2d::Vec2(cmd->getAdjustX(), cmd->getAdjustY());

		switch (cmd->getPositionType()) {
			// 「このオブジェクトを中心とする」
			case agtk::data::ObjectCommandEffectShowData::EnumPosition::kPositionCenter: {

				// 接続点を使用する場合
				if (cmd->getUseConnect()) {
					// 接続点を設定してエフェクトを生成する
#ifdef USE_REDUCE_RENDER_TEXTURE
					EffectManager::getInstance()->addEffectAnimation(_object, offset, cmd->getConnectId(), duration300, animationData, false);
#else
					EffectManager::getInstance()->addEffectAnimation(_object, offset, cmd->getConnectId(), duration300, animationData);
#endif
				}
				// 接続点を使用しない場合
				else {
					// エフェクトを生成する
#ifdef USE_REDUCE_RENDER_TEXTURE
					EffectManager::getInstance()->addEffectAnimation(_object, offset, duration300, animationData, false);
#else
					EffectManager::getInstance()->addEffectAnimation(_object, offset, duration300, animationData);
#endif
				}
			} break;

			// 「このオブジェクトがロックしたオブジェクトを中心とする」
			case agtk::data::ObjectCommandEffectShowData::EnumPosition::kPositionLockObjectCenter: {

				auto scene = this->getScene();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
				auto objectAll = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
				auto objectAll = scene->getObjectAll(this->getSceneLayer()->getType());
#endif
				CC_ASSERT(objectAll);

				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(objectAll, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = static_cast<agtk::Object *>(ref);
#else
					auto object = dynamic_cast<agtk::Object *>(ref);
#endif
					auto playObjectData = object->getPlayObjectData();

					if (playObjectData->isLocked(_object->getInstanceId())) {
						// エフェクトを生成する
#ifdef USE_REDUCE_RENDER_TEXTURE
						EffectManager::getInstance()->addEffectAnimation(object, offset, duration300, animationData, false);
#else
						EffectManager::getInstance()->addEffectAnimation(object, offset, duration300, animationData);
#endif
					}
				}
			} break;
		}
	}
}

void ObjectAction::execActionMovieShow(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandMovieShowData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandMovieShowData *>(commandData);
#endif
	CC_ASSERT(cmd);
	MovieManager::getInstance()->addMovie(_object, cmd);
}

void ObjectAction::execActionImageShow(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandImageShowData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandImageShowData *>(commandData);
#endif
	CC_ASSERT(cmd);
	ImageManager::getInstance()->addImage(_object, cmd);
}

void ObjectAction::execActionSwitchVariableChange(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandSwitchVariableChangeData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandSwitchVariableChangeData *>(commandData);
#endif
	CC_ASSERT(cmd);

// #AGTK-NX
#ifndef USE_SAR_OPTIMIZE_4
	//「オブジェクトの接続」のスイッチに変化があるかチェックのために情報を取得する。
	auto scene = this->getScene();
	if (scene) {
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		auto objectList = scene->getObjectAllReference();
#else
		auto objectList = scene->getObjectAll();
#endif
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			object->getSwitchInfoCreateConnectObject();
		}
	}
#endif

	if (cmd->getSwtch()) {
		std::function<void(int, agtk::data::PlaySwitchData *)> setSwitchData = [&](int switchValue, agtk::data::PlaySwitchData *switchData) {
			switch (switchValue) {
			case agtk::data::ObjectCommandSwitchVariableChangeData::kSwitchAssignOn: {
				switchData->setValue(true);
				break; }
			case agtk::data::ObjectCommandSwitchVariableChangeData::kSwitchAssignOff: {
				switchData->setValue(false);
				break; }
			case agtk::data::ObjectCommandSwitchVariableChangeData::kSwitchAssignToggle: {
				switchData->setValue(!switchData->getValue());
				break; }
			default:CC_ASSERT(0);
			}
		};
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_4
		class AutoReleaseSetValueCallback
		{
		public:
			AutoReleaseSetValueCallback(std::function<void(agtk::data::PlaySwitchData*)> callback) { agtk::data::PlaySwitchData::setSetValueCallback(callback); }
			~AutoReleaseSetValueCallback() { agtk::data::PlaySwitchData::setSetValueCallback(nullptr); }
		};

		struct UpdatedSwitchInfo
		{
			agtk::data::PlaySwitchData* switchData;
			bool isObjectData;
		};

		// 更新スイッチ検出用
		std::vector<UpdatedSwitchInfo> updatedSwitchInfoList;
		// 更新されたスイッチによる接続オブジェクトの無効スイッチ更新検出用
		std::vector<UpdatedSwitchInfo> updatedDisableSwitchInfoList;

		std::function<void(agtk::data::PlaySwitchData*)> setValueCallback = [&](agtk::data::PlaySwitchData* switchData) {
			UpdatedSwitchInfo info = { switchData, agtk::data::PlaySwitchData::getSetValueCallbackArg_isObjectSwitch() };
			updatedSwitchInfoList.push_back(info);
		};

		std::function<void(agtk::data::PlaySwitchData*)> setDisableValueCallback = [&](agtk::data::PlaySwitchData* switchData) {
			UpdatedSwitchInfo info = { switchData, agtk::data::PlaySwitchData::getSetValueCallbackArg_isObjectSwitch() };
			updatedDisableSwitchInfoList.push_back(info);
		};

		AutoReleaseSetValueCallback svc(setValueCallback);
		// デフォルトはオブジェクトスイッチを更新対象として設定
		agtk::data::PlaySwitchData::setSetValueCallbackArg(true);
#endif
		//スイッチを変更
		if (cmd->getSwitchObjectId() == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {//Resources共通
#ifdef USE_SAR_OPTIMIZE_4
			// プロジェクトスイッチを更新対象として設定
			agtk::data::PlaySwitchData::setSetValueCallbackArg(false);
#endif
			auto projectPlayData = GameManager::getInstance()->getPlayData();
			auto switchData = projectPlayData->getSwitchData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, cmd->getSwitchId());
			if (switchData == nullptr) {
				CC_ASSERT(0);
				return;
			}
			setSwitchData(cmd->getSwitchValue(), switchData);
			projectPlayData->adjustCommonSwitchData(switchData);
		}
		else if (cmd->getSwitchObjectId() > 0) {//オブジェクト共通
			auto projectPlayData = GameManager::getInstance()->getPlayData();
			switch (cmd->getSwitchQualifierId()) {
			case agtk::data::ObjectCommandData::kQualifierSingle: {//単体
				auto singleInstanceData = projectPlayData->getVariableData(cmd->getSwitchObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID);
				auto object = this->getTargetObjectInstanceId((int)singleInstanceData->getValue(), agtk::SceneLayer::kTypeAll);
				if (object) {
					auto playObjectData = object->getPlayObjectData();
					auto switchData = playObjectData->getSwitchData(cmd->getSwitchId());
					if (switchData == nullptr) {
						CC_ASSERT(0);
						return;
					}
					setSwitchData(cmd->getSwitchValue(), switchData);
					playObjectData->adjustSwitchData(switchData);
				}
				break; }
			case agtk::data::ObjectCommandData::kQualifierWhole: {//全体
				auto objectList = this->getTargetObjectById(cmd->getSwitchObjectId(), agtk::SceneLayer::kTypeAll);
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = static_cast<agtk::Object *>(ref);
#else
					auto object = dynamic_cast<agtk::Object *>(ref);
#endif
					auto playObjectData = object->getPlayObjectData();
					auto switchData = playObjectData->getSwitchData(cmd->getSwitchId());
					if (switchData == nullptr) {
						CC_ASSERT(0);
						return;
					}
					setSwitchData(cmd->getSwitchValue(), switchData);
					playObjectData->adjustSwitchData(switchData);
				}
				break; }
			default: {
				if (cmd->getSwitchQualifierId() >= 0) {
					auto objectList = this->getTargetObjectById(cmd->getSwitchObjectId());
					cocos2d::Ref *ref;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						if (object->getInstanceId() == cmd->getSwitchQualifierId()) {
							auto playObjectData = object->getPlayObjectData();
							auto switchData = playObjectData->getSwitchData(cmd->getSwitchId());
							if (switchData == nullptr) {
								CC_ASSERT(0);
								return;
							}
							setSwitchData(cmd->getSwitchValue(), switchData);
							playObjectData->adjustSwitchData(switchData);
						}
					}
				}
				else {
					CC_ASSERT(0);
				}
				break; }
			}
		}
		else if(cmd->getSwitchObjectId() == agtk::data::ObjectCommandData::kSelfObject) {//自身のオブジェクト
			auto playData = _object->getPlayObjectData();
			auto switchData = playData->getSwitchData(cmd->getSwitchId());
			if (switchData == nullptr) {
				CC_ASSERT(0);
				return;
			}
			setSwitchData(cmd->getSwitchValue(), switchData);
			playData->adjustSwitchData(switchData);
		}
		else if (cmd->getSwitchObjectId() == agtk::data::ObjectCommandData::kLockedObject) {//ロックしたオブジェクト
			auto objectList = this->getTargetObjectLocked(agtk::SceneLayer::kTypeAll);
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				auto playObjectData = object->getPlayObjectData();
				auto switchData = playObjectData->getSwitchData(cmd->getSwitchId());
				if (switchData == nullptr) {
					CC_ASSERT(0);
					return;
				}
				setSwitchData(cmd->getSwitchValue(), switchData);
				playObjectData->adjustSwitchData(switchData);
			}
		}
		else if (cmd->getSwitchObjectId() == agtk::data::ObjectCommandData::kParentObject) {//親オブジェクト
			// 親オブジェクトのスイッチを変更
			auto parentObjectInstanceId = _object->getPlayObjectData()->getParentObjectInstanceId();
			if (parentObjectInstanceId == -1) {
				return;
			}
			auto parentObject = this->getTargetObjectInstanceId(parentObjectInstanceId, agtk::SceneLayer::kTypeAll);
			if (parentObject == nullptr) {
				return;
			}
			auto playObjectData = parentObject->getPlayObjectData();
			auto switchData = playObjectData->getSwitchData(cmd->getSwitchId());
			if (switchData == nullptr) {
				return;
			}
			setSwitchData(cmd->getSwitchValue(), switchData);
			playObjectData->adjustSwitchData(switchData);
		}
		else if (cmd->getSwitchObjectId() == agtk::data::ObjectCommandData::kUnsetObject) {//未設定
			return;
		}
		else {
			//エラー
			CC_ASSERT(0);
		}
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_4
		// 接続オブジェクトの無効スイッチの更新検出用設定
		agtk::data::PlaySwitchData::setSetValueCallback(setDisableValueCallback);
		agtk::data::PlaySwitchData::setSetValueCallbackArg(true);

		for(auto switchInfoIt = updatedSwitchInfoList.begin(); switchInfoIt != updatedSwitchInfoList.end(); switchInfoIt++)
		{
			auto switchData = switchInfoIt->switchData;
			auto r = GameManager::getInstance()->getSwitchWatcherObjectList()->equal_range(reinterpret_cast<intptr_t>(switchData));
			for (auto it = r.first; it != r.second; it++)
			{
				auto object = it->second;

				// スイッチ変更による接続オブジェクトの設定変更処理
				object->checkCreateConnectObject(false, true, switchData);
			}
		}

		agtk::data::PlaySwitchData::setSetValueCallback(nullptr);
#endif
	}
	else {
		std::function<double(agtk::data::PlayVariableData *, double)> calcOperator = [&](agtk::data::PlayVariableData *variableData, double value) {
			auto _value = variableData->getValue();
			switch (cmd->getVariableAssignOperator()) {
			case agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignOperatorSet:
				return value;
			case agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignOperatorAdd:
				return _value + value;
			case agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignOperatorSub:
				return _value - value;
			case agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignOperatorMul:
				return _value * value;
			case agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignOperatorDiv: {
				if (value == 0) return std::nan("1");
				return _value / value; }
			case agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignOperatorMod: {
				if (value == 0) return std::nan("1");
				return (double)((int)_value % (int)value); }
			}
			CC_ASSERT(0);
			return 0.0;
		};
		std::function<void(agtk::data::PlayVariableData *)> setVariableData = [&](agtk::data::PlayVariableData *variableData) {
			if (variableData == nullptr) {
				//CC_ASSERT(0);//TODO:バグ
				return;
			}
			auto projectPlayData = GameManager::getInstance()->getPlayData();
			switch (cmd->getVariableAssignValueType()) {
			case agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignValue: {//定数
				auto value = calcOperator(variableData, cmd->getAssignValue());
				variableData->setValue(value);
				break; }
			case agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignVariable: {//変数
				if (cmd->getAssignVariableObjectId() == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {//共通
					auto assignVariableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, cmd->getAssignVariableId());
					if (assignVariableData == nullptr) {
						return;
					}
					auto value = calcOperator(variableData, assignVariableData->getValue());
					variableData->setValue(value);
				}
				else if (cmd->getAssignVariableObjectId() > 0) {//オブジェクト共通
					agtk::Object *object = nullptr;
					if (cmd->getAssignVariableQualifierId() == agtk::data::ObjectCommandData::kQualifierSingle) {
						auto singleInstanceData = projectPlayData->getVariableData(cmd->getAssignVariableObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID);
						object = this->getTargetObjectInstanceId((int)singleInstanceData->getValue(), agtk::SceneLayer::kTypeAll);
					}
					else if (cmd->getAssignVariableQualifierId() >= 0) {
						auto objectList = this->getTargetObjectById(cmd->getAssignVariableObjectId());
						cocos2d::Ref *ref;
						CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto obj = static_cast<agtk::Object *>(ref);
#else
							auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
							if (obj->getInstanceId() == cmd->getAssignVariableQualifierId()) {
								object = obj;
								break;
							}
						}
					} else {
						CC_ASSERT(0);
					}
					if (object) {
						auto assignVariableData = object->getPlayObjectData()->getVariableData(cmd->getAssignVariableId());
						if (assignVariableData == nullptr) {
							return;
						}
						auto value = calcOperator(variableData, assignVariableData->getValue());
						variableData->setValue(value);
					}
				}
				else if (cmd->getAssignVariableObjectId() == agtk::data::ObjectCommandData::kSelfObject) {//自身のオブジェクト
					auto playData = _object->getPlayObjectData();
					auto assignVariableData = playData->getVariableData(cmd->getAssignVariableId());
					if (assignVariableData == nullptr) {
						return;
					}
					auto value = calcOperator(variableData, assignVariableData->getValue());
					variableData->setValue(value);
				}
				else if (cmd->getAssignVariableObjectId() == agtk::data::ObjectCommandData::kLockedObject) {//ロックしたオブジェクト
					auto objectList = this->getTargetObjectLocked(agtk::SceneLayer::kTypeAll);
					if (objectList->count() > 0) {
						auto object = dynamic_cast<agtk::Object *>(objectList->getObjectAtIndex(0));//※ひとまず、リストの先頭オブジェクトの値を使う。
						if (object) {
							auto assignVariableData = object->getPlayObjectData()->getVariableData(cmd->getAssignVariableId());
							CC_ASSERT(assignVariableData);
							auto value = calcOperator(variableData, assignVariableData->getValue());
							variableData->setValue(value);
						}
					}
					else {
						return;
					}
				}
				else if (cmd->getAssignVariableObjectId() == agtk::data::ObjectCommandData::kParentObject) {//親オブジェクト
					auto parentObjectInstanceId = _object->getPlayObjectData()->getParentObjectInstanceId();
					if (parentObjectInstanceId == -1) {
						return;
					}
					auto parentObject = this->getTargetObjectInstanceId(parentObjectInstanceId, agtk::SceneLayer::kTypeAll);
					if (parentObject == nullptr) {
						return;
					}
					auto assignVariableData = parentObject->getPlayObjectData()->getVariableData(cmd->getAssignVariableId());
					if (assignVariableData == nullptr) {
						return;
					}
					auto value = calcOperator(variableData, assignVariableData->getValue());
					variableData->setValue(value);
				}
				else {
					//エラー
					CC_ASSERT(0);
				}
				break; }
			case agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignRandom: {//乱数
				double value = cmd->getRandomMin() + rand() % (cmd->getRandomMax() - cmd->getRandomMin() + 1);
				value = calcOperator(variableData, (double)value);
				variableData->setValue(value);
				break; }
			case agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignScript: {//スクリプト
				if (strlen(cmd->getAssignScript()) > 0) {
#ifdef USE_SCRIPT_PRECOMPILE
					//JSB_AUTOCOMPARTMENT_WITH_GLOBAL_OBJCET
					auto sc = ScriptingCore::getInstance();
					auto cx = sc->getGlobalContext();
					auto _global = sc->getGlobalObject();
					JS::RootedObject gobj(cx, _global);
					JSAutoCompartment ac(cx, gobj);

					JS::RootedObject ns(cx);
					JS::MutableHandleObject jsObj = &ns;
					JS::RootedValue nsval(cx);
					JS_GetProperty(cx, gobj, "Agtk", &nsval);
					JS::RootedValue rv(cx);
					auto sceneId = (_object->getScenePartObjectData() && !_object->getScenePartObjectData()->isStartPointObject() && cmd->getInstanceConfigurable()) ? _object->getSceneData()->getId() : -1;
					auto isCommonAction = this->getIsCommon();
					auto disappearFlag = _object->getDisappearFlag();
					auto actionId = disappearFlag ? -1 : isCommonAction ? this->getCommonActionSettingId() : this->getId();
					bool ret = false;
					if (nsval != JSVAL_VOID) {
						jsObj.set(nsval.toObjectOrNull());
						JS::RootedValue v(cx);
						JS_GetProperty(cx, jsObj, "scriptFunctions", &v);
						if (v.isObject()) {
							JS::RootedObject rscriptFunctions(cx, &v.toObject());
							JS_GetProperty(cx, rscriptFunctions, "execObjectActionCommandSwitchVariableChange", &v);
							if (v.isObject()) {
								JS::RootedValue rexec(cx, v);
								jsval args[6];
								args[0] = JS::Int32Value(sceneId);
								args[1] = JS::Int32Value(_object->getObjectData()->getId());
								args[2] = JS::Int32Value(_object->getInstanceId());
								args[3] = JS::Int32Value(isCommonAction);
								args[4] = JS::Int32Value(actionId);
								args[5] = JS::Int32Value(commandData->getId());
								ret = JS_CallFunctionValue(cx, rscriptFunctions, rexec, JS::HandleValueArray::fromMarkedLocation(6, args), &rv);
							}
						}
					}
#else
					auto scriptingCore = ScriptingCore::getInstance();
					auto context = scriptingCore->getGlobalContext();
					JS::RootedValue rv(context);
					JS::MutableHandleValue mhv(&rv);
					auto script = String::createWithFormat("(function(){ var objectId = %d; var instanceId = %d; var actionId = %d; var commandId = %d; return (%s\n); })()", _object->getObjectData()->getId(), _object->getInstanceId(), this->getId(), commandData->getId(), cmd->getAssignScript());
					auto ret = ScriptingCore::getInstance()->evalString(script->getCString(), mhv);
#endif
					if (!ret) {
						//スクリプトエラー
						auto errorStr = String::createWithFormat("Runtime error in switchVariableChange(objectId: %d, instanceId: %d, actionId: %d, commandId: %d, script: %s).", _object->getObjectData()->getId(), _object->getInstanceId(), this->getId(), commandData->getId(), cmd->getAssignScript())->getCString();
						agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
						auto fp = GameManager::getScriptLogFp();
						if (fp) {
							fwrite(errorStr, 1, strlen(errorStr), fp);
							fwrite("\n", 1, 1, fp);
						}
#endif
						variableData->setValue(std::nan("1"));
					}
					if (rv.isDouble()) {
						variableData->setValue(rv.toDouble());
					}
					else if (rv.isInt32()) {
						variableData->setValue(rv.toInt32());
					}
					else {
						//数値でない
						variableData->setValue(std::nan("1"));
					}
				}
				break; }
			}
		};
		//変数を変更
		auto projectPlayData = GameManager::getInstance()->getPlayData();
		if (cmd->getVariableObjectId() == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {//共通
			auto variableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, cmd->getVariableId());
			setVariableData(variableData);
			projectPlayData->adjustCommonVariableData(variableData);
		}
		else if (cmd->getVariableObjectId() > 0) {//オブジェクト共通
			auto projectPlayData = GameManager::getInstance()->getPlayData();
			switch (cmd->getVariableQualifierId()) {
			case agtk::data::ObjectCommandData::kQualifierSingle: {//単体
				auto singleInstanceData = projectPlayData->getVariableData(cmd->getVariableObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID);
				auto object = this->getTargetObjectInstanceId((int)singleInstanceData->getValue(), agtk::SceneLayer::kTypeAll);
				if (object) {
					auto playObjectData = object->getPlayObjectData();
					auto variableData = playObjectData->getVariableData(cmd->getVariableId());
					setVariableData(variableData);
					playObjectData->adjustVariableData(variableData);
				}
				break; }
			case agtk::data::ObjectCommandData::kQualifierWhole: {//全体
				auto objectList = this->getTargetObjectById(cmd->getVariableObjectId(), agtk::SceneLayer::kTypeAll);
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = static_cast<agtk::Object *>(ref);
#else
					auto object = dynamic_cast<agtk::Object *>(ref);
#endif
					auto playObjectData = object->getPlayObjectData();
					auto variableData = playObjectData->getVariableData(cmd->getVariableId());
					setVariableData(variableData);
					playObjectData->adjustVariableData(variableData);
				}
				break; }
			default: {
				if (cmd->getVariableQualifierId() >= 0) {
					auto objectList = this->getTargetObjectById(cmd->getVariableObjectId());
					cocos2d::Ref *ref;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						if (object->getInstanceId() == cmd->getVariableQualifierId()) {
							auto playObjectData = object->getPlayObjectData();
							auto variableData = playObjectData->getVariableData(cmd->getVariableId());
							setVariableData(variableData);
							playObjectData->adjustVariableData(variableData);
						}
					}
				}
				else {
					CC_ASSERT(0);
				}
				break; }
			}
		}
		else if (cmd->getVariableObjectId() == agtk::data::ObjectCommandData::kSelfObject) {//自身のオブジェクト
			auto playData = _object->getPlayObjectData();
			auto variableData = playData->getVariableData(cmd->getVariableId());
			setVariableData(variableData);
			playData->adjustVariableData(variableData);
		}
		else if (cmd->getVariableObjectId() == agtk::data::ObjectCommandData::kLockedObject) {//ロックしたオブジェクト
			auto objectList = this->getTargetObjectLocked(agtk::SceneLayer::kTypeAll);
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				auto playObjectData = object->getPlayObjectData();
				auto variableData = playObjectData->getVariableData(cmd->getVariableId());
				setVariableData(variableData);
				playObjectData->adjustVariableData(variableData);
			}
		}
		else if (cmd->getVariableObjectId() == agtk::data::ObjectCommandData::kParentObject) {//親オブジェクト
			// 親オブジェクトの変数を変更
			auto parentObjectInstanceId = _object->getPlayObjectData()->getParentObjectInstanceId();
			if (parentObjectInstanceId == -1) {
				return;
			}
			auto parentObject = this->getTargetObjectInstanceId(parentObjectInstanceId, agtk::SceneLayer::kTypeAll);
			if (parentObject == nullptr) {
				return;
			}
			auto playObjectData = parentObject->getPlayObjectData();
			auto variableData = playObjectData->getVariableData(cmd->getVariableId());
			if (variableData == nullptr) {
				return;
			}
			setVariableData(variableData);
			playObjectData->adjustVariableData(variableData);
		}
		else if (cmd->getVariableObjectId() == agtk::data::ObjectCommandData::kUnsetObject) {//未設定
			return;
		}
		else {
			//エラー
			CC_ASSERT(0);
		}
	}

	//変数・スイッチ変更による更新
	GameManager::getInstance()->updateByChangingVariableAndSwitch();
}

void ObjectAction::execActionSwitchVariableReset(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandSwitchVariableResetData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandSwitchVariableResetData *>(commandData);
#endif
	CC_ASSERT(cmd);
	auto projectPlayData = GameManager::getInstance()->getPlayData();
	auto playData = _object->getPlayObjectData();
	auto switchVariableResetList = cmd->getSwitchVariableResetList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(switchVariableResetList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto switchVariableData = static_cast<agtk::data::ObjectCommandSwitchVariableResetData::SwitchVariableData *>(ref);
#else
		auto switchVariableData = dynamic_cast<agtk::data::ObjectCommandSwitchVariableResetData::SwitchVariableData *>(ref);
#endif
		int objectId = switchVariableData->getObjectId();
		int itemId = switchVariableData->getItemId();
		if (switchVariableData->getSwtch()) {//スイッチ
			int objectId = switchVariableData->getObjectId();
			if (objectId == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {//プロジェクト共通
				auto switchData = projectPlayData->getSwitchData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, itemId);
				CC_ASSERT(switchData);
				switchData->reset();
			}
			else if (objectId > 0) {//オブジェクト共通
				auto objectList = this->getTargetObjectById(objectId, agtk::SceneLayer::kTypeAll);
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = static_cast<agtk::Object *>(ref);
#else
					auto object = dynamic_cast<agtk::Object *>(ref);
#endif
					auto playObjectData = object->getPlayObjectData();
					auto switchData = playObjectData->getSwitchData(itemId);
					switchData->reset();
				}
			}
			else if (objectId == agtk::data::ObjectCommandData::kSelfObject) {//自身のオブジェクト
				auto switchData = playData->getSwitchData(itemId);
				CC_ASSERT(switchData);
				switchData->reset();
			}
			else {
				//エラー
				CC_ASSERT(0);
			}
		}
		else {//変数
			if (objectId == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {//プロジェクト共通
				auto variableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, itemId);
				CC_ASSERT(variableData);
				variableData->reset();
			}
			else if (objectId > 0) {//オブジェクト共通
				auto objectList = this->getTargetObjectById(objectId, agtk::SceneLayer::kTypeAll);
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = static_cast<agtk::Object *>(ref);
#else
					auto object = dynamic_cast<agtk::Object *>(ref);
#endif
					auto playObjectData = object->getPlayObjectData();
					auto variableData = playObjectData->getVariableData(itemId);
					variableData->reset();
				}
			}
			else if (objectId == agtk::data::ObjectCommandData::kSelfObject) {//自身のオブジェクト
				auto variableData = playData->getVariableData(itemId);
				CC_ASSERT(variableData);
				variableData->reset();
			}
			else {
				//エラー
				CC_ASSERT(0);
			}
		}
	}

	//変数・スイッチ変更時のオブジェクトに対して変更処理。
	GameManager::getInstance()->updateObjectVariableAndSwitch();
	GameManager::getInstance()->updateSystemVariableAndSwitch();
}

/**
* ゲームスピードを変更する
* @param	commandData	コマンドデータ
*/
void ObjectAction::execActionGameSpeedChange(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandGameSpeedChangeData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandGameSpeedChangeData *>(commandData);
#endif
	CC_ASSERT(cmd);
	auto scene = this->getScene();
	CC_ASSERT(scene);

	auto sceneGameSpeed = scene->getGameSpeed();

	auto const gameSpeed = cmd->getGameSpeed();
	auto const duration = cmd->getDurationUnlimited() ? GameSpeed::DURATION_UNLIMITED : cmd->getDuration300();

	// 「効果対象」に「オブジェクト」がチェックされている場合
	if (cmd->getTargetObject()) {

		// ターゲットリストを初期化
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		auto objectAll = scene->getObjectAllReference(agtk::SceneLayer::kTypeScene);
#else
		auto objectAll = scene->getObjectAll(agtk::SceneLayer::kTypeScene);
#endif
		cocos2d::RefPtr<cocos2d::__Array> targetObjectList = cocos2d::Array::create();

		// 「効果対象のオブジェクトを指定」処理
		switch (cmd->getTargettingType()) {

			// 「このオブジェクトに触れたオブジェクト」
		case agtk::data::ObjectCommandGameSpeedChangeData::EnumTargettingType::kTargettingTouched: {
			if (!_object->getCollisionWallWallChecked()) {
				_object->updateCollisionWallWallList();
			}
			cocos2d::Ref *ref;
			auto collisionWallWallList = _object->getCollisionWallWallList();
			CCARRAY_FOREACH(collisionWallWallList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				if (cmd->getExcludeObjectGroupBit() & (object->getObjectData()->getGroupBit())) { continue; }
				targetObjectList->addObject(object);
			}
		} break;

			// 「このオブジェクトがロックしたオブジェクト」
		case agtk::data::ObjectCommandGameSpeedChangeData::EnumTargettingType::kTargettingLocked: {
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectAll, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				auto playObjectData = object->getPlayObjectData();

				if (playObjectData->isLocked(_object->getInstanceId())) {
					// ロックしているオブジェクトを効果対象とする
					targetObjectList->addObject(object);
				}
			}
		} break;
			// 「オブジェクトで指定」
		case agtk::data::ObjectCommandGameSpeedChangeData::kTargettingById: {
			auto targetObjectId = cmd->getTargetObjectId();
			if (targetObjectId == agtk::data::ObjectCommandData::kSelfObject) {//自身のオブジェクト
				targetObjectList->addObject(_object);
			} else
			if (targetObjectId == agtk::data::ObjectCommandData::kOtherThanSelfObject) {//自身以外のオブジェクト
				auto objectAllList = scene->getObjectAll(this->getSceneLayer()->getType());
				cocos2d::Ref *ref;
				CCARRAY_FOREACH(objectAllList, ref) {
					auto object = dynamic_cast<agtk::Object *>(ref);
					if (object == _object) continue;
					targetObjectList->addObject(object);
				}
			} else 
			if (targetObjectId != TARGET_NONE) {	// 指定が「設定無し」以外の場合
				auto projectPlayData = GameManager::getInstance()->getPlayData();
				switch (cmd->getTargetQualifierId())
				{
					// 単体
					case agtk::data::ObjectCommandData::kQualifierSingle:
					{
						auto singleInstanceData = projectPlayData->getVariableData(targetObjectId, agtk::data::kObjectSystemVariableSingleInstanceID);
						cocos2d::Ref *ref = nullptr;
						CCARRAY_FOREACH(objectAll, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto object = static_cast<agtk::Object *>(ref);
#else
							auto object = dynamic_cast<agtk::Object *>(ref);
#endif
							if (object->getInstanceId() == (int)singleInstanceData->getValue()) {
								targetObjectList->addObject(object);
							}
						}
						break;
					}
					// 全体
					case agtk::data::ObjectCommandData::kQualifierWhole:
					{
						cocos2d::Ref *ref = nullptr;
						CCARRAY_FOREACH(objectAll, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto object = static_cast<agtk::Object *>(ref);
#else
							auto object = dynamic_cast<agtk::Object *>(ref);
#endif
							auto objectData = object->getObjectData();
							if (objectData->getId() == targetObjectId) {
								targetObjectList->addObject(object);
							}
						}
						break;
					}
					default: {
						if (cmd->getTargetQualifierId() >= 0) {
							auto objectList = this->getTargetObjectById(targetObjectId);
							cocos2d::Ref *ref;
							CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
								auto object = static_cast<agtk::Object *>(ref);
#else
								auto object = dynamic_cast<agtk::Object *>(ref);
#endif
								if (object->getInstanceId() == cmd->getTargetQualifierId()) {
									targetObjectList->addObject(object);
								}
							}
						} else {
							CC_ASSERT(0);
						}
						break;
					}
				}
			}

		} break;
			// 「オブジェクトグループで指定」
		case agtk::data::ObjectCommandGameSpeedChangeData::kTargettingByGroup:
		{
			auto targetObjectGroup = cmd->getTargetObjectGroup();

			// 「全てのオブジェクト」が対象の場合
			if (targetObjectGroup == EnumTargetObjectGroup::kAll) {
				targetObjectList->addObjectsFromArray(objectAll);
			}
			// 「プレイヤーグループ」または「エネミーグループ」の場合
			else {
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(objectAll, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = static_cast<agtk::Object *>(ref);
#else
					auto object = dynamic_cast<agtk::Object *>(ref);
#endif
					if (targetObjectGroup == object->getObjectData()->getGroup() ){
						targetObjectList->addObject(object);
					}
				}
			}
		} break;
#ifdef AGTK_DEBUG
		// 「全てのオブジェクト」
		// ※デバッグビルドで新しい方式に置換えられていないプロジェクトを実行すると落ちるので互換性として残す
		case agtk::data::ObjectCommandGameSpeedChangeData::kTargettingAllObject:
		{
			targetObjectList->addObjectsFromArray(objectAll);
		} break;
#endif
		default:CC_ASSERT(0);
		}

		sceneGameSpeed->set(SceneGameSpeed::eTYPE_OBJECT, cmd->getTargettingType(), cmd->getTargetObjectGroup(), cmd->getTargetObjectId(), cmd->getTargetQualifierId(), targetObjectList, gameSpeed, duration);
	}

	// 「効果対象」に「エフェクト」がチェックされている場合
	if (cmd->getTargetEffect()) {
		sceneGameSpeed->set(SceneGameSpeed::eTYPE_EFFECT, gameSpeed, duration);
	}

	// 「効果対象」に「タイル」がチェックされている場合
	if (cmd->getTargetTile()) {
		sceneGameSpeed->set(SceneGameSpeed::eTYPE_TILE, gameSpeed, duration);
	}

	// 効果対象に「メニュー関連」がチェックされている場合
	if (cmd->getTargetMenu()) {
		sceneGameSpeed->set(SceneGameSpeed::eTYPE_MENU, gameSpeed, duration);
	}
}

/**
* その他アクション:ウェイト
* @param	commandData	オブジェクトコマンドデータ
*/
void ObjectAction::execActionWait(agtk::data::ObjectCommandData *commandData)
{
	// コマンドデータをウェイトコマンドデータへキャスト
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandWaitData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandWaitData *>(commandData);
#endif
	CC_ASSERT(cmd);

	// 「ゲームの進行」の場合
	if (cmd->getStopGame()) {
		// 待ちフレームをシーンに設定
		auto scene = this->getScene();
		scene->setWaitDuration300(cmd->getDuration300());
	}
	// 「この実行アクション」の場合
	else {
		// 待ちフレームをこのアクションに設定
		_waitDuration300 = cmd->getDuration300();
	}
}

/**
 * シーン終了
 * @param	commandData	コマンドデータ
 */
void ObjectAction::execActionSceneTerminate(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandSceneTerminateData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandSceneTerminateData *>(commandData);
#endif
	CC_ASSERT(cmd);
	GameManager::getInstance()->setNeedTerminateScene(true);
}

void ObjectAction::execActionDirectionMove(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandDirectionMoveData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandDirectionMoveData *>(commandData);
#endif
	CC_ASSERT(cmd);

	//テンプレート移動中の場合はテンプレート移動を終了。
	auto objectTemplate = _object->getObjectTemplateMove();
	if (objectTemplate->isMoving()) {
		objectTemplate->end(true);
	}

	//移動方向
	auto direction = agtk::GetDirectionFromDegrees(cmd->getDirection());
	direction = _object->directionCorrection(direction);
	int moveDirectionId = agtk::GetMoveDirectionId(cmd->getDirection());

	//表示方向
	if (cmd->getDirectionId() == agtk::data::ObjectCommandData::kAccordingToMoveDirection) {//移動方向に合わせる。
		int actionId = _object->getCurrentObjectAction()->getId();
		if (moveDirectionId != _object->getMoveDirection()) {
			_object->playAction(actionId, moveDirectionId);
		}
	}
	else if (cmd->getDirectionId() == -3) {//表示なし
		//保留
		//移動時に非表示にする。
		CC_ASSERT(0);
	}
	else {
		//アニメーションの表示方向ID指定。
		int actionId = _object->getCurrentObjectAction()->getId();
		_object->playAction(actionId, moveDirectionId, cmd->getDirectionId());
		//表示方向IDを設定。
		auto direct = _object->calcDispDirection();
		_object->setDispDirection(direct);
	}
	auto objectMovement = _object->getObjectMovement();
	if (cmd->getMoveDistanceEnabled()) {
		objectMovement->setDistanceMax(cmd->getMoveDistance());
		objectMovement->setMovedDistance(0);
	}
	else {
		objectMovement->setDistanceMax(-1);
	}
	objectMovement->setDirectionForce(direction, true);//強制移動設定。
	objectMovement->setForceMoveChangeMoveSpeed();//移動速度を変更（％）をデフォルト値（100）に戻す。
}

void ObjectAction::execActionForthBackMoveTurn(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandForthBackMoveTurnData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandForthBackMoveTurnData *>(commandData);
#endif
	CC_ASSERT(cmd);

	//テンプレート移動中の場合はテンプレート移動を終了。
	auto objectTemplate = _object->getObjectTemplateMove();
	if (objectTemplate->isMoving()) {
		objectTemplate->end(true);
	}

	int inputDirectionId = 0;
	auto objectMovement = _object->getObjectMovement();
	auto objectData = _object->getObjectData();

	//基本移動の場合
	if (objectData->getMoveType() == agtk::data::ObjectData::kMoveNormal) {
		//処理しない。
		return;
	}

	//         上
	//    +---+---+---+
	//    | 7 | 8 | 9 |
	//    +---+---+---+
	// 左 | 4 |   | 6 | 右
	//    +---+---+---+
	//    | 1 | 2 | 3 |
	//    +---+---+---+
	//         下

	//移動
	if (cmd->getMoveType() == 0) {//なし
		//旋回
		if (cmd->getTurnType() == 0) {//なし
		}
		else if (cmd->getTurnType() == 1) {//右旋回
			inputDirectionId = 6;
		}
		else if (cmd->getTurnType() == 2) {//左旋回
			inputDirectionId = 4;
		}
		else {
			//エラー
			CC_ASSERT(0);
		}
	}
	else if (cmd->getMoveType() == 1) {//前方移動
		//旋回
		if (cmd->getTurnType() == 0) {//なし
			inputDirectionId = 8;
		}
		else if (cmd->getTurnType() == 1) {//右旋回
			inputDirectionId = 9;
		}
		else if (cmd->getTurnType() == 2) {//左旋回
			inputDirectionId = 7;
		}
		else {
			//エラー
			CC_ASSERT(0);
		}
	}
	else if (cmd->getMoveType() == 2) {//後方移動
		//旋回
		if (cmd->getTurnType() == 0) {//なし
			inputDirectionId = 2;
		}
		else if (cmd->getTurnType() == 1) {//右旋回
			inputDirectionId = 1;
		}
		else if (cmd->getTurnType() == 2) {//左旋回
			inputDirectionId = 3;
		}
		else {
			//エラー
			CC_ASSERT(0);
		}
	}
	else {
		//エラー
		CC_ASSERT(0);
	}
	_object->setInputDirectionId(inputDirectionId);

	//移動方向
	int directionId = 0;
	int forceDirectionId = -1;
	int actionId = _object->getCurrentObjectAction()->getId();
	directionId = _object->getDispDirection();
	if (directionId <= 0) {
		directionId = _object->calcDispDirection();
	}
	if (cmd->getDirectionId() != -2) {//表示方向の指定
		forceDirectionId = cmd->getDirectionId();
	}
	_object->playAction(actionId, directionId, forceDirectionId);

	auto inputMoveDirection = agtk::GetDirectionFromMoveDirectionId(inputDirectionId);
	objectMovement->setInputDirectionForce(inputMoveDirection);

	if (objectMovement->getDirection() == cocos2d::Vec2::ZERO) {
		auto direction = agtk::GetDirectionFromMoveDirectionId(directionId);
		objectMovement->setDirectionForce(direction);//強制移動設定。
	}
	if (objectMovement->getForceMove()->isMoving()) {
		objectMovement->getForceMove()->setIgnoredEndReset(true);
		objectMovement->setForceMoveChangeMoveSpeed();//移動速度を変更（％）をデフォルト値（100）に戻す。
	}
}

/**
 * オブジェクトのアクションを実行
 * @param	commandData	コマンドデータ
 * @return				True: 自身の別アクションを実行した / False:他のオブジェクトのアクションを実行した
 */
bool ObjectAction::execActionActionExec(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandActionExecData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandActionExecData *>(commandData);
#endif
	CC_ASSERT(cmd);

	bool isActionMyOtherAction = false;

	switch (cmd->getObjectId()) {
	case -1://設定無し。
		break;
	case agtk::data::ObjectCommandData::kSelfObject: {//自身のオブジェクト
		int directionId = this->getDispDirection();
		_object->playAction(cmd->getActionId(), directionId);
		isActionMyOtherAction = true;
		break; }
	default: {
		switch (cmd->getQualifierId()) {
		case agtk::data::ObjectCommandData::kQualifierSingle: {//単体
			auto objectId = cmd->getObjectId();
			auto projectPlayData = GameManager::getInstance()->getPlayData();
			auto singleInstanceData = projectPlayData->getVariableData(objectId, agtk::data::kObjectSystemVariableSingleInstanceID);
			auto object = this->getTargetObjectInstanceId((int)singleInstanceData->getValue());
			if (object != nullptr) {
				int directionId = object->getDispDirection();
				object->playAction(cmd->getActionId(), directionId);
				isActionMyOtherAction = (object == _object);
			}
			break; }
		case agtk::data::ObjectCommandAttackSettingData::kQualifierWhole: {//全体
			auto objectList = this->getTargetObjectById(cmd->getObjectId());
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				int directionId = object->getDispDirection();
				object->playAction(cmd->getActionId(), directionId);
				if (object->getInstanceId() == _object->getInstanceId()) {
					isActionMyOtherAction = true;
				}
			}
			break; }
		default: {
			if (cmd->getQualifierId() >= 0) {
				auto objectList = this->getTargetObjectById(cmd->getObjectId());
				cocos2d::Ref *ref;
				CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = static_cast<agtk::Object *>(ref);
#else
					auto object = dynamic_cast<agtk::Object *>(ref);
#endif
					if (object->getInstanceId() == cmd->getQualifierId()) {
						int directionId = object->getDispDirection();
						object->playAction(cmd->getActionId(), directionId);
						if (object->getInstanceId() == _object->getInstanceId()) {
							isActionMyOtherAction = true;
						}
						break;
					}
				}
			}
			break; }
		}
		break; }
	}
	return isActionMyOtherAction;
}

/**
 * パーティクルの表示
 * @param	commandData	コマンドデータ
 */
void ObjectAction::execActionParticleShow(agtk::data::ObjectCommandData *commandData)
{
	// パーティクル表示用コマンドデータに変換
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandParticleShowData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandParticleShowData *>(commandData);
#endif
	CC_ASSERT(cmd);

	// 設定なしの場合
	if (cmd->getParticleId() < 0) {
		// 何もしない
		return;
	}

	// ワーク変数
	auto scene = this->getScene();
	auto sceneLayer = this->getSceneLayer();
	auto objectData = _object->getObjectData();
	auto useConnect = cmd->getUseConnect();
	cocos2d::__Array *objectList = nullptr;

	// 指定オブジェクトを元に生成位置を割り出し
	std::function<bool(agtk::Object *, cocos2d::Vec2 &)> calcObjectPosition = [&](agtk::Object *object, cocos2d::Vec2 &pos) {
		if (cmd->getPositionType() == agtk::data::ObjectCommandObjectCreateData::kPositionCenter) {
			// 接続点を使用する場合
			int connectId = cmd->getConnectId();
			if (useConnect && connectId >= 0) {
				//接続点を使用
				agtk::Vertex4 vertex4;
				if (object->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, connectId, vertex4)) {
					pos = Scene::getPositionSceneFromCocos2d(vertex4.addr()[0]);
				}
				else {
					return false;
				}
			}
			// オブジェクトの中心点の場合
			else {
				pos = object->getCenterPosition();
			}
		}
		else if (cmd->getPositionType() == agtk::data::ObjectCommandObjectCreateData::kPositionLockObjectCenter) {
			pos = object->getCenterPosition();
		}
		else {
			return false;
		}
		return true;
	};

	// どの表示位置タイプか
	switch (cmd->getPositionType())
	{
		// ----------------------------------------
		// このオブジェクトの位置
		// ----------------------------------------
		case agtk::data::ObjectCommandObjectCreateData::kPositionCenter:
		{
			objectList = cocos2d::__Array::create();
			objectList->addObject(_object);
			break;
		}
		// ----------------------------------------
		// ロックしたオブジェクトの位置
		// ----------------------------------------
		case agtk::data::ObjectCommandObjectCreateData::kPositionLockObjectCenter:
		{
			// このオブジェクトがロックしているオブジェクトのリストを取得
			objectList = this->getTargetObjectLocked();
			break;
		}
		default:CC_ASSERT(0);
	}

	// 表示位置の対象となるオブジェクトが存在しない場合
	if (nullptr == objectList || objectList->count() <= 0) {
		// 何もしない
		return;
	}

	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(objectList, ref) {
		// 生成位置対象オブジェクト
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto targetObject = static_cast<agtk::Object *>(ref);
#else
		auto targetObject = dynamic_cast<agtk::Object *>(ref);
#endif

		//オブジェクト位置算出
		cocos2d::Vec2 pos;
		if (calcObjectPosition(targetObject, pos) == false) {
			continue;
		}

		//位置を調整
		pos.x += (float)cmd->getAdjustX();
		pos.y += (float)cmd->getAdjustY();

		// 補正分の座標を算出
		pos = pos - targetObject->getPosition();

		auto projectData = GameManager::getInstance()->getProjectData();
		auto particleList = projectData->getAnimationData(cmd->getParticleId())->getParticleList();

		int connectId = (useConnect && cmd->getPositionType() == agtk::data::ObjectCommandObjectCreateData::kPositionCenter) ? cmd->getConnectId() : -1;
		// パーティクル生成
		auto particle = ParticleManager::getInstance()->addParticle(
			targetObject,
			targetObject->getSceneIdOfFirstCreated(),
			targetObject->getLayerId(),
			cmd->getParticleId(),
			pos,
			connectId,
			cmd->getDuration300(),
			cmd->getDurationUnlimited(),
			particleList,
#ifdef USE_REDUCE_RENDER_TEXTURE
			false,
#endif
			_object
		);

		// パーティクル生成位置オフセット設定
		particle->setPositionOffset(Vec2(cmd->getAdjustX(), cmd->getAdjustY()));
	}
}

void ObjectAction::execActionTimer(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandTimerData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandTimerData *>(commandData);
#endif
	CC_ASSERT(cmd);
	auto scene = this->getScene();
	auto projectPlayData = GameManager::getInstance()->getPlayData();

	agtk::Object *targetObject = nullptr;
	//todo cmd->getTimerVariableQualifierId(), (cmd->getTimerVariableQualifierId()参照。
	agtk::data::PlayVariableData *variableData = nullptr;
	if (cmd->getTimerVariableObjectId() == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {//Resources共通
		variableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, cmd->getTimerVariableId());
	}
	else if (cmd->getTimerVariableObjectId() == agtk::data::ObjectCommandData::kSelfObject) {//自身のオブジェクト
		targetObject = _object;
		auto playObjectData = _object->getPlayObjectData();
		variableData = playObjectData->getVariableData(cmd->getTimerVariableId());
		if (variableData == nullptr) {
			//設定無し。
			CC_ASSERT(cmd->getTimerVariableId() == -1);
			return;
		}
	}
	else if (cmd->getTimerVariableObjectId() == -1) {//設定無し。
		return;
	}
	else if (cmd->getTimerVariableObjectId() > 0) {//オブジェクト
		auto singleInstanceData = projectPlayData->getVariableData(cmd->getTimerVariableObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID);
		auto object = this->getTargetObjectInstanceId((int)singleInstanceData->getValue());
		if (object == nullptr) {
			//対象オブジェクトが存在しない場合は、処理を終了する。
			return;
		}
		targetObject = object;
		auto playObjectData = object->getPlayObjectData();
		variableData = playObjectData->getVariableData(cmd->getTimerVariableId());
	}
	else {
		CC_ASSERT(0);
		return;
	}
	if (variableData == nullptr) {
		CC_ASSERT(0);
		return;
	}

	if (cmd->getStart()) {//開始
		double seconds = 0.0f;
		if (cmd->getSecondType() == 0) {//秒数を入力
			seconds = (double)cmd->getSecond300() / 300.0;
		}
		else if (cmd->getSecondType() == 1) {//変数で指定
			agtk::data::PlayVariableData *secondVariableData = nullptr;
			if (cmd->getSecondVariableObjectId() == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {//Resources共通
				secondVariableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, cmd->getSecondVariableId());
			}
			else if (cmd->getSecondVariableObjectId() == agtk::data::ObjectCommandData::kSelfObject) {//自身のオブジェクト
				auto playObjectData = _object->getPlayObjectData();
				secondVariableData = playObjectData->getVariableData(cmd->getSecondVariableId());
			}
			else if (cmd->getSecondVariableObjectId() > 0) {//オブジェクト
				auto singleInstanceData = projectPlayData->getVariableData(cmd->getSecondVariableObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID);
				auto object = this->getTargetObjectInstanceId((int)singleInstanceData->getValue());
				if (object == nullptr) {
					//対象オブジェクトが存在しない場合は、処理を終了する。
					return;
				}
				auto playObjectData = object->getPlayObjectData();
				secondVariableData = playObjectData->getVariableData(cmd->getSecondVariableId());
			}
			if (secondVariableData) {
				seconds = secondVariableData->getValue();
			}
		}
		else {
			//エラー
			CC_ASSERT(0);
		}
		scene->startVariableTimer(variableData, cmd->getCountUp(), seconds, targetObject);
	}
	else {//停止
		scene->endVariableTimer(variableData);
	}
}

void ObjectAction::execActionSceneShake(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandSceneShakeData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandSceneShakeData *>(commandData);
#endif
	CC_ASSERT(cmd);

	// 「画面振動を無効」がONの場合は無効にする。
	if (DebugManager::getInstance()->getDisplayData()->getDisableScreenShake()) {
		return;
	}

	auto scene = this->getScene();
	scene->getShake()->start(cmd);
}

void ObjectAction::execActionEffectRemove(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandEffectRemoveData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandEffectRemoveData *>(commandData);
#endif
	CC_ASSERT(cmd);
	auto effectManager = EffectManager::getInstance();

	int effectId = cmd->getEffectId();
	if (effectId == -1) {
		//設定無し。
		return;
	}

	auto scene = this->getScene();
	auto objectList = cocos2d::__Array::create();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	auto objectAllList = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
	auto objectAllList = scene->getObjectAll(this->getSceneLayer()->getType());
#endif
	switch (cmd->getTargettingType()) {
	case agtk::data::ObjectCommandEffectRemoveData::kTargettingByGroup: {//オブジェクトの種類で指定
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(objectAllList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			if (cmd->getTargetObjectGroup() == agtk::data::ObjectData::kObjGroupAll) {//すべてのオブジェクト
				objectList->addObject(object);
			}
			else if (cmd->getTargetObjectGroup() == object->getObjectData()->getGroup()) {
				objectList->addObject(object);
			}
		}
		break; }
	case agtk::data::ObjectCommandEffectRemoveData::kTargettingById: {//オブジェクトで指定
		int targetObjectId = cmd->getTargetObjectId();
		if (targetObjectId == -1) {//設定無し。
			return;
		}
		if (targetObjectId == agtk::data::ObjectCommandData::kSelfObject) {//自身のオブジェクト
			objectList->addObject(_object);
		}
		else if (targetObjectId == agtk::data::ObjectCommandData::kOtherThanSelfObject) {//自身以外のオブジェクト
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(objectAllList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				if (object == _object) continue;
				objectList->addObject(object);
			}
		}
		else if (targetObjectId > 0) {
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(objectAllList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				if (object->getObjectData()->getId() == targetObjectId) {
					objectList->addObject(object);
				}
			}
		}
		break; }
	case agtk::data::ObjectCommandEffectRemoveData::kTargettingSceneEffect: {//シーンに表示中のエフェクト
		objectList->addObjectsFromArray(objectAllList);
		break; }
	default: CC_ASSERT(0);
	}

	//削除
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object = static_cast<agtk::Object *>(ref);
#else
		auto object = dynamic_cast<agtk::Object *>(ref);
#endif
		if (effectId == -2) {
			effectManager->removeEffectAll(object, true);//インスタンス破棄。
		}
		else {
			effectManager->removeEffect(object, effectId, true);//インスタンス破棄。
		}
	}
}

/**
* パーティクルの非表示
* @param	commandData		コマンドデータ
*/
void ObjectAction::execActionParticleRemove(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandParticleRemoveData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandParticleRemoveData *>(commandData);
#endif
	CC_ASSERT(cmd);

	// 設定なしの場合
	if (cmd->getParticleId() == agtk::data::ObjectCommandParticleRemoveData::NO_PARTICLE_TARGET) {
		// 何もしない
		return;
	}

	auto particleManager = ParticleManager::getInstance();

	int particleId = cmd->getParticleId();
	auto scene = this->getScene();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	auto objectAllList = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
	auto objectAllList = scene->getObjectAll(this->getSceneLayer()->getType());
#endif
	auto objectList = cocos2d::__Array::create();
	switch (cmd->getTargettingType()) {
	case agtk::data::ObjectCommandParticleRemoveData::kTargettingByGroup: {//オブジェクトの種類で指定
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(objectAllList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			if (cmd->getTargetObjectGroup() == agtk::data::ObjectData::kObjGroupAll) {//すべてのオブジェクト
				objectList->addObject(object);
			}
			else if (cmd->getTargetObjectGroup() == object->getObjectData()->getGroup()) {
				objectList->addObject(object);
			}
		}
		break; }
	case agtk::data::ObjectCommandParticleRemoveData::kTargettingById: {//オブジェクトで指定
		int targetObjectId = cmd->getTargetObjectId();
		if (targetObjectId == agtk::data::ObjectCommandData::kSelfObject) {//自身のオブジェクト
			objectList->addObject(_object);
		}
		else if (targetObjectId == agtk::data::ObjectCommandData::kOtherThanSelfObject) {//自身以外のオブジェクト
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(objectAllList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				if (object == _object) continue;
				objectList->addObject(object);
			}
		}
		else if (targetObjectId > 0) {
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(objectAllList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				if (object->getObjectData()->getId() == targetObjectId) {
					objectList->addObject(object);
				}
			}
		}
		break; }
	case agtk::data::ObjectCommandParticleRemoveData::kTargettingSceneEffect: {//シーンに表示中のエフェクト
		objectList->addObjectsFromArray(objectAllList);
		break; }
	default: CC_ASSERT(0);
	}

	//パーティクルの発生を停止
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object = static_cast<agtk::Object *>(ref);
#else
		auto object = dynamic_cast<agtk::Object *>(ref);
#endif
		object->stopEmitteParticles(particleId);
	}
}

/**
* レイヤーの表示OFF
* @param	commandData	コマンドデータ
*/
void ObjectAction::execActionLayerHide(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandLayerHide *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandLayerHide *>(commandData);
#endif
	CC_ASSERT(cmd);

	setLayerVisible(cmd->getLayerIndex(), cmd->getExceptFlag(), false);
}

/**
* レイヤーの表示ON
* @param	commandData	コマンドデータ
*/
void ObjectAction::execActionLayerShow(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandLayerShow *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandLayerShow *>(commandData);
#endif
	CC_ASSERT(cmd);

	setLayerVisible(cmd->getLayerIndex(), cmd->getExceptFlag(), true);
}

/**
* レイヤーの表示変更設定
* @param	layerIdx	対象レイヤーIDX
* @param	isExcept	対象レイヤーIDX以外か？
* @param	isVisible	表示フラグ
*/
void ObjectAction::setLayerVisible(int layerIdx, bool isExcept, bool isVisible)
{
	auto scene = this->getScene();
	auto layerDic = scene->getSceneLayerList();
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(layerDic, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto layer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto layer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		auto isTargetLayer = layerIdx == layer->getLayerId() - 1;

		// 対象レイヤーIDXのみ かつ 対象レイヤーIDXである
		// または
		// 対象レイヤーIDX以外 かつ 対象レイヤーIDXでない場合
		if (isExcept ^ isTargetLayer) {
			layer->setIsVisible(isVisible);
		}
	}
}

/**
* レイヤーの動作OFF
* @param	commandData	コマンドデータ
*/
void ObjectAction::execActionLayerDisable(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandLayerDisable *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandLayerDisable *>(commandData);
#endif
	CC_ASSERT(cmd);

	setLayerActive(cmd->getLayerIndex(), cmd->getExceptFlag(), false);
}

/**
* レイヤーの動作ON
* @param	commandData	コマンドデータ
*/
void ObjectAction::execActionLayerEnable(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandLayerEnable *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandLayerEnable *>(commandData);
#endif
	CC_ASSERT(cmd);

	setLayerActive(cmd->getLayerIndex(), cmd->getExceptFlag(), true);
}

/**
* レイヤーの動作変更設定
* @param	layerIdx	対象レイヤーIDX
* @param	isExcept	対象レイヤーIDX以外か？
* @param	enable		動作ON/OFFフラグ
*/
void ObjectAction::setLayerActive(int layerIdx, bool isExcept, bool enable)
{
	if (layerIdx < 0) {
		//設定無し。
		return;
	}
	auto scene = this->getScene();
	auto layerDic = scene->getSceneLayerList();
	auto emDisabledLayerList = EffectManager::getInstance()->getDisabledLayerIdList();
	auto pmDisabledlayerList = ParticleManager::getInstance()->getDisabledLayerIdList();
	auto bmDisabledlayerList = BulletManager::getInstance()->getDisabledLayerIdList();

	emDisabledLayerList->removeAllObjects();
	pmDisabledlayerList->removeAllObjects();
	bmDisabledlayerList->removeAllObjects();

	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(layerDic, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto layer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto layer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		auto layerId = layer->getLayerId();
		auto isTargetLayer = (layerIdx == layerId - 1);

		// 対象レイヤーIDXのみ かつ 対象レイヤーIDXである
		// または
		// 対象レイヤーIDX以外 かつ 対象レイヤーIDXでない場合
		if (isExcept ^ isTargetLayer) {
			layer->setActiveFlg(enable);

			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(layer->getPhysicsObjectList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto pObj = static_cast<agtk::PhysicsBase *>(ref);
#else
				auto pObj = dynamic_cast<agtk::PhysicsBase *>(ref);
#endif
				auto body = pObj->getPhysicsBody();
				if (body) {
					body->setEnabled(enable);
				}
			}
		}

		// 無効化されているレイヤーの場合
		if (!layer->getActiveFlg()) {
			auto data = cocos2d::Integer::create(layerId);
			emDisabledLayerList->addObject(data);
			pmDisabledlayerList->addObject(data);
			bmDisabledlayerList->addObject(data);
		}
	}
}

/**
* スクリプトを記述して実行
* @param	commandData	コマンドデータ
*/
int ObjectAction::execActionScriptEvaluate(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandScriptEvaluate *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandScriptEvaluate *>(commandData);
#endif
	CC_ASSERT(cmd);

	if (strlen(cmd->getScript()) > 0) {
#ifdef USE_SCRIPT_PRECOMPILE
		//JSB_AUTOCOMPARTMENT_WITH_GLOBAL_OBJCET
		auto sc = ScriptingCore::getInstance();
		auto cx = sc->getGlobalContext();
		auto _global = sc->getGlobalObject();
		JS::RootedObject gobj(cx, _global);
		JSAutoCompartment ac(cx, gobj);

		JS::RootedObject ns(cx);
		JS::MutableHandleObject jsObj = &ns;
		JS::RootedValue nsval(cx);
		JS_GetProperty(cx, gobj, "Agtk", &nsval);
		JS::RootedValue rv(cx);
		auto sceneId = (_object->getScenePartObjectData() && !_object->getScenePartObjectData()->isStartPointObject() && cmd->getInstanceConfigurable()) ? _object->getSceneData()->getId() : -1;
		auto isCommonAction = this->getIsCommon();
		auto disappearFlag = _object->getDisappearFlag();
		auto actionId = disappearFlag ? -1 : isCommonAction ? this->getCommonActionSettingId() : this->getId();
		bool ret = false;
		if (nsval != JSVAL_VOID) {
			jsObj.set(nsval.toObjectOrNull());
			JS::RootedValue v(cx);
			JS_GetProperty(cx, jsObj, "scriptFunctions", &v);
			if (v.isObject()) {
				JS::RootedObject rscriptFunctions(cx, &v.toObject());
				JS_GetProperty(cx, rscriptFunctions, "execObjectActionCommandScriptEvaluate", &v);
				if (v.isObject()) {
					JS::RootedValue rexec(cx, v);
					jsval args[6];
					args[0] = JS::Int32Value(sceneId);
					args[1] = JS::Int32Value(_object->getObjectData()->getId());
					args[2] = JS::Int32Value(_object->getInstanceId());
					args[3] = JS::Int32Value(isCommonAction);
					args[4] = JS::Int32Value(actionId);
					args[5] = JS::Int32Value(commandData->getId());
					ret = JS_CallFunctionValue(cx, rscriptFunctions, rexec, JS::HandleValueArray::fromMarkedLocation(6, args), &rv);
				}
			}
		}
#else
		auto scriptingCore = ScriptingCore::getInstance();
		auto context = scriptingCore->getGlobalContext();
		JS::RootedValue rv(context);
		JS::MutableHandleValue mhv(&rv);
		auto script = String::createWithFormat("(function(){ var objectId = %d; var instanceId = %d; var actionId = %d; var commandId = %d; %s\n})()", _object->getObjectData()->getId(), _object->getInstanceId(), this->getId(), commandData->getId(), cmd->getScript());
		auto ret = ScriptingCore::getInstance()->evalString(script->getCString(), mhv);
#endif
		if (!ret) {
			//スクリプトエラー
			auto errorStr = String::createWithFormat("Runtime error in scriptEvaluate(objectId: %d, instanceId: %d, actionId: %d, commandId: %d, script: %s).", _object->getObjectData()->getId(), _object->getInstanceId(), this->getId(), commandData->getId(), cmd->getScript())->getCString();
			agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
			auto fp = GameManager::getScriptLogFp();
			if (fp) {
				fwrite(errorStr, 1, strlen(errorStr), fp);
				fwrite("\n", 1, 1, fp);
			}
#endif
			return kCommandBehaviorNext;
		}
		if (rv.isNumber()) {
			return JavascriptManager::getInt32(rv);
		}
	}
	return kCommandBehaviorNext;
}

void ObjectAction::execActionSoundStop(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandSoundStopData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandSoundStopData *>(commandData);
#endif
	CC_ASSERT(cmd);
	float fadeSeconds = cmd->getFadeout() ? (float)cmd->getDuration300() / 300.0f : 0.0f;
	auto audioManager = AudioManager::getInstance();
	switch (cmd->getSoundType()) {
	case agtk::data::ObjectCommandSoundStopData::kSoundSe: {
			auto _seId = cmd->getSeId();
			if (cmd->getStopOnlySoundByThisObject()) {
				// このオブジェクトが再生しているサウンドの停止
				if (_object != nullptr) {
					if (_seId != -1) {
						// 指定されたSEのみ停止
						_object->stopSeObject(_seId, fadeSeconds);
					}
					else {
						// オブジェクトが再生したSEをすべて停止
						_object->stopAllSeObject(fadeSeconds);
					}
				}
			}
			else {
				if (_seId != -1) {
					audioManager->stopSe(_seId, fadeSeconds);
				}
				else {
					audioManager->stopAllSe(fadeSeconds);
				}
			}
			break;
		}
	case agtk::data::ObjectCommandSoundStopData::kSoundVoice: {
			auto _voiceId = cmd->getVoiceId();
			if (cmd->getStopOnlySoundByThisObject()) {
				// このオブジェクトが再生しているサウンドの停止
				if (_object != nullptr) {
					if (_voiceId != -1) {
						// 指定された音声のみ停止
						_object->stopVoiceObject(_voiceId, fadeSeconds);
					}
					else {
						// オブジェクトが再生した音声をすべて停止
						_object->stopAllVoiceObject(fadeSeconds);
					}
				}
			}
			else {
				if (_voiceId != -1) {
					audioManager->stopVoice(_voiceId, fadeSeconds);
				}
				else {
					audioManager->stopAllVoice(fadeSeconds);
				}
			}
			break;
		}
	case agtk::data::ObjectCommandSoundStopData::kSoundBgm: {
			auto _bgmId = cmd->getBgmId();
			if (cmd->getStopOnlySoundByThisObject()) {
				// このオブジェクトが再生しているサウンドの停止
				if (_object != nullptr) {
					if (_bgmId != -1) {
						// 指定されたBGMのみ停止
						_object->stopBgmObject(_bgmId, fadeSeconds);
					}
					else {
						// オブジェクトが再生したBGMをすべて停止
						_object->stopAllBgmObject(fadeSeconds);
					}
				}
			}
			else {
				if (_bgmId != -1) {
					audioManager->stopBgm(_bgmId, fadeSeconds);
				}
				else {
					audioManager->stopAllBgm(fadeSeconds);
				}
			}
			break;
		}
	}
}

void ObjectAction::execActionMenuShow(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandMenuShowData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandMenuShowData *>(commandData);
#endif
	CC_ASSERT(cmd);
	auto scene = GameManager::getInstance()->getCurrentScene();
	auto layer = GameManager::getInstance()->getCurrentLayer();
	auto projectData = GameManager::getInstance()->getProjectData();
	auto menuSceneData = projectData->getMenuSceneData();
	if (cmd->getLayerId() < 0) {
		//設定無し。
		return;
	}
	auto menuLayerData = menuSceneData->getLayer(cmd->getLayerId());
	if (!menuLayerData) {
		return;
	}
	auto sceneSize = projectData->getSceneSize(menuSceneData);
	
	agtk::SceneLayer *menuLayer = scene->getMenuLayer(menuLayerData->getLayerId());
	if (menuLayer == nullptr) {
		//create menuLayer
		auto zOrder = scene->getTopPrioritySceneLayer() + 1;

		menuLayer = agtk::SceneLayer::createMenu(scene, menuSceneData, menuLayerData->getLayerId(), sceneSize, false);
		menuLayer->setType(agtk::SceneLayer::kTypeMenu);
		scene->getMenuLayerList()->setObject(menuLayer, menuLayerData->getLayerId());
		scene->getSceneTopMost()->addChild(menuLayer, zOrder);

		//set collision information.
		int tileWidth = projectData->getTileWidth();
		int tileHeight = projectData->getTileHeight();
		//auto sceneSize = projectData->getSceneSize(menuSceneData);
		int sceneHorzTileCount = 0;
		int sceneVertTileCount = 0;
		sceneHorzTileCount = ceil(sceneSize.width / tileWidth);
		sceneVertTileCount = ceil(sceneSize.height / tileHeight);

		//singleIdHashで未設定のオブジェクト
		{
			auto projectPlayData = GameManager::getInstance()->getPlayData();
			auto playObjectList = projectPlayData->getObjectList();
			auto scene = this->getScene();
			cocos2d::DictElement *el = nullptr;
			CCDICT_FOREACH(playObjectList, el) {
				// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto playObjectData = static_cast<agtk::data::PlayObjectData *>(el->getObject());
#else
				auto playObjectData = dynamic_cast<agtk::data::PlayObjectData *>(el->getObject());
#endif
				//単体として指定されている同オブジェクトのインスタンスのインスタンスIDが格納。
				auto variableData = playObjectData->getVariableData(agtk::data::kObjectSystemVariableSingleInstanceID);
				auto objectList = scene->getObjectAll(playObjectData->getId(), agtk::SceneLayer::kTypeMenu);
				if (objectList->count() > 0 && variableData->getValue() == 0.0f) {
					agtk::Object *object = nullptr;
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
						// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto p = static_cast<agtk::Object *>(ref);
#else
						auto p = dynamic_cast<agtk::Object *>(ref);
#endif
						if (object != nullptr) {
							if (p->getInstanceId() < object->getInstanceId()) {
								object = p;
							}
						}
						else {
							object = p;
						}
					}
					if (object) {
						variableData->setValue((double)object->getInstanceId());
					}
				}
			}
			//シーンに配置された同インスタンス数を設定。
			// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
			auto objectList = scene->getObjectAllReference(SceneLayer::kTypeMenu);
#else
			auto objectList = scene->getObjectAll(SceneLayer::kTypeMenu);
#endif
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(objectList, ref) {
				// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = dynamic_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				auto playObjectData = object->getPlayObjectData();
				auto count = scene->getObjectInstanceCount(object->getObjectData()->getId());
				playObjectData->setInstanceCount(count);
			}
		}

		//ACT2-4410 addCollisionDetection()は、createMenu()関数内で呼び出されているが、
		//メニューレイヤーをaddChild()した後で、さらにaddCollisionDetection()を呼び出してしまうと、CollisionComponentにノードが登録されず、当たり判定が正しく動作しなくなるためコメントアウトする。
		//menuLayer->addCollisionDetection();
	}
	else {
		menuLayer->getPositionValue()->setValue(cocos2d::Vec2(0, 0));
		menuLayer->getAlphaValue()->setValue(255);
	}

	//ACT2-4410 addCollisionDetection()は、createMenu()関数内で呼び出されているが、
	//メニューレイヤーをaddChild()した後で、さらにaddCollisionDetection()を呼び出してしまうと、CollisionComponentにノードが登録されず、当たり判定が正しく動作しなくなるためコメントアウトする。
	//menuLayer->addCollisionDetection();

	//演出時間
	int duration300 = cmd->getDuration300();
	if (cmd->getUseEffect()) {
		cocos2d::Vec2 startPosition;
		switch(cmd->getEffectType()) {
		case 0: startPosition.y = -sceneSize.height; break;//0: スライド上へ
		case 1: startPosition.y = sceneSize.height; break;//1: スライド下へ
		case 2: startPosition.x = sceneSize.width; break;//2: スライド左へ
		case 3: startPosition.x = -sceneSize.width; break;//3: スライド右へ
		}
		//表示するメニュー画面を選択
		auto positionValue = menuLayer->getPositionValue();
		positionValue->setValue(startPosition);
		positionValue->update(0.0f);
		positionValue->setValue(cocos2d::Vec2(0, 0), duration300 / 300.0f);
	}
	if (cmd->getFadein()) {
		//フェードイン
		menuLayer->setFade(0, 0);
		menuLayer->setFade(255, duration300 / 300.0f);
	}

	if (cmd->getUseEffect() || cmd->getFadein()) {
		if (_object->getSceneLayer() != menuLayer) {
			menuLayer->update(0.0f);
		}
	}
}

void ObjectAction::execActionMenuHide(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandMenuHideData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandMenuHideData *>(commandData);
#endif
	CC_ASSERT(cmd);
	auto scene = this->getScene();
	auto sceneData = scene->getSceneData();
	int initialMenuLayerId = sceneData->getInitialMenuLayerId();
	auto projectData = GameManager::getInstance()->getProjectData();
	auto menuSceneData = projectData->getMenuSceneData();

	auto sceneLayerList = cocos2d::__Array::create();
	if (cmd->getHideExceptInitial()) {
		//初期表示メニュー画面以外を非表示
		auto menuLayerList = scene->getMenuLayerList();
		cocos2d::DictElement *el;
		CCDICT_FOREACH(menuLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto menuLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
			auto menuLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
			auto layerId = menuLayer->getLayerId();
			if (menuLayer->isFadeout() || menuLayer->isSlideout()) {
				continue;
			}
			if (initialMenuLayerId != layerId && 
				layerId != agtk::data::SceneData::kHudMenuLayerId && layerId != agtk::data::SceneData::kHudTopMostLayerId) {
				sceneLayerList->addObject(menuLayer);
			}
		}
	}
	else {
		auto menuLayerList = scene->getMenuLayerList();
		if (menuLayerList->count() > 0) {
			auto menuLayerKeyList = menuLayerList->allKeys();
			cocos2d::Ref *ref;
			CCARRAY_FOREACH_REVERSE(menuLayerKeyList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto index = static_cast<cocos2d::__Integer *>(ref);
				auto menuLayer = static_cast<agtk::SceneLayer *>(menuLayerList->objectForKey(index->getValue()));
#else
				auto index = dynamic_cast<cocos2d::__Integer *>(ref);
				auto menuLayer = dynamic_cast<agtk::SceneLayer *>(menuLayerList->objectForKey(index->getValue()));
#endif
				auto layerId = menuLayer->getLayerId();
				if (layerId == agtk::data::SceneData::kHudMenuLayerId || layerId == agtk::data::SceneData::kHudTopMostLayerId) {
					continue;
				}
				if (menuLayer->isFadeout() || menuLayer->isSlideout()) {
					continue;
				}
				sceneLayerList->addObject(menuLayer);
				break;
			}
		}
	}
	auto sceneSize = projectData->getSceneSize(menuSceneData);

	//演出時間
	int duration300 = cmd->getDuration300();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(sceneLayerList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto menuLayer = static_cast<agtk::SceneLayer *>(ref);
#else
		auto menuLayer = dynamic_cast<agtk::SceneLayer *>(ref);
#endif
		//演出（スライド）
		if (cmd->getUseEffect()) {
			cocos2d::Vec2 startPosition;
			switch (cmd->getEffectType()) {
			case -1: menuLayer->setFade(0, 0); break;//設定無し。
			case 0: startPosition.y = sceneSize.height; break;//0: スライド上へ
			case 1: startPosition.y = -sceneSize.height; break;//1: スライド下へ
			case 2: startPosition.x = -sceneSize.width; break;//2: スライド左へ
			case 3: startPosition.x = sceneSize.width; break;//3: スライド右へ
			}
			//「設定無し」以外。
			if (0 <= cmd->getEffectType() && cmd->getEffectType() <= 3) {
				//表示するメニュー画面を選択
				auto positionValue = menuLayer->getPositionValue();
				positionValue->setValue(startPosition, duration300 / 300.0f);
			}
		}
		//フェードアウト
		if (cmd->getFadeout()) {
			menuLayer->setFade(0, duration300 / 300.0f, true);
		}
		//演出無しかつフェードアウト無し。
		if (!cmd->getUseEffect() && !cmd->getFadeout()) {
			menuLayer->setFade(0, 0, true);
		}
		menuLayer->setMenuWorkMargin(cmd->getDisableObjects() ? 2 : -1);
	}
}

void ObjectAction::execActionDisplayDirectionMove(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandDisplayDirectionMoveData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandDisplayDirectionMoveData *>(commandData);
#endif
	CC_ASSERT(cmd);

	//テンプレート移動中の場合はテンプレート移動を終了。
	auto objectTemplate = _object->getObjectTemplateMove();
	if (objectTemplate->isMoving()) {
		objectTemplate->end(true);
	}

	int moveDistance = cmd->getDistanceOverride() ? cmd->getMoveDistance() : -1;//移動距離
	_object->retrieveDisplayDirectionVariable();
	int dispDirectionId = _object->getDispDirection();
	if (dispDirectionId == 0 || dispDirectionId == -1) {
		dispDirectionId = _object->calcDispDirection();
	}
	dispDirectionId = cmd->getReverse() ? 10 - dispDirectionId : dispDirectionId;
	auto dispDirection = agtk::GetDirectionFromMoveDirectionId(dispDirectionId);
	if (_object->isAutoGeneration()) {//「回転の自動生成」の場合。
		auto player = _object->getPlayer();
		if (player != nullptr) {
			auto angle = player->getCenterRotation();
			if (cmd->getReverse()) {//逆方向。
				angle = agtk::GetDegree360(angle + 180.0f);
			}
			dispDirection = agtk::GetDirectionFromDegrees(angle);
		}
	}
	dispDirection = _object->directionCorrection(dispDirection);
	if (cmd->getReverse()) {//※表示方向の逆方向の場合は入力を無効にする。
		_object->setInputDirectionId(-1);
	}
	auto movement = _object->getObjectMovement();
	movement->setDisplayDistanceMove(dispDirection, moveDistance);
	//ジャンプ中の場合
	if (_object->_jumping) {
		movement->setFixedJumpDirectionId(dispDirectionId);
	}
	//移動速度を変更（％）をデフォルト値（100）に戻す。
	movement->setForceMoveChangeMoveSpeed();
}

void ObjectAction::execActionFileLoad(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandFileLoadData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandFileLoadData *>(commandData);
#endif

	struct Item{
		std::function<bool()> func;
		int bit;
	} const TABLE[] = {
		{ [&cmd]() { return cmd->getProjectCommonVariables(); },           GameManager::kLoadBit_CommonVariable },
		{ [&cmd]() { return cmd->getProjectCommonSwitches(); },            GameManager::kLoadBit_CommonSwitch },
		{ [&cmd]() { return cmd->getSceneAtTimeOfSave(); },                GameManager::kLoadBit_Scene },
		{ [&cmd]() { return cmd->getObjectsStatesInSceneAtTimeOfSave(); }, GameManager::kLoadBit_ObjectList },
	};

	int bit = 0;
	for (auto & elem : TABLE) {
		if (elem.func()) {
			bit |= elem.bit;
		}
	}
	if ( bit > 0 ) {
		GameManager::getInstance()->getPlayData()->getCommonSwitchData(agtk::data::EnumProjectSystemSwitch::kProjectSystemSwitchLoadFile)->setValue(true);
		GameManager::getInstance()->setLoadBit(bit);
		GameManager::getInstance()->setLoadEffectType(cmd->getEffectType());
		GameManager::getInstance()->setLoadEffectDuration300(cmd->getDuration300());
	}
}

void ObjectAction::execActionSoundPositionRemember(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandSoundPositionRememberData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandSoundPositionRememberData *>(commandData);
#endif
	CC_ASSERT(cmd);

	// サウンドの再生位置を取得
	float _currentTime = -1;
	auto audioManager = AudioManager::getInstance();
	switch (cmd->getSoundType()) {
	case agtk::data::ObjectCommandSoundPlayData::kSoundSe: {
		auto seList = audioManager->getSeIdList();
		if (seList != nullptr && seList->count() > 0) {
			// 最後に格納したSEを取得
			cocos2d::Ref* ref = seList->getLastObject();
			int _seId = dynamic_cast<Integer *>(ref)->getValue();
			_currentTime = audioManager->getSeCurrentTime(_seId);
		}
		break; }
	case agtk::data::ObjectCommandSoundPlayData::kSoundVoice: {
		auto voiceList = audioManager->getVoiceIdList();
		if (voiceList != nullptr && voiceList->count() > 0) {
			// 最後に格納したVoiceを取得
			cocos2d::Ref* ref = voiceList->getLastObject();
			int _voiceId = dynamic_cast<Integer *>(ref)->getValue();
			_currentTime = audioManager->getVoiceCurrentTime(_voiceId);
		}
		break; }
	case agtk::data::ObjectCommandSoundPlayData::kSoundBgm: {
		auto bgmList = audioManager->getBgmIdList();
		if (bgmList != nullptr && bgmList->count() > 0) {
			// 最後に格納したBGMを取得
			cocos2d::Ref* ref = bgmList->getLastObject();
			int _bgmId = dynamic_cast<Integer *>(ref)->getValue();
			_currentTime = audioManager->getBgmCurrentTime(_bgmId);
		}
		break; }
	}

	// 変数に保存
	auto projectPlayData = GameManager::getInstance()->getPlayData();
	if (cmd->getVariableObjectId() == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {//共通
		auto variableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, cmd->getVariableId());
		if (variableData) {
			variableData->setValue(_currentTime);
			projectPlayData->adjustCommonVariableData(variableData);
		}
	}
	else if (cmd->getVariableObjectId() > 0) {//オブジェクト共通
		auto projectPlayData = GameManager::getInstance()->getPlayData();
		switch (cmd->getVariableQualifierId()) {
		case agtk::data::ObjectCommandData::kQualifierSingle: {//単体
			auto singleInstanceData = projectPlayData->getVariableData(cmd->getVariableObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID);
			auto object = this->getTargetObjectInstanceId((int)singleInstanceData->getValue(), agtk::SceneLayer::kTypeAll);
			if (object) {
				auto playObjectData = object->getPlayObjectData();
				auto variableData = playObjectData->getVariableData(cmd->getVariableId());
				if (variableData) {
					variableData->setValue(_currentTime);
					playObjectData->adjustVariableData(variableData);
				}
			}
			break; }
		case agtk::data::ObjectCommandData::kQualifierWhole: {//全体
			auto objectList = this->getTargetObjectById(cmd->getVariableObjectId(), agtk::SceneLayer::kTypeAll);
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				auto playObjectData = object->getPlayObjectData();
				auto variableData = playObjectData->getVariableData(cmd->getVariableId());
				if (variableData) {
					variableData->setValue(_currentTime);
					playObjectData->adjustVariableData(variableData);
				}
			}
			break; }
		default: {
			if (cmd->getVariableQualifierId() >= 0) {
				auto objectList = this->getTargetObjectById(cmd->getVariableObjectId());
				cocos2d::Ref *ref;
				CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = static_cast<agtk::Object *>(ref);
#else
					auto object = dynamic_cast<agtk::Object *>(ref);
#endif
					if (object->getInstanceId() == cmd->getVariableQualifierId()) {
						auto playObjectData = object->getPlayObjectData();
						auto variableData = playObjectData->getVariableData(cmd->getVariableId());
						if (variableData) {
							variableData->setValue(_currentTime);
							playObjectData->adjustVariableData(variableData);
						}
					}
				}
			}
			else {
				CC_ASSERT(0);
			}
			break; }
		}
	}
	else if (cmd->getVariableObjectId() == agtk::data::ObjectCommandData::kSelfObject) {//自身のオブジェクト
		auto playData = _object->getPlayObjectData();
		auto variableData = playData->getVariableData(cmd->getVariableId());
		if (variableData) {
			variableData->setValue(_currentTime);
			playData->adjustVariableData(variableData);
		}
	}
	else if (cmd->getVariableObjectId() == agtk::data::ObjectCommandData::kLockedObject) {//ロックしたオブジェクト
		auto objectList = this->getTargetObjectLocked(agtk::SceneLayer::kTypeAll);
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			auto playObjectData = object->getPlayObjectData();
			auto variableData = playObjectData->getVariableData(cmd->getVariableId());
			if (variableData) {
				variableData->setValue(_currentTime);
				playObjectData->adjustVariableData(variableData);
			}
		}
	}
	else if (cmd->getVariableObjectId() == agtk::data::ObjectCommandData::kUnsetObject) {//未設定
		return;
	}
	else {
		//エラー
		CC_ASSERT(0);
	}
}

/**
* ロックを解除
* @param	commandData	コマンドデータ
*/
void ObjectAction::execActionObjectUnlock(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandObjectUnlockData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandObjectUnlockData *>(commandData);
#endif
	CC_ASSERT(cmd);
	auto scene = this->getScene();
	//ロックしているのオブジェクトを取得する。
	auto objectList = this->getTargetObjectLocked(agtk::SceneLayer::kTypeAll);

	//オブジェクトの種類で指定
	switch (cmd->getObjectType()) {
	// -----------------------------------------------
	// オブジェクトの種類で指定
	// -----------------------------------------------
	case agtk::data::ObjectCommandData::kObjectByGroup:
	{
		if (cmd->getObjectGroup() != agtk::data::ObjectCommandData::kObjectTypeAll) {
			for (int i = objectList->count() - 1; i >= 0; i--) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#else
				auto object = dynamic_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#endif
				if (cmd->getObjectGroup() != object->getObjectData()->getGroup()) {
					objectList->removeObjectAtIndex(i); // 対象じゃないものを除外
				}
			}
		}
		break;
	}

	// -----------------------------------------------
	// オブジェクトで指定
	// -----------------------------------------------
	case agtk::data::ObjectCommandData::kObjectById:
	{
		// オブジェクトID別
		switch (cmd->getObjectId())
		{
		// -----------------------------------------------
		// 設定無し
		// -----------------------------------------------
		case -1: {
			return;
		}

		// -----------------------------------------------
		// 指定のオブジェクト
		// -----------------------------------------------
		default:
		{
			// 指定のオブジェクト以外は対象リストから省く
			for (int i = objectList->count() - 1; i >= 0; i--) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#else
				auto object = dynamic_cast<agtk::Object *>(objectList->getObjectAtIndex(i));
#endif
				if (object->getObjectData()->getId() != cmd->getObjectId()) {
					objectList->removeObjectAtIndex(i);
				}
			}
		} break;
		}
		break;
	}
	default:CC_ASSERT(0);
	}

	// ロックを解除。
	{
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			auto playObjectData = object->getPlayObjectData();
			playObjectData->setObjectId(object->getObjectData()->getId());
			playObjectData->removeLocking(_object->getInstanceId());
			if (playObjectData->getLockingObjectIdList()->count() == 0) {
				playObjectData->setLockTarget(false);//ロック対象OFF
			}
		}
	}
}

/**
* アニメーションの素材セットを変更
* @param	commandData	コマンドデータ
*/
void ObjectAction::execActionResourceSetChange(agtk::data::ObjectCommandData *commandData)
{
	auto cmd = dynamic_cast<agtk::data::ObjectCommandResourceSetChangeData *>(commandData);
	CC_ASSERT(cmd);

	switch (cmd->getObjectId()) {
	case -1://設定無し。
		break;
	case agtk::data::ObjectCommandData::kSelfObject: {//自身のオブジェクト
		auto object = _object;
		auto player = _object->getPlayer();
		if (player) {
			player->setResourceSetId(cmd->getResourceSetId());
			object->setResourceSetId(cmd->getResourceSetId());
		}
		break; }
	default: {
		switch (cmd->getQualifierId()) {
		case agtk::data::ObjectCommandData::kQualifierSingle: {//単体
			auto objectId = cmd->getObjectId();
			auto projectPlayData = GameManager::getInstance()->getPlayData();
			auto singleInstanceData = projectPlayData->getVariableData(objectId, agtk::data::kObjectSystemVariableSingleInstanceID);
			auto object = this->getTargetObjectInstanceId((int)singleInstanceData->getValue());
			if (object != nullptr) {
				auto player = object->getPlayer();
				if (player) {
					player->setResourceSetId(cmd->getResourceSetId());
					object->setResourceSetId(cmd->getResourceSetId());
				}
			}
			break; }
		case agtk::data::ObjectCommandAttackSettingData::kQualifierWhole: {//全体
			auto objectList = this->getTargetObjectById(cmd->getObjectId());
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
				auto object = dynamic_cast<agtk::Object *>(ref);
				auto player = object->getPlayer();
				if (player) {
					player->setResourceSetId(cmd->getResourceSetId());
					object->setResourceSetId(cmd->getResourceSetId());
				}
			}
			break; }
		default: {
			if (cmd->getQualifierId() >= 0) {
				auto objectList = this->getTargetObjectById(cmd->getObjectId());
				cocos2d::Ref *ref;
				CCARRAY_FOREACH(objectList, ref) {
					auto object = dynamic_cast<agtk::Object *>(ref);
					auto player = object->getPlayer();
					if (player) {
						player->setResourceSetId(cmd->getResourceSetId());
						object->setResourceSetId(cmd->getResourceSetId());
					}
				}
			}
			break; }
		}
		break; }
	}
}

/**
* データベースの値反映
* @param	commandData	コマンドデータ
*/
void ObjectAction::execActionDatabaseReflect(agtk::data::ObjectCommandData *commandData)
{
	auto cmd = dynamic_cast<agtk::data::ObjectCommandDatabaseReflectData *>(commandData);
	CC_ASSERT(cmd);

	// 反映するデータベースのIDを取得
	auto databaseId = cmd->getDatabaseId();
	// レコード単位で反映するかどうか
	bool fromRow = cmd->getFromRow();

	// データベースのデータを取得する
	auto projectData = GameManager::getInstance()->getProjectData();
	auto databaseData = projectData->getDatabaseData(databaseId);

	if (cmd->getFromObject()) {
		switch (cmd->getObjectId()) {
		case -1://設定無し。
			break;
		case agtk::data::ObjectCommandData::kSelfObject: {//自身のオブジェクト
			auto object = _object;
			auto objectData = object->getObjectData();
			auto dbId = objectData->getDatabaseId();
			databaseData = projectData->getDatabaseData(dbId);
			break; }
		default: {
			auto objectList = this->getTargetObjectById(cmd->getObjectId());
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(objectList, ref) {
				auto object = dynamic_cast<agtk::Object *>(ref);
				auto objectData = object->getObjectData();
				auto dbId = objectData->getDatabaseId();
				databaseData = projectData->getDatabaseData(dbId);
			}
			break; }
		}
	}
	
	if (!databaseData)	return;

	auto rowIndex = -1;
	auto columnIndex = -1;

	if (cmd->getRowIndexFromName()) {
		// レコードをIDで指定している
		rowIndex = cmd->getRowIndex();
	}
	else {
		if (cmd->getRowNumberFromValue()) {
			// レコードを数値で指定している
			rowIndex = cmd->getRowIndex();
	}
	else {
		// レコードを変数で指定している
			rowIndex = getReflectVariableValue(cmd->getRowVariableObjectId(), cmd->getRowVariableId(), cmd->getRowVariableQualifierId());
	}
	}
	if (cmd->getColumnIndexFromName()) {
		// カラムIDで指定している
		columnIndex = cmd->getColumnIndex();
	}
	else {
		if (cmd->getColumnNumberFromValue()) {
			// カラムを数値で指定している
			columnIndex = cmd->getColumnIndex();
		}
		else {
		// カラムを変数で指定している
		columnIndex = getReflectVariableValue(cmd->getColumnVariableObjectId(), cmd->getColumnVariableId(), cmd->getColumnVariableQualifierId());
	}
	}
	
	if (fromRow) {
		// レコード単位で値変更

		// カラムのリストを取得
		for (auto i = 0; i < databaseData->getColumnList().size(); i++) {
			if (RESOURCE_SET_STR == databaseData->getColumnList()[i]) {
				// 素材セットを変更する
				reflectDatabaseValue(databaseData, agtk::data::DatabaseData::DatabaseType::kResourceSet, cmd, i, rowIndex);
			}
			else if (MOTION_STR == databaseData->getColumnList()[i]) {
				// モーションを変更する
				reflectDatabaseValue(databaseData, agtk::data::DatabaseData::DatabaseType::kMotion, cmd, i, rowIndex);
			}
			else {
				// その他変数を変更する
				reflectDatabaseValueInVariable(databaseData, agtk::data::DatabaseData::DatabaseType::kMotion, cmd, i, rowIndex);
			}
		}
	}
	else {
		// フィールドで変更する
		// 反映する変数を取得する
		auto reflectVariable = cmd->getReflectVariableId();
		if (0 == reflectVariable) {
			auto columnName = databaseData->getDatabaseColumnName(columnIndex);
			if (RESOURCE_SET_STR == columnName) {
				// 素材セットを変更する
				reflectDatabaseValue(databaseData, agtk::data::DatabaseData::DatabaseType::kResourceSet, cmd, columnIndex, rowIndex);
			}
			else if (MOTION_STR == columnName) {
				// モーションを変更する
				reflectDatabaseValue(databaseData, agtk::data::DatabaseData::DatabaseType::kMotion, cmd, columnIndex, rowIndex);
			}
		}
		else {
			// その他変数を変更する
			reflectDatabaseValueInVariable(databaseData, agtk::data::DatabaseData::DatabaseType::kMotion, cmd, columnIndex, rowIndex);
		}
		
	}
}

void ObjectAction::reflectDatabaseValue(agtk::data::DatabaseData *data, agtk::data::DatabaseData::DatabaseType type, agtk::data::ObjectCommandDatabaseReflectData *cmd, int columnIndex, int rowIndex)
{
	// 値を取得する
	auto value = data->getDatabaseValue(columnIndex, rowIndex);
	
	switch (cmd->getReflectObjectId()) {
	case -1://設定無し。
		break;
	case agtk::data::ObjectCommandData::kSelfObject: {//自身のオブジェクト
		auto object = _object;
		setDatabaseValueInType(type, object, value.c_str());
		break; }
	default: {
		switch (cmd->getReflectQualifierId()) {
		case agtk::data::ObjectCommandData::kQualifierSingle: {//単体
			auto objectId = cmd->getReflectObjectId();
			auto projectPlayData = GameManager::getInstance()->getPlayData();
			auto singleInstanceData = projectPlayData->getVariableData(objectId, agtk::data::kObjectSystemVariableSingleInstanceID);
			auto object = this->getTargetObjectInstanceId((int)singleInstanceData->getValue());
			if (object != nullptr) {
				setDatabaseValueInType(type, object, value.c_str());
			}
			break; }
		case agtk::data::ObjectCommandAttackSettingData::kQualifierWhole: {//全体
			auto objectList = this->getTargetObjectById(cmd->getReflectObjectId());
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
				auto object = dynamic_cast<agtk::Object *>(ref);
				setDatabaseValueInType(type, object, value.c_str());
			}
			break; }
		default: {
			if (cmd->getReflectQualifierId() >= 0) {
				auto objectList = this->getTargetObjectById(cmd->getReflectObjectId());
				cocos2d::Ref *ref;
				CCARRAY_FOREACH(objectList, ref) {
					auto object = dynamic_cast<agtk::Object *>(ref);
					setDatabaseValueInType(type, object, value.c_str());
				}
			}
			break; }
		}
		break; }
	}
}

int ObjectAction::getReflectVariableValue(int objectId, int variableId, int qualifierId)
{
	auto projectPlayData = GameManager::getInstance()->getPlayData();
	if (objectId == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {//共通
		auto variableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, variableId);
		if (variableData) {
			return variableData->getValue();
		}
	}
	else if (objectId > 0) {//オブジェクト共通
		switch (qualifierId) {
		case agtk::data::ObjectCommandData::kQualifierSingle: {//単体
			auto singleInstanceData = projectPlayData->getVariableData(objectId, agtk::data::kObjectSystemVariableSingleInstanceID);
			auto object = this->getTargetObjectInstanceId((int)singleInstanceData->getValue(), agtk::SceneLayer::kTypeAll);
			if (object) {
				auto playObjectData = object->getPlayObjectData();
				auto variableData = playObjectData->getVariableData(variableId);
				if (variableData) {
					return variableData->getValue();
				}
			}
			break; }
		case agtk::data::ObjectCommandData::kQualifierWhole: {//全体
			// 全体の時の値注意
			auto objectList = this->getTargetObjectById(objectId, agtk::SceneLayer::kTypeAll);
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
				auto object = dynamic_cast<agtk::Object *>(ref);
				auto playObjectData = object->getPlayObjectData();
				auto variableData = playObjectData->getVariableData(variableId);
				if (variableData) {
					return variableData->getValue();
				}
			}
			break; }
		default: {
			if (qualifierId >= 0) {
				auto objectList = this->getTargetObjectById(objectId);
				cocos2d::Ref *ref;
				CCARRAY_FOREACH(objectList, ref) {
					auto object = dynamic_cast<agtk::Object *>(ref);
					if (object->getInstanceId() == qualifierId) {
						auto playObjectData = object->getPlayObjectData();
						auto variableData = playObjectData->getVariableData(variableId);
						if (variableData) {
							return variableData->getValue();
						}
					}
				}
			}
			else {
				CC_ASSERT(0);
			}
			break; }
		}
	}
	else if (objectId == agtk::data::ObjectCommandData::kSelfObject) {//自身のオブジェクト
		auto playData = _object->getPlayObjectData();
		auto variableData = playData->getVariableData(variableId);
		if (variableData) {
			return variableData->getValue();
		}
	}
	else if (objectId == agtk::data::ObjectCommandData::kLockedObject) {//ロックしたオブジェクト
		auto objectList = this->getTargetObjectLocked(agtk::SceneLayer::kTypeAll);
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
			auto object = dynamic_cast<agtk::Object *>(ref);
			auto playObjectData = object->getPlayObjectData();
			auto variableData = playObjectData->getVariableData(variableId);
			if (variableData) {
				return variableData->getValue();
			}
		}
	}
	else if (objectId == agtk::data::ObjectCommandData::kUnsetObject) {//未設定
		return 0;
	}
	else {
		//エラー
		CC_ASSERT(0);
	}
	return 0;
}

void ObjectAction::reflectDatabaseValueInVariable(agtk::data::DatabaseData *data, agtk::data::DatabaseData::DatabaseType type, agtk::data::ObjectCommandDatabaseReflectData *cmd, int columnIndex, int rowIndex)
{
	//変数を変更
	auto projectPlayData = GameManager::getInstance()->getPlayData();
	if (cmd->getReflectObjectId() == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {//共通
		setVariableDataFromDatabase(data, nullptr, cmd, columnIndex, rowIndex);
	}
	else if (cmd->getReflectObjectId() > 0) {//オブジェクト共通
		auto projectPlayData = GameManager::getInstance()->getPlayData();
		switch (cmd->getReflectQualifierId()) {
		case agtk::data::ObjectCommandData::kQualifierSingle: {//単体
			auto singleInstanceData = projectPlayData->getVariableData(cmd->getReflectObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID);
			auto object = this->getTargetObjectInstanceId((int)singleInstanceData->getValue(), agtk::SceneLayer::kTypeAll);
			if (object) {
				setVariableDataFromDatabase(data, object, cmd, columnIndex, rowIndex);
			}
			break; }
		case agtk::data::ObjectCommandData::kQualifierWhole: {//全体 
			auto objectList = this->getTargetObjectById(cmd->getReflectObjectId(), agtk::SceneLayer::kTypeAll);
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
				auto object = dynamic_cast<agtk::Object *>(ref);
				auto playObjectData = object->getPlayObjectData();
				if (object) {
					setVariableDataFromDatabase(data, object, cmd, columnIndex, rowIndex);
				}
			}
			break; }
		default: {
			if (cmd->getReflectQualifierId() >= 0) {
				auto objectList = this->getTargetObjectById(cmd->getReflectObjectId());
				cocos2d::Ref *ref;
				CCARRAY_FOREACH(objectList, ref) {
					auto object = dynamic_cast<agtk::Object *>(ref);
					if (object->getInstanceId() == cmd->getReflectQualifierId()) {
						setVariableDataFromDatabase(data, object, cmd, columnIndex, rowIndex);
					}
				}
			}
			else {
				CC_ASSERT(0);
			}
			break; }
		}
	}
	else if (cmd->getReflectObjectId() == agtk::data::ObjectCommandData::kSelfObject) {//自身のオブジェクト
		setVariableDataFromDatabase(data, _object, cmd, columnIndex, rowIndex);
	}
	else if (cmd->getReflectObjectId() == agtk::data::ObjectCommandData::kLockedObject) {//ロックしたオブジェクト
		auto objectList = this->getTargetObjectLocked(agtk::SceneLayer::kTypeAll);
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
			auto object = dynamic_cast<agtk::Object *>(ref);
			setVariableDataFromDatabase(data, object, cmd, columnIndex, rowIndex);
		}
	}
	else if (cmd->getReflectObjectId() == agtk::data::ObjectCommandData::kParentObject) {//親オブジェクト
																						 // 親オブジェクトの変数を変更
		auto parentObjectInstanceId = _object->getPlayObjectData()->getParentObjectInstanceId();
		if (parentObjectInstanceId == -1) {
			return;
		}
		auto parentObject = this->getTargetObjectInstanceId(parentObjectInstanceId, agtk::SceneLayer::kTypeAll);
		if (parentObject == nullptr) {
			return;
		}
		setVariableDataFromDatabase(data, parentObject, cmd, columnIndex, rowIndex);
	}
	else if (cmd->getReflectObjectId() == agtk::data::ObjectCommandData::kUnsetObject) {//未設定
		return;
	}
	else {
		//エラー
		CC_ASSERT(0);
	}

	//変数変更時のオブジェクトに対して変更処理。
	GameManager::getInstance()->updateByChangingVariableAndSwitch();
}

void ObjectAction::setVariableDataFromDatabase(agtk::data::DatabaseData *data, agtk::Object * object, agtk::data::ObjectCommandDatabaseReflectData * cmd, int columnIndex, int rowIndex)
{
	// 反映する時の計算方法を考慮して値を算出
	std::function<double(agtk::data::PlayVariableData *, double)> calcOperator = [&](agtk::data::PlayVariableData *variableData, double value) {
		auto _value = variableData->getValue();

		switch (cmd->getReflectVariableAssignOperator()) {
		case agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignOperatorSet:
			return value;
		case agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignOperatorAdd:
			return _value + value;
		case agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignOperatorSub:
			return _value - value;
		case agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignOperatorMul:
			return _value * value;
		case agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignOperatorDiv: {
			if (value == 0) return std::nan("1");
			return _value / value; }
		case agtk::data::ObjectCommandSwitchVariableChangeData::kVariableAssignOperatorMod: {
			if (value == 0) return std::nan("1");
			return (double)((int)_value % (int)value); }
		}
		CC_ASSERT(0);
		return 0.0;
	};

	// 値反映
	std::function<void(agtk::data::PlayVariableData *)> setVariableData = [&](agtk::data::PlayVariableData *variableData) {
		if (variableData == nullptr) {
			//CC_ASSERT(0);//TODO:バグ
			return;
		}
		string valStr = data->getDatabaseValue(columnIndex, rowIndex);
		double val = -1;
		auto columnName = data->getDatabaseColumnName(columnIndex);
		if (RESOURCE_SET_STR == columnName) {
			// 素材セットIDを取得する
			auto player = object->getPlayer();
			if (player) {
				if (0 < valStr.length()) {
					val = player->getResourceSetIdByName(valStr);
				}
			}
		}
		else if (MOTION_STR == columnName) {
			// モーションIDを取得する
			auto player = object->getPlayer();
			if (player) {
				auto motionId = 0;
				if (0 < valStr.length()) {
					auto basePlayer = object->getBasePlayer();
					auto animationData = basePlayer->getAnimationMotionList();
					if (animationData) {
						cocos2d::DictElement *el;
						CCDICT_FOREACH(animationData, el) {
							auto motion = dynamic_cast<agtk::AnimationMotion *>(el->getObject());
							if (!strcmp(valStr.c_str(), motion->getName())) {
								val = motion->getMotionData()->getId();
								break;
							}
						}
					}
				}
			}
		}
		else {
			if (std::regex_match(valStr, std::regex("(\\+|-)?[0-9]*(\\.?([0-9]+))$"))){
				val = std::stod(valStr);
			}
		}

		if (-1 != val) {
			auto value = calcOperator(variableData, val);
			variableData->setValue(value);
		}
	};

	agtk::data::PlayVariableData * variableData = nullptr;
	if (nullptr == object) {
		// プロジェクト共通変数
		auto projectPlayData = GameManager::getInstance()->getPlayData();
		if (cmd->getFromRow()) {
			variableData = projectPlayData->getVariableDataByName(agtk::data::PlayData::COMMON_PLAY_DATA_ID, data->getDatabaseColumnName(columnIndex).c_str());
		}
		else {
		variableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, cmd->getReflectVariableId());
		}
	}
	else {
	auto playObjectData = object->getPlayObjectData();
		if (cmd->getFromRow()) {
		variableData = playObjectData->getVariableDataByName(data->getDatabaseColumnName(columnIndex).c_str());
	}
	else {
		variableData = playObjectData->getVariableData(cmd->getReflectVariableId());
	}
	}
	if(nullptr != variableData)
	setVariableData(variableData);
}

void ObjectAction::setDatabaseValueInType(agtk::data::DatabaseData::DatabaseType type, agtk::Object * object, const char* value)
{
	switch (type) {
	case agtk::data::DatabaseData::DatabaseType::kResourceSet: {//素材セット
		setResourceSet(object, value);
		break; }
	case agtk::data::DatabaseData::DatabaseType::kMotion: {//モーション
		setMotion(object, value);
		break; }
	default: {
		break; }
	}
}

void ObjectAction::setResourceSet(agtk::Object * object, const char* resouceSetName)
{
	auto player = object->getPlayer();
	if (player) {
		if (0 < strlen(resouceSetName)) {
			player->setResourceSetName(resouceSetName);
			object->setResourceSetName(resouceSetName);
		}		
	}
}

void ObjectAction::setMotion(agtk::Object * object, const char* motionName)
{
	auto player = object->getPlayer();
	if (player) {
		auto motionId = 0;
		if (0 < strlen(motionName)) {
			auto basePlayer = object->getBasePlayer();
			auto animationData = basePlayer->getAnimationMotionList();
			auto newDirectionNo = -1;
			if (animationData) {
				cocos2d::DictElement *el;
				CCDICT_FOREACH(animationData, el) {
					auto motion = dynamic_cast<agtk::AnimationMotion *>(el->getObject());
					if (!strcmp(motionName, motion->getName())) {
						// ACT2-6164
						// モーション引き継ぎ設定を使用する時currentDirectionNoはモーション間で互換しないことがあるため、表示方向Bitから再取得
						auto motionData = motion->getMotionData();
						auto directionBit = object->getDispDirectionBit();
						auto directionData = motionData->getDirectionDataByDirectionBit(directionBit);

						motionId = motionData->getId();
						newDirectionNo = directionData->getId();
						break;
					}
				}				
			}

			if (0 != motionId) {
				if (newDirectionNo == -1) {
					player->play(motionId, player->getCurrentDirectionNo());
				} else {
					player->play(motionId, newDirectionNo);
				}
				object->setCurrentAnimMotionId(motionId);
				// ACT2-6400 データベースからのモーション変更エラー対応
				object->setBasePlayer(player->getBasePlayer());
			}
		}
	}
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
/**
* カスタム
* @param	commandData	コマンドデータ
*/
int ObjectAction::execActionCustom(agtk::data::ObjectCommandData *commandData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto cmd = static_cast<agtk::data::ObjectCommandCustomData *>(commandData);
#else
	auto cmd = dynamic_cast<agtk::data::ObjectCommandCustomData *>(commandData);
#endif
	CC_ASSERT(cmd);

	int pluginId = (commandData->getCommandType() - agtk::data::ObjectCommandData::kCustomHead) / agtk::data::ObjectCommandData::kPluginCustomMax;
	int index = (commandData->getCommandType() - agtk::data::ObjectCommandData::kCustomHead) % agtk::data::ObjectCommandData::kPluginCustomMax;
	//pluginIdのindex番目の実行アクションを実行する。
#if 1
	//JSB_AUTOCOMPARTMENT_WITH_GLOBAL_OBJCET
	auto sc = ScriptingCore::getInstance();
	auto cx = sc->getGlobalContext();
	auto _global = sc->getGlobalObject();
	JS::RootedObject gobj(cx, _global);
	JSAutoCompartment ac(cx, gobj);

	JS::RootedObject ns(cx);
	std::string name("Agtk");
	JS::MutableHandleObject jsObj = &ns;
	JS::RootedValue nsval(cx);
	JS_GetProperty(cx, gobj, name.c_str(), &nsval);
	if (nsval != JSVAL_VOID) {
		jsObj.set(nsval.toObjectOrNull());
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsObj, "plugins", &v);
		if (v.isObject()) {
			JS::RootedObject rplugins(cx, &v.toObject());
			JS_GetProperty(cx, rplugins, "execActionCommand", &v);
			if (v.isObject()) {
				JS::RootedValue rexecActionCommand(cx, v);
				jsval args[7];
				args[0] = JS::Int32Value(pluginId);
				args[1] = JS::Int32Value(index);
				args[2] = std_string_to_jsval(cx, cmd->getValueJson());
				args[3] = JS::Int32Value(_objectData->getId());
				args[4] = JS::Int32Value(_object->getInstanceId());
				args[5] = JS::Int32Value(this->getId());
				args[6] = JS::Int32Value(commandData->getId());
				JS::RootedValue rval(cx);
				auto result = JS_CallFunctionValue(cx, rplugins, rexecActionCommand, JS::HandleValueArray::fromMarkedLocation(7, args), &rval);
				if (!result) {
					//スクリプトエラー
					return kCommandBehaviorNext;
				}
				if (rval.isNumber()) {
					return JavascriptManager::getInt32(rval);
				}
			}

		}
	}
#else
	auto scriptingCore = ScriptingCore::getInstance();
	auto context = scriptingCore->getGlobalContext();
	JS::RootedValue rv(context);
	JS::MutableHandleValue mhv(&rv);
	auto script = String::createWithFormat("(function(){ var plugin = Agtk.plugins.getById(%d); if(plugin != null){ if(plugin.execActionCommand){ return plugin.execActionCommand(%d, %s, %d, %d, %d, %d); } } })()", pluginId, index, cmd->getValueJson(), _objectData->getId(), _object->getInstanceId(), this->getId(), commandData->getId());
	//CCLOG("script: %s", script->getCString());
	auto ret = ScriptingCore::getInstance()->evalString(script->getCString(), mhv);
	if (!ret) {
		//スクリプトエラー
		return kCommandBehaviorNext;
	}
	if (rv.isNumber()) {
		return JavascriptManager::getInt32(rv);
	}
#endif
	return kCommandBehaviorNext;
}

cocos2d::__Array *ObjectAction::getTargetObjectByGroup(int group, int sceneLayerType)
{
	auto scene = this->getScene();
	auto sceneLayer = this->getSceneLayer();
	if (sceneLayerType < 0) sceneLayerType = sceneLayer->getType();
	auto objectList = cocos2d::__Array::create();
	cocos2d::Ref *ref = nullptr;
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	auto objectAllList = scene->getObjectAllReference((agtk::SceneLayer::EnumType)sceneLayerType);
#else
	auto objectAllList = scene->getObjectAll((agtk::SceneLayer::EnumType)sceneLayerType);
#endif
	CCARRAY_FOREACH(objectAllList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object = static_cast<agtk::Object *>(ref);
#else
		auto object = dynamic_cast<agtk::Object *>(ref);
#endif
		auto objectData = object->getObjectData();
		if (objectData->getGroup() == group || group == agtk::data::ObjectData::kObjGroupAll ) {
			objectList->addObject(object);
		}
	}
	return objectList;
}

cocos2d::__Array *ObjectAction::getTargetObjectById(int id, int sceneLayerType)
{
	auto scene = this->getScene();
	auto sceneLayer = this->getSceneLayer();
	auto objectList = cocos2d::__Array::create();
	if (sceneLayerType < 0) sceneLayerType = sceneLayer->getType();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
	auto objectAll = scene->getObjectAll(id, (agtk::SceneLayer::EnumType)sceneLayerType);
	objectList->addObjectsFromArray(objectAll);
#else
#ifdef USE_SAR_OPTIMIZE_1
	auto objectAllList = scene->getObjectAllReference((agtk::SceneLayer::EnumType)sceneLayerType);
#else
	auto objectAllList = scene->getObjectAll((agtk::SceneLayer::EnumType)sceneLayerType);
#endif
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(objectAllList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object = static_cast<agtk::Object *>(ref);
#else
		auto object = dynamic_cast<agtk::Object *>(ref);
#endif
		auto objectData = object->getObjectData();
		if (objectData->getId() == id) {
			objectList->addObject(object);
		}
	}
#endif
	return objectList;
}

cocos2d::__Array *ObjectAction::getTargetObjectLocked(int sceneLayerType)
{
	auto scene = this->getScene();
	auto sceneLayer = this->getSceneLayer();
	if (sceneLayerType < 0) sceneLayerType = sceneLayer->getType();
	return scene->getObjectAllLocked(_object->getInstanceId(), (agtk::SceneLayer::EnumType)sceneLayerType);
}

agtk::Object *ObjectAction::getTargetObjectInstanceId(int instanceId, int sceneLayerType)
{
	auto scene = this->getScene();
	auto sceneLayer = this->getSceneLayer();
	if (sceneLayerType < 0) sceneLayerType = sceneLayer->getType();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
	if (instanceId < 0) {
		return nullptr;
	}
	auto object = scene->getObjectInstance(-1, instanceId, (agtk::SceneLayer::EnumType)sceneLayerType);
	return object;
#else
#ifdef USE_SAR_OPTIMIZE_1
	auto allObjects = scene->getObjectAllReference((agtk::SceneLayer::EnumType)sceneLayerType);
#else
	auto allObjects = scene->getObjectAll((agtk::SceneLayer::EnumType)sceneLayerType);
#endif
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(allObjects, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object = static_cast<agtk::Object *>(ref);
#else
		auto object = dynamic_cast<agtk::Object *>(ref);
#endif
		if (object->getInstanceId() == instanceId) {
			return object;
		}
	}
	return nullptr;
#endif
}

int ObjectAction::getId()
{
	CC_ASSERT(_objectActionData);
	return this->getObjectActionData()->getId();
}

bool ObjectAction::checkInputCondition(agtk::data::ObjectActionLinkData *objectActionLinkData)
{
	bool result = false;
	// オブジェクトが360度ループを自動移動中はキー入力アクションを受け付けない
	if (_object->getObjectLoopMove()->getMovingAuto()) {
		return false;
	}

	auto inputManager = InputManager::getInstance();

	//何も入力がされなかった
	if (objectActionLinkData->getNoInput() && inputManager->isNoneInput(_object->getControllerId())) {
		return true;
	}

	//入力操作がされた(false)
	if (objectActionLinkData->getUseInput() == false) {
		return false;
	}

	std::function<bool(agtk::data::ObjectInputConditionData::EnumTriggerType, int, int, int, int)> isInputState = [&](agtk::data::ObjectInputConditionData::EnumTriggerType type, int keyCode, int id, int instanceId, int acceptFrameCount) {
		switch (type) {
		case agtk::data::ObjectInputConditionData::kTriggerPressed: return inputManager->isPressed(keyCode, id); break;//押された
		case agtk::data::ObjectInputConditionData::kTriggerJustPressed: return inputManager->isTriggered(keyCode, id, type, instanceId, acceptFrameCount); break;//押された瞬間
		case agtk::data::ObjectInputConditionData::kTriggerJustReleased: return inputManager->isReleased(keyCode, id, type, instanceId, acceptFrameCount); break;//離された瞬間
		case agtk::data::ObjectInputConditionData::kTriggerReleased: return inputManager->isReleasing(keyCode, id); break;//離されている
		default:CC_ASSERT(0);
		}
		return false;
	};

	auto inputConditionGroupList = objectActionLinkData->getInputConditionGroupList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(inputConditionGroupList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto inputConditionList = static_cast<cocos2d::__Array *>(ref);
#else
		auto inputConditionList = dynamic_cast<cocos2d::__Array *>(ref);
#endif
		cocos2d::Ref *ref2 = nullptr;
		int retCount = 0;
		CCARRAY_FOREACH(inputConditionList, ref2) {
			bool ret = false;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto inputConditionData = static_cast<agtk::data::ObjectInputConditionData *>(ref2);
#else
			auto inputConditionData = dynamic_cast<agtk::data::ObjectInputConditionData *>(ref2);
#endif
			agtk::data::ObjectInputConditionData::EnumTriggerType triggerType = inputConditionData->getTriggerType();
			if (inputConditionData->getUseKey()) {//入力に使う操作キー
				ret = isInputState(triggerType, inputConditionData->getOperationKeyId(), _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount());
			}
			else {
				switch (inputConditionData->getDirectionInputType()) {
				case agtk::data::ObjectInputConditionData::kDirectionInputCross://方向キー
																				//左下
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionLeftDown) {
						ret |= (isInputState(triggerType, InputController::Left, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()))
							&& (isInputState(triggerType, InputController::Down, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//下
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionDown) {
						ret |= (isInputState(triggerType, InputController::Down, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//右下
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionRightDown) {
						ret |= (isInputState(triggerType, InputController::Right, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()))
							&& (isInputState(triggerType, InputController::Down, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//左
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionLeft) {
						ret |= (isInputState(triggerType, InputController::Left, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//右
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionRight) {
						ret |= (isInputState(triggerType, InputController::Right, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//左上
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionLeftUp) {
						ret |= (isInputState(triggerType, InputController::Left, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()))
							&& (isInputState(triggerType, InputController::Up, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//上
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionUp) {
						ret |= (isInputState(triggerType, InputController::Up, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//右上
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionRightUp) {
						ret |= (isInputState(triggerType, InputController::Right, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()))
							&& (isInputState(triggerType, InputController::Up, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					break;
				case agtk::data::ObjectInputConditionData::kDirectionInputLeftStick://左スティック
																					//左下
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionLeftDown) {
						ret |= (isInputState(triggerType, InputController::LeftStickLeft, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()))
							&& (isInputState(triggerType, InputController::LeftStickDown, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//下
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionDown) {
						ret |= (isInputState(triggerType, InputController::LeftStickDown, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//右下
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionRightDown) {
						ret |= (isInputState(triggerType, InputController::LeftStickRight, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()))
							&& (isInputState(triggerType, InputController::LeftStickDown, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//左
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionLeft) {
						ret |= (isInputState(triggerType, InputController::LeftStickLeft, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//右
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionRight) {
						ret |= (isInputState(triggerType, InputController::LeftStickRight, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//左上
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionLeftUp) {
						ret |= (isInputState(triggerType, InputController::LeftStickLeft, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()))
							&& (isInputState(triggerType, InputController::LeftStickUp, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//上
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionUp) {
						ret |= (isInputState(triggerType, InputController::LeftStickUp, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//右上
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionRightUp) {
						ret |= (isInputState(triggerType, InputController::LeftStickRight, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()))
							&& (isInputState(triggerType, InputController::LeftStickUp, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					break;
				case agtk::data::ObjectInputConditionData::kDirectionInputRightStick://右スティック
																					 //左下
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionLeftDown) {
						ret |= (isInputState(triggerType, InputController::RightStickLeft, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()))
							&& (isInputState(triggerType, InputController::RightStickDown, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//下
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionDown) {
						ret |= (isInputState(triggerType, InputController::RightStickDown, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//右下
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionRightDown) {
						ret |= (isInputState(triggerType, InputController::RightStickRight, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()))
							&& (isInputState(triggerType, InputController::RightStickDown, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//左
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionLeft) {
						ret |= (isInputState(triggerType, InputController::RightStickLeft, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//右
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionRight) {
						ret |= (isInputState(triggerType, InputController::RightStickRight, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//左上
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionLeftUp) {
						ret |= (isInputState(triggerType, InputController::RightStickLeft, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()))
							&& (isInputState(triggerType, InputController::RightStickUp, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//上
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionUp) {
						ret |= (isInputState(triggerType, InputController::RightStickUp, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					//右上
					if (inputConditionData->getDirectionBit() & 1 << InputManager::kDirectionRightUp) {
						ret |= (isInputState(triggerType, InputController::RightStickRight, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()))
							&& (isInputState(triggerType, InputController::RightStickUp, _object->getControllerId(), _object->getInstanceId(), inputConditionData->getAcceptFrameCount()));
					}
					break;
				}
			}
			if (ret) retCount++;
		}
		if (retCount == inputConditionList->count()) {
			result = true;
		}
	}
	return result;
}

int ObjectAction::checkLinkCondition(agtk::data::ObjectActionLinkData *objectActionLinkData)
{
	//[戻り値]
	// -1 : その他の条件設定が無い。
	//  0(false) or 1(true) : その他の条件設定が有る。

	auto linkConditionList = objectActionLinkData->getLinkConditionList();
	cocos2d::DictElement *el = nullptr;
	if (linkConditionList->count() == 0) {
		return -1;
	}

	//無効数をチェック
	{
		int ignoredCount = 0;
		int maxLinkCount = 0;
		auto linkConditionGroupList = objectActionLinkData->getLinkConditionGroupList();
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(linkConditionGroupList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto linkConditionList = static_cast<cocos2d::__Array *>(ref);
#else
			auto linkConditionList = dynamic_cast<cocos2d::__Array *>(ref);
#endif
			cocos2d::Ref *ref2;
			int maxCount = linkConditionList->count();
			CCARRAY_FOREACH(linkConditionList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto linkConditionData = static_cast<agtk::data::ObjectActionLinkConditionData *>(ref2);
#else
				auto linkConditionData = dynamic_cast<agtk::data::ObjectActionLinkConditionData *>(ref2);
#endif
				if (linkConditionData->getIgnored()) {
					ignoredCount++;
				}
			}
			maxLinkCount += maxCount;
		}
		//全て無効の場合
		if (ignoredCount == maxLinkCount) {
			return -1;
		}
	}

	bool result = false;
	auto linkConditionGroupList = objectActionLinkData->getLinkConditionGroupList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(linkConditionGroupList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto linkConditionList = static_cast<cocos2d::__Array *>(ref);
#else
		auto linkConditionList = dynamic_cast<cocos2d::__Array *>(ref);
#endif
		cocos2d::Ref *ref2 = nullptr;
		bool ret = true;
		int ignoredCount = 0;
		int maxCount = linkConditionList->count();
		CCARRAY_FOREACH(linkConditionList, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto linkConditionData = static_cast<agtk::data::ObjectActionLinkConditionData *>(ref2);
#else
			auto linkConditionData = dynamic_cast<agtk::data::ObjectActionLinkConditionData *>(ref2);
#endif
			if (linkConditionData->getIgnored()) {
				//無効
				ignoredCount++;
				continue;
			}
			if (linkConditionData->getType() >= agtk::data::ObjectActionLinkConditionData::kConditionCustomHead) {
				//カスタム(1000～)
				ret = this->checkLinkConditionCustom(linkConditionData, objectActionLinkData);
			} else
			switch (linkConditionData->getType()) {
			case agtk::data::ObjectActionLinkConditionData::kConditionWallTouched: {//壁判定に接触(0)
				ret = this->checkLinkConditionWallTouched(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionNoWall: {//壁判定が無い(1)
				ret = this->checkLinkConditionNoWall(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionWallAhead: {//進んだ先で壁判定に接触(2)
				ret = this->checkLinkConditionWallAhead(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionNoWallAhead: {//進んだ先で判定が無い(3)
				ret = this->checkLinkConditionNoWallAhead(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionObjectWallTouched: {//他オブジェクトの壁判定に接触(4)
				ret = this->checkLinkConditionObjectWallTouched(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionAttackAreaTouched: {//攻撃判定に当たる(5)
				ret = this->checkLinkConditionAttackAreaTouched(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionAttackAreaNear: {//攻撃判定との距離(6)
				ret = this->checkLinkConditionAttackAreaNear(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionObjectNear: {//他オブジェクトとの距離(7)
				ret = this->checkLinkConditionObjectNear(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionObjectFacingEachOther: {//他のオブジェクトと向かい合っている(8)
				ret = this->checkLinkConditionObjectFacingEachOther(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionObjectFacing: {//他のオブジェクトの方向を向かっている(9)
				ret = this->checkLinkConditionObjectFacing(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionObjectFound: {//他のオブジェクトを発見した(10)
				ret = this->checkLinkConditionObjectFound(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionObjectFacingDirection: {//他のオブジェクトが指定方向を向いている(11)
				ret = this->checkLinkConditionObjectFacingDirection(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionHpZero: {//体力が0(12)
				ret = this->checkLinkConditionHpZero(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionCameraOutOfRange: {//カメラの範囲外にでた(13)
				ret = this->checkLinkConditionCameraOutOfRange(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionLocked: {//ロックした/された(14)
				ret = this->checkLinkConditionLocked(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionProbability: {//確率を使用(15)
				ret = this->checkLinkConditionProbability(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionWaitTime: {//一定時間が経過(16)
				ret = this->checkLinkConditionWaitTime(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionSwitchVariableChanged: {//スイッチ・変数が変化(17)
				ret = this->checkLinkConditionSwitchVariableChanged(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionAnimationFinished: {//モーションの表示が全て終わった(18)
				ret = this->checkLinkConditionAnimationFinished(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionJumpTop: {//ジャンプが頂点になった(19)
				ret = this->checkLinkConditionJumpTop(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionObjectActionChanged: {//オブジェクトのアクションが変化(20)
				ret = this->checkLinkConditionObjectActionChanged(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionSlopeTouched://坂に接触(21)
				ret = this->checkLinkConditionSlopeTouched(linkConditionData);
				break;
			case agtk::data::ObjectActionLinkConditionData::kConditionBuriedInWall://壁判定に埋まった(22)
				ret = this->checkLinkConditionBuriedInWallData(linkConditionData);
				break;
			case agtk::data::ObjectActionLinkConditionData::kConditionNoObjectWallTouched: {//他オブジェクトの壁判定に接触していない(23)
				ret = this->checkLinkConditionNoObjectWallTouched(linkConditionData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionScript: {//他オブジェクトの壁判定に接触していない(24)
				ret = this->checkLinkConditionScript(linkConditionData, objectActionLinkData);
				break; }
			case agtk::data::ObjectActionLinkConditionData::kConditionObjectHit: {//他オブジェクトの当たり判定に接触(25)
				ret = this->checkLinkConditionObjectHit(linkConditionData);
				break; }
			}
			if (linkConditionData->getNotFlag()) {
				ret = !ret;
			}
			if (!ret) {
				break;
			}
		}
		if (ret && ignoredCount < maxCount) {
			result = true;
		}
	}
	return result;
}

bool ObjectAction::checkLinkConditionWallTouched(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionWallTouchedData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionWallTouchedData *>(linkConditionData);
#endif
	CC_ASSERT(condition);

	int const bit = condition->getWallBit();
	auto const wallBit = agtk::data::ObjectActionLinkConditionData::kWallBitUp | agtk::data::ObjectActionLinkConditionData::kWallBitLeft | agtk::data::ObjectActionLinkConditionData::kWallBitRight | agtk::data::ObjectActionLinkConditionData::kWallBitDown;

	auto const & hitTileWalls = _object->getLinkConditionTileWall();
	for (auto const & htw : hitTileWalls) {
		//「対象のタイルグループを指定」チェック有り。
		if (condition->getUseTileGroup()) {
			auto group = htw.tile->getBeforeChangeGroup();
			if (group < 0) {
				group = htw.tile->getGroup();
			}
			if (group == condition->getTileGroup()) {
				if ((bit & htw.bit) & wallBit) {
					return true;
				}
			}
		}
		//「対象のタイルグループを指定」チェック無し。
		else {
			if (_object->getObjectData()->getCollideWithTileGroupBit() & htw.tile->getGroupBit()) {
				if ((bit & htw.bit) & wallBit) {
					return true;
				}
			}
		}
	}
	return false;
}

bool ObjectAction::checkLinkConditionNoWall(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionNoWallData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionNoWallData *>(linkConditionData);
#endif
	CC_ASSERT(condition);
	int bit = condition->getWallBit();
	int slopeBit = _object->getSlopeBit();
	int tileWallBit = _object->getLinkConditionTileWallBit();
	if (bit & agtk::data::ObjectActionLinkConditionData::kWallBitUp) {//上
		CC_ASSERT(agtk::data::ObjectActionLinkConditionData::kWallBitUp == agtk::data::ObjectActionLinkConditionData::kSlopeBitUp);
		if ((bit & tileWallBit) == 0 && (slopeBit & tileWallBit) == 0) {
			return true;
		}
	}
	if (bit & agtk::data::ObjectActionLinkConditionData::kWallBitLeft) {//左
		if ((bit & tileWallBit) == 0) {
			return true;
		}
	}
	if (bit & agtk::data::ObjectActionLinkConditionData::kWallBitRight) {//右
		if ((bit & tileWallBit) == 0) {
			return true;
		}
	}
	if (bit & agtk::data::ObjectActionLinkConditionData::kWallBitDown) {//下
		CC_ASSERT(agtk::data::ObjectActionLinkConditionData::kWallBitDown == agtk::data::ObjectActionLinkConditionData::kSlopeBitDown);
		if ((bit & tileWallBit) == 0 && (slopeBit & tileWallBit) == 0) {
			return true;
		}
	}
	return false;
}

bool ObjectAction::checkLinkConditionWallAhead(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionWallAheadData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionWallAheadData *>(linkConditionData);
#endif
	CC_ASSERT(condition);
	auto const bit = condition->getWallBit();
	auto bPlayerType = _object->getObjectData()->getOperatable();
	auto dispDirection = agtk::GetDirectionFromMoveDirectionId(_object->getInputDirectionId());
	if (bPlayerType == false && dispDirection == cocos2d::Vec2::ZERO) {
		dispDirection = agtk::GetDirectionFromMoveDirectionId(_object->getDispDirection());
	}

	//todo AheadTileWallBitの判定を要変更。進行方向に1タイル進んだところで各方向に触れている判定を行い、記録しておき、ここで参照する。
	auto const & hitTileWalls = _object->getAheadTileWall();
	for (auto const & htw : hitTileWalls) {
		//「対象のタイルグループを指定」チェック有り。
		if (condition->getUseTileGroup()) {
			auto group = htw.tile->getBeforeChangeGroup();
			if (group < 0) {
				group = htw.tile->getGroup();
			}
			if (group == condition->getTileGroup()) {
				auto tileWallBit = bit & htw.bit;
				if (tileWallBit & agtk::data::ObjectActionLinkConditionData::kWallBitUp && ((bPlayerType && dispDirection.y > 0) || !bPlayerType)) {//上
					return true;
				}
				if (tileWallBit & agtk::data::ObjectActionLinkConditionData::kWallBitLeft && ((bPlayerType && dispDirection.x < 0) || !bPlayerType)) {//左
					return true;
				}
				if (tileWallBit & agtk::data::ObjectActionLinkConditionData::kWallBitRight && ((bPlayerType && dispDirection.x > 0) || !bPlayerType)) {//右
					return true;
				}
				if (tileWallBit & agtk::data::ObjectActionLinkConditionData::kWallBitDown && ((bPlayerType && dispDirection.y < 0) || !bPlayerType)) {//下
					return true;
				}
			}
		}
		//「対象のタイルグループを指定」チェック無し。
		else {
			if (_object->getObjectData()->getCollideWithTileGroupBit() & htw.tile->getGroupBit()) {
				auto tileWallBit = bit & htw.bit;
				if (tileWallBit & agtk::data::ObjectActionLinkConditionData::kWallBitUp && ((bPlayerType && dispDirection.y > 0) || !bPlayerType)) {//上
					return true;
				}
				if (tileWallBit & agtk::data::ObjectActionLinkConditionData::kWallBitLeft && ((bPlayerType && dispDirection.x < 0) || !bPlayerType)) {//左
					return true;
				}
				if (tileWallBit & agtk::data::ObjectActionLinkConditionData::kWallBitRight && ((bPlayerType && dispDirection.x > 0) || !bPlayerType)) {//右
					return true;
				}
				if (tileWallBit & agtk::data::ObjectActionLinkConditionData::kWallBitDown && ((bPlayerType && dispDirection.y < 0) || !bPlayerType)) {//下
					return true;
				}
			}
		}
	}
	return false;
}

bool ObjectAction::checkLinkConditionNoWallAhead(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionNoWallAheadData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionNoWallAheadData *>(linkConditionData);
#endif
	CC_ASSERT(condition);
	int bit = condition->getWallBit();
	int slopeBit = _object->getAheadSlopeBit();
	int tileWallBit = _object->getAheadTileWallBit();
	if (bit & agtk::data::ObjectActionLinkConditionData::kWallBitUp) {//上
		CC_ASSERT(agtk::data::ObjectActionLinkConditionData::kWallBitUp == agtk::data::ObjectActionLinkConditionData::kSlopeBitUp);
		if ((bit & tileWallBit) == 0 && (bit & slopeBit) == 0) {
			return true;
		}
	}
	if (bit & agtk::data::ObjectActionLinkConditionData::kWallBitLeft) {//左
		if ((bit & tileWallBit) == 0) {
			return true;
		}
	}
	if (bit & agtk::data::ObjectActionLinkConditionData::kWallBitRight) {//右
		if ((bit & tileWallBit) == 0) {
			return true;
		}
	}
	if (bit & agtk::data::ObjectActionLinkConditionData::kWallBitDown) {//下
		CC_ASSERT(agtk::data::ObjectActionLinkConditionData::kWallBitDown == agtk::data::ObjectActionLinkConditionData::kSlopeBitDown);
		if ((bit & tileWallBit) == 0 && (bit & slopeBit) == 0) {
			return true;
		}
	}
	return false;
}

bool ObjectAction::checkLinkConditionObjectWallTouched(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionObjectWallTouchedData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionObjectWallTouchedData *>(linkConditionData);
#endif
	int bit = condition->getWallBit();
	cocos2d::Ref *ref = nullptr;
	auto leftWallObjectList = _object->getLeftWallObjectList();
	auto rightWallObjectList = _object->getRightWallObjectList();
	auto upWallObjectList = _object->getUpWallObjectList();
	auto downWallObjectList = _object->getDownWallObjectList();
	std::function<bool(Object *)> isTargetObjectMatched = [&isTargetObjectMatched, condition, this](Object *obj)
	{
		if (condition->getObjectType() == agtk::data::ObjectActionLinkConditionData::kObjectByGroup) {//オブジェクトの種類を指定
			if (condition->getObjectGroup() == agtk::data::ObjectActionLinkConditionData::kObjectTypeAll ) {//すべてのオブジェクト
				return true;
			}
			else if (condition->getObjectGroup() == obj->getObjectData()->getGroup()) {
				return true;
			}
		}
		else if (condition->getObjectType() == agtk::data::ObjectActionLinkConditionData::kObjectById) {//オブジェクトで指定
			//オブジェクト指定（-2:自身のオブジェクト,-3:自身以外のオブジェクト,それ以外はオブジェクトID）
			if (condition->getObjectId() == -2) {//自身のオブジェクト
				CC_ASSERT(0);
				if (_object == obj) {
					return true;
				}
			}
			else if (condition->getObjectId() == -3) {//自身以外のオブジェクト
				if (_object != obj) {
					return true;
				}
			}
			else {//オブジェクトID
				if (condition->getObjectId() == obj->getObjectData()->getId()) {
					return true;
				}
			}
		}
		return false;
	};
	if ((bit & agtk::data::ObjectActionLinkConditionData::kWallBitUp) && upWallObjectList->count() > 0) {//上
		bool found = false;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		auto size = upWallObjectList->size();
		for(int i = 0; i < (int)size; i++){
			auto obj = (*upWallObjectList)[i];
#else
		CCARRAY_FOREACH(upWallObjectList, ref) {
			auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
			if (isTargetObjectMatched(obj)) {
				found = true;
				break;
			}
		}
		if (found) return true;
	}
	if ((bit & agtk::data::ObjectActionLinkConditionData::kWallBitLeft) && leftWallObjectList->count() > 0) {//左
		bool found = false;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		auto size = leftWallObjectList->size();
		for (int i = 0; i < (int)size; i++) {
			auto obj = (*leftWallObjectList)[i];
#else
		CCARRAY_FOREACH(leftWallObjectList, ref) {
			auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
			if (isTargetObjectMatched(obj)) {
				found = true;
				break;
			}
		}
		if (found) return true;
	}
	if ((bit & agtk::data::ObjectActionLinkConditionData::kWallBitRight) && rightWallObjectList->count() > 0) {//右
		bool found = false;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		auto size = rightWallObjectList->size();
		for (int i = 0; i < (int)size; i++) {
			auto obj = (*rightWallObjectList)[i];
#else
		CCARRAY_FOREACH(rightWallObjectList, ref) {
			auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
			if (isTargetObjectMatched(obj)) {
				found = true;
				break;
			}
		}
		if (found) return true;
	}
	if (bit & agtk::data::ObjectActionLinkConditionData::kWallBitDown) {
		if (downWallObjectList->count() > 0) {//下
			bool found = false;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			auto size = downWallObjectList->size();
			for (int i = 0; i < (int)size; i++) {
				auto obj = (*downWallObjectList)[i];
#else
			CCARRAY_FOREACH(downWallObjectList, ref) {
				auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
				if (isTargetObjectMatched(obj)) {
					found = true;
					break;
				}
			}
			if (found) return true;
		}
#if 0	//ACT2-4772 このタイミングでは、_object->_floorObjectは1フレーム前の情報が入っているため、downWallObjectListだけを考慮すれば良い。
		else {
			//downWallObjectListで情報が無い場合、オブジェクトの上に乗っている場合。
			if (_object->_floorObject) {
				return true;
			}
		}
#endif
	}
	return false;
}

bool ObjectAction::checkLinkConditionAttackAreaTouched(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionAttackAreaTouchedData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionAttackAreaTouchedData *>(linkConditionData);
#endif
	CC_ASSERT(condition);
	auto objectData = _object->getObjectData();
	if (objectData->isGroupPlayer()) {
		//無敵モード
		//プレイヤーがダメージを受けない状態で進める事ができる。
		auto debugManager = DebugManager::getInstance();
		if (debugManager->getInvincibleModeEnabled()) {
			return false;
		}
	}
	int bit = condition->getWallBit();
	cocos2d::Ref *ref;
	auto collisionHitAttackList = _object->getCollisionHitAttackList();
	CCARRAY_FOREACH(collisionHitAttackList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto temp = static_cast<ObjectCollision::ObjectWallIntersectTemp *>(ref);
#else
		auto temp = dynamic_cast<ObjectCollision::ObjectWallIntersectTemp *>(ref);
#endif
		auto obj = temp->getObject();
		auto objData = obj->getObjectData();
		auto playObjData = obj->getPlayObjectData();

		bool bCheck = false;
		
		if ( (1 << objectData->getGroup()) && playObjData->getHitObjectGroupBit()) {
			bCheck = true;
		}
		
		if (bCheck == false) continue;

		if (condition->getObjectType() == agtk::data::ObjectActionLinkConditionData::kObjectByGroup) {//オブジェクトの種類を指定
			if (condition->getObjectGroup() != agtk::data::ObjectActionLinkConditionData::kObjectTypeAll 
		     && condition->getObjectGroup() != obj->getObjectData()->getGroup()) {
				continue;
			}
		}
		else if (condition->getObjectType() == agtk::data::ObjectActionLinkConditionData::kObjectById) {//オブジェクトで指定
			//オブジェクト指定（-2:自身のオブジェクト,-3:自身以外のオブジェクト,それ以外はオブジェクトID）
			switch (condition->getObjectId()){
			case -1: continue;//設定なし（空白）
			case -2://自身のオブジェクト
				if (_object != obj) {
					continue;
				}
				break;
			case -3://自身以外のオブジェクト
				if (_object == obj) {
					continue;
				}
				break;
			default: {//オブジェクトID
				if (condition->getObjectId() != obj->getObjectData()->getId()) {
					continue;
				}
				break; }
			}
		}
		else {
			continue;
		}

		//攻撃の属性を指定。
		//プリセットの属性
		if (condition->getAttributeType() == agtk::data::ObjectActionLinkConditionAttackAreaTouchedData::kAttributePreset) {
			if (condition->getAttributePresetId() != obj->getPlayObjectData()->getAttackAttribute()) {
				continue;
			}
		}
		//数値で設定
		else if (condition->getAttributeType() == agtk::data::ObjectActionLinkConditionAttackAreaTouchedData::kAttributeValue) {
			bool bCheck = false;
			if (condition->getAttributeEqual() && condition->getAttributeValue() == obj->getAttackAttribute()) {// "="
				bCheck = true;
			}
			else if(!condition->getAttributeEqual() && condition->getAttributeValue() != obj->getAttackAttribute()) {// "!="
				bCheck = true;
			}
			if (!bCheck) continue;
		}

		auto wallList = temp->getWallList();
		auto wallBit = agtk::ObjectWallIntersect::getWallBit(wallList);
		if ((bit & wallBit) & agtk::data::ObjectActionLinkConditionData::kWallBitUp) {//上
			return true;
		}
		if ((bit & wallBit) & agtk::data::ObjectActionLinkConditionData::kWallBitLeft) {//左
			return true;
		}
		if ((bit & wallBit) & agtk::data::ObjectActionLinkConditionData::kWallBitRight) {//右
			return true;
		}
		if ((bit & wallBit) & agtk::data::ObjectActionLinkConditionData::kWallBitDown) {//下
			return true;
		}
	}
	return false;
}

bool ObjectAction::checkLinkConditionAttackAreaNear(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionAttackAreaNearData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionAttackAreaNearData *>(linkConditionData);
#endif
	CC_ASSERT(condition);
	std::function<bool(agtk::Object *, agtk::Object *)> checkAttackAreaNear = [&](agtk::Object *object, agtk::Object *target)
	{
		bool bResultDirect = false;
		auto posObject = agtk::Scene::getPositionCocos2dFromScene(object->getCenterPosition());
		auto posTarget = agtk::Scene::getPositionCocos2dFromScene(target->getCenterPosition());
		auto n = (posTarget - posObject).getNormalized();

		//オブジェクトの方向（このオブジェクトの表示方向:true, ８方向から指定:false）
		int myDirectNum = object->getDispDirection();
		auto degree = agtk::GetDegreeFromVector(n);
		auto directNum = agtk::GetMoveDirectionId(degree);
		//攻撃判定の方向
		//このオブジェクトの方向（このオブジェクトの表示方向:true, ８方向から指定:false）
		if (condition->getObjectDirection()) {//このオブジェクトの表示方向
			int myDirectNum = object->getDispDirection();
			//このオブジェクトの表示方向
			if (condition->getOtherDirections()) {//指定の方向以外
				if (directNum != myDirectNum) {
					bResultDirect = true;
				}
			}
			else {
				if (directNum == myDirectNum) {
					bResultDirect = true;
				}
			}
		}
		else {
			//８方向から指定
			if (condition->getDirectionBit() == agtk::data::ObjectActionLinkConditionData::kAllDirectionBit) {//全方向
				bResultDirect = condition->getOtherDirections() ? false : true;
			}
			else {
				if (condition->getOtherDirections()) {
					if (!((1 << directNum) & condition->getDirectionBit())) {//８方向（1～9,5を除く）
						bResultDirect = true;
					}
				}
				else {
					if ((1 << directNum) & condition->getDirectionBit()) {//８方向（1～9,5を除く）
						bResultDirect = true;
					}
				}
			}
		}

		std::vector<Vertex4> hitAreaList;
		object->getTimelineList(agtk::data::TimelineInfoData::kTimelineHit, hitAreaList);
		if (hitAreaList.size() == 0) {
			return false;
		}

		//距離
		std::vector<Vertex4> attackAreaList;
		target->getTimelineList(agtk::data::TimelineInfoData::kTimelineAttack, attackAreaList);
		if (attackAreaList.size() == 0) {
			return false;
		}

		std::function<float(cocos2d::Vec2, cocos2d::Vec2, cocos2d::Vec2, cocos2d::Vec2 &)> getDistancePointLine = [&](cocos2d::Vec2 p, cocos2d::Vec2 a, cocos2d::Vec2 b, cocos2d::Vec2 &pp) {
			cocos2d::Vec2 ab = b - a;
			cocos2d::Vec2 ap = p - a;
			auto n = ab.getNormalized();
			float d = n.dot(ap);
			pp = a + (n * d);
			if (cocos2d::Vec2::isSegmentIntersect(p, pp, a, b)) {
				return pp.getDistance(p);
			}
			float l1 = p.getDistance(a);
			float l2 = p.getDistance(b);
			return (l1 >= l2) ? l2 : l1;
		};

		//オブジェクトの方向と距離をチェックする
		std::function<float(agtk::Vertex4, agtk::Vertex4, bool&)> getNearestDistance = [&](agtk::Vertex4 v1, agtk::Vertex4 v2, bool &bCollision) {
			bCollision = false;
			if (agtk::Vertex4::intersectsVertex4(v1, v2)) {
				bCollision = true;
				return 0.0f;
			}
			float distance = -1.0f;
			//4点同士で一番近い距離
			for (int i = 0; i < v1.length(); i++) {
				for (int j = 0; j < v2.length(); j++) {
					float d = v1.addr()[i].getDistance(v2.addr()[j]);
					if (d < distance || distance < 0) {
						distance = d;
					}
				}
			}
			cocos2d::Vec2 n;
			bool bParallel = cocos2d::Vec2::isLineParallel(v1[0], v1[1], v2[0], v2[1]);
			if (bParallel) {//平行
				for (int i = 0; i < v1.length(); i++) {
					for (int j = 0; j < v2.length(); j++) {
						//平行。
						if (cocos2d::Vec2::isLineParallel(v1[i], v1[(i + 1) % v1.length()], v2[j], v2[(j + 1) % v2.length()])) {
							cocos2d::Vec2 v11 = v1[i];
							cocos2d::Vec2 v12 = v1[(i + 1) % v1.length()];
							cocos2d::Vec2 v21 = v2[j];
							cocos2d::Vec2 v22 = v2[(i + 1) % v2.length()];
							float d = getDistancePointLine(v21, v11, v12, n);
							if (d < distance || distance < 0) {
								float d1 = (n - v11).cross(v21 - v11);
								float d2 = (n - v12).cross(v21 - v12);
								if (d1 > 0 && d2 < 0 || d1 < 0 && d2 > 0) {
									distance = d;
								}
							}
							d = getDistancePointLine(v22, v11, v12, n);
							if (d < distance || distance < 0) {
								float d1 = (n - v11).cross(v22 - v11);
								float d2 = (n - v12).cross(v22 - v12);
								if (d1 > 0 && d2 < 0 || d1 < 0 && d2 > 0) {
									distance = d;
								}
							}
							d = getDistancePointLine(v11, v21, v22, n);
							if (d < distance || distance < 0) {
								float d1 = (n - v21).cross(v11 - v21);
								float d2 = (n - v22).cross(v11 - v22);
								if (d1 > 0 && d2 < 0 || d1 < 0 && d2 > 0) {
									distance = d;
								}
							}
							d = getDistancePointLine(v12, v21, v22, n);
							if (d < distance || distance < 0) {
								float d1 = (n - v21).cross(v12 - v21);
								float d2 = (n - v22).cross(v12 - v22);
								if (d1 > 0 && d2 < 0 || d1 < 0 && d2 > 0) {
									distance = d;
								}
							}
						}
					}
				}
			} else {
				for (int i = 0; i < v1.length(); i++) {
					for (int j = 0; j < v2.length(); j++) {
						float d = getDistancePointLine(v2[j], v1[i], v1[(i + 1) % v1.length()], n);
						if (d < distance || distance < 0) {
							distance = d;
						}
						d = getDistancePointLine(v1[i], v2[j], v2[(j + 1) % v2.length()], n);
						if (d < distance || distance < 0) {
							distance = d;
						}
					}
				}
			}
			return distance;
		};

		float distance = -1.0f;
		bool bCheckCollision = false;
		for (auto hit : hitAreaList) {
			for (auto attack : attackAreaList) {
				float d = getNearestDistance(hit, attack, bCheckCollision);
				if (d < distance || distance < 0) {
					distance = d;
				}
				if (bCheckCollision) goto lSkip;
			}
		}
		lSkip:

		//攻撃判定領域の距離（距離を指定しない:0, 指定距離以上:1, 指定距離以下:2）
		bool bResultDistance = false;
		switch (condition->getDistanceType()) {
		case 0://距離を指定しない
			bResultDistance = true;
			break;
		case 1://指定距離以上
			if (distance >= condition->getDistance()) {
				bResultDistance = true;
			}
			break;
		case 2://指定距離以下
			if (distance <= condition->getDistance()) {
				bResultDistance = true;
			}
			break;
		default:CC_ASSERT(0);
		}

		//攻撃の属性を指定
		bool bResultAttackAttribute = false;
		switch (condition->getAttributeType()) {
		case 0://属性を指定しない
			bResultAttackAttribute = true;
			break;
		case 1: {//プリセットの属性
			if (target->getAttackAttribute() == condition->getAttributePresetId()) {
				bResultAttackAttribute = true;
			}
			break; }
		case 2: {//数値で指定
			condition->getAttributeValue();
			if (condition->getAttributeEqual()) {// "="
				if (target->getAttackAttribute() == condition->getAttributeValue()) {
					bResultAttackAttribute = true;
				}
			}
			else {// "!="
				if (target->getAttackAttribute() != condition->getAttributeValue()) {
					bResultAttackAttribute = true;
				}
			}
			break; }
		}
		return bResultDirect && bResultDistance && bResultAttackAttribute;
	};
	auto scene = this->getScene();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	auto objectList = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
	auto objectList = scene->getObjectAll(this->getSceneLayer()->getType());
#endif

	switch (condition->getObjectType()) {//オブジェクトの種類を指定
	case agtk::data::ObjectActionLinkConditionData::kObjectByGroup: {//オブジェクトの種類で指定
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			if (object == _object) continue;
			if (condition->getObjectGroup() == agtk::data::ObjectActionLinkConditionData::kObjectTypeAll ||
				condition->getObjectGroup() == object->getObjectData()->getGroup()) {
				if (checkAttackAreaNear(_object, object)) {
					return true;
				}
			}
		}
		break; }
	case agtk::data::ObjectActionLinkConditionData::kObjectById: {//オブジェクトで指定
		switch (condition->getObjectId()) {
		case -1: break;//設定無し。
		case -2: {//自身のオブジェクト
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
			auto object = scene->getObjectInstance(-1, _object->getInstanceId(), this->getSceneLayer()->getType());
			if (object) {
				if (checkAttackAreaNear(_object, object)) {
					return true;
				}
			}
#else
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				if (object == _object) {
					if (checkAttackAreaNear(_object, object)) {
						return true;
					}
				}
			}
#endif
			break; }
		case -3: {//自身以外のオブジェクト
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				if (object != _object) {
					if (checkAttackAreaNear(_object, object)) {
						return true;
					}
				}
			}
			break; }
		default: {
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
			auto objectList = scene->getObjectAll(condition->getObjectId(), this->getSceneLayer()->getType());
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
				auto object = static_cast<agtk::Object *>(ref);
				if (checkAttackAreaNear(_object, object)) {
					return true;
				}
			}
#else
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				if (object->getObjectData()->getId() == condition->getObjectId()) {
					if (checkAttackAreaNear(_object, object)) {
						return true;
					}
				}
			}
#endif
			break;  }
		}
		break; }
	default: CC_ASSERT(0);
	}
	return false;
}

bool ObjectAction::checkLinkConditionObjectNear(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionObjectNearData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionObjectNearData *>(linkConditionData);
#endif
	CC_ASSERT(condition);

	//オブジェクトの方向と距離をチェックする
	std::function<bool(agtk::Object *, agtk::Object *)> FuncCheckObjectNear = [&](agtk::Object *object, agtk::Object *target)
	{
		bool bResultDirect = false;
		auto posObject = object->getCenterPosition();
		auto posTarget = target->getCenterPosition();
		// オブジェクトを配置しているレイヤーのスクロール速度が100％以外の場合は、レイヤー座標を加えて距離の補正を行う
		float moveSpeedX = object->getSceneData()->getLayerMoveSpeedX(object->getLayerId());
		float moveSpeedY = object->getSceneData()->getLayerMoveSpeedY(object->getLayerId());
		if (moveSpeedX != 100.0f || moveSpeedY != 100.0f) {
			posObject += object->getSceneLayer()->getPosition();
		}
		moveSpeedX = object->getSceneData()->getLayerMoveSpeedX(target->getLayerId());
		moveSpeedY = object->getSceneData()->getLayerMoveSpeedY(target->getLayerId());
		if (moveSpeedX != 100.0f || moveSpeedY != 100.0f) {
			posTarget += target->getSceneLayer()->getPosition();
		}
		posObject = agtk::Scene::getPositionCocos2dFromScene(posObject);
		posTarget = agtk::Scene::getPositionCocos2dFromScene(posTarget);
		auto n = (posTarget - posObject).getNormalized();

		//オブジェクトの方向（このオブジェクトの表示方向:true, ８方向から指定:false）
		int myDirectNum = object->getDispDirection();
		auto degree = agtk::GetDegreeFromVector(n);
		auto directNum = agtk::GetMoveDirectionId(degree);
		if (condition->getObjectDirection()) {
			//このオブジェクトの表示方向
			if (condition->getOtherDirections()) {//指定の方向以外
				if (directNum != myDirectNum) {
					bResultDirect = true;
				}
			}
			else {
				if (directNum == myDirectNum) {
					bResultDirect = true;
				}
			}
		}
		else {
			//８方向から指定
			if (condition->getDirectionBit() == agtk::data::ObjectActionLinkConditionData::kAllDirectionBit) {//全方向
				bResultDirect = condition->getOtherDirections() ? false : true;
			}
			else {
				if (condition->getOtherDirections()) {
					if (!((1 << directNum) & condition->getDirectionBit())) {//８方向（1～9,5を除く）
						bResultDirect = true;
					}
				}
				else {
					if ((1 << directNum) & condition->getDirectionBit()) {//８方向（1～9,5を除く）
						bResultDirect = true;
					}
				}
			}
		}
		if (bResultDirect == false) {
			return false;
		}

		//オブジェクトの距離（距離を指定しない:0, 指定距離以上:1, 指定距離以下:2）
		switch (condition->getDistanceType()) {
		case 0://距離を指定しない
			return true;
		case 1://指定距離以上
			if (posObject.getDistance(posTarget) >= condition->getDistance()) {
				return true;
			}
			break;
		case 2://指定距離以下
			if (posObject.getDistance(posTarget) <= condition->getDistance()) {
				return true;
			}
			break;
		default:CC_ASSERT(0);
		}
		return false;
	};

	auto scene = this->getScene();
#ifdef USE_SAR_OPTIMIZTEST
	auto objectList = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
	auto objectList = scene->getObjectAll(this->getSceneLayer()->getType());
#endif
	switch (condition->getObjectType()) {
	case agtk::data::ObjectActionLinkConditionData::kObjectByGroup: {//オブジェクトの種類で指定。
		bool bCheck = false;
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			if (_object == object) continue;
			auto objectData = object->getObjectData();
			if (condition->getObjectGroup() == agtk::data::ObjectActionLinkConditionData::kObjectTypeAll || condition->getObjectGroup() == objectData->getGroup()) {
				if (FuncCheckObjectNear(_object, object)) {
					bCheck = true;
				}
			}
		}
		if (bCheck == false) return false;
		break;}
	case agtk::data::ObjectActionLinkConditionData::kObjectById://オブジェクトで指定。
		switch (condition->getObjectId()) {
		case -1://設定無し。
			return false;
		case -3: {//自身以外のオブジェクト
			bool bCheck = false;
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				if (object != _object) {
					if (FuncCheckObjectNear(_object, object)) {
						bCheck = true;
					}
				}
			}
			if (bCheck == false) return false;
			break; }
		default: {
			//オブジェクト指定
			bool bCheck = false;
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
			auto objectList = scene->getObjectAll(condition->getObjectId(), this->getSceneLayer()->getType());
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
				auto object = static_cast<agtk::Object *>(ref);
				if (FuncCheckObjectNear(_object, object)) {
					bCheck = true;
				}
			}
#else
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				auto objectData = object->getObjectData();
				if (objectData->getId() == condition->getObjectId()) {
					if (FuncCheckObjectNear(_object, object)) {
						bCheck = true;
					}
				}
			}
#endif
			if (bCheck == false) return false;
			break; }
		}
		break;
	default: {
		CC_ASSERT(0);
		return false; }
	}
	return true;
}

bool ObjectAction::checkLinkConditionObjectFacingEachOther(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
	std::function<bool(agtk::Object *, agtk::Object *)> FuncCheckFacingEachOther = [&](agtk::Object *object1, agtk::Object *object2) {
		auto pos1 = agtk::Scene::getPositionCocos2dFromScene(object1->getPosition());
		auto pos2 = agtk::Scene::getPositionCocos2dFromScene(object2->getPosition());
		//位置よりそれぞれの方向番号を得る。
		float angle1 = agtk::GetDegreeFromVector((pos2 - pos1).getNormalized());
		float angle2 = agtk::GetDegreeFromVector((pos1 - pos2).getNormalized());
		int directNum11 = agtk::GetMoveDirectionId(angle1);
		int directNum21 = agtk::GetMoveDirectionId(angle2);
		int directNum12 = object1->getDispDirection();
		int directNum22 = object2->getDispDirection();
		if (directNum12 == 0) {
			directNum12 = object1->getDispDirectionByCurrentDirectionData();
		}
		if (directNum22 == 0) {
			directNum22 = object2->getDispDirectionByCurrentDirectionData();
		}
		//表示方向が、位置より求めた方向番号と同じで、さらに向き合っている場合はTRUEを返す。
		if (directNum11 == directNum12 && directNum21 == directNum22) {
			if (directNum12 + directNum22 == 10) {
				return true;
			}
		}
		return false;
	};

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionObjectFacingEachOtherData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionObjectFacingEachOtherData *>(linkConditionData);
#endif
	CC_ASSERT(condition);
	auto scene = this->getScene();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	auto objectAllList = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
	auto objectAllList = scene->getObjectAll(this->getSceneLayer()->getType());
#endif
	switch (condition->getObjectType()) {
	case agtk::data::ObjectActionLinkConditionData::kObjectByGroup: {//オブジェクトの種類で指定。
		if (objectAllList->containsObject(_object) && objectAllList->count() == 1) {
			return false;
		}
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectAllList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			if (_object == object) continue;
			auto objectData = object->getObjectData();
			if (condition->getObjectGroup() == agtk::data::ObjectActionLinkConditionAnimationFinishedData::kObjectTypeAll || condition->getObjectGroup() == objectData->getGroup()) {
				if (FuncCheckFacingEachOther(object, _object)) {
					return true;
				}
			}
		}
		break; }
	case agtk::data::ObjectActionLinkConditionData::kObjectById://オブジェクトで指定。
		switch (condition->getObjectId()) {
		case -1://設定無し。
			return false;
		case -3: {//自身以外のオブジェクト
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectAllList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				if (object != _object) {
					if (FuncCheckFacingEachOther(object, _object)) {
						return true;
					}
				}
			}
			break; }
		default: {
			//オブジェクト指定
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
			auto objectAllList = scene->getObjectAll(condition->getObjectId(), this->getSceneLayer()->getType());
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectAllList, ref) {
				auto object = static_cast<agtk::Object *>(ref);
				if (FuncCheckFacingEachOther(object, _object)) {
					return true;
				}
			}
#else
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectAllList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				auto objectData = object->getObjectData();
				if (objectData->getId() == condition->getObjectId()) {
					if (FuncCheckFacingEachOther(object, _object)) {
						return true;
					}
				}
			}
#endif
			break; }
		}
		break;
	default: {
		CC_ASSERT(0);
		return false; }
	}
	return false;
}

bool ObjectAction::checkLinkConditionObjectFound(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionObjectFoundData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionObjectFoundData *>(linkConditionData);
#endif
	CC_ASSERT(condition);
	if (condition->getViewportId() < 0) {
		//設定無し。
		return false;
	}
	auto scene = this->getScene();
	auto sceneLayer = this->getSceneLayer();
	auto viewportLightSceneLayer = scene->getViewportLight()->getViewportLightSceneLayer(_object->getLayerId());
	auto viewportLightObject = viewportLightSceneLayer->getViewportLightObject(_object);
	if (viewportLightObject == nullptr) {
		return false;
	}
	auto viewportLightSprite = viewportLightObject->getViewportLightSprite(condition->getViewportId());
	CC_ASSERT(viewportLightSprite);
	auto settingData = viewportLightSprite->getObjectViewportLightSettingData();
	if (!settingData->getViewport()) {
		//照明の場合。
		return false;
	}
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	auto objectList = scene->getObjectAllReference(sceneLayer->getType());
#else
	auto objectList = scene->getObjectAll(sceneLayer->getType());
#endif
	switch (condition->getObjectType()) {
	case agtk::data::ObjectActionLinkConditionData::kObjectByGroup:{//オブジェクトの種類で指定。
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			//if (_object == object) continue;//自身はチェックしない。
			if (condition->getObjectGroup() == agtk::data::ObjectActionLinkConditionData::kObjectTypeAll || condition->getObjectGroup() == object->getObjectData()->getGroup()) {
				if (!condition->getDiscoveredAcrossLayersObject()) {
					if (_object->getLayerId() != object->getLayerId()) continue;
				}

				if (viewportLightSprite->intersects(object))
					return true;
			}
		}
		break; }
	case agtk::data::ObjectActionLinkConditionData::kObjectById://オブジェクトで指定
		switch (condition->getObjectId()) {
		case -2: {//自身のオブジェクト
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
			auto object = scene->getObjectInstance(-1, _object->getInstanceId(), sceneLayer->getType());
			if (object)
			{
				if (viewportLightSprite->intersects(object)) {
					return true;
				}
			}
#else
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				if (object == _object) {
					if (!condition->getDiscoveredAcrossLayersObject()) {
						if (_object->getLayerId() != object->getLayerId()) continue;
					}

					if (viewportLightSprite->intersects(object)) {
						return true;
					}
				}
			}
#endif
			break; }
		case -3: {//自身以外のオブジェクト
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				if (object != _object) {
					if (!condition->getDiscoveredAcrossLayersObject()) {
						if (_object->getLayerId() != object->getLayerId()) continue;
					}

					if (viewportLightSprite->intersects(object)) {
						return true;
					}
				}
			}
			break; }
		default: {
			//設定無し。
			if (condition->getObjectId() < 0) {
				return false;
			}
			//オブジェクト指定
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				if (object->getObjectData()->getId() == condition->getObjectId()) {
					if (!condition->getDiscoveredAcrossLayersObject()) {
						if (_object->getLayerId() != object->getLayerId()) continue;
					}

					if (viewportLightSprite->intersects(object)) {
						return true;
					}
				}
			}
			break; }
		}
		break;
	}
	return false;
}

bool ObjectAction::checkLinkConditionObjectFacing(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
	std::function<bool(agtk::Object *, agtk::Object *)> FuncCheckFacing = [&](agtk::Object *object, agtk::Object *target) {
		auto pos1 = agtk::Scene::getPositionCocos2dFromScene(object->getCenterPosition());
		auto pos2 = agtk::Scene::getPositionCocos2dFromScene(target->getCenterPosition());
		//位置より方向番号を得る。
		float angle = agtk::GetDegreeFromVector((pos2 - pos1).getNormalized());
		int directNum = agtk::GetMoveDirectionId(angle);
		int directNum1 = object->getDispDirection();
		//表示方向が、位置より求めた方向番号が同じ場合TRUEを返す。
		if (directNum == directNum1) {
			return true;
		}
		return false;
	};

// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionObjectFacingData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionObjectFacingData *>(linkConditionData);
#endif
	CC_ASSERT(condition);
	auto scene = this->getScene();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	auto objectList = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
	auto objectList = scene->getObjectAll(this->getSceneLayer()->getType());
#endif
	switch (condition->getObjectType()) {
	case agtk::data::ObjectActionLinkConditionData::kObjectByGroup: {//オブジェクトの種類で指定。														   
		//自身オブジェクトのみの場合。
		if (objectList->containsObject(_object) && objectList->count() == 1) {
			return false;
		}
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			if (_object == object) continue;
			auto objectData = object->getObjectData();
			if (condition->getObjectGroup() == agtk::data::ObjectActionLinkConditionData::kObjectTypeAll || condition->getObjectGroup() == objectData->getGroup()) {
				if (FuncCheckFacing(_object, object)) {
					return true;
				}
			}
		}
		break; }
	case agtk::data::ObjectActionLinkConditionData::kObjectById://オブジェクトで指定
		switch (condition->getObjectId()) {
		case -1://設定無し。
			return false;
		case -3: {//自身以外のオブジェクト
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				if (object != _object) {
					if (FuncCheckFacing(_object, object)) {
						return true;
					}
				}
			}
			break; }
		default: {//プロジェクトに登録されているオブジェクト
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
			auto objectList = scene->getObjectAll(condition->getObjectId(), this->getSceneLayer()->getType());
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
				auto object = static_cast<agtk::Object *>(ref);
				if (FuncCheckFacing(_object, object)) {
					return true;
				}
			}
#else
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				auto objectData = object->getObjectData();
				if (objectData->getId() == condition->getObjectId()) {
					if (FuncCheckFacing(_object, object)) {
						return true;
					}
				}
			}
#endif
			break; }
		}
		break;
	default: {
		CC_ASSERT(0);
		return false; }
	}
	return false;
}

bool ObjectAction::checkLinkConditionObjectFacingDirection(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionObjectFacingDirectionData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionObjectFacingDirectionData *>(linkConditionData);
#endif
	CC_ASSERT(condition);

	std::function<bool(agtk::Object *, agtk::Object *)> FuncCheckFacingDirection = [&](agtk::Object *object, agtk::Object *target)
	{
		int directNum = target->getDispDirection();
		if (directNum == 0) {
			directNum = target->getDispDirectionByCurrentDirectionData();
		}
		if (condition->getObjectDirection()) {//このオブジェクトの表示方向
			int myDirectNum = object->getDispDirection();
			if (myDirectNum == 0) {
				myDirectNum = object->getDispDirectionByCurrentDirectionData();
			}
			if (condition->getOtherDirections()) {//指定の方向以外
				if (directNum != myDirectNum) {
					return true;
				}
			}
			else {
				if (directNum == myDirectNum) {
					return true;
				}
			}
		}
		else {//８方向から指定
			if (condition->getDirectionBit() == agtk::data::ObjectActionLinkConditionData::kAllDirectionBit) {//全方向
				return condition->getOtherDirections() ? false : true;
			}
			else {
				if (condition->getOtherDirections()) {
					if (!(condition->getDirectionBit() & (1 << directNum))) {
						return true;
					}
				} else {
					if (condition->getDirectionBit() & (1 << directNum)) {
						return true;
					}
				}
			}
		}
		return false;
	};

	auto scene = this->getScene();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	auto objectList = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
	auto objectList = scene->getObjectAll(this->getSceneLayer()->getType());
#endif
	switch (condition->getObjectType()) {
	case agtk::data::ObjectActionLinkConditionData::kObjectByGroup: {//オブジェクトの種類で指定。
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			auto objectData = object->getObjectData();
			if (_object == object) continue;
			if (condition->getObjectGroup() == agtk::data::ObjectActionLinkConditionData::kObjectTypeAll || condition->getObjectGroup() == objectData->getGroup()) {
				if (FuncCheckFacingDirection(_object, object)) {
					return true;
				}
			}
		}
		break;}
	case agtk::data::ObjectActionLinkConditionData::kObjectById://オブジェクトで指定。
		switch (condition->getObjectId()) {
		case -1://設定なし
			break;
		case -2://自身のオブジェクト
			if (FuncCheckFacingDirection(_object, _object)) {
				return true;
			}
			break;
		case -3: {//自身以外のオブジェクト
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				if (object != _object) {
					if (FuncCheckFacingDirection(_object, object)) {
						return true;
					}
				}
			}
			break; }
		default: {//プロジェクトに登録されているオブジェクト
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
			auto objectList = scene->getObjectAll(condition->getObjectId(), this->getSceneLayer()->getType());
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
				auto object = static_cast<agtk::Object *>(ref);
				if (FuncCheckFacingDirection(_object, object)) {
					return true;
				}
			}
#else
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				auto objectData = object->getObjectData();
				if (objectData->getId() == condition->getObjectId()) {
					if (FuncCheckFacingDirection(_object, object)) {
						return true;
					}
				}
			}
#endif
			break; }
		}
		break;
	default:CC_ASSERT(0);
	}
	return false;
}

bool ObjectAction::checkLinkConditionHpZero(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
	//体力０以下
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionHpZeroData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionHpZeroData *>(linkConditionData);
#endif
	CC_ASSERT(condition);
	auto scene = this->getScene();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	auto objectList = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
	auto objectList = scene->getObjectAll(this->getSceneLayer()->getType());
#endif
	switch (condition->getObjectId()) {
	case -1:
		CC_ASSERT(0);
		break;
	case -2: {//自身のオブジェクト
		if (_object->getPlayObjectData()->getHp() <= 0) {
			return true;
		}
		break; }
	case -3: {//自身以外のオブジェクト
		cocos2d::Ref *ref = nullptr;
		bool ret = true;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			if (object != _object) {
				if (object->getPlayObjectData()->getHp() > 0) {
					ret = false;
					break;
				}
			}
		}
		if (ret) {
			return true;
		}
		break; }
	default: {//プロジェクトに登録されているオブジェクト
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
		auto objectList = scene->getObjectAll(condition->getObjectId(), this->getSceneLayer()->getType());
		cocos2d::Ref *ref = nullptr;
		bool ret = true;
		CCARRAY_FOREACH(objectList, ref) {
			auto object = static_cast<agtk::Object *>(ref);
			if (object->getPlayObjectData()->getHp() > 0) {
				ret = false;
				break;
			}
		}
#else
		cocos2d::Ref *ref = nullptr;
		bool ret = true;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			auto objectData = object->getObjectData();
			if (objectData->getId() == condition->getObjectId()) {
				if (object->getPlayObjectData()->getHp() > 0) {
					ret = false;
					break;
				}
			}
		}
#endif
		if (ret) {
			return true;
		}
		break; }
	}
	return false;
}

bool ObjectAction::checkLinkConditionCameraOutOfRange(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionCameraOutOfRangeData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionCameraOutOfRangeData *>(linkConditionData);
#endif
	CC_ASSERT(condition);

	auto camera = this->getScene()->getCamera();
	cocos2d::Rect rect;
	if (condition->getObjectId() == -1) {//設定無し。
		return false;
	}
	else if (condition->getObjectId() == -2) {//自身
		if (condition->getDistanceFlag() == false) {//距離を指定しない。
			rect = _object->getRect();
			if (camera->isPositionScreenWithinCamera(rect) == false) {
				return true;
			}
		}
		else {
			//距離を指定する。
			rect = _object->getRect();
			if (camera->isPositionScreenWithinCamera(rect, cocos2d::Vec2(condition->getDistance(), condition->getDistance())) == false) {
				return true;
			}
		}
	}
	else if (condition->getObjectId() == -3) {//自身以外
		bool ret = false;
		auto scene = this->getScene();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		auto objectList = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
		auto objectList = scene->getObjectAll(this->getSceneLayer()->getType());
#endif
		if (objectList->count() == 0) {
			return false;
		}
		if (objectList->count() == 1 && objectList->containsObject(_object)) {
			//自身のみの場合。
			return false;
		}
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			if (_object == object) continue;
			if (condition->getDistanceFlag() == false) {//距離を指定しない。
				rect = object->getRect();
				if (camera->isPositionScreenWithinCamera(rect)) {
					ret = true;
				}
			}
			else {
				//距離を指定する。
				rect = object->getRect();
				if (camera->isPositionScreenWithinCamera(rect, cocos2d::Vec2(condition->getDistance(), condition->getDistance()))) {
					ret = true;
				}
			}
		}
		if (!ret) {
			return true;
		}
	}
	else {
		auto scene = this->getScene();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
		auto objectList = scene->getObjectAll(condition->getObjectId(), this->getSceneLayer()->getType());
#else
#ifdef USE_SAR_OPTIMIZE_1
		auto objectList = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
		auto objectList = scene->getObjectAll(this->getSceneLayer()->getType());
#endif
#endif
		if (objectList->count() == 0) {
			return false;
		}
		bool bCheck = false;
		bool ret = false;
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
			if (object == _object) continue;
			if (condition->getDistanceFlag() == false) {//距離を指定しない。
				rect = object->getRect();
				if (camera->isPositionScreenWithinCamera(rect)) {
					ret = true;
				}
				bCheck = true;
			}
			else {
				//距離を指定する。
				rect = object->getRect();
				if (camera->isPositionScreenWithinCamera(rect, cocos2d::Vec2(condition->getDistance(), condition->getDistance()))) {
					ret = true;
				}
				bCheck = true;
			}
#else
			if (object == _object) continue;
			if (condition->getDistanceFlag() == false) {//距離を指定しない。
				if (object->getObjectData()->getId() == condition->getObjectId()) {
					rect = object->getRect();
					if (camera->isPositionScreenWithinCamera(rect)) {
						ret = true;
					}
					bCheck = true;
				}
			}
			else {
				//距離を指定する。
				if (object->getObjectData()->getId() == condition->getObjectId()) {
					rect = object->getRect();
					if (camera->isPositionScreenWithinCamera(rect, cocos2d::Vec2(condition->getDistance(), condition->getDistance()))) {
						ret = true;
					}
					bCheck = true;
				}
			}
#endif
		}
		if (!ret && bCheck) {
			return true;
		}
	}
	return false;
}

/**
* ロックした/された
* @param	linkConditionData	条件データ
* @return	True:ロックした/された or False:ロックしていない/されていない
*/
bool ObjectAction::checkLinkConditionLocked(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionLockedData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionLockedData *>(linkConditionData);
#endif
	CC_ASSERT(condition);
	auto scene = this->getScene();

	std::function<bool(agtk::Object *)> FuncCheckLockedByObject = [&](agtk::Object *object)
	{
		switch (condition->getLockedObjectType()) {
		case agtk::data::ObjectActionLinkConditionData::kObjectByGroup://オブジェクトの種類で指定
			if (condition->getLockedObjectGroup() == agtk::data::ObjectActionLinkConditionData::kObjectTypeAll || condition->getLockedObjectGroup() == object->getObjectData()->getGroup()) {
				return true;
			}
			break;
		case agtk::data::ObjectActionLinkConditionData::kObjectById://オブジェクトで指定
			switch (condition->getLockedObjectId()) {
			case -1://設定なし
				break;
			case -2://自身のオブジェクト
				if (_object->getId() == object->getId()) {
					return true;
				}
				break;
			case -3://自身以外のオブジェクト
				if (_object->getId() != object->getId()) {
					return true;
				}
				break;
			default://指定オブジェクト
				if (object->getObjectData()->getId() == condition->getLockedObjectId()) {
					return true;
				}
				break;
			}
			break;
		}
		return false;
	};

	switch (condition->getLockingObjectId()) {//ロック動作をしたオブジェクト指定（なし:-1,自身のオブジェクト:-2,自身以外のオブジェクト:-3,オブジェクトID:n>0）
	case -1://設定なし
		break;
	case -2: {//自身のオブジェクト
			  //自身のオブジェクトがロックした場合は、ロックされたオブジェクトを探しだす。
		cocos2d::Ref *ref = nullptr;
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		auto objectAll = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
		auto objectAll = scene->getObjectAll(this->getSceneLayer()->getType());
#endif
		CCARRAY_FOREACH(objectAll, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			auto playObjectData = object->getPlayObjectData();

			if (playObjectData->isLocked(_object->getInstanceId())) {
				if (FuncCheckLockedByObject(object) == true) {
					return true;
				}
			}
		}
		break; }
	case -3: {//自身以外のオブジェクト
			  //自身以外のオブジェクトがロックした場合は、自身以外のオブジェクトがロックしているか探し出す。
		cocos2d::Ref *ref = nullptr;
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		auto objectAll = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
		auto objectAll = scene->getObjectAll(this->getSceneLayer()->getType());
#endif
		CCARRAY_FOREACH(objectAll, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			auto playObjectData = object->getPlayObjectData();

			// 誰からもロックされていない
			if (!playObjectData->getLock()) {
				continue;
			}

			if (!playObjectData->isLocked(_object->getInstanceId())) {
				if (FuncCheckLockedByObject(object) == true) {
					return true;
				}
			}
		}
		break; }
	default: {//オブジェクトID
			  //指定オブジェクトがロックしたかチェックする。
		cocos2d::Ref *ref = nullptr;
		auto sceneType = this->getSceneLayer()->getType();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		auto objectAll = scene->getObjectAllReference(sceneType);
#else
		auto objectAll = scene->getObjectAll(sceneType);
#endif

		auto lockingObjctAll = scene->getObjectAll(condition->getLockingObjectId(), sceneType);

		CCARRAY_FOREACH(objectAll, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto object = static_cast<agtk::Object *>(ref);
#else
			auto object = dynamic_cast<agtk::Object *>(ref);
#endif
			auto playObjectData = object->getPlayObjectData();

			// 誰からもロックされていない
			if (!playObjectData->getLock()) {
				continue;
			}

			cocos2d::Ref *ref2 = nullptr;
			CCARRAY_FOREACH(lockingObjctAll, ref2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto target = static_cast<agtk::Object *>(ref2);
#else
				auto target = dynamic_cast<agtk::Object *>(ref2);
#endif
				if (playObjectData->isLocked(target->getInstanceId())) {
					if (FuncCheckLockedByObject(object) == true) {
						return true;
					}
				}
			}
		}
		break; }
	}
	return false;
}

bool ObjectAction::checkLinkConditionProbability(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionProbabilityData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionProbabilityData *>(linkConditionData);
#endif
	CC_ASSERT(condition);
	int r = rand() % PROBABILITY_MAX;
	int p = static_cast<int>(condition->getProbability() * 100.0);
	if (r < p) {
		return true;
	}
	return false;
}

bool ObjectAction::checkLinkConditionWaitTime(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionWaitTimeData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionWaitTimeData *>(linkConditionData);
#endif
	CC_ASSERT(condition);
	if (_duration >= condition->getTime()) {
		return true;
	}
	return false;
}

/**
 * アクションリンクのスイッチ・変数による遷移チェック
 * @param	linkConditionData	アクションリンクの遷移データ
 * @return						遷移の可否
 */
bool ObjectAction::checkLinkConditionSwitchVariableChanged(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionSwitchVariableChangedData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionSwitchVariableChangedData *>(linkConditionData);
#endif
	CC_ASSERT(condition);

	// プロジェクトのプレイデータとオブジェクト固有のプレイデータを取得
	auto projectPlayData = GameManager::getInstance()->getPlayData();

	// スイッチの変化の場合
	if (condition->getSwtch()) {
		std::function<bool(agtk::data::PlaySwitchData *)> checkSwitchCondition = [&](agtk::data::PlaySwitchData *switchData) {
			if (switchData == nullptr) {
				return false;
			}
			// スイッチの状態で分岐
			switch (condition->getSwitchCondition())
			{
				// -------------------------------------------
				// スイッチがON
				// -------------------------------------------
				case agtk::data::ObjectActionLinkConditionSwitchVariableChangedData::kSwitchConditionOn:
				{
					if (nullptr != switchData && switchData->getValue() == true) {
						break;
					}
					return false;
				}
				// -------------------------------------------
				// スイッチがOFF
				// -------------------------------------------
				case agtk::data::ObjectActionLinkConditionSwitchVariableChangedData::kSwitchConditionOff:
				{
					if (nullptr != switchData && switchData->getValue() == false) {
						break;
					}
					return false;
				}
				// -------------------------------------------
				// スイッチがOFFからONになった
				// -------------------------------------------
				case agtk::data::ObjectActionLinkConditionSwitchVariableChangedData::kSwitchConditionOnFromOff:
				{
					if (nullptr != switchData && switchData->isState() == agtk::data::PlaySwitchData::kStateOnFromOff) {
						break;
					}
					return false;
				}
				// -------------------------------------------
				// スイッチがONからOFFになった
				// -------------------------------------------
				case agtk::data::ObjectActionLinkConditionSwitchVariableChangedData::kSwitchConditionOffFromOn:
				{
					if (nullptr != switchData && switchData->isState() == agtk::data::PlaySwitchData::kStateOffFromOn) {
						break;
					}
					return false;
				}
			}
			return true;
		};
		// プロジェクト共通のスイッチデータ取得
		if (condition->getSwitchObjectId() == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
			auto switchData = projectPlayData->getSwitchData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, condition->getSwitchId());
			if (checkSwitchCondition(switchData) == false) {
				return false;
			}
		}
		// オブジェクト固有のスイッチデータ取得
		else if (condition->getSwitchObjectId() == agtk::data::ObjectActionLinkConditionData::kSelfObject) {
			auto playObjectData = _object->getPlayObjectData();
			auto switchData = playObjectData->getSwitchData(condition->getSwitchId());
			if (checkSwitchCondition(switchData) == false) {
				return false;
			}
		}
		//ロックしたオブジェクト
		else if (condition->getSwitchObjectId() == agtk::data::ObjectActionLinkConditionData::kLockedObject) {
			if (condition->getSwitchQualifierId() == agtk::data::ObjectActionLinkConditionData::kQualifierSingle) {//単体
				bool bExistObject = false;
				auto objectList = this->getTargetObjectLocked();
				cocos2d::Ref *ref;
				CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto p = static_cast<agtk::Object *>(ref);
#else
					auto p = dynamic_cast<agtk::Object *>(ref);
#endif
					auto singleInstanceData = projectPlayData->getVariableData(p->getObjectData()->getId(), agtk::data::kObjectSystemVariableSingleInstanceID);
					auto object = this->getTargetObjectInstanceId((int)singleInstanceData->getValue(), agtk::SceneLayer::kTypeAll);
					if (object == nullptr) {
						continue;
					}
					auto playObjectData = object->getPlayObjectData();
					auto switchData = playObjectData->getSwitchData(condition->getSwitchId());
					if (checkSwitchCondition(switchData) == false) {
						continue;
					}
					bExistObject = true;
				}
				if (!bExistObject) return false;
			} else if (condition->getSwitchQualifierId() == agtk::data::ObjectActionLinkConditionData::kQualifierWhole) {//全体
				bool bExistObject = false;
				auto objectList = this->getTargetObjectLocked();
				cocos2d::Ref *ref;
				CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = static_cast<agtk::Object *>(ref);
#else
					auto object = dynamic_cast<agtk::Object *>(ref);
#endif
					auto playObjectData = object->getPlayObjectData();
					auto switchData = playObjectData->getSwitchData(condition->getSwitchId());
					if (checkSwitchCondition(switchData) == true) {
						bExistObject = true;
						break;
					}
				}
				if (!bExistObject) return false;
			}
			else {
				CC_ASSERT(0);
				return false;
			}
		}
		// 親オブジェクト
		else if (condition->getSwitchObjectId() == agtk::data::ObjectActionLinkConditionData::kParentObject) {
			// 親のスイッチ変化判定
			bool bExistObject = false;
			auto parentObjectInstanceId = _object->getPlayObjectData()->getParentObjectInstanceId();
			if (parentObjectInstanceId == -1) {
				return false;
			}
			auto parentObject = this->getTargetObjectInstanceId(parentObjectInstanceId, agtk::SceneLayer::kTypeAll);
			if (parentObject == nullptr) {
				return false;
			}
			auto playObjectData = parentObject->getPlayObjectData();
			auto switchData = playObjectData->getSwitchData(condition->getSwitchId());
			if (checkSwitchCondition(switchData) == false) {
				return false;
			}
		}
		else if (condition->getSwitchObjectId() == -1) {//設定無し。
			return false;
		}
		//オブジェクト（単体/全体）
		else if (condition->getSwitchObjectId() > 0) {
			switch (condition->getSwitchQualifierId()){
			case agtk::data::ObjectActionLinkConditionData::kQualifierSingle: {//単体
				auto singleInstanceData = projectPlayData->getVariableData(condition->getSwitchObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID);
				auto object = this->getTargetObjectInstanceId((int)singleInstanceData->getValue(), agtk::SceneLayer::kTypeAll);
				if (object == nullptr) {
					return false;
				}
				auto playObjectData = object->getPlayObjectData();
				auto switchData = playObjectData->getSwitchData(condition->getSwitchId());
				if (checkSwitchCondition(switchData) == false) {
					return false;
				}
				break; }
			case agtk::data::ObjectActionLinkConditionData::kQualifierWhole: {//全体
				bool bExistObject = false;
				auto objectList = this->getTargetObjectById(condition->getSwitchObjectId(), agtk::SceneLayer::kTypeAll);
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto object = static_cast<agtk::Object *>(ref);
#else
					auto object = dynamic_cast<agtk::Object *>(ref);
#endif
					auto playObjectData = object->getPlayObjectData();
					auto switchData = playObjectData->getSwitchData(condition->getSwitchId());
					if (checkSwitchCondition(switchData) == true) {
						bExistObject = true;
						break;
					}
				}
				if (!bExistObject) return false;
				break; }
			default:CC_ASSERT(0);
			}
		}
		else {
			//エラー
			CC_ASSERT(0);
		}
	}
	else {
		//浮動小数点比較（誤差を考慮）
		std::function<bool(double, double)> isEqual = [&](double a, double b) {
			return fabs(a - b) <= FLT_EPSILON;
		};

		std::function<bool(double, agtk::data::PlayVariableData *)> checkVariableCondition = [&](double compareValue, agtk::data::PlayVariableData *variableData) {

			if (variableData == nullptr) {
				return false;
			}
			double src_value = variableData->getValue();
			// 比較演算子タイプ取得
			auto op = condition->getCompareVariableOperator();
			bool src_isnan = std::isnan(src_value);
			bool compare_isnan = std::isnan(compareValue);

			// 比較演算子タイプ別処理
			if (src_isnan || compare_isnan) {
				//非数が含まれている場合は特別処理。
				switch (op) {
				case 0:// "<"
					break;
				case 1:// "<="
					if (src_isnan && compare_isnan) {
						return true;
					}
					break;
				case 2:// "="
					if (src_isnan && compare_isnan) {
						return true;
					}
					break;
				case 3:// ">="
					if (src_isnan && compare_isnan) {
						return true;
					}
					break;
				case 4:// ">"
					break;
				case 5:// "!="
					if (src_isnan != compare_isnan) {
						return true;
					}
					break;
				default:CC_ASSERT(0);
				}
			}
			else {
				switch (op) {
				case 0:// "<"
					if (src_value < compareValue) {
						return true;
					}
					break;
				case 1:// "<="
					if (src_value <= compareValue || isEqual(src_value, compareValue) == true) {
						return true;
					}
					break;
				case 2:// "="
					if (src_value == compareValue || isEqual(src_value, compareValue) == true) {
						return true;
					}
					break;
				case 3:// ">="
					if (src_value >= compareValue || isEqual(src_value, compareValue) == true) {
						return true;
					}
					break;
				case 4:// ">"
					if (src_value > compareValue) {
						return true;
					}
					break;
				case 5:// "!="
					if (isEqual(src_value, compareValue) == false) {//src_value != compareValue
						return true;
					}
					break;
				default:CC_ASSERT(0);
				}
			}
			return false;
		};

		// 比較用の値
		double compareValue = 0;

		// 比較対象のタイプで分岐
		switch (condition->getCompareValueType())
		{
			// -------------------------------------------
			// 定数で比較
			// -------------------------------------------
			case agtk::data::ObjectActionLinkConditionSwitchVariableChangedData::kVariableCompareValue:
			{
				compareValue = condition->getCompareValue();
				break;
			}
			// -------------------------------------------
			// 特定の変数で比較
			// -------------------------------------------
			case agtk::data::ObjectActionLinkConditionSwitchVariableChangedData::kVariableCompareVariable:
			{
#if 1
				if (!GameManager::getInstance()->getVariableValue(condition->getCompareVariableObjectId(), condition->getCompareVariableQualifierId(), condition->getCompareVariableId(), compareValue, _object)) {
					return false;
				}
#else
				agtk::data::PlayVariableData *compareVariableData = nullptr;
				// プロジェクトの共通変数データ取得
				if (condition->getCompareVariableObjectId() == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
					compareVariableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, condition->getCompareVariableId());
				}
				//自身
				else if (condition->getCompareVariableObjectId() == agtk::data::ObjectActionLinkConditionData::kSelfObject) {
					auto playObjectData = _object->getPlayObjectData();
					if (condition->getCompareVariableId() == -1) {//設定無し。
						return false;
					}
					compareVariableData = playObjectData->getVariableData(condition->getCompareVariableId());
				}
				else if (condition->getCompareVariableObjectId() == -1) {//設定無し。
					return false;
				}
				//オブジェクト（単体/全体）
				else if (condition->getCompareVariableObjectId() > 0) {
					CC_ASSERT(condition->getCompareVariableQualifierId() == agtk::data::ObjectActionLinkConditionData::kQualifierSingle);
					auto singleInstanceData = projectPlayData->getVariableData(condition->getCompareVariableObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID);
					auto object = this->getTargetObjectInstanceId((int)singleInstanceData->getValue(), agtk::SceneLayer::kTypeAll);
					if (object == nullptr) {
						return false;
					}
					auto playObjectData = object->getPlayObjectData();
					if (condition->getCompareVariableId() == -1) {//設定無し。
						return false;
					}
					compareVariableData = playObjectData->getVariableData(condition->getCompareVariableId());
				}
				else {
					//エラー
					CC_ASSERT(0);
				}
				compareValue = compareVariableData->getValue();
#endif
				break;
			}
			// -------------------------------------------
			// 非数で比較
			// -------------------------------------------
			case agtk::data::ObjectActionLinkConditionSwitchVariableChangedData::kVariableCompareNaN:
			{
				compareValue = std::nan("1");
				break;
			}
			default:CC_ASSERT(0);
		}

		// プロジェクト共通の変数データ取得
		if (condition->getVariableObjectId() == agtk::data::PlayData::COMMON_PLAY_DATA_ID) {
			auto variableData = projectPlayData->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, condition->getVariableId());
			if (checkVariableCondition(compareValue, variableData) == false) {
				return false;
			}
		}
		// オブジェクト固有のスイッチデータ取得
		else if (condition->getVariableObjectId() == agtk::data::ObjectActionLinkConditionData::kSelfObject) {
			auto playObjectData = _object->getPlayObjectData();
			if (condition->getVariableId() == -1) {//設定無し。
				return false;
			}
			auto variableData = playObjectData->getVariableData(condition->getVariableId());
			if (checkVariableCondition(compareValue, variableData) == false) {
				return false;
			}
		}
		//ロックしたオブジェクト
		else if (condition->getVariableObjectId() == agtk::data::ObjectActionLinkConditionData::kLockedObject) {
			if (condition->getVariableQualifierId() == agtk::data::ObjectActionLinkConditionData::kQualifierSingle) {//単体
				bool bExistObject = false;
				auto objectList = this->getTargetObjectLocked();
				cocos2d::Ref *ref;
				CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto p = static_cast<agtk::Object *>(ref);
#else
					auto p = dynamic_cast<agtk::Object *>(ref);
#endif
					auto singleInstanceData = projectPlayData->getVariableData(p->getObjectData()->getId(), agtk::data::kObjectSystemVariableSingleInstanceID);
					auto obj = this->getTargetObjectInstanceId((int)singleInstanceData->getValue(), agtk::SceneLayer::kTypeAll);
					if (obj == nullptr) {
						continue;
					}
					auto playObjectData = obj->getPlayObjectData();
					auto variableData = playObjectData->getVariableData(condition->getVariableId());
					if (checkVariableCondition(compareValue, variableData) == false) {
						continue;
					}
					//条件に該当あり。
					bExistObject = true;
				}
				if (!bExistObject) return false;
			}
			else if (condition->getVariableQualifierId() == agtk::data::ObjectActionLinkConditionData::kQualifierWhole) {//全体
				bool bExistObject = false;
				auto objectList = this->getTargetObjectLocked();
				cocos2d::Ref *ref = nullptr;
				CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto obj = static_cast<agtk::Object *>(ref);
#else
					auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
					auto playObjectData = obj->getPlayObjectData();
					auto variableData = playObjectData->getVariableData(condition->getVariableId());
					if (checkVariableCondition(compareValue, variableData) == true) {
						bExistObject = true;
						break;
					}
				}
				if (!bExistObject) return false;
			}
			else {
				CC_ASSERT(0);
				return false;
			}
		}
		//親オブジェクト
		else if (condition->getVariableObjectId() == agtk::data::ObjectActionLinkConditionData::kParentObject) {
			// 親の変数変化判定
			auto parentObjectInstanceId = _object->getPlayObjectData()->getParentObjectInstanceId();
			if (parentObjectInstanceId == -1) {
				return false;
			}
			auto parentObject = this->getTargetObjectInstanceId(parentObjectInstanceId, agtk::SceneLayer::kTypeAll);
			if (parentObject == nullptr) {
				return false;
			}
			auto playObjectData = parentObject->getPlayObjectData();
			auto variableData = playObjectData->getVariableData(condition->getVariableId());
			if (checkVariableCondition(compareValue, variableData) == false) {
				return false;
			}
		}
		//オブジェクト（単体/全体）
		else if (condition->getVariableObjectId() > 0) {
			switch (condition->getVariableQualifierId()) {
			case agtk::data::ObjectActionLinkConditionData::kQualifierSingle: {//単体
				agtk::data::PlayVariableData *variableData = nullptr;
				auto variableId = condition->getVariableId();
				if (variableId == agtk::data::kObjectSystemVariableSingleInstanceID || variableId == agtk::data::kObjectSystemVariableInstanceCount) {
					variableData = projectPlayData->getVariableData(condition->getVariableObjectId(), variableId);
				}
				else {
					auto singleInstanceData = projectPlayData->getVariableData(condition->getVariableObjectId(), agtk::data::kObjectSystemVariableSingleInstanceID);
					auto object = this->getTargetObjectInstanceId((int)singleInstanceData->getValue(), agtk::SceneLayer::kTypeAll);
					if (object == nullptr) {
						return false;
					}
					auto playObjectData = object->getPlayObjectData();
					variableData = playObjectData->getVariableData(variableId);
				}
				if (checkVariableCondition(compareValue, variableData) == false) {
					return false;
				}
				break; }
			case agtk::data::ObjectActionLinkConditionData::kQualifierWhole: {//全体
				bool bExistObject = false;
				auto variableId = condition->getVariableId();
				if (variableId == agtk::data::kObjectSystemVariableSingleInstanceID || variableId == agtk::data::kObjectSystemVariableInstanceCount) {
					auto variableData = projectPlayData->getVariableData(condition->getVariableObjectId(), variableId);
					if (checkVariableCondition(compareValue, variableData) == true) {
						bExistObject = true;
						break;
					}
				}
				else {
					auto objectList = this->getTargetObjectById(condition->getVariableObjectId(), agtk::SceneLayer::kTypeAll);
					cocos2d::Ref *ref = nullptr;
					CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
						auto object = static_cast<agtk::Object *>(ref);
#else
						auto object = dynamic_cast<agtk::Object *>(ref);
#endif
						auto playObjectData = object->getPlayObjectData();
						auto variableData = playObjectData->getVariableData(condition->getVariableId());
						if (checkVariableCondition(compareValue, variableData) == true) {
							bExistObject = true;
							break;
						}
					}
				}
				if (!bExistObject) return false;
				break; }
			default:CC_ASSERT(0);
			}
		}
	}
	return true;
}

bool ObjectAction::checkLinkConditionAnimationFinished(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionAnimationFinishedData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionAnimationFinishedData *>(linkConditionData);
#endif
	CC_ASSERT(condition);
	auto basePlayer = _object->getBasePlayer();
	if (basePlayer == nullptr) {
		//ゼロフレームのアニメーションとして、遷移を進める。
		return true;
	}
	auto animationMotion = basePlayer->getCurrentAnimationMotion();
	if (animationMotion->isAllAnimationFinished() == true) {
		return true;
	}
	return false;
}

bool ObjectAction::checkLinkConditionJumpTop(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionJumpTopData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionJumpTopData *>(linkConditionData);
#endif
	CC_ASSERT(condition);
	if (_object->_jumpTop == true) {
		return true;
	}
	return false;
}

bool ObjectAction::checkLinkConditionObjectActionChanged(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionObjectActionChangedData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionObjectActionChangedData *>(linkConditionData);
#endif
	CC_ASSERT(condition);
	auto scene = this->getScene();
	if (scene != nullptr) {
		auto objectId = condition->getObjectId();
		auto actionObjectId = objectId;
		if (objectId == agtk::data::ObjectCommandData::kSelfObject) {
			actionObjectId = _object->getObjectData()->getId();
		} else
		if(objectId == agtk::data::ObjectCommandData::kChildObject
		|| objectId == agtk::data::ObjectCommandData::kLockedObject
		|| objectId == agtk::data::ObjectCommandData::kParentObject){
			actionObjectId = condition->getActionObjectId();
		}
		const char *actionName = nullptr;
		if (actionObjectId >= 0) {
			auto projectData = GameManager::getInstance()->getProjectData();
			auto objectData = projectData->getObjectData(actionObjectId);
			if (objectData) {
				auto actionData = objectData->getActionData(condition->getActionId());
				if (actionData) {
					actionName = actionData->getName();
				}
			}
		}
		auto isActionMatched = [&](agtk::Object *object) {
			auto objectAction = object->getCurrentObjectAction();
			auto actionMatched = (actionName && strcmp(objectAction->getObjectActionData()->getName(), actionName) == 0);
			if (actionMatched && condition->getOtherActions() == false) {
				return true;
			}
			if (!actionMatched && condition->getOtherActions() == true) {
				return true;
			}
			return false;
		};
		if (objectId == agtk::data::ObjectCommandData::kSelfObject) {
			if (isActionMatched(_object)) {
				return true;
			}
		}
		else if (objectId == agtk::data::ObjectCommandData::kParentObject) {
			auto parentObject = _object->getOwnParentObject();
			if (parentObject && isActionMatched(parentObject)) {
				return true;
			}
		}
		else {
			cocos2d::Array *objectList = nullptr;
			if (objectId == agtk::data::ObjectCommandData::kChildObject) {
				objectList = _object->getChildrenObjectList();
			}
			else {
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
				objectList = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
				objectList = scene->getObjectAll(this->getSceneLayer()->getType());
#endif
			}
			cocos2d::Ref *ref = nullptr;
			CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto object = static_cast<agtk::Object *>(ref);
#else
				auto object = dynamic_cast<agtk::Object *>(ref);
#endif
				if (objectId >= 0) {
					if (object->getObjectData()->getId() != objectId) {
						continue;
					}
				}
				else if (objectId == agtk::data::ObjectCommandData::kLockedObject) {
					auto playObjectData = object->getPlayObjectData();
					if (!playObjectData->isLocked(_object->getInstanceId())) {
						continue;
					}
				}
				if (isActionMatched(object)) {
					return true;
				}
			}
		}
	}
	return false;
}

bool ObjectAction::checkLinkConditionSlopeTouched(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionSlopeTouchedData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionSlopeTouchedData *>(linkConditionData);
#endif
	CC_ASSERT(condition);

	int bit = _object->getSlopeBit();

	// 上辺が坂に接触しているか？
	bool isHitUp = (bit & agtk::data::ObjectActionLinkConditionData::kSlopeBitUp);
	// 下辺が坂に接触しているか？
	bool isHitDown = (bit & agtk::data::ObjectActionLinkConditionData::kSlopeBitDown);

	// 「坂に対し接触している方向を指定」
	switch (condition->getDirectionType())
	{
		// 「坂に対し上から接触」
		case agtk::data::ObjectActionLinkConditionSlopeTouchedData::kDirectionUpper:
		{
			// 下辺が坂に接触していない場合は条件未達成
			if (!isHitDown) { return false; }
		} break;

		// 「坂に対し下から接触」
		case agtk::data::ObjectActionLinkConditionSlopeTouchedData::kDirectionLower:
		{
			// 上辺が坂に接触していない場合は条件未達成
			if (!isHitUp) { return false; }
		} break;

		// 「指定しない」
		case agtk::data::ObjectActionLinkConditionSlopeTouchedData::kDirectionNone:
		{
			// 上辺、下辺いずれも接触していない場合は条件未達成
			if (!isHitDown && !isHitUp) { return false; }
		} break;

		default:
		{
			CC_ASSERT(0); // 不正なパラメータを設定
		} break;
	}

	auto slopeList = _object->getSlopeTouchedList();
	cocos2d::Ref *ref = nullptr;

	// 「坂の種類を指定」
	switch (condition->getDownwardType()) 
	{
		// 「左に下っている」
		case agtk::data::ObjectActionLinkConditionSlopeTouchedData::kDownwardLeft:
		{
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			for(int i = 0; i < (int)slopeList->size(); i++){
				auto slope = (*slopeList)[i];
#else
			CCARRAY_FOREACH(slopeList, ref)
			{
				auto slope = dynamic_cast<agtk::Slope*>(ref);
#endif

				// 坂が上り坂の場合は条件達成とする
				if (slope->getType() == Slope::kTypeUp) {
					return true;
				}
			}
		} break;

		// 「右に下っている」
		case agtk::data::ObjectActionLinkConditionSlopeTouchedData::kDownwardRight:
		{
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			for (int i = 0; i < (int)slopeList->size(); i++) {
				auto slope = (*slopeList)[i];
#else
			CCARRAY_FOREACH(slopeList, ref)
			{
				auto slope = dynamic_cast<agtk::Slope*>(ref);
#endif

				// 坂が下り坂の場合は条件達成とする
				if (slope->getType() == Slope::kTypeDown) {
					return true;
				}
			}
		} break;

		// 「指定しない」
		case agtk::data::ObjectActionLinkConditionSlopeTouchedData::kDirectionNone:
		{
			// 接触方向の条件を達成していないものは、上で省かれているはずなので
			// ここでは無条件で条件達成とする
			return true;
		} break;

		default:
		{
			CC_ASSERT(0); // 不正なパラメータを設定
		} break;
	}

	return false;
}

bool ObjectAction::checkLinkConditionBuriedInWallData(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionBuriedInWallData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionBuriedInWallData *>(linkConditionData);
#endif
	CC_ASSERT(condition);
	cocos2d::__Array *objectList = nullptr;
	int objectId = condition->getObjectId();
	if (objectId == -1) {
		//設定無し。
		return false;
	}
	if (objectId == agtk::data::ObjectActionLinkConditionData::kSelfObject) {//自身のオブジェクト
		objectList = cocos2d::__Array::create();
		objectList->addObject(_object);
	}
	else if (objectId == agtk::data::ObjectActionLinkConditionData::kOtherThanSelfObject) {//自身以外のオブジェクト
		auto scene = this->getScene();
		// ※getObjectAllReference()との置き換え不可
		objectList = scene->getObjectAll(this->getSceneLayer()->getType());
		objectList->removeObject(_object);
	}
	else if (objectId > 0) {
		objectList = this->getTargetObjectById(objectId);
	}

	if (objectList->count() == 0) {
		return false;
	}

	//壁に埋まっているかチェックする。
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object = static_cast<agtk::Object *>(ref);
#else
		auto object = dynamic_cast<agtk::Object *>(ref);
#endif
		if (object->isBuriedInWall() == false) {
			return false;
		}
	}
	return true;
}

bool ObjectAction::checkLinkConditionNoObjectWallTouched(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionNoObjectWallTouchedData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionNoObjectWallTouchedData *>(linkConditionData);
#endif
	int bit = condition->getWallBit();
	cocos2d::Ref *ref = nullptr;
	auto leftWallObjectList = _object->getLeftWallObjectList();
	auto rightWallObjectList = _object->getRightWallObjectList();
	auto upWallObjectList = _object->getUpWallObjectList();
	auto downWallObjectList = _object->getDownWallObjectList();
	std::function<bool(Object *)> isTargetObjectMatched = [&isTargetObjectMatched, condition](Object *obj)
	{
		if (condition->getObjectType() == 0) {//オブジェクトの種類を指定
			if (condition->getObjectTypeByType() == 0) {//すべてのオブジェクト
				return true;
			}
			else if (condition->getObjectTypeByType() == 1) {//プレイヤー
				if (obj->getObjectData()->isGroupPlayer()) {
					return true;
				}
			}
			else if (condition->getObjectTypeByType() == 2) {//エネミー
				if (obj->getObjectData()->isGroupEnemy()) {
					return true;
				}
			}
		}
		else if (condition->getObjectType() == 1) {//オブジェクトで指定
												   //オブジェクト指定（-2:自身のオブジェクト,-3:自身以外のオブジェクト,それ以外はオブジェクトID）
			if (condition->getObjectId() == -2) {//自身のオブジェクト
				CC_ASSERT(0);
			}
			else if (condition->getObjectId() == -3) {//自身以外のオブジェクト
				return true;
			}
			else {//オブジェクトID
				if (condition->getObjectId() == obj->getObjectData()->getId()) {
					return true;
				}
			}
		}
		else if (condition->getObjectType() == 2) {//指定しない
			return true;
		}
		return false;
	};
	if ((bit & agtk::data::ObjectActionLinkConditionData::kWallBitUp) && upWallObjectList->count() > 0) {//上
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		auto size = upWallObjectList->size();
		for(int i = 0; i < (int)size; i++){
			auto obj = (*upWallObjectList)[i];
#else
		CCARRAY_FOREACH(upWallObjectList, ref) {
			auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
			if (isTargetObjectMatched(obj)) {
				return false;
			}
		}
	}
	if ((bit & agtk::data::ObjectActionLinkConditionData::kWallBitLeft) && leftWallObjectList->count() > 0) {//左
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		auto size = leftWallObjectList->size();
		for (int i = 0; i < (int)size; i++) {
			auto obj = (*leftWallObjectList)[i];
#else
		CCARRAY_FOREACH(leftWallObjectList, ref) {
			auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
			if (isTargetObjectMatched(obj)) {
				return false;
			}
		}
	}
	if ((bit & agtk::data::ObjectActionLinkConditionData::kWallBitRight) && rightWallObjectList->count() > 0) {//右
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		auto size = rightWallObjectList->size();
		for (int i = 0; i < (int)size; i++) {
			auto obj = (*rightWallObjectList)[i];
#else
		CCARRAY_FOREACH(rightWallObjectList, ref) {
			auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
			if (isTargetObjectMatched(obj)) {
				return false;
			}
		}
	}
	if ((bit & agtk::data::ObjectActionLinkConditionData::kWallBitDown) && downWallObjectList->count() > 0) {//下
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		auto size = downWallObjectList->size();
		for (int i = 0; i < (int)size; i++) {
			auto obj = (*downWallObjectList)[i];
#else
		CCARRAY_FOREACH(downWallObjectList, ref) {
			auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
			if (isTargetObjectMatched(obj)) {
				return false;
			}
		}
	}
	return true;
}

bool ObjectAction::checkLinkConditionScript(agtk::data::ObjectActionLinkConditionData *linkConditionData, agtk::data::ObjectActionLinkData *objectActionLinkData)
{
	int linkId = objectActionLinkData->getId();
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionScriptData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionScriptData *>(linkConditionData);
#endif
	auto script = condition->getScript();

	if (strlen(script) > 0) {
#ifdef USE_SCRIPT_PRECOMPILE
		//JSB_AUTOCOMPARTMENT_WITH_GLOBAL_OBJCET
		auto sc = ScriptingCore::getInstance();
		auto cx = sc->getGlobalContext();
		auto _global = sc->getGlobalObject();
		JS::RootedObject gobj(cx, _global);
		JSAutoCompartment ac(cx, gobj);

		JS::RootedObject ns(cx);
		JS::MutableHandleObject jsObj = &ns;
		JS::RootedValue nsval(cx);
		JS_GetProperty(cx, gobj, "Agtk", &nsval);
		JS::RootedValue rv(cx);
		auto commonActionLinkIndex = objectActionLinkData->getCommonActionSettingData() ? (linkId > 0 ? 1 : 0) : -1;
		auto scriptLinkId = objectActionLinkData->getCommonActionSettingData() ? objectActionLinkData->getCommonActionSettingData()->getId() : linkId;
		bool ret = false;
		if (nsval != JSVAL_VOID) {
			jsObj.set(nsval.toObjectOrNull());
			JS::RootedValue v(cx);
			JS_GetProperty(cx, jsObj, "scriptFunctions", &v);
			if (v.isObject()) {
				JS::RootedObject rscriptFunctions(cx, &v.toObject());
				JS_GetProperty(cx, rscriptFunctions, "execObjectLinkConditionScript", &v);
				if (v.isObject()) {
					JS::RootedValue rexec(cx, v);
					jsval args[5];
					args[0] = JS::Int32Value(_object->getObjectData()->getId());
					args[1] = JS::Int32Value(_object->getInstanceId());
					args[2] = JS::Int32Value(scriptLinkId);
					args[3] = JS::Int32Value(linkConditionData->getId());
					args[4] = JS::Int32Value(commonActionLinkIndex);
					ret = JS_CallFunctionValue(cx, rscriptFunctions, rexec, JS::HandleValueArray::fromMarkedLocation(5, args), &rv);
				}
			}
		}
#else
		auto scriptingCore = ScriptingCore::getInstance();
		auto context = scriptingCore->getGlobalContext();
		JS::RootedValue rv(context);
		JS::MutableHandleValue mhv(&rv);
		auto js = String::createWithFormat("(function(){ var objectId = %d; var instanceId = %d; return (%s\n); })()", _object->getObjectData()->getId(), _object->getInstanceId(), script);
		auto ret = ScriptingCore::getInstance()->evalString(js->getCString(), mhv);
#endif
		if (!ret) {
			//スクリプトエラー
			auto errorStr = String::createWithFormat("Runtime error in linkConditionScript(objectId: %d, instanceId: %d, script: %s).", _object->getObjectData()->getId(), _object->getInstanceId(), script)->getCString();
			agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
			auto fp = GameManager::getScriptLogFp();
			if (fp) {
				fwrite(errorStr, 1, strlen(errorStr), fp);
				fwrite("\n", 1, 1, fp);
			}
#endif
			return false;
		}
		else if (rv.isNumber()) {
			return JavascriptManager::getInt32(rv) != 0;
		}
		else if (rv.isBoolean()) {
			return rv.toBoolean();
		}
		else {
			//数値でない
			return false;
		}
	}
	return false;
}

bool ObjectAction::checkLinkConditionObjectHit(agtk::data::ObjectActionLinkConditionData *linkConditionData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionObjectHitData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionObjectHitData *>(linkConditionData);
#endif
	CC_ASSERT(condition);
	std::vector<agtk::Vertex4> attackCollision;
	_object->getTimelineList(agtk::data::TimelineInfoData::kTimelineAttack, attackCollision);
	if (attackCollision.size() == 0) {
		return false;
	}
	auto scene = this->getScene();
	auto objectList = cocos2d::__Array::create();
	switch (condition->getObjectType()) {
	case agtk::data::ObjectActionLinkConditionData::kObjectByGroup: {//オブジェクトの種類を指定
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
		auto objectAll = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
		auto objectAll = scene->getObjectAll(this->getSceneLayer()->getType());
#endif
		cocos2d::Ref *ref;
		CCARRAY_FOREACH(objectAll, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto obj = static_cast<agtk::Object *>(ref);
#else
			auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
			if (obj == _object) continue;
			if (condition->getObjectGroup() == agtk::data::ObjectActionLinkConditionData::kObjectTypeAll || condition->getObjectGroup() == obj->getObjectData()->getGroup()) {
				objectList->addObject(obj);
			}
		}
		break; }
	case agtk::data::ObjectActionLinkConditionData::kObjectById: {//オブジェクトで指定
		switch(condition->getObjectId()) {
		case -1: break;//設定なし（空白）
		case agtk::data::ObjectActionLinkConditionData::kSelfObject: {//自身のオブジェクト
			objectList->addObject(_object);
			break; }
		case agtk::data::ObjectActionLinkConditionData::kOtherThanSelfObject: {//自身以外のオブジェクト
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
			auto objectAll = scene->getObjectAllReference(this->getSceneLayer()->getType());
#else
			auto objectAll = scene->getObjectAll(this->getSceneLayer()->getType());
#endif
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(objectAll, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto obj = static_cast<agtk::Object *>(ref);
#else
				auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
				if (obj != _object) {
					objectList->addObject(obj);
				}
			}
			break; }
		default: {//オブジェクトID
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
			auto objectAll = scene->getObjectAll(condition->getObjectId(), this->getSceneLayer()->getType());
			objectList->addObjectsFromArray(objectAll);
#else
			auto objectAll = scene->getObjectAll(this->getSceneLayer()->getType());
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(objectAll, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto obj = static_cast<agtk::Object *>(ref);
#else
				auto obj = dynamic_cast<agtk::Object *>(ref);
#endif
				if (obj->getObjectData()->getId() == condition->getObjectId()) {
					objectList->addObject(obj);
				}
			}
#endif
			break; }
		}
		break; }
	default: /* CC_ASSERT(0); */ CCLOG("%d,%s", __LINE__, __FUNCTION__); break;
	}

	if (objectList->count() <= 0) {
		return false;
	}

	// 攻撃判定から当たり判定をチェックしたリストを走査
	int wallBit = condition->getWallBit();
	auto collisionAttackHitList = _object->getCollisionAttackHitList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(collisionAttackHitList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto temp = static_cast<ObjectCollision::ObjectWallIntersectTemp *>(ref);
#else
		auto temp = dynamic_cast<ObjectCollision::ObjectWallIntersectTemp *>(ref);
#endif

		// 対象オブジェクトの場合
		if (objectList->getIndexOfObject(temp->getObject()) >= 0) {
			auto wallList = temp->getWallList();
			auto bit = agtk::ObjectWallIntersect::getWallBit(wallList);
			if ((bit & wallBit) & agtk::data::ObjectActionLinkConditionData::kWallBitUp) {//上
				return true;
			}
			if ((bit & wallBit) & agtk::data::ObjectActionLinkConditionData::kWallBitLeft) {//左
				return true;
			}
			if ((bit & wallBit) & agtk::data::ObjectActionLinkConditionData::kWallBitRight) {//右
				return true;
			}
			if ((bit & wallBit) & agtk::data::ObjectActionLinkConditionData::kWallBitDown) {//下
				return true;
			}
		}
	}

	return false;
}

bool ObjectAction::checkLinkConditionCustom(agtk::data::ObjectActionLinkConditionData *linkConditionData, agtk::data::ObjectActionLinkData *objectActionLinkData)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto condition = static_cast<agtk::data::ObjectActionLinkConditionCustomData *>(linkConditionData);
#else
	auto condition = dynamic_cast<agtk::data::ObjectActionLinkConditionCustomData *>(linkConditionData);
#endif

	int pluginId = (linkConditionData->getType() - agtk::data::ObjectActionLinkConditionCustomData::kConditionCustomHead) / agtk::data::ObjectActionLinkConditionCustomData::kPluginConditionCustomMax;
	int index = (linkConditionData->getType() - agtk::data::ObjectActionLinkConditionCustomData::kConditionCustomHead) % agtk::data::ObjectActionLinkConditionCustomData::kPluginConditionCustomMax;

	//pluginIdのindex番目のリンク条件を処理させる。
#ifdef USE_SCRIPT_PRECOMPILE
	//JSB_AUTOCOMPARTMENT_WITH_GLOBAL_OBJCET
	auto sc = ScriptingCore::getInstance();
	auto cx = sc->getGlobalContext();
	auto _global = sc->getGlobalObject();
	JS::RootedObject gobj(cx, _global);
	JSAutoCompartment ac(cx, gobj);

	JS::RootedObject ns(cx);
	JS::MutableHandleObject jsObj = &ns;
	JS::RootedValue nsval(cx);
	JS_GetProperty(cx, gobj, "Agtk", &nsval);
	JS::RootedValue rv(cx);
	bool ret = false;
	if (nsval != JSVAL_VOID) {
		jsObj.set(nsval.toObjectOrNull());
		JS::RootedValue v(cx);
		JS_GetProperty(cx, jsObj, "plugins", &v);
		if (v.isObject()) {
			JS::RootedObject rplugins(cx, &v.toObject());
			JS_GetProperty(cx, rplugins, "execLinkCondition", &v);
			if (v.isObject()) {
				JS::RootedValue rexecLinkCondition(cx, v);
				jsval args[7];
				args[0] = JS::Int32Value(pluginId);
				args[1] = JS::Int32Value(index);
				args[2] = std_string_to_jsval(cx, condition->getValueJson());
				args[3] = JS::Int32Value(_objectData->getId());
				args[4] = JS::Int32Value(_object->getInstanceId());
				args[5] = JS::Int32Value(this->getId());
				args[6] = JS::Int32Value(linkConditionData->getId());
				ret = JS_CallFunctionValue(cx, rplugins, rexecLinkCondition, JS::HandleValueArray::fromMarkedLocation(7, args), &rv);
			}
		}
	}
#else
	auto scriptingCore = ScriptingCore::getInstance();
	auto context = scriptingCore->getGlobalContext();
	JS::RootedValue rv(context);
	JS::MutableHandleValue mhv(&rv);
	auto script = String::createWithFormat("(function(){ var plugin = Agtk.plugins.getById(%d); if(plugin != null){ if(plugin.execLinkCondition){ return plugin.execLinkCondition(%d, %s, %d, %d, %d); } } return false; })()", pluginId, index, condition->getValueJson(), _objectData->getId(), _object->getInstanceId(), objectActionLinkData->getId());
	//CCLOG("script: %s", script->getCString());
	auto ret = ScriptingCore::getInstance()->evalString(script->getCString(), mhv);
#endif
	if (!ret) {
		//スクリプトエラー
		auto errorStr = String::createWithFormat("Runtime error in linkConditionCustom(pluginId: %d, index: %d, value: %s, objectId: %d, instanceId: %d, linkId: %d).", pluginId, index, condition->getValueJson(), _objectData->getId(), _object->getInstanceId(), objectActionLinkData->getId())->getCString();
		agtk::DebugManager::getInstance()->getDebugExecuteLogWindow()->addLog(errorStr);
#if defined(USE_PREVIEW)
		auto fp = GameManager::getScriptLogFp();
		if (fp) {
			fwrite(errorStr, 1, strlen(errorStr), fp);
			fwrite("\n", 1, 1, fp);
		}
#endif
		return false;
	}
	if (rv.isBoolean()) {
		return rv.toBoolean();
	}
	return false;
}

void ObjectAction::setup(bool bClearFlag)
{
	if (bClearFlag == false) {
		return;
	}
	this->setDuration(0.0f);
	_waitDuration300 = 0;

	//remove objCommandData
	this->getObjCommandList()->removeAllObjects();
	//add objCommandData
	auto scenePartData = _object->getScenePartObjectData();
	cocos2d::__Dictionary *objCommandList = nullptr;
	if (scenePartData == nullptr) {
		objCommandList = this->getObjectActionData()->getObjCommandList();
	}
	else {
		if (this->getIsCommon()) {
			int commonActionSettingId = this->getCommonActionSettingId();
			objCommandList = this->getObjCommandListByInstanceConfigurable(
				this->getObjectActionData()->getObjCommandList(),
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				static_cast<cocos2d::__Dictionary *>(scenePartData->getCommonActionCommandListObject()->objectForKey(commonActionSettingId)),
#else
				dynamic_cast<cocos2d::__Dictionary *>(scenePartData->getCommonActionCommandListObject()->objectForKey(commonActionSettingId)),
#endif
				_object
			);
		}
		else {
			int animMotionId = this->getObjectActionData()->getId();
			objCommandList = this->getObjCommandListByInstanceConfigurable(
				this->getObjectActionData()->getObjCommandList(),
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				static_cast<cocos2d::__Dictionary *>(scenePartData->getActionCommandListObject()->objectForKey(animMotionId)),
#else
				dynamic_cast<cocos2d::__Dictionary *>(scenePartData->getActionCommandListObject()->objectForKey(animMotionId)),
#endif
				_object
			);
		}
	}
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(objCommandList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto commandData = static_cast<agtk::data::ObjectCommandData *>(el->getObject());
#else
		auto commandData = dynamic_cast<agtk::data::ObjectCommandData *>(el->getObject());
#endif
		this->getObjCommandList()->addObject(commandData);
	}
	_pushPullCommandIdMapEffectedObjectIdSet.clear();
}

cocos2d::__Dictionary *ObjectAction::getObjCommandListByInstanceConfigurable(cocos2d::__Dictionary *baseObjCommandList, cocos2d::__Dictionary *instanceObjCommandList, agtk::Object *object)
{
	//スタートポイントオブジェクトかどうかチェックする。
	bool bStartPointObject = false;
	auto scenePartObjectData = object->getScenePartObjectData();
	if (scenePartObjectData) {
		bStartPointObject = scenePartObjectData->isStartPointObjectData();
	}

	auto objCommandList = cocos2d::__Dictionary::create();
	CC_ASSERT(objCommandList);
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(baseObjCommandList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto baseCommandData = static_cast<agtk::data::ObjectCommandData *>(el->getObject());
#else
		auto baseCommandData = dynamic_cast<agtk::data::ObjectCommandData *>(el->getObject());
#endif
		int commandDataId = el->getIntKey();
		if (baseCommandData->getInstanceConfigurable() && !bStartPointObject) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto instanceCommandData = static_cast<agtk::data::ObjectCommandData *>(instanceObjCommandList->objectForKey(commandDataId));
#else
			auto instanceCommandData = dynamic_cast<agtk::data::ObjectCommandData *>(instanceObjCommandList->objectForKey(commandDataId));
#endif
			CC_ASSERT(instanceCommandData);
			CC_ASSERT(instanceCommandData->getId() == commandDataId);
			objCommandList->setObject(instanceCommandData, commandDataId);
		}
		else {
			CC_ASSERT(baseCommandData->getId() == commandDataId);
			objCommandList->setObject(baseCommandData, commandDataId);
		}
	}
	return objCommandList;
}

cocos2d::__Dictionary *ObjectAction::getObjCommandListByInstanceConfigurable(cocos2d::__Dictionary *baseObjCommandList, cocos2d::__Dictionary *instanceObjCommandList)
{
	return ObjectAction::getObjCommandListByInstanceConfigurable(baseObjCommandList, instanceObjCommandList, _object);
}

NS_AGTK_END
