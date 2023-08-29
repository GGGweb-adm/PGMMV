#ifndef __AGTK_OBJECT_COMMAND_DATA_H__
#define	__AGTK_OBJECT_COMMAND_DATA_H__

#include "cocos2d.h"
#include "json/document.h"
#include "Lib/Macros.h"

USING_NS_CC;

NS_AGTK_BEGIN
NS_DATA_BEGIN

//-------------------------------------------------------------------------------------------------------------------
// json や js からのデータ設定処理群
struct DataSetter {
	std::function<void(const rapidjson::Value&)> json;
	std::function<void(void *, void *)> js;
	std::function<void()> dump;
};
// DataSetter をまとめて実行する者
struct DataSetterList {
	void execJson(const rapidjson::Value& json) {
		std::for_each(setter.begin(), setter.end(), [&json](DataSetter const & elem) { elem.json(json); });
	}
	void execJs(void *jsCx, void *jsObj) {
		std::for_each(setter.begin(), setter.end(), [&jsCx, &jsObj](DataSetter const & elem) { elem.js(jsCx, jsObj); });
	}
	void execDump() {
		std::for_each(setter.begin(), setter.end(), [](DataSetter const & elem) { elem.dump(); });
	}
public:
	std::vector<DataSetter> setter;
};

// 以下 DataSetter のメンバを簡単に定義するための簡易マクロ群
// bool 型のデータを json や js が取得するラムダ式
#define FUNC_BOOL(keyName,variableName)                          \
[&](const rapidjson::Value& json){                               \
	if (json.HasMember(keyName)) {                               \
		variableName = json[keyName].GetBool();                  \
	}										                     \
},                                                               \
[&](void * jsCx, void * jsObj){                                  \
	auto cx = static_cast<JSContext *>(jsCx);                    \
	JS::RootedValue v(cx);										 \
	JS::RootedObject rparams(cx, static_cast<JSObject *>(jsObj));\
	bool bValue;												 \
	if (getJsBoolean(cx, rparams, keyName, &bValue)) {			 \
		variableName = bValue;									 \
	}															 \
},																 \
[&](){                                                           \
	CCLOG(#variableName":%d", variableName); 	                 \
}
// int 型のデータを json や js が取得するラムダ式
#define FUNC_INT(keyName,variableName)                          \
[&](const rapidjson::Value& json){                               \
	if (json.HasMember(keyName)) {                               \
		variableName = json[keyName].GetInt();                  \
	}										                     \
},                                                               \
[&](void * jsCx, void * jsObj){                                  \
	auto cx = static_cast<JSContext *>(jsCx);                    \
	JS::RootedValue v(cx);										 \
	JS::RootedObject rparams(cx, static_cast<JSObject *>(jsObj));\
	int iValue;												 \
	if (getJsInt32(cx, rparams, keyName, &iValue)) {			 \
		variableName = iValue;									 \
	}															 \
},																 \
[&](){                                                           \
	CCLOG(#variableName":%d", variableName); 	                 \
}

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API FilterEffect : public cocos2d::Ref
{
public:
	enum EnumEffectType {
		kEffectNoise,//ノイズ
		kEffectMosaic,//モザイク
		kEffectMonochrome,//モノクロ
		kEffectSepia,//セピア
		kEffectNegaPosiReverse,//ネガ反転
		kEffectDefocus,//ぼかし
		kEffectChromaticAberration,//色収差
		kEffectDarkness,//暗闇
		kEffectDispImage,//画像表示
		kEffectFillColor,//指定色で塗る
		kEffectTransparency,//透過
		kEffectBlink,//点滅
		kEffectMax
	};
	enum EnumPlacementType {
		kPlacementCenter,//中央
		kPlacementMagnify,//拡大
		kPlacementTiling,//タイル
		kPlacementKeepRatio,//比率を維持して拡大
		kPlacementObjectCenter,//このオブジェクトの中心
		kPlacementMax
	};
private:
	FilterEffect();
	virtual ~FilterEffect();
public:
	CREATE_FUNC_PARAM(FilterEffect, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(FilterEffect, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(EnumEffectType, _effectType, EffectType);//エフェクトタイプ
	CC_SYNTHESIZE(int, _noise, Noise);//ノイズ
	CC_SYNTHESIZE(int, _mosaic, Mosaic);//モザイク
	CC_SYNTHESIZE(int, _monochrome, Monochrome);//モノクロ
	CC_SYNTHESIZE(int, _sepia, Sepia);//セピア
	CC_SYNTHESIZE(int, _negaPosiReverse, NegaPosiReverse);//ネガ反転
	CC_SYNTHESIZE(int, _defocus, Defocus);//ぼかし
	CC_SYNTHESIZE(int, _chromaticAberration, ChromaticAberration);//色収差
	CC_SYNTHESIZE(int, _darkness, Darkness);//暗闇
	CC_SYNTHESIZE(int, _transparency, Transparency);//透過
	CC_SYNTHESIZE(int, _blinkInterval300, BlinkInterval300);//点滅間隔
	CC_SYNTHESIZE(int, _imageId, ImageId);//画像ID
	CC_SYNTHESIZE(int, _imageTransparency, ImageTransparency);//透過性（アルファ）
	CC_SYNTHESIZE(EnumPlacementType, _imagePlacement, ImagePlacement);//配置方法
	CC_SYNTHESIZE(int, _fillA, FillA);//指定色で塗る（アルファ）
	CC_SYNTHESIZE(int, _fillR, FillR);//指定色で塗る（R値）
	CC_SYNTHESIZE(int, _fillG, FillG);//指定色で塗る（G値）
	CC_SYNTHESIZE(int, _fillB, FillB);//指定食で塗る（B値）
	CC_SYNTHESIZE(int, _duration300, Duration300);//完了までの時間
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandData : public cocos2d::Ref
{
public:
	enum EnumTouchedObjectType {
		kObjectByGroup,
		kObjectById,
		kObjectNone,
		kObjectMax
	};
	enum {
		kUnsetObject = -1,
		kSelfObject = -2,
		kOtherThanSelfObject = -3,
		kChildObject = -4,
		kLockedObject = -5,
		kTouchedObject = -6,
		kParentObject = -7,
	};
	enum {
		kAccordingToMoveDirection = -2,
	};
	enum EnumQualifierType {
		kQualifierSingle = -1,
		kQualifierWhole = -2,
	};
	enum EnumObjectTypeByType {
		kObjectTypeAll=-1,
		kObjectTypePlayer,
		kObjectTypeEnemy,
		kObjectTypeMax
	};
	enum EnumObjectGroup {
		kObjectGroupAll = -1,
		kObjectGroupPlayer,
		kObjectGroupEnemy,
		kObjectGroupAllOffset = 1,
	};
	enum EnumTileGroup {
		kTileGroupDefault = 0,
	};
	enum EnumCompareOperatorType {
		kOperatorLess,
		kOperatorLessEqual,
		kOperatorEqual,
		kOperatorGreaterEqual,
		kOperatorGreater,
		kOperatorNotEqual,
		kOperatorMax
	};
	enum EnumVariableAssignOperatorType {
		kVariableAssignOperatorSet,
		kVariableAssignOperatorAdd,
		kVariableAssignOperatorSub,
		kVariableAssignOperatorMul,
		kVariableAssignOperatorDiv,
		kVariableAssignOperatorMod,
		kVariableAssignOperatorMax
	};
	enum EnumPriorityType {
		kPriorityBackground,
		kPriorityMostFront,
		kPriorityMostFrontWithMenu,
		kPriorityMax
	};
public:
	enum EnumObjCommandType {
		kTemplateMove,//テンプレート移動の設定(0)
		kObjectLock,//オブジェクトをロック(1)
		kObjectCreate,//オブジェクトを生成(2)
		kObjectChange,//オブジェクトを変更(3)
		kObjectMove,//オブジェクトを移動(4)
		kObjectPushPull,//オブジェクトを押す・引く(5)
		kParentChildChange,
		kLayerMove,//レイヤーを移動(7)
		kAttackSetting,//攻撃の設定(8)
		kBulletFire,//弾を発射(9)
		kDisappear,//オブジェクトを消滅する(10)
		kDisappearObjectRecover,
		kDisable,//オブジェクトを無効にする(12)
		kDisableObjectEnable,//無効状態のオブジェクトを有効にする(13)
		kItemGive,	//Reserved
		kItemShow,	//Reserved
		kMenuPartShow,	//Reserved
		kObjectFilterEffect,//オブジェクトにフィルター効果を設定(17)
		kObjectFilterEffectRemove,//オブジェクトにフィルター効果を削除(18)
		kSceneEffect,//シーンに画面効果を設定(19)
		kSceneEffectRemove,//シーンに画面効果を削除(20)
		kSceneGravityChange,//重力効果を変更する(21)
		kSceneWaterChange,//Reserved
		kSceneRotateFlip,//シーンを回転・反転(23)
		kCameraAreaChange,//カメラの表示領域を変更する(24)
		kSoundPlay,//音の再生(25)
		kMessageShow,
		kScrollMessageShow,
		kEffectShow,//エフェクトの表示(28)
		kMovieShow,//動画を再生(29)
		kImageShow,//画像を表示(30)
		kSwitchVariableChange,//スイッチ・変数を変更(31)
		kSwitchVariableReset,//スイッチ・変数を初期値に戻す(32)
		kGameSpeedChange,//ゲームスピード変更(33)
		kWait,//ウェイトを入れる(34)
		kSceneTerminate,//シーン終了(35)
		kSaveScreenOpen,	//Reserved
		kQuickSave,	//Reserved
		kDirectionMove,//移動方向を指定して移動(38)
		kForthBackMoveTurn,//前後移動と旋回(39)
		kActionExec,//オブジェクトのアクションを実行(40)
		kParticleShow,//パーティクルを表示(41)
		kTimer,//タイマー機能(42)
		kSceneShake,//画面振動(43)
		kEffectRemove,//エフェクトを非表示(44)
		kParticleRemove,//パーティクルを非表示(45)
		kLayerHide,//レイヤーの表示をOFF(46)
		kLayerShow,//レイヤーの表示をON(47)
		kLayerDisable,//レイヤーの動作をOFF(48)
		kLayerEnable,//レイヤーの動作をON(49)
		kScriptEvaluate,//スクリプトを記述して実行(50)
		kSoundStop,//音の停止(51)
		kMenuShow,//メニュー画面を表示(52)
		kMenuHide,//メニュー画面を非表示(53)
		kDisplayDirectionMove,//表示方向と同じ方へ移動(54)
		kFileLoad,//ファイルをロード(55)
		kSoundPositionRemember,//音の再生位置を保存(56)
		kObjectUnlock,//ロックを解除(57)
		kResourceSetChange,//アニメーションの素材セットを変更(58)
		kDatabaseReflect,//データベースの値反映(59)
		kNXVibrateController,// #AGTK-NX Nintendo Switch 振動コントローラーアクション(60)
		kNXShowControllerApplet,// #AGTK-NX Nintendo Switch コントローラー接続設定表示アクション(61)
		kMax,
		kCustomHead = 1000,//カスタム(1000～)
	};
	enum {
		kPluginCustomMax = 100,
	};
protected:
	ObjectCommandData();
	virtual ~ObjectCommandData(){}
public:
	static ObjectCommandData *create(const rapidjson::Value& json);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
	static const char *getCommandTypeName(EnumObjCommandType type);
protected:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE(EnumObjCommandType, _commandType, CommandType);
	CC_SYNTHESIZE(bool, _ignored, Ignored);
	CC_SYNTHESIZE(bool, _instanceConfigurable, InstanceConfigurable);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandTemplateMoveData : public ObjectCommandData
{
public:
	enum EnumMoveType
	{
		kMoveHorizontal,//左右移動
		kMoveVertical,//上下移動
		kMoveBound,//バウンド
		kMoveRandom,//ランダム移動（移動と停止を繰り返す）
		kMoveNearObject,//近くのオブジェクトへ移動
		kMoveApartNearObject,//近くのオブジェクトから離れる
		kMoveStop,//停止
		kMoveMax
	};
public:
	ObjectCommandTemplateMoveData();
	virtual ~ObjectCommandTemplateMoveData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandTemplateMoveData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandTemplateMoveData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(EnumMoveType, _moveType, MoveType);//移動タイプ
	//左右移動
	CC_SYNTHESIZE(bool, _horizontalMoveStartRight, HorizontalMoveStartRight);//右移動から開始
	CC_SYNTHESIZE(int, _horizontalMoveDuration300, HorizontalMoveDuration300);//移動時間
	CC_SYNTHESIZE(bool, _horizontalInfinite, HorizontalInfinite);//無限フラグ
	//上下移動
	CC_SYNTHESIZE(bool, _verticalMoveStartDown, VerticalMoveStartDown);//下移動から開始
	CC_SYNTHESIZE(int, _verticalMoveDuration300, VerticalMoveDuration300);//移動時間
	CC_SYNTHESIZE(bool, _verticalInfinite, VerticalInfinite);//無限フラグ
	//ランダム移動
	CC_SYNTHESIZE(int, _randomMoveDuration300, RandomMoveDuration300);//移動時間
	CC_SYNTHESIZE(int, _randomMoveStop300, RandomMoveStop300);//停止時間
	//近くのオブジェクトへ移動
	CC_SYNTHESIZE(int, _nearObjectGroup, NearObjectGroup);//近くのオブジェクトグループ
	CC_SYNTHESIZE(bool, _nearObjectLockedObjectPrior, NearObjectLockedObjectPrior);
	//近くのオブジェクトから離れる
	CC_SYNTHESIZE(int, _apartNearObjectGroup, ApartNearObjectGroup);//近くのオブジェクトグループ
	CC_SYNTHESIZE(bool, _apartNearObjectLockedObjectPrior, ApartNearObjectLockedObjectPrior);
	CC_SYNTHESIZE(bool, _fallFromStep, FallFromStep);//段差から落ちない
	CC_SYNTHESIZE(bool, _ignoreOtherObjectWallArea, IgnoreOtherObjectWallArea);//他オブジェクトの壁判定を無視する
	CC_SYNTHESIZE(bool, _ignoreWall, IgnoreWall);//壁判定を無視する
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandObjectLockData : public ObjectCommandData
{
public:
	enum EnumSwitchCondition {
		kSwitchConditionOn,
		kSwitchConditionOff,
		kSwitchConditionOnFromOff,
		kSwitchConditionOffFromOn,
		kSwitchConditionMax
	};
	enum EnumVariableCompareValueType {
		kVariableCompareValue,//定数
		kVariableCompareVariable,//他の変数
		kVariableCompareNaN,//数値以外（非数）
		kVariableCompareValueMax
	};
	enum EnumUseSwitchVariable {
		kUseSwitch,
		kUseVariable,
		kUseNone,
		kUseMax,
	};
private:
	ObjectCommandObjectLockData() : ObjectCommandData() {};
	virtual ~ObjectCommandObjectLockData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandObjectLockData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandObjectLockData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(bool, _lockTouchedObject, LockTouchedObject);
	CC_SYNTHESIZE(bool, _lockViewportLightObject, LockViewportLightObject);
	CC_SYNTHESIZE(bool, _lockViewportLightOfAcrossLayerObject, LockViewportLightOfAcrossLayerObject);
	CC_SYNTHESIZE(bool, _lockObjectOnScreen, LockObjectOnScreen);
	CC_SYNTHESIZE(bool, _lockObjectTouchedByThisObjectAttack, LockObjectTouchedByThisObjectAttack);
	CC_SYNTHESIZE(int, _objectType, ObjectType);// オブジェクトの指定方法
	CC_SYNTHESIZE(int, _objectGroup, ObjectGroup);
	CC_SYNTHESIZE(int, _objectId, ObjectId);
	CC_SYNTHESIZE(EnumUseSwitchVariable, _useType, UseType);
	CC_SYNTHESIZE(int, _switchId, SwitchId);
	CC_SYNTHESIZE(EnumSwitchCondition, _switchCondition, SwitchCondition);
	CC_SYNTHESIZE(int, _variableId, VariableId);
	CC_SYNTHESIZE(int, _compareVariableOperator, CompareVariableOperator);
	CC_SYNTHESIZE(EnumVariableCompareValueType, _compareValueType, CompareValueType);
	CC_SYNTHESIZE(double, _compareValue, CompareValue);
	CC_SYNTHESIZE(int, _compareVariableObjectId, CompareVariableObjectId);
	CC_SYNTHESIZE(int, _compareVariableId, CompareVariableId);
	CC_SYNTHESIZE(EnumQualifierType, _compareVariableQualifierId, CompareVariableQualifierId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandObjectCreateData : public ObjectCommandData
{
public:
	enum EnumCreatePosition {
		kPositionCenter,
		kPositionLockObjectCenter,
		kPositionMax,
	};
private:
	ObjectCommandObjectCreateData() : ObjectCommandData(), _actionId(-1) {};
	virtual ~ObjectCommandObjectCreateData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandObjectCreateData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandObjectCreateData, void *, jsCx, void *, jsObj);
	static ObjectCommandObjectCreateData *createForScript();
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _objectId, ObjectId);
	CC_SYNTHESIZE(int, _actionId, ActionId);
	CC_SYNTHESIZE(EnumCreatePosition, _createPosition, CreatePosition);
	CC_SYNTHESIZE(bool, _useConnect, UseConnect);
	CC_SYNTHESIZE(int, _connectId, ConnectId);
	CC_SYNTHESIZE(int, _adjustX, AdjustX);
	CC_SYNTHESIZE(int, _adjustY, AdjustY);
	CC_SYNTHESIZE(double, _probability, Probability);
	CC_SYNTHESIZE(bool, _childObject, ChildObject);
	CC_SYNTHESIZE(bool, _useRotation, UseRotation);
	CC_SYNTHESIZE(bool, _lowerPriority, LowerPriority);
	CC_SYNTHESIZE(bool, _gridMagnet, GridMagnet);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandObjectChangeData : public ObjectCommandData
{
public:
	enum EnumCreatePosition {
		kPositionCenter,
		kPositionLockObjectCenter,
		kPositionMax,
	};
private:
	ObjectCommandObjectChangeData() : ObjectCommandData(), _actionId(-1) {};
	virtual ~ObjectCommandObjectChangeData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandObjectChangeData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandObjectChangeData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _objectId, ObjectId);// 変更後のオブジェクトID
	CC_SYNTHESIZE(int, _actionId, ActionId);
	CC_SYNTHESIZE(int, _createPosition, CreatePosition);// 変更後のオブジェクト生成位置
	CC_SYNTHESIZE(bool, _useConnect, UseConnect);// 接続点を使用
	CC_SYNTHESIZE(int, _connectId, ConnectId);// 接続するオブジェクトID
	CC_SYNTHESIZE(int, _adjustX, AdjustX);// 位置を調整x
	CC_SYNTHESIZE(int, _adjustY, AdjustY);// 位置を調整y
	CC_SYNTHESIZE(double, _probability, Probability);// オブジェクトの変更の確率
	CC_SYNTHESIZE(bool, _takeOverSwitches, TakeOverSwitches);// オブジェクトの変数を引き継ぐ
	CC_SYNTHESIZE(bool, _takeOverVariables, TakeOverVariables);// オブジェクトのスイッチを引き継ぐ
	CC_SYNTHESIZE(bool, _takeOverParentChild, TakeOverParentChild);// 親子関係を引き継ぐ
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandObjectMoveData : public ObjectCommandData
{
public:
	enum EnumMoveType {
		kMoveWithDirection,//方向を指定して移動
		kMoveToPosition,//座標を指定して移動
		kMoveToObjectCenter,//指定したオブジェクトの中心点に移動
		kMoveToObjectOrigin,//指定したオブジェクトの原点に移動
		kMoveToObjectConnectionPoint,//指定したオブジェクトの接続点に移動
		kMoveMax
	};
	enum EnumTargettingType {
		kTargettingByGroup,//オブジェクトの種類で指定
		kTargettingById,//オブジェクトで指定
		kTargettingTouched,//このオブジェクトに触れたオブジェクト
		kTargettingLocked,//このオブジェクトがロックしたオブジェクト
		kTargettingMax
	};
private:
	ObjectCommandObjectMoveData() : ObjectCommandData(), _moveInDisplayCoordinates(false), _followCameraMoving(false), _connectId(-1){};
	virtual ~ObjectCommandObjectMoveData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandObjectMoveData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandObjectMoveData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	//▼移動方法を指定　※シーン配置後も選択が可能です。
	CC_SYNTHESIZE(EnumMoveType, _moveType, MoveType);//移動方法（方向を指定して移動:0, 座標を指定して移動:1, 指定したオブジェクトの中心点に移動:2）
	//方向を指定して移動
	CC_SYNTHESIZE(double, _angle, Angle);//方向を指定して移動（角度）
	CC_SYNTHESIZE(int, _moveDistance, MoveDistance);//移動距離を指定
	//座標を指定して移動
	CC_SYNTHESIZE(int, _posX, PosX);//座標を指定して移動（X座標）
	CC_SYNTHESIZE(int, _posY, PosY);//座標を指定して移動（Y座標）
	CC_SYNTHESIZE(bool, _moveInDisplayCoordinates, MoveInDisplayCoordinates);//表示エリアに対しての座標に移動
	CC_SYNTHESIZE(bool, _followCameraMoving, FollowCameraMoving);//スクロールの影響を受ける
	//指定したオブジェクトへ移動
	CC_SYNTHESIZE(int, _centerObjectId, CenterObjectId);//オブジェクトID
	CC_SYNTHESIZE(EnumQualifierType, _centerQualifierId, CenterQualifierId);//単体/全体
	CC_SYNTHESIZE(int, _centerAdjustX, CenterAdjustX);//位置調整（X座標）
	CC_SYNTHESIZE(int, _centerAdjustY, CenterAdjustY);//位置調整（Y座標）
	CC_SYNTHESIZE(int, _connectId, ConnectId);//接続点
	//▼移動スピードを指定
	CC_SYNTHESIZE(bool, _useObjectParameter, UseObjectParameter);//対象オブジェクトの基本移動パラメータを使用:true,指定位置までの移動が完了する時間を設定:false
	CC_SYNTHESIZE(double, _changeMoveSpeed, ChangeMoveSpeed);//移動速度を変更（％）
	CC_SYNTHESIZE(int, _moveDuration300, MoveDuration300);//指定位置までの移動が完了する時間（秒）
	//▼移動対象のオブジェクトを指定
	CC_SYNTHESIZE(EnumTargettingType, _targettingType, TargettingType);//移動対象のオブジェクトを指定タイプ（オブジェクトの種類で指定:0,オブジェクトで指定:1,このオブジェクトに触れたオブジェクト:2,このオブジェクトがロックしたオブジェクト:3）
	//オブジェクトの種類で指定
	CC_SYNTHESIZE(int, _targetObjectGroup, TargetObjectGroup);//オブジェクトグループ
	//オブジェクトで指定
	CC_SYNTHESIZE(int, _targetObjectId, TargetObjectId);//オブジェクトで指定ID
	CC_SYNTHESIZE(EnumQualifierType, _targetQualifierId, TargetQualifierId);//単体/全体
	//このオブジェクトに触れたオブジェクト
	CC_SYNTHESIZE(int, _excludeObjectGroupBit, ExcludeObjectGroupBit);// 対象外のオブジェクトグループ
	//オプション
	CC_SYNTHESIZE(bool, _fitDispDirToMoveDir, FitDispDirToMoveDir);//表示方向を移動方向に合わせる
	CC_SYNTHESIZE(bool, _dispWhileMoving, DispWhileMoving);//移動中オブジェクトを表示しない（表示:true,非表示:false）
	CC_SYNTHESIZE(bool, _stopActionWhileMoving, StopActionWhileMoving);//移動中オブジェクトのアクションを停止。
	CC_SYNTHESIZE(bool, _stopAnimWhileMoving, StopAnimWhileMoving);//移動中オブジェクトのアニメーションを停止。
	CC_SYNTHESIZE(bool, _finalGridMagnet, FinalGridMagnet);//移動完了後のオブジェクトがグリッドに吸着する
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandObjectPushPullData : public ObjectCommandData
{
public:
	enum EnumDirection {
		kDirectionAngle,//角度を指定
		kDirectionObjectDisp,//このオブジェクトの表示方向
		kDirectionObjectConnect,//このオブジェクトの接続点
		kDirectionObject,//このオブジェクトの方向
		kDirectionMax
	};
	enum EnumEffectDirection {
		kEffectDirectionAngle,//角度指定
		kEffectDirectionObjectDisp,//このオブジェクトの表示方向
		kEffectDirectionObjectConnect,//このオブジェクトの接続点
		kEffectDirectionMax
	};
	enum EnumTargettingType {
		kTargettingByGroup,//オブジェクトの種類で指定
		kTargettingById,//オブジェクトで指定
		kTargettingTouched,//このオブジェクトに触れたオブジェクト
		kTargettingLocked,//このオブジェクトがロックしたオブジェクト
		kTargettingMax
	};
private:
	ObjectCommandObjectPushPullData() : ObjectCommandData() {};
	virtual ~ObjectCommandObjectPushPullData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandObjectPushPullData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandObjectPushPullData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	//効果の方向と範囲 -----------------------------------------------------------------------------
	CC_SYNTHESIZE(EnumDirection, _directionType, DirectionType);//方向タイプ（角度を指定、このオブジェクトの表示方向、このオブジェクトの接続点、このオブジェクトの方向へ）
	CC_SYNTHESIZE(double, _angle, Angle);//[角度を指定] 角度
	CC_SYNTHESIZE(bool, _rectangle, Rectangle);//矩形(true)、円形(false)
	CC_SYNTHESIZE(int, _rectangleDistance, RectangleDistance);//矩形距離
	CC_SYNTHESIZE(int, _rectangleHeight, RectangleHeight);//矩形高さ
	CC_SYNTHESIZE(int, _circleDistance, CircleDistance);//円距離
	CC_SYNTHESIZE(int, _arcAngle, ArcAngle);//放射角
	CC_SYNTHESIZE(bool, _effectRangeBaseConnect, EffectRangeBaseConnect);//効果範囲のベースを接続点に設定有無
	CC_SYNTHESIZE(int, _effectRangeBaseConnectId, EffectRangeBaseConnectId);//効果範囲のベースの接続点ID
	//効果設定 -------------------------------------------------------------------------------------
	CC_SYNTHESIZE(EnumEffectDirection, _effectDirectionType, EffectDirectionType);//効果の方向（角度を指定:0, このオブジェクトの表示方向:1,このオブジェクトの接続点:2）
	CC_SYNTHESIZE(double, _effectDirection, EffectDirection);//角度指定
	CC_SYNTHESIZE(int, _connectId, ConnectId);//[このオブジェクトの接続点] ID
	CC_SYNTHESIZE(int, _effectValue, EffectValue);//効果の強さ
	CC_SYNTHESIZE(bool, _distanceEffect, DistanceEffect);//このオブジェクトとの距離によって効果を増減
	CC_SYNTHESIZE(double, _nearValue, NearValue);//近い時の係数（％）
	CC_SYNTHESIZE(double, _farValue, FarValue);//遠い時の係数（％）
	CC_SYNTHESIZE(bool, _oneTimeEffect, OneTimeEffect);//効果は対象となった時一度だけ受ける
	//対象のオブジェクトを指定 ----------------------------------------------------------------------
	CC_SYNTHESIZE(EnumTargettingType, _targettingType, TargettingType);//対象オブジェクト指定タイプ（オブジェクトの種類で指定、オブジェクトで指定、このオブジェクトに触れたオブジェクト、このオブジェクトがロックしたオブジェクト）
	CC_SYNTHESIZE(int, _targetObjectGroup, TargetObjectGroup);//オブジェクトのグループで指定 all = -1
	CC_SYNTHESIZE(int, _targetObjectId, TargetObjectId);//[オブジェクトで指定] ID
	CC_SYNTHESIZE(EnumQualifierType, _targetQualifierId, TargetQualifierId);//単体/全体
	CC_SYNTHESIZE(int, _excludeObjectGroupBit, ExcludeObjectGroupBit);//[このオブジェクトに触れたオブジェクト] 対象外とするグループ
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandParentChildChangeData : public ObjectCommandData
{
private:
	ObjectCommandParentChildChangeData() : ObjectCommandData() {};
	virtual ~ObjectCommandParentChildChangeData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandParentChildChangeData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(int, _relation, Relation);
	CC_SYNTHESIZE(int, _targettingType, TargettingType);
	CC_SYNTHESIZE(int, _targetObjectId, TargetObjectId);
	CC_SYNTHESIZE(bool, _ignorePlayerObject, IgnorePlayerObject);
	CC_SYNTHESIZE(bool, _ignoreEnemyObject, IgnoreEnemyObject);
	CC_SYNTHESIZE(bool, _changeTimingPosition, ChangeTimingPosition);
	CC_SYNTHESIZE(int, _connectId, ConnectId);
	CC_SYNTHESIZE(int, _adjustX, AdjustX);
	CC_SYNTHESIZE(int, _adjustY, AdjustY);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandLayerMoveData : public ObjectCommandData
{
private:
	ObjectCommandLayerMoveData() : ObjectCommandData() {};
	virtual ~ObjectCommandLayerMoveData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandLayerMoveData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandLayerMoveData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _layerIndex, LayerIndex);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandAttackSettingData : public ObjectCommandData
{
public:
	enum AttributeType {
		kNone,//無し
		kPreset,//プリセット
		kConstant//定数
	};
private:
	ObjectCommandAttackSettingData() : ObjectCommandData() {};
	virtual ~ObjectCommandAttackSettingData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandAttackSettingData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandAttackSettingData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(double, _attackChange, AttackChange);
	CC_SYNTHESIZE(bool, _hitObjectFlag, HitObjectFlag);// 攻撃判定が当たるオブジェクトグループを設定
	CC_SYNTHESIZE(int, _objectGroupBit, ObjectGroupBit);
	CC_SYNTHESIZE(bool, _hitTileFlag, HitTileFlag);// 攻撃判定が当たるタイルグループを設定
	CC_SYNTHESIZE(int, _tileGroupBit, TileGroupBit);
	CC_SYNTHESIZE(int, _attributeType, AttributeType);
	CC_SYNTHESIZE(int, _attributePresetId, AttributePresetId);
	CC_SYNTHESIZE(int, _attributeValue, AttributeValue);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandBulletFireData : public ObjectCommandData
{
private:
	ObjectCommandBulletFireData() : ObjectCommandData() {};
	virtual ~ObjectCommandBulletFireData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandBulletFireData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandBulletFireData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _bulletId, BulletId);
	CC_SYNTHESIZE(int, _connectId, ConnectId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandDisappearData : public ObjectCommandData
{
private:
	ObjectCommandDisappearData() : ObjectCommandData() {};
	virtual ~ObjectCommandDisappearData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandDisappearData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandDisappearData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandDisappearObjectRecoverData : public ObjectCommandData
{
private:
	ObjectCommandDisappearObjectRecoverData() : ObjectCommandData(), _objectId(-1) {};
	virtual ~ObjectCommandDisappearObjectRecoverData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandDisappearObjectRecoverData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandDisappearObjectRecoverData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _objectId, ObjectId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandDisableData : public ObjectCommandData
{
private:
	ObjectCommandDisableData() : ObjectCommandData() {};
	virtual ~ObjectCommandDisableData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandDisableData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandDisableData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandDisableObjectEnableData : public ObjectCommandData
{
private:
	ObjectCommandDisableObjectEnableData() : ObjectCommandData(), _objectId(-1) {};
	virtual ~ObjectCommandDisableObjectEnableData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandDisableObjectEnableData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandDisableObjectEnableData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _objectId, ObjectId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandObjectFilterEffectData : public ObjectCommandData
{
private:
	ObjectCommandObjectFilterEffectData();
	virtual ~ObjectCommandObjectFilterEffectData();
public:
	CREATE_FUNC_PARAM(ObjectCommandObjectFilterEffectData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandObjectFilterEffectData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE_RETAIN(FilterEffect *, _filterEffect, FilterEffect);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandObjectFilterEffectRemoveData : public ObjectCommandData
{
private:
	ObjectCommandObjectFilterEffectRemoveData() : ObjectCommandData() {};
	virtual ~ObjectCommandObjectFilterEffectRemoveData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandObjectFilterEffectRemoveData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandObjectFilterEffectRemoveData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _removeBit, RemoveBit);
	CC_SYNTHESIZE(int, _duration300, Duration300);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandSceneEffectData : public ObjectCommandData
{
private:
	ObjectCommandSceneEffectData();
	virtual ~ObjectCommandSceneEffectData();
public:
	CREATE_FUNC_PARAM(ObjectCommandSceneEffectData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandSceneEffectData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _layerIndex, LayerIndex);
	CC_SYNTHESIZE_RETAIN(FilterEffect *, _filterEffect, FilterEffect);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandSceneEffectRemoveData : public ObjectCommandData
{
private:
	ObjectCommandSceneEffectRemoveData() : ObjectCommandData() {};
	virtual ~ObjectCommandSceneEffectRemoveData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandSceneEffectRemoveData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandSceneEffectRemoveData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _layerIndex, LayerIndex);
	CC_SYNTHESIZE(int, _removeBit, RemoveBit);
	CC_SYNTHESIZE(int, _duration300, Duration300);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandSceneGravityChangeData : public ObjectCommandData
{
private:
	ObjectCommandSceneGravityChangeData() : ObjectCommandData() {};
	virtual ~ObjectCommandSceneGravityChangeData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandSceneGravityChangeData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandSceneGravityChangeData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(double, _gravity, Gravity);
	CC_SYNTHESIZE(double, _direction, Direction);
	CC_SYNTHESIZE(int, _duration300, Duration300);
	CC_SYNTHESIZE(bool, _durationUnlimited, DurationUnlimited);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandSceneRotateFlipData : public ObjectCommandData
{
public:
	enum EnumType {
		kTypeReset,//リセット
		kTypeRotationFlip,//回転・反転
		kTypeMax
	};
	enum EnumPosition {//todo: EnumPositionの定義を削除。
		kPositionCenter,//このオブジェクトを中心とする
		kPositionLockObjectCenter,//このオブジェクトがロックしたオブジェクトを中心とする
		kPositionScreenCenter,//画面の中心
		kPositionMax,
	};
private:
	ObjectCommandSceneRotateFlipData() : ObjectCommandData(), _type(kTypeRotationFlip), _absoluteRotation(false) {};
	virtual ~ObjectCommandSceneRotateFlipData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandSceneRotateFlipData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandSceneRotateFlipData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	//-----------------------------------------------------------------------------------------------------------
	//シーンを回転・反転
	CC_SYNTHESIZE(int, _type, Type);//リセットか回転・反転
	CC_SYNTHESIZE(bool, _rotationFlag, RotationFlag);//回転チェック
	CC_SYNTHESIZE(double, _rotation, Rotation);//回転値
	CC_SYNTHESIZE(bool, _absoluteRotation, AbsoluteRotation);//絶対角度
	CC_SYNTHESIZE(bool, _flipX, FlipX);//左右フリップ
	CC_SYNTHESIZE(bool, _flipY, FlipY);//上下フリップ
	CC_SYNTHESIZE(int, _duration300, Duration300);//完了までの時間
	//-----------------------------------------------------------------------------------------------------------
	//回転・反転の対象外を指定
	//初期実装から除外
	//CC_SYNTHESIZE(bool, _playerObject, PlayerObject);//プレイヤーオブジェクト
	//CC_SYNTHESIZE(bool, _enemyObject, EnemyObject);//エネミーオブジェクト
	//CC_SYNTHESIZE(bool, _gravityDirection, GravityDirection);//重力方向
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandCameraAreaChangeData : public ObjectCommandData
{
public:
	enum EnumPosition {
		kPositionCenter,//このオブジェクトを中心とする
		kPositionLockObjectCenter,//このオブジェクトがロックしたオブジェクトを中心とする
		kPositionMax,
	};
private:
	ObjectCommandCameraAreaChangeData() : ObjectCommandData() {};
	virtual ~ObjectCommandCameraAreaChangeData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandCameraAreaChangeData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandCameraAreaChangeData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(double, _xRate, XRate);//表示領域の比率X
	CC_SYNTHESIZE(double, _yRate, YRate);//表示領域の比率Y
	CC_SYNTHESIZE(int, _duration300, Duration300);//完了までの時間
	CC_SYNTHESIZE(EnumPosition, _positionType, PositionType);//タイプ
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandSoundPlayData : public ObjectCommandData
{
public:
	enum EnumSound {
		kSoundSe,
		kSoundVoice,
		kSoundBgm,
		kSoundMax
	};
private:
	ObjectCommandSoundPlayData() : ObjectCommandData(), _soundType(kSoundSe), _seId(-1), _voiceId(-1), _bgmId(-1), _loop(false), _fadein(false), _duration300(300), _volume(100), _pan(0), _pitch(0) {};
	virtual ~ObjectCommandSoundPlayData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandSoundPlayData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandSoundPlayData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(EnumSound, _soundType, SoundType);
	CC_SYNTHESIZE(int, _seId, SeId);
	CC_SYNTHESIZE(int, _voiceId, VoiceId);
	CC_SYNTHESIZE(int, _bgmId, BgmId);
	CC_SYNTHESIZE(bool, _loop, Loop);
	CC_SYNTHESIZE(bool, _fadein, Fadein);
	CC_SYNTHESIZE(int, _duration300, Duration300);
	CC_SYNTHESIZE(int, _volume, Volume);
	CC_SYNTHESIZE(int, _pan, Pan);
	CC_SYNTHESIZE(int, _pitch, Pitch);
	CC_SYNTHESIZE(bool, _specifyAudioPosition, SpecifyAudioPosition);// 再生位置を指定
	CC_SYNTHESIZE(int, _audioPositionVariableObjectId, AudioPositionVariableObjectId);//ObjectId
	CC_SYNTHESIZE(EnumQualifierType, _audioPositionVariableQualifierId, AudioPositionVariableQualifierId);//単体/全体
	CC_SYNTHESIZE(int, _audioPositionVariableId, AudioPositionVariableId);//VariableId
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandMessageShowData : public ObjectCommandData
{
public:
	// ウィンドウ種類
	enum EnumWindow {
		kWindowNone = -1,		// 無し
		kWindowTemplate = 0,	// テンプレートから選択
		kWindowImage = 1,		// 画像素材から選択
	};

	// ウィンドウ種類：テンプレート
	enum EnumWindowTemplate {
		kWindowTemplateWhiteFrame,	// 白枠
		kWindowTemplateBlack,		// 黒
		kWindowTemplateWhite,		// 白
	};

	// 表示位置
	enum EnumPosition {
		kPositionCenter,			// このオブジェクトの中心
		kPositionLockObjectCenter,	// このオブジェクトがロックしたオブジェクトの中心
		kPositionBaseScreen,		// 画面を基準にする
		kPositionMax
	};

	// メッセージ水平方向揃え
	enum EnumHorzAlign {
		kHorzAlignLeft,		// 左寄せ
		kHorzAlignCenter,	// 中央
		kHorzAlignRight,	// 右寄せ
		kHorzAlignMax
	};

	// メッセージ垂直方向揃え
	enum EnumVertAlign {
		kVertAlignTop,		// 上段	
		kVertAlignCenter,	// 中央
		kVertAlignBottom,	// 下段
	};

	enum {
		kSelfObject = -2,
		kLockedObject = -5,
	};

private:
	ObjectCommandMessageShowData() : ObjectCommandData(), _topBottomMargin(0), _leftRightMargin(0) {};
	virtual ~ObjectCommandMessageShowData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandMessageShowData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandMessageShowData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(bool, _textFlag, TextFlag);
	CC_SYNTHESIZE(int, _textId, TextId);
	CC_SYNTHESIZE(int, _variableObjectId, VariableObjectId);
	CC_SYNTHESIZE(int, _variableQualifierId, VariableQualifierId);
	CC_SYNTHESIZE(int, _variableId, VariableId);
	CC_SYNTHESIZE(int, _fontId, FontId);
	CC_SYNTHESIZE(bool, _digitFlag, DigitFlag);	//桁数を指定(bool)
	CC_SYNTHESIZE(int, _digits, Digits);	//桁数(int)
	CC_SYNTHESIZE(bool, _zeroPadding, ZeroPadding);	//桁数分0を付ける(bool)
	CC_SYNTHESIZE(bool, _comma, Comma);		//”，”で区切る ※金額表記(bool)
	CC_SYNTHESIZE(bool, _withoutDecimalPlaces, WithoutDecimalPlaces);		//少数点以下を非表示(bool)
	CC_SYNTHESIZE(int, _colorA, ColorA);
	CC_SYNTHESIZE(int, _colorR, ColorR);
	CC_SYNTHESIZE(int, _colorG, ColorG);
	CC_SYNTHESIZE(int, _colorB, ColorB);
	CC_SYNTHESIZE(EnumWindow, _windowType, WindowType);
	CC_SYNTHESIZE(EnumWindowTemplate, _templateId, TemplateId);
	CC_SYNTHESIZE(int, _imageId, ImageId);
	CC_SYNTHESIZE(int, _windowTransparency, WindowTransparency);
	CC_SYNTHESIZE(int, _windowWidth, WindowWidth);
	CC_SYNTHESIZE(int, _windowHeight, WindowHeight);
	CC_SYNTHESIZE(EnumPosition, _positionType, PositionType);
	CC_SYNTHESIZE(bool, _useConnect, UseConnect);
	CC_SYNTHESIZE(int, _connectId, ConnectId);
	CC_SYNTHESIZE(int, _adjustX, AdjustX);
	CC_SYNTHESIZE(int, _adjustY, AdjustY);
	CC_SYNTHESIZE(int, _topBottomMargin, TopBottomMargin);
	CC_SYNTHESIZE(int, _leftRightMargin, LeftRightMargin);
	CC_SYNTHESIZE(EnumHorzAlign, _horzAlign, HorzAlign);
	CC_SYNTHESIZE(EnumVertAlign, _vertAlign, VertAlign);
	CC_SYNTHESIZE(int, _duration300, Duration300);
	CC_SYNTHESIZE(bool, _durationUnlimited, DurationUnlimited);
	CC_SYNTHESIZE(bool, _actionChangeHide, ActionChangeHide);
	CC_SYNTHESIZE(bool, _closeByKey, CloseByKey);
	CC_SYNTHESIZE(int, _keyId, KeyId);
	CC_SYNTHESIZE(bool, _objectStop, ObjectStop);
	CC_SYNTHESIZE(bool, _gameStop, GameStop);
	CC_SYNTHESIZE(bool, _priority, Priority);//優先順位フラグ
	CC_SYNTHESIZE(int, _priorityType, PriorityType);//表示位置（背景に表示、最前面に表示、最前面＋メニューに表示）
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandScrollMessageShowData : public ObjectCommandData
{
public:
	// 背景種類
	enum EnumBackground {
		kBackgroundNone = -1,		// 無し
		kBackgroundTemplate = 0,	// テンプレートから選択
		kBackgroundImage = 1,		// 画像素材から選択
	};

	// 背景種類：テンプレート
	enum EnumBackgroundTemplate {
		kBackgroundTemplateWhiteFrame,	// 白枠
		kBackgroundTemplateBlack,		// 黒
		kBackgroundTemplateWhite,		// 白
	};

	// 表示位置
	enum EnumPosition {
		kPositionCenter,			// このオブジェクトの中心
		kPositionLockObjectCenter,	// このオブジェクトがロックしたオブジェクトの中心
		kPositionBaseScreen,		// 画面を基準にする
		kPositionMax
	};

	// メッセージ水平方向揃え
	enum EnumHorzAlign {
		kHorzAlignLeft,		// 左寄せ
		kHorzAlignCenter,	// 中央
		kHorzAlignRight,	// 右寄せ
		kHorzAlignMax
	};

private:
	ObjectCommandScrollMessageShowData() : ObjectCommandData(), _topBottomMargin(0), _leftRightMargin(0){};
	virtual ~ObjectCommandScrollMessageShowData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandScrollMessageShowData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandScrollMessageShowData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _textId, TextId);
	CC_SYNTHESIZE(int, _colorA, ColorA);
	CC_SYNTHESIZE(int, _colorR, ColorR);
	CC_SYNTHESIZE(int, _colorG, ColorG);
	CC_SYNTHESIZE(int, _colorB, ColorB);
	CC_SYNTHESIZE(EnumBackground, _backgroundType, BackgroundType);
	CC_SYNTHESIZE(EnumBackgroundTemplate, _templateId, TemplateId);
	CC_SYNTHESIZE(int, _imageId, ImageId);
	CC_SYNTHESIZE(int, _backgroundTransparency, BackgroundTransparency);
	CC_SYNTHESIZE(int, _backgroundWidth, BackgroundWidth);
	CC_SYNTHESIZE(int, _backgroundHeight, BackgroundHeight);
	CC_SYNTHESIZE(EnumPosition, _positionType, PositionType);
	CC_SYNTHESIZE(bool, _useConnect, UseConnect);
	CC_SYNTHESIZE(int, _connectId, ConnectId);
	CC_SYNTHESIZE(int, _adjustX, AdjustX);
	CC_SYNTHESIZE(int, _adjustY, AdjustY);
	CC_SYNTHESIZE(EnumHorzAlign, _horzAlign, HorzAlign);
	CC_SYNTHESIZE(int, _topBottomMargin, TopBottomMargin);
	CC_SYNTHESIZE(int, _leftRightMargin, LeftRightMargin);
	CC_SYNTHESIZE(double, _scrollSpeed, ScrollSpeed);
	CC_SYNTHESIZE(bool, _scrollUp, ScrollUp);
	CC_SYNTHESIZE(bool, _actionChangeHide, ActionChangeHide);
	CC_SYNTHESIZE(bool, _speedUpByKey, SpeedUpByKey);
	CC_SYNTHESIZE(int, _keyId, KeyId);
	CC_SYNTHESIZE(bool, _objectStop, ObjectStop);
	CC_SYNTHESIZE(bool, _gameStop, GameStop);
	CC_SYNTHESIZE(bool, _priority, Priority);//優先順位フラグ
	CC_SYNTHESIZE(int, _priorityType, PriorityType);//表示位置（背景に表示、最前面に表示、最前面＋メニューに表示）
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandEffectShowData : public ObjectCommandData
{
public:
	enum EnumPosition {
		kPositionCenter,//このオブジェクトを中心とする
		kPositionLockObjectCenter,//このオブジェクトがロックしたオブジェクトを中心とする
		kPositionMax,
	};

private:
	ObjectCommandEffectShowData() : ObjectCommandData() {};
	virtual ~ObjectCommandEffectShowData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandEffectShowData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandEffectShowData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _effectId, EffectId);
	CC_SYNTHESIZE(EnumPosition, _positionType, PositionType);
	CC_SYNTHESIZE(bool, _useConnect, UseConnect);
	CC_SYNTHESIZE(int, _connectId, ConnectId);
	CC_SYNTHESIZE(int, _adjustX, AdjustX);
	CC_SYNTHESIZE(int, _adjustY, AdjustY);
	CC_SYNTHESIZE(int, _duration300, Duration300);
	CC_SYNTHESIZE(bool, _durationUnlimited, DurationUnlimited);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandMovieShowData : public ObjectCommandData
{
public:
	enum EnumPosition {
		kPositionCenter,
		kPositionLockObjectCenter,
		kPositionScenePosition,
		kPositionMax,
	};
	enum EnumVertAlign {
		kVertAlignCenter,//中段
		kVertAlignTop,//上段
		kVertAlignBottom,//下段
		kVertAlignMax
	};
	enum EnumHorzAlign {
		kHorzAlignCenter,//中央
		kHorzAlignLeft,//左寄せ
		kHorzAlignRight,//右寄せ
		kHorzAlignMax
	};
private:
	ObjectCommandMovieShowData() : ObjectCommandData() {};
	virtual ~ObjectCommandMovieShowData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandMovieShowData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandMovieShowData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	//▼動画を表示
	CC_SYNTHESIZE(int, _movieId, MovieId);//表示する動画ID
	//▼再生の設定
	CC_SYNTHESIZE(bool, _loop , Loop);//ループ
	CC_SYNTHESIZE(int, _volume, Volume);//ボリューム
	//▼レイアウトと動作
	//動画サイズ
	CC_SYNTHESIZE(bool, _defaultSize, DefaultSize);//デフォルトのサイズ
	CC_SYNTHESIZE(int, _width, Width);//サイズを調整（幅）
	CC_SYNTHESIZE(int, _height, Height);//サイズを調整（高さ）
	//表示位置
	CC_SYNTHESIZE(EnumPosition, _positionType, PositionType);//表示位置タイプ（EnumPosition）
	CC_SYNTHESIZE(bool, _useConnect, UseConnect);//接続点を使用
	CC_SYNTHESIZE(int, _connectId, ConnectId);//接続点ID
	CC_SYNTHESIZE(EnumVertAlign, _vertAlign, VertAlign);//シーンを基準にする（上段、中段、下段など）
	CC_SYNTHESIZE(EnumHorzAlign, _horzAlign, HorzAlign);//シーンを基準にする（左寄り、中央、右より）
	CC_SYNTHESIZE(int, _adjustX, AdjustX);//位置を調整X
	CC_SYNTHESIZE(int, _adjustY, AdjustY);//位置を調整Y
	//オプション
	CC_SYNTHESIZE(bool, _hideOnObjectActionChange, HideOnObjectActionChange);//オブジェクトのアクションが切り替わったら表示を終了
	CC_SYNTHESIZE(bool, _stopObject, StopObject);//動画再生中はすべてのオブジェクト動作を停止
	CC_SYNTHESIZE(bool, _stopGame, StopGame);//動画再生中はゲームの動作を一時停止
	CC_SYNTHESIZE(bool, _fillBlack, FillBlack);//動画以外のエリアを黒塗りにする
	CC_SYNTHESIZE(bool, _priority, Priority);//優先順位フラグ
	CC_SYNTHESIZE(int, _priorityType, PriorityType);//表示位置（背景に表示、最前面に表示、最前面＋メニューに表示）
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandImageShowData : public ObjectCommandData
{
public:
	enum EnumPosition {
		kPositionCenter,
		kPositionLockObjectCenter,
		kPositionScenePosition,
		kPositionMax,
	};
	enum EnumVertAlign {
		kVertAlignCenter,
		kVertAlignTop,
		kVertAlignBottom,
		kVertAlignMax
	};
	enum EnumHorzAlign {
		kHorzAlignCenter,
		kHorzAlignLeft,
		kHorzAlignRight,
		kHorzAlignMax
	};
private:
	ObjectCommandImageShowData() : ObjectCommandData() {};
	virtual ~ObjectCommandImageShowData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandImageShowData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandImageShowData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	//▼画像を表示
	CC_SYNTHESIZE(int, _imageId, ImageId);//表示する画像ID
	//▼レイアウトと動作
	//画像サイズ
	CC_SYNTHESIZE(bool, _defaultSize, DefaultSize);//デフォルトサイズフラグ
	CC_SYNTHESIZE(int, _width, Width);//サイズ調整（幅）
	CC_SYNTHESIZE(int, _height, Height);//サイズ調整（高さ）
	//表示位置
	CC_SYNTHESIZE(EnumPosition, _positionType, PositionType);//表示位置（EnumPosition）
	CC_SYNTHESIZE(bool, _useConnect, UseConnect);//接続点を使用
	CC_SYNTHESIZE(int, _connectId, ConnectId);//接続点ID
	CC_SYNTHESIZE(EnumVertAlign, _vertAlign, VertAlign);//シーンに基準にする（上段、中段、下段）
	CC_SYNTHESIZE(EnumHorzAlign, _horzAlign, HorzAlign);//シーンに基準にする（左寄せ、中央、右寄せ）
	CC_SYNTHESIZE(int, _adjustX, AdjustX);//位置を調整X
	CC_SYNTHESIZE(int, _adjustY, AdjustY);//位置を調整Y
	//表示時間
	CC_SYNTHESIZE(int, _duration300, Duration300);//表示時間
	CC_SYNTHESIZE(bool, _durationUnlimited, DurationUnlimited);//時間制限なし
	//オプション
	CC_SYNTHESIZE(bool, _hideOnObjectActionChange, HideOnObjectActionChange);//オブジェクトのアクションが切り替わったら表示を終了
	CC_SYNTHESIZE(bool, _closeByOk, CloseByOk);//決定ボタンが押されたら画像を閉じる
	CC_SYNTHESIZE(bool, _stopObject, StopObject);//画像表示中はすべてのオブジェクト動作を停止
	CC_SYNTHESIZE(bool, _stopGame, StopGame);//画像表示中はゲームの動作を一時停止
	CC_SYNTHESIZE(bool, _priority, Priority);//位置表示フラグ
	CC_SYNTHESIZE(int, _priorityType, PriorityType);//表示位置（背景に表示、最前面に表示、最前面＋メニューに表示）
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandSwitchVariableChangeData : public ObjectCommandData
{
public:
	enum EnumSwitchAssignValue {
		kSwitchAssignOn,
		kSwitchAssignOff,
		kSwitchAssignToggle,
		kSwitchAssignMax
	};
	enum EnumVariableAssignOperatorType {
		kVariableAssignOperatorSet,
		kVariableAssignOperatorAdd,
		kVariableAssignOperatorSub,
		kVariableAssignOperatorMul,
		kVariableAssignOperatorDiv,
		kVariableAssignOperatorMod,
		kVariableAssignOperatorMax
	};
	enum EnumVariableAssignValueType {
		kVariableAssignValue,
		kVariableAssignVariable,
		kVariableAssignRandom,
		kVariableAssignScript,
		kVariableAssignValueMax
	};
private:
	ObjectCommandSwitchVariableChangeData();
	virtual ~ObjectCommandSwitchVariableChangeData();
public:
	CREATE_FUNC_PARAM(ObjectCommandSwitchVariableChangeData, const rapidjson::Value&, json);
	const char *getAssignScript();
	CREATE_FUNC_PARAM2(ObjectCommandSwitchVariableChangeData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(bool, _swtch, Swtch);//スイッチor変数(スイッチ:TRUE,変数:FALSE)
	CC_SYNTHESIZE(int, _switchObjectId, SwitchObjectId);//スイッチObjectID
	CC_SYNTHESIZE(int, _switchId, SwitchId);//スイッチID
	CC_SYNTHESIZE(EnumQualifierType, _switchQualifierId, SwitchQualifierId);//全体/単体
	CC_SYNTHESIZE(EnumSwitchAssignValue, _switchValue, SwitchValue);//スイッチ値（ON,OFF,切り替え)
	CC_SYNTHESIZE(int, _variableObjectId, VariableObjectId);//VariableObjectId
	CC_SYNTHESIZE(int, _variableId, VariableId);//VariableId
	CC_SYNTHESIZE(EnumQualifierType, _variableQualifierId, VariableQualifierId);//単体/全体
	CC_SYNTHESIZE(EnumVariableAssignOperatorType, _variableAssignOperator, VariableAssignOperator);//演算(=,+=,-=,*=,/=,%=)
	CC_SYNTHESIZE(EnumVariableAssignValueType, _variableAssignValueType, VariableAssignValueType);//タイプ（定数、変数、乱数、スクリプト）
	CC_SYNTHESIZE(double, _assignValue, AssignValue);//定数
	CC_SYNTHESIZE(int, _assignVariableObjectId, AssignVariableObjectId);//VariableオブジェクトID
	CC_SYNTHESIZE(int, _assignVariableId, AssignVariableId);//VariableID
	CC_SYNTHESIZE(EnumQualifierType, _assignVariableQualifierId, AssignVariableQualifierId);//単体/全体
	CC_SYNTHESIZE(int, _randomMin, RandomMin);//乱数最小
	CC_SYNTHESIZE(int, _randomMax, RandomMax);//乱数最大
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _assignScript ,AssignScript);//スクリプト
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandSwitchVariableResetData : public ObjectCommandData
{
public:
	class SwitchVariableData : public cocos2d::Ref {
	private:
		SwitchVariableData() {};
		virtual ~SwitchVariableData() {};
	public:
		CREATE_FUNC_PARAM(SwitchVariableData, const rapidjson::Value&, json);
		CREATE_FUNC_PARAM2(SwitchVariableData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
		void dump() {
			CCLOG("swtch:%d", this->getSwtch());
			CCLOG("objectId:%d", this->getObjectId());
			CCLOG("itemId:%d", this->getItemId());
		}
#endif
	private:
		virtual bool init(const rapidjson::Value& json) {
			this->setSwtch(json["swtch"].GetBool());
			this->setObjectId(json["objectId"].GetInt());
			this->setItemId(json["itemId"].GetInt());
			return true;
		}
		bool init(void *jsCx, void *jsObj);
	private:
		CC_SYNTHESIZE(bool, _swtch, Swtch);
		CC_SYNTHESIZE(int, _objectId, ObjectId);
		CC_SYNTHESIZE(int, _itemId, ItemId);
	};
private:
	ObjectCommandSwitchVariableResetData();
	virtual ~ObjectCommandSwitchVariableResetData();
public:
	CREATE_FUNC_PARAM(ObjectCommandSwitchVariableResetData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandSwitchVariableResetData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _switchVariableResetList, SwitchVariableResetList);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandGameSpeedChangeData : public ObjectCommandData
{
public:
	enum EnumTargettingType {
		kTargettingByGroup,// 「オブジェクトの種類で指定」
		kTargettingById,	// 「オブジェクトで指定」
		kTargettingTouched,	// 「このオブジェクトに触れたオブジェクト」
		kTargettingLocked, // 「このオブジェクトがロックしたオブジェクト」
		kTargettingAllObject, // 「シーン内すべてのオブジェクト」
		kTargettingMax
	};
private:
	ObjectCommandGameSpeedChangeData() : ObjectCommandData() {};
	virtual ~ObjectCommandGameSpeedChangeData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandGameSpeedChangeData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandGameSpeedChangeData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(double, _gameSpeed, GameSpeed);
	CC_SYNTHESIZE(int, _duration300, Duration300);
	CC_SYNTHESIZE(bool, _durationUnlimited, DurationUnlimited);
	CC_SYNTHESIZE(bool, _targetObject, TargetObject);
	CC_SYNTHESIZE(bool, _targetEffect, TargetEffect);
	CC_SYNTHESIZE(bool, _targetTile, TargetTile);
	CC_SYNTHESIZE(bool, _targetMenu, TargetMenu);
	CC_SYNTHESIZE(EnumTargettingType, _targettingType, TargettingType);
	CC_SYNTHESIZE(int, _targetObjectGroup, TargetObjectGroup);
	CC_SYNTHESIZE(int, _targetObjectId, TargetObjectId);
	CC_SYNTHESIZE(EnumQualifierType, _targetQualifierId, TargetQualifierId);
	CC_SYNTHESIZE(int, _excludeObjectGroupBit, ExcludeObjectGroupBit);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandWaitData : public ObjectCommandData
{
private:
	ObjectCommandWaitData() : ObjectCommandData() {};
	virtual ~ObjectCommandWaitData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandWaitData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(int, _duration300, Duration300);
	CC_SYNTHESIZE(bool, _stopGame, StopGame);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandSceneTerminateData : public ObjectCommandData
{
private:
	ObjectCommandSceneTerminateData() : ObjectCommandData() {};
	virtual ~ObjectCommandSceneTerminateData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandSceneTerminateData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandSceneTerminateData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandDirectionMoveData : public ObjectCommandData
{
public:
	enum {
		kAccordingToMoveDirection = -2,
	};
private:
	ObjectCommandDirectionMoveData() : ObjectCommandData(), _moveDistance(0), _moveDistanceEnabled(false){};
	virtual ~ObjectCommandDirectionMoveData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandDirectionMoveData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandDirectionMoveData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(double, _direction, Direction);
	CC_SYNTHESIZE(int, _directionId, DirectionId);
	CC_SYNTHESIZE(int, _moveDistance, MoveDistance);
	CC_SYNTHESIZE(bool, _moveDistanceEnabled, MoveDistanceEnabled);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandForthBackMoveTurnData : public ObjectCommandData
{
public:
	enum {
		kMoveNone = 0,
		kMoveForth,
		kMoveBack
	};
	enum {
		kTurnNone = 0,
		kTurnRight,
		kTurnLeft
	};
	enum {
		kAccordingToMoveDirection = -2,
	};
private:
	ObjectCommandForthBackMoveTurnData() : ObjectCommandData() {};
	virtual ~ObjectCommandForthBackMoveTurnData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandForthBackMoveTurnData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandForthBackMoveTurnData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _moveType, MoveType);//移動（0:なし、1:前方移動、2:後方移動）
	CC_SYNTHESIZE(int, _turnType, TurnType);//旋回（0:なし、1:右旋回、2:左旋回）
	CC_SYNTHESIZE(int, _directionId, DirectionId);//移動方向
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandActionExecData : public ObjectCommandData
{
private:
	ObjectCommandActionExecData() : ObjectCommandData() {};
	virtual ~ObjectCommandActionExecData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandActionExecData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandActionExecData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _objectId, ObjectId);
	CC_SYNTHESIZE(EnumQualifierType, _qualifierId, QualifierId);
	CC_SYNTHESIZE(int, _actionId, ActionId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandParticleShowData : public ObjectCommandData
{
public:
	enum EnumPosition {
		kPositionCenter,
		kPositionLockObjectCenter,
		kPositionMax,
	};
private:
	ObjectCommandParticleShowData() : ObjectCommandData() {};
	virtual ~ObjectCommandParticleShowData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandParticleShowData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandParticleShowData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _particleId, ParticleId);
	CC_SYNTHESIZE(bool, _useConnect, UseConnect);
	CC_SYNTHESIZE(int, _connectId, ConnectId);
	CC_SYNTHESIZE(int, _adjustX, AdjustX);
	CC_SYNTHESIZE(int, _adjustY, AdjustY);
	CC_SYNTHESIZE(int, _duration300, Duration300);
	CC_SYNTHESIZE(bool, _durationUnlimited, DurationUnlimited);
	CC_SYNTHESIZE(int, _positionType, PositionType);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandTimerData : public ObjectCommandData
{
public:
	enum EnumSecondType {
		kSecondByValue,
		kSecondByVariable,
		kScondTypeMax
	};
private:
	ObjectCommandTimerData() : ObjectCommandData() {};
	virtual ~ObjectCommandTimerData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandTimerData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandTimerData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(bool, _start, Start);
	CC_SYNTHESIZE(int, _timerVariableObjectId, TimerVariableObjectId);
	CC_SYNTHESIZE(int, _timerVariableQualifierId, TimerVariableQualifierId);
	CC_SYNTHESIZE(int, _timerVariableId, TimerVariableId);
	CC_SYNTHESIZE(bool, _countUp, CountUp);
	CC_SYNTHESIZE(int, _secondType, SecondType);
	CC_SYNTHESIZE(int, _second300, Second300);
	CC_SYNTHESIZE(int, _secondVariableObjectId, SecondVariableObjectId);
	CC_SYNTHESIZE(int, _secondVariableQualifierId, SecondVariableQualifierId);
	CC_SYNTHESIZE(int, _secondVariableId, SecondVariableId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandSceneShakeData : public ObjectCommandData
{
private:
	ObjectCommandSceneShakeData() : ObjectCommandData() {};
	virtual ~ObjectCommandSceneShakeData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandSceneShakeData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandSceneShakeData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _duration300, Duration300);
	CC_SYNTHESIZE(bool, _fadeIn, FadeIn);
	CC_SYNTHESIZE(bool, _fadeOut, FadeOut);
	CC_SYNTHESIZE(int, _interval300, Interval300);
	CC_SYNTHESIZE(int, _height, Height);
	CC_SYNTHESIZE(int, _heightDispersion, HeightDispersion);
	CC_SYNTHESIZE(int, _width, Width);
	CC_SYNTHESIZE(int, _widthDispersion, WidthDispersion);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandEffectRemoveData : public ObjectCommandData
{
public:
	enum {
		kAllParticles = -2,
	};
	enum EnumTargettingType {
		kTargettingByGroup,
		kTargettingById,
		//kTargettingTouched,
		//kTargettingLocked,
		//kTargettingAllObject,
		kTargettingSceneEffect = 5,
		kTargettingMax
	};
private:
	ObjectCommandEffectRemoveData() : ObjectCommandData() {};
	virtual ~ObjectCommandEffectRemoveData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandEffectRemoveData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandEffectRemoveData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _effectId, EffectId);
	CC_SYNTHESIZE(int, _targetObjectId, TargetObjectId);
	CC_SYNTHESIZE(int, _targetObjectGroup, TargetObjectGroup);
	CC_SYNTHESIZE(EnumTargettingType, _targettingType, TargettingType);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandParticleRemoveData : public ObjectCommandData
{
public:
	enum {
		kAllParticles = -2,
	};
	enum EnumTargettingType {
		kTargettingByGroup,
		kTargettingById,
		//kTargettingTouched,
		//kTargettingLocked,
		//kTargettingAllObject,
		kTargettingSceneEffect = 5,
		kTargettingMax
	};
	static const int NO_PARTICLE_TARGET = -1;	// 「設定無し」
	static const int ALL_PARTICLE = -2;			// 「全てのパーティクル」
private:
	ObjectCommandParticleRemoveData() : ObjectCommandData() {};
	virtual ~ObjectCommandParticleRemoveData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandParticleRemoveData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandParticleRemoveData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _particleId, ParticleId);
	CC_SYNTHESIZE(int, _targetObjectId, TargetObjectId);
	CC_SYNTHESIZE(int, _targetObjectGroup, TargetObjectGroup);
	CC_SYNTHESIZE(EnumTargettingType, _targettingType, TargettingType);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandLayerHide : public ObjectCommandData
{
private:
	ObjectCommandLayerHide() : ObjectCommandData() {};
	virtual ~ObjectCommandLayerHide() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandLayerHide, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandLayerHide, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(bool, _exceptFlag, ExceptFlag);
	CC_SYNTHESIZE(int, _layerIndex, LayerIndex);
};
//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandLayerShow : public ObjectCommandData
{
private:
	ObjectCommandLayerShow() : ObjectCommandData() {};
	virtual ~ObjectCommandLayerShow() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandLayerShow, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandLayerShow, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(bool, _exceptFlag, ExceptFlag);
	CC_SYNTHESIZE(int, _layerIndex, LayerIndex);
};
//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandLayerDisable : public ObjectCommandData
{
private:
	ObjectCommandLayerDisable() : ObjectCommandData() {};
	virtual ~ObjectCommandLayerDisable() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandLayerDisable, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandLayerDisable, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(bool, _exceptFlag, ExceptFlag);
	CC_SYNTHESIZE(int, _layerIndex, LayerIndex);
};
//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandLayerEnable : public ObjectCommandData
{
private:
	ObjectCommandLayerEnable() : ObjectCommandData() {};
	virtual ~ObjectCommandLayerEnable() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandLayerEnable, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandLayerEnable, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(bool, _exceptFlag, ExceptFlag);
	CC_SYNTHESIZE(int, _layerIndex, LayerIndex);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandScriptEvaluate : public ObjectCommandData
{
public:
private:
	ObjectCommandScriptEvaluate();
	virtual ~ObjectCommandScriptEvaluate();
public:
	CREATE_FUNC_PARAM(ObjectCommandScriptEvaluate, const rapidjson::Value&, json);
	const char *getScript();
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _script, Script);//スクリプト
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandSoundStopData : public ObjectCommandData
{
public:
	enum EnumSound {
		kSoundSe,
		kSoundVoice,
		kSoundBgm,
		kSoundMax
	};
private:
	ObjectCommandSoundStopData() : ObjectCommandData(), _soundType(kSoundSe), _fadeout(false), _duration300(300) {};
	virtual ~ObjectCommandSoundStopData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandSoundStopData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandSoundStopData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(EnumSound, _soundType, SoundType);
	CC_SYNTHESIZE(bool, _fadeout, Fadeout);
	CC_SYNTHESIZE(int, _duration300, Duration300);
	CC_SYNTHESIZE(int, _seId, SeId);
	CC_SYNTHESIZE(int, _voiceId, VoiceId);
	CC_SYNTHESIZE(int, _bgmId, BgmId);
	CC_SYNTHESIZE(bool, _stopOnlySoundByThisObject, StopOnlySoundByThisObject);// このオブジェクトが再生した音のみを停止
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandMenuShowData : public ObjectCommandData
{
public:
	enum EnumEffectType {
		kNone = -1,
		kSlideUp,
		kSlideDown,
		kSlideLeft,
		kSlideRight,
		kEffectMax
	};
private:
	ObjectCommandMenuShowData() : ObjectCommandData() {};
	virtual ~ObjectCommandMenuShowData() {};
public:
	static ObjectCommandMenuShowData *create(int layerId, bool useEffect, int effectType, bool fadein, int duration300);
	CREATE_FUNC_PARAM(ObjectCommandMenuShowData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandMenuShowData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(int layerId, bool useEffect, int effectType, bool fadein, int duration300);
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _layerId, LayerId);
	CC_SYNTHESIZE(bool, _useEffect, UseEffect);
	CC_SYNTHESIZE(int, _effectType, EffectType);
	CC_SYNTHESIZE(bool, _fadein, Fadein);
	CC_SYNTHESIZE(int, _duration300, Duration300);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandMenuHideData : public ObjectCommandData
{
public:
	enum EnumEffectType {
		kNone = -1,
		kSlideUp,
		kSlideDown,
		kSlideLeft,
		kSlideRight,
		kEffectMax
	};
private:
	ObjectCommandMenuHideData() : ObjectCommandData() {};
	virtual ~ObjectCommandMenuHideData() {};
public:
	static ObjectCommandMenuHideData *create(bool hideExceptInitial, bool useEffect, int effectType, bool fadeout, int duration300, bool disableObjects);
	CREATE_FUNC_PARAM(ObjectCommandMenuHideData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandMenuHideData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(bool hideExceptInitial, bool useEffect, int effectType, bool fadeout, int duration300, bool disableObjects);
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(bool, _hideExceptInitial, HideExceptInitial);
	CC_SYNTHESIZE(bool, _useEffect, UseEffect);
	CC_SYNTHESIZE(int, _effectType, EffectType);
	CC_SYNTHESIZE(bool, _fadeout, Fadeout);
	CC_SYNTHESIZE(int, _duration300, Duration300);
	CC_SYNTHESIZE(bool, _disableObjects, DisableObjects);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandDisplayDirectionMoveData : public ObjectCommandData
{
private:
	ObjectCommandDisplayDirectionMoveData() : ObjectCommandData() {};
	virtual ~ObjectCommandDisplayDirectionMoveData() {};
public:
	static ObjectCommandDisplayDirectionMoveData *create(int moveDistance, bool reverse, bool distanceOverride);
	CREATE_FUNC_PARAM(ObjectCommandDisplayDirectionMoveData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandDisplayDirectionMoveData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(int moveDistance, bool reverse, bool distanceOverride);
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _moveDistance, MoveDistance);
	CC_SYNTHESIZE(bool, _reverse, Reverse);
	CC_SYNTHESIZE(bool, _distanceOverride, DistanceOverride);
};

//-------------------------------------------------------------------------------------------------------------------

class AGTKPLAYER_API ObjectCommandFileLoadData : public ObjectCommandData
{
private:
	ObjectCommandFileLoadData();
	virtual ~ObjectCommandFileLoadData() {};
public:
	enum {
		kEffectTypeNone = -1,
		kEffectTypeBlack,
		kEffectTypeWhite,
		kEffectTypeSlideUp,
		kEffectTypeSlideDown,
		kEffectTypeSlideLeft,
		kEffectTypeSlideRight,
		kEffectTypeMax
	};
	CREATE_FUNC_PARAM(ObjectCommandFileLoadData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandFileLoadData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(bool, _projectCommonVariables, ProjectCommonVariables);///< プロジェクト共通変数のロードを行う。
	CC_SYNTHESIZE(bool, _projectCommonSwitches, ProjectCommonSwitches);///< プロジェクト共通スイッチのロードを行う。
	CC_SYNTHESIZE(bool, _sceneAtTimeOfSave, SceneAtTimeOfSave);///< セーブ時のシーンのロードを行う。
	CC_SYNTHESIZE(bool, _objectsStatesInSceneAtTimeOfSave, ObjectsStatesInSceneAtTimeOfSave);///< シーンに配置されているオブジェクトの状態のロードを行う
	CC_SYNTHESIZE(int, _effectType, EffectType);///< 画面演出
	CC_SYNTHESIZE(int, _duration300, Duration300);///< 演出時間

	DataSetterList _dataSetterList;// メンバ変数に値を設定する
};

//-------------------------------------------------------------------------------------------------------------------
// 音の再生位置を保存
class AGTKPLAYER_API ObjectCommandSoundPositionRememberData : public ObjectCommandData {
public:
	enum EnumSound {
		kSoundSe,
		kSoundVoice,
		kSoundBgm,
		kSoundMax
	};
private:
	ObjectCommandSoundPositionRememberData();
	virtual ~ObjectCommandSoundPositionRememberData();
public:
	CREATE_FUNC_PARAM(ObjectCommandSoundPositionRememberData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandSoundPositionRememberData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);

	CC_SYNTHESIZE(EnumSound, _soundType, SoundType);//サウンドタイプ
	CC_SYNTHESIZE(int, _variableObjectId, VariableObjectId);//ObjectID
	CC_SYNTHESIZE(EnumQualifierType, _variableQualifierId, VariableQualifierId);//単体/全体
	CC_SYNTHESIZE(int, _variableId, VariableId);//VariableId
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandObjectUnlockData : public ObjectCommandData
{
public:
private:
	ObjectCommandObjectUnlockData() : ObjectCommandData() {};
	virtual ~ObjectCommandObjectUnlockData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandObjectUnlockData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandObjectUnlockData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _objectType, ObjectType);// オブジェクトの指定方法
	CC_SYNTHESIZE(int, _objectGroup, ObjectGroup);
	CC_SYNTHESIZE(int, _objectId, ObjectId);
};

//-------------------------------------------------------------------------------------------------------------------

class AGTKPLAYER_API ObjectCommandCustomData : public ObjectCommandData
{
private:
	ObjectCommandCustomData();
	virtual ~ObjectCommandCustomData();
public:
	CREATE_FUNC_PARAM(ObjectCommandCustomData, const rapidjson::Value&, json);
	const char *getValueJson();
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(int, _customId, CustomId);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _valueJson, ValueJson);//設定データ
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandResourceSetChangeData : public ObjectCommandData
{
private:
	ObjectCommandResourceSetChangeData() : ObjectCommandData() {};
	virtual ~ObjectCommandResourceSetChangeData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandResourceSetChangeData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandResourceSetChangeData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _objectId, ObjectId);
	CC_SYNTHESIZE(EnumQualifierType, _qualifierId, QualifierId);
	CC_SYNTHESIZE(int, _resourceSetId, ResourceSetId);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectCommandDatabaseReflectData : public ObjectCommandData
{
private:
	ObjectCommandDatabaseReflectData();
	virtual ~ObjectCommandDatabaseReflectData();
public:
	CREATE_FUNC_PARAM(ObjectCommandDatabaseReflectData, const rapidjson::Value&, json);
	const char *getResourceSetName();
	const char *getAnimMotionName();
	CREATE_FUNC_PARAM2(ObjectCommandDatabaseReflectData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _objectId, ObjectId);
	CC_SYNTHESIZE(int, _databaseId, DatabaseId);
	CC_SYNTHESIZE(bool, _fromObject, FromObject);
	CC_SYNTHESIZE(bool, _fromRow, FromRow);
	CC_SYNTHESIZE(int, _columnIndex, ColumnIndex);
	CC_SYNTHESIZE(int, _rowIndex, RowIndex);
	CC_SYNTHESIZE(bool, _rowIndexFromName, RowIndexFromName);
	CC_SYNTHESIZE(bool, _columnIndexFromName, ColumnIndexFromName);
	CC_SYNTHESIZE(bool, _rowNumberFromValue, RowNumberFromValue);
	CC_SYNTHESIZE(bool, _columnNumberFromValue, ColumnNumberFromValue);
	CC_SYNTHESIZE(int, _rowVariableObjectId, RowVariableObjectId);
	CC_SYNTHESIZE(int, _rowVariableId, RowVariableId);
	CC_SYNTHESIZE(int, _rowVariableQualifierId, RowVariableQualifierId);
	CC_SYNTHESIZE(int, _columnVariableObjectId, ColumnVariableObjectId);
	CC_SYNTHESIZE(int, _columnVariableId, ColumnVariableId);
	CC_SYNTHESIZE(int, _columnVariableQualifierId, ColumnVariableQualifierId);

	CC_SYNTHESIZE(int, _reflectObjectId, ReflectObjectId);
	CC_SYNTHESIZE(int, _reflectVariableId, ReflectVariableId);
	CC_SYNTHESIZE(EnumQualifierType, _reflectQualifierId, ReflectQualifierId);
	CC_SYNTHESIZE(EnumVariableAssignOperatorType, _reflectVariableAssignOperator, ReflectVariableAssignOperator);//演算(=,+=,-=,*=,/=,%=)

};

//-------------------------------------------------------------------------------------------------------------------
// #AGTK-NX
class AGTKPLAYER_API ObjectCommandkNXVibrateControlData : public ObjectCommandData
{
private:
	ObjectCommandkNXVibrateControlData() : ObjectCommandData() {};
	virtual ~ObjectCommandkNXVibrateControlData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandkNXVibrateControlData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandkNXVibrateControlData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, vibration_pattern_, VibrationPattern);
	CC_SYNTHESIZE(int, variable_id_, VariableId);
};

//-------------------------------------------------------------------------------------------------------------------
// #AGTK-NX
class AGTKPLAYER_API ObjectCommandNXShowControllerAppletData : public ObjectCommandData
{
private:
	ObjectCommandNXShowControllerAppletData() : ObjectCommandData() {};
	virtual ~ObjectCommandNXShowControllerAppletData() {};
public:
	CREATE_FUNC_PARAM(ObjectCommandNXShowControllerAppletData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM2(ObjectCommandNXShowControllerAppletData, void *, jsCx, void *, jsObj);
#if defined(AGTK_DEBUG)
	virtual void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	bool init(void *jsCx, void *jsObj);
private:
	CC_SYNTHESIZE(int, _playerCount, PlayerCount);
};

NS_DATA_END
NS_AGTK_END

#endif	//__AGTK_OBJECT_COMMAND_DATA_H__
