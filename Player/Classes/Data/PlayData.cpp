/**
* @brief プレイ関連データ
*/
#include "PlayData.h"
#include "json/stringbuffer.h"
#include "json/writer.h"
#include "Lib/Object.h"
#include "Manager/GameManager.h"

NS_AGTK_BEGIN
NS_DATA_BEGIN

//-------------------------------------------------------------------------------------------------------------------
//システム　スイッチ
typedef struct {
	int id;
	char *name;
	bool initialValue;
	bool toBeSaved;
	char *memo;
} StSystemSwitchData;

//プロジェクト共通
static StSystemSwitchData ProjectCommonSwitch[] = {
	{ kProjectSystemSwitchInit, "Init", false, false, "ゲームプレイ時に、このプロジェクトで使用されている変数、スイッチを初期化することができます。" },
	{ kProjectSystemSwitchReset, "Reset", false, false, "ソフトリセットします。" },
	{ kProjectSystemSwitchPadding1, "", false, false, "" },
	{ kProjectSystemSwitchPadding2, "", false, false, "" },
	{ kProjectSystemSwitchPadding3, "", false, false, "" },
	{ kProjectSystemSwitchPadding4, "", false, false, "" },
	{ kProjectSystemSwitchSaveFile, "Save file", false, false, "指定されたファイルスロットにデータをセーブします。" },
	{ kProjectSystemSwitchLoadFile, "Load file", false, false, "指定されたファイルスロットからデータをロードします。" },
	{ kProjectSystemSwitchCopyFile, "Copy file", false, false, "指定されたファイルスロットのデータを、コピー先ファイルスロットへコピーします。" },
	{ kProjectSystemSwitchDeleteFile, "Delete file", false, false, "指定されたファイルスロットのデータを削除します。" },
	{ kProjectSystemSwitchInitialCamera, "Initial camera", true, true, "シーンの初期カメラを有効にするためのスイッチ。デフォルトはON。" },
	{ kProjectSystemSwitchLoadingScene, "Loading scene", true, true, "プロジェクト設定で指定したロード画面を表示・非表示を切り替える。" },
	{ kProjectSystemSwitchQuitTheGame, "Quit the game", false, false, "このスイッチをONにするとゲームを終了します。" },
	{ kProjectSystemSwitchFileExists, "File Exists", false, false, "指定されたファイルスロットにデータがある場合は自動でONになります。”ファイルをセーブ”や”ファイルスロットの切替え”、”ファイルを削除”などにより自動で更新されます。" },
};

//オブジェクト共通
static StSystemSwitchData ObjectCommonSwitch[] = {
	{ kObjectSystemSwitchInvincible, "Invincible", false, true, "このオブジェクトを無敵状態にします。基本設定のダメージを受けた際の無敵時間の設定が反映されます。" },
	{ kObjectSystemSwitchFreeMove, "Free move", false, true, "このオブジェクトが壁判定を無視して移動できるようになります。" },
	{ kObjectSystemSwitchLockTarget, "Lock target", false, true, "このオブジェクトが他のオブジェクトからロックされている時にONになります。" },
	{ kObjectSystemSwitchPortalTouched, "Portal touched", false, true, "このオブジェクトがポータルに触れている時にONになります。" },
	{ kObjectSystemSwitchCriticalDamaged, "Critical damaged", false, true, "このオブジェクトがクリティカルダメージを受けた時にONになります。" },
	{ kObjectSystemSwitchDisabled, "Disabled", false, true, "このオブジェクトが無効時にONになります。" },
	{ kObjectSystemSwitchSlipOnSlope, "Slip on slopes", false, true, "移動とジャンプの「坂で滑る」の✔を切替える" },
	{ kObjectSystemSwitchAffectOtherObjects, "Affect other physics", false, true, "他の物理演算オブジェクトに影響を与える。" },
	{ kObjectSystemSwitchAffectedByOtherObjects, "Affected by other physics", false, true, "他の物理演算オブジェクトから影響を受ける。" },
	{ kObjectSystemSwitchFollowConnectedPhysics, "Follow connected physics", false, true, "接続されている物理オブジェクトの動作を優先。" },
	{ kObjectSystemSwitchDisplayAfterimage, "Display Afterimage", false, true, "残像を表示。" },
};

//システム　変数
typedef struct {
	int id;
	char *name;
	double initialValue;
	bool toBeSaved;
	char *memo;
} StSystemVariableData;

//プロジェクト共通
static StSystemVariableData ProjectCommonVariable[] = {
	{ kProjectSystemVariablePadding1, "", 0.0, false, "" },
	{ kProjectSystemVariablePadding2, "", 0.0, false, "" },
	{ kProjectSystemVariablePadding3, "", 0.0, false, "" },
	{ kProjectSystemVariableAttribute, "", 0.0, false, "" },
	{ kProjectSystemVariablePadding4, "", 0.0, false, "" },
	{ kProjectSystemVariablePlayerCount, "Player count", 0.0, true, "ゲームプレイ時にプレイ人数が代入されます。" },
	{ kProjectSystemVariable1PCharacter, "1P character", -1.0, true, "1PキャラクターのオブジェクトIDが代入されます。" },
	{ kProjectSystemVariable2PCharacter, "2P character", -1.0, true, "2PキャラクターのオブジェクトIDが代入されます。" },
	{ kProjectSystemVariable3PCharacter, "3P character", -1.0, true, "3PキャラクターのオブジェクトIDが代入されます。" },
	{ kProjectSystemVariable4PCharacter, "4P character", -1.0, true, "4PキャラクターのオブジェクトIDが代入されます。" },

	{ kProjectSystemVariable1PInstance, "1P instance", -1.0, false, "1PキャラクターのインスタンスIDが代入されます。" },
	{ kProjectSystemVariable2PInstance, "2P instance", -1.0, false, "2PキャラクターのインスタンスIDが代入されます。" },
	{ kProjectSystemVariable3PInstance, "3P instance", -1.0, false, "3PキャラクターのインスタンスIDが代入されます。" },
	{ kProjectSystemVariable4PInstance, "4P instance", -1.0, false, "4PキャラクターのインスタンスIDが代入されます。" },

	{ kProjectSystemVariable1PController, "1P controller", -1.0, false, "1PキャラクターのコントローラーIDが代入されます。" },
	{ kProjectSystemVariable2PController, "2P controller", -1.0, false, "2PキャラクターのコントローラーIDが代入されます。" },
	{ kProjectSystemVariable3PController, "3P controller", -1.0, false, "3PキャラクターのコントローラーIDが代入されます。" },
	{ kProjectSystemVariable4PController, "4P controller", -1.0, false, "4PキャラクターのコントローラーIDが代入されます。" },

	{ kProjectSystemVariablePortalMoveStartTime, "Portal move start time", 0.0, true, "ポータル移動開始時間の初期値になります。" },
	{ kProjectSystemVariableFileSlot, "File slot", 0.0, true, "ゲームデータをセーブ、ロード、削除する際のファイルスロットを設定できます。" },
	{ kProjectSystemVariableCopyDestinationFileSlot, "Copy destination file slot", 0.0, true, "ゲームデータをコピーする際のコピー先のファイルスロットを設定できます。" },
	{ kProjectSystemVariableMouseX, "Mouse X", -1.0, true, "プレイヤーのゲーム画面上にマウスがある場合0以上の値が代入される。-1で画面外" },
	{ kProjectSystemVariableMouseY, "Mouse Y", -1.0, true, "プレイヤーのゲーム画面上にマウスがある場合0以上の値が代入される。-1で画面外" },

	{ kProjectSystemVariableBgmVolumeAdjust,   "BGM Volume Adjust"  , 100.0, true, "ゲームプレイ中に変数を変更することでBGMの音量の調整ができます。" },
	{ kProjectSystemVariableSeVolumeAdjust,    "SE Volume Adjust"   , 100.0, true, "ゲームプレイ中に変数を変更することでSEの音量の調整ができます。" },
	{ kProjectSystemVariableVoiceVolumeAdjust, "Voice Volume Adjust", 100.0, true, "ゲームプレイ中に変数を変更することで音声の音量の調整ができます。" },

};

//オブジェクト共通
static StSystemVariableData ObjectCommonVariable[] = {
	{ kObjectSystemVariableObjectID, "Object ID", 0.0, true, "オブジェクトIDが格納されます。ユーザによる変更は不可能です。" },
	{ kObjectSystemVariableHP, "HP", 0.0, true, "基本設定の体力の値が代入されます。" },
	{ kObjectSystemVariableMaxHP, "Max HP", 0.0, true, "基本設定の最大体力の値が代入されます。" },
	{ kObjectSystemVariableMinimumAttack, "Minimum attack", 0.0, true, "基本設定の最小攻撃力の値が代入されます。" },
	{ kObjectSystemVariableMaximumAttack, "Maximum attack", 0.0, true, "基本設定の最大攻撃力の値が代入されます。" },
	{ kObjectSystemVariableCriticalRatio, "Critical ratio(%)", 0.0, true, "基本設定のクリティカル倍率の値が代入されます。" },
	{ kObjectSystemVariableCriticalIncidence, "Critical incidence(%)", 0.0, true, "基本設定のクリティカル発生率の値が代入されます。" },
	{ kObjectSystemVariableDamageRatio, "Damage ratio", 0.0, true, "基本設定の被ダメージ率の値が代入されます。" },
	{ kObjectSystemVariableDamageVariationValue, "Amount of Variation used to calculate damage spread", 0.0, true, "基本設定の変動値の値が代入されます。" },
	{ kObjectSystemVariableAttackAttribute, "Attack attribute", 0.0, true, "このオブジェクトに設定されている攻撃属性の値が代入されます。" },
	{ kObjectSystemVariableAreaAttribute, "Area attribute", -1.0, true, "このオブジェクトがいるエリアの値が代入されます。" },
	{ kObjectSystemVariableX, "X", 0.0, true, "毎フレームこのオブジェクトのシーンに対するX座標位置が代入されます。" },
	{ kObjectSystemVariableY, "Y", 0.0, true, "毎フレームこのオブジェクトのシーンに対するY座標位置が代入されます。" },
	{ kObjectSystemVariableDisplayDirection, "Display Direction", 0.0, true, "オブジェクトの表示方向が0～359の数値で代入されます。" },
	{ kObjectSystemVariableParentObjectInstanceID, "Parent Object Instance ID", -1.0, true, "このオブジェクトの親オブジェクトのインスタンスIDが自動で格納されます。" },
	{ kObjectSystemVariableScalingX, "X", 100.0, true, "オブジェクトのX方向の拡縮を格納(%)" },
	{ kObjectSystemVariableScalingY, "Y", 100.0, true, "オブジェクトのY方向の拡縮を格納(%)" },
	{ kObjectSystemVariableVelocityX, "Velocity X", 0.0, true, "毎フレームこのオブジェクトのX方向移動量が代入されます。" },
	{ kObjectSystemVariableVelocityY, "Velocity Y", 0.0, true, "毎フレームこのオブジェクトのY方向移動量が代入されます。" },
	{ kObjectSystemVariableHorizontalMove, "Horizontal move", 0.0, true, "移動とジャンプの「左右の移動量」の数値を代入・変更可能。" },
	{ kObjectSystemVariableVerticalMove, "Vertical move", 0.0, true, "移動とジャンプの「上下の移動量」の数値を代入・変更可能。" },
	{ kObjectSystemVariableHorizontalAccel, "Horizontal accel", 0.0, true, "移動とジャンプの「左右の加速量」の数値を代入・変更可能。" },
	{ kObjectSystemVariableVerticalAccel, "Vertical accel", 0.0, true, "移動とジャンプの「上下の加速量」の数値を代入・変更可能。" },
	{ kObjectSystemVariableHorizontalMaxMove, "Horizontal max move", 0.0, true, "移動とジャンプの「左右の最大移動量」の数値を代入・変更可能。" },
	{ kObjectSystemVariableVerticalMaxMove, "Vertical max move", 0.0, true, "移動とジャンプの「上下の最大移動量」の数値を代入・変更可能。" },
	{ kObjectSystemVariableHorizontalDecel, "Horizontal decel", 0.0, true, "移動とジャンプの「左右の減速量」の数値を代入・変更可能。" },
	{ kObjectSystemVariableVerticalDecel, "Vertical decel", 0.0, true, "移動とジャンプの「上下の減速量」の数値を代入・変更可能。" },
	{ kObjectSystemVariablePlayerID, "Player ID", 0.0, true, "プレイヤーが操作するオブジェクトの場合、ゲーム中に操作しているプレイヤーの値(1～4まで)が代入されます。" },
	{ kObjectSystemVariableDamageValue, "Damage value", 0.0, true, "このオブジェクトが受けた被ダメージ値が代入されます。" },
	{ kObjectSystemVariableInvincibleDuration, "Invincible duration", 0.0, true, "基本設定の被ダメージ時の無敵時間の値が代入されます。" },
	{ kObjectSystemVariableFixedAnimationFrame, "Fixed animation frame", -1.0, true, "この変数に代入されている値で、アニメの表示フレームを固定します。(-1で通常再生)" },
	{ kObjectSystemVariableInstanceID, "Instance ID", 0.0, true, "このオブジェクトがシーンに配置された際のインスタンスIDが格納されます。" },
	{ kObjectSystemVariableInstanceCount, "Instance count", 0.0, true, "シーンに配置された同インスタンスの数が格納されます。" },
	{ kObjectSystemVariableSingleInstanceID, "Single instance ID", 0.0, true, "スイッチ・変数参照時、単体を指定する際に使用。単体として指定されている同オブジェクトのインスタンスのインスタンスIDが格納される。"},
	{ kObjectSystemVariableControllerID, "Controller ID", -1.0, false, "このオブジェクトのインスタンスを操作するコントローラーIDが代入される。" },
	{ kObjectSystemVariableDurationToTakeOverAccelerationMoveSpeed, "Duration to take over acceleration move speed", 0.2, true, "アクション切り替え時に加速移動速度を引き継ぐ時間。" },
	{ kObjectSystemVariableDispPriority, "Display priority", 0, true, "値が大きいほどレイヤー内で手前に表示されます。" },
	{ kObjectSystemVariableInitialJumpSpeed, "Initial jump speed", 0, true, "このオブジェクトのジャンプの初速。" },
};

//-------------------------------------------------------------------------------------------------------------------
PlaySwitchData::PlaySwitchData()
{
	_switchData = nullptr;
	_value = false;
	_valueOld = false;
	_readOnly = false;
	_requestData.flag = false;
	_requestData.delta = 0.0f;
	_requestData.seconds = 0.0f;
	_requestData.value = false;
	_changeCallbackKeyGen = 0;
}

PlaySwitchData::~PlaySwitchData()
{
	CC_SAFE_RELEASE_NULL(_switchData);
}

bool PlaySwitchData::init(const rapidjson::Value& json)
{
	this->setId(json["id"].GetInt());
	_value = json["value"].GetBool();
	_valueOld = json["value"].GetBool();
	_type = kTypeNormal;
	if(json.HasMember("type")) {
		this->setType((EnumType)json["type"].GetInt());
	}
	_valueExternal = 0.0;
	_valueExternalFlag = false;
	_readOnly = false;
	if (json.HasMember("readOnly")) {
		_readOnly = json["readOnly"].GetBool();
	}
	return true;
}

bool PlaySwitchData::init(agtk::data::SwitchData *switchData)
{
	CC_ASSERT(switchData);
	this->setId(switchData->getId());
	this->initValue(switchData->getInitialValue());
	this->setSwitchData(switchData);
	return true;
}

rapidjson::Value PlaySwitchData::json(rapidjson::Document::AllocatorType& allocator)
{
	rapidjson::Value obj(rapidjson::kObjectType);
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	obj.AddMember(rapidjson::Value("id", allocator), this->getId(), allocator);

	auto switchData = this->getSwitchData();
	if (switchData == nullptr) {
		// 現在値を保存
		obj.AddMember(rapidjson::Value("value", allocator), this->getValue(), allocator);
		return obj;
	}
	// ゲームプレイ中のセーブで保存するフラグがONの場合
	if (switchData->getToBeSaved()) {
		// 現在値を保存
		obj.AddMember(rapidjson::Value("value", allocator), this->getValue(), allocator);
	}
	// ゲームプレイ中のセーブで保存するフラグがOFの場合
	else {
		// 初期値を保存
		obj.AddMember(rapidjson::Value("value", allocator), switchData->getInitialValue(), allocator);
	}
	//外部入力
	if (_type == kTypeExternalSupport) {
		obj.AddMember(rapidjson::Value("type", allocator), (int)this->getType(), allocator);
	}
	//リードオンリー
	if (_readOnly) {
		obj.AddMember(rapidjson::Value("readOnly", allocator), _readOnly, allocator);
	}
#else
#endif
	return obj;
}

void PlaySwitchData::reset()
{
	auto switchData = this->getSwitchData();
	if (switchData) {
		this->initValue(switchData->getInitialValue());
	}
	else {
		this->initValue(false);
	}
	_requestData.flag = false;
	_requestData.delta = 0.0f;
	_requestData.seconds = 0.0f;
	_requestData.value = false;
}

void PlaySwitchData::initValue(bool value)
{
	_value = value;
	_valueOld = value;
	_valueExternal = false;
	_valueExternalFlag = false;
}

bool PlaySwitchData::setValue(bool value)
{
	if (_readOnly) {
		//変更不可の変数の値を書き換えられないように。
		return _value;
	}
	_valueOld = _value;
	_value = value;
	if (this->getType() == kTypeExternalSupport) {
		_valueExternal = value;
		_valueExternalFlag = true;
	}
	if (_value != _valueOld) {
		for (auto callbackInfo : _changeCallbackInfoList) {
			callbackInfo.func(_id, value, callbackInfo.arg);
		}
	}
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_4
	if (_setValueCallback) {
		_setValueCallback(this);
	}
#endif
	return _valueOld;
}

bool PlaySwitchData::setExternalValue(bool value)
{
	_valueOld = _value;
	if (this->getType() == kTypeExternalSupport && _valueExternalFlag) {
		value = _valueExternal;
		_valueExternal = false;
		_valueExternalFlag = false;
	}
	_value = value;
	return _valueOld;
}

void PlaySwitchData::resetExternalValue()
{
	if (this->getType() == kTypeExternalSupport && _valueExternalFlag) {
		_valueExternal = false;
		_valueExternalFlag = false;
	}
}

PlaySwitchData::EnumState PlaySwitchData::isState()
{
	if (_valueOld == false && _value == true) {
		return kStateOnFromOff;
	}
	else if (_valueOld == true && _value == false) {
		return kStateOffFromOn;
	}
	CC_ASSERT(_value == _valueOld);
	return kStateNoChange;
}

void PlaySwitchData::requestValue(bool value, float delta)
{
	_requestData.flag = true;
	_requestData.value = value;
	_requestData.delta = delta;
	_requestData.seconds = 0;
}

void PlaySwitchData::update(float delta)
{
	if (_requestData.flag) {
		if (_requestData.delta <= _requestData.seconds) {
			this->setValue(_requestData.value);
			_requestData.flag = false;
			_requestData.delta = 0.0f;
			_requestData.seconds = 0.0f;
			return;
		}
		_requestData.seconds += delta;
	}
}

int PlaySwitchData::registerChangeCallback(ChangeCallbackFuncType func, void *arg)
{
	auto key = _changeCallbackKeyGen++;
	_changeCallbackInfoList.emplace_back(ChangeCallbackInfo(func, arg, key));
	return key;
}

void PlaySwitchData::unregisterChangeCallback(int key)
{
	for (auto it = _changeCallbackInfoList.begin(); it != _changeCallbackInfoList.end();) {
		if (it->key == key) {
			it = _changeCallbackInfoList.erase(it);
			continue;
		}
		it++;
	}
}

#if defined(AGTK_DEBUG)
void PlaySwitchData::dump()
{
	CCLOG("-- PSD --");
	CCLOG("PSD id:%d", this->getId());
	CCLOG("value:%d", this->getValue());
	CCLOG("-- PSD DEF --");
	if (this->getSwitchData()) {
		this->getSwitchData()->dump();
	}
	CCLOG("-------------");
}
#endif

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_4
std::function<void(agtk::data::PlaySwitchData*)> PlaySwitchData::_setValueCallback = nullptr;
bool PlaySwitchData::_svcArg_isObjectSwitch;

void PlaySwitchData::setSetValueCallback(std::function<void(agtk::data::PlaySwitchData*)> callback)
{
	_setValueCallback = callback;
}

void PlaySwitchData::setSetValueCallbackArg(bool isObjectSwitch)
{
	_svcArg_isObjectSwitch = isObjectSwitch;
}

bool PlaySwitchData::getSetValueCallbackArg_isObjectSwitch()
{
	return _svcArg_isObjectSwitch;
}
#endif

//-------------------------------------------------------------------------------------------------------------------
PlayVariableData::PlayVariableData()
{
	_variableData = nullptr;
	_readOnly = false;
	_requestData.flag = false;
	_requestData.delta = 0.0f;
	_requestData.seconds = 0.0f;
	_requestData.value = 0.0;
	_object = nullptr;
	_firstSetValueFlag = false;
}

PlayVariableData::~PlayVariableData()
{
	CC_SAFE_RELEASE_NULL(_variableData);
}

bool PlayVariableData::init(const rapidjson::Value& json)
{
	this->setId(json["id"].GetInt());
	_value = json["value"].GetDouble();
	_valueOld = json["value"].GetDouble();
	_type = kTypeNormal;
	if (json.HasMember("type")) {
		_type = (EnumType)json["type"].GetInt();
	}
	_valueExternal = 0.0;
	_valueExternalFlag = false;
	_readOnly = false;
	if (json.HasMember("readOnly")) {
		_readOnly = json["readOnly"].GetBool();
	}
	return true;
}

bool PlayVariableData::init(agtk::data::VariableData *variableData)
{
	CC_ASSERT(variableData);
	this->setId(variableData->getId());
	_value = variableData->getInitialValue();
	_valueOld = variableData->getInitialValue();
	_valueFirst = variableData->getInitialValue();
	_type = kTypeNormal;
	_valueExternal = 0.0;
	_valueExternalFlag = false;
	this->setVariableData(variableData);
	return true;
}

rapidjson::Value PlayVariableData::json(rapidjson::Document::AllocatorType& allocator)
{
	rapidjson::Value obj(rapidjson::kObjectType);
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	obj.AddMember(rapidjson::Value("id", allocator), this->getId(), allocator);

	auto variableData = this->getVariableData();
	if (variableData == nullptr) {
		// 現在値を保存
		obj.AddMember(rapidjson::Value("value", allocator), this->getValue(), allocator);
		return obj;
	}
	// ゲームプレイ中のセーブで保存するフラグがONの場合
	if (variableData->getToBeSaved()) {
		// 現在値を保存
		obj.AddMember(rapidjson::Value("value", allocator), this->getValue(), allocator);
	}
	// ゲームプレイ中のセーブで保存するフラグがOFFの場合
	else {
		// 初期値を保存
		obj.AddMember(rapidjson::Value("value", allocator), variableData->getInitialValue(), allocator);
	}
	//外部入力
	if (_type == kTypeExternalSupport) {
		obj.AddMember(rapidjson::Value("type", allocator), (int)_type, allocator);
	}
	//リードオンリー
	if (_readOnly) {
		obj.AddMember(rapidjson::Value("readOnly", allocator), _readOnly, allocator);
	}
#else
#endif
	return obj;
}

void PlayVariableData::reset()
{
	auto variableData = this->getVariableData();
	if (_readOnly && (_id == kObjectSystemVariableInstanceID || _id == kObjectSystemVariableSingleInstanceID)) {
		//読込専用かつ（インスタンスIDか単体インスタンスID）の場合、初回設定値を初期値とする。
		_value = this->getValueFirst();
		_valueOld = this->getValueFirst();
	}
	else if (variableData) {
		_value = variableData->getInitialValue();
		_valueOld = variableData->getInitialValue();
	}
	else {
		_value = 0.0;
		_valueOld = 0.0;
	}
	_valueExternal = 0.0;
	_valueExternalFlag = false;
	_requestData.flag = false;
	_requestData.delta = 0.0f;
	_requestData.seconds = 0.0f;
	_requestData.value = 0.0;
}

double PlayVariableData::setValue(double value)
{
	if (_readOnly) {
		//変更不可の変数の値を書き換えられないように。
		return _value;
	}
	_valueOld = _value;
	_value = value;
	if (this->getType() == kTypeExternalSupport) {
		_valueExternal = value;
		_valueExternalFlag = true;
	}
	if (this->getId() == kObjectSystemVariableFixedAnimationFrame && _object) {
		_object->updateFixedFrame(_value);
	}
	if (_firstSetValueFlag == false) {
		_valueFirst = value;
	}
	_firstSetValueFlag = true;//On!
	return _valueOld;
}

double PlayVariableData::setExternalValue(double value)
{
	_valueOld = _value;
	if (this->getType() == kTypeExternalSupport && _valueExternalFlag) {
		value = _valueExternal;
		_valueExternal = 0.0f;
		_valueExternalFlag = false;
	}
	_value = value;
	if (_firstSetValueFlag == false) {
		_valueFirst = value;
	}
	_firstSetValueFlag = true;//On!
	return _valueOld;
}

void PlayVariableData::resetExternalValue()
{
	if (this->getType() == kTypeExternalSupport && _valueExternalFlag) {
		_valueExternal = 0.0f;
		_valueExternalFlag = false;
	}
}

void PlayVariableData::requestValue(double value, float delta)
{
	_requestData.flag = true;
	_requestData.value = value;
	_requestData.delta = delta;
	_requestData.seconds = 0;
}

void PlayVariableData::update(float delta)
{
	if (_requestData.flag) {
		if (_requestData.delta <= _requestData.seconds) {
			this->setValue(_requestData.value);
			_requestData.flag = false;
			_requestData.delta = 0.0f;
			_requestData.seconds = 0.0f;
			return;
		}
		_requestData.seconds += delta;
	}
}

#if defined(AGTK_DEBUG)
void PlayVariableData::dump()
{
	CCLOG("-- PVD --");
	CCLOG("PVD id:%d", this->getId());
	CCLOG("value:%f", this->getValue());
	CCLOG("-- PVD DEF --");
	if (this->getVariableData()) {
		this->getVariableData()->dump();
	}
	CCLOG("-------------");
}
#endif

//-------------------------------------------------------------------------------------------------------------------
PlayObjectData::PlayObjectData()
{
	_switchList = nullptr;
	_variableList = nullptr;
	_hitObjectGroupBit = 0;
	_hitTileGroupBit = 0;
	_objectData = nullptr;
	_objectId = 0;

	_lockingObjectIdList = nullptr;
	this->setLockingObjectIdList(cocos2d::__Array::create());
}

PlayObjectData::~PlayObjectData()
{
	CC_SAFE_RELEASE_NULL(_switchList);
	CC_SAFE_RELEASE_NULL(_variableList);
	CC_SAFE_RELEASE_NULL(_objectData);

	CC_SAFE_RELEASE_NULL(_lockingObjectIdList);
}

bool PlayObjectData::init(const rapidjson::Value& json)
{
	this->setId(json["id"].GetInt());

	auto switchList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["switchList"].Size(); i++) {
		auto playSwitchData = PlaySwitchData::create(json["switchList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(switchList->objectForKey(playSwitchData->getId()) == nullptr);
#endif
		switchList->setObject(playSwitchData, playSwitchData->getId());
#ifdef USE_SAR_OPTIMIZE_2
		this->getSwitchArray().push_back(playSwitchData);
#endif
	}
	this->setSwitchList(switchList);

	auto variableList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["variableList"].Size(); i++) {
		auto playVariableData = PlayVariableData::create(json["variableList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(variableList->objectForKey(playVariableData->getId()) == nullptr);
#endif
		variableList->setObject(playVariableData, playVariableData->getId());
#ifdef USE_SAR_OPTIMIZE_2
		this->getVariableArray().push_back(playVariableData);
#endif
	}
	this->setVariableList(variableList);

	if (json.HasMember("hitObjectGroupBit")) {
		this->setHitObjectGroupBit(json["hitObjectGroupBit"].GetInt());
	}

	return true;
}

bool PlayObjectData::init(agtk::data::ObjectData *objectData)
{
	CC_ASSERT(objectData);
	this->setId(objectData->getId());
	this->setObjectData(objectData);

	auto playSwitchList = cocos2d::__Dictionary::create();
	if (playSwitchList == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setSwitchList(playSwitchList);
		
	//user switch data
	cocos2d::Ref *ref;
	auto switchList = objectData->getSwitchArray();
	CCARRAY_FOREACH(switchList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto switchData = static_cast<agtk::data::SwitchData *>(ref);
#else
		auto switchData = dynamic_cast<agtk::data::SwitchData *>(ref);
#endif
		auto playSwitchData = PlaySwitchData::create(switchData);
		playSwitchList->setObject(playSwitchData, playSwitchData->getId());
#ifdef USE_SAR_OPTIMIZE_2
		this->getSwitchArray().push_back(playSwitchData);
#endif
	}
	//add system switch data if not loaded
	for (int i = 0; i < CC_ARRAYSIZE(ObjectCommonSwitch); i++) {
		if (ObjectCommonSwitch[i].id < 0) continue;
		if (playSwitchList->objectForKey(ObjectCommonSwitch[i].id) != nullptr) continue;
		auto switchData = agtk::data::SwitchData::create(
			ObjectCommonSwitch[i].id,
			ObjectCommonSwitch[i].name,
			ObjectCommonSwitch[i].initialValue,
			ObjectCommonSwitch[i].toBeSaved,
			ObjectCommonSwitch[i].memo
		);
		auto playSwitchData = agtk::data::PlaySwitchData::create(switchData);
		playSwitchList->setObject(playSwitchData, playSwitchData->getId());
#ifdef USE_SAR_OPTIMIZE_2
		this->getSwitchArray().push_back(playSwitchData);
#endif
	}

	//変更不可フラグを立てる。
	this->getSwitchData(kObjectSystemSwitchLockTarget)->setReadOnly(true);
	this->getSwitchData(kObjectSystemSwitchPortalTouched)->setReadOnly(true);

	auto playVariableList = cocos2d::__Dictionary::create();
	if (playVariableList == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setVariableList(playVariableList);

	//user variable data
	auto variableList = objectData->getVariableArray();
	CCARRAY_FOREACH(variableList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto variableData = static_cast<agtk::data::VariableData * > (ref);
#else
		auto variableData = dynamic_cast<agtk::data::VariableData * > (ref);
#endif
		auto playVariableData = PlayVariableData::create(variableData);
		playVariableList->setObject(playVariableData, playVariableData->getId());
#ifdef USE_SAR_OPTIMIZE_2
		this->getVariableArray().push_back(playVariableData);
#endif
	}

	//system variable data
	for (int i = 0; i < CC_ARRAYSIZE(ObjectCommonVariable); i++) {
		if (ObjectCommonVariable[i].id < 0) continue;
		auto playVariableData = dynamic_cast<PlayVariableData *>(playVariableList->objectForKey(ObjectCommonVariable[i].id));
		if (!playVariableData) {
			auto variableData = agtk::data::VariableData::create(
				ObjectCommonVariable[i].id,
				ObjectCommonVariable[i].name,
				ObjectCommonVariable[i].initialValue,
				ObjectCommonVariable[i].toBeSaved,
				ObjectCommonVariable[i].memo
			);
			playVariableData = agtk::data::PlayVariableData::create(variableData);
		}

		//-----------------------------------------------------------------------------------------------------------------------------
		//外部入力サポート
		if (playVariableData->getId() == kObjectSystemVariableX//X
		||  playVariableData->getId() == kObjectSystemVariableY//Y
		||  playVariableData->getId() == kObjectSystemVariableScalingX//scaleX
		||  playVariableData->getId() == kObjectSystemVariableScalingY//scaleY
		||  playVariableData->getId() == kObjectSystemVariableDisplayDirection//表示方向
		||  playVariableData->getId() == agtk::data::kObjectSystemVariableControllerID) {//コントローラID
			playVariableData->setType(agtk::data::PlayVariableData::kTypeExternalSupport);
		}
		playVariableList->setObject(playVariableData, playVariableData->getId());
#ifdef USE_SAR_OPTIMIZE_2
		this->getVariableArray().push_back(playVariableData);
#endif
	}

	//-----------------------------------------------------------------------------------------------------------------------------
	//システム変数共通
	this->setInitSystemVariableData(kObjectSystemVariableObjectID, (double)objectData->getId());//オブジェクトIDが格納されます。ユーザによる変更は不可能です。
	this->setInitSystemVariableData(kObjectSystemVariableHP, (double)objectData->getHp());//基本設定の体力の値が代入されます。
	this->setInitSystemVariableData(kObjectSystemVariableMaxHP, (double)objectData->getMaxHp());//基本設定の最大体力の値が代入されます。
	this->setInitSystemVariableData(kObjectSystemVariableMinimumAttack, (double)objectData->getMinAttack());//基本設定の最小攻撃力の値が代入されます。
	this->setInitSystemVariableData(kObjectSystemVariableMaximumAttack, (double)objectData->getMaxAttack());//基本設定の最大攻撃力の値が代入されます。
	this->setInitSystemVariableData(kObjectSystemVariableDamageRatio, (double)objectData->getInitialDamageRate());//基本設定の被ダメージ率の値が代入されます。
	this->setInitSystemVariableData(kObjectSystemVariableDamageVariationValue, (double)objectData->getDamageVariationValue());//基本設定の変動値が代入されます。
	if (objectData->getCritical()) {
		// ACT2-2601: 「攻撃にクリティカルを設定」のチェックをオフにしている時は、変数の初期値のままを使うように。
		this->setInitSystemVariableData(kObjectSystemVariableCriticalRatio, (double)objectData->getCriticalRatio());//基本設定のクリティカル倍率の値が代入されます。
		this->setInitSystemVariableData(kObjectSystemVariableCriticalIncidence, (double)objectData->getCriticalIncidence());//基本設定のクリティカル発生率の値が代入されます。
	}
	this->setInitSystemVariableData(kObjectSystemVariableInvincibleDuration, (double)objectData->getInvincibleDuration300() / 300.0);//基本設定の被ダメージ時の無敵時間の値が代入されます。

	this->setInitSystemVariableData(kObjectSystemVariableHorizontalMove, (double)objectData->getHorizontalMove());//移動とジャンプの「左右の移動量」の数値を代入・変更可能。
	this->setInitSystemVariableData(kObjectSystemVariableVerticalMove, (double)objectData->getVerticalMove());//移動とジャンプの「上下の移動量」の数値を代入・変更可能。
	this->setInitSystemVariableData(kObjectSystemVariableHorizontalAccel, (double)objectData->getHorizontalAccel());//移動とジャンプの「左右の加速量」の数値を代入・変更可能。
	this->setInitSystemVariableData(kObjectSystemVariableVerticalAccel, (double)objectData->getVerticalAccel());//移動とジャンプの「上下の加速量」の数値を代入・変更可能。
	this->setInitSystemVariableData(kObjectSystemVariableHorizontalMaxMove, (double)objectData->getHorizontalMaxMove());//移動とジャンプの「左右の最大移動量」の数値を代入・変更可能。
	this->setInitSystemVariableData(kObjectSystemVariableVerticalMaxMove, (double)objectData->getVerticalMaxMove());//移動とジャンプの「上下の最大移動量」の数値を代入・変更可能。
	this->setInitSystemVariableData(kObjectSystemVariableHorizontalDecel, (double)objectData->getHorizontalDecel());//移動とジャンプの「左右の減速量」の数値を代入・変更可能。
	this->setInitSystemVariableData(kObjectSystemVariableVerticalDecel, (double)objectData->getVerticalDecel());//移動とジャンプの「上下の減速量」の数値を代入・変更可能。
	this->setInitSystemVariableData(kObjectSystemVariableDispPriority, (double)objectData->getDispPriority());//値が大きいほどレイヤー内で手前に表示されます。
	this->setInitSystemVariableData(kObjectSystemVariableInitialJumpSpeed, (double)objectData->getJumpInitialSpeed());//このオブジェクトのジャンプの初速。

	//変更不可フラグを立てる。
	this->getVariableData(kObjectSystemVariableObjectID)->setReadOnly(true);
	this->getVariableData(kObjectSystemVariableInstanceID)->setReadOnly(true);
	this->getVariableData(kObjectSystemVariableInstanceCount)->setReadOnly(true);
	this->getVariableData(kObjectSystemVariableParentObjectInstanceID)->setReadOnly(true);
	//-----------------------------------------------------------------------------------------------------------------------------

	this->setHitObjectGroupBit(objectData->getHitObjectGroupBit());

	return true;
}

void PlayObjectData::setup(agtk::data::ObjectData *objectData)
{
	CC_ASSERT(this->getId() == objectData->getId());
	cocos2d::Ref *ref;
	auto switchList = objectData->getSwitchArray();
	auto thisSwitchList = this->getSwitchList();
	CCARRAY_FOREACH(switchList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto switchData = static_cast<agtk::data::SwitchData *>(ref);
#else
		auto switchData = dynamic_cast<agtk::data::SwitchData *>(ref);
#endif
		auto playSwitchData = this->getSwitchData(switchData->getId());
		if (playSwitchData) {
			playSwitchData->setSwitchData(switchData);
		}
		else {
			playSwitchData = agtk::data::PlaySwitchData::create(switchData);
			auto playSwitchId = playSwitchData->getId();
			//未登録のidはスキップさせる。
			if (thisSwitchList->objectForKey(playSwitchId)) {
				thisSwitchList->setObject(playSwitchData, playSwitchId);
			}
		}
	}
	auto variableList = objectData->getVariableArray();
	auto thisVariableList = this->getVariableList();
	CCARRAY_FOREACH(variableList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto variableData = static_cast<agtk::data::VariableData *>(ref);
#else
		auto variableData = dynamic_cast<agtk::data::VariableData *>(ref);
#endif
		auto playVariableData = this->getVariableData(variableData->getId());
		if (playVariableData) {
			playVariableData->setVariableData(variableData);
		}
		else {
			playVariableData = agtk::data::PlayVariableData::create(variableData);
			auto playVariableId = playVariableData->getId();
			//未登録のidはスキップさせる。
			if (thisVariableList->objectForKey(playVariableId)) {
				thisVariableList->setObject(playVariableData, playVariableId);
			}
		}
	}

	if (this->getHitObjectGroupBit() == 0) {
		this->setHitObjectGroupBit(objectData->getHitObjectGroupBit());
	}

	this->setObjectData(objectData);
}

/**
* 対象のIDにロックされているか？
* @param	id	対象ID
* @return		True:ロックされている / False:ロックされていない
*/
bool PlayObjectData::isLocked(int id)
{
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(_lockingObjectIdList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto lockingObjectInstanceId = static_cast<Integer *>(ref)->getValue();
#else
		auto lockingObjectInstanceId = dynamic_cast<Integer *>(ref)->getValue();
#endif
		if (lockingObjectInstanceId == id) {
			return true;
		}
	}

	return false;
}

/**
* 対象IDをロック中IDリストに追加
* @param	id	対象ID
*/
void PlayObjectData::addLocking(int id)
{
	_lockingObjectIdList->addObject(Integer::create(id));
}

/**
* 対象IDをロック中IDリストから除外
* @param	id	対象ID
*/
void PlayObjectData::removeLocking(int id)
{
	for (int i = _lockingObjectIdList->count() - 1; i >= 0; i--) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto targettedObjectInstanceId = static_cast<Integer *>(_lockingObjectIdList->getObjectAtIndex(i))->getValue();
#else
		auto targettedObjectInstanceId = dynamic_cast<Integer *>(_lockingObjectIdList->getObjectAtIndex(i))->getValue();
#endif
		if (targettedObjectInstanceId == id) {
			_lockingObjectIdList->removeObjectAtIndex(i);
			break;
		}
	}
}

rapidjson::Value PlayObjectData::json(rapidjson::Document::AllocatorType& allocator)
{
	rapidjson::Value obj(rapidjson::kObjectType);
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	obj.AddMember(rapidjson::Value("id", allocator), this->getId(), allocator);
#else
#endif

	rapidjson::Value switchList(rapidjson::kArrayType);
	cocos2d::DictElement *el = nullptr;
	auto playSwitchList = this->getSwitchList();
	CCDICT_FOREACH(playSwitchList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto playSwitchData = static_cast<agtk::data::PlaySwitchData *>(el->getObject());
#else
		auto playSwitchData = dynamic_cast<agtk::data::PlaySwitchData *>(el->getObject());
#endif
		rapidjson::Value obj = playSwitchData->json(allocator);
		switchList.PushBack(obj, allocator);
	}
	obj.AddMember("switchList", switchList, allocator);

	rapidjson::Value variableList(rapidjson::kArrayType);
	auto playVariableList = this->getVariableList();
	CCDICT_FOREACH(playVariableList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto playVariableData = static_cast<agtk::data::PlayVariableData *>(el->getObject());
#else
		auto playVariableData = dynamic_cast<agtk::data::PlayVariableData *>(el->getObject());
#endif
		rapidjson::Value obj = playVariableData->json(allocator);
		variableList.PushBack(obj, allocator);
	}
	obj.AddMember("variableList", variableList, allocator);

// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	obj.AddMember(rapidjson::Value("hitObjectGroupBit", allocator), this->getHitObjectGroupBit(), allocator);
#else
#endif

	return obj;
}

PlaySwitchData *PlayObjectData::getSwitchData(int id)
{
// #AGTK-NX
#if defined(STATIC_DOWN_CAST) && defined(FORCE_STATIC_DOWN_CAST)
	return static_cast<PlaySwitchData *>(this->getSwitchList()->objectForKey(id));
#else
	return dynamic_cast<PlaySwitchData *>(this->getSwitchList()->objectForKey(id));
#endif
}

PlaySwitchData *PlayObjectData::getSwitchDataByName(const char *name)
{
	cocos2d::DictElement *el = nullptr;
	CCDICT_FOREACH(_switchList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto data = static_cast<PlaySwitchData *>(el->getObject());
#else
		auto data = dynamic_cast<PlaySwitchData *>(el->getObject());
#endif
		if (strcmp(data->getSwitchData()->getName(), name) == 0) {
			return data;
		}
	}
	return nullptr;
}

PlayVariableData *PlayObjectData::getVariableData(int id)
{
	if (_objectId > 0 && (id == agtk::data::kObjectSystemVariableSingleInstanceID || id == agtk::data::kObjectSystemVariableInstanceCount)) {
		auto projectPlayData = GameManager::getInstance()->getPlayData();
		return projectPlayData->getVariableData(_objectId, id);
	}
// #AGTK-NX
#if defined(STATIC_DOWN_CAST) && defined(FORCE_STATIC_DOWN_CAST)
	return static_cast<PlayVariableData *>(this->getVariableList()->objectForKey(id));
#else
	return dynamic_cast<PlayVariableData *>(this->getVariableList()->objectForKey(id));
#endif
}

PlayVariableData *PlayObjectData::getVariableDataByName(const char *name)
{
	cocos2d::DictElement *el;
	CCDICT_FOREACH(_variableList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto data = static_cast<PlayVariableData *>(el->getObject());
#else
		auto data = dynamic_cast<PlayVariableData *>(el->getObject());
#endif
		if (strcmp(data->getVariableData()->getName(), name) == 0) {
			if (_objectId > 0 && (data->getId() == agtk::data::kObjectSystemVariableSingleInstanceID || data->getId() == agtk::data::kObjectSystemVariableInstanceCount)) {
				auto projectPlayData = GameManager::getInstance()->getPlayData();
				return projectPlayData->getVariableData(_objectId, data->getId());
			}
			return data;
		}
	}
	return nullptr;
}

/**
* 変数の引き継ぎ
* @param	variable_list	引き継ぎ元の変数データリスト
* @note	引き継ぎ元と引き継ぎ先の「同一ID」の変数のみ引き継ぐ
*/
void PlayObjectData::takeOverVariableList(PlayObjectData *playObjectData)
{
	auto variableList = playObjectData->getVariableList();

	// 引き継ぎ元の変数リストを回す
	cocos2d::DictElement *el;
	CCDICT_FOREACH(variableList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto src_data = static_cast<PlayVariableData *>(el->getObject());
#else
		auto src_data = dynamic_cast<PlayVariableData *>(el->getObject());
#endif
		int src_id = src_data->getId();
		src_data = playObjectData->getVariableData(src_id);

		// 指定IDのデータを取得
		agtk::data::PlayVariableData *target = this->getVariableData(src_id);
		if (src_id >= 2000) {
			target = this->getVariableDataByName(src_data->getVariableData()->getName());
		}

		// 指定IDのデータがある場合
		if (nullptr != target) {
			target->setValue(src_data->getValue());
		}
	}
}

/**
* スイッチの引き継ぎ
* @param	switch_list	引き継ぎ元のスイッチデータリスト
* @note	引き継ぎ元と引き継ぎ先の「同一ID」のスイッチのみ引き継ぐ
*/
void PlayObjectData::takeOverSwitchList(PlayObjectData *playObjectData)
{
	auto switchList = playObjectData->getSwitchList();

	// 引き継ぎ元の変数リストを回す
	cocos2d::DictElement *el;
	CCDICT_FOREACH(switchList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto src_data = static_cast<PlaySwitchData *>(el->getObject());
#else
		auto src_data = dynamic_cast<PlaySwitchData *>(el->getObject());
#endif
		int src_id = src_data->getId();
		src_data = playObjectData->getSwitchData(src_id);

		// 指定IDのデータを取得
		agtk::data::PlaySwitchData *target = this->getSwitchData(src_id);
		if (src_id >= 2000) {
			target = this->getSwitchDataByName(src_data->getSwitchData()->getName());
		}
		// 指定IDのデータがある場合
		if (nullptr != target) {
			target->setValue(src_data->getValue());
		}
	}
}

void PlayObjectData::setInitSystemSwitchData(EnumObjectSystemSwitch id, bool value)
{
	auto switchData = this->getSwitchData(id);
	switchData->getSwitchData()->setInitialValue(value);
	switchData->reset();
}

void PlayObjectData::setInitSystemVariableData(EnumObjectSystemVariable id, double value)
{
	auto variableData = this->getVariableData(id);
	variableData->getVariableData()->setInitialValue(value);
	variableData->reset();
}

bool PlayObjectData::setSystemSwitchData(EnumObjectSystemSwitch id, bool value)
{
	auto switchData = this->getSwitchData(id);
	bool oldValue = switchData->getValue();
	switchData->setValue(value);
	return oldValue;
}

double PlayObjectData::setSystemVariableData(EnumObjectSystemVariable id, double value)
{
	auto variableData = this->getVariableData(id);
	return variableData->setValue(value);
}

void PlayObjectData::setInstanceId(int id)
{
#if 1
	//この関数の場合だけ、変更不可のInstanceIdの値を変更できるようにする。
	auto variableData = this->getVariableData(kObjectSystemVariableInstanceID);
	auto lReadOnly = variableData->getReadOnly();
	variableData->setReadOnly(false);
	variableData->setValue((double)id);
	variableData->setReadOnly(lReadOnly);
#else
	this->setSystemVariableData(kObjectSystemVariableInstanceID, (double)id);
#endif
}

int PlayObjectData::getInstanceId()
{
	auto variableData = this->getVariableData(kObjectSystemVariableInstanceID);
	return (int)variableData->getValue();
}

void PlayObjectData::setControllerId(int id)
{
	this->setSystemVariableData(kObjectSystemVariableControllerID, (double)id);
}

int PlayObjectData::getControllerId()
{
	auto variableData = this->getVariableData(kObjectSystemVariableControllerID);
	return (int)variableData->getValue();
}

// #AGTK-NX #AGTK-WIN
#ifdef USE_SAR_PROVISIONAL_1
void PlayObjectData::setInitControllerId(int id)
{
	this->setInitSystemVariableData(kObjectSystemVariableControllerID, (double)id);
}
#endif

void PlayObjectData::setPlayerId(int id)
{
	this->setSystemVariableData(kObjectSystemVariablePlayerID, (double)id);
}

int PlayObjectData::getPlayerId()
{
	auto variableData = this->getVariableData(kObjectSystemVariablePlayerID);
	return (int)variableData->getValue();
}

void PlayObjectData::setAttackAttribute(int attr)
{
	this->setSystemVariableData(kObjectSystemVariableAttackAttribute, (double)attr);
}

int PlayObjectData::getAttackAttribute()
{
	auto variableData = this->getVariableData(kObjectSystemVariableAttackAttribute);
	return (int)variableData->getValue();
}

void PlayObjectData::setHp(double hp)
{
	this->setSystemVariableData(kObjectSystemVariableHP, hp);
}

double PlayObjectData::getHp()
{
	return this->getVariableData(kObjectSystemVariableHP)->getValue();
}

double PlayObjectData::addHp(double hp)
{
	double oldHp = this->getHp();
	this->setHp(oldHp + hp);
	return oldHp;
}

int PlayObjectData::setInstanceCount(int count)
{
#if 1
	//この関数の場合だけ、変更不可のInstanceCountの値を変更できるようにする。
	auto variableData = this->getVariableData(kObjectSystemVariableInstanceCount);
	auto lReadOnly = variableData->getReadOnly();
	variableData->setReadOnly(false);
	auto lastValue = (int)variableData->setValue((double)count);
	variableData->setReadOnly(lReadOnly);
	return lastValue;
#else
	auto variableData = this->getVariableData(kObjectSystemVariableInstanceCount);
	variableData->setValue((double)count);
#endif
}

int PlayObjectData::getInstanceCount()
{
	auto variableData = this->getVariableData(kObjectSystemVariableInstanceCount);
	return (int)variableData->getValue();
}

bool PlayObjectData::setLockTarget(bool value)
{
	//この関数の場合だけ、変更不可のkObjectSystemSwitchLockTargetの値を変更できるようにする。
	auto switchData = this->getSwitchData(kObjectSystemSwitchLockTarget);
	auto lReadOnly = switchData->getReadOnly();
	switchData->setReadOnly(false);
	auto lastValue = (int)switchData->setValue(value);
	switchData->setReadOnly(lReadOnly);
	return lastValue;
}

bool PlayObjectData::setPortalTouched(bool value)
{
	//この関数の場合だけ、変更不可のkObjectSystemSwitchPortalTouchedの値を変更できるようにする。
	auto switchData = this->getSwitchData(kObjectSystemSwitchPortalTouched);
	auto lReadOnly = switchData->getReadOnly();
	switchData->setReadOnly(false);
	auto lastValue = (int)switchData->setValue(value);
	switchData->setReadOnly(lReadOnly);
	return lastValue;
}

void PlayObjectData::reset(bool bExcludeInstanceId)
{
	cocos2d::DictElement *el;
	auto switchList = this->getSwitchList();
	CCDICT_FOREACH(switchList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::PlaySwitchData *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::data::PlaySwitchData *>(el->getObject());
#endif
		p->reset();
	}
	auto variableList = this->getVariableList();
	CCDICT_FOREACH(variableList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::PlayVariableData *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::data::PlayVariableData *>(el->getObject());
#endif
		if (bExcludeInstanceId && (p->getId() == kObjectSystemVariableInstanceID || p->getId() == kObjectSystemVariableSingleInstanceID)) {
			continue;
		}
		p->reset();
	}
}

void PlayObjectData::adjustSwitchData(PlaySwitchData *switchData)
{
}

void PlayObjectData::adjustVariableData(PlayVariableData *variableData)
{
	if (variableData == nullptr) {
		return;
	}
	int id = variableData->getId();
	switch (id) {
	case kObjectSystemVariableHP: {
		auto value = variableData->getValue();
		//最大体力値より大きくなった場合。
		auto maxHp = this->getVariableData(kObjectSystemVariableMaxHP);
		if (value > maxHp->getValue()) {
			value = maxHp->getValue();
		}
		//体力が０以下になった場合。
		if (value < 0.0) value = 0.0;
		variableData->setValue(value);
		break; }
	case kObjectSystemVariablePlayerID: {
		auto value = variableData->getValue();
		bool isChange = false;
		// プレイヤーの値0～4までに収める(0はシーンに直接配置した場合)
		if (value > 4) {
			value = 4;
			isChange = true;
		}
		if (value < 0) {
			value = 0;
			isChange = true;
		}
		if (isChange) {
			variableData->setValue(value);
		}
		break; }
	case kObjectSystemVariableSingleInstanceID: {
		auto value = (int)variableData->getValue();
		auto newValue = value;
		if (value != (int)value) {
			newValue = (int)value;
		}
		auto scene = GameManager::getInstance()->getCurrentScene();
		if (scene != nullptr) {
			if (_objectData) {
				auto objectId = _objectData->getId();
				auto instance = scene->getObjectInstance(objectId, (int)newValue);
				if (!instance) {
					//Object::removeSelf()同様に単体インスタンスIDを求める。
					auto type = SceneLayer::kTypeMax;
					{
						auto objectAll = scene->getObjectAll(objectId);
						cocos2d::Ref *ref = nullptr;
						CCARRAY_FOREACH(objectAll, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
							auto p = static_cast<agtk::Object *>(ref);
#else
							auto p = dynamic_cast<agtk::Object *>(ref);

#endif
							if (p->getPlayObjectData() == this) {
								auto sceneLayer = p->getSceneLayer();
								type = sceneLayer->getType();
								break;
							}
						}
					}
					//共通オブジェクトでインスタンスIDが若いオブジェクトIDを設定する。
					auto objectAll = scene->getObjectAll(objectId, type);
					cocos2d::Ref *ref = nullptr;
					agtk::Object *object = nullptr;
					CCARRAY_FOREACH(objectAll, ref) {
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
					if (object && objectAll->count() > 0) {
						newValue = object->getInstanceId();
					}
					else {
						//共通オブジェクトが見つからない場合は設定しない。
						break;
					}
				}
			}
		}
		if (newValue > 0 && newValue != value) {
			variableData->setValue(newValue);
		}
		break; }
	default:
		break;
	}
}

void PlayObjectData::adjustData()
{
	//adjust switch data
	auto switchList = this->getSwitchList();
	{
		cocos2d::DictElement *el;
		CCDICT_FOREACH(switchList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto switchData = static_cast<agtk::data::PlaySwitchData *>(el->getObject());
#else
			auto switchData = dynamic_cast<agtk::data::PlaySwitchData *>(el->getObject());
#endif
			this->adjustSwitchData(switchData);
		}
	}

	//adjust variable data.
	auto variableList = this->getVariableList();
	{
		cocos2d::DictElement *el;
		CCDICT_FOREACH(variableList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto variableData = static_cast<agtk::data::PlayVariableData *>(el->getObject());
#else
			auto variableData = dynamic_cast<agtk::data::PlayVariableData *>(el->getObject());
#endif
			if (variableData->getId() == kObjectSystemVariableSingleInstanceID || variableData->getId() == kObjectSystemVariableInstanceCount) {
				variableData = getVariableData(variableData->getId());
			}
			this->adjustVariableData(variableData);
		}
	}
}

void PlayObjectData::update(float delta)
{
	{
#ifdef USE_SAR_OPTIMIZE_2
		for (auto it = this->getSwitchArray().begin(); it != this->getSwitchArray().end(); it++)
		{
			(*it)->update(delta);
		}
#else
		cocos2d::DictElement *el;
		auto switchList = this->getSwitchList();
		CCDICT_FOREACH(switchList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::PlaySwitchData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::PlaySwitchData *>(el->getObject());
#endif
			p->update(delta);
		}
#endif
	}

	{
#ifdef USE_SAR_OPTIMIZE_2
		for (auto it = this->getVariableArray().begin(); it != this->getVariableArray().end(); it++)
		{
			(*it)->update(delta);
		}
#else
		cocos2d::DictElement *el;
		auto variableList = this->getVariableList();
		CCDICT_FOREACH(variableList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::PlayVariableData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::PlayVariableData *>(el->getObject());
#endif
			p->update(delta);
		}
#endif
	}
}

bool PlayObjectData::isHitObjectGroup(int group)const
{
	CC_ASSERT(0 <= group);
	return getHitObjectGroupBit() & (1 << group);
}

#if defined(AGTK_DEBUG)
void PlayObjectData::dump()
{
	CCLOG("PID id:%d", this->getId());
	cocos2d::DictElement *el = nullptr;
	auto switchList = this->getSwitchList();
	CCDICT_FOREACH(switchList, el) {
		auto switchData = dynamic_cast<PlaySwitchData *>(el->getObject());
		switchData->dump();
	}
	auto variableList = this->getVariableList();
	CCDICT_FOREACH(variableList, el) {
		auto variableData = dynamic_cast<PlayVariableData *>(el->getObject());
		variableData->dump();
	}
}
#endif

// 親オブジェクトのインスタンスIDをセット
void PlayObjectData::setParentObjectInstanceId(int id)
{
	//この関数の場合だけ、変更不可のInstanceIdの値を変更できるようにする。
	auto variableData = this->getVariableData(kObjectSystemVariableParentObjectInstanceID);
	auto lReadOnly = variableData->getReadOnly();
	variableData->setReadOnly(false);
	variableData->setValue((double)id);
	variableData->setReadOnly(lReadOnly);
}

int PlayObjectData::getParentObjectInstanceId()
{
	auto variableData = this->getVariableData(kObjectSystemVariableParentObjectInstanceID);
	return (int)variableData->getValue();
}

void PlayObjectData::clearObjectReference()
{
	cocos2d::DictElement *el;
	auto variableList = this->getVariableList();
	CCDICT_FOREACH(variableList, el) {
		auto playVariableData = dynamic_cast<agtk::data::PlayVariableData *>(el->getObject());
		playVariableData->setObject(nullptr);
	}
}

//-------------------------------------------------------------------------------------------------------------------
PlayData::PlayData()
{
	_commonSwitchList = nullptr;
	_commonVariableList = nullptr;
	_objectList = nullptr;
	_projectData = nullptr;
}

PlayData::~PlayData()
{
	CC_SAFE_RELEASE_NULL(_commonSwitchList);
	CC_SAFE_RELEASE_NULL(_commonVariableList);
	CC_SAFE_RELEASE_NULL(_objectList);
	CC_SAFE_RELEASE_NULL(_projectData);
}

bool PlayData::init(agtk::data::ProjectData *projectData)
{
	auto switchList = cocos2d::__Dictionary::create();
	if (switchList == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setCommonSwitchList(switchList);

	//system switch data
	for (int i = 0; i < CC_ARRAYSIZE(ProjectCommonSwitch); i++) {
		if (ProjectCommonSwitch[i].id < 0) continue;

		auto switchData = agtk::data::SwitchData::create(
			ProjectCommonSwitch[i].id,
			ProjectCommonSwitch[i].name,
			ProjectCommonSwitch[i].initialValue,
			ProjectCommonSwitch[i].toBeSaved,
			ProjectCommonSwitch[i].memo
		);
		auto playSwitchData = agtk::data::PlaySwitchData::create(switchData);
		this->getCommonSwitchList()->setObject(playSwitchData, playSwitchData->getId());
#ifdef USE_SAR_OPTIMIZE_2
		this->getCommonSwitchArray().push_back(playSwitchData);
#endif
	}

	auto variableList = cocos2d::__Dictionary::create();
	if (variableList == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setCommonVariableList(variableList);

	//system variable data
	for (int i = 0; i < CC_ARRAYSIZE(ProjectCommonVariable); i++) {
		if (ProjectCommonVariable[i].id < 0) continue;
		auto variableData = agtk::data::VariableData::create(
			ProjectCommonVariable[i].id,
			ProjectCommonVariable[i].name,
			ProjectCommonVariable[i].initialValue,
			ProjectCommonVariable[i].toBeSaved,
			ProjectCommonVariable[i].memo
		);
		auto playVariableData = agtk::data::PlayVariableData::create(variableData);
		if (playVariableData->getId() == kProjectSystemVariable1PController
		||  playVariableData->getId() == kProjectSystemVariable2PController
		||  playVariableData->getId() == kProjectSystemVariable3PController
		||  playVariableData->getId() == kProjectSystemVariable4PController) {
			playVariableData->setType(agtk::data::PlayVariableData::kTypeExternalSupport);
		}
		this->getCommonVariableList()->setObject(playVariableData, playVariableData->getId());
#ifdef USE_SAR_OPTIMIZE_2
		this->getCommonVariableArray().push_back(playVariableData);
#endif
	}
	//変更不可フラグを立てる。
	this->getCommonVariableData(agtk::data::kProjectSystemVariablePlayerCount)->setReadOnly(true);
	this->getCommonSwitchData(agtk::data::kProjectSystemSwitchFileExists)->setReadOnly(true);

	auto objectList = cocos2d::__Dictionary::create();
	if (objectList == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setObjectList(objectList);
#if 0//TODO ロードするのを保留
	//load
	this->load();
#endif
	//setup
	this->setup(projectData);

	return true;
}

void PlayData::setup(agtk::data::ProjectData *projectData)
{
	cocos2d::Ref *ref;
	//switch
	auto switchList = projectData->getSwitchArray();
	CCARRAY_FOREACH(switchList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto switchData = static_cast<agtk::data::SwitchData *>(ref);
#else
		auto switchData = dynamic_cast<agtk::data::SwitchData *>(ref);
#endif
		auto playSwitchData = this->getCommonSwitchData(switchData->getId());
		if (playSwitchData) {
			playSwitchData->setSwitchData(switchData);
		}
		else {
			auto playSwitchData = agtk::data::PlaySwitchData::create(switchData);
			this->getCommonSwitchList()->setObject(playSwitchData, playSwitchData->getId());
		}
	}
	//variable
	auto variableList = projectData->getVariableArray();
	CCARRAY_FOREACH(variableList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto variableData = static_cast<agtk::data::VariableData *>(ref);
#else
		auto variableData = dynamic_cast<agtk::data::VariableData *>(ref);
#endif
		auto playVariableData = this->getCommonVariableData(variableData->getId());
		if (playVariableData) {
			playVariableData->setVariableData(variableData);
		}
		else {
			auto playVariableData = agtk::data::PlayVariableData::create(variableData);
			this->getCommonVariableList()->setObject(playVariableData, playVariableData->getId());
		}
	}
	//object
	auto objectList = projectData->getObjectArray();
	CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto objectData = static_cast<agtk::data::ObjectData *>(ref);
#else
		auto objectData = dynamic_cast<agtk::data::ObjectData *>(ref);
#endif
		CC_ASSERT(objectData->getFolder() == false);
		auto playObjectData = this->getObjectData(objectData->getId());
		if (playObjectData) {
			playObjectData->setup(objectData);
		}
		else {
			auto playObjectData = agtk::data::PlayObjectData::create(objectData);
			this->getObjectList()->setObject(playObjectData, playObjectData->getId());
		}
	}
	//初期値を設定するため、ここでリセット処理をする。
	this->reset();
}

void PlayData::reset(bool bExcludeInstanceId)
{
	cocos2d::DictElement *el;
	auto commonSwitchList = this->getCommonSwitchList();
	CCDICT_FOREACH(commonSwitchList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::PlaySwitchData *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::data::PlaySwitchData *>(el->getObject());
#endif
		p->reset();
	}
	auto commonVariableList = this->getCommonVariableList();
	CCDICT_FOREACH(commonVariableList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::PlayVariableData *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::data::PlayVariableData *>(el->getObject());
#endif
		p->reset();
	}
	auto objectList = this->getObjectList();
	CCDICT_FOREACH(objectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<agtk::data::PlayObjectData *>(el->getObject());
#else
		auto p = dynamic_cast<agtk::data::PlayObjectData *>(el->getObject());
#endif
		p->reset(bExcludeInstanceId);
	}
}

agtk::data::PlaySwitchData *PlayData::getCommonSwitchData(int id)
{
// #AGTK-NX
#if defined(STATIC_DOWN_CAST) && defined(FORCE_STATIC_DOWN_CAST)
	return static_cast<agtk::data::PlaySwitchData *>(this->getCommonSwitchList()->objectForKey(id));
#else
	return dynamic_cast<agtk::data::PlaySwitchData *>(this->getCommonSwitchList()->objectForKey(id));
#endif
}

agtk::data::PlayVariableData *PlayData::getCommonVariableData(int id)
{
// #AGTK-NX
#if defined(STATIC_DOWN_CAST) && defined(FORCE_STATIC_DOWN_CAST)
	return static_cast<agtk::data::PlayVariableData *>(this->getCommonVariableList()->objectForKey(id));
#else
	return dynamic_cast<agtk::data::PlayVariableData *>(this->getCommonVariableList()->objectForKey(id));
#endif
}

agtk::data::PlayVariableData *PlayData::getCommonVariableDataByName(const char *name)
{ 
	auto commonVariableList = this->getCommonVariableList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(commonVariableList, el) {
		auto p = dynamic_cast<agtk::data::PlayVariableData *>(el->getObject());
		auto variableName = p->getVariableData()->getName();
		if (strcmp(variableName, name) == 0) {
			return p;
		}
	}
	return nullptr;
}

agtk::data::PlayObjectData *PlayData::getObjectData(int id)
{
// #AGTK-NX
#if defined(STATIC_DOWN_CAST) && defined(FORCE_STATIC_DOWN_CAST)
	return static_cast<agtk::data::PlayObjectData *>(this->getObjectList()->objectForKey(id));
#else
	return dynamic_cast<agtk::data::PlayObjectData *>(this->getObjectList()->objectForKey(id));
#endif
}

agtk::data::PlaySwitchData *PlayData::getSwitchData(int objectId, int id)
{
	//共通
	if (objectId == COMMON_PLAY_DATA_ID) {
		return this->getCommonSwitchData(id);
	}
	//オブジェクト
	else {
		auto objectData = this->getObjectData(objectId);
		if (objectData == nullptr) {
			return nullptr;
		}
		return objectData->getSwitchData(id);
	}
}

agtk::data::PlayVariableData *PlayData::getVariableData(int objectId, int id)
{
	//共通
	if (objectId == COMMON_PLAY_DATA_ID) {
		return this->getCommonVariableData(id);
	}
	//オブジェクト
	else {
		auto objectData = this->getObjectData(objectId);
		if (objectData == nullptr) {
			return nullptr;
		}
		return objectData->getVariableData(id);
	}
}

agtk::data::PlayVariableData *PlayData::getVariableDataByName(int objectId, const char *name)
{
	//共通
	if (objectId == COMMON_PLAY_DATA_ID) {
		return this->getCommonVariableDataByName(name);
	}
	//オブジェクト
	else {
		auto objectData = this->getObjectData(objectId);
		if (objectData == nullptr) {
			return nullptr;
		}
		return objectData->getVariableDataByName(name);
	}
}

/**
* プレイデータのロード
* @param	json	プレイデータのJSONオブジェクト
*/
void PlayData::loadData(const rapidjson::Value& json, bool sw, bool var, bool obj)
{
	// プロジェクト共通スイッチデータ
	if (sw) {
		auto switchList = this->getCommonSwitchList();
		for (rapidjson::SizeType i = 0; i < json["switchList"].Size(); i++) {
			auto loadPlaySwitchData = PlaySwitchData::create(json["switchList"][i]);
			auto switchDataId = loadPlaySwitchData->getId();
			auto playSwitchData = this->getCommonSwitchData(switchDataId);
			if (playSwitchData) {
				loadPlaySwitchData->setSwitchData(playSwitchData->getSwitchData());
			}
			//未登録のidはスキップさせる。
			if (switchList->objectForKey(switchDataId)) {
				switchList->setObject(loadPlaySwitchData, switchDataId);
			}
		}
	}
	if (var) {
		// プロジェクト共通変数データ
		auto variableList = this->getCommonVariableList();
		for (rapidjson::SizeType i = 0; i < json["variableList"].Size(); i++) {
			auto id = json["variableList"][i]["id"].GetInt();
			// 各プレイヤーのコントローラーIDは除外
			if (kProjectSystemVariable1PController <= id && id <= kProjectSystemVariable4PController) {
				continue;
			}
			auto loadedPlayVariableData = PlayVariableData::create(json["variableList"][i]);
			auto playVariableData = this->getCommonVariableData(id);
			if (playVariableData) {
				loadedPlayVariableData->setVariableData(playVariableData->getVariableData());
			}
			//未登録のidはスキップさせる。
			if (variableList->objectForKey(id)) {
				variableList->setObject(loadedPlayVariableData, id);
			}
		}
	}
	if (obj) {
		// オブジェクト毎のデータ
		auto objectList = this->getObjectList();
		for (rapidjson::SizeType i = 0; i < json["objectList"].Size(); i++) {
			auto loadedObjectData = PlayObjectData::create(json["objectList"][i]);
			auto objectData = this->getObjectData(loadedObjectData->getId());
			if (objectData && objectData->getObjectData()) {
				loadedObjectData->setup(objectData->getObjectData());
			}
			objectList->setObject(loadedObjectData, loadedObjectData->getId());
		}
		this->setObjectList(objectList);
	}
}

/**
* プレイデータのセーブ
* @param	allocator	JSONアロケーター
* @return				プレイデータのJSONオブジェクト
*/
rapidjson::Value PlayData::saveData(rapidjson::Document::AllocatorType& allocator)
{
	rapidjson::Value obj(rapidjson::kObjectType);

	cocos2d::DictElement *el = nullptr;
	// プロジェクト共通スイッチデータ
	rapidjson::Value switchList(rapidjson::kArrayType);
	auto commonSwitchList = this->getCommonSwitchList();
	CCDICT_FOREACH(commonSwitchList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto playSwitchData = static_cast<agtk::data::PlaySwitchData *>(el->getObject());
#else
		auto playSwitchData = dynamic_cast<agtk::data::PlaySwitchData *>(el->getObject());
#endif
		rapidjson::Value obj = playSwitchData->json(allocator);
		switchList.PushBack(obj, allocator);
	}
	obj.AddMember("switchList", switchList, allocator);

	// プロジェクト共通変数データ
	rapidjson::Value variableList(rapidjson::kArrayType);
	auto commonVariableList = this->getCommonVariableList();
	CCDICT_FOREACH(commonVariableList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto playVariableData = static_cast<agtk::data::PlayVariableData *>(el->getObject());
#else
		auto playVariableData = dynamic_cast<agtk::data::PlayVariableData *>(el->getObject());
#endif
		rapidjson::Value obj = playVariableData->json(allocator);
		variableList.PushBack(obj, allocator);
	}
	obj.AddMember("variableList", variableList, allocator);


	// オブジェクト毎のデータ
	rapidjson::Value objectList(rapidjson::kArrayType);
	auto playObjectList = this->getObjectList();
	CCDICT_FOREACH(playObjectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto playObjectData = static_cast<agtk::data::PlayObjectData *>(el->getObject());
#else
		auto playObjectData = dynamic_cast<agtk::data::PlayObjectData *>(el->getObject());
#endif
		rapidjson::Value obj = playObjectData->json(allocator);
		objectList.PushBack(obj, allocator);
	}
	obj.AddMember("objectList", objectList, allocator);

	return obj;
}

void PlayData::adjustCommonSwitchData(agtk::data::PlaySwitchData *switchData)
{
	//スイッチデータの調整。
}

void PlayData::adjustCommonVariableData(agtk::data::PlayVariableData *variableData)
{
	//変数データの調整。
	if (variableData == nullptr) {
		return;
	}
	int id = variableData->getId();
	switch (id) {
	case kProjectSystemVariableBgmVolumeAdjust:
	case kProjectSystemVariableSeVolumeAdjust:
	case kProjectSystemVariableVoiceVolumeAdjust: {
		auto value = variableData->getValue();
		// 音量の範囲は0～100
		if (value > 100) {
			value = 100;
		}
		else if (value < 0.0) {
			value = 0.0;
		}
		variableData->setValue(value);
		break; }
	default:
		break;
	}
}

void PlayData::adjustSwitchData(int objectId, agtk::data::PlaySwitchData *switchData)
{
	//スイッチデータの調整。
	auto playObjectData = this->getObjectData(objectId);
	playObjectData->adjustSwitchData(switchData);
}

void PlayData::adjustVariableData(int objectId, agtk::data::PlayVariableData *variableData)
{
	//変数データの調整。
	auto playObjectData = this->getObjectData(objectId);
	playObjectData->adjustVariableData(variableData);
}

void PlayData::adjustData()
{
	//object
	auto objectList = this->getObjectList();
	{
		cocos2d::DictElement *el;
		CCDICT_FOREACH(objectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::PlayObjectData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::PlayObjectData *>(el->getObject());
#endif
			p->adjustData();
		}
	}
	//common switch
	auto commonSwitchList = this->getCommonSwitchList();
	{
		cocos2d::DictElement *el;
		CCDICT_FOREACH(commonSwitchList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::PlaySwitchData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::PlaySwitchData *>(el->getObject());
#endif
			this->adjustCommonSwitchData(p);
		}
	}
	//common variable
	auto commonVariableList = this->getCommonVariableList();
	{
		cocos2d::DictElement *el;
		CCDICT_FOREACH(commonVariableList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::PlayVariableData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::PlayVariableData *>(el->getObject());
#endif
			this->adjustCommonVariableData(p);
		}
	}
}

void PlayData::update(float delta)
{
	//object
	auto objectList = this->getObjectList();
	{
		cocos2d::DictElement *el;
		CCDICT_FOREACH(objectList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::PlayObjectData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::PlayObjectData *>(el->getObject());
#endif
			p->update(delta);
		}
	}
	//common switch
#ifdef USE_SAR_OPTIMIZE_2
	for (auto it = this->getCommonSwitchArray().begin(); it != this->getCommonSwitchArray().end(); it++)
	{
		(*it)->update(delta);
	}
#else
	auto commonSwitchList = this->getCommonSwitchList();
	{
		cocos2d::DictElement *el;
		CCDICT_FOREACH(commonSwitchList, el) {
			// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::PlaySwitchData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::PlaySwitchData *>(el->getObject());
#endif
			p->update(delta);
		}
	}
#endif
	//common variable
#ifdef USE_SAR_OPTIMIZE_2
	for (auto it = this->getCommonVariableArray().begin(); it != this->getCommonVariableArray().end(); it++)
	{
		(*it)->update(delta);
	}
#else
	auto commonVariableList = this->getCommonVariableList();
	{
		cocos2d::DictElement *el;
		CCDICT_FOREACH(commonVariableList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<agtk::data::PlayVariableData *>(el->getObject());
#else
			auto p = dynamic_cast<agtk::data::PlayVariableData *>(el->getObject());
#endif
			p->update(delta);
		}
	}
#endif
}

#if defined(AGTK_DEBUG)
void PlayData::dump()
{
	CCLOG("-- dump PlayData --");
	cocos2d::DictElement *el = nullptr;
	auto switchList = this->getCommonSwitchList();
	CCLOG("-- dump PlayData: common switch --");
	CCDICT_FOREACH(switchList, el) {
		auto switchData = dynamic_cast<PlaySwitchData *>(el->getObject());
		switchData->dump();
	}
	auto variableList = this->getCommonVariableList();
	CCLOG("-- dump PlayData: common variable --");
	CCDICT_FOREACH(variableList, el) {
		auto variableData = dynamic_cast<PlayVariableData *>(el->getObject());
		variableData->dump();
	}
	auto objectList = this->getObjectList();
	CCLOG("-- dump PlayData: objects --");
	CCDICT_FOREACH(objectList, el) {
		auto objectData = dynamic_cast<PlayObjectData *>(el->getObject());
		objectData->dump();
	}
	CCLOG("-------------------");
}
#endif

NS_DATA_END

//-------------------------------------------------------------------------------------------------------------------
ObjectTakeoverStatesData::ObjectTakeoverStatesData()
{
	_playObjectData = nullptr;
	_actionNo = -1;
	_directionNo = -1;
	_prevActionNo = -1;
	_scenePartsId = -1;
	_moveDirection = cocos2d::Vec2::ZERO;
	_takeOverAnimMotionId = -1;
	_sceneIdOfFirstCreated = -1;
}

ObjectTakeoverStatesData::~ObjectTakeoverStatesData()
{
	CC_SAFE_RELEASE_NULL(_playObjectData);
}

bool ObjectTakeoverStatesData::init(agtk::Object *object)
{
	if (object->getScenePartObjectData() == nullptr) {
		return false;
	}
	this->setSceneId(object->getSceneData()->getId());
	this->setSceneLayerId(object->getLayerId());
	this->setScenePartsId(object->getScenePartObjectData()->getId());
	this->setObjectId(object->getObjectData()->getId());
	this->setPosition(object->getPosition());
	this->setScale(cocos2d::Vec2(object->getScaleX(), object->getScaleY()));
	auto player = object->getPlayer();
	if (player) {
//		this->setActionNo(player->getCurrentActionNo());
		this->setDirectionNo(player->getCurrentDirectionNo());
	}
	this->setActionNo(object->getCurrentObjectAction()->getId());
	this->setPrevActionNo(object->getPrevObjectActionId());
	this->setPlayObjectData(object->getPlayObjectData());
	this->setDispDirection(object->getDispDirection());
	this->setMoveDirection(object->getObjectMovement()->getDirectionDirect());
	this->setTakeOverAnimMotionId(object->getTakeOverAnimMotionId());
	this->setObjectPosInCamera(object->getObjectPosInCamera());
	this->setSceneIdOfFirstCreated(object->getSceneIdOfFirstCreated());
	return true;
}

bool ObjectTakeoverStatesData::init(agtk::data::PlayObjectData *playObjectData)
{
	this->setPlayObjectData(playObjectData);
	return true;
}

#if defined(AGTK_DEBUG)
void ObjectTakeoverStatesData::dump()
{
	CCLOG("sceneId:%d", this->getSceneId());
	CCLOG("scenePartsId:%d", this->getScenePartsId());
	CCLOG("objectId:%d", this->getObjectId());
	CCLOG("position:%f,%f",  this->getPosition().x, this->getPosition().y);
	CCLOG("scale:%f,%f", this->getScale().x, this->getScale().y);
	CCLOG("actionNo:%d", this->getActionNo());
	CCLOG("prevActionNo:%d", this->getPrevActionNo());
	CCLOG("directionNo:%d", this->getDirectionNo());
	CCLOG("dispDirection:%d", this->getDispDirection());
	CCLOG("moveDirection:%f,%f", this->getMoveDirection().x, this->getMoveDirection().y);
	CCLOG("takeOverAnimMotionId:%d", this->getTakeOverAnimMotionId());
	CCLOG("sceneIdOfFirstCreated:%d", this->getSceneIdOfFirstCreated());
	this->getPlayObjectData()->dump();
}
#endif

NS_AGTK_END
