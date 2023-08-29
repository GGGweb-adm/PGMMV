#ifndef __OBJECT_COMMAND_H__
#define	__OBJECT_COMMAND_H__

#include "Lib/Macros.h"
#include "Data/ObjectData.h"
#include "Data/DatabaseData.h"

NS_AGTK_BEGIN
//-------------------------------------------------------------------------------------------------------------------
class Object;
class Scene;
class SceneLayer;
class PlayVariableData;
class AGTKPLAYER_API ObjectAction : public cocos2d::Ref
{
public:
	const int TARGET_NONE = -1;//「設定無し」

	const char* RESOURCE_SET_STR = "Set Resources";
	const char* MOTION_STR = "Motion";

	// 対象オブジェクトグループ
	enum EnumTargetObjectGroup
	{
		kAll,
		kPlayer,
		kEnemy
	};
	enum EnumCommandBehavior {
		kCommandBehaviorNext,
		kCommandBehaviorLoop,
		kCommandBehaviorBlock,
		kCommandBehaviorBreak,
	};
private:
	ObjectAction();
	virtual ~ObjectAction();
public:
	CREATE_FUNC_PARAM4(ObjectAction, agtk::data::ObjectActionData *, objectActionData, agtk::Object *, object, bool, isCommon, int, commonActionSettingId);
	void setup(bool bClearFlag = true);
	void registActionLink(cocos2d::__Array* commonActionLinkList);
	int checkActionLinkCondition();
	void update(float dt);
	int getId();
	agtk::Scene *getScene();
	agtk::SceneLayer *getSceneLayer();
	agtk::data::ObjectData *getObjectData();

#ifdef USE_PREVIEW
	static int callObjectCreate(int objectId, int actionId, int layerId, cocos2d::Vec2 pos);//オブジェクトを生成(プレビュー用)
#endif
	static ObjectAction *createForScript();
// #AGTK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#else
	static int ObjectAction::execActionObjectCreateForScript(int objectId, int x, int y, int layerId);//オブジェクトを生成(2)
#endif
private:
	virtual bool init(agtk::data::ObjectActionData* objectActionData, agtk::Object *object, bool isCommon, int commonActionSettingId);
	bool checkInputCondition(agtk::data::ObjectActionLinkData *objectActionLinkData);
	int checkLinkCondition(agtk::data::ObjectActionLinkData *objectActionLinkData);
	int getReflectVariableValue(int objectId, int variableId, int qualifierId);
	void setVariableDataFromDatabase(agtk::data::DatabaseData *data, agtk::Object *object, agtk::data::ObjectCommandDatabaseReflectData *cmd, int columnIndex, int rowIndex);

	void reflectDatabaseValue(agtk::data::DatabaseData *data, agtk::data::DatabaseData::DatabaseType type, agtk::data::ObjectCommandDatabaseReflectData *cmd, int columnIndex, int rowIndex);
	void reflectDatabaseValueInVariable(agtk::data::DatabaseData *data, agtk::data::DatabaseData::DatabaseType type, agtk::data::ObjectCommandDatabaseReflectData *cmd, int columnIndex, int rowIndex);
	void setDatabaseValueInType(agtk::data::DatabaseData::DatabaseType type, agtk::Object * object, const char* value);
	void setResourceSet(agtk::Object * object, const char* resouceSetName);
	void setMotion(agtk::Object * object, const char* motionName);
public:
	bool checkLinkConditionWallTouched(agtk::data::ObjectActionLinkConditionData *linkConditionData);//壁判定に接触(0)
	bool checkLinkConditionNoWall(agtk::data::ObjectActionLinkConditionData *linkConditionData);//壁判定が無い(1)
	bool checkLinkConditionWallAhead(agtk::data::ObjectActionLinkConditionData *linkConditionData);//進んだ先で壁判定に接触(2)
	bool checkLinkConditionNoWallAhead(agtk::data::ObjectActionLinkConditionData *linkConditionData);//進んだ先で判定が無い(3)
	bool checkLinkConditionObjectWallTouched(agtk::data::ObjectActionLinkConditionData *linkConditionData);//他オブジェクトの壁判定に接触(4)
	bool checkLinkConditionAttackAreaTouched(agtk::data::ObjectActionLinkConditionData *linkConditionData);//攻撃判定に接触(5)
	bool checkLinkConditionAttackAreaNear(agtk::data::ObjectActionLinkConditionData *linkConditionData);//攻撃判定が近くにある(6)
	bool checkLinkConditionObjectNear(agtk::data::ObjectActionLinkConditionData *linkConditionData);//他オブジェクトが近くにある(7)
	bool checkLinkConditionObjectFacingEachOther(agtk::data::ObjectActionLinkConditionData *linkConditionData);//他のオブジェクトと向かいあっている(8)
	bool checkLinkConditionObjectFacing(agtk::data::ObjectActionLinkConditionData *linkConditionData);//他のオブジェクトの方を向いている(9)
	bool checkLinkConditionObjectFound(agtk::data::ObjectActionLinkConditionData *linkConditionData);////他のオブジェクトを発見した(10)
	bool checkLinkConditionObjectFacingDirection(agtk::data::ObjectActionLinkConditionData *linkConditionData);//他のオブジェクトが指定方向を向いている(11)
	bool checkLinkConditionHpZero(agtk::data::ObjectActionLinkConditionData *linkConditionData);//体力が０(12)
	bool checkLinkConditionCameraOutOfRange(agtk::data::ObjectActionLinkConditionData *linkConditionData);//カメラの範囲外にでた(13)
	bool checkLinkConditionLocked(agtk::data::ObjectActionLinkConditionData *linkConditionData);//ロックした/された(14)
	bool checkLinkConditionProbability(agtk::data::ObjectActionLinkConditionData *linkConditionData);//確率を使用(15)
	bool checkLinkConditionWaitTime(agtk::data::ObjectActionLinkConditionData *linkConditionData);//一定時間が経過(16)
	bool checkLinkConditionSwitchVariableChanged(agtk::data::ObjectActionLinkConditionData *linkConditionData);//スイッチ・変数が変化(17)
	bool checkLinkConditionAnimationFinished(agtk::data::ObjectActionLinkConditionData *linkConditionData);//モーションの表示が全て終わった(18)
	bool checkLinkConditionJumpTop(agtk::data::ObjectActionLinkConditionData *linkConditionData);//ジャンプが頂点になった(19)
	bool checkLinkConditionObjectActionChanged(agtk::data::ObjectActionLinkConditionData *linkConditionData);//オブジェクトのアクションが変化(20)
	bool checkLinkConditionSlopeTouched(agtk::data::ObjectActionLinkConditionData *linkConditionData);//坂に接触(21)
	bool checkLinkConditionBuriedInWallData(agtk::data::ObjectActionLinkConditionData *linkConditionData);//壁判定に埋まった(22)
	bool checkLinkConditionNoObjectWallTouched(agtk::data::ObjectActionLinkConditionData *linkConditionData);//他オブジェクトの壁判定に接触していない(23)
	bool checkLinkConditionScript(agtk::data::ObjectActionLinkConditionData *linkConditionData, agtk::data::ObjectActionLinkData *objectActionLinkData);//スクリプトを実行(して判定)(24)
	bool checkLinkConditionObjectHit(agtk::data::ObjectActionLinkConditionData *linkConditionData);//他オブジェクトの当たり判定に接触(25)
	bool checkLinkConditionCustom(agtk::data::ObjectActionLinkConditionData *linkConditionData, agtk::data::ObjectActionLinkData *objectActionLinkData);//カスタム(1000～)
private:
	void updateOtherExecAction();
	void logExecAction(agtk::data::ObjectCommandData *commandData);//実行アクションログ
public:
	void execActionTemplateMove(agtk::data::ObjectCommandData *commandData);//テンプレート移動の設定(0)
	bool execActionObjectLock(agtk::data::ObjectCommandData *commandData);//オブジェクトをロック(1)
	void execActionObjectCreate(agtk::data::ObjectCommandData *commandData);//オブジェクトを生成(2)
	void execActionObjectChange(agtk::data::ObjectCommandData *commandData);//オブジェクトを変更(3)
	void execActionObjectMove(agtk::data::ObjectCommandData *commandData);//オブジェクトを移動させる(4)
	bool execActionObjectPushPull(agtk::data::ObjectCommandData *commandData);//オブジェクトを押す・引く(5)
	void execActionLayerMove(agtk::data::ObjectCommandData *commandData);//レイヤーを移動(7)
	void execActionAttackSetting(agtk::data::ObjectCommandData *commandData);//攻撃の設定(8)
	void execActionBulletFire(agtk::data::ObjectCommandData *commandData);//弾を発射(9)
	void execActionDisappear(agtk::data::ObjectCommandData *commandData);//オブジェクトを消滅する(10)
	void execActionDisappearObjectRecover(agtk::data::ObjectCommandData *commandData);//消滅状態のオブジェクトを復活させる(11)
	void execActionDisable(agtk::data::ObjectCommandData *commandData);//オブジェクトを無効にする(12)
	void execActionDisableObjectEnable(agtk::data::ObjectCommandData *commandData);//無効状態のオブジェクトを有効にする(13)
	void execActionObjectFilterEffect(agtk::data::ObjectCommandData *commandData);//オブジェクトにフィルター効果を設定(17)
	void execActionObjectFilterEffectRemove(agtk::data::ObjectCommandData *commandData);//オブジェクトにフィルター効果を削除(18)
	void execActionSceneEffect(agtk::data::ObjectCommandData *commandData);//シーンに画面効果を設定(19)
	void execActionSceneEffectRemove(agtk::data::ObjectCommandData *commandData);//シーンに画面効果を削除(20)
	void execActionSceneGravityChange(agtk::data::ObjectCommandData *commandData);//重力効果を変更する(21)
	void execActionSceneRotateFlip(agtk::data::ObjectCommandData *commandData);//シーンを回転・反転(23)
	void execActionCameraAreaChange(agtk::data::ObjectCommandData *commandData);//カメラの表示領域を変更する(24)
	void execActionSoundPlay(agtk::data::ObjectCommandData *commandData);//音の再生(25)
	void execActionMessageShow(agtk::data::ObjectCommandData* commandData);//テキストを表示(26)
	void execActionScrollMessageShow(agtk::data::ObjectCommandData* commandData);//スクロールメッセージを設定(27)
	void execActionEffectShow(agtk::data::ObjectCommandData *commandData);//エフェクトを表示(28)
	void execActionMovieShow(agtk::data::ObjectCommandData *commandData);//動画を再生(29)
	void execActionImageShow(agtk::data::ObjectCommandData *commandData);//画像を表示(30)
	void execActionSwitchVariableChange(agtk::data::ObjectCommandData *commandData);//スイッチ・変数を変更(31)
	void execActionSwitchVariableReset(agtk::data::ObjectCommandData *commandData);//スイッチ・変数を初期値に戻す(32)
	void execActionGameSpeedChange(agtk::data::ObjectCommandData *commandData);//ゲームスピードを変更(33)
	void execActionWait(agtk::data::ObjectCommandData *commandData);//ウェイトを入れる(34)
	void execActionSceneTerminate(agtk::data::ObjectCommandData *commandData);//シーン終了(35)
	void execActionDirectionMove(agtk::data::ObjectCommandData *commandData);//移動方向を指定して移動(38)
	void execActionForthBackMoveTurn(agtk::data::ObjectCommandData *commandData);//前後移動と旋回(39)
	bool execActionActionExec(agtk::data::ObjectCommandData *commandData);//オブジェクトのアクションを実行(40)
	void execActionParticleShow(agtk::data::ObjectCommandData *commandData);//パーティクルの表示(41)
	void execActionTimer(agtk::data::ObjectCommandData *commandData);//タイマー機能(42)
	void execActionSceneShake(agtk::data::ObjectCommandData *commandData);//画面振動(43)
	void execActionEffectRemove(agtk::data::ObjectCommandData *commandData);//エフェクトを非表示(44)
	void execActionParticleRemove(agtk::data::ObjectCommandData *commandData);//パーティクルを非表示(45)
	void execActionLayerHide(agtk::data::ObjectCommandData *commandData);//レイヤーの表示OFF(46)
	void execActionLayerShow(agtk::data::ObjectCommandData *commandData);//レイヤーの表示ON(47)
	void execActionLayerDisable(agtk::data::ObjectCommandData *commandData);//レイヤーの動作OFF(48)
	void execActionLayerEnable(agtk::data::ObjectCommandData *commandData);//レイヤーの動作ON(49)
	int execActionScriptEvaluate(agtk::data::ObjectCommandData *commandData);//スクリプトを記述して実行(50)
	void execActionSoundStop(agtk::data::ObjectCommandData *commandData);//音の停止(51)
	void execActionMenuShow(agtk::data::ObjectCommandData *commandData);//メニュー画面を表示(52)
	void execActionMenuHide(agtk::data::ObjectCommandData *commandData);//メニュー画面を非表示(53)
	void execActionDisplayDirectionMove(agtk::data::ObjectCommandData *commandData);//表示方向と同じ方へ移動(54)
	void execActionFileLoad(agtk::data::ObjectCommandData *commandData);//ファイルをロード(55)
	void execActionSoundPositionRemember(agtk::data::ObjectCommandData *commandData);//音の再生位置を保存(56)
	void execActionObjectUnlock(agtk::data::ObjectCommandData *commandData);//ロックを解除(57)
	void execActionResourceSetChange(agtk::data::ObjectCommandData *commandData);//アニメーションの素材セットを変更(58)
	void execActionDatabaseReflect(agtk::data::ObjectCommandData *commandData);//データベースの値を反映(59)
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	int execActionCustom(agtk::data::ObjectCommandData *commandData);//カスタム(1000～)
public:
	void execActionObjectFilterEffect();
private:
	cocos2d::__Array *getTargetObjectByGroup(int group, int sceneLayerType = -1);//種類指定オブジェクト
	cocos2d::__Array *getTargetObjectById(int id, int sceneLayerType = -1);//指定オブジェクト
	cocos2d::__Array *getTargetObjectLocked(int sceneLayerType = -1);//ロックしたオブジェクト
	agtk::Object *getTargetObjectInstanceId(int instanceId, int sceneLayerType = -1);//単体インスタンスIDより得られたオブジェクト
	static cocos2d::__Dictionary *getObjCommandListByInstanceConfigurable(cocos2d::__Dictionary *baseObjCommandList, cocos2d::__Dictionary *instanceObjCommandList, agtk::Object *object);//インスタンスコマンド反映。
	cocos2d::__Dictionary *getObjCommandListByInstanceConfigurable(cocos2d::__Dictionary *baseObjCommandList, cocos2d::__Dictionary *instanceObjCommandList);//インスタンスコマンド反映。
	void setLayerVisible(int layerIdx, bool isExcept, bool isVisible);//シーンレイヤーの表示変更設定
	void setLayerActive(int layerIdx, bool isExcept, bool enable);//シーンレイヤーの動作変更設定
private:
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectData *, _objectData, ObjectData);
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectActionData *, _objectActionData, ObjectActionData);
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectActionLinkData *, _objectActionLinkData, ObjectActionLinkData);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _nextObjectActionLinkList, NextObjectActionLinkList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _objCommandList, ObjCommandList);
	CC_SYNTHESIZE(float, _duration, Duration);
	CC_SYNTHESIZE(bool, _ignored, Ignored);//アクション無効。
	CC_SYNTHESIZE(bool, _isCommon, IsCommon);//コモンアクションかのフラグ
	CC_SYNTHESIZE(int, _commonActionSettingId, CommonActionSettingId);
	CC_SYNTHESIZE(int, _preActionId, PreActionID);//遷移前のアクションID
	CC_SYNTHESIZE(bool, _hasHoldCommandList, HasHoldCommandList);//保留となったコマンドがあるフラグ
	CC_SYNTHESIZE(bool, _disableChangingFileSaveSwitchNextExecOtherAction, DisableChangingFileSaveSwitchNextExecOtherAction);// 次回「その他の実行アクション」実行時に「ファイルをセーブ」スイッチの変更を無効にする。1度「その他の実行アクション」が実行されれば有効に戻る。
	bool _actionObjectLockOnceMore;//「オブジェクトをロックする」でもう一度実行するようにするフラグ
	int _waitDuration300;//待ちフレーム
	agtk::Object *_object;
	CC_SYNTHESIZE(int, _dispDirection, DispDirection);//表示方向
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectCommandData *, _objectFilterEffectCommandData, ObjectFilterEffectCommandData);
	std::map<int, std::set<int>> _pushPullCommandIdMapEffectedObjectIdSet;
public:
	friend class Object;
	friend class ObjectMovement;
	friend class ObjectTemplateMove;
};

NS_AGTK_END

#endif	//__OBJECT_COMMAND_H__
