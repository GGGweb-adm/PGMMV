#ifndef __COMMON_H__
#define	__COMMON_H__

#include "cocos2d.h"
#include "2d/CCActionInterval.h"
#include "json/document.h"
#include "Lib/Macros.h"
#include <chrono>

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
//[***Data]のベースクラスとして使いたい。
//目的：データのセーブロードに使う。データ階層はプロジェクトデータと同じにする。
//[***Data]には、プロジェクトデータ以外に、プレイデータを入れる。
//ルール：[***Data]のプロジェクトデータは１つのアンダーバーから始めるデータ。
//　　　　プレイデータは、３つのアンダーバーから始まるデータ。
//プロジェクトデータ：project.json
//プレイデータ：ゲームプレイする時に必要となるデータ

class AGTKPLAYER_API BaseData : public cocos2d::Ref
{
protected:
	virtual bool serialize(rapidjson::Value& json, rapidjson::Document::AllocatorType& allocator) = 0;
	virtual bool deserialize(const rapidjson::Value& json) = 0;
};

//-------------------------------------------------------------------------------------------------------------------
struct AGTKPLAYER_API Vertex4
{
	Vertex4(){ }
	Vertex4(cocos2d::Vec2 p0, cocos2d::Vec2 p1, cocos2d::Vec2 p2, cocos2d::Vec2 p3) {
		this->p0 = p0; this->p1 = p1; this->p2 = p2; this->p3 = p3;
	}
	cocos2d::Vec2& operator[](int index) {
		cocos2d::Vec2 *v = (cocos2d::Vec2 *)&p0;
		CC_ASSERT(0 <= index && index < 4);
		return v[index];
	}
	cocos2d::Vec2 *addr() { return (cocos2d::Vec2 *)&p0; };
	int length() { return 4; };
	Vertex4& operator+=(cocos2d::Vec2 v) {
		p0 += v; p1 += v; p2 += v; p3 += v;
		return (*this);
	}
	Vertex4& operator-=(cocos2d::Vec2 v) {
		p0 -= v; p1 -= v; p2 -= v; p3 -= v;
		return (*this);
	}
	cocos2d::Rect getRect();
	void Zero() {
		p0 = p1 = p2 = p3 = cocos2d::Vec2::ZERO;
	}
	cocos2d::Vec2 p0, p1, p2, p3;
	bool operator==(const Vertex4 &data) {
		return (this->p0 == data.p0 && this->p1 == data.p1 && this->p2 == data.p2 && this->p3 == data.p3);
	}
	bool operator!=(const Vertex4 &data) { return !(*this == data); }
#if defined(AGTK_DEBUG)
	void dump() {
		CCLOG("vertex -----------");
		CCLOG("p0:%f,%f", p0.x, p0.y);
		CCLOG("p1:%f,%f", p1.x, p1.y);
		CCLOG("p2:%f,%f", p2.x, p2.y);
		CCLOG("p3:%f,%f", p3.x, p3.y);
		CCLOG("size:%f,%f", p1.x - p0.x, p2.y - p1.y);
	}
#endif
public:
	static bool intersectsPoint(agtk::Vertex4 vertex4, cocos2d::Vec2 pos);
	static bool intersectsLine(agtk::Vertex4 vertex4, cocos2d::Vec2 v1, cocos2d::Vec2 v2);
	static bool intersectsVertex4(agtk::Vertex4 v1, agtk::Vertex4 v2);
	static bool intersectsRect(agtk::Vertex4 v, cocos2d::Rect rect);
	static cocos2d::Rect getRectMerge(std::vector<agtk::Vertex4> &vertex4List);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API Line {
public:
	Line(cocos2d::Vec2 p, cocos2d::Vec2 v) {
		this->p = p;
		this->v = v;
	};
	Line& operator&() { return (*this); };

	cocos2d::Vec2 p;
	cocos2d::Vec2 v;
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API IfCallFunc : public cocos2d::ActionInstant
{
public:
	static IfCallFunc * create(const std::function<bool()>& func);

public:
	/** Executes the callback.
	*/
	virtual bool execute();

	//
	// Overrides
	//
	/**
	* @param time In seconds.
	*/
	virtual void update(float time) override;
	virtual IfCallFunc* reverse() const override;
	virtual IfCallFunc* clone() const override;
	virtual bool result() const { return _result; };

CC_CONSTRUCTOR_ACCESS:
	IfCallFunc()
		: _function(nullptr)
	{
	}
	virtual ~IfCallFunc();

	bool initWithFunction(const std::function<bool()>& func);

protected:
	/** function that will be called */
	std::function<bool()> _function;
	bool _result;
	bool _update;

private:
	CC_DISALLOW_COPY_AND_ASSIGN(IfCallFunc);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API Sequence : public cocos2d::ActionInterval
{
public:
	/** Helper constructor to create an array of sequenceable actions.
	*
	* @return An autoreleased Sequence object.
	*/
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	// VS2013 does not support nullptr in variable args lists and variadic templates are also not supported
	typedef FiniteTimeAction* M;
	static Sequence* create(M m1, std::nullptr_t listEnd) { return variadicCreate(m1, NULL); }
	static Sequence* create(M m1, M m2, std::nullptr_t listEnd) { return variadicCreate(m1, m2, NULL); }
	static Sequence* create(M m1, M m2, M m3, std::nullptr_t listEnd) { return variadicCreate(m1, m2, m3, NULL); }
	static Sequence* create(M m1, M m2, M m3, M m4, std::nullptr_t listEnd) { return variadicCreate(m1, m2, m3, m4, NULL); }
	static Sequence* create(M m1, M m2, M m3, M m4, M m5, std::nullptr_t listEnd) { return variadicCreate(m1, m2, m3, m4, m5, NULL); }
	static Sequence* create(M m1, M m2, M m3, M m4, M m5, M m6, std::nullptr_t listEnd) { return variadicCreate(m1, m2, m3, m4, m5, m6, NULL); }
	static Sequence* create(M m1, M m2, M m3, M m4, M m5, M m6, M m7, std::nullptr_t listEnd) { return variadicCreate(m1, m2, m3, m4, m5, m6, m7, NULL); }
	static Sequence* create(M m1, M m2, M m3, M m4, M m5, M m6, M m7, M m8, std::nullptr_t listEnd) { return variadicCreate(m1, m2, m3, m4, m5, m6, m7, m8, NULL); }
	static Sequence* create(M m1, M m2, M m3, M m4, M m5, M m6, M m7, M m8, M m9, std::nullptr_t listEnd) { return variadicCreate(m1, m2, m3, m4, m5, m6, m7, m8, m9, NULL); }
	static Sequence* create(M m1, M m2, M m3, M m4, M m5, M m6, M m7, M m8, M m9, M m10, std::nullptr_t listEnd) { return variadicCreate(m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, NULL); }

	// On WP8 for variable argument lists longer than 10 items, use the other create functions or variadicCreate with NULL as the last argument
	static Sequence* variadicCreate(FiniteTimeAction* item, ...);
#else
	static Sequence* create(cocos2d::FiniteTimeAction *action1, ...) CC_REQUIRES_NULL_TERMINATION;
#endif
	static Sequence* create(const cocos2d::Vector<FiniteTimeAction*>& arrayOfActions);
	static Sequence* createWithVariableList(FiniteTimeAction *action1, va_list args);

	//
	// Overrides
	//
	virtual Sequence* clone() const override;
	virtual Sequence* reverse() const override;
	virtual void startWithTarget(cocos2d::Node *target) override;
	virtual void stop(void) override;
	/**
	* @param t In seconds.
	*/
	virtual void step(float t) override;
	virtual void update(float t) override;
	virtual bool isDone(void) const override;

CC_CONSTRUCTOR_ACCESS:
	Sequence();
	virtual ~Sequence();

	/** initializes the action */
	bool init(const cocos2d::Vector<FiniteTimeAction*>& arrayOfActions);
	bool initWithVariableList(FiniteTimeAction *action1, va_list args);

protected:
	int _current;
	cocos2d::Vector<FiniteTimeAction *> _actionList;
	cocos2d::Vector<cocos2d::__Float *> _splitList;
	cocos2d::Vector<cocos2d::__Float *> _splitStartList;

private:
	CC_DISALLOW_COPY_AND_ASSIGN(Sequence);
};

//-------------------------------------------------------------------------------------------------------------------
/**
 * @brief 関数オブジェクトを保持するためのクラス。
 */
template <typename T>
class AGTKPLAYER_API Function : public cocos2d::Ref
{
protected:
	Function() {
		_func = nullptr;
	}
	virtual ~Function() {}
public:
	CREATE_FUNC_PARAM(Function, T, _func);
private:
	virtual bool init(T func) {
		this->setFunc(func);
		return true;
	}
private:
	CC_SYNTHESIZE(T, _func, Func);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API Timer : public cocos2d::Ref
{
public:
	enum EnumState {
		kStateIdle,
		kStateStart,
		kStateStop,
	};
private:
	Timer() { }
	virtual ~Timer() { }
public:
	CREATE_FUNC(Timer);
	void start() {
		_begin = getCurrentClock();
		_state = kStateStart;
	}
	void stop() {
		if (_state != kStateStart) {
			return;
		}
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(getCurrentClock() - _begin).count();
		if (_max < elapsed) _max = elapsed;
		if (_min > elapsed) _min = elapsed;
		_last = elapsed;
		_sum += elapsed;
		_state = kStateStop;
		_count++;
	}
	void laptime() {
		if (_state == kStateIdle) {
			return;
		}
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(getCurrentClock() - _begin).count();
		_begin = getCurrentClock();
		if (_max < elapsed) _max = elapsed;
		if (_min > elapsed) _min = elapsed;
		_last = elapsed;
		_sum += elapsed;
		_count++;
	}
	void reset() {
		_state = kStateIdle;
		_begin = std::chrono::time_point<std::chrono::system_clock>();
		_sum = _max = _last = 0;
		_min = 0x7FFFFFFFFFFFFFFF;
		_count = 0;
	}
	float getSeconds() { return (float)_sum / 1000000.0f; };
	float getAverageSeconds() { return getSeconds() / (float)getCount(); };
	float getMinSeconds() { return (float)_min / 1000000.0f; }
	float getMaxSeconds() { return (float)_max / 1000000.0f; }
	float getLastSeconds() { return (float)_last / 1000000.0f; }
	unsigned long getCount() { return _count; }
	EnumState getState() { return _state; }
	void dump() {
		cocos2d::log("------------------------------");
		cocos2d::log("Total Sec: %f", getSeconds());
		cocos2d::log("Last Sec: %f", getLastSeconds());
		cocos2d::log("MinMax Sec: %f,%f", getMinSeconds(), getMaxSeconds());
		cocos2d::log("Average Sec(%lu): %f", getCount(), getAverageSeconds());
	}
private:
	virtual bool init() {
		reset();
		return true;
	}
	std::chrono::time_point<std::chrono::system_clock> getCurrentClock() {
		return std::chrono::system_clock::now();
	}
private:
	EnumState _state;
	std::chrono::time_point<std::chrono::system_clock> _begin;
	long long _sum;
	long long _min;
	long long _max;
	long long _last;
	unsigned long _count;
};

//-------------------------------------------------------------------------------------------------------------------
/**
 * @brief 開始時、終了時、処理中時にコールバック関数を呼び出すタイマーを管理。
 */
class AGTKPLAYER_API EventTimer : public cocos2d::Ref
{
public:
	enum EnumState {
		kStateIdle,
		kStateStart,
		kStateProcessing,
		kStateEnd,
	};
protected:
	EventTimer();
	virtual ~EventTimer();
public:
	static EventTimer *create();
	virtual void update(float dt);
	virtual void start(float seconds, std::function<void()> endFunc = nullptr);
	virtual void end();
	void setStartFunc(std::function<void()> func);
	void setProcessingFunc(std::function<void(float)> func);
	void setEndFunc(std::function<void()> func);
	bool isProcessing();
protected:
	virtual bool init();
private:
	CC_SYNTHESIZE_READONLY(EnumState, _state, State);
	float _timer;
	CC_SYNTHESIZE(float, _seconds, Seconds);
protected:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _startFuncList, StartFuncList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _processingFuncList, ProcessingFuncList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _endFuncList, EndFuncList);
};

//-------------------------------------------------------------------------------------------------------------------
template <typename T>
class ValueTimer : public cocos2d::Ref
{
public:
	enum EnumState {
		kStateIdle,//待機
		kStateStart,//開始
		kStateExecuting,//実行中
		kStatePause,//一時停止
		kStateResume,//再開
		kStateEnd,//終了
	};
protected:
	ValueTimer() {
		onInterpolateFunc = nullptr;
		_initializedFlag = false;
	}
	virtual ~ValueTimer() {}
public:
	CREATE_FUNC_PARAM(ValueTimer, T, value);
	virtual void update(float dt) {
		switch (this->getState()) {
		case kStateIdle:
			break;
		case kStateStart:
			_state = kStateExecuting;
			_oldValue = _value;
			_value = _nextValue;
			if (_timer >= _seconds) {
				_state = kStateEnd;
				break;
			}
		case kStateExecuting://実行中
			_timer += dt;
			if (_timer > _seconds) _timer = _seconds;
			_oldValue = _value;
			if (onInterpolateFunc) {
				_value = onInterpolateFunc(_prevValue, _nextValue, _seconds, _timer);
			}
			if (_timer >= _seconds) {
				this->end();
			}
			break;
		case kStatePause://一時停止
			break;
		case kStateResume://再開
			_state = kStateExecuting;
			break;
		case kStateEnd:
			_state = kStateIdle;
			break;
		default:CC_ASSERT(0);
		}
	}
	virtual void start(T value, float seconds) {
		auto state = this->getState();
		if (state == kStateIdle) {
			if (value == _value) {
				return;
			}
		}
		else {
			if (value == _nextValue) {
				return;
			}
		}
		if (seconds == 0.0f) {
			_value = value;
		}
		_nextValue = value;
		_prevValue = _value;
		_state = kStateStart;
		_seconds = seconds;
		_timer = 0.0f;
	}
	virtual void start(T value, T initValue, float seconds) {
		init(initValue);
		start(value, seconds);
	}
	virtual void start(float seconds) {
		_state = kStateStart;
		_seconds = seconds;
		_timer = 0.0f;
	}
	virtual void end() {
		_state = kStateEnd;
		_oldValue = _value;
		_value = _nextValue;
	}
	void pause() { _state = kStatePause; };//一時停止
	void resume() { _state = kStateResume; }//再開
	bool isExecuting() { return _state != kStateIdle; };
protected:
	virtual bool init(T value) {
		_timer = 0.0f;
		_seconds = 0.0f;
		_state = kStateIdle;
		_value = value;
		_prevValue = value;
		_nextValue = value;
		_oldValue = value;
		_initializedFlag = true;
		return true;
	}
private:
	CC_SYNTHESIZE_READONLY(EnumState, _state, State);
	float _timer;
	float _seconds;
	CC_SYNTHESIZE(T, _value, Value);
	CC_SYNTHESIZE(T, _nextValue, NextValue);
	CC_SYNTHESIZE(T, _prevValue, PrevValue);
	CC_SYNTHESIZE(T, _oldValue, OldValue);
	CC_SYNTHESIZE(bool, _initializedFlag, InitializedFlag);
public:
	std::function<T(T/*startValue*/, T/*endValue*/, float/*range*/, float/*pos*/)> onInterpolateFunc;
};

//-------------------------------------------------------------------------------------------------------------------
struct AGTKPLAYER_API AreaData : public cocos2d::Ref
{
private:
	AreaData();
	virtual ~AreaData();
public:
	static AreaData *create(cocos2d::Vec2 pos, cocos2d::Size size, cocos2d::Vec2 origin, float degrees);
	cocos2d::Vec2 *calcArea();
	cocos2d::Vec2 *getVertices() { return _vertices; };
	cocos2d::Vec2 getVertice1() { return _vertices[0]; };
	cocos2d::Vec2 getVertice2() { return _vertices[1]; };
	cocos2d::Vec2 getVertice3() { return _vertices[2]; };
	cocos2d::Vec2 getVertice4() { return _vertices[3]; };
	cocos2d::Rect getRect() { return _rect; };
	cocos2d::Size getSize() { return _size; };
	cocos2d::Vec2 getPosition() { return _pos; };
	unsigned int getSegments() { return _segments; };
public:
	static bool intersectsPoint(AreaData *p, cocos2d::Vec2 pos);
	static bool intersectsLine(AreaData *p, cocos2d::Vec2 v1, cocos2d::Vec2 v2);
	static bool intersectsArea(AreaData *p1, AreaData *p2);
private:
	cocos2d::Vec2 _origin;
	cocos2d::Vec2 _pos;
	cocos2d::Size _size;
	float _degrees;

	cocos2d::Vec2 *_vertices;
	unsigned int _segments;
	cocos2d::Rect _rect;
};

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
cocos2d::Rect GetRectVertices(cocos2d::Vec2 *vertices, unsigned int segments);
std::string GuidToString(GUID guid);
GUID StringToGuid(std::string str);
int GetStringLength(std::string str);
cocos2d::Vec2 GetCenterOfGravity(cocos2d::Vec2 pos, cocos2d::Size size);
bool IsSegmentIntersect(const agtk::Line& line1, const agtk::Line& line2);
cocos2d::Vec2 GetIntersectPoint(const agtk::Line& line1, const agtk::Line& line2);
bool GetWallBit(cocos2d::Rect r1, cocos2d::Rect r2, int *wallBit1, int *wallBit2);
int GetMoveDirectionId(double angle);
cocos2d::Vec2 GetDirectionFromDegrees(double degrees);
cocos2d::Vec2 GetDirectionFromMoveDirectionId(int moveDirectionId);
float GetDegreeFromVector(cocos2d::Vec2 v);
cocos2d::Vec2 GetRotateByAngle(cocos2d::Vec2 v, double angle);
std::string UTF8toSjis(std::string srcUTF8);
cocos2d::Texture2D *CreateTexture2D(const char *filename, bool bTiling, unsigned char **pBuffer, float *pWidth = nullptr, float *pHeight = nullptr);
int CalcDirectionId(int bit);
float GetDegree360(float degree);

NS_AGTK_END

#endif	//__COMMON_H__
