#ifndef __AGKT_PLAY_DATA_H__
#define	__AGKT_PLAY_DATA_H__

#include "cocos2d.h"
#include "json/document.h"
#include "Lib/Macros.h"
#include "Lib/Common.h"
#include "Data/AssetData.h"
#include "Data/ProjectData.h"

USING_NS_CC;

NS_AGTK_BEGIN
class Object;

NS_DATA_BEGIN

enum EnumProjectSystemSwitch {
	kProjectSystemSwitchInit = 1,//ゲームプレイ時に、このプロジェクトで使用されている変数、スイッチを初期化することができます。
	kProjectSystemSwitchReset,//ソフトリセットします。
	kProjectSystemSwitchPadding1 = -1,
	kProjectSystemSwitchPadding2 = -1,
	kProjectSystemSwitchPadding3 = -1,
	kProjectSystemSwitchPadding4 = -1,
	kProjectSystemSwitchSaveFile = 6,//指定されたファイルスロットにデータをセーブします。
	kProjectSystemSwitchLoadFile,//指定されたファイルスロットからデータをロードします。
	kProjectSystemSwitchCopyFile,//指定されたファイルスロットのデータを、コピー先ファイルスロットへコピーします。
	kProjectSystemSwitchDeleteFile,//指定されたファイルスロットのデータを削除します。
	kProjectSystemSwitchInitialCamera,//シーンの初期カメラを有効にするためのスイッチ。デフォルトはON。
	kProjectSystemSwitchLoadingScene,//プロジェクト設定で指定したロード画面を表示・非表示を切り替える。
	kProjectSystemSwitchQuitTheGame,//このスイッチをONにするとゲームを終了します。
	kProjectSystemSwitchFileExists,//指定されたファイルスロットにデータがある場合は自動でONになります。”ファイルをセーブ”や”ファイルスロットの切替え”、”ファイルを削除”などにより自動で更新されます。
	kProjectSystemSwitchMax,
};

enum EnumObjectSystemSwitch {
	kObjectSystemSwitchInvincible = 1,//このオブジェクトを無敵状態にします。基本設定のダメージを受けた際の無敵時間の設定が反映されます。
	kObjectSystemSwitchFreeMove,//このオブジェクトが壁判定を無視して移動できるようになります。
	kObjectSystemSwitchLockTarget,//このオブジェクトが他のオブジェクトからロックされている時にONになります。
	kObjectSystemSwitchPortalTouched,//このオブジェクトがポータルに触れている時にONになります。
	kObjectSystemSwitchCriticalDamaged,//このオブジェクトがクリティカルダメージを受けた時にONになります。
	kObjectSystemSwitchDisabled,//このオブジェクトが無効時にONになります。
	kObjectSystemSwitchSlipOnSlope,//移動とジャンプの「坂で滑る」の✔を切替える。
	kObjectSystemSwitchAffectOtherObjects,//他の物理演算オブジェクトに影響を与える。
	kObjectSystemSwitchAffectedByOtherObjects,//他の物理演算オブジェクトから影響を受ける。
	kObjectSystemSwitchFollowConnectedPhysics,//接続されている物理オブジェクトの動作を優先。
	kObjectSystemSwitchDisplayAfterimage,//残像を表示。
	kObjectSystemSwitchMax,
};

enum EnumProjectSystemVariable {
	kProjectSystemVariablePadding1 = -1,
	kProjectSystemVariablePadding2 = -1,
	kProjectSystemVariablePadding3 = -1,
	kProjectSystemVariableAttribute = -1,//予約
	kProjectSystemVariablePadding4 = -1,
	kProjectSystemVariablePlayerCount = 6,//ゲームプレイ時にプレイ人数が代入されます。
	kProjectSystemVariable1PCharacter = 7,//1PキャラクターのオブジェクトIDが代入されます。
	kProjectSystemVariable2PCharacter = 8,//2PキャラクターのオブジェクトIDが代入されます。
	kProjectSystemVariable3PCharacter = 9,//3PキャラクターのオブジェクトIDが代入されます。
	kProjectSystemVariable4PCharacter = 10,//4PキャラクターのオブジェクトIDが代入されます。
	kProjectSystemVariablePortalMoveStartTime = 11,//ポータル移動開始時間の初期値になります。
	kProjectSystemVariableFileSlot = 12,//ゲームデータをセーブ、ロード、削除する際のファイルスロットを設定できます。
	kProjectSystemVariableCopyDestinationFileSlot = 13,//ゲームデータをコピーする際のコピー先のファイルスロットを設定できます。
	kProjectSystemVariable1PInstance = 14,//1PキャラクターのインスタンスIDが代入されます。
	kProjectSystemVariable2PInstance = 15,//2PキャラクターのインスタンスIDが代入されます。
	kProjectSystemVariable3PInstance = 16,//3PキャラクターのインスタンスIDが代入されます。
	kProjectSystemVariable4PInstance = 17,//4PキャラクターのインスタンスIDが代入されます。
	kProjectSystemVariable1PController = 18,//1PキャラクターのコントローラーIDが代入されます。
	kProjectSystemVariable2PController = 19,//2PキャラクターのコントローラーIDが代入されます。
	kProjectSystemVariable3PController = 20,//3PキャラクターのコントローラーIDが代入されます。
	kProjectSystemVariable4PController = 21,//4PキャラクターのコントローラーIDが代入されます。
	kProjectSystemVariableMouseX = 22,//プレイヤーのゲーム画面上にマウスがある場合0以上の値が代入される。-1で画面外
	kProjectSystemVariableMouseY = 23,//プレイヤーのゲーム画面上にマウスがある場合0以上の値が代入される。-1で画面外
	kProjectSystemVariableBgmVolumeAdjust   = 24,//ゲームプレイ中に変数を変更することでBGMの音量の調整ができます。
	kProjectSystemVariableSeVolumeAdjust    = 25,//ゲームプレイ中に変数を変更することでSEの音量の調整ができます。
	kProjectSystemVariableVoiceVolumeAdjust = 26,//ゲームプレイ中に変数を変更することで音声の音量の調整ができます。
	kProjectSystemVariableMax,
};

enum EnumObjectSystemVariable {
	kObjectSystemVariableObjectID = 1,//オブジェクトIDが格納されます。ユーザによる変更は不可能です。
	kObjectSystemVariableHP = 2,//基本設定の体力の値が代入されます。
	kObjectSystemVariableMaxHP = 3,//基本設定の最大体力の値が代入されます。
	kObjectSystemVariableMinimumAttack = 4,//基本設定の最小攻撃力の値が代入されます。
	kObjectSystemVariableMaximumAttack = 5,//基本設定の最大攻撃力の値が代入されます。
	kObjectSystemVariableDamageRatio = 6,//基本設定の被ダメージ率の値が代入されます。
	kObjectSystemVariableAttackAttribute = 7,//このオブジェクトに設定されている攻撃属性の値が代入されます。
	kObjectSystemVariableAreaAttribute = 8,//このオブジェクトがいるエリアの値が代入されます。
	kObjectSystemVariableX = 9,//毎フレームこのオブジェクトのシーンに対するX座標位置が代入されます。
	kObjectSystemVariableY = 10,//毎フレームこのオブジェクトのシーンに対するY座標位置が代入されます。
	kObjectSystemVariableVelocityX = 11,//毎フレームこのオブジェクトのX方向移動量が代入されます。
	kObjectSystemVariableVelocityY = 12,//毎フレームこのオブジェクトのY方向移動量が代入されます。
	kObjectSystemVariablePlayerID = 13,//プレイヤーが操作するオブジェクトの場合、ゲーム中に操作しているプレイヤーの値(1～4まで)が代入されます。
	kObjectSystemVariableDamageValue = 14,//このオブジェクトが受けた被ダメージ値が代入されます。
	kObjectSystemVariableCriticalRatio = 15,//基本設定のクリティカル倍率の値が代入されます。
	kObjectSystemVariableCriticalIncidence = 16,//基本設定のクリティカル発生率の値が代入されます。
	kObjectSystemVariableInvincibleDuration = 17,//基本設定の被ダメージ時の無敵時間の値が代入されます。
	kObjectSystemVariableFixedAnimationFrame = 18,//この変数に代入されている値で、アニメの表示フレームを固定します。(-1で通常再生)
	kObjectSystemVariableInstanceID = 19,//このオブジェクトがシーンに配置された際のインスタンスIDが格納されます。
	kObjectSystemVariableInstanceCount = 20,//シーンに配置された同インスタンスの数が格納されます。
	kObjectSystemVariableSingleInstanceID = 21,//スイッチ・変数参照時、単体を指定する際に使用。単体として指定されている同オブジェクトのインスタンスのインスタンスIDが格納される。
	kObjectSystemVariableControllerID = 22,//このオブジェクトのインスタンスを操作するコントローラーIDが代入される。
	kObjectSystemVariableHorizontalMove = 23,//移動とジャンプの「左右の移動量」の数値を代入・変更可能。
	kObjectSystemVariableVerticalMove = 24,//移動とジャンプの「上下の移動量」の数値を代入・変更可能。
	kObjectSystemVariableHorizontalAccel = 25,//移動とジャンプの「左右の加速量」の数値を代入・変更可能。
	kObjectSystemVariableVerticalAccel = 26,//移動とジャンプの「上下の加速量」の数値を代入・変更可能。
	kObjectSystemVariableHorizontalMaxMove = 27,//移動とジャンプの「左右の最大移動量」の数値を代入・変更可能。
	kObjectSystemVariableVerticalMaxMove = 28,//移動とジャンプの「上下の最大移動量」の数値を代入・変更可能。
	kObjectSystemVariableHorizontalDecel = 29,//移動とジャンプの「左右の減速量」の数値を代入・変更可能。
	kObjectSystemVariableVerticalDecel = 30,//移動とジャンプの「上下の減速量」の数値を代入・変更可能。
	kObjectSystemVariableDurationToTakeOverAccelerationMoveSpeed = 31,//アクション切り替え時に加速移動速度を引き継ぐ時間。
	kObjectSystemVariableScalingX = 32,//オブジェクトのX方向の拡縮を格納(%)
	kObjectSystemVariableScalingY = 33,//オブジェクトのY方向の拡縮を格納(%)
	kObjectSystemVariableDispPriority = 34,//値が大きいほどレイヤー内で手前に表示されます。
	kObjectSystemVariableInitialJumpSpeed = 35,//このオブジェクトのジャンプの初速。
	kObjectSystemVariableDamageVariationValue = 36,//基本設定の変動値の値が代入されます。
	kObjectSystemVariableDisplayDirection = 37,//オブジェクトの表示方向が0～359の数値で代入されます。
	kObjectSystemVariableParentObjectInstanceID = 38,//このオブジェクトの親オブジェクトのインスタンスIDが自動で格納されます。このオブジェクトが親子関係に無い場合は ” - 1” が格納されます。
	kObjectSystemVariableMax,
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API PlaySwitchData : public cocos2d::Ref
{
public:
	enum EnumState {
		kStateOnFromOff,
		kStateOffFromOn,
		kStateNoChange,
		kStateMax,
	};
	enum EnumType {
		kTypeNormal,//通常
		kTypeExternalSupport,//外部入力サポート
	};
private:
	PlaySwitchData();
	virtual ~PlaySwitchData();
public:
	CREATE_FUNC_PARAM(PlaySwitchData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM(PlaySwitchData, agtk::data::SwitchData *, switchData);
	rapidjson::Value json(rapidjson::Document::AllocatorType& allocator);
	void reset();
	void initValue(bool value);
	bool setValue(bool value);
	bool setExternalValue(bool value);
	void resetExternalValue();
	bool checkChangeValue() { return _value != _valueOld; }
	bool getValue() { return _value; }
	EnumState isState();
	void requestValue(bool value, float delta);
#if defined(AGTK_DEBUG)
	void dump();
#endif
	void update(float delta);
	typedef std::function<void(int, bool, void *)> ChangeCallbackFuncType;
	int registerChangeCallback(ChangeCallbackFuncType func, void *arg);
	void unregisterChangeCallback(int key);
private:
	virtual bool init(const rapidjson::Value& json);
	virtual bool init(agtk::data::SwitchData *switchData);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE(EnumType, _type, Type);
	CC_SYNTHESIZE(bool, _readOnly, ReadOnly);
	bool _value;
	bool _valueOld;
	bool _valueExternal;
	bool _valueExternalFlag;
	CC_SYNTHESIZE_RETAIN(agtk::data::SwitchData *, _switchData, SwitchData);
	struct {
		bool flag;
		bool value;
		float delta;
		float seconds;
	} _requestData;//登録リクエスト
	class ChangeCallbackInfo {
	public:
		ChangeCallbackInfo(ChangeCallbackFuncType a_func, void *a_arg, int a_key) : func(a_func), arg(a_arg), key(a_key){}
		ChangeCallbackFuncType func;
		void *arg;
		int key;
	};
	std::list<ChangeCallbackInfo> _changeCallbackInfoList;
	int _changeCallbackKeyGen;
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_4
private:
	static std::function<void(agtk::data::PlaySwitchData*)> _setValueCallback;
	static bool _svcArg_isObjectSwitch;
public:
	static void setSetValueCallback(std::function<void(agtk::data::PlaySwitchData*)> callback);
	static void setSetValueCallbackArg(bool isObjectSwitch);
	static bool getSetValueCallbackArg_isObjectSwitch();
#endif
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API PlayVariableData : public cocos2d::Ref
{
public:
	enum EnumType {
		kTypeNormal,//通常
		kTypeExternalSupport,//外部入力サポート
	};
private:
	PlayVariableData();
	virtual ~PlayVariableData();
public:
	CREATE_FUNC_PARAM(PlayVariableData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM(PlayVariableData, agtk::data::VariableData *, variableData);
	rapidjson::Value json(rapidjson::Document::AllocatorType& allocator);
	void reset();
	double setValue(double value);
	double setExternalValue(double value);
	void resetExternalValue();
	bool checkChangeValue() { return _value != _valueOld; }
	double getValue() { return _value; }
	bool isExternalValue() { return _valueExternalFlag; }
	void requestValue(double value, float delta);
	void update(float delta);
	bool isSetValue() { return _firstSetValueFlag; }
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	virtual bool init(agtk::data::VariableData *variableData);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE(EnumType, _type, Type);
	CC_SYNTHESIZE(bool, _readOnly, ReadOnly);
	double _value;
	double _valueOld;
	double _valueExternal;
	bool _valueExternalFlag;
	CC_SYNTHESIZE_RETAIN(agtk::data::VariableData *, _variableData, VariableData);
	struct {
		bool flag;
		double value;
		float delta;
		float seconds;
	} _requestData;//登録リクエスト
	CC_SYNTHESIZE(agtk::Object *, _object, Object);	// ACT2-5331 kObjectSystemVariableFixedAnimationFrameが-2に設定されたときに更新を行うために使用。weak ptr.
	bool _firstSetValueFlag;//最初に値を設定したフラグ
	CC_SYNTHESIZE(double, _valueFirst, ValueFirst);//最初に設定した値。
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API PlayObjectData : public cocos2d::Ref
{
private:
	PlayObjectData();
	virtual ~PlayObjectData();
public:
	CREATE_FUNC_PARAM(PlayObjectData, const rapidjson::Value&, json);
	CREATE_FUNC_PARAM(PlayObjectData, agtk::data::ObjectData *, objectData);
	void setup(agtk::data::ObjectData *objectData);
	rapidjson::Value json(rapidjson::Document::AllocatorType& allocator);
public:
	PlaySwitchData *getSwitchData(int id);
	PlaySwitchData *getSwitchDataByName(const char *name);
	PlayVariableData *getVariableData(int id);
	PlayVariableData *getVariableDataByName(const char *name);
	void takeOverVariableList(PlayObjectData *playObjectData);
	void takeOverSwitchList(PlayObjectData *playObjectData);
	void setInitSystemSwitchData(EnumObjectSystemSwitch id, bool value);
	void setInitSystemVariableData(EnumObjectSystemVariable id, double value);
	bool setSystemSwitchData(EnumObjectSystemSwitch id, bool value);
	double setSystemVariableData(EnumObjectSystemVariable id, double value);
	void setInstanceId(int id);//PlayVariableDataのkObjectSystemVariableInstanceIDに値を設定する。
	int getInstanceId();//PlayVariableDataのkObjectSystemVariableInstanceIDから値を取得する。
	void setControllerId(int id);//PlayerVariableDataのkObjectSystemVariableControllerIDに値を設定する。
	int getControllerId();//PlayerVariableDataのkObjectSystemVariableControllerIDから値を取得する。
// #AGTK-NX #AGTK-WIN
#ifdef USE_SAR_PROVISIONAL_1
	void setInitControllerId(int id); //PlayerVariableDataのkObjectSystemVariableControllerIDに値を初期値と共に設定する。
#endif
	void setPlayerId(int id);//PlayerVariableData::kObjectSystemVariablePlayerIDに値を設定する。
	int getPlayerId();//PlayerVariableData::kObjectSystemVariablePlayerIDから値を取得する。
	void setAttackAttribute(int id);
	int getAttackAttribute();
	void setHp(double hp);
	double getHp();
	double addHp(double hp);
	int setInstanceCount(int count);//シーンに配置された同インスタンス数を設定する。
	int getInstanceCount();//シーンに配置された同インスタンス数を取得する。
	bool setLockTarget(bool value);//kObjectSystemSwitchLockTargetに値を設定する。
	bool setPortalTouched(bool value);//kObjectSystemSwitchPortalTouchedに値を設定する。
	void reset(bool bExcludeInstanceId = false);
	void adjustSwitchData(PlaySwitchData *switchData);
	void adjustVariableData(PlayVariableData *variableData);
	void adjustData();
	void update(float delta);
	bool isHitObjectGroup(int group)const;
	void setParentObjectInstanceId(int id);//PlayVariableDataのkObjectSystemVariableParentObjectInstanceIDに値を設定する。
	int getParentObjectInstanceId();//PlayVariableDataのkObjectSystemVariableParentObjectInstanceIDから値を取得する。
	void clearObjectReference();	//このプレイデータ中のオブジェクト参照をクリアする。（このプレイデータに紐づくオブジェクトが終了化する際に呼び出される想定。）
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	virtual bool init(agtk::data::ObjectData *objectData);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE(unsigned int, _objectId, ObjectId);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _switchList, SwitchList);//->PlaySwitchData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _variableList, VariableList);//->PlayVariableData
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectData *, _objectData, ObjectData);

	CC_SYNTHESIZE(int, _hitObjectGroupBit, HitObjectGroupBit); // 当たるオブジェクトグループ
	CC_SYNTHESIZE(int, _hitTileGroupBit, HitTileGroupBit); // 当たるタイルグループ

private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _lockingObjectIdList, LockingObjectIdList);//自身をロックしているオブジェクトIDリスト
public:
	bool getLock() { return (_lockingObjectIdList && _lockingObjectIdList->count() > 0); }
	bool isLocked(int id);
	void addLocking(int id);
	void removeLocking(int id);
#ifdef USE_SAR_OPTIMIZE_2
public:
	CC_SYNTHESIZE(std::vector<PlaySwitchData*>, _switchArray, SwitchArray);
	CC_SYNTHESIZE(std::vector<PlayVariableData*>, _variableArray, VariableArray);
#endif
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API PlayData : public cocos2d::Ref
{
public:
	static const int COMMON_PLAY_DATA_ID = 0;	// 共通プレイデータID
private:
	PlayData();
	virtual ~PlayData();
public:
	CREATE_FUNC_PARAM(PlayData, agtk::data::ProjectData *, projectData);
	void loadData(const rapidjson::Value& json,bool sw = true, bool var = true,bool obj = true);
	rapidjson::Value saveData(rapidjson::Document::AllocatorType& allocator);
public:
	agtk::data::PlaySwitchData *getCommonSwitchData(int id);
	agtk::data::PlayVariableData *getCommonVariableData(int id);
	agtk::data::PlayVariableData *getCommonVariableDataByName(const char *name);
	agtk::data::PlayObjectData *getObjectData(int id);
public:
	agtk::data::PlaySwitchData *getSwitchData(int objectId, int id);
	agtk::data::PlayVariableData *getVariableData(int objectId, int id);
	agtk::data::PlayVariableData *getVariableDataByName(int objectId, const char *name);
#if defined(AGTK_DEBUG)
	void dump();
#endif
	void reset(bool bExcludeInstanceId = false);
	void adjustCommonSwitchData(agtk::data::PlaySwitchData *switchData);
	void adjustCommonVariableData(agtk::data::PlayVariableData *variableData);
	void adjustSwitchData(int objectId, agtk::data::PlaySwitchData *switchData);
	void adjustVariableData(int objectId, agtk::data::PlayVariableData *variableData);
	void adjustData();
	void update(float delta);
private:
	virtual bool init(agtk::data::ProjectData *projectData);
	void setup(agtk::data::ProjectData *projectData);
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _commonSwitchList, CommonSwitchList);//->PlaySwitchData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _commonVariableList, CommonVariableList);//->PlayVariableData
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _objectList, ObjectList);//->PlayObjectData
	CC_SYNTHESIZE_RETAIN(agtk::data::ProjectData *, _projectData, ProjectData);
#ifdef USE_SAR_OPTIMIZE_2
public:
	CC_SYNTHESIZE(std::vector<PlaySwitchData*>, _commonSwitchArray, CommonSwitchArray);
	CC_SYNTHESIZE(std::vector<PlayVariableData*>, _commonVariableArray, CommonVariableArray);
#endif
};

NS_DATA_END

//-------------------------------------------------------------------------------------------------------------------
class Object;
class AGTKPLAYER_API ObjectTakeoverStatesData : public cocos2d::Ref
{
private:
	ObjectTakeoverStatesData();
	virtual ~ObjectTakeoverStatesData();
public:
	CREATE_FUNC_PARAM(ObjectTakeoverStatesData, agtk::Object *, object);
	CREATE_FUNC_PARAM(ObjectTakeoverStatesData, agtk::data::PlayObjectData *, playObjectData);
#if defined(AGTK_DEBUG)
	void dump();
#endif

private:
	virtual bool init(agtk::Object *object);
	virtual bool init(agtk::data::PlayObjectData *playObjectData);
private:
	CC_SYNTHESIZE(int, _sceneId, SceneId);
	CC_SYNTHESIZE(int, _sceneLayerId, SceneLayerId);
	CC_SYNTHESIZE(unsigned int, _scenePartsId, ScenePartsId);
	CC_SYNTHESIZE(int, _objectId, ObjectId);
	CC_SYNTHESIZE(int, _actionNo, ActionNo);
	CC_SYNTHESIZE(int, _directionNo, DirectionNo);
	CC_SYNTHESIZE(int, _prevActionNo, PrevActionNo);
	CC_SYNTHESIZE(cocos2d::Vec2, _position, Position);
	CC_SYNTHESIZE(cocos2d::Vec2, _scale, Scale);
	CC_SYNTHESIZE(int, _dispDirection, DispDirection);
	CC_SYNTHESIZE(cocos2d::Vec2, _moveDirection, MoveDirection);
	CC_SYNTHESIZE_RETAIN(agtk::data::PlayObjectData *, _playObjectData, PlayObjectData);
	CC_SYNTHESIZE(int, _takeOverAnimMotionId, TakeOverAnimMotionId);//遷移前のモーションを引き継ぐ時のモーションID
	CC_SYNTHESIZE(cocos2d::Vec2, _objectPosInCamera, ObjectPosInCamera);
	CC_SYNTHESIZE(int, _sceneIdOfFirstCreated, SceneIdOfFirstCreated);//最初に生成されたシーンのID
};

NS_AGTK_END

#endif	//__AGKT_PLAY_DATA_H__
