#include "PrimitiveManager.h"
#include "AppMacros.h"
#include "Lib/Common.h"

//-------------------------------------------------------------------------------------------------------------------
PrimitiveNode::PrimitiveNode() : cocos2d::DrawNode(1)
{
	_color = cocos2d::Color4F::WHITE;
	_fillColor = cocos2d::Color4F(0, 0, 0, 0);
	_update = false;
	_primTimer = nullptr;
	memset(&_data, 0, sizeof(_data));
}

PrimitiveNode::~PrimitiveNode()
{
	if (_type == Type::Polygon) {
		CC_SAFE_DELETE_ARRAY(_data.polygon.vertices);
	}
	CC_SAFE_RELEASE_NULL(_primTimer);
}

PrimitiveNode *PrimitiveNode::createWithType(PrimitiveNode::Type type)
{
	PrimitiveNode *p = new (std::nothrow) PrimitiveNode();
	if (p && p->initWithType(type)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool PrimitiveNode::initWithType(PrimitiveNode::Type type)
{
	if (!DrawNode::init()) {
		return false;
	}
	this->_type = type;
	this->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
	return true;
}

void PrimitiveNode::setDisk(float x, float y, float radius)
{
	CC_ASSERT(_type == Type::Disk);
	_data.disk.x = x;
	_data.disk.y = y;
	_data.disk.radius = radius;
	setPosition(_data.disk.x, _data.disk.y);
	_update = true;
}

void PrimitiveNode::setLine(float x1, float y1, float x2, float y2)
{
	CC_ASSERT(_type == Type::Line);
	_data.line.x1 = x1;
	_data.line.y1 = y1;
	_data.line.x2 = x2;
	_data.line.y2 = y2;
	setPosition(_data.line.x1, _data.line.y1);
	_update = true;
}

void PrimitiveNode::setRectangle(float x, float y, float w, float h)
{
	CC_ASSERT(_type == Type::Rectangle);
	_data.rect.x = x;
	_data.rect.y = y;
	_data.rect.w = w;
	_data.rect.h = h;
	setPosition(_data.rect.x, _data.rect.y);
	_update = true;
}

void PrimitiveNode::setCircle(float x, float y, float radius)
{
	CC_ASSERT(_type == Type::Circle);
	_data.circle.x = x;
	_data.circle.y = y;
	_data.circle.radius = radius;
	setPosition(_data.circle.x, _data.circle.y);
	_update = true;
}

void PrimitiveNode::setPlate(float x, float y, float w, float h)
{
	CC_ASSERT(_type == Type::Plate);
	_data.plate.x = x;
	_data.plate.y = y;
	_data.plate.w = w;
	_data.plate.h = h;
	setPosition(_data.plate.x, _data.plate.y);
	_update = true;
}

void PrimitiveNode::setEllipse(float x, float y, float rx, float ry, float angle)
{
	CC_ASSERT(_type == Type::Ellipse);
	_data.ellipse.x = x;
	_data.ellipse.y = y;
	_data.ellipse.rx = rx;
	_data.ellipse.ry = ry;
	_data.ellipse.angle = angle;
	setPosition(_data.ellipse.x, _data.ellipse.y);
	_update = true;
}

void PrimitiveNode::setPolygon(float x, float y, cocos2d::Vec2 *vertices, unsigned int segments)
{
	CC_ASSERT(_type == Type::Polygon);
	_data.polygon.x = x;
	_data.polygon.y = y;
	if (vertices == nullptr) {
		return;
	}
	if (_data.polygon.segments != segments) {
		if (_data.polygon.vertices) {
			CC_SAFE_DELETE_ARRAY(_data.polygon.vertices);
		}
		_data.polygon.segments = 0;
		_data.polygon.vertices = new (std::nothrow) cocos2d::Vec2[segments];
	}
	for (unsigned int i = 0; i < segments; i++) {
		_data.polygon.vertices[i] = vertices[i];
	}
	_data.polygon.segments = segments;
	setPosition(_data.polygon.x, _data.polygon.y);
	_update = true;
}

void PrimitiveNode::setRGBA(float r, float g, float b, float a)
{
	_color.r = r;
	_color.g = g;
	_color.b = b;
	_color.a = a;
	if (_type == Type::Polygon && _data.polygon.segments == 0) {
		return;
	}
	_update = true;
}

void PrimitiveNode::setFillRGBA(float r, float g, float b, float a)
{
	_fillColor.r = r;
	_fillColor.g = g;
	_fillColor.b = b;
	_fillColor.a = a;
	if (_type == Type::Polygon && _data.polygon.segments == 0) {
		return;
	}
	_update = true;
}

void PrimitiveNode::getDisk(float *x, float *y, float *radius)
{
	CC_ASSERT(_type == Type::Disk);
	*x = _data.disk.x;
	*y = _data.disk.y;
	*radius = _data.disk.radius;
}

void PrimitiveNode::getLine(float *x1, float *y1, float *x2, float *y2)
{
	CC_ASSERT(_type == Type::Line);
	*x1 = _data.line.x1;
	*y1 = _data.line.y1;
	*x2 = _data.line.x2;
	*y2 = _data.line.y2;
}

void PrimitiveNode::getRectangle(float *x, float *y, float *w, float *h)
{
	CC_ASSERT(_type == Type::Rectangle);
	*x = _data.rect.x;
	*y = _data.rect.y;
	*w = _data.rect.w;
	*h = _data.rect.h;
}

void PrimitiveNode::getCircle(float *x, float *y, float *radius)
{
	CC_ASSERT(_type == Type::Circle);
	*x = _data.circle.x;
	*y = _data.circle.y;
	*radius = _data.circle.radius;
}

void PrimitiveNode::getPlate(float *x, float *y, float *w, float *h)
{
	CC_ASSERT(_type == Type::Plate);
	*x = _data.plate.x;
	*y = _data.plate.y;
	*w = _data.plate.w;
	*h = _data.plate.h;
}

void PrimitiveNode::getPolygon(cocos2d::Vec2 **vertices, unsigned int *segments)
{
	CC_ASSERT(_type == Type::Polygon);
	*vertices = _data.polygon.vertices;
	*segments = _data.polygon.segments;
}

void PrimitiveNode::setTimer(double duration)
{
	auto primTimer = PrimitiveTimer::create(duration);
	this->setPrimTimer(primTimer);
}

//--------------------------------------------------------------------------------------------------------------------
// シングルトンのインスタンス
//--------------------------------------------------------------------------------------------------------------------
PrimitiveManager* PrimitiveManager::_primitiveManager = NULL;

//--------------------------------------------------------------------------------------------------------------------
// コンストラクタ
//--------------------------------------------------------------------------------------------------------------------
PrimitiveManager::PrimitiveManager()
{
	_primList = nullptr;
	_timer = 0.0;
}

//--------------------------------------------------------------------------------------------------------------------
// デストラクタ
//--------------------------------------------------------------------------------------------------------------------
PrimitiveManager::~PrimitiveManager()
{
	CC_SAFE_RELEASE_NULL(_primList);
}

//--------------------------------------------------------------------------------------------------------------------
// インスタンスを取得する。
//--------------------------------------------------------------------------------------------------------------------
PrimitiveManager* PrimitiveManager::getInstance()
{
	if (!_primitiveManager)
	{
		_primitiveManager = new PrimitiveManager();
		_primitiveManager->init();
	}
	return _primitiveManager;
}

//--------------------------------------------------------------------------------------------------------------------
// シングルトンを破棄する。
//--------------------------------------------------------------------------------------------------------------------
void PrimitiveManager::purge()
{
	if (!_primitiveManager)
		return;

	// スケジューラーを停止する。
	//Scheduler *s = Director::getInstance()->getScheduler();
	//s->unscheduleUpdate(_primitiveManager);

	PrimitiveManager *pm = _primitiveManager;
	_primitiveManager = NULL;
	pm->release();
}

void PrimitiveManager::init()
{
	this->setPrimList(cocos2d::__Array::create());

	// スケジューラーを登録する。
	//Scheduler *s = Director::getInstance()->getScheduler();
	//s->scheduleUpdate(this, kSchedulePriorityPrimitiveManager, false);
}

PrimitiveNode *PrimitiveManager::createDisk(float x, float y, float radius, cocos2d::Color4F color)
{
	PrimitiveNode *node = PrimitiveNode::createWithType(PrimitiveNode::Type::Disk);
	node->setDisk(x, y, radius);
	node->setRGBA(color.r, color.g, color.b, color.a);
	node->setContentSize(cocos2d::Size(radius, radius));
	node->setAnchorPoint(cocos2d::Vec2(0.0f, 0.0f));
	this->getPrimList()->addObject(node);
	return node;
}

PrimitiveNode *PrimitiveManager::createLine(float x1, float y1, float x2, float y2, cocos2d::Color4F color)
{
	PrimitiveNode *node = PrimitiveNode::createWithType(PrimitiveNode::Type::Line);
	node->setLine(x1, y1, x2, y2);
	node->setRGBA(color.r, color.g, color.b, color.a);
	this->getPrimList()->addObject(node);
	return node;
}

PrimitiveNode *PrimitiveManager::createRectangle(float x, float y, float width, float height, cocos2d::Color4F color)
{
	PrimitiveNode *node = PrimitiveNode::createWithType(PrimitiveNode::Type::Rectangle);
	node->setRectangle(x, y, width, height);
	node->setRGBA(color.r, color.g, color.b, color.a);
	node->setContentSize(cocos2d::Size(width, height));
	this->getPrimList()->addObject(node);
	return node;
}

PrimitiveNode *PrimitiveManager::createCircle(float x, float y, float radius, cocos2d::Color4F color)
{
	PrimitiveNode *node = PrimitiveNode::createWithType(PrimitiveNode::Type::Circle);
	node->setCircle(x, y, radius);
	node->setRGBA(color.r, color.g, color.b, color.a);
	node->setContentSize(cocos2d::Size(radius, radius));
	node->setAnchorPoint(cocos2d::Vec2(0.0f, 0.0f));
	this->getPrimList()->addObject(node);
	return node;
}

PrimitiveNode *PrimitiveManager::createPlate(float x, float y, float width, float height, cocos2d::Color4F color)
{
	PrimitiveNode *node = PrimitiveNode::createWithType(PrimitiveNode::Type::Plate);
	node->setPlate(x, y, width, height);
	node->setRGBA(color.r, color.g, color.b, color.a);
	node->setContentSize(cocos2d::Size(width, height));
//	node->setAnchorPoint(cocos2d::Vec2(0.0f, 0.0f));
	this->getPrimList()->addObject(node);
	return node;
}

PrimitiveNode *PrimitiveManager::createEllipse(float x, float y, float rx, float ry, float angle, cocos2d::Color4F color)
{
	PrimitiveNode *node = PrimitiveNode::createWithType(PrimitiveNode::Type::Ellipse);
	node->setEllipse(x, y, rx, ry, angle);
	node->setRGBA(color.r, color.g, color.b, color.a);
	node->setContentSize(cocos2d::Size(rx * 2.0f, ry * 2.0f));
	node->setAnchorPoint(cocos2d::Vec2(0.0f, 0.0f));
	this->getPrimList()->addObject(node);
	return node;
}

PrimitiveNode *PrimitiveManager::createPolygon(float x, float y, cocos2d::Vec2 *vertices, unsigned int segments, cocos2d::Color4F color, cocos2d::Color4F fillColor)
{
	PrimitiveNode *node = PrimitiveNode::createWithType(PrimitiveNode::Type::Polygon);
	node->setPolygon(x, y, vertices, segments);
	node->setRGBA(color.r, color.g, color.b, color.a);
	node->setFillRGBA(fillColor.r, fillColor.g, fillColor.b, fillColor.a);
	cocos2d::Rect rect = agtk::GetRectVertices(vertices, segments);
	node->setContentSize(rect.size);
	node->setAnchorPoint(cocos2d::Vec2(0.0f, 0.0f));
	this->getPrimList()->addObject(node);
	return node;
}

void PrimitiveManager::removeAll()
{
	this->getPrimList()->removeAllObjects();
}

void PrimitiveManager::remove(cocos2d::Node *node)
{
	auto primList = this->getPrimList();
	CC_ASSERT(primList);
	if (primList->getIndexOfObject(node) != UINT_MAX) {
		primList->removeObject(node);
	}
}

void PrimitiveManager::removeFromParent(cocos2d::Node *node)
{
	bool bParent = (node->getParent() != nullptr);
	this->remove(node);
	if (bParent) {
		node->removeFromParent();
	}
}

void PrimitiveManager::setTimer(PrimitiveNode *node, double duration)
{
	node->setTimer(duration);
}

void PrimitiveManager::update(float delta)
{
	this->setTimer(this->getTimer() + delta);
	cocos2d::Ref *itr = nullptr;

	auto primList = this->getPrimList();
	bool bRemove = false;
	do {
		bRemove = false;
		CCARRAY_FOREACH(primList, itr) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			PrimitiveNode *node = static_cast<PrimitiveNode *>(itr);
#else
			PrimitiveNode *node = dynamic_cast<PrimitiveNode *>(itr);
#endif
			auto primTimer = node->getPrimTimer();
			if (primTimer) {
				if (!primTimer->isDuration()) {
					this->removeFromParent(node);
					bRemove = true;
					break;
				}
			}
		}
	} while (bRemove);

	CCARRAY_FOREACH(primList, itr){
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		PrimitiveNode *node = static_cast<PrimitiveNode *>(itr);
#else
		PrimitiveNode *node = dynamic_cast<PrimitiveNode *>(itr);
#endif
		auto primTimer = node->getPrimTimer();
		if (primTimer) {
			primTimer->update(delta);
		}
		// ノードの更新フラグがOFF または ノードが非表示の場合
		if(!node->_update || !node->isVisible()){
			// スキップ
			continue;
		}
		node->clear();
		switch(node->_type){
		case PrimitiveNode::Type::Disk:
			node->drawDot(cocos2d::Vec2(0, 0), node->_data.disk.radius, node->_color);
			break;
		case PrimitiveNode::Type::Line:
			node->drawLine(cocos2d::Vec2(0, 0), cocos2d::Vec2(node->_data.line.x2 - node->_data.line.x1, node->_data.line.y2 - node->_data.line.y1), node->_color);
			break;
		case PrimitiveNode::Type::Rectangle:
			node->drawRect(cocos2d::Vec2(0, 0), cocos2d::Vec2(node->_data.rect.w, node->_data.rect.h), node->_color);
			break;
		case PrimitiveNode::Type::Circle:
			node->drawCircle(cocos2d::Vec2(0, 0), node->_data.circle.radius, 0, 360, false, 1, 1, node->_color);
			break;
		case PrimitiveNode::Type::Plate:
			node->drawSolidRect(cocos2d::Vec2(0, 0), cocos2d::Vec2(node->_data.plate.w, node->_data.plate.h), node->_color);
			break;
		case PrimitiveNode::Type::Ellipse:
			node->drawEllipse(cocos2d::Vec2(0, 0), node->_data.ellipse.rx, node->_data.ellipse.ry, 0.0f, 1.0f, 1.0f, 360, node->_color);
			break;
		case PrimitiveNode::Type::Polygon:
			node->drawPolygon(node->_data.polygon.vertices, node->_data.polygon.segments, node->_fillColor, 0.5, node->_color);
			break;
		}
		node->_update = false;
	}
}
