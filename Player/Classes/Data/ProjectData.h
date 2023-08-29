#ifndef __AGTK_PROJECT_DATA_H__
#define	__AGTK_PROJECT_DATA_H__

#include "cocos2d.h"
#include "json/document.h"
#include "Lib/Macros.h"
#include "Data/AssetData.h"
#include "Data/AnimationData.h"
#include "Data/TileData.h"
#include "Data/ObjectData.h"
#include "Data/SceneData.h"
#include "Data/DatabaseData.h"

USING_NS_CC;

NS_AGTK_BEGIN
NS_DATA_BEGIN

static const int MOVE_BGM_ID_NONE = -1;		// 演出切替のBGMの未設定時ID
static const int MOVE_SE_ID_NONE = -1;		// 演出切替のSEの未設定時ID

/* 画面切替条件 */
enum EnumChangeConditionType {
	kAllConditionsSatisfied,	// すべての条件が有効
	kAnyConditionSatisfied,		// いずれかの条件が有効
};

/* BGM変更タイミング */
enum EnumBgmChangeTiming {
	kBgmChangeTimingBeforeMoveEffect,	// 画面切替演出前
	kBgmChangeTimingAfterMoveEffect,	// 画面切替演出後
	kBgmChangeTimingMax
};

/* 画面切替時のBGM演出設定 */
enum EnumBgmChangeType {
	kBgmChangeTypeContinue,	// 継続再生
	kBgmChangeTypeStop,		// 再生停止
	kBgmChangeTypeChange,	// BGM変更
	kBgmChangeTypeMax
};

/* 画面切替演出設定 */
enum EnumMoveEffect {
	kMoveEffectNone,				// 無し
	kMoveEffectBlack,				// ブラック(黒フェード)
	kMoveEffectWhite,				// ホワイト(白フェード)
	kMoveEffectSlideUp,				// スライドアップ
	kMoveEffectSlideDown,			// スライドダウン
	kMoveEffectSlideLeft,			// スライドレフト
	kMoveEffectSlideRight,			// スライドライト
	kMoveEffectSlideUpLink,			// スライドアップ(連結)
	kMoveEffectSlideDownLink,		// スライドダウン(連結)
	kMoveEffectSlideLeftLink,		// スライドレフト(連結)
	kMoveEffectSlideRightLink,		// スライドライト(連結)
	kMoveEffectMax
};

/* 条件のオブジェクト対象制限 */
enum EnumQualifierType {
	kQualifierSingle = -1,//単体
	kQualifierWhole = -2,//全体
};

//-------------------------------------------------------------------------------------------------------------------
//! 画面遷移演出基本データ
class AGTKPLAYER_API BaseSceneChangeEffectData : public cocos2d::Ref
{
protected:
	BaseSceneChangeEffectData();
	virtual ~BaseSceneChangeEffectData();

public:
	CREATE_FUNC_PARAM2(BaseSceneChangeEffectData, const rapidjson::Value&, json, int, id);
#if defined(AGTK_DEBUG)
	void dump();
#endif

protected:
	virtual bool init(const rapidjson::Value& json, int id);

private:
	CC_SYNTHESIZE(int, _id, Id);// portalId or transitionLinkId
	// --------------------------------------------------------------------------------------------------------------------
	//! 切替え前演出のパラメータ
	// ▼演出の設定
	CC_SYNTHESIZE(EnumMoveEffect, _preMoveEffect, PreMoveEffect);// 切替え演出
	CC_SYNTHESIZE(int, _preMoveDuration300, PreMoveDuration300);// 切替え完了までの時間

	CC_SYNTHESIZE(EnumBgmChangeType, _preMoveBgmChangeType, PreMoveBgmChangeType);// BGMの演出タイプ
	CC_SYNTHESIZE(bool, _preMoveBgmFadeout, PreMoveBgmFadeout);// BGMのフェードアウトフラグ
	CC_SYNTHESIZE(int, _preMoveBgmId, PreMoveBgmId);// 変更先BGMのID
	CC_SYNTHESIZE(bool, _preMoveBgmLoop, PreMoveBgmLoop);// 変更後のBGMのループフラグ

	CC_SYNTHESIZE(EnumBgmChangeTiming, _preMoveBgmChangeTiming, PreMoveBgmChangeTiming);// BGMの変更や停止のタイミング

	CC_SYNTHESIZE(bool, _preMovePlaySe, PreMovePlaySe);// SE再生フラグ
	CC_SYNTHESIZE(int, _preMoveSeId, PreMoveSeId);// 再生するSEのID

	// --------------------------------------------------------------------------------------------------------------------
	//! 切替え後演出のパラメータ
	// ▼演出の設定
	CC_SYNTHESIZE(EnumMoveEffect, _postMoveEffect, PostMoveEffect);// 切替え演出
	CC_SYNTHESIZE(int, _postMoveDuration300, PostMoveDuration300);// 切替え完了までの時間

	CC_SYNTHESIZE(EnumBgmChangeType, _postMoveBgmChangeType, PostMoveBgmChangeType);// BGMの演出タイプ
	CC_SYNTHESIZE(bool, _postMoveBgmFadeout, PostMoveBgmFadeout);// BGMのフェードアウトフラグ
	CC_SYNTHESIZE(int, _postMoveBgmId, PostMoveBgmId);// 変更先BGMのID
	CC_SYNTHESIZE(bool, _postMoveBgmLoop, PostMoveBgmLoop);// 変更後のBGMのループフラグ
	CC_SYNTHESIZE(EnumBgmChangeTiming, _postMoveBgmChangeTiming, PostMoveBgmChangeTiming);// BGMの変更や停止のタイミング

	CC_SYNTHESIZE(bool, _postMovePlaySe, PostMovePlaySe);// SE再生フラグ
	CC_SYNTHESIZE(int, _postMoveSeId, PostMoveSeId);// 再生するSEのID

													// ▼その他
	CC_SYNTHESIZE(bool, _postMoveChangeSwitchVariable, PostMoveChangeSwitchVariable);// スイッチ・変数を変更のフラグ
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _postMoveSwitchVariableAssignList, PostMoveSwitchVariableAssignList);// スイッチ・変数を変更のデータリスト (ObjectActionData)

	CC_SYNTHESIZE(bool, _postMoveExecuteObjectAction, PostMoveExecuteObjectAction);// オブジェクトのアクションプログラムの実行フラグ
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _postMoveObjectActionList, PostMoveObjectActionList);// オブジェクトのアクションプログラムのデータリスト (ObjectActionData)
};

//-------------------------------------------------------------------------------------------------------------------
//! ポータル設定データ
class AGTKPLAYER_API MoveSettingData : public data::BaseSceneChangeEffectData
{
public:
	/* 移動開始の条件 */
    enum EnumConditionType {
        kConditionTypeAnyPlayerTouched,//プレイヤーが1人でも触れた
        kConditionTypeAllPlayersTouched,//全てのプレイヤーが触れた
        kConditionTypeAnyPlayerTouchedAndDurationPassed,//プレイヤーが1人でも触れて一定時間が経過
        kConditionTypeAllPlayersTouchedAndDurationPassed,//全てのプレイヤーが触れて一定時間が経過
    };

protected:
	MoveSettingData();
	virtual ~MoveSettingData();

public:
	CREATE_FUNC_PARAM3(MoveSettingData, const rapidjson::Value&, json, int, id, int, abId);
#if defined(AGTK_DEBUG)
	void dump();
#endif

private:
	virtual bool init(const rapidjson::Value& json, int id, int abId);

private:
	// ▼ポータル・入口：移動開始の条件
	CC_SYNTHESIZE(EnumConditionType, _preMoveConditionType, PreMoveConditionType);//移動開始の条件
	CC_SYNTHESIZE(int, _preMoveConditionDuration300, PreMoveConditionDuration300);//移動開始までの時間の設定値
	CC_SYNTHESIZE(bool, _preMoveSwitchVariableCondition, PreMoveSwitchVariableCondition);//「スイッチ、変数が変化」のチェック状態
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _preMoveSwitchVariableConditionList, PreMoveSwitchVariableConditionList);//「スイッチ、変数が変化」の設定データリスト
	
	// ▼ポータル・入口：プレイヤーが触れたときの設定
	CC_SYNTHESIZE(bool, _preMoveInvalidateTouchedPlayer, PreMoveInvalidateTouchedPlayer);//「触れたプレイヤーを無効にする」のチェック状態
	CC_SYNTHESIZE(bool, _preMoveKeepDisplay, PreMoveKeepDisplay);//「表示だけ残す」のチェック状態
	CC_SYNTHESIZE(bool, _preMoveChangeSwitchVariable, PreMoveChangeSwitchVariable);//「スイッチ、変数を変更」のチェック状態
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _preMoveSwitchVariableAssignList, PreMoveSwitchVariableAssignList);//「スイッチ、変数を変更」の設定データリスト
	
	// ▼ポータル・入口：プレイヤーが触れた判定に条件を追加
	CC_SYNTHESIZE(bool, _preMoveKeyInput, PreMoveKeyInput);//「入力操作がされた」のチェック状態
	CC_SYNTHESIZE(int, _preMoveOperationKeyId, PreMoveOperationKeyId);//「入力操作がされた」で設定された操作キーID
	CC_SYNTHESIZE(bool, _preMoveTouchedSwitchVariableCondition, PreMoveTouchedSwitchVariableCondition);//「スイッチ、変数が変化」のチェック状態
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _preMoveTouchedSwitchVariableConditionList, PreMoveTouchedSwitchVariableConditionList);//「スイッチ、変数が変化」の設定データリスト

	// ▼ポータル・出口：ポータルへの移動後の設定
	CC_SYNTHESIZE(bool, _postMoveOnlyTransportPlayersTouching, PostMoveOnlyTransportPlayersTouching);//「Aポータルに触れたプレイヤーのみ移動」のチェック状態
	CC_SYNTHESIZE(bool, _postMoveShowPlayersInTouchedOrder, PostMoveShowPlayersInTouchedOrder);//「Aポータルに最初に触れたプレイヤーから表示」のチェック状態
	CC_SYNTHESIZE(int, _postMoveShowDuration300, PostMoveShowDuration300);//「表示間隔（秒）」の設定値
	CC_SYNTHESIZE(int, _abId, AbId);//A=0, B=1
};

//-------------------------------------------------------------------------------------------------------------------
//! ポータル設置設定データ
class AGTKPLAYER_API AreaSettingData : public cocos2d::Ref
{
private:
	AreaSettingData();
	virtual ~AreaSettingData();

public:
	CREATE_FUNC_PARAM(AreaSettingData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif

private:
	virtual bool init(const rapidjson::Value& json);

private:
	CC_SYNTHESIZE(int, _x, X);//ポータルのX座標(※左上が原点であることに注意)
	CC_SYNTHESIZE(int, _y, Y);//ポータルのY座標(※左上が原点であることに注意)
	CC_SYNTHESIZE(int, _width, Width);//ポータルの横幅
	CC_SYNTHESIZE(int, _height, Height);//ポータルの縦幅
	CC_SYNTHESIZE(int, _sceneId, SceneId);//配置シーンID
	CC_SYNTHESIZE(int, _layerIndex, LayerIndex);//配置レイヤーIDX
	CC_SYNTHESIZE(bool, _keepHorzPosition, KeepHorzPosition);//移動先でX方向の位置をあわせるフラグ
	CC_SYNTHESIZE(bool, _keepVertPosition, KeepVertPosition);//移動先でY方向の位置をあわせるフラグ
	CC_SYNTHESIZE(int, _rePortalDirectionBit, RePortalDirectionBit);//出現直後の再移動設定
};

//-------------------------------------------------------------------------------------------------------------------
//! ポータル移動データクラス
class AGTKPLAYER_API TransitionPortalData : public cocos2d::Ref
{
public:
	/* ポータルタイプ */
	enum EnumPortalType {
		A,
		B,
		MAX
	};

private:
	TransitionPortalData();
	virtual ~TransitionPortalData();

public:
	CREATE_FUNC_PARAM(TransitionPortalData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif

private:
	virtual bool init(const rapidjson::Value& json);

private:
	CC_SYNTHESIZE(int, _id, Id);// ポータルID
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);// ポータル名
	CC_SYNTHESIZE(bool, _reverseMoveSettingSame, ReverseMoveSettingSame);//「B→Aも同じ設定」のフラグ
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _areaSettingList, AreaSettingList);// 設定データリスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _movableList, MovableList);// ポータルの移動可否フラグリスト([0]:A→Bの移動可否, [1]:B→Aの移動可否)
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _moveSettingList, MoveSettingList);// ポータルの移動設定データリスト([0]:A→Bの移動設定データ, [1]:B→Aの移動設定データ)

	CC_SYNTHESIZE(bool, _folder, Folder);// フォルダフラグ
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _children, Children);// 子のポータル移動データクラス

};

//-------------------------------------------------------------------------------------------------------------------
//! 画面フロー条件のスイッチ・変数が変更の設定データクラス
class AGTKPLAYER_API SwitchVariableConditionData : public cocos2d::Ref
{
public:
	/* 比較演算子タイプ */
	enum EnumCompareOperator {
		kCompareSmaller,//「＜」
		kCompareSmallerEqual,//「＜＝」
		kCompareEqual,//「＝」
		kCompareLargerEqual,//「＞＝」
		kCompareLarger,//「＞」
		kCompareNotEqual//「!＝」
	};

	/* 比較対象の値のタイプ */
    enum EnumCompareValueType {
        kCompareValue,// 定数
        kCompareVariable,//他の変数
        kCompareNan,//数値以外(非数)
    };

	/* スイッチの条件 */
	enum EnumSwitchValueType {
		kSwitchConditionOn,
		kSwitchConditionOff,
		kSwitchConditionOffFromOn,
		kSwitchConditionOnFromOff,
	};

private:
	SwitchVariableConditionData();
	virtual ~SwitchVariableConditionData();

public:
	CREATE_FUNC_PARAM(SwitchVariableConditionData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif

private:
	virtual bool init(const rapidjson::Value& json);

private:
	CC_SYNTHESIZE(bool, _swtch, Swtch);//比較対象がスイッチかのフラグ
	// ▼スイッチを条件に設定
	CC_SYNTHESIZE(int, _switchObjectId, SwitchObjectId);//比較先が「スイッチの場合」の対象オブジェクトID
	CC_SYNTHESIZE(int, _switchQualifierId, SwitchQualifierId);//比較先がスイッチの場合の対象オブジェクト制限
	CC_SYNTHESIZE(int, _switchId, SwitchId);//比較先が「スイッチ」の場合の対象のスイッチID
	CC_SYNTHESIZE(EnumSwitchValueType, _switchValue, SwitchValue);//比較するスイッチの状態
	// ▼変数を条件に設定
	CC_SYNTHESIZE(int, _variableObjectId, VariableObjectId);//比較元のオブジェクトID
	CC_SYNTHESIZE(int, _variableQualifierId, VariableQualifierId);//比較元のオブジェクト制限
	CC_SYNTHESIZE(int, _variableId, VariableId);//比較元の変数ID
	CC_SYNTHESIZE(EnumCompareOperator, _compareOperator, CompareOperator);//変数を条件に設定時の比較演算子タイプ
	CC_SYNTHESIZE(EnumCompareValueType, _compareValueType, CompareValueType);//変数の比較先のタイプ (0=「定数」, 1=「他の変数」, 2=「数値以外(非数)」)
	CC_SYNTHESIZE(double, _comparedValue, ComparedValue);//比較先が「定数」の場合の定数値
	CC_SYNTHESIZE(int, _comparedVariableObjectId, ComparedVariableObjectId);//比較先が「変数」の場合の対象のオブジェクトID
	CC_SYNTHESIZE(int, _comparedVariableQualifierId, ComparedVariableQualifierId);//比較先が「変数」の場合の対象のオブジェクト制限
	CC_SYNTHESIZE(int, _comparedVariableId, ComparedVariableId);//比較先が「変数」の場合の対象の変数ID
};

//-------------------------------------------------------------------------------------------------------------------
//! スイッチ・変数を変更の設定データクラス
class AGTKPLAYER_API SwitchVariableAssignData : public cocos2d::Ref
{
public:
	/* スイッチの変更タイプ */
	enum EnumSwitchAssignValue {
		kSwitchAssignOn,//ONに変更
		kSwitchAssignOff,//OFFに変更
		kSwitchAssignToggle,//トグル
		kSwitchAssignMax
	};

	/* 変数の計算タイプ */
	enum EnumVariavleAssignValue {
		kVariavleSubstitute,//代入
		kVariableAdd,//加算
		kVariableSub,//減算
		kVariableMul,//乗算
		kVariableDiv,//除算
		kVariableMod,//除算の余りを代入
	};

	/* 変更先の値タイプ */
	enum EnumVariableAssignValueType
	{
		kVariableAssignConstant,//定数
		kVariableAssignVariable,//変数
		kVariableAssignRandom,//乱数
		kVariableAssignScript,//スクリプト
	};

private:
	SwitchVariableAssignData();
	virtual ~SwitchVariableAssignData();

public:
	CREATE_FUNC_PARAM(SwitchVariableAssignData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif
	const char *getAssignScript();

private:
	virtual bool init(const rapidjson::Value& json);

private:
	CC_SYNTHESIZE(bool, _swtch, Swtch);// 変更対象がスイッチかのフラグ
	// ▼スイッチを変更
	CC_SYNTHESIZE(int, _switchId, SwitchId);// 対象スイッチID
	CC_SYNTHESIZE(int, _switchObjectId, SwitchObjectId);// 対象オブジェクトID
	CC_SYNTHESIZE(int, _switchQualifierId, SwitchQualifierId);// 対象オブジェクト制限
	CC_SYNTHESIZE(EnumSwitchAssignValue, _switchValue, SwitchValue);// スイッチの変更値(0:ON, 1:OFF, 2:トグル)
	// ▼変数を変更
	CC_SYNTHESIZE(int, _variableId, VariableId);// 変更元の変数ID
	CC_SYNTHESIZE(int, _variableObjectId, VariableObjectId);// 変更元のオブジェクトID
	CC_SYNTHESIZE(int, _variableQualifierId, VariableQualifierId);// 変更元のオブジェクト制限
	CC_SYNTHESIZE(EnumVariableAssignValueType, _variableAssignValueType, VariableAssignValueType);// 変更先のタイプ(0:定数, 1:変数, 2:乱数, 3:スクリプト)
	CC_SYNTHESIZE(EnumVariavleAssignValue, _variableAssignOperator, VariableAssignOperator);// 変数計算方法
	CC_SYNTHESIZE(double, _assignValue, AssignValue);// 変更先が「定数」の定数値
	CC_SYNTHESIZE(int, _assignVariableId, AssignVariableId);// 変更先が「変数」の対象変数ID
	CC_SYNTHESIZE(int, _assignVariableObjectId, AssignVariableObjectId);// 変更先が「変数」の対象オブジェクトID
	CC_SYNTHESIZE(int, _assignVariableQualifierId, AssignVariableQualifierId);// 変更先が「変数」の対象オブジェクト制限
	CC_SYNTHESIZE(int, _randomMin, RandomMin);// 変更先が「乱数」の乱数最小値
	CC_SYNTHESIZE(int, _randomMax, RandomMax);// 変更先が「乱数」の乱数最大値
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _assignScript, AssignScript);// 変更先が「スクリプト」のスクリプト文字列
};

//-------------------------------------------------------------------------------------------------------------------
//! オブジェクトのアクションプログラムを実行データクラス
class AGTKPLAYER_API ObjectActionExcecData : public cocos2d::Ref
{
private:
	ObjectActionExcecData();
	virtual ~ObjectActionExcecData();

public:
	CREATE_FUNC_PARAM(ObjectActionExcecData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif

private:
	virtual bool init(const rapidjson::Value& json);

private:
	CC_SYNTHESIZE(bool, _actionBox, ActionBox);// アクションボックスにチェックがあるかフラグ(true:アクションボックス / false:コモンアクション)
	CC_SYNTHESIZE(int, _actionId, ActionId);// アクションID
	CC_SYNTHESIZE(int, _commonActionId, CommonActionId);// コモンアクションID
	CC_SYNTHESIZE(int, _objectId, ObjectId);// 実行対象のアクションプログラムを持つオブジェクトのID
	CC_SYNTHESIZE(int, _qualifierId, QualifierId);// 制限ID
};

//-------------------------------------------------------------------------------------------------------------------
//! 画面フロー入力条件データクラス
class AGTKPLAYER_API FlowLinkInputConditionData : public data::ObjectInputConditionData
{
protected:
	FlowLinkInputConditionData() : ObjectInputConditionData() {};
	virtual ~FlowLinkInputConditionData() {};

public:
	CREATE_FUNC_PARAM(FlowLinkInputConditionData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif

protected:
	virtual bool init(const rapidjson::Value& json);

private:
};

//-------------------------------------------------------------------------------------------------------------------
//! 画面フロースクリーンデータクラス
class AGTKPLAYER_API FlowScreenData : public cocos2d::Ref
{
public:
	static const int SCENE_MENU_ID_START = -1;	// START の sceneMenuId
private:
	FlowScreenData();
	virtual ~FlowScreenData();

public:
	CREATE_FUNC_PARAM(FlowScreenData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif

private:
	virtual bool init(const rapidjson::Value& json);

private:
	CC_SYNTHESIZE(int, _id, Id);					// フロー画面ID
	CC_SYNTHESIZE(bool, _scene, Scene);				// sceneMenuId にシーンまたはメニューどちらのIDが格納されているかのフラグ [true: シーンIDが格納 / false: メニューIDが格納]
	CC_SYNTHESIZE(int, _sceneMenuId, SceneMenuId);	// シーンまたはメニューのID (-1: START)
};

//-------------------------------------------------------------------------------------------------------------------
//! 画面フローリンクデータクラス
class AGTKPLAYER_API FlowLinkData : public data::BaseSceneChangeEffectData
{
protected:
	FlowLinkData();
	virtual ~FlowLinkData();

public:
	CREATE_FUNC_PARAM(FlowLinkData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif

private:
	virtual bool init(const rapidjson::Value& json);
	bool initConditionGroupList();

private:
	CC_SYNTHESIZE(int, _id, Id);
	// --------------------------------------------------------------------------------------------------------------------
	//! 基本パラメータ
	CC_SYNTHESIZE(int, _priority, Priority);// 判定優先度(数値が大きいほど優先される)
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _screenIdPair, screenIdPair);// リンク元とリンク先のフロー画面ID([0]:リンク元 / [1]:リンク先)

	// --------------------------------------------------------------------------------------------------------------------
	//! 切替後の初期位置パラメータ
	CC_SYNTHESIZE(int, _startPointGroup, StartPointGroup);// スタートポイントのグループ(A:0, B:1, ...)

	// --------------------------------------------------------------------------------------------------------------------
	//! 切替条件のパラメータ
	// ▼画面を切り替える条件
	CC_SYNTHESIZE(EnumChangeConditionType, _changeConditionType, ChangeConditionType);// 切替条件タイプ
	// ▼入力に関する条件設定
	CC_SYNTHESIZE(bool, _noInput, NoInput);// 何も入力されなかったフラグ
	CC_SYNTHESIZE(bool, _useInput, UseInput);// 以下の入力操作がされたフラグ
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _inputConditionList, InputConditionList);// 入力操作の条件リスト
	// ▼その他条件設定
	CC_SYNTHESIZE(bool, _sceneTerminated, SceneTerminated);// 前のシーンが終了したフラグ
	CC_SYNTHESIZE(bool, _timeElapsed, TimeElapsed);// 一定時間が経過のフラグ
	CC_SYNTHESIZE(int, _timeElapsed300, TimeElapsed300);// 一定時間が経過の時間
	CC_SYNTHESIZE(bool, _switchVariableChanged, SwitchVariableChanged);// スイッチ・変数が変化のフラグ
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _switchVariableConditionList, SwitchVariableConditionList);// スイッチ・変数が変化の設定データリスト
	// ▼メモ
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _memo, Memo);// メモ

	//Additional Data
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _inputConditionGroupList, InputConditionGroupList);//->FlowLinkInputConditionData
};

//-------------------------------------------------------------------------------------------------------------------
//! 画面フローデータクラス
class AGTKPLAYER_API TransitionFlowData : public cocos2d::Ref
{
private:
	TransitionFlowData();
	virtual ~TransitionFlowData();

public:
	CREATE_FUNC_PARAM(TransitionFlowData, const rapidjson::Value&, json);
#if defined(AGTK_DEBUG)
	void dump();
#endif
	FlowScreenData *getFlowScreenData(int id);
	FlowLinkData *getFlowLinkData(int id);
private:
	virtual bool init(const rapidjson::Value& json);

private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _flowLinkList, FlowLinkList);// flowLinkDataリスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _flowScreenList, FlowScreenList);// flowScreenDataリスト
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API GameInformationData : public cocos2d::Ref
{
private:
	GameInformationData();
	virtual ~GameInformationData();
public:
	CREATE_FUNC_PARAM(GameInformationData, const rapidjson::Value&, json);
	const char *getTitle();
	const char *getAuthor();
	const char *getGenre();
	const char *getDescription();
	const char *getMainLanguage();
	const char *getInitMainLanguage();
	const char *getLanguage(int id);
	int getLanguageCount();
	int getMainLanguageId();
	int getInitMainLanguageId();
	int getLanguageId(const char* lang);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _title, Title);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _author, Author);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _genre, Genre);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _description, Description);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _mainLanguage, MainLanguage);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _initMainLanguage, InitMainLanguage);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _languageList, LanguageList);//->cocos2d::__String
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API SoundSettingData : public cocos2d::Ref
{
private:
	SoundSettingData();
	virtual ~SoundSettingData();
public:
	CREATE_FUNC_PARAM(SoundSettingData, const rapidjson::Value&, json);
	rapidjson::Value serialize(rapidjson::Document::AllocatorType& allocator);
	bool deserialize(const rapidjson::Value& json);
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(int, _bgmVolume, BgmVolume);
	CC_SYNTHESIZE(int, _seVolume, SeVolume);
	CC_SYNTHESIZE(int, _voiceVolume, VoiceVolume);
	CC_SYNTHESIZE(int, _initBgmVolume, InitBgmVolume);
	CC_SYNTHESIZE(int, _initSeVolume, InitSeVolume);
	CC_SYNTHESIZE(int, _initVoiceVolume, InitVoiceVolume);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ActionProgramSettingData : public cocos2d::Ref
{
private:
	ActionProgramSettingData();
	virtual ~ActionProgramSettingData();
public:
	CREATE_FUNC_PARAM(ActionProgramSettingData, const rapidjson::Value&, json);
	const char *getMemo();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(bool, _autoApart, AutoApart);
	CC_SYNTHESIZE(bool, _gridShow, GridShow);
	CC_SYNTHESIZE(bool, _gridMagnet, GridMagnet);
	CC_SYNTHESIZE(bool, _actionIconHide, ActionIconHide);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _memo, Memo);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API InputMapping : public cocos2d::Ref
{
public:
	//Editor より----------------------------------------------------------------------------------------------------
	enum EnumInput {
		//物理入力
		kInputButton1,   //A Square W
		kInputButton2,   //B Cross  A
		kInputButton3,   //X Circle S
		kInputButton4,   //Y Triangle   D
		kInputButton5,   //LB    L1 Left click
		kInputButton6,   //RB    R1 Right click
		kInputButton7,   //Select
		kInputButton8,   //Start
		kInputButton9,   //Left stick(Press)  SHARE
		kInputButton10,   //Right stick(Press)  OPTIONS
		kInputButton11,   //Up   Left stick(Press)  Up
		kInputButton12,   //Right    Right stick(Press) Right
		kInputButton13,   //Down PS Down
		kInputButton14,   //Left TouchPadButton Left
		kInputButton15,   // Up
		kInputButton16,   // Right
		kInputButton17,   // Down
		kInputButton18,   // Left

		kInputAxis1, //Left stick(X) Left stick(X)
		kInputAxis2, //Left stick(Y) Left stick(Y)
		kInputAxis3, //Right stick(X)    Right stick(X)
		kInputAxis4, //Right stick(Y)    L2
		kInputAxis5, //LT    R2 Middle click
		kInputAxis6, //RT    Right stick(Y)

		//論理入力
		kInputLeftStickUp,    //Left stick(Up)  Wheel(Up)   Mouse
		kInputLeftStickRight,    //Left stick(Right)    Mouse
		kInputLeftStickDown,    //Left stick(Down)  Wheel(Down) Mouse
		kInputLeftStickLeft,    //Left stick(Left)  Mouse
		kInputRightStickUp,    //Right stick(Up)    Mouse
		kInputRightStickRight,    //Right stick(Right)  Mouse
		kInputRightStickDown,    //Right stick(Down)    Mouse
		kInputRightStickLeft,    //Right stick(Left)    Mouse

		kInputMax
	};
private:
	InputMapping();
	virtual ~InputMapping();
public:
	CREATE_FUNC_PARAM(InputMapping, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM(InputMapping, const agtk::data::InputMapping *, inputMapping);
	void copy(const agtk::data::InputMapping *inputMapping);
	rapidjson::Value serialize(rapidjson::Document::AllocatorType& allocator)const;
	bool deserialize(const rapidjson::Value& json);
	bool operator==(const InputMapping& rhs)const;

#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	virtual bool init(const agtk::data::InputMapping *inputMapping);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE(bool, _system, System);
	CC_SYNTHESIZE(int, _keyId, KeyId);
	CC_SYNTHESIZE(int, _custom1Input, Custom1Input);
	CC_SYNTHESIZE(int, _custom2Input, Custom2Input);
	CC_SYNTHESIZE(int, _diInput, DiInput);
	CC_SYNTHESIZE(int, _pcInput, PcInput);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API OperationKey : public cocos2d::Ref
{
public:
private:
	OperationKey();
	virtual ~OperationKey();
public:
	CREATE_FUNC_PARAM(OperationKey, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM(OperationKey, const agtk::data::OperationKey *, operationKey);
	CREATE_FUNC_PARAM2(OperationKey, int, id, const char *, name);
	const char *getName();
	void copy(const agtk::data::OperationKey *operationKey);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	virtual bool init(const agtk::data::OperationKey *operationKey);
	virtual bool init(int id, const char *name);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API InputMappingData : public cocos2d::Ref
{
private:
	InputMappingData();
	virtual ~InputMappingData();
public:
	enum {
		kSystemKeyDisplaySwitch1OperationKeyId = -1,
		kSystemKeyDisplaySwitch2OperationKeyId = -2,
		kSystemKeyDisplaySwitch3OperationKeyId = -3,
		kSystemKeyDisplaySwitch4OperationKeyId = -4,
		kSystemKeyMenuDisplay1OperationKeyId = -5,
		kSystemKeyMenuDisplay2OperationKeyId = -6,
		kSystemKeyMenuDisplay3OperationKeyId = -7,
		kSystemKeyMenuDisplay4OperationKeyId = -8,
		kSystemKeyReset1OperationKeyId = -9,
		kSystemKeyReset2OperationKeyId = -10,
		kSystemKeyReset3OperationKeyId = -11,
		kSystemKeyReset4OperationKeyId = -12,
	};
	CREATE_FUNC_PARAM(InputMappingData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM(InputMappingData, const InputMappingData&, data);
	InputMapping *getInputMapping(int id);
	OperationKey *getOperationKey(int id);
	int getInputMappingCount();
	int getOperationKeyCount();
	rapidjson::Value serialize(rapidjson::Document::AllocatorType& allocator)const;
	bool deserialize(const rapidjson::Value& json);
	void copy(const InputMappingData& data);
	static bool checkSystemKey(int keyId);//システムキーかチェックする。
#if defined(AGTK_DEBUG)
	bool operator==(const InputMappingData& rhs)const;// シリアライズ、デシリアライズ動作確認のため用意
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	virtual bool init(const InputMappingData& data);
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _inputMappingList, InputMappingList);//->InputMapping
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _operationKeyList, OperationKeyList);//->OperationKey
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ObjectActionProgramSettingsData : public cocos2d::Ref
{
private:
	ObjectActionProgramSettingsData();
	virtual ~ObjectActionProgramSettingsData();
public:
	CREATE_FUNC_PARAM(ObjectActionProgramSettingsData, const rapidjson::Value&, json);
	const char *getMemo();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(bool, _autoApart, AutoApart);
	CC_SYNTHESIZE(bool, _gridShow, GridShow);
	CC_SYNTHESIZE(bool, _gridMagnet, GridMagnet);
	CC_SYNTHESIZE(bool, _actionIconHide, ActionIconHide);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _memo, Memo);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API PlayerCharacterData : public cocos2d::Ref
{
private:
	PlayerCharacterData();
	virtual ~PlayerCharacterData();
public:
	CREATE_FUNC_PARAM(PlayerCharacterData, const rapidjson::Value&, json);
	cocos2d::__Array *getPlayerCharacterLine(int line);
	cocos2d::__Array *getPlayerCharacterList(int index);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _player1CharacterList, Player1CharacterList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _player2CharacterList, Player2CharacterList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _player3CharacterList, Player3CharacterList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _player4CharacterList, Player4CharacterList);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API AnimParticleImageData : public cocos2d::Ref
{
public:
	static const int BUILT_IN_PARTICLE_ID_START = 1;	// 組込みパーティクルIDの開始値
	static const int BUILT_IN_PARTICLE_ID_MAX = 31;		// 組込みパーティクルIDの最大数
private:
	AnimParticleImageData();
	virtual ~AnimParticleImageData();
public:
	CREATE_FUNC_PARAM(AnimParticleImageData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM(AnimParticleImageData, int, id);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	virtual bool init(int id);
private:
	CC_SYNTHESIZE(int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _filename, Filename);
};


//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ProjectData : public cocos2d::Ref
{
public:
	enum EnumGameType {
		kGameTypeSideView,//サイドビュー
		kGameTypeTopView,//トップビュー
		kGameTypeMax,
	};
	enum EnumPixelMagnificationType {
		kPixelMagnificationTypeEnlargeByIntegalMultiple,
		kPixelMagnificationTypeUseWindowMaginification,
	};
private:
	ProjectData();
	virtual ~ProjectData();
public:
	CREATE_FUNC_PARAM2(ProjectData, const rapidjson::Value&, json, const std::string &, projectPath);
	const char *getGameIcon();
	cocos2d::Size getTileSize();
	cocos2d::Size getScreenSize();
	cocos2d::Size getSceneSize(SceneData *sceneData);
	bool getScreenEffect(int id, float *value);
	int getScreenEffectCount();
public:
	SceneData *getSceneData(int id);
	SceneData *getSceneDataByName(const char *name);
	SceneData *getMenuSceneData();
	TilesetData *getTilesetData(int id);
	TilesetData *getTilesetDataByName(const char *name);
	FontData *getFontData(int id);
	FontData *getFontDataByName(const char *name);
	ImageData *getImageData(int id);
	ImageData *getImageDataByName(const char *name);
	TextData *getTextData(int id);
	TextData *getTextDataByName(const char *name);
	MovieData *getMovieData(int id);
	MovieData *getMovieDataByName(const char *name);
	BgmData *getBgmData(int id);
	BgmData *getBgmDataByName(const char *name);
	SeData *getSeData(int id);
	SeData *getSeDataByName(const char *name);
	VoiceData *getVoiceData(int id);
	VoiceData *getVoiceDataByName(const char *name);
	VariableData *getVariableData(int id);
	SwitchData *getSwitchData(int id);
	AnimationData *getAnimationData(int id);
	AnimationData *getAnimationDataByName(const char *name);
	AnimationOnlyData *getAnimationOnlyData(int id);
	ObjectData *getObjectData(int id);
	ObjectData *getObjectDataByName(const char *name);
	TransitionPortalData *getTransitionPortalData(int id);
	TransitionPortalData *getTransitionPortalDataByName(const char *name);
	DatabaseData *getDatabaseData(int id);
	DatabaseData *getDatabaseDataByName(const char *name);
	static bool isNumber(const char *str, int *pNum);
	static bool isHexNumber(const char *str, int *pNum);
	static std::vector<std::string> stringSplit(const std::string &text, char delimiter);
	std::string getExpandedText(const char *locale, const std::string &text, std::list<int> &textIdList);
#ifdef USE_PREVIEW
	void setObjectData(int id, const rapidjson::Value& json);
	void setSceneData(int id, const rapidjson::Value& json);
	void setTilesetData(int id, const rapidjson::Value& json);
	void setImageData(int id, const rapidjson::Value& json);
	void setTextData(int id, const rapidjson::Value& json);
#endif
	AnimParticleImageData *getParticleImageData(int id);
	PluginData *getPluginData(int id);
	cocos2d::__Array *getPluginArray();
	cocos2d::__Array *getVariableArray();
	cocos2d::__Array *getSwitchArray();
public:
	cocos2d::__Array *getAnimationDataAllKeys();
	void getPortalDataList(cocos2d::__Array * portalDataList, int sceneId, int scenelayerId, cocos2d::__Array *out);
#if defined(AGTK_DEBUG)
	void dump();
#endif
public:
	cocos2d::__Array *getObjectArray();

	rapidjson::Document serializeConfig()const;
	void deserializeConfig(const rapidjson::Document& doc);

protected:
	virtual bool init(const rapidjson::Value& json, const std::string &projectPath);
	FontData *getFontData(int id, cocos2d::__Dictionary *children);
	ImageData *getImageData(int id, cocos2d::__Dictionary *children);
	MovieData *getMovieData(int id, cocos2d::__Dictionary *children);
	BgmData *getBgmData(int id, cocos2d::__Dictionary *children);
	SeData *getSeData(int id, cocos2d::__Dictionary *children);
	VoiceData *getVoiceData(int id, cocos2d::__Dictionary *children);
	VariableData *getVariableData(int id, cocos2d::__Dictionary *children);
	SwitchData *getSwitchData(int id, cocos2d::__Dictionary *children);
	AnimationData *getAnimationData(int id, cocos2d::__Dictionary *children);
	AnimParticleImageData *getParticleImageData(int id, cocos2d::__Dictionary *children);
	PluginData *getPluginData(int id, cocos2d::__Dictionary *children);
	DatabaseData *getDatabaseData(int id, cocos2d::__Dictionary *children);
	bool splitTextByTag(const std::string &text, std::string &before, std::string &tag, std::string &after);
protected:
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _gameIcon, GameIcon);
	CC_SYNTHESIZE(EnumGameType, _gameType, GameType);
	CC_SYNTHESIZE(int, _playerCount, PlayerCount);
// #AGTK-NX, #AGTK-WIN
#ifdef USE_30FPS
	CC_SYNTHESIZE(bool, _mode30Fps, Mode30Fps);	//プロジェクト設定の「30FPSモード」
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(USE_NX60FPS)
#endif
	CC_SYNTHESIZE(int, _tileWidth, TileWidth);
	CC_SYNTHESIZE(int, _tileHeight, TileHeight);
	CC_SYNTHESIZE(int, _screenWidth, ScreenWidth);
	CC_SYNTHESIZE(int, _screenHeight, ScreenHeight);
	CC_SYNTHESIZE(int, _screenSettings, ScreenSettings);
	CC_SYNTHESIZE(int, _initScreenSettings, InitScreenSettings);
	CC_SYNTHESIZE(bool, _magnifyWindow, MagnifyWindow);
	CC_SYNTHESIZE(bool, _initMagnifyWindow, InitMagnifyWindow);
	CC_SYNTHESIZE(int, _windowMagnification, WindowMagnification);
	CC_SYNTHESIZE(int, _initWindowMagnification, InitWindowMagnification);
	CC_SYNTHESIZE(bool, _adjustPixelMagnification, AdjustPixelMagnification);
	CC_SYNTHESIZE(int, _pixelMagnificationType, PixelMagnificationType);
	CC_SYNTHESIZE(bool, _displayMenuBar, DisplayMenuBar);
	CC_SYNTHESIZE(bool, _displayMousePointer, DisplayMousePointer);
	CC_SYNTHESIZE(int, _loadingSceneId, LoadingSceneId);
	CC_SYNTHESIZE(float, _wallDetectionOverlapMargin, WallDetectionOverlapMargin);
	CC_SYNTHESIZE(bool, _multithreading, Multithreading);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	//todo screenEffectFlagListの読み込みが必要
	CC_SYNTHESIZE_RETAIN(GameInformationData *, _gameInformation, GameInformation);//->GameInformationData
	CC_SYNTHESIZE_RETAIN(SoundSettingData *, _soundSetting, SoundSetting);//->SoundSetting
	CC_SYNTHESIZE_RETAIN(ActionProgramSettingData *, _actionProgramSetting, ActionProgramSetting);//->ActionProgramSettingData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _screenEffectValueList, ScreenEffectValueList);//->cocos2d::Double
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _screenEffectFlagList, ScreenEffectFlagList);//->cocos2d::Bool
	CC_SYNTHESIZE_RETAIN(InputMappingData *, _inputMapping, InputMapping);//->InputMappingData
	CC_SYNTHESIZE_RETAIN(InputMappingData *, _initInputMapping, InitInputMapping);//->InputMappingData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _sceneList, SceneList);//->SceneData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _tilesetList, TilesetList);//->TilesetData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _fontList, FontList);//->FontData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _imageList, ImageList);//->ImageData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _movieList, MovieList);//->MovieData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _bgmList, BgmList);//->BgmData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _seList, SeList);//->SeData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _voiceList, VoiceList);//->VoiceData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _variableList, VariableList);//->VariableData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _switchList, SwitchList);//->SwitchData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _textList, TextList);//->TextData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _animationOnlyList, AnimationOnlyList);//->AnimationOnlyData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _objectList, ObjectList);//->ObjectData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _animationList, AnimationList);//->AnimationData
	CC_SYNTHESIZE_RETAIN(ObjectActionProgramSettingsData *, _objectActionProgramSettings, ObjectActionProgramSettings);//->ObjectActionProgramSettingsData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _animParticleImageList, AnimParticleImageList);
	CC_SYNTHESIZE_RETAIN(TransitionFlowData *, _transitionFlow, TransitionFlow);//->TransitionFlowData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array*, _transitionPortalList, TransitionPortalList);//->TransitionPortalList
	CC_SYNTHESIZE_RETAIN(PlayerCharacterData *, _playerCharacterData, PlayerCharacterData);//->PlayerCharacterData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _pluginList, PluginList);//->PluginData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _databaseList, DatabaseList);//->DatabaseData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array*, _objectGroup, ObjectGroup);// オブジェクトのグループ数
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
};

NS_DATA_END
NS_AGTK_END

#endif	//__AGTK_PROJECT_DATA_H__
