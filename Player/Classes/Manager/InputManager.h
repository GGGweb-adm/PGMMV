#ifndef __INPUT_MANAGER_H__
#define	__INPUT_MANAGER_H__

#include "cocos2d.h"
#include "base/CCGameController.h"
#include "base/CCEventListenerMouse.h"
#include "Data/ProjectData.h"
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

#define USE_CONTROLLER_DATA

USING_NS_CC;

//-------------------------------------------------------------------------------------------------------------------
/**
 * @brief マウス入力を管理する
 */
class AGTKPLAYER_API InputMouseData : public cocos2d::Ref
{
public:
	class InputData : public cocos2d::Ref
	{
	public:
		CREATE_FUNC_PARAM(InputData, int, keyCode);
		void update();
		void setTrigger(bool trigger);
		void setRelease(bool release);
		void reset();
	private:
		virtual bool init(int keyCode);
	private:
		CC_SYNTHESIZE_READONLY(bool, _trigger, Trigger);
		CC_SYNTHESIZE(bool, _press, Press);
		CC_SYNTHESIZE_READONLY(bool, _release, Release);
		CC_SYNTHESIZE_READONLY(int, _keyCode, KeyCode);
		bool _updateTrigger;
		bool _updateRelease;
	};
	static cocos2d::Vec2 CalcTransLeftUp(cocos2d::Vec2 v, cocos2d::Size size, cocos2d::Size displaySize);
private:
	InputMouseData();
	virtual ~InputMouseData();
public:
	CREATE_FUNC(InputMouseData);
	void reset();
	void update();
	InputData *getInputData(int keyCode);
	bool isPressOr();
	cocos2d::Vec2 setPoint(cocos2d::Vec2 v);
	cocos2d::Vec2 getPoint() { return _point; }
	cocos2d::Vec2 getOldPoint() { return _oldPoint; }
	void setMove(bool move);
	bool getMove() { return _move; }
	cocos2d::Vec2 setScrollPoint(cocos2d::Vec2 v);
	cocos2d::Vec2 getScrollPoint() { return _scrollPoint; }
	cocos2d::Vec2 getOldScrollPoint() { return _oldScrollPoint; }
private:
	virtual bool init();
private:
	cocos2d::Vec2 _point;
	cocos2d::Vec2 _oldPoint;
	CC_SYNTHESIZE(cocos2d::Vec2, _movePoint, MovePoint);
	CC_SYNTHESIZE(cocos2d::Vec2, _startPoint, StartPoint);
	cocos2d::Vec2 _scrollPoint;
	cocos2d::Vec2 _oldScrollPoint;
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _inputList, InputList);
	bool _move;
	bool _updateMove;
	CC_SYNTHESIZE(bool, _wheelTrigger, WheelTrigger);
	CC_SYNTHESIZE(int, _wheelRelease, WheelRelease);	//ホイール入力が無くなった瞬間に直前のホイール入力の方向(-1 or 1)が設定される。
	bool _updateWheel;
};

//-------------------------------------------------------------------------------------------------------------------
/**
 * @brief キーボード入力を管理する
 */
class AGTKPLAYER_API InputKeyboardData : public cocos2d::Ref
{
public:
	// 小文字アルファベットの KeyCode を 大文字アルファベットの KeyCode に変換
	// 小文字アルファベットでなければなにもしない。
	static int upperKeyCode(int keyCode);
	class InputData : public cocos2d::Ref
	{
	public:
		CREATE_FUNC_PARAM(InputData, int, keyCode);
		void update();
		void setTrigger(bool trigger);
		void setRelease(bool release);
		void setCharData(int keyCode);
		int getKeyCode();
		int getCharCode() { return _charCode; }
	private:
		virtual bool init(int keyCode);
	private:
		CC_SYNTHESIZE_READONLY(bool, _trigger, Trigger);
		CC_SYNTHESIZE(bool, _press, Press);
		CC_SYNTHESIZE_READONLY(bool, _release, Release);
		int _keyCode;
		int _charCode;
		CC_SYNTHESIZE(int, _scancode, Scancode);
		bool _updateTrigger;
		bool _updateRelease;
	};
private:
	InputKeyboardData();
	virtual ~InputKeyboardData();
public:
	CREATE_FUNC(InputKeyboardData);
	void reset(bool isReset=false);
	void update();
	InputData *setPressData(int keyCode, int scancode);
	InputData *setReleaseData(int keyCode, int scancode);
	InputData *setRepeatData(int keyCode, int scancode);
	InputData *setCharInputData(int keyCode);
	InputData *getInputData(int keyCode);
	static const char *getKeyCodeName(int keyCode);
private:
	virtual bool init();
private:
	CC_SYNTHESIZE(bool, _ignored, Ignored);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _inputList, InputList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _pressDataList, PressDataList);
	CC_SYNTHESIZE_RETAIN(InputData *, _emptyData, EmptyData);
};

//-------------------------------------------------------------------------------------------------------------------
/**
 * @brief ゲームコントローラー入力を管理する
 */
class AGTKPLAYER_API InputGamepadData : public cocos2d::Ref
{
public:
	enum EnumGamepadType {
		kGamepadXbox360,
		kGamepadPlayStation4,
		kGamepadDirectInput,
// #AGTK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#endif
		kGamepadMax,
	};
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

		//論理入力(DirectInput)
		kInputAxis1Minus = kInputAxis1,
		kInputAxis1Plus,
		kInputAxis2Minus,
		kInputAxis2Plus,
		kInputAxis3Minus,
		kInputAxis3Plus,
		kInputAxis4Minus,
		kInputAxis4Plus,
		kInputAxis5Minus,
		kInputAxis5Plus,
		kInputAxis6Minus,
		kInputAxis6Plus,
		kInputAxis7Minus,
		kInputAxis7Plus,

		kInputMax,
	};
public:
	class InputData : public cocos2d::Ref
	{
	public:
		enum EnumType { kTypeButton, kTypeAxis, kTypeLogic };
		static float StickThreshold;
	public:
		CREATE_FUNC_PARAM(InputData, EnumType, type);
		void update();
		void reset();
		void setTrigger(bool trigger);
		void setRelease(bool release);
		void setValue(float value);
		void setValue(float value, float minValue, float maxValue);
		float getValue() { return _value; }
		bool isChanged() { return _value != _oldValue; }
		bool isValue() { return _firstSetValueFlag; }//アナログ（数値）入力の場合。
	private:
		virtual bool init(EnumType type);
	private:
		CC_SYNTHESIZE(EnumType, _type, Type);
		CC_SYNTHESIZE_READONLY(bool, _trigger, Trigger);
		CC_SYNTHESIZE(bool, _press, Press);
		CC_SYNTHESIZE_READONLY(bool, _release, Release);
		float _oldValue;
		float _value;
		float _minValue;
		float _maxValue;
		bool _updateTrigger;
		bool _updateRelease;
		bool _firstSetValueFlag;
	};
private:
	InputGamepadData();
	virtual ~InputGamepadData();
public:
	CREATE_FUNC(InputGamepadData);
	void setup(EnumGamepadType gamepadType);
	void reset(bool isReset=false);
	void setNameWithChar(const char *name);
	const char *getName();
	EnumGamepadType getGamepadType();
	InputData *getInput(int keyCode);
	void update();
private:
	virtual bool init();
private:
	CC_SYNTHESIZE(bool, _connected, Connected);
	CC_SYNTHESIZE(bool, _ignored, Ignored);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _inputList, InputList);
};

//-------------------------------------------------------------------------------------------------------------------
/**
 * @brief マウス、キーボード、ゲームコントローラー入力をまとめて管理する
 */
class AGTKPLAYER_API InputDataRaw : public Ref
{
public:
	InputDataRaw();
	virtual ~InputDataRaw();
public:
	CREATE_FUNC(InputDataRaw);
private:
	virtual bool init();
public:
	void reset(bool isReset=false);
    class DeviceData {
    public:
        enum EnumInputType {
            kConnected,
            kDisconnected,
            kButtonDown,
            kButtonUp,
            kButtonRepeat,
            kAxis,
            kMouseDown,
            kMouseMove,
            kMouseUp,
            kMouseScroll,
            kKeyPressed,
            kKeyReleased,
			kKeyRepeat,
            kCharInput,
            kRandomSeed,
            kRestartProjectData,
            kInputTypeMax
        };
        DeviceData(): inputType(kInputTypeMax), deviceId(-1), deviceName(), keyCode(-1), axisValue(0), cursorX(0), cursorY(0), scrollX(0), scrollY(0), scancode(-1){}
        EnumInputType inputType;
        int deviceId;
        std::string deviceName;
        int keyCode;
        float axisValue;
        int mouseButton;
		int cursorX;
		int cursorY;
		int scrollX;
		int scrollY;
        int scancode;
    };
	void connected(const DeviceData &deviceData);
	void disconnected(const DeviceData &deviceData);
	void connected(cocos2d::Controller *controller);
	void disconnected(cocos2d::Controller *controller);
	void update();
	InputGamepadData *getGamepad(int padId);
#ifdef USE_PREVIEW
    bool startRecording();
    void stopRecording();
    bool isRecording();
    bool startPlaying(const char *filename);
    bool isReplaying();
#endif
    void applyRegisteredData();
    void registerConnected(int deviceId, const char *deviceName);
    void registerDisconnected(int deviceId, const char *deviceName);
    void registerButtonDown(int deviceId, const char *deviceName, int keyCode);
    void registerButtonUp(int deviceId, const char *deviceName, int keyCode);
    void registerButtonRepeat(int deviceId, const char *deviceName, int keyCode);
    void registerAxis(int deviceId, int keyCode, float value);
    void registerMouseDown(int button, int x, int y);
    void registerMouseMove(int x, int y);
    void registerMouseUp(int button, int x, int y);
    void registerMouseScroll(int x, int y);
    void registerKeyPressed(int keyCode, int scancode);
    void registerKeyReleased(int keyCode, int scancode);
	void registerKeyRepeat(int keyCode, int scancode);
    void registerCharInput(int keyCode);
    void registerRandomSeed(int seed);
    void registerRestart(bool restart);
#ifdef USE_PREVIEW
    void recordInput();
    void applyPlayInput();
	bool getRestartFlg(const char *filename);
#endif
public:
	static const int MAX_GAMEPAD = 16;
	static const int MAX_TOUCH = 10;
private:
	CC_SYNTHESIZE_RETAIN(InputMouseData *, _mouse, Mouse);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _gamepadList, GamepadList);
	CC_SYNTHESIZE_RETAIN(InputKeyboardData *, _keyboard, Keyboard);
#ifdef USE_PREVIEW
	CC_SYNTHESIZE(bool, _recording, Recording);
	CC_SYNTHESIZE(bool, _playing, Playing);
    FILE *_recordFp;
    std::string _playBuf;
    int _playHead;
    int _playFrameOffset;
    class FrameDeviceData {
    public:
        int frame;
        std::list<DeviceData> deviceDataList;
    };
    std::list<FrameDeviceData> _recordedFrameDeviceDataList;
	cocos2d::__Dictionary **_gamePadAxisDic;
#endif
    std::list<DeviceData> _recordedDeviceDataList;

	friend class InputEventListener;
	friend class InputManager;
};

//-------------------------------------------------------------------------------------------------------------------
/**
 * @brief マウス、キーボード、ゲームコントローラー入力のイベントを監視する
 */
class AGTKPLAYER_API InputEventListener
{
protected:
	void registerGamepadListener(cocos2d::EventDispatcher *eventDispatcher, cocos2d::Node *node);
	void registerMouseListener(cocos2d::EventDispatcher *eventDispatcher, cocos2d::Node *node);
	void registerKeyboardListener(cocos2d::EventDispatcher *eventDispatcher, cocos2d::Node *node);
public:
	void registerInputListener(cocos2d::EventDispatcher *eventDispatcher, cocos2d::Node *node);
};

#if defined(USE_CONTROLLER_DATA)
//-------------------------------------------------------------------------------------------------------------------
/**
 * @brief マウス・キーボード、ゲームコントローラーに割り振られたIDの１つを管理する
 */
class AGTKPLAYER_API InputController : public cocos2d::Ref
{
public:
	enum EnumController {
		kControllerNone = -1,//none
		kControllerXbox360 = 0,//Xbox360
		kControllerPs4,//PlayStation4
		kControllerDirectInput,//DirectInput
		kControllerPc,//Keyboard & Mouse
		kControllerNx,	// Nintendo Switch #AGTK-NX
		kControllerMax,
	};
	//ゲームプレイ時に使用
	enum EnumOperationKey {
		None,
		A,//A
		B,//B
		X,//X
		Y,//Y
		R1,//R1
		R2,//R2
		L1,//L1
		L2,//L2
		Up,//Up
		Down,//Down
		Left,//Left
		Right,//Right
		LeftStickUp,//Left stick(Up)
		LeftStickDown,//Left stick(Down)
		LeftStickLeft,//Left stick(Left)
		LeftStickRight,//Left stick(Right)
		RightStickUp,//Right stick(Up)
		RightStickDown,//Right stick(Down)
		RightStickLeft,//Right stick(Left)
		RightStickRight,//Right stick(Right)
		Reserved1,//LeftClick,//Left click
		Reserved2,//RightClick,//Right click
		Reserved3,//START,//START
		Reserved4,//SELECT,//SELECT
		Reserved5,//HOME,//HOME
		OK,//OK
		CANCEL,//CANCEL
	};
	//予約キーコードpc
	enum EnumReservedKeyCodePc {
		kReservedKeyCodePc_None = -1,
		kReservedKeyCodePc_W = 0,
		kReservedKeyCodePc_A,
		kReservedKeyCodePc_S,
		kReservedKeyCodePc_D,
		kReservedKeyCodePc_LeftClick,
		kReservedKeyCodePc_RightClick,
		kReservedKeyCodePc_Up = 10,
		kReservedKeyCodePc_Right,
		kReservedKeyCodePc_Down,
		kReservedKeyCodePc_Left,
		kReservedKeyCodePc_MiddleClick = 22,
		kReservedKeyCodePc_WheelUp = 24,
		kReservedKeyCodePc_WhellDown = 26,
		kReservedKeyCodePc_MousePointer = 28,
		kReservedKeyCodePc_Max = 32,
	};
	enum EnumMove {
		kMoveDownLeft = 1,
		kMoveDown,
		kMoveDownRight,
		kMoveLeft,
		kMoveCenter,
		kMoveRight,
		kMoveUpLeft,
		kMoveUp,
		kMoveUpRight,
	};
public:
	class ReservedKeyCodeData : public cocos2d::Ref {
	public:
		CREATE_FUNC_PARAM2(ReservedKeyCodeData, int, id, int, keyCode);
		virtual bool init(int id, int keyCode) {
			_id = id;
			_keyCode = keyCode;
			return true;
		};
	private:
		CC_SYNTHESIZE(int, _id, Id);
		CC_SYNTHESIZE(int, _keyCode, KeyCode);
	};
private:
	InputController();
	virtual ~InputController();
public:
	static InputController *create(int id, agtk::data::InputMappingData *inputMappingData, InputDataRaw *inputDataRaw);
	void setGamepad(int gamepadNo);//-1:pc, 0～n:gamepad
	virtual void update(float delta);
private:
	bool init(int id, agtk::data::InputMappingData *inputMappingData, InputDataRaw *inputDataRaw);
public:
	bool isPressed(int button);
	bool isPressed(EnumMove move, cocos2d::Vec2 point, int button);
	bool isKeyPressed(int keyCode);
	float getValue(int keyCode);
	float getOperationKeyValue(int button);
	bool isTriggered(int button);
	bool isReleased(int button);
	bool isReleasing(int button);
	bool isNonePressedAll();
	bool isPressedOr(bool ignoreCancel);
// #AGTK-NX
#if 1
	bool isTriggeredOr(bool ignoreCancel);
#endif
	static const char *getKeyCodeNamePc(int keyCode);
	bool calcSceneMousePoint(cocos2d::Vec2& point);
private:
	cocos2d::__Array *getTemplateButtonIds(int button, bool bContainSystem = true);
	bool isPressedPc(int keyCode);
	bool isPressedPc(EnumMove move, cocos2d::Vec2 point, int keyCode);
	float getValuePc(int keyCode);
	bool isTriggeredPc(int keyCode);
	bool isReleasedPc(int keyCode);
	bool isReleasingPc(int keyCode);
	bool getReservedKeyCode(int reservedKeyCode, const char* &name, int &keyCode);
private:
	CC_SYNTHESIZE(int, _id, Id);
	CC_SYNTHESIZE(EnumController, _controller, Controller);
	CC_SYNTHESIZE(int, _gamepadNo, GamepadNo);
	CC_SYNTHESIZE_RETAIN(agtk::data::InputMappingData *, _inputMappingData, InputMappingData);
	CC_SYNTHESIZE_RETAIN(InputDataRaw *, _inputDataRaw, InputDataRaw);
	CC_SYNTHESIZE_RETAIN(InputGamepadData *, _inputGamepadData, InputGamepadData);
	CC_SYNTHESIZE_RETAIN(InputKeyboardData *, _inputKeyboardData, InputKeyboardData);
	CC_SYNTHESIZE_RETAIN(InputMouseData *, _inputMouseData, InputMouseData);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _reservedKeyCodePc, ReservedKeyCodePc);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _reservedKeyCodePcBase, ReservedKeyCodePcBase);
	CC_SYNTHESIZE(bool, _containSystemKey, ContainSystemKey);
};
#endif

//-------------------------------------------------------------------------------------------------------------------
/**
 * @brief 入力データを管理する
 */
class AGTKPLAYER_API InputManager : public cocos2d::Ref
{
public:
	enum EnumController {
		kControllerAll = -2,//全ての入力デバイス（※キーボード＆マウス、ゲームパッド）
		kControllerNone = -1,//入力デバイス無し
		kControllerKeyboardAndMouse = 0,//キーボード＆マウス
		//1～16はゲームパッド
	};
	enum EnumTypeInput {
		kTypeCustom1Input,
		kTypeCustom2Input,
		kTypeDirectInput,
		kTypeDiInput,
		kTypeInputMax,
	};
	enum EnumInputDirection {
		kDirectionLeftDown = 1,		// 1: 左下
		kDirectionDown,				// 2: 下
		kDirectionRightDown,		// 3: 右下
		kDirectionLeft,				// 4: 左
		kDirectionNeutral,			// 5: ニュートラル
		kDirectionRight,			// 6: 右
		kDirectionLeftUp,			// 7: 左上
		kDirectionUp,				// 8: 上
		kDirectionRightUp,			// 9: 右上
		kDirectionMax,
	};
private:
	InputManager();
	static InputManager *_inputManager;
public:
	virtual ~InputManager();
	static InputManager* getInstance();
	static void purge();
public:
	bool init(agtk::data::InputMappingData * inputMappingData);
public:
	const char *getInputName(int inputId);
public:
	void reset(bool isReset=false);
	void update(float delta);
	void afterUpdate(float delta);
	void setupController(int controllerId);
public:
	InputGamepadData *getGamepad(int id);
	int getGamepadCount();
	bool setIgnored(int id, bool bIgnored);
	bool isPressed(int button, int id = -1);
	bool isPressed(InputController::EnumMove move, cocos2d::Vec2 point, int button, int id = -1);
	bool isTriggered(int button, int id = -1, agtk::data::ObjectInputConditionData::EnumTriggerType type = agtk::data::ObjectInputConditionData::kTriggerJustPressed, int instanceId = -1, int acceptFrameCount = 1);
	bool isReleased(int button, int id = -1, agtk::data::ObjectInputConditionData::EnumTriggerType type = agtk::data::ObjectInputConditionData::kTriggerJustReleased, int instanceId = -1, int acceptFrameCount = 1);
	bool isReleasing(int button, int id = -1);
	bool isNoneInput(int id = -1);//※TODO
	bool isNoneInputWithin(cocos2d::__Array *inputKeys, int id = 0);//※TODO
	bool isNoneInputWithout(cocos2d::__Array *inputKeys, int id = 0);//※TODO
	bool isPressedKeyboard(int keyCode);
	bool isTriggeredKeyboard(int keyCode);
	bool isReleasedKeyboard(int keyCode);
	void setPrecedeInputJudgeData();
	void setPrecedeInputTriggered(int button, int id, agtk::data::ObjectInputConditionData::EnumTriggerType type, int instanceId);
	void setPrecedeInputReleased(int button, int id, agtk::data::ObjectInputConditionData::EnumTriggerType type, int instanceId);
	cocos2d::__Array *getPressedKeyboard();
	cocos2d::__Array *getTriggeredKeyboard();
	cocos2d::__Array *getReleasedKeyboard();
	EnumTypeInput getTypeInput(int id = 0);//※TODO
#if defined(USE_CONTROLLER_DATA)
	InputController *getController(int id);//※TODO
	InputController *getSelectController(int id);
#endif
	int getPlayControllerId(int controllerId);
#ifdef USE_PREVIEW
    bool startRecording();
    void stopRecording();
    bool isRecording();
	bool startPlaying(const char *filename);
	bool getRestartFlg(const char *filename);
    bool isReplaying();
#endif
private:
	int getInputIdByName(const char *name, int button, int id);//※TODO
private:
	CC_SYNTHESIZE_RETAIN(InputDataRaw *, _inputDataRaw, InputDataRaw);
	CC_SYNTHESIZE_READONLY(agtk::data::InputMappingData *, _inputMappingData, InputMappingData);	//retainされる
public: virtual void setInputMappingData(agtk::data::InputMappingData *inputMappingData);
#if defined(USE_CONTROLLER_DATA)
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _inputControllerList, InputControllerList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _selectInputControllerList, SelectInputControllerList);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
#endif
	CC_SYNTHESIZE(size_t, _lastControllerCount, LastControllerCount);
	CC_SYNTHESIZE(bool, _ignoreInput, IgnoreInput);//入力無効。
	friend class InputEventListener;
private:
	// 先行入力データ
	class PrecedeInputData {
	public:
		PrecedeInputData(agtk::data::ObjectInputConditionData::EnumTriggerType type, int button, int id, int instanceId) {
			_type = type;
			_button = button;
			_id = id;
			_instanceId = instanceId;
			_frame = 0.0f;
			_isRemove = false;
		}
	private:
		CC_SYNTHESIZE(agtk::data::ObjectInputConditionData::EnumTriggerType, _type, Type);
		CC_SYNTHESIZE(int, _button, Button);
		CC_SYNTHESIZE(int, _id, Id);
		CC_SYNTHESIZE(int, _instanceId, InstanceId);
		CC_SYNTHESIZE(float, _frame, Frame);
		CC_SYNTHESIZE(bool, _isRemove, IsRemove);
	};
	std::list<PrecedeInputData> _precedeInputAcceptDataList;
	std::list<PrecedeInputData> _precedeInputDataList;
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
};

#endif	//__INPUT_MANAGER_H__
