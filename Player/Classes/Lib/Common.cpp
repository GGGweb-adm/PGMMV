#include "Common.h"

// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif

#define LIMIT_DIGIT 10000.0

NS_AGTK_BEGIN //---------------------------------------------------------------------------------//

//-------------------------------------------------------------------------------------------------------------------
IfCallFunc *IfCallFunc::create(const std::function<bool()> &func)
{
	IfCallFunc *ret = new (std::nothrow) IfCallFunc();
	if (ret && ret->initWithFunction(func))
	{
		ret->autorelease();
		return ret;
	}

	CC_SAFE_DELETE(ret);
	return nullptr;
}

bool IfCallFunc::initWithFunction(const std::function<bool()> &func)
{
	_function = func;
	_result = false;
	_update = false;
	return true;
}

IfCallFunc::~IfCallFunc()
{
	_function = nullptr;
}

IfCallFunc *IfCallFunc::clone() const
{
	// no copy constructor
	auto a = new (std::nothrow) IfCallFunc();
	if (_function)
	{
		a->initWithFunction(_function);
	}

	a->autorelease();
	return a;
}

IfCallFunc *IfCallFunc::reverse() const
{
	// no reverse here, just return a clone
	return this->clone();
}

void IfCallFunc::update(float /*time*/)
{
	_update = false;
}

bool IfCallFunc::execute()
{
	bool ret = _result;
	if (_function && !_update) {
		ret = _function();
		_update = true;
		_result = ret;
	}
	return ret;
}

//-------------------------------------------------------------------------------------------------------------------
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
Sequence* SequenceIf::variadicCreate(FiniteTimeAction *action1, ...)
{
	va_list params;
	va_start(params, action1);

	Sequence *ret = Sequence::createWithVariableList(action1, params);

	va_end(params);

	return ret;
}
#else
Sequence* Sequence::create(FiniteTimeAction *action1, ...)
{
	va_list params;
	va_start(params, action1);

	Sequence *ret = Sequence::createWithVariableList(action1, params);

	va_end(params);

	return ret;
}
#endif

Sequence* Sequence::create(const cocos2d::Vector<FiniteTimeAction*>& arrayOfActions)
{
	Sequence* seq = new (std::nothrow) Sequence;
	if (seq && seq->init(arrayOfActions))
	{
		seq->autorelease();
		return seq;
	}

	delete seq;
	return nullptr;
}

Sequence* Sequence::createWithVariableList(FiniteTimeAction *action1, va_list args)
{
	Sequence* seq = new (std::nothrow) Sequence;
	if (seq && seq->initWithVariableList(action1, args))
	{
		seq->autorelease();
		return seq;
	}

	delete seq;
	return nullptr;
}

bool Sequence::init(const cocos2d::Vector<FiniteTimeAction*>& arrayOfActions)
{
	auto count = arrayOfActions.size();
	if (count == 0) {
		return false;
	}
	float duration = 0.0f;
	for (int i = 0; i < count; i++) {
		auto action = arrayOfActions.at(i);
		duration += action->getDuration();
		_actionList.pushBack(action);
	}
	this->setDuration(duration);

	float splitStart = 0.0f;
	for (int i = 0; i < count; i++) {
		auto action = arrayOfActions.at(i);
		float split = action->getDuration() / duration;

		_splitStartList.pushBack(cocos2d::__Float::create(splitStart / duration));
		splitStart += action->getDuration();

		_splitList.pushBack(cocos2d::__Float::create(split));
	}
	return true;
}

bool Sequence::initWithVariableList(FiniteTimeAction *action1, va_list args)
{
	if (action1 == nullptr) {
		return false;
	}
	_actionList.pushBack(action1);
	float duration = action1->getDuration();
	while (action1) {
		FiniteTimeAction *now = va_arg(args, FiniteTimeAction*);
		if (now == nullptr)
			break;
		duration += now->getDuration();
		_actionList.pushBack(now);
	}
	this->setDuration(duration);

	float splitStart = 0.0f;
	for (int i = 0; i < _actionList.size(); i++) {
		auto action = _actionList.at(i);
		float split = action->getDuration() / duration;

		_splitStartList.pushBack(cocos2d::__Float::create(splitStart / duration));
		splitStart += action->getDuration();

		_splitList.pushBack(cocos2d::__Float::create(split));
	}
	return true;
}

Sequence* Sequence::clone() const
{
	// no copy constructor
	if (_actionList.size() > 0) {
		return Sequence::create(_actionList);
	}
	else {
		return nullptr;
	}
}

Sequence::Sequence()
{
	_current = -1;
}

Sequence::~Sequence()
{
	_actionList.clear();
	_splitList.clear();
	_splitStartList.clear();
}

void Sequence::startWithTarget(cocos2d::Node *target)
{
	if (target == nullptr)
	{
		//log("Sequence::startWithTarget error: target is nullptr!");
		return;
	}
	if (_actionList.size() == 0) {
		return;
	}
	ActionInterval::startWithTarget(target);
	_current = 0;
	auto action = _actionList.at(_current);
	action->startWithTarget(target);
}

void Sequence::stop(void)
{
	for (int i = 0; i < _actionList.size(); i++) {
		auto action = _actionList.at(i);
		action->stop();
	}
	ActionInterval::stop();
}

void Sequence::step(float dt)
{
	if (_firstTick)
	{
		_firstTick = false;
		_elapsed = 0;
	}
	else
	{
		auto action = dynamic_cast<agtk::IfCallFunc *>(_actionList.at(_current));
		if (action && !action->execute()) {
			action->update(dt);
			return;
		}
		_elapsed += dt;
	}

	float updateDt = MAX(0,                                  // needed for rewind. elapsed could be negative
		MIN(1, _elapsed / _duration)
	);

	if (sendUpdateEventToScript(updateDt, this)) return;

	this->update(updateDt);
}

void Sequence::update(float t)
{
	float now_t = 0.0f;
	if (_current < 0) {
		return;
	}
	if (_current >= _actionList.size()) {
		return;
	}

	bool bChangeActionFlag = false;
lRetry:;
	auto splitStart = _splitStartList.at(_current);
	auto split = _splitList.at(_current);
	auto action = _actionList.at(_current);
	auto ifCallFunc = dynamic_cast<agtk::IfCallFunc *>(action);
	float splitVal = split->getValue();

	if (splitVal == 0.0f) {
		now_t = 0.0f;
	} else {
		now_t = (t - splitStart->getValue()) / split->getValue();
	}
	if (now_t >= 1.0f || action->getDuration() == 0.0f) {
		if (!ifCallFunc || ifCallFunc->execute()) {
			action->update(1.0f);
			_current++;
			if (_current >= _actionList.size()) {
				_current = _actionList.size() - 1;
				return;
			}
			//next action
			action = _actionList.at(_current);
			action->startWithTarget(_target);
			goto lRetry;
		}
	}
	if (!sendUpdateEventToScript(now_t, action)) {
		action->update(now_t);
	}
}

Sequence* Sequence::reverse() const
{
	if (_actionList.size() > 0) {
		return Sequence::create(_actionList);
	}
	return nullptr;
}

bool Sequence::isDone() const
{
	bool ret = ((_elapsed + FLT_EPSILON) >= _duration);
	if (_current == _actionList.size() - 1) {
		auto action = dynamic_cast<agtk::IfCallFunc *>(_actionList.at(_current));
		if (action) {
			return action->result();
		}
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------------------------
cocos2d::Rect Vertex4::getRect()
{
	return GetRectVertices(addr(), length());
}

bool Vertex4::intersectsPoint(agtk::Vertex4 vertex4, cocos2d::Vec2 pos)
{
	bool sign = false;
	for (int i = 0; i < vertex4.length(); i++) {
		cocos2d::Vec2 v = vertex4[i] - vertex4[(i + 1) % vertex4.length()];
		float c = v.cross(pos - vertex4[(i + 1) % vertex4.length()]);
		if (i == 0) { sign = (c > 0); }
		if ((c > 0 && sign == false) || (c < 0 && sign == true)) {
			return false;
		}
	}
	return true;
}

bool Vertex4::intersectsLine(agtk::Vertex4 vertex4, cocos2d::Vec2 v1, cocos2d::Vec2 v2)
{
	for (int i = 0; i < vertex4.length(); i++) {
		if (cocos2d::Vec2::isSegmentIntersect(vertex4[i], vertex4[(i + 1) % vertex4.length()], v1, v2)) {
			return true;
		}
	}
	return false;
}

bool Vertex4::intersectsVertex4(agtk::Vertex4 v1, agtk::Vertex4 v2)
{
	if (!v1.getRect().intersectsRect(v2.getRect())) {
		return false;
	}
	for (int i = 0; i < v1.length(); i++) {
		if (intersectsPoint(v2, v1[i])) {
			return true;
		}
		if (intersectsLine(v2, v1[i], v1[(i + 1) % v1.length()])) {
			return true;
		}
	}
	for (int i = 0; i < v2.length(); i++) {
		if (intersectsPoint(v1, v2[i])) {
			return true;
		}
		if (intersectsLine(v1, v2[i], v2[(i + 1) % v2.length()])) {
			return true;
		}
	}
	return false;
}

bool Vertex4::intersectsRect(agtk::Vertex4 v, cocos2d::Rect rect)
{
	if (!v.getRect().intersectsRect(rect)) {
		return false;
	}
	for (int i = 0; i < v.length(); i++) {
		if (rect.containsPoint(v[i])) {
			return true;
		}
	}
	//point
	if(intersectsPoint(v, cocos2d::Vec2(rect.getMinX(), rect.getMinY()))) {
		return true;
	}
	if (intersectsPoint(v, cocos2d::Vec2(rect.getMinX(), rect.getMaxY()))) {
		return true;
	}
	if (intersectsPoint(v, cocos2d::Vec2(rect.getMaxX(), rect.getMaxY()))) {
		return true;
	}
	if (intersectsPoint(v, cocos2d::Vec2(rect.getMaxX(), rect.getMinY()))) {
		return true;
	}
	//line
	if (intersectsLine(v, cocos2d::Vec2(rect.getMinX(), rect.getMinY()), cocos2d::Vec2(rect.getMinX(), rect.getMaxY()))) {
		return true;
	}
	if (intersectsLine(v, cocos2d::Vec2(rect.getMinX(), rect.getMaxY()), cocos2d::Vec2(rect.getMaxX(), rect.getMaxY()))) {
		return true;
	}
	if (intersectsLine(v, cocos2d::Vec2(rect.getMaxX(), rect.getMaxY()), cocos2d::Vec2(rect.getMaxX(), rect.getMinY()))) {
		return true;
	}
	if (intersectsLine(v, cocos2d::Vec2(rect.getMaxX(), rect.getMinY()), cocos2d::Vec2(rect.getMinX(), rect.getMinY()))) {
		return true;
	}
	return false;
}

cocos2d::Rect Vertex4::getRectMerge(std::vector<agtk::Vertex4> &vertex4List)
{
	if (vertex4List.size() == 0) {
		return cocos2d::Rect();
	}
	cocos2d::Rect rect = vertex4List[0].getRect();
	for (unsigned int i = 1; i < vertex4List.size(); i++) {
		rect.merge(vertex4List[i].getRect());
	}
	return rect;
}

//-------------------------------------------------------------------------------------------------------------------
EventTimer::EventTimer()
{
	_state = kStateIdle;
	_seconds = 0.0f;
	_timer = 0.0f;
	_startFuncList = nullptr;
	_processingFuncList = nullptr;
	_endFuncList = nullptr;
}

EventTimer::~EventTimer()
{
	CC_SAFE_RELEASE_NULL(_startFuncList);
	CC_SAFE_RELEASE_NULL(_processingFuncList);
	CC_SAFE_RELEASE_NULL(_endFuncList);
}

EventTimer *EventTimer::create()
{
	auto p = new (std::nothrow) EventTimer();
	if (p && p->init()) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool EventTimer::init()
{
	this->setStartFuncList(cocos2d::__Array::create());
	this->setProcessingFuncList(cocos2d::__Array::create());
	this->setEndFuncList(cocos2d::__Array::create());

	_timer = 0.0f;
	_seconds = 0.0f;
	_state = kStateIdle;
	return true;
}

/**
 * @brief 更新処理を行う。状態の変更、コールバック関数の呼び出しが行われる。
 * @note 終了時間に達したときのコールバック関数の呼び出しのされ方に注意：
 *		開始時は、開始時コールバック、終了時コールバックが連続して呼び出される。
 *		処理中時は、処理中コールバックが呼ばれるが、終了時コールバックは次のupdate()で呼び出される。
 */
void EventTimer::update(float dt)
{
	cocos2d::Ref *ref;
	switch (this->getState()) {
	case kStateIdle:
		break;
	case kStateStart: {
		_state = kStateProcessing;
		auto startFuncList = this->getStartFuncList();
		CCARRAY_FOREACH(startFuncList, ref) {
			auto p = dynamic_cast<agtk::Function<std::function<void()>> *>(ref);
			if (p) {
				p->getFunc()();
			}
		}
		if (_timer >= _seconds) {
			_state = kStateEnd;
			auto endFuncList = this->getEndFuncList();
			CCARRAY_FOREACH(endFuncList, ref) {
				auto p = dynamic_cast<agtk::Function<std::function<void()>> *>(ref);
				if (p) {
					p->getFunc()();
				}
			}
			break;
		}
		}
	case kStateProcessing: {
		_timer += dt;
		if (_timer > _seconds) _timer = _seconds;
		auto processingFuncList = this->getProcessingFuncList();
		CCARRAY_FOREACH(processingFuncList, ref) {
			auto p = dynamic_cast<agtk::Function<std::function<void(float)>> *>(ref);
			if (p) {
				p->getFunc()(dt);
			}
		}
		if (_timer >= _seconds) {
			this->end();
		}
		break; }
	case kStateEnd: {
		_state = kStateIdle;
		auto endFuncList = this->getEndFuncList();
		CCARRAY_FOREACH(endFuncList, ref) {
			auto p = dynamic_cast<agtk::Function<std::function<void()>> *>(ref);
			if (p) {
				p->getFunc()();
			}
		}
		break; }
	default:CC_ASSERT(0);
	}
}

/**
 * @brief 状態をStartに変更する。開始時コールバックは次のupdate()時に呼び出される。
 */
void EventTimer::start(float seconds, std::function<void()> endFunc)
{
	this->setEndFunc(endFunc);
	_state = kStateStart;
	_seconds = seconds;
	_timer = 0.0f;
}

/**
 * @brief 状態をEndに変更する。終了時コールバックは次のupdate()時に呼び出される。
 */
void EventTimer::end()
{
	_state = kStateEnd;
}

/**
 * @brief 状態がIdleでないとき真を返す。
 */
bool EventTimer::isProcessing()
{
	return _state != kStateIdle;
}

/**
 * @brief 開始時に呼び出されるコールバック関数を登録する。同じ関数は多重登録されない。
 */
void EventTimer::setStartFunc(std::function<void()> func)
{
	if (func == nullptr) {
		return;
	}

	//同じ関数かチェックする。
	auto _type = &func.target_type();
	cocos2d::Ref *ref;
	auto startFuncList = this->getStartFuncList();
	CCARRAY_FOREACH(startFuncList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<Function<std::function<void()>> *>(ref);
#else
		auto p = dynamic_cast<Function<std::function<void()>> *>(ref);
#endif
		if (&p->getFunc().target_type() == _type) {
			return;
		}
	}

	auto f = Function<std::function<void()>>::create(func);
	startFuncList->addObject(f);
}

/**
 * @brief 処理時毎に呼び出されるコールバック関数を登録する。同じ関数は多重登録されない。
 */
void EventTimer::setProcessingFunc(std::function<void(float)> func)
{
	if (func == nullptr) {
		return;
	}

	//同じ関数かチェックする。
	auto _type = &func.target_type();
	cocos2d::Ref *ref;
	auto processingFuncList = this->getProcessingFuncList();
	CCARRAY_FOREACH(processingFuncList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<Function<std::function<void()>> *>(ref);
#else
		auto p = dynamic_cast<Function<std::function<void()>> *>(ref);
#endif
		if (&p->getFunc().target_type() == _type) {
			return;
		}
	}

	//登録する
	auto f = Function<std::function<void(float)>>::create(func);
	processingFuncList->addObject(f);
}

/**
 * @brief 終了時に呼び出されるコールバック関数を登録する。同じ関数は多重登録されない。
 */
void EventTimer::setEndFunc(std::function<void()> func)
{
	if (func == nullptr) {
		return;
	}

	//同じ関数かチェックする。
	auto _type = &func.target_type();
	cocos2d::Ref *ref;
	auto endFuncList = this->getEndFuncList();
	CCARRAY_FOREACH(endFuncList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<Function<std::function<void()>> *>(ref);
#else
		auto p = dynamic_cast<Function<std::function<void()>> *>(ref);
#endif
		if (&p->getFunc().target_type() == _type) {
			return;
		}
	}

	//登録する。
	auto f = Function<std::function<void()>>::create(func);
	endFuncList->addObject(f);
}

//-------------------------------------------------------------------------------------------------------------------
AreaData::AreaData()
{
	_segments = 4;
	_vertices = new cocos2d::Vec2[_segments];
	CC_ASSERT(_vertices);
	_degrees = 0;
}

AreaData::~AreaData()
{
	delete[] _vertices;
}

AreaData *AreaData::create(cocos2d::Vec2 pos, cocos2d::Size size, cocos2d::Vec2 origin, float degrees)
{
	auto p = new (std::nothrow) AreaData();
	if (p) {
		p->_pos = pos;
		p->_size = size;
		p->_origin = origin;
		p->_degrees = degrees;
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

cocos2d::Vec2 *AreaData::calcArea()
{
	//左下
	_vertices[0] = _origin.rotateByAngle(cocos2d::Vec2::ZERO, -CC_DEGREES_TO_RADIANS(_degrees)) + _pos;
	//左上
	_vertices[1] = cocos2d::Vec2(_origin + cocos2d::Vec2(0, _size.height)).rotateByAngle(cocos2d::Vec2::ZERO, -CC_DEGREES_TO_RADIANS(_degrees)) + _pos;
	//右上
	_vertices[2] = cocos2d::Vec2(_origin + cocos2d::Vec2(_size)).rotateByAngle(cocos2d::Vec2::ZERO, -CC_DEGREES_TO_RADIANS(_degrees)) + _pos;
	//右下
	_vertices[3] = cocos2d::Vec2(_origin + cocos2d::Vec2(_size.width, 0)).rotateByAngle(cocos2d::Vec2::ZERO, -CC_DEGREES_TO_RADIANS(_degrees)) + _pos;
	//rect
	_rect = agtk::GetRectVertices(_vertices, _segments);
	return _vertices;
}

bool AreaData::intersectsPoint(AreaData *p, cocos2d::Vec2 pos)
{
	bool sign = false;
	for (unsigned int i = 0; i < p->_segments; i++) {
		cocos2d::Vec2 v = p->_vertices[i] - p->_vertices[(i + 1) % p->_segments];
		float c = v.cross(pos - p->_vertices[(i + 1) % p->_segments]);
		if (i == 0) { sign = (c > 0); }
		if ((c > 0 && sign == false) || (c < 0 && sign == true)) {
			return false;
		}
	}
	return true;
}

bool AreaData::intersectsLine(AreaData *p, cocos2d::Vec2 v1, cocos2d::Vec2 v2)
{
	for (unsigned int i = 0; i < p->_segments; i++) {
		if (cocos2d::Vec2::isSegmentIntersect(p->_vertices[i], p->_vertices[(i + 1) % p->_segments], v1, v2)) {
			return true;
		}
	}
	return false;
}

bool AreaData::intersectsArea(AreaData *p1, AreaData *p2)
{
	if (!p1->_rect.intersectsRect(p2->_rect)) {
		return false;
	}
	AreaData *smallp = nullptr;
	AreaData *largep = nullptr;
	if (cocos2d::Vec2(p1->_rect.size).getLengthSq() < cocos2d::Vec2(p2->_rect.size).getLengthSq()) {
		smallp = p1;
		largep = p2;
	}
	else {
		smallp = p2;
		largep = p1;
	}
	for (unsigned int i = 0; i < smallp->_segments; i++) {
		if (AreaData::intersectsPoint(largep, smallp->_vertices[i])) {
			return true;
		}
		if (AreaData::intersectsLine(largep, smallp->_vertices[i], smallp->_vertices[(i + 1) % smallp->_segments])) {
			return true;
		}
	}
	for (unsigned int i = 0; i < largep->_segments; i++) {
		if (AreaData::intersectsPoint(smallp, largep->_vertices[i])) {
			return true;
		}
		if (AreaData::intersectsLine(smallp, largep->_vertices[i], largep->_vertices[(i + 1) % largep->_segments])) {
			return true;
		}
	}
	return false;
}

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
cocos2d::Rect GetRectVertices(cocos2d::Vec2 *vertices, unsigned int segments)
{
	if (vertices == nullptr) {
		return cocos2d::Rect();
	}
	cocos2d::Vec2 minv = vertices[0];
	cocos2d::Vec2 maxv = vertices[0];
	for (unsigned int i = 1; i < segments; i++) {
		if (vertices[i].x < minv.x) {
			minv.x = vertices[i].x;
		}
		if (vertices[i].x > maxv.x) {
			maxv.x = vertices[i].x;
		}
		if (vertices[i].y < minv.y) {
			minv.y = vertices[i].y;
		}
		if (vertices[i].y > maxv.y) {
			maxv.y = vertices[i].y;
		}
	}
	return cocos2d::Rect(minv.x, minv.y, maxv.x - minv.x, maxv.y - minv.y);
}

std::string GuidToString(GUID guid)
{
	std::string s;
	char tmp[16];

	sprintf(tmp, "%08X", guid.Data1);
	s += tmp;
	s += "-";
	sprintf(tmp, "%04X", guid.Data2);
	s += tmp;
	s += "-";
	sprintf(tmp, "%04X", guid.Data3);
	s += tmp;
	s += "-";
	for (int i = 0; i < 2; i++) {
		sprintf(tmp, "%02X", guid.Data4[i]);
		s += tmp;
	}
	s += "-";
	for (int i = 2; i < 8; i++) {
		sprintf(tmp, "%02X", guid.Data4[i]);
		s += tmp;
	}
	return s;
}

GUID StringToGuid(std::string str)
{
	int offset = 0, oldOffset = 0;
	GUID guid;
	std::string tmp;
	char *endptr = nullptr;

	//Data1
	oldOffset = offset;
	offset = str.find("-", offset);
	tmp = str.substr(oldOffset, offset);
	guid.Data1 = (unsigned long)strtoul(tmp.c_str(), &endptr, 16);

	//Data2
	oldOffset = offset + 1;
	offset = str.find("-", offset + 1);
	tmp = str.substr(oldOffset, offset - oldOffset);
	guid.Data2 = (short)strtol(tmp.c_str(), &endptr, 16);

	//Data3
	oldOffset = offset + 1;
	offset = str.find("-", offset + 1);
	tmp = str.substr(oldOffset, offset - oldOffset);
	guid.Data3 = (short)strtol(tmp.c_str(), &endptr, 16);

	//Data4[0-1]
	oldOffset = offset + 1;
	offset = str.find("-", offset + 1);
	tmp = str.substr(oldOffset, offset - oldOffset);
	for (int i = 0; i < 2; i++) {
		std::string v;
		v = tmp.substr(i * 2, 2);
		guid.Data4[i] = (unsigned char)strtol(v.c_str(), &endptr, 16);
	}
	//Data4[2-7]
	oldOffset = offset + 1;
	offset = str.find("-", offset + 1);
	tmp = str.substr(oldOffset, offset - oldOffset);
	for (int i = 2; i < 8; i++) {
		std::string v;
		v = tmp.substr((i - 2) * 2, 2);
		guid.Data4[i] = (unsigned char)strtol(v.c_str(), &endptr, 16);
	}

	return guid;
}

//指定文字がSJIS漢字の第一バイトのとき 1 を返す。そうでなければ 0 を返す。
int IsSjisHead(unsigned char c)
{
	if (c < 0x81 || (c >= 0xa0 && c <= 0xdf) || c >= 0xfd) {
		/* 非漢字 */
		return 0;
	}
	return 1;
}

inline int GetUtf8LetterLen(const unsigned char c)
{
	if (c <= 0x7f) {
		return 1;
	}
	if (c <= 0xdf) {
		//２バイト文字。
		return 2;
	}
	if (c <= 0xef) {
		//３バイト文字。
		return 3;
	}
	if (c <= 0xf7) {
		//４バイト文字。
		return 4;
	}
	//	EB_ASSERT(0);
	return 1;
}

int GetStringLength(std::string str)
{
	int length = 0;
	int char_size = 0;
	for (int pos = 0; pos < (int)str.length(); pos += char_size) {
		char_size = GetUtf8LetterLen(str[pos]);
		length++;
	}
	return length;
}

cocos2d::Vec2 GetCenterOfGravity(cocos2d::Vec2 pos, cocos2d::Size size)
{
	cocos2d::Vec2 v;
	v += pos;
	v += pos + cocos2d::Vec2(size);
	v += pos + cocos2d::Vec2(size.width, 0);
	v += pos + cocos2d::Vec2(0, size.height);
	v = v / 4;
	return v;
}

bool IsSegmentIntersect(const agtk::Line& line1, const agtk::Line& line2)
{
	return cocos2d::Vec2::isSegmentIntersect(line1.p, line1.v, line2.p, line2.v);
}

cocos2d::Vec2 GetIntersectPoint(const agtk::Line& line1, const agtk::Line& line2)
{
	float S, T;
	if (cocos2d::Vec2::isLineIntersect(line1.p, line1.v, line2.p, line2.v, &S, &T) && (S >= 0.0f && S <= 1.0f && T >= 0.0f && T <= 1.0f)) {
		// Vec2 of intersection
		cocos2d::Vec2 P;
		P.x = line1.p.x + S * (line1.v.x - line1.p.x);
		P.y = line1.p.y + S * (line1.v.y - line1.p.y);
		return P;
	}

	return cocos2d::Vec2::ZERO;
}

bool GetWallBit(cocos2d::Rect r1, cocos2d::Rect r2, int *wallBit1, int *wallBit2)
{
	//up:    0x0001
	//left:  0x0010
	//right: 0x0100
	//down:  0x1000

	if (r1.intersectsRect(r2)) {
		//r1
		agtk::Line up1(cocos2d::Vec2(r1.origin.x, r1.origin.y + r1.size.height), cocos2d::Vec2(r1.origin.x + r1.size.width, r1.origin.y + r1.size.height));
		agtk::Line left1(cocos2d::Vec2(r1.origin.x, r1.origin.y), cocos2d::Vec2(r1.origin.x, r1.origin.y + r1.size.height));
		agtk::Line right1(cocos2d::Vec2(r1.origin.x + r1.size.width, r1.origin.y), cocos2d::Vec2(r1.origin.x + r1.size.width, r1.origin.y + r1.size.height));
		agtk::Line down1(cocos2d::Vec2(r1.origin.x, r1.origin.y), cocos2d::Vec2(r1.origin.x + r1.size.width, r1.origin.y));
		//r2
		agtk::Line up2(cocos2d::Vec2(r2.origin.x, r2.origin.y + r2.size.height), cocos2d::Vec2(r2.origin.x + r2.size.width, r2.origin.y + r2.size.height));
		agtk::Line left2(cocos2d::Vec2(r2.origin.x, r2.origin.y), cocos2d::Vec2(r2.origin.x, r2.origin.y + r2.size.height));
		agtk::Line right2(cocos2d::Vec2(r2.origin.x + r2.size.width, r2.origin.y), cocos2d::Vec2(r2.origin.x + r2.size.width, r2.origin.y + r2.size.height));
		agtk::Line down2(cocos2d::Vec2(r2.origin.x, r2.origin.y), cocos2d::Vec2(r2.origin.x + r2.size.width, r2.origin.y));

		if (wallBit1) {
			int wb1 = 0;
			wb1 |= (IsSegmentIntersect(up1, left2) || IsSegmentIntersect(up1, right2)) << 0;
			wb1 |= (IsSegmentIntersect(left1, up2) || IsSegmentIntersect(left1, down2)) << 1;
			wb1 |= (IsSegmentIntersect(right1, up2) || IsSegmentIntersect(right1, down2)) << 2;
			wb1 |= (IsSegmentIntersect(down1, left2) || IsSegmentIntersect(down1, right2)) << 3;
			*wallBit1 = wb1;
		}
		if (wallBit2) {
			int wb2 = 0;
			wb2 |= (IsSegmentIntersect(up2, left1) || IsSegmentIntersect(up2, right1)) << 0;
			wb2 |= (IsSegmentIntersect(left2, up1) || IsSegmentIntersect(left2, down1)) << 1;
			wb2 |= (IsSegmentIntersect(right2, up1) || IsSegmentIntersect(right2, down1)) << 2;
			wb2 |= (IsSegmentIntersect(down2, left1) || IsSegmentIntersect(down2, right1)) << 3;
			*wallBit2 = wb2;
		}
		return true;
	}
	return false;
}

int GetMoveDirectionId(double angle)
{
	//角度から移動方向を数字キー方向の８分割して値を返す。
	//
    //         上
	//    +---+---+---+
	//    | 7 | 8 | 9 |
	//    +---+---+---+
	// 左 | 4 |   | 6 | 右
	//    +---+---+---+
	//    | 1 | 2 | 3 |
	//    +---+---+---+
	//         下

	angle = GetDegree360(angle);
	//右上
	if (22.5f <= angle && angle < 67.5f) {
		return 9;
	}
	//右
	else if (67.5f <= angle && angle < 112.5f) {
		return 6;
	}
	//右下
	else if (112.5f <= angle && angle < 157.5f) {
		return 3;
	}
	//下
	else if (157.5f <= angle && angle < 202.5f) {
		return 2;
	}
	//左下
	else if (202.5f <= angle && angle < 247.5f) {
		return 1;
	}
	//左
	else if (247.5f <= angle && angle < 292.5f) {
		return 4;
	}
	//左上
	else if (292.5f <= angle && angle < 337.5f) {
		return 7;
	}
	//上
	else if (337.5f <= angle && angle <= 360.0f || 0.0f <= angle && angle < 22.5f) {
		return 8;
	}
	CC_ASSERT(0);
	return -1;
}

cocos2d::Vec2 GetDirectionFromDegrees(double degrees)
{
	cocos2d::Vec2 v(0, 1);
	auto vv = v.rotateByAngle(cocos2d::Vec2::ZERO, -CC_DEGREES_TO_RADIANS(degrees));
	//小数点第４以下を切り捨て。
	double vx = int(vv.x * LIMIT_DIGIT) / LIMIT_DIGIT;
	double vy = int(vv.y * LIMIT_DIGIT) / LIMIT_DIGIT;
	return cocos2d::Vec2(vx, vy).getNormalized();
}

cocos2d::Vec2 GetRotateByAngle(cocos2d::Vec2 v, double angle)
{
	cocos2d::Vec2 vv = v.rotateByAngle(cocos2d::Vec2::ZERO, angle);
	//小数点第４以下を切り捨て。
	double vx = int(vv.x * LIMIT_DIGIT) / LIMIT_DIGIT;
	double vy = int(vv.y * LIMIT_DIGIT) / LIMIT_DIGIT;
	return cocos2d::Vec2(vx, vy).getNormalized();
}

cocos2d::Vec2 GetDirectionFromMoveDirectionId(int moveDirectionId)
{
	cocos2d::Vec2 vec = cocos2d::Vec2::ZERO;
	//x方向
	if (moveDirectionId == 1 || moveDirectionId == 4 || moveDirectionId == 7) {
		vec.x = -1;
	}
	else if (moveDirectionId == 3 || moveDirectionId == 6 || moveDirectionId == 9) {
		vec.x = 1;
	}
	//y方向
	if (1 <= moveDirectionId && moveDirectionId <= 3) {
		vec.y = -1;
	}
	else if (7 <= moveDirectionId && moveDirectionId <= 9) {
		vec.y = 1;
	}
	return vec;
}

float GetDegreeFromVector(cocos2d::Vec2 v)
{
	cocos2d::Vec2 vv = cocos2d::Vec2(0, 1);
	float angle = vv.getAngle(v);
	float degree = -CC_RADIANS_TO_DEGREES(angle);
	if (degree < 0) degree = 360.0f + degree;
	return degree;
}

std::string UTF8toSjis(std::string srcUTF8)
{
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	//Unicodeへ変換後の文字列長を得る
	int lenghtUnicode = MultiByteToWideChar(CP_UTF8, 0, srcUTF8.c_str(), srcUTF8.size() + 1, NULL, 0);

	//必要な分だけUnicode文字列のバッファを確保
	wchar_t* bufUnicode = new wchar_t[lenghtUnicode];

	//UTF8からUnicodeへ変換
	MultiByteToWideChar(CP_UTF8, 0, srcUTF8.c_str(), srcUTF8.size() + 1, bufUnicode, lenghtUnicode);

	//ShiftJISへ変換後の文字列長を得る
	int lengthSJis = WideCharToMultiByte(CP_THREAD_ACP, 0, bufUnicode, -1, NULL, 0, NULL, NULL);

	//必要な分だけShiftJIS文字列のバッファを確保
	char* bufShiftJis = new char[lengthSJis];

	//UnicodeからShiftJISへ変換
	WideCharToMultiByte(CP_THREAD_ACP, 0, bufUnicode, lenghtUnicode + 1, bufShiftJis, lengthSJis, NULL, NULL);

	std::string strSJis(bufShiftJis);

	delete bufUnicode;
	delete bufShiftJis;

	return strSJis;
#endif
}

cocos2d::Texture2D *CreateTexture2D(const char *filename, bool bTiling, unsigned char **pBuffer, float *pWidth, float *pHeight)
{
	//２のべき乗のテクスチャサイズのテクスチャを作成する。
	std::function<int(int)> pow2 = [](int n) {
		if (n < 0) return 0;
		if (n == 0) return 1;
		if (!(n & (n - 1))) {
			return n;
		}
		int _n = 2;
		while (1) {
			_n *= 2;
			if (_n >= n) break;
		}
		return _n;
	};

	auto img = new cocos2d::Image();
	img->initWithImageFile(filename);
	int width = img->getWidth();
	int height = img->getHeight();
	if (pWidth) { *pWidth = width; }
	if (pHeight) { *pHeight = height; }
	if (bTiling == false) {//タイリング以外。
		auto texture2d = new cocos2d::Texture2D();
		texture2d->initWithImage(img);
		delete img;
		return texture2d;
	}
	int nw = pow2(width);
	int nh = pow2(height);
	int pn = img->getBitPerPixel() / 8;
	if (pn == 0) {
		CC_ASSERT(0);//TODO:未対応。
		auto texture2d = new cocos2d::Texture2D();
		texture2d->initWithImage(img);
		delete img;
		return texture2d;
	}
	auto data = new unsigned char[nw * nh * pn];
	memset(data, 0, nw * nh * pn);
	for (int i = 0; i < img->getHeight(); i++) {
		for (int j = 0; j < img->getWidth(); j++) {
			memcpy(&data[(i * nw + j) * pn], &img->getData()[(i * img->getWidth() + j) * pn], pn);
		}
	}
	auto texture2d = new cocos2d::Texture2D();
	texture2d->initWithData(data, nw * nh * pn, img->getRenderFormat(), nw, nh, cocos2d::Size(nw, nh));
	//((cocos2d::Size&)texture2d->getContentSizeInPixels()).setSize((float)width, (float)height);//これによりEnumBgImagePlacementのすべての処理がうまくいく。強制処理が気になる。
	if (pBuffer != nullptr) { *pBuffer = data; }
	else { CC_SAFE_DELETE_ARRAY(data); }
	CC_SAFE_DELETE(img);
	return texture2d;
}

int CalcDirectionId(int bit)
{
	int direction = 0;
	//移動方向の指定が無い場合は、表示方向を取得する。
	//※ただ、複数方向を指定している場合は、方向番号の小さいほうから優先される。
	//指定(moveDirectionId < 0)が無い場合。
	//表示方向を取得する
	for (int i = 1; i <= 9; i++) {
		if (bit & (1 << i)) {
			direction = i;
			break;
		}
	}
	return direction;
}

float GetDegree360(float degree)
{
	float v = (degree >= 0.0f) ? fmod(degree, 360.0f) : (360.0f - fmod(-degree, 360.0f));
	if (v == -0.0f) {
		//マイナス0がデバッグ表示されると見た目が悪いので0にする。
		v = 0;
	}
	return v;
}

NS_AGTK_END //-----------------------------------------------------------------------------------//
