#include "Collision.h"
#include "Lib/Common.h"
#include "Lib/Object.h"
#include "Manager/GameManager.h"
#include "Manager/PrimitiveManager.h"

NS_AGTK_BEGIN

float TILE_COLLISION_THRESHOLD = 0.6f;

void setTileCollisionThreshold(float threshold)
{
	TILE_COLLISION_THRESHOLD = threshold;
}

//-------------------------------------------------------------------------------------------------------------------
PolygonShape::PolygonShape()
{
	_position = cocos2d::Vec2::ZERO;
	_vertices = nullptr;
	_segments = 0;
	_rect = cocos2d::Rect::ZERO;
	_angle = 0.0f;
	_initPosition = _position;
	_initVertices = _vertices;
}


PolygonShape::~PolygonShape()
{
	CC_SAFE_DELETE_ARRAY(_vertices);
	CC_SAFE_DELETE_ARRAY(_initVertices);
}


PolygonShape *PolygonShape::create(cocos2d::Vec2 pos, const cocos2d::Vec2 *vertices, unsigned int segments)
{
	PolygonShape *p = new (std::nothrow) PolygonShape();
	if (p) {
		p->set(pos, vertices, segments);
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

PolygonShape *PolygonShape::createCircle(cocos2d::Vec2 pos, float radius, unsigned int segments)
{
	PolygonShape *p = new (std::nothrow) PolygonShape();
	if (p && p->initCircle(pos, radius, segments)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

PolygonShape *PolygonShape::createEllipse(cocos2d::Vec2 pos, float radiusX, float radiusY, float angle, unsigned int segments)
{
	PolygonShape *p = new (std::nothrow) PolygonShape();
	if (p && p->initEllipse(pos, radiusX, radiusY, angle, segments)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

PolygonShape *PolygonShape::createRectangle(cocos2d::Rect rect, float angle)
{
	PolygonShape *p = new (std::nothrow) PolygonShape();
	cocos2d::Vec2 pos = rect.origin + rect.size * 0.5f;
	if(p && p->initRectangle(pos, rect.size, angle)){
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

PolygonShape *PolygonShape::createRectangle(cocos2d::Vec2 pos, cocos2d::Size size, float angle)
{
	PolygonShape *p = new (std::nothrow) PolygonShape();
	if (p && p->initRectangle(pos, size, angle)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

PolygonShape *PolygonShape::createRectangle(cocos2d::Vec2 v1, cocos2d::Vec2 v2, cocos2d::Vec2 v3, cocos2d::Vec2 v4, float angle)
{
	PolygonShape *p = new (std::nothrow) PolygonShape();
	if (p && p->initRectangle(v1, v2, v3, v4, angle)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

PolygonShape *PolygonShape::createRectangleCenterPoint(cocos2d::Vec2 pos, cocos2d::Size size, cocos2d::Vec2 centerPoint, float angle)
{
	auto p = new (std::nothrow) PolygonShape();
	if (p && p->initRectangleCenterPoint(pos, size, centerPoint, angle)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

PolygonShape *PolygonShape::createFan(cocos2d::Vec2 pos, float radius, float angle, float arcAngle, unsigned int segments)
{
	PolygonShape *p = new (std::nothrow) PolygonShape();
	if (p && p->initFan(pos, radius, angle, arcAngle, 1.0f, 1.0f, segments)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

PolygonShape *PolygonShape::createFan(cocos2d::Vec2 pos, float radius, float angle, float arcAngle, float scaleX, float scaleY, unsigned int segments)
{
	PolygonShape *p = new (std::nothrow) PolygonShape();
	if (p && p->initFan(pos, radius, angle, arcAngle, scaleX, scaleY, segments)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}


PolygonShape *PolygonShape::createTriangle(cocos2d::Vec2 v1, cocos2d::Vec2 v2, cocos2d::Vec2 v3, float angle)
{
	PolygonShape *p = new (std::nothrow) PolygonShape();
	if (p && p->initTriangle(v1, v2, v3, angle)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

void PolygonShape::set(cocos2d::Vec2 pos, const cocos2d::Vec2 *vertices, unsigned int segments)
{
	this->reset();
	CC_ASSERT(segments >= 3);
	_vertices = new (std::nothrow) cocos2d::Vec2[segments];
	CC_ASSERT(_vertices);
	for (unsigned int i = 0; i < segments; i++) {
		_vertices[i] = vertices[i];
	}
	_segments = segments;
	_position = pos;
	_angle = 0.0f;
	_rect = this->getRect();

	_initAngle = _angle;
	CC_SAFE_DELETE_ARRAY(_initVertices);
	this->copy(&_initVertices, _initPosition);
}

void PolygonShape::reset()
{
	CC_SAFE_DELETE_ARRAY(_vertices);
	_position = cocos2d::Vec2::ZERO;
	_segments = 0;
	_angle = 0.0f;
	_rect = cocos2d::Rect::ZERO;
}

bool PolygonShape::initCircle(cocos2d::Vec2 pos, float radius, unsigned int segments)
{
	CC_ASSERT(segments >= 3);
	_vertices = new (std::nothrow) cocos2d::Vec2[segments];
	CC_ASSERT(_vertices);

	const float coef = 2.0f * (float)M_PI / segments;
	for (unsigned int i = 0; i < segments; i++) {
		float rads = i * coef;
		_vertices[i].x = radius * cosf(rads) + pos.x;
		_vertices[i].y = radius * sinf(rads) + pos.y;
	}
	_position = pos;
	_segments = segments;
	_angle = 0.0f;
	_rect = this->getRect();

	_initAngle = _angle;
	this->copy(&_initVertices, _initPosition);
	return true;
}

bool PolygonShape::initEllipse(cocos2d::Vec2 pos, float radiusX, float radiusY, float angle, unsigned int segments)
{
	CC_ASSERT(segments >= 3);
	_vertices = new (std::nothrow) cocos2d::Vec2[segments];
	CC_ASSERT(_vertices);

	const float coef = 2.0f * (float)M_PI / segments;
	float rads, distance, a;
	for (unsigned int i = 0; i < segments; i++) {
		rads = i * coef;
		distance = sqrt(pow(sinf(rads) * radiusY, 2) + pow(cosf(rads) * radiusX, 2));
		a = atan2(sinf(rads) * radiusY, cosf(rads) * radiusX);
		_vertices[i].x = distance * cosf(a + angle) + pos.x;
		_vertices[i].y = distance * sinf(a + angle) + pos.y;
	}
	_position = pos;
	_segments = segments;
	_angle = angle;
	_rect = this->getRect();

	_initAngle = _angle;
	this->copy(&_initVertices, _initPosition);
	return true;
}

bool PolygonShape::initRectangle(cocos2d::Vec2 pos, cocos2d::Size size, float angle)
{
	unsigned int segments = 4;
	_vertices = new (std::nothrow) cocos2d::Vec2[segments];
	CC_ASSERT(_vertices);

	cocos2d::Rect rect(pos - size * 0.5f, size);
	_vertices[0].x = rect.getMinX();
	_vertices[0].y = rect.getMinY();
	_vertices[1].x = rect.getMaxX();
	_vertices[1].y = rect.getMinY();
	_vertices[2].x = rect.getMaxX();
	_vertices[2].y = rect.getMaxY();
	_vertices[3].x = rect.getMinX();
	_vertices[3].y = rect.getMaxY();

	//回転
	for (unsigned int i = 0; i < segments; i++) {
		_vertices[i] -= pos;
		_vertices[i] = _vertices[i].rotate(cocos2d::Vec2::forAngle(CC_DEGREES_TO_RADIANS(angle)));
		_vertices[i] += pos;
	}

	_position = pos;
	_segments = segments;
	_angle = angle;
	_rect = this->getRect();

	_initAngle = _angle;
	this->copy(&_initVertices, _initPosition);
	return true;
}

bool PolygonShape::initRectangle(cocos2d::Vec2 v1, cocos2d::Vec2 v2, cocos2d::Vec2 v3, cocos2d::Vec2 v4, float angle)
{
	unsigned int segments = 4;
	_vertices = new (std::nothrow) cocos2d::Vec2[segments];
	CC_ASSERT(_vertices);

	_vertices[0] = v1;
	_vertices[1] = v2;
	_vertices[2] = v3;
	_vertices[3] = v4;

	cocos2d::Vec2 pos;
	for (unsigned int i = 0; i < segments; i++) {
		pos += _vertices[i];
	}
	pos.x = pos.x / segments;
	pos.y = pos.y / segments;

	//回転
	for (unsigned int i = 0; i < segments; i++) {
		_vertices[i] -= pos;
		_vertices[i] = _vertices[i].rotate(cocos2d::Vec2::forAngle(CC_DEGREES_TO_RADIANS(angle)));
		_vertices[i] += pos;
	}

	_position = pos;
	_segments = segments;
	_angle = angle;
	_rect = this->getRect();

	_initAngle = _angle;
	this->copy(&_initVertices, _initPosition);
	return true;
}

bool PolygonShape::initRectangleCenterPoint(cocos2d::Vec2 pos, cocos2d::Size size, cocos2d::Vec2 centerPoint, float angle)
{
	unsigned int segments = 4;
	_vertices = new (std::nothrow) cocos2d::Vec2[segments];
	CC_ASSERT(_vertices);

	cocos2d::Rect rect(pos - centerPoint, size);
	_vertices[0].x = rect.getMinX();
	_vertices[0].y = rect.getMinY();
	_vertices[1].x = rect.getMaxX();
	_vertices[1].y = rect.getMinY();
	_vertices[2].x = rect.getMaxX();
	_vertices[2].y = rect.getMaxY();
	_vertices[3].x = rect.getMinX();
	_vertices[3].y = rect.getMaxY();

	//回転
	for (unsigned int i = 0; i < segments; i++) {
		_vertices[i] -= pos;
		_vertices[i] = _vertices[i].rotate(cocos2d::Vec2::forAngle(CC_DEGREES_TO_RADIANS(angle)));
		_vertices[i] += pos;
	}

	_position = pos;
	_segments = segments;
	_angle = angle;
	_rect = this->getRect();

	_initAngle = _angle;
	this->copy(&_initVertices, _initPosition);
	return true;
}

bool PolygonShape::initFan(cocos2d::Vec2 pos, float radius, float angle, float arcAngle, float scaleX, float scaleY, unsigned int segments)
{
	CC_ASSERT(segments >= 3);
	_vertices = new (std::nothrow) cocos2d::Vec2[segments + 1];
	CC_ASSERT(_vertices);

	_vertices[0] = pos;
	const float srads = 1.0f * (float)M_PI * (angle - arcAngle * 0.5f) / 180.0f;
	const float coef = 2.0f * (float)M_PI * (arcAngle / 360.0f) / (segments - 1);
	for (unsigned int i = 0; i <= segments - 1; i++) {
		float rads = srads + i * coef;
		_vertices[i + 1].x = radius * scaleX * cosf(rads) + pos.x;
		_vertices[i + 1].y = radius * scaleY * sinf(rads) + pos.y;
	}
	_position = pos;
	_segments = segments + 1;
	_angle = angle;
	_rect = this->getRect();

	_initAngle = _angle;
	this->copy(&_initVertices, _initPosition);
	return true;
}

bool PolygonShape::initTriangle(cocos2d::Vec2 v1, cocos2d::Vec2 v2, cocos2d::Vec2 v3, float angle)
{
	unsigned int segments = 3;
	_vertices = new (std::nothrow) cocos2d::Vec2[segments];
	_vertices[0] = v1;
	_vertices[1] = v2;
	_vertices[2] = v3;
	//回転
	cocos2d::Vec2 pos = v1;
	for (unsigned int i = 0; i < segments; i++) {
		_vertices[i] -= pos;
		_vertices[i] = _vertices[i].rotate(cocos2d::Vec2::forAngle(CC_DEGREES_TO_RADIANS(angle)));
		_vertices[i] += pos;
	}
	_position = v1;
	_segments = segments;
	_angle = angle;
	_rect = this->getRect();

	_initAngle = _angle;
	this->copy(&_initVertices, _initPosition);
	return true;
}

cocos2d::Rect PolygonShape::getRect()
{
	return agtk::GetRectVertices(_vertices, _segments);
}

void PolygonShape::copy(cocos2d::Vec2 **vertices, cocos2d::Vec2 &pos)
{
	*vertices = new (std::nothrow) cocos2d::Vec2[_segments];
	for (unsigned int i = 0; i < _segments; i++) {
		(*vertices)[i] = _vertices[i];
	}
	pos = _position;
}

bool PolygonShape::intersectsPoint(PolygonShape *p, cocos2d::Vec2 pos)
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

bool PolygonShape::intersectsLine(PolygonShape *p, cocos2d::Vec2 v1, cocos2d::Vec2 v2)
{
	for (unsigned int i = 0; i < p->_segments; i++) {
		if(cocos2d::Vec2::isSegmentIntersect(p->_vertices[i], p->_vertices[(i + 1) % p->_segments], v1, v2)) {
			return true;
		}
	}
	return false;
}

bool PolygonShape::intersectsRect(PolygonShape *p, cocos2d::Rect rect)
{
	if (!p->_rect.intersectsRect(rect)) {
		return false;
	}
	auto pp = PolygonShape::createRectangle(rect, 0.0f);
	return PolygonShape::intersectsPolygonShape(p, pp);
}

bool PolygonShape::intersectsTriangle(PolygonShape *p, cocos2d::Vec2 v1, cocos2d::Vec2 v2, cocos2d::Vec2 v3)
{
	auto pp = PolygonShape::createTriangle(v1, v2, v3, 0.0f);
	return PolygonShape::intersectsPolygonShape(p, pp);
}

bool PolygonShape::intersectsPolygonShape(PolygonShape *p1, PolygonShape *p2)
{
	if (!p1->_rect.intersectsRect(p2->_rect)) {
		return false;
	}
	PolygonShape *smallp = nullptr;
	PolygonShape *largep = nullptr;
	if (cocos2d::Vec2(p1->_rect.size).getLengthSq() < cocos2d::Vec2(p2->_rect.size).getLengthSq()) {
		smallp = p1;
		largep = p2;
	}
	else {
		smallp = p2;
		largep = p1;
	}

	for (unsigned int i = 0; i < smallp->_segments; i++) {
		if (PolygonShape::intersectsPoint(largep, smallp->_vertices[i])) {
			return true;
		}
		if (PolygonShape::intersectsLine(largep, smallp->_vertices[i], smallp->_vertices[(i + 1) % smallp->_segments])) {
			return true;
		}
	}
	for (unsigned int i = 0; i < largep->_segments; i++) {
		if (PolygonShape::intersectsPoint(smallp, largep->_vertices[i])) {
			return true;
		}
		if (PolygonShape::intersectsLine(smallp, largep->_vertices[i], largep->_vertices[(i + 1) % largep->_segments])) {
			return true;
		}
	}
	return false;
}

bool PolygonShape::intersectsFunPolygonShape(PolygonShape *fan, PolygonShape *ps)
{
	if (!fan->_rect.intersectsRect(ps->_rect)) {
		return false;
	}
	for (unsigned int i = 0; i < fan->_segments; i++) {
		if (PolygonShape::intersectsPoint(ps, fan->_vertices[i])) {
			return true;
		}
		if (PolygonShape::intersectsLine(ps, fan->_vertices[i], fan->_vertices[(i + 1) % fan->_segments])) {
			return true;
		}
	}
	for (unsigned int i = 0; i < ps->_segments; i++) {
		for (unsigned int j = 0; j < fan->_segments - 2; j++) {
			if (PolygonShape::intersectsTriangle(ps, fan->_position, fan->_vertices[j + 1], fan->_vertices[j + 2])) {
				return true;
			}
		}
	}
	return false;
}

bool PolygonShape::intersectsFanPoint(PolygonShape *fan, cocos2d::Vec2 pos)
{
	cocos2d::Vec2 v[3];
	unsigned int segments = 3;
	if (!fan->getRect().containsPoint(pos)) {
		return false;
	}
	for (unsigned int j = 0; j < fan->_segments - 2; j++) {
		v[0] = fan->_position;
		v[1] = fan->_vertices[j + 1];
		v[2] = fan->_vertices[j + 2];
		bool ret = true;
		for (unsigned int i = 0; i < segments; i++) {
			cocos2d::Vec2 vv = v[i] - v[(i + 1) % segments];
			float c = vv.cross(pos - v[(i + 1) % segments]);
			if (c > 0) {
				ret = false;
			}
		}
		if (ret) return true;
	}
	return false;
}

bool PolygonShape::intersectsFanPoint(const PolygonShape *fan, float radiusX, float radiusY, const cocos2d::Vec2& pos)
{
	cocos2d::Vec2 v = cocos2d::Vec2::ZERO;
	cocos2d::Vec2 c = cocos2d::Vec2::ZERO;

	//範囲外のチェック。
	v.set(fan->_position.x - pos.x, fan->_position.y - pos.y);
	float radius = (radiusX >= radiusY) ? radiusX : radiusY;
	if (radius * radius < v.getLengthSq()) {
		return false;
	}

	unsigned int size = fan->_segments - 2;
	for (unsigned int j = 0; j < size; ++j) {
		v.set(fan->_position.x - fan->_vertices[j + 1].x, fan->_position.y - fan->_vertices[j + 1].y);
		c.set(pos.x - fan->_vertices[j + 1].x, pos.y - fan->_vertices[j + 1].y);
		if (v.cross(c) > 0) {
			continue;
		}
		v.set(fan->_vertices[j + 1].x - fan->_vertices[j + 2].x, fan->_vertices[j + 1].y - fan->_vertices[j + 2].y);
		c.set(pos.x - fan->_vertices[j + 2].x, pos.y - fan->_vertices[j + 2].y);
		if (v.cross(c) > 0) {
			continue;
		}
		v.set(fan->_vertices[j + 2].x - fan->_position.x, fan->_vertices[j + 2].y - fan->_position.y);
		c.set(pos.x - fan->_position.x, pos.y - fan->_position.y);
		if (v.cross(c) > 0) {
			continue;
		}
		return true;
	}
	return false;
}

#ifdef USE_FAST_INTERSECTS_FAN_POINT
bool PolygonShape::intersectsFanPoint(const cocos2d::Vec2& center, float angle, float arcAngle, float radiusX, float radiusY, const cocos2d::Vec2& pos)
{
	cocos2d::Vec2 v = cocos2d::Vec2::ZERO;
	cocos2d::Vec2 c = cocos2d::Vec2::ZERO;

	//範囲外のチェック。
	if (radiusX < 0 || radiusY < 0) {
		return false;
	}
	auto targetVec = pos - center;
	auto targetXyVec = cocos2d::Vec2(targetVec.x / radiusX, targetVec.y / radiusY);
	auto unitRadSq = targetXyVec.lengthSquared();
	if (unitRadSq > 1.0f) {
		return false;
	}

	if (arcAngle >= 360.0f) {
		return true;
	}
	auto baseRad = (float)M_PI * angle / 180.0f;
	auto baseVec = cocos2d::Vec2(std::cosf(baseRad), std::sinf(baseRad));
	//	const float srads = 1.0f * (float)M_PI * (angle - arcAngle * 0.5f) / 180.0f;
	auto baseTargetCross = baseVec.cross(targetXyVec);
	if (baseTargetCross >= 0.0f) {
		auto upperRad = (float)M_PI * (angle + arcAngle * 0.5f) / 180.0f;
		auto upperVec = cocos2d::Vec2(std::cosf(upperRad), std::sinf(upperRad));
		auto targetUpperCross = targetXyVec.cross(upperVec);
		if (targetUpperCross >= 0.0f) {
			return true;
		}
		return false;
	}
	else {
		auto lowerRad = (float)M_PI * (angle - arcAngle * 0.5f) / 180.0f;
		auto lowerVec = cocos2d::Vec2(std::cosf(lowerRad), std::sinf(lowerRad));
		auto targetLowerCross = targetXyVec.cross(lowerVec);
		if (targetLowerCross <= 0.0f) {
			return true;
		}
		return false;
	}
}
#endif

void PolygonShape::setPosition(cocos2d::Vec2 pos)
{
	for (unsigned int i = 0; i < _segments; i++) {
		_vertices[i] -= _position;
		_vertices[i] += pos;
	}
	_position = pos;
	_rect = this->getRect();
}

void PolygonShape::setAngle(float angle)
{
	for (unsigned int i = 0; i < _segments; i++) {
		cocos2d::Vec2 v = _initVertices[i] - _initPosition;
		auto vv = v.rotateByAngle(cocos2d::Vec2::ZERO, -CC_DEGREES_TO_RADIANS(angle));
		_vertices[i] = vv + _position;
	}
	_angle = _initAngle + angle;
}

#if defined(AGTK_DEBUG)
void PolygonShape::dump()
{
	CCLOG("vertices:%d", _segments);
	for (unsigned int i = 0; i < _segments; i++) {
		CCLOG("%d:%f,%f", i, _vertices[i].x, _vertices[i].y);
	}
	CCLOG("rect:%f,%f,%f,%f", _rect.origin.x, _rect.origin.y, _rect.size.width, _rect.size.height);
	CCLOG("pos:%f,%f", _position.x, _position.y);
	CCLOG("angle:%f", _angle);
}
#endif

//-------------------------------------------------------------------------------------------------------------------
Shape *Shape::createWithRectangle(float x, float y, float w, float h, float angle)
{
	auto p = new (std::nothrow) agtk::Rectangle(x, y, w, h, angle);
	CC_ASSERT(p);
	return p;
}

Shape *Shape::createWithCircle(float x, float y, float radius)
{
	auto p = new (std::nothrow) agtk::Circle();
	CC_ASSERT(p);
	return p;
}

Shape *Shape::createWithEllipse(float x, float y, float rx, float ry, float angle)
{
	auto p = new (std::nothrow) agtk::Ellipse(x, y, rx, ry, angle);
	CC_ASSERT(p);
	return p;
}

//-------------------------------------------------------------------------------------------------------------------
Rectangle::Rectangle()
{
	_type = Shape::Rect;
	_rect = cocos2d::Rect::ZERO;
	_angle = 0.0f;
}

Rectangle::Rectangle(cocos2d::Rect rect, float angle)
{
	_type = Shape::Rect;
	_rect = rect;
	_angle = angle;
}

Rectangle::Rectangle(float x, float y, float w, float h, float angle)
{
	_type = Shape::Rect;
	_rect.setRect(x, y, w, h);
	_angle = angle;
}

Rectangle::Rectangle(cocos2d::Vec2 position, cocos2d::Size size, float angle)
{
	_type = Shape::Rect;
	_rect.origin = position;
	_rect.size = size;
	_angle = angle;
}

cocos2d::Rect Rectangle::getRect()
{
	auto p = this->getPolygonShape();
	return p->_rect;
}

PolygonShape *Rectangle::getPolygonShape()
{
	return agtk::PolygonShape::createRectangle(_rect, _angle);
}

//-------------------------------------------------------------------------------------------------------------------
Circle::Circle()
{
	_type = Shape::Circle;
	_position = cocos2d::Vec2::ZERO;
	_radius = 0.0f;
}

Circle::Circle(cocos2d::Vec2 position, float radius)
{
	_type = Shape::Circle;
	_position = position;
	_radius = radius;
}

Circle::Circle(float x, float y, float radius)
{
	_type = Shape::Circle;
	_position = cocos2d::Vec2(x, y);
	_radius = radius;
}

cocos2d::Rect Circle::getRect()
{
	cocos2d::Rect rect;
	rect.setRect(
		_position.x - _radius,
		_position.y - _radius,
		_radius * 2.0f,
		_radius * 2.0f
	);
	return rect;
}

PolygonShape *Circle::getPolygonShape()
{
	return agtk::PolygonShape::createCircle(_position, _radius);
}

//-------------------------------------------------------------------------------------------------------------------
Ellipse::Ellipse()
{
	_type = Shape::Ellipse;
	_position = cocos2d::Vec2::ZERO;
	_radiusXY = cocos2d::Vec2::ZERO;
	_angle = 0.0f;
}

Ellipse::Ellipse(cocos2d::Vec2 position, cocos2d::Vec2 radiusXY, float angle)
{
	_type = Shape::Ellipse;
	_position = position;
	_radiusXY = radiusXY;
	_angle = angle;
}

Ellipse::Ellipse(float x, float y, float rx, float ry, float angle)
{
	_type = Shape::Ellipse;
	_position = cocos2d::Vec2(x, y);
	_radiusXY = cocos2d::Vec2(rx, ry);
	_angle = angle;
}

cocos2d::Rect Ellipse::getRect()
{
	auto p = this->getPolygonShape();
	return p->_rect;
}

PolygonShape *Ellipse::getPolygonShape()
{
	return agtk::PolygonShape::createEllipse(_position, _radiusXY.x, _radiusXY.y, _angle);
}

//-------------------------------------------------------------------------------------------------------------------
Collision::Collision()
{
	_ignored = false;
	_samePrevFrame = false;
	_type = Shape::None;
}


Collision *Collision::create(const rapidjson::Value& json)
{
	Shape::Type type = Shape::None;
	if (json.HasMember("type")) {
		type = Collision::getType(json);
	}
	if (type == Shape::None) {
		type = Shape::Rect;
	}
	Collision *c = nullptr;
	switch (type) {
	case Shape::Circle:
		c = CollisionCircle::create(json);
		break;
	case Shape::Rect:
		c = CollisionRectangle::create(json);
		break;
	case Shape::Ellipse:
		c = CollisionEllipse::create(json);
		break;
	default:CC_ASSERT(0);
	}
	return c;
}

bool Collision::init(const rapidjson::Value& json)
{
	if (json.HasMember("ignored")) {
		this->setIgnored(json["ignored"].GetBool());
	}
	if (json.HasMember("samePrevFrame")) {
		this->setSamePrevFrame(json["samePrevFrame"].GetBool());
	}
	this->setType(Shape::Rect);
	if (json.HasMember("type")) {
		Shape::Type type = Collision::getType(json);
		this->setType(type);
	}
	return true;
}

Shape::Type Collision::getType(const rapidjson::Value& json)
{
	Shape::Type type = Shape::None;
	if (json.HasMember("type")) {
		static char* typeName[] = {
			"none", "rect", "circle", "ellipse"
		};
		for (int i = 0; i < CC_ARRAYSIZE(typeName); i++) {
			if (!strcmp(json["type"].GetString(), typeName[i])) {
				type = (Shape::Type)i;
				break;
			}
		}
	}
	return type;
}

Shape *Collision::getShape(Collision *c)
{
	Shape *p = nullptr;
	switch (c->getType()) {
	case agtk::Shape::Rect: {
		if (!c->getIgnored() && !c->getSamePrevFrame()) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			CollisionRectangle *cr = static_cast<CollisionRectangle *>(c);
#else
			CollisionRectangle *cr = dynamic_cast<CollisionRectangle *>(c);
#endif
			p = new Rectangle(cr->getRectangle());
		}
		break; }
	case agtk::Shape::Circle: {
		if (!c->getIgnored() && !c->getSamePrevFrame()) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			CollisionCircle *cc = static_cast<CollisionCircle *>(c);
#else
			CollisionCircle *cc = dynamic_cast<CollisionCircle *>(c);
#endif
			p = new Circle(cc->getCircle());
		}
		break; }
	case agtk::Shape::Ellipse: {
		if (!c->getIgnored() && !c->getSamePrevFrame()) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			CollisionEllipse *ce = static_cast<CollisionEllipse *>(c);
#else
			CollisionEllipse *ce = dynamic_cast<CollisionEllipse *>(c);
#endif
			p = new Ellipse(ce->getEllipse());
		}
		break; }
	default: CC_ASSERT(0);
	}
	return p;
}


Shape *Collision::getLinear(Collision *c1, Collision *c2, float range, float pos)
{
	Shape *p = nullptr;
	CC_ASSERT(c1->getType() == c2->getType());
	switch (c1->getType()){
	case agtk::Shape::Rect: {
		if (!c1->getIgnored() && !c1->getSamePrevFrame()) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			CollisionRectangle *cr1 = static_cast<CollisionRectangle *>(c1);
			CollisionRectangle *cr2 = static_cast<CollisionRectangle *>(c2);
#else
			CollisionRectangle *cr1 = dynamic_cast<CollisionRectangle *>(c1);
			CollisionRectangle *cr2 = dynamic_cast<CollisionRectangle *>(c2);
#endif
			CC_ASSERT(cr1 && cr2);
			if (!cr2->getIgnored() && !cr2->getSamePrevFrame()) {
				float x = AGTK_LINEAR_INTERPOLATE(cr1->getRectangle()._rect.origin.x, cr2->getRectangle()._rect.origin.x, range, pos);
				float y = AGTK_LINEAR_INTERPOLATE(cr1->getRectangle()._rect.origin.x, cr2->getRectangle()._rect.origin.y, range, pos);
				float w = AGTK_LINEAR_INTERPOLATE(cr1->getRectangle()._rect.size.width, cr2->getRectangle()._rect.size.width, range, pos);
				float h = AGTK_LINEAR_INTERPOLATE(cr1->getRectangle()._rect.size.height, cr2->getRectangle()._rect.size.height, range, pos);
				float a = AGTK_LINEAR_INTERPOLATE(cr1->getRectangle()._angle, cr2->getRectangle()._angle, range, pos);
				p = new Rectangle(x, y, w, h, a);
			}
			else {
				p = new Rectangle(cr1->getRectangle());
			}
		}
		break; }
	case agtk::Shape::Circle: {
		if (!c1->getIgnored() && !c1->getSamePrevFrame()) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			CollisionCircle *cc1 = static_cast<CollisionCircle *>(c1);
			CollisionCircle *cc2 = static_cast<CollisionCircle *>(c2);
#else
			CollisionCircle *cc1 = dynamic_cast<CollisionCircle *>(c1);
			CollisionCircle *cc2 = dynamic_cast<CollisionCircle *>(c2);
#endif
			CC_ASSERT(cc1 && cc2);
			if (!cc2->getIgnored() && !cc2->getSamePrevFrame()) {
				float x = AGTK_LINEAR_INTERPOLATE(cc1->getCircle()._position.x, cc2->getCircle()._position.x, range, pos);
				float y = AGTK_LINEAR_INTERPOLATE(cc1->getCircle()._position.y, cc2->getCircle()._position.y, range, pos);
				float r = AGTK_LINEAR_INTERPOLATE(cc1->getCircle()._radius, cc2->getCircle()._radius, range, pos);
				p = new Circle(x, y, r);
			}
			else {
				p = new Circle(cc1->getCircle());
			}
		}
		break; }
	case agtk::Shape::Ellipse: {
		if (!c1->getIgnored() && !c1->getSamePrevFrame()) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto ce1 = static_cast<CollisionEllipse *>(c1);
			auto ce2 = static_cast<CollisionEllipse *>(c2);
#else
			auto ce1 = dynamic_cast<CollisionEllipse *>(c1);
			auto ce2 = dynamic_cast<CollisionEllipse *>(c2);
#endif
			CC_ASSERT(ce1 && ce2);
			if (!c2->getIgnored() && !c2->getSamePrevFrame()) {
				float x = AGTK_LINEAR_INTERPOLATE(ce1->getEllipse()._position.x, ce2->getEllipse()._position.x, range, pos);
				float y = AGTK_LINEAR_INTERPOLATE(ce1->getEllipse()._position.y, ce2->getEllipse()._position.y, range, pos);
				float rx = AGTK_LINEAR_INTERPOLATE(ce1->getEllipse()._radiusXY.x, ce2->getEllipse()._radiusXY.x, range, pos);
				float ry = AGTK_LINEAR_INTERPOLATE(ce1->getEllipse()._radiusXY.y, ce2->getEllipse()._radiusXY.y, range, pos);
				float a = AGTK_LINEAR_INTERPOLATE(ce1->getEllipse()._angle, ce2->getEllipse()._angle, range, pos);
				p = new Ellipse(x, y, rx, ry, a);
			}
			else {
				p = new Ellipse(ce1->getEllipse());
			}
		}
		break; }
	default: CC_ASSERT(0);
	}
	return p;
}

Shape *Collision::getReLinear(Collision *c1, Collision *c2, float range, float pos)
{
	Shape *p = nullptr;
	CC_ASSERT(c1->getType() == c2->getType());
	switch (c1->getType()) {
	case agtk::Shape::Rect: {
		if (!c1->getIgnored() && !c1->getSamePrevFrame()) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			CollisionRectangle *cr1 = static_cast<CollisionRectangle *>(c1);
			CollisionRectangle *cr2 = static_cast<CollisionRectangle *>(c2);
#else
			CollisionRectangle *cr1 = dynamic_cast<CollisionRectangle *>(c1);
			CollisionRectangle *cr2 = dynamic_cast<CollisionRectangle *>(c2);
#endif
			CC_ASSERT(cr1 && cr2);
			if (!cr2->getIgnored() && !cr2->getSamePrevFrame()) {
				float x = AGTK_LINEAR_INTERPOLATE2(cr1->getRectangle()._rect.origin.x, cr2->getRectangle()._rect.origin.x, range, pos);
				float y = AGTK_LINEAR_INTERPOLATE2(cr1->getRectangle()._rect.origin.x, cr2->getRectangle()._rect.origin.y, range, pos);
				float w = AGTK_LINEAR_INTERPOLATE2(cr1->getRectangle()._rect.size.width, cr2->getRectangle()._rect.size.width, range, pos);
				float h = AGTK_LINEAR_INTERPOLATE2(cr1->getRectangle()._rect.size.height, cr2->getRectangle()._rect.size.height, range, pos);
				float a = AGTK_LINEAR_INTERPOLATE2(cr1->getRectangle()._angle, cr2->getRectangle()._angle, range, pos);
				p = new Rectangle(x, y, w, h, a);
			}
			else {
				p = new Rectangle(cr1->getRectangle());
			}
		}
		break; }
	case agtk::Shape::Circle: {
		if (!c1->getIgnored() && !c1->getSamePrevFrame()) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			CollisionCircle *cc1 = static_cast<CollisionCircle *>(c1);
			CollisionCircle *cc2 = static_cast<CollisionCircle *>(c2);
#else
			CollisionCircle *cc1 = dynamic_cast<CollisionCircle *>(c1);
			CollisionCircle *cc2 = dynamic_cast<CollisionCircle *>(c2);
#endif
			CC_ASSERT(cc1 && cc2);
			if (!cc2->getIgnored() && !cc2->getSamePrevFrame()) {
				float x = AGTK_LINEAR_INTERPOLATE2(cc1->getCircle()._position.x, cc2->getCircle()._position.x, range, pos);
				float y = AGTK_LINEAR_INTERPOLATE2(cc1->getCircle()._position.y, cc2->getCircle()._position.y, range, pos);
				float r = AGTK_LINEAR_INTERPOLATE2(cc1->getCircle()._radius, cc2->getCircle()._radius, range, pos);
				p = new Circle(x, y, r);
			}
			else {
				p = new Circle(cc1->getCircle());
			}
		}
		break; }
	case agtk::Shape::Ellipse: {
		if (!c1->getIgnored() && !c1->getSamePrevFrame()) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto ce1 = static_cast<CollisionEllipse *>(c1);
			auto ce2 = static_cast<CollisionEllipse *>(c2);
#else
			auto ce1 = dynamic_cast<CollisionEllipse *>(c1);
			auto ce2 = dynamic_cast<CollisionEllipse *>(c2);
#endif
			CC_ASSERT(ce1 && ce2);
			if (!c2->getIgnored() && !c2->getSamePrevFrame()) {
				float x = AGTK_LINEAR_INTERPOLATE2(ce1->getEllipse()._position.x, ce2->getEllipse()._position.x, range, pos);
				float y = AGTK_LINEAR_INTERPOLATE2(ce1->getEllipse()._position.y, ce2->getEllipse()._position.y, range, pos);
				float rx = AGTK_LINEAR_INTERPOLATE2(ce1->getEllipse()._radiusXY.x, ce2->getEllipse()._radiusXY.x, range, pos);
				float ry = AGTK_LINEAR_INTERPOLATE2(ce1->getEllipse()._radiusXY.y, ce2->getEllipse()._radiusXY.y, range, pos);
				float a = AGTK_LINEAR_INTERPOLATE2(ce1->getEllipse()._angle, ce2->getEllipse()._angle, range, pos);
				p = new Ellipse(x, y, rx, ry, a);
			}
			else {
				p = new Ellipse(ce1->getEllipse());
			}
		}
		break; }
	default: CC_ASSERT(0);
	}
	return p;
}

//-------------------------------------------------------------------------------------------------------------------
CollisionRect::CollisionRect()
{
	_rect = cocos2d::Rect::ZERO;
}

bool CollisionRect::init(const rapidjson::Value& json)
{
	if (!Collision::init(json)) {
		return false;
	}
	//CC_ASSERT(this->getType() == Collision::Rect);
	if(json.HasMember("x") && json.HasMember("y") && json.HasMember("width") && json.HasMember("height")){
		this->setRect(cocos2d::Rect(
			json["x"].GetInt(),
			json["y"].GetInt(),
			json["width"].GetInt(),
			json["height"].GetInt()
		));
	}
	return true;
}

cocos2d::Rect CollisionRect::GetCollisionRect(cocos2d::Size sz)
{
	cocos2d::Rect ret;
	//位置
	ret.origin.set(sz.width, sz.height);
	ret.origin.scale(_rect.origin * 0.01f);
	//サイズ
	cocos2d::Vec2 v(sz.width, sz.height);
	v.scale(_rect.size * 0.01f);
	ret.size = v;
	//trans
	ret.origin.y = (ret.size.height - sz.height) + ret.origin.y;
	return ret;
}

//-------------------------------------------------------------------------------------------------------------------
CollisionRectangle::CollisionRectangle()
{
	_shape = nullptr;
}

CollisionRectangle::~CollisionRectangle()
{
	CC_SAFE_RELEASE_NULL(_shape);
}

bool CollisionRectangle::init(const rapidjson::Value& json)
{
	if (!Collision::init(json)) {
		return false;
	}
	float x = (float)json["x"].GetInt();
	float y = (float)json["y"].GetInt();
	float w = (float)json["width"].GetInt();
	float h = (float)json["height"].GetInt();
	float angle = 0.0f;
	this->setRectangle(agtk::Rectangle(x, y, w, h, angle));
	return true;
}

cocos2d::Rect CollisionRectangle::getRect()
{
	return this->getRectangle().getRect();
}

cocos2d::Rect CollisionRectangle::getRect(cocos2d::Size sz)
{
	cocos2d::Rect ret;
	//位置
	ret.origin.set(sz.width, sz.height);
	ret.origin.scale(_rectangle._rect.origin * 0.01f);
	//サイズ
	cocos2d::Vec2 v(sz.width, sz.height);
	v.scale(_rectangle._rect.size * 0.01f);
	ret.size = v;
	//trans
	ret.origin.y = (ret.size.height - sz.height) + ret.origin.y;
	return ret;
}

//-------------------------------------------------------------------------------------------------------------------
CollisionCircle::CollisionCircle()
{
}

CollisionCircle::~CollisionCircle()
{
}

bool CollisionCircle::init(const rapidjson::Value& json)
{
	if (!Collision::init(json)) {
		return false;
	}
	CC_ASSERT(this->getType() == Shape::Circle);
	int x = 0, y = 0;
	//とりあえず
	if (json.HasMember("x") && json.HasMember("y")) {
		x = json["x"].GetInt();
		y = json["y"].GetInt();
	}
	int radius = 0;
	if (json.HasMember("width")) {
		radius = json["width"].GetInt();
	}
	this->setCircle(agtk::Circle(cocos2d::Vec2(x, y), radius));
	return true;
}

cocos2d::Rect CollisionCircle::getRect()
{
	return this->getCircle().getRect();
}

//-------------------------------------------------------------------------------------------------------------------
CollisionEllipse::CollisionEllipse()
{
}

CollisionEllipse::~CollisionEllipse()
{
}

bool CollisionEllipse::init(const rapidjson::Value& json)
{
	if (!Collision::init(json)) {
		return false;
	}
	CC_ASSERT(this->getType() == Shape::Ellipse);
	int x = 0, y = 0;
	//とりあえず
	if (json.HasMember("x") && json.HasMember("y")) {
		x = json["x"].GetInt();
		y = json["y"].GetInt();
	}
	int radiusX = 0;
	int radiusY = 0;
	if (json.HasMember("width") && json.HasMember("height")) {
		radiusX = json["width"].GetInt();
		radiusY = json["height"].GetInt();
	}
	int angle = 0;
	this->setEllipse(agtk::Ellipse(cocos2d::Vec2(x, y), cocos2d::Vec2(radiusX, radiusY), angle));
	return true;
}

cocos2d::Rect CollisionEllipse::getRect()
{
	return this->getEllipse().getRect();
}

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
void WallHitInfo::dump()
{
	CCLOG("------------------------------------------------------");
	CCLOG("center:%.2f,%.2f", center.x, center.y);
	CCLOG("boundMin:%.2f,%.2f", boundMin.x, boundMin.y);
	CCLOG("boundMax:%.2f,%.2f", boundMax.x, boundMax.y);
	CCLOG("sizes:%.2f,%.2f", size.width, size.height);
	CCLOG("hit (up:y,down:y,left:x,right:x):%.2f,%.2f,%.2f,%.2f", hitUp, hitDown, hitLeft, hitRight);
	CCLOG("wall(up:y,down:y,left:x,right:x):%.2f,%.2f,%.2f,%.2f", wallUp, wallDown, wallLeft, wallRight);
	CCLOG("openUp(x):%.2f,%.2f", openUpMin, openUpMax);
	CCLOG("openDown(x):%.2f,%.2f", openDownMin, openDownMax);
	CCLOG("openLeft(y):%.2f,%.2f", openLeftMin, openLeftMax);
	CCLOG("openRight(y):%.2f,%.2f", openRightMin, openRightMax);
}

bool WallHitInfoGroup::init(agtk::Object *object)
{
	this->setRect(cocos2d::Rect::ZERO);
	this->setCenter(cocos2d::Point::ZERO);
	this->setBoundMin(cocos2d::Point::ZERO);
	this->setBoundMax(cocos2d::Point::ZERO);
	_object = object;
	return true;
}

WallHitInfo &WallHitInfoGroup::getWallHitInfo(int id)
{
	CC_ASSERT(0 <= id && id < static_cast<int>(_wallHitInfoList.size()));
	return _wallHitInfoList.at(id);
}

WallHitInfo &WallHitInfoGroup::findWallHitInfo(int id)
{
	for(unsigned int i = 0; i < _wallHitInfoList.size(); i++) {
		auto wallHitInfo = _wallHitInfoList.at(i);
		if (wallHitInfo.id == id) {
			return _wallHitInfoList.at(i);
		}
	}
	return _errorWallHitInfo;
}

void WallHitInfoGroup::addWallHitInfo(agtk::Vertex4 v)
{
	WallHitInfo wallHitInfo;
	int id = _wallHitInfoList.size();
	auto rect = v.getRect();

	wallHitInfo.boundMin = cocos2d::Point(rect.getMinX(), rect.getMinY());
	wallHitInfo.boundMax = cocos2d::Point(rect.getMaxX(), rect.getMaxY());
	wallHitInfo.center = cocos2d::Point(rect.getMidX(), rect.getMidY());
	wallHitInfo.initHit(_object);
	wallHitInfo.id = id;
	wallHitInfo.wallVertex4 = v;
	wallHitInfo.size = rect.size;
	_wallHitInfoList.push_back(wallHitInfo);
	this->mergeRect();
}

void WallHitInfoGroup::addWallHitInfo(std::vector<agtk::Vertex4> &vertex4List)
{
	for (auto v : vertex4List) {
		this->addWallHitInfo(v);
	}
}

void WallHitInfoGroup::addWallHitInfo(WallHitInfo &wallHitInfo)
{
	WallHitInfo info = wallHitInfo;
	_wallHitInfoList.push_back(info);
	this->mergeRect();
}

void WallHitInfoGroup::addWallHitInfo(std::vector<WallHitInfo> &wallHitInfoList)
{
	for (auto v : wallHitInfoList) {
		this->addWallHitInfo(v);
	}
}

void WallHitInfoGroup::remove()
{
	this->setRect(cocos2d::Rect::ZERO);
	this->setCenter(cocos2d::Point::ZERO);
	this->setBoundMin(cocos2d::Point::ZERO);
	this->setBoundMax(cocos2d::Point::ZERO);

	_wallHitInfoList.clear();
}

void WallHitInfoGroup::mergeRect()
{
	if (_wallHitInfoList.size() == 0) {
		return;
	}
	auto wallHitInfo = _wallHitInfoList.at(0);
	cocos2d::Rect rect = wallHitInfo.wallVertex4.getRect();
	for (unsigned int i = 1; i < _wallHitInfoList.size(); i++) {
		auto wallHitInfo2 = _wallHitInfoList.at(i);
		rect.merge(wallHitInfo2.wallVertex4.getRect());
	}
	this->setRect(rect);
	getWallCenterAndBound(rect.origin, rect.size, &_center, &_boundMin, &_boundMax);
}

#if defined(USE_WALL_DEBUG_DISPLAY)
//-------------------------------------------------------------------------------------------------------------------
ObjectWallDebugDisplay::ObjectWallDebugDisplay()
{
	_node = nullptr;
	_primitiveNodeList = nullptr;
	_tilePrimitiveNodeList = nullptr;
	_objectPrimitiveNodeList = nullptr;
}

ObjectWallDebugDisplay::~ObjectWallDebugDisplay()
{
	CC_SAFE_RELEASE_NULL(_node);
	CC_SAFE_RELEASE_NULL(_primitiveNodeList);
	CC_SAFE_RELEASE_NULL(_tilePrimitiveNodeList);
	CC_SAFE_RELEASE_NULL(_objectPrimitiveNodeList);
}

bool ObjectWallDebugDisplay::init(cocos2d::Node *node)
{
	if (node == nullptr) {
		return false;
	}
	this->setNode(node);
	this->setPrimitiveNodeList(cocos2d::Array::create());
	this->setTilePrimitiveNodeList(cocos2d::Array::create());
	this->setObjectPrimitiveNodeList(cocos2d::Array::create());
	auto pm = PrimitiveManager::getInstance();
	//_wallCenter
	for (int i = 0; i < 2; i++) {
		auto line = pm->createLine(0, 0, 0, 0, cocos2d::Color4F(1, 1, 1, 0.5f));
		_primitiveNodeList->addObject(line);
		node->addChild(line);
	}
	//_hitLeft, _hitRight, _hitUp, _hitDown
	for (int i = 0; i < 4; i++) {
		auto line = pm->createLine(0, 0, 0, 0, cocos2d::Color4F(1, 0, 0, 0.5f));
		_primitiveNodeList->addObject(line);
		node->addChild(line);
	}
	//_openLeftMin, _openLeftMax, _openRightMin, _openRightMax, _openUpMin, _openUpMax, _openDownMin, _openDownMax
	for (int i = 0; i < 8; i++) {
		auto line = pm->createLine(0, 0, 0, 0, cocos2d::Color4F(0, 1, 0, 0.5f));
		_primitiveNodeList->addObject(line);
		node->addChild(line);
	}
	reset();
	return true;
}

void ObjectWallDebugDisplay::destroy()
{
	auto node = this->getNode();
	cocos2d::Ref *ref = nullptr;
	auto array = _primitiveNodeList;
	CCARRAY_FOREACH(array, ref) {
		auto primitiveNode = dynamic_cast<cocos2d::Node *>(ref);
		node->removeChild(primitiveNode);
	}
	_primitiveNodeList->removeAllObjects();
	array = _tilePrimitiveNodeList;
	CCARRAY_FOREACH(array, ref) {
		auto primitiveNode = dynamic_cast<cocos2d::Node *>(ref);
		node->removeChild(primitiveNode);
	}
	_tilePrimitiveNodeList->removeAllObjects();
	array = _objectPrimitiveNodeList;
	CCARRAY_FOREACH(array, ref) {
		auto primitiveNode = dynamic_cast<cocos2d::Node *>(ref);
		node->removeChild(primitiveNode);
	}
	_objectPrimitiveNodeList->removeAllObjects();
}

void ObjectWallDebugDisplay::reset()
{
	_boundMin = cocos2d::Vec2(-16, -16);
	_boundMax = cocos2d::Vec2(-16, -16);
	_wallCenter = cocos2d::Vec2(-16, -16);
	_hitLeft = -16;
	_hitRight = -16;
	_hitUp = -16;
	_hitDown = -16;
	_openLeftMin = -16;
	_openLeftMax = -16;
	_openRightMin = -16;
	_openRightMax = -16;
	_openUpMin = -16;
	_openUpMax = -16;
	_openDownMin = -16;
	_openDownMax = -16;
	_tileWallList.clear();
	_objectWallList.clear();
}

void ObjectWallDebugDisplay::addTileWall(cocos2d::Vec2 &boundMin, cocos2d::Vec2 &boundMax)
{
	_tileWallList.push_back(WallRect(boundMin, boundMax));
}

void ObjectWallDebugDisplay::addObjectWall(cocos2d::Vec2 &boundMin, cocos2d::Vec2 &boundMax)
{
	_objectWallList.push_back(WallRect(boundMin, boundMax));
}

PrimitiveNode *ObjectWallDebugDisplay::getNextTilePrimitiveNode()
{
	if (_tilePrimitiveNodeIndex < _tilePrimitiveNodeList->count()) {
		return dynamic_cast<PrimitiveNode *>(_tilePrimitiveNodeList->getObjectAtIndex(_tilePrimitiveNodeIndex++));
	}
	auto node = this->getNode();
	auto pm = PrimitiveManager::getInstance();
	auto line = pm->createLine(0, 0, 0, 0, cocos2d::Color4F(1, 0, 0, 0.5f));
	_tilePrimitiveNodeList->addObject(line);
	node->addChild(line);
	return line;
}

PrimitiveNode *ObjectWallDebugDisplay::getNextObjectPrimitiveNode()
{
	if (_objectPrimitiveNodeIndex < _objectPrimitiveNodeList->count()) {
		return dynamic_cast<PrimitiveNode *>(_objectPrimitiveNodeList->getObjectAtIndex(_objectPrimitiveNodeIndex++));
	}
	auto node = this->getNode();
	auto pm = PrimitiveManager::getInstance();
	auto line = pm->createLine(0, 0, 0, 0, cocos2d::Color4F(1, 0, 1, 0.5f));
	_objectPrimitiveNodeList->addObject(line);
	node->addChild(line);
	return line;
}

void ObjectWallDebugDisplay::update(float dt)
{
	cocos2d::Ref *ref = nullptr;
	PrimitiveNode *primitiveNode = nullptr;
	_tilePrimitiveNodeIndex = 0;
	_objectPrimitiveNodeIndex = 0;

	for (auto &wallInfo : _tileWallList) {
		primitiveNode = getNextTilePrimitiveNode();
		primitiveNode->setLine(wallInfo.boundMin.x, wallInfo.boundMin.y, wallInfo.boundMax.x, wallInfo.boundMin.y);
		primitiveNode = getNextTilePrimitiveNode();
		primitiveNode->setLine(wallInfo.boundMin.x, wallInfo.boundMax.y, wallInfo.boundMax.x, wallInfo.boundMax.y);
		primitiveNode = getNextTilePrimitiveNode();
		primitiveNode->setLine(wallInfo.boundMin.x, wallInfo.boundMin.y, wallInfo.boundMin.x, wallInfo.boundMax.y);
		primitiveNode = getNextTilePrimitiveNode();
		primitiveNode->setLine(wallInfo.boundMax.x, wallInfo.boundMin.y, wallInfo.boundMax.x, wallInfo.boundMax.y);
		primitiveNode = getNextTilePrimitiveNode();
		primitiveNode->setLine(wallInfo.boundMin.x, wallInfo.boundMin.y, wallInfo.boundMax.x, wallInfo.boundMax.y);
		primitiveNode = getNextTilePrimitiveNode();
		primitiveNode->setLine(wallInfo.boundMax.x, wallInfo.boundMin.y, wallInfo.boundMin.x, wallInfo.boundMax.y);
	}
	while (_tilePrimitiveNodeIndex < _tilePrimitiveNodeList->count()) {
		primitiveNode = dynamic_cast<PrimitiveNode *>(_tilePrimitiveNodeList->getObjectAtIndex(_tilePrimitiveNodeIndex++));
		primitiveNode->setLine(-16, -16, -16, -16);
	}

	for (auto &wallInfo : _objectWallList) {
		primitiveNode = getNextObjectPrimitiveNode();
		primitiveNode->setLine(wallInfo.boundMin.x, wallInfo.boundMin.y, wallInfo.boundMax.x, wallInfo.boundMin.y);
		primitiveNode = getNextObjectPrimitiveNode();
		primitiveNode->setLine(wallInfo.boundMin.x, wallInfo.boundMax.y, wallInfo.boundMax.x, wallInfo.boundMax.y);
		primitiveNode = getNextObjectPrimitiveNode();
		primitiveNode->setLine(wallInfo.boundMin.x, wallInfo.boundMin.y, wallInfo.boundMin.x, wallInfo.boundMax.y);
		primitiveNode = getNextObjectPrimitiveNode();
		primitiveNode->setLine(wallInfo.boundMax.x, wallInfo.boundMin.y, wallInfo.boundMax.x, wallInfo.boundMax.y);
		primitiveNode = getNextObjectPrimitiveNode();
		primitiveNode->setLine(wallInfo.boundMin.x, wallInfo.boundMin.y, wallInfo.boundMax.x, wallInfo.boundMax.y);
		primitiveNode = getNextObjectPrimitiveNode();
		primitiveNode->setLine(wallInfo.boundMax.x, wallInfo.boundMin.y, wallInfo.boundMin.x, wallInfo.boundMax.y);
	}
	while (_objectPrimitiveNodeIndex < _objectPrimitiveNodeList->count()) {
		primitiveNode = dynamic_cast<PrimitiveNode *>(_objectPrimitiveNodeList->getObjectAtIndex(_objectPrimitiveNodeIndex++));
		primitiveNode->setLine(-16, -16, -16, -16);
	}

	auto primitiveNodeList = this->getPrimitiveNodeList();
	int index = 0;
	//_wallCenter
	primitiveNode = dynamic_cast<PrimitiveNode *>(primitiveNodeList->getObjectAtIndex(index++));
	primitiveNode->setLine(_wallCenter.x - 8, _wallCenter.y, _wallCenter.x + 8, _wallCenter.y);
	primitiveNode = dynamic_cast<PrimitiveNode *>(primitiveNodeList->getObjectAtIndex(index++));
	primitiveNode->setLine(_wallCenter.x, _wallCenter.y - 8, _wallCenter.x, _wallCenter.y + 8);

	//_hitLeft, _hitRight, _hitUp, _hitDown
	primitiveNode = dynamic_cast<PrimitiveNode *>(primitiveNodeList->getObjectAtIndex(index++));
	primitiveNode->setLine(_hitLeft, _boundMin.y, _hitLeft, _boundMax.y);
	primitiveNode = dynamic_cast<PrimitiveNode *>(primitiveNodeList->getObjectAtIndex(index++));
	primitiveNode->setLine(_hitRight, _boundMin.y, _hitRight, _boundMax.y);
	primitiveNode = dynamic_cast<PrimitiveNode *>(primitiveNodeList->getObjectAtIndex(index++));
	primitiveNode->setLine(_boundMin.x, _hitUp, _boundMax.x, _hitUp);
	primitiveNode = dynamic_cast<PrimitiveNode *>(primitiveNodeList->getObjectAtIndex(index++));
	primitiveNode->setLine(_boundMin.x, _hitDown, _boundMax.x, _hitDown);

	//_openLeftMin, _openLeftMax
	primitiveNode = dynamic_cast<PrimitiveNode *>(primitiveNodeList->getObjectAtIndex(index++));
	if (_openLeftMin >= _boundMin.y) {
		primitiveNode->setLine(_boundMin.x - 8, _openLeftMin, _openDownMin, _openLeftMin);
	}
	else {
		primitiveNode->setLine(-16, -16, -16, -16);
	}
	primitiveNode = dynamic_cast<PrimitiveNode *>(primitiveNodeList->getObjectAtIndex(index++));
	if (_openLeftMax <= _boundMax.y) {
		primitiveNode->setLine(_boundMin.x - 8, _openLeftMax, _openUpMin, _openLeftMax);
	}
	else {
		primitiveNode->setLine(-16, -16, -16, -16);
	}
	//_openRightMin, _openRightMax
	primitiveNode = dynamic_cast<PrimitiveNode *>(primitiveNodeList->getObjectAtIndex(index++));
	if (_openRightMin >= _boundMin.y) {
		primitiveNode->setLine(_openDownMax, _openRightMin, _boundMax.x + 8, _openRightMin);
	}
	else {
		primitiveNode->setLine(-16, -16, -16, -16);
	}
	primitiveNode = dynamic_cast<PrimitiveNode *>(primitiveNodeList->getObjectAtIndex(index++));
	if (_openRightMax <= _boundMax.y) {
		primitiveNode->setLine(_openUpMax, _openRightMax, _boundMax.x + 8, _openRightMax);
	}
	else {
		primitiveNode->setLine(-16, -16, -16, -16);
	}
	//_openUpMin, _openUpMax
	primitiveNode = dynamic_cast<PrimitiveNode *>(primitiveNodeList->getObjectAtIndex(index++));
	if (_openUpMin >= _boundMin.x) {
		primitiveNode->setLine(_openUpMin, _openLeftMax, _openUpMin, _boundMax.y + 8);
	}
	else {
		primitiveNode->setLine(-16, -16, -16, -16);
	}
	primitiveNode = dynamic_cast<PrimitiveNode *>(primitiveNodeList->getObjectAtIndex(index++));
	if (_openUpMax <= _boundMax.x) {
		primitiveNode->setLine(_openUpMax, _openRightMax, _openUpMax, _boundMax.y + 8);
	}
	else {
		primitiveNode->setLine(-16, -16, -16, -16);
	}
	//_openDownMin, _openDownMax
	primitiveNode = dynamic_cast<PrimitiveNode *>(primitiveNodeList->getObjectAtIndex(index++));
	if (_openDownMin >= _boundMin.x) {
		primitiveNode->setLine(_openDownMin, _boundMin.y - 8, _openDownMin, _openLeftMin);
	}
	else {
		primitiveNode->setLine(-16, -16, -16, -16);
	}
	primitiveNode = dynamic_cast<PrimitiveNode *>(primitiveNodeList->getObjectAtIndex(index++));
	if (_openDownMax <= _boundMax.x) {
		primitiveNode->setLine(_openDownMax, _boundMax.y - 8, _openDownMax, _openRightMin);
	}
	else {
		primitiveNode->setLine(-16, -16, -16, -16);
	}
}
#endif

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
void getWallCenterAndBound(cocos2d::Vec2 pos, cocos2d::Vec2 size, cocos2d::Point *center, cocos2d::Point *boundMin, cocos2d::Point *boundMax)
{
	float minX, maxX, minY, maxY;
	float x0 = pos.x;
	float y0 = pos.y;
	float x1 = pos.x + size.x;
	float y1 = pos.y + size.y;
	minX = x0;
	minY = y0;
	//! float型の計算による桁落ち誤差を考慮
	maxX = x1 - (FLT_EPSILON * x1);
	maxY = y1 - (FLT_EPSILON * y1);
	if (minX > maxX) maxX = minX;
	if (minY > maxY) maxY = minY;
	*center = cocos2d::Point(floorf(minX * 0.5f + maxX * 0.5f), floorf(minY * 0.5f + maxY * 0.5f));
	*boundMin = cocos2d::Point(minX, minY);
	*boundMax = cocos2d::Point(maxX, maxY);
}

void getWallCenterAndBound(cocos2d::Node *node, Point *center, Point *boundMin, Point *boundMax)
{
	float minX, maxX, minY, maxY;
	//todo: 回転を考慮。
	//auto wallShape = agtk::PolygonShape::createRectangle(wall->getPosition(), wall->getContentSize(), wall->getRotation());
	auto pos = node->getPosition();

	// 上下左右のシーン接続が発生している可能性があるので
	// 親ノードの座標分、座標をずらす
	auto parent = node->getParent();
	if (parent) {
		pos += parent->getPosition();
	}
	auto size = node->getContentSize();
	float x0 = pos.x;
	float y0 = pos.y;
	float x1 = pos.x + size.width;
	float y1 = pos.y + size.height;
	minX = x0;
	minY = y0;
	maxX = x1;
	maxY = y1;
	*center = Point(floorf((minX + maxX) / 2), floorf((minY + maxY) / 2));
	*boundMin = Point(minX, minY);
	*boundMax = Point(maxX, maxY);
}

bool checkWallHit(WallHitInfo &wallHitInfo, const Point &pos, const Size &size, bool isTile, ObjectWallDebugDisplay *debug)
{
	float x0 = pos.x + TILE_COLLISION_THRESHOLD;
	float y0 = pos.y + TILE_COLLISION_THRESHOLD;
	float x1 = pos.x + (size.width - TILE_COLLISION_THRESHOLD);
	float y1 = pos.y + (size.height - TILE_COLLISION_THRESHOLD);
#if defined(USE_WALL_DEBUG_DISPLAY)
	if (debug != nullptr) {
		auto wallDebugDisplay = debug;
		if (isTile) {
			wallDebugDisplay->addTileWall(cocos2d::Vec2(x0, y0), cocos2d::Vec2(x1, y1));
		}
		else {
			wallDebugDisplay->addObjectWall(cocos2d::Vec2(x0, y0), cocos2d::Vec2(x1, y1));
		}
	}
#endif
	if (x0 > wallHitInfo.boundMax.x || y0 > wallHitInfo.boundMax.y || x1 < wallHitInfo.boundMin.x || y1 < wallHitInfo.boundMin.y) {
		//当たっていない。
		return false;
	}

	if (x0 <= wallHitInfo.center.x && x1 >= wallHitInfo.boundMin.x) {
		if (y0 <= wallHitInfo.center.y && y1 >= wallHitInfo.center.y) {
			wallHitInfo.openLeftMin = wallHitInfo.center.y;
			wallHitInfo.openLeftMax = wallHitInfo.center.y;
		}
		else if (y1 <= wallHitInfo.center.y && y1 >= wallHitInfo.boundMin.y) {
			if (wallHitInfo.openLeftMin < y1) {
				wallHitInfo.openLeftMin = y1;
			}
		}
		else if (y0 >= wallHitInfo.center.y &&  y0 <= wallHitInfo.boundMax.y) {
			if (wallHitInfo.openLeftMax > y0) {
				wallHitInfo.openLeftMax = y0;
			}
		}
	}
	if (x0 <= wallHitInfo.boundMax.x && x1 >= wallHitInfo.center.x) {
		if (y0 <= wallHitInfo.center.y && y1 >= wallHitInfo.center.y) {
			wallHitInfo.openRightMin = wallHitInfo.center.y;
			wallHitInfo.openRightMax = wallHitInfo.center.y;
		}
		else if (y1 <= wallHitInfo.center.y && y1 >= wallHitInfo.boundMin.y) {
			if (wallHitInfo.openRightMin < y1) {
				wallHitInfo.openRightMin = y1;
			}
		}
		else if (y0 >= wallHitInfo.center.y &&  y0 <= wallHitInfo.boundMax.y) {
			if (wallHitInfo.openRightMax > y0) {
				wallHitInfo.openRightMax = y0;
			}
		}
	}
	if (y0 <= wallHitInfo.center.y && y1 >= wallHitInfo.boundMin.y) {
		if (x0 <= wallHitInfo.center.x && x1 >= wallHitInfo.center.x) {
			wallHitInfo.openDownMin = wallHitInfo.center.x;
			wallHitInfo.openDownMax = wallHitInfo.center.x;
		}
		else if (x1 <= wallHitInfo.center.x && x1 >= wallHitInfo.boundMin.x) {
			if (wallHitInfo.openDownMin < x1) {
				wallHitInfo.openDownMin = x1;
			}
		}
		else if (x0 >= wallHitInfo.center.x &&  x0 <= wallHitInfo.boundMax.x) {
			if (wallHitInfo.openDownMax > x0) {
				wallHitInfo.openDownMax = x0;
			}
		}
	}
	if (y0 <= wallHitInfo.boundMax.y &&  y1 >= wallHitInfo.center.y) {
		if (x0 <= wallHitInfo.center.x && x1 >= wallHitInfo.center.x) {
			wallHitInfo.openUpMin = wallHitInfo.center.x;
			wallHitInfo.openUpMax = wallHitInfo.center.x;
		}
		else if (x1 <= wallHitInfo.center.x && x1 >= wallHitInfo.boundMin.x) {
			if (wallHitInfo.openUpMin < x1) {
				wallHitInfo.openUpMin = x1;
			}
		}
		else if (x0 >= wallHitInfo.center.x &&  x0 <= wallHitInfo.boundMax.x) {
			if (wallHitInfo.openUpMax > x0) {
				wallHitInfo.openUpMax = x0;
			}
		}
	}
	if (x0 <= wallHitInfo.center.x && x1 > wallHitInfo.hitLeft) {
		wallHitInfo.hitLeft = x1;
	}
	if (x1 >= wallHitInfo.center.x && x0 < wallHitInfo.hitRight) {
		wallHitInfo.hitRight = x0;
	}
	if (y0 <= wallHitInfo.center.y && y1 > wallHitInfo.hitDown) {
		wallHitInfo.hitDown = y1;
	}
	if (y1 >= wallHitInfo.center.y && y0 < wallHitInfo.hitUp) {
		wallHitInfo.hitUp = y0;
	}

	if (x0 < wallHitInfo.wallLeft) {
		wallHitInfo.wallLeft = x0;
	}
	if (x1 > wallHitInfo.wallRight) {
		wallHitInfo.wallRight = x1;
	}
	if (y0 < wallHitInfo.wallDown) {
		wallHitInfo.wallDown = y0;
	}
	if (y1 > wallHitInfo.wallUp) {
		wallHitInfo.wallUp = y1;
	}

	return true;
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
bool checkBuriedWall(cocos2d::Rect& objectRect, std::vector<agtk::Tile *> &tileList)
#else
bool checkBuriedWall(cocos2d::Rect& objectRect, cocos2d::Array* tileList)
#endif
{
	Rect wallRect;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
	cocos2d::Ref *ref = nullptr;
#endif
	
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	for (auto tile : tileList) {
#else
	CCARRAY_FOREACH(tileList, ref) {
		auto tile = dynamic_cast<agtk::Tile*>(ref);
#endif
		auto rect = tile->convertToLayerSpaceRect();
		auto pos = rect.origin;
		auto size = rect.size;
		size.width += 1;
		size.height += 1;

		wallRect = Rect(pos, size);

		if (wallRect.intersectsRect(objectRect)) {
			return true;
		}
	}

	return false;
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
bool checkBuriedWallDownPos(cocos2d::Rect& objectRect, float y, std::vector<agtk::Tile *> &tileList)
#else
bool checkBuriedWallDownPos(cocos2d::Rect& objectRect, float y, cocos2d::Array* tileList)
#endif
{
	Rect wallRect;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
	cocos2d::Ref *ref = nullptr;
#endif


	auto objRect = objectRect;
	objRect.origin.y -= y;
	

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	for (auto tile : tileList) {
#else
	CCARRAY_FOREACH(tileList, ref) {
		auto tile = dynamic_cast<agtk::Tile*>(ref);
#endif
		auto rect = tile->convertToLayerSpaceRect();
		auto pos = rect.origin;
		auto size = rect.size;
		pos.x += TILE_COLLISION_THRESHOLD;
		pos.y += TILE_COLLISION_THRESHOLD;
		size.width -= TILE_COLLISION_THRESHOLD * 2;
		size.height -= TILE_COLLISION_THRESHOLD * 2;

		wallRect = Rect(pos, size);

		if (wallRect.intersectsRect(objRect)) {
			return true;
		}
	}

	return false;
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
bool pushBackBuriedWall(agtk::Object* object, cocos2d::Rect& objectRect, cocos2d::Vec2& move, std::vector<agtk::Tile *> &tileList)
#else
bool pushBackBuriedWall(agtk::Object* object, cocos2d::Rect& objectRect, cocos2d::Vec2& move, cocos2d::Array* tileList)
#endif
{
	Rect wallRect, checkRect;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
	cocos2d::Ref *ref = nullptr;
#endif
	bool hitLT = false;
	bool hitRT = false;
	bool hitLB = false;
	bool hitRB = false;

	float rectWidthHalf = objectRect.size.width * 0.5f;
	float rectHeightHalf = objectRect.size.height * 0.5f;

	auto projectData = GameManager::getInstance()->getProjectData();

	float left = objectRect.getMaxX() + 10000;
	float right = objectRect.getMinX() - 10000;
	float down = objectRect.getMaxY() + 10000;
	float up = objectRect.getMinY() - 10000;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	for (auto tile : tileList) {
#else
	CCARRAY_FOREACH(tileList, ref) {
		auto tile = dynamic_cast<agtk::Tile*>(ref);
#endif

		// プレイヤーグループでないオブジェクトが行動範囲制限との接触を確認する場合は処理しない
		if (tile->getType() == agtk::Tile::kTypeLimitTile && !dynamic_cast<LimitTile*>(tile)->isNeedCheck(object)) {
			continue;
		}
		if (object && !(object->getObjectData()->getCollideWithTileGroupBit() & tile->getGroupBit())) {
			continue;
		}

		auto rect = tile->convertToLayerSpaceRect();
		auto pos = rect.origin;
		auto size = rect.size;
		pos.x += TILE_COLLISION_THRESHOLD;
		pos.y += TILE_COLLISION_THRESHOLD;
		size.width -= TILE_COLLISION_THRESHOLD * 2;
		size.height -= TILE_COLLISION_THRESHOLD * 2;

		float x0 = pos.x;
		float y0 = pos.y;
		float x1 = pos.x + size.width;
		float y1 = pos.y + size.height;

		wallRect = Rect(pos, size);

		int wallBit = tile->getWallBit();
		if (wallBit != 0x0f) {

			continue;
		}

		bool isHit = false;

		bool checkLeft = false;
		bool checkRight = false;
		bool checkUp = false;
		bool checkDown = false;

		// オブジェクト左上の接触チェック
		checkRect = Rect(objectRect.getMinX(), objectRect.getMidY(), rectWidthHalf, rectHeightHalf);
		//hitLT |= wallRect.intersectsRect(checkRect);
		isHit = wallRect.intersectsRect(checkRect);
		if (isHit) {
			hitLT |= isHit;
			
			checkRight = true;
			checkDown = true;
		}

		// オブジェクト右上の接触チェック
		checkRect = Rect(objectRect.getMidX(), objectRect.getMidY(), rectWidthHalf, rectHeightHalf);
		//hitRT |= wallRect.intersectsRect(checkRect);
		isHit = wallRect.intersectsRect(checkRect);

		if (isHit) {
			hitRT |= isHit;

			checkLeft = true;
			checkDown = true;
		}


		// オブジェクト左下の接触チェック
		checkRect = Rect(objectRect.getMinX(), objectRect.getMinY(), rectWidthHalf, rectHeightHalf);
		hitLB |= wallRect.intersectsRect(checkRect);

		// オブジェクト右下の接触チェック
		checkRect = Rect(objectRect.getMidX(), objectRect.getMinY(), rectWidthHalf, rectHeightHalf);
		hitRB |= wallRect.intersectsRect(checkRect);

		if (checkRight && (right < x1)) {
			right = x1;
		}

		if (checkDown && (y0 < down)) {
			down = y0;
		}
	}

	// 4隅すべて接触している場合は埋まっている
	if (hitLT && hitRT && hitLB && hitRB) {
		return false;
	}

	// 全て接触していない場合は埋まっていない
	if (!hitLT && !hitRT && !hitLB && !hitRB) {
		return true;
	}

//	CCLOG("押し戻し：%d, %d, %d, %d", hitLT, hitRT, hitLB, hitRB);

	Vec2 moveVec = Vec2::ZERO;

	if (hitLT && hitRT && hitLB && !hitRB) {
		// 右下のみ空きがある
		moveVec.x = -1.0f;
		moveVec.y = 1.0f;
	}
	else if (hitLT && hitRT && !hitLB && hitRB) {
		// 左下のみ空きがある
		moveVec.x = 1.0f;
		moveVec.y = 1.0f;
	}
	else if (hitLT && !hitRT && hitLB && hitRB) {
		// 右上のみ空きがある
		moveVec.x = -1.0f;
		moveVec.y = -1.0f;
	}
	else if (!hitLT && hitRT && hitLB && hitRB) {
		// 左上のみ空きがある
		moveVec.x = 1.0f;
		moveVec.y = -1.0f;
	}
	else if (hitLT && hitRT && !hitLB && !hitRB) {
		// 食い込み量チェック
		float pushY = objectRect.getMaxY() - down;

		// 少し地面から離しているために接触していないだけかもしれないので
		// 一応地面との接触をチェックする
		if (checkBuriedWallDownPos(objectRect, pushY + 0.1f, tileList)) {
			return false;
		}

		// 下に空きがある場合
		moveVec.y = 1.0f;
	}
	else if (!hitLT && !hitRT && hitLB && hitRB) {
		// 上に空きがある場合
		moveVec.y = -1.0f;
	}
	else if (hitLT && !hitRT && hitLB && !hitRB) {
		// 右に空きがある場合
		moveVec.x = -1.0f;
	}
	else if (!hitLT && hitRT && !hitLB && hitRB) {
		// 左に空きがある場合
		moveVec.x = 1.0f;
	}
	else if (hitLT && !hitRT && !hitLB && !hitRB) {
		// 左上のみ接触
		moveVec.x = -1.0f;
		moveVec.y = 1.0f;
	}
	else if (!hitLT && hitRT && !hitLB && !hitRB) {
		// 右上のみ接触
		moveVec.x = 1.0f;
		moveVec.y = 1.0f;
	}
	else if (!hitLT && !hitRT && hitLB && !hitRB) {
		// 左下のみ接触
		moveVec.x = -1.0f;
		moveVec.y = -1.0f;
	}
	else if (!hitLT && !hitRT && !hitLB && hitRB) {
		// 右下のみ接触
		moveVec.x = 1.0f;
		moveVec.y = -1.0f;
	}
	else {
		// 特殊な接触が発生していると思われる
		return false;
	}

	bool hitX, hitY;
	agtk::HitWallTiles hitTiles;
	WallHitInfo info;
	info.boundMin = objectRect.origin;
	info.boundMax = Point(objectRect.getMaxX(), objectRect.getMaxY());
	info.center = info.boundMin;
	info.center.x += objectRect.size.width * 0.5f;
	info.center.y += objectRect.size.height * 0.5f;
	info.size = objectRect.size;
	info.initHit(object);

	moveVec.x *= objectRect.size.width;
	moveVec.y *= objectRect.size.height;

	// 埋まらない位置まで移動させてみる
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	agtk::MtVector<agtk::Slope *> slopeList;
	newCheckWallHit(info, hitX, hitY, 0, tileList, &slopeList, nullptr, moveVec, Vec2(0, 0), objectRect, false, false, hitTiles);
#else
	newCheckWallHit(info, hitX, hitY, 0, tileList, nullptr, nullptr, moveVec, Vec2(0, 0), objectRect, false, false, hitTiles);
#endif
#else
#endif

	objectRect.origin.x = info.boundMin.x;
	objectRect.origin.y = info.boundMin.y;

	// 埋まらない位置まで移動できたと仮定
	return true;
}

bool pushBackBuriedObjectWall(cocos2d::Rect& objectPrevRect, cocos2d::Rect& objectCrntRect, cocos2d::Vec2& move, agtk::WallHitInfoGroup* infoGroup)
{
	Rect wallRect, checkRect;
	cocos2d::Ref *ref = nullptr;
	bool hitLT = false;
	bool hitRT = false;
	bool hitLB = false;
	bool hitRB = false;

	float rectWidthHalf = objectPrevRect.size.width * 0.5f;
	float rectHeightHalf = objectPrevRect.size.height * 0.5f;

	for (unsigned int i = 0; i < infoGroup->getWallHitInfoListCount(); i++) {
		auto wallInfo = infoGroup->getWallHitInfo(i);

		wallRect = Rect(wallInfo.boundMin, Size(wallInfo.boundMax - wallInfo.boundMin));

		wallRect.origin.x += TILE_COLLISION_THRESHOLD;
		wallRect.origin.y += TILE_COLLISION_THRESHOLD;
		wallRect.size.width -= TILE_COLLISION_THRESHOLD * 2;
		wallRect.size.height -= TILE_COLLISION_THRESHOLD * 2;

		// オブジェクト左上の接触チェック
		checkRect = Rect(objectPrevRect.getMinX(), objectPrevRect.getMidY(), rectWidthHalf, rectHeightHalf);
		hitLT |= wallRect.intersectsRect(checkRect);

		// オブジェクト右上の接触チェック
		checkRect = Rect(objectPrevRect.getMidX(), objectPrevRect.getMidY(), rectWidthHalf, rectHeightHalf);
		hitRT |= wallRect.intersectsRect(checkRect);

		// オブジェクト左下の接触チェック
		checkRect = Rect(objectPrevRect.getMinX(), objectPrevRect.getMinY(), rectWidthHalf, rectHeightHalf);
		hitLB |= wallRect.intersectsRect(checkRect);

		// オブジェクト右下の接触チェック
		checkRect = Rect(objectPrevRect.getMidX(), objectPrevRect.getMinY(), rectWidthHalf, rectHeightHalf);
		hitRB |= wallRect.intersectsRect(checkRect);
	}

	// 全て接触していない場合は埋まっていない
	if (!hitLT && !hitRT && !hitLB && !hitRB) {
		return true;
	}

	Vec2 moveVec = Vec2::ZERO;

	// 4隅すべて接触している場合は埋まっている
	if (hitLT && hitRT && hitLB && hitRB) {
		moveVec = objectPrevRect.origin - objectCrntRect.origin;
		if (moveVec.x == 0 && moveVec.y == 0) {
			return false;
		}
	}

	else if (hitLT && hitRT && hitLB && !hitRB) {
		// 右下のみ空きがある
		moveVec.x = -1.0f;
		moveVec.y = 1.0f;
	}
	else if (hitLT && hitRT && !hitLB && hitRB) {
		// 左下のみ空きがある
		moveVec.x = 1.0f;
		moveVec.y = 1.0f;
	}
	else if (hitLT && !hitRT && hitLB && hitRB) {
		// 右上のみ空きがある
		moveVec.x = -1.0f;
		moveVec.y = -1.0f;
	}
	else if (!hitLT && hitRT && hitLB && hitRB) {
		// 左上のみ空きがある
		moveVec.x = 1.0f;
		moveVec.y = -1.0f;
	}
	else if (hitLT && hitRT && !hitLB && !hitRB) {
		// 下に空きがある場合
		moveVec.y = 1.0f;
	}
	else if (!hitLT && !hitRT && hitLB && hitRB) {
		// 上に空きがある場合
		moveVec.y = -1.0f;
	}
	else if (hitLT && !hitRT && hitLB && !hitRB) {
		// 右に空きがある場合
		moveVec.x = -1.0f;
	}
	else if (!hitLT && hitRT && !hitLB && hitRB) {
		// 左に空きがある場合
		moveVec.x = 1.0f;
	}
	else if (hitLT && !hitRT && !hitLB && !hitRB) {
		// 左上のみ接触
		moveVec.x = -1.0f;

		if (move.y != 0) {
			moveVec.y = 1.0f;
		}
	}
	else if (!hitLT && hitRT && !hitLB && !hitRB) {
		// 右上のみ接触
		moveVec.x = 1.0f;

		if (move.y != 0) {
			moveVec.y = 1.0f;
		}
	}
	else if (!hitLT && !hitRT && hitLB && !hitRB) {
		// 左下のみ接触
		moveVec.x = -1.0f;
		moveVec.y = -1.0f;
	}
	else if (!hitLT && !hitRT && !hitLB && hitRB) {
		// 右下のみ接触
		moveVec.x = 1.0f;
		moveVec.y = -1.0f;
	}
	else {
		// 特殊な接触が発生していると思われる
		return false;
	}

	bool hitX, hitY;
	WallHitInfo info;
	info.boundMin = objectPrevRect.origin;
	info.boundMax = Point(objectPrevRect.getMaxX(), objectPrevRect.getMaxY());
	info.center = info.boundMin;
	info.center.x += objectPrevRect.size.width * 0.5f;
	info.center.y += objectPrevRect.size.height * 0.5f;
	info.initHit();

	Vec2 addVec = moveVec;
	wallRect = Rect(infoGroup->getBoundMin(), Size(infoGroup->getBoundMax() - infoGroup->getBoundMin()));

	float x, y;
	if (moveVec.x < 0) {
		x = infoGroup->getBoundMin().x - objectPrevRect.getMaxX() - 1.0f;
	}
	else if (moveVec.x > 0) {
		x = infoGroup->getBoundMax().x - objectPrevRect.getMinX() + 1.0f;
	}

	if (moveVec.y < 0) {
		y = infoGroup->getBoundMin().y - objectPrevRect.getMaxY() - 1.0f;
	}
	else if (moveVec.y > 0) {
		y = infoGroup->getBoundMax().y - objectPrevRect.getMinY() + 1.0f;
	}

	if (moveVec.x == 0 && moveVec.y != 0) {
		moveVec.y *= abs(y);
	}
	else if (moveVec.x != 0 && moveVec.y == 0) {
		moveVec.x *= abs(x);
	}
	else {
		moveVec *= (abs(x) > abs(y)) ? abs(y) : abs(x);
	}

	Rect cr = objectPrevRect;
	Rect pr = objectPrevRect;
	pr.origin -= moveVec;

	Rect mr = pr;
	mr.merge(cr);

	int div = 2;

	// 移動量分割数を算出
	float rectLengthSq = Vec2(wallRect.size.width, wallRect.size.height).getLengthSq();
	float moveRectLengthSq = Vec2(mr.size.width, mr.size.height).getLengthSq();
	if (rectLengthSq > 0) {
		div = (int)roundf(moveRectLengthSq / rectLengthSq);
	}

	if (div <= 1) {
		div = 2;
	}

	Vec2 pos, newPos;

	// 移動量を分割して埋まらない位置まで移動させる
	for (int i = 1; i <= div; i++) {
		pos.x = (pr.getMinX() == cr.getMinX()) ? cr.getMinX() : MathUtil::lerp(pr.getMinX(), cr.getMinX(), (float)(i - 1) / div);
		pos.y = (pr.getMinY() == cr.getMinY()) ? cr.getMinY() : MathUtil::lerp(pr.getMinY(), cr.getMinY(), (float)(i - 1) / div);
		newPos.x = (pr.getMinX() == cr.getMinX()) ? cr.getMinX() : MathUtil::lerp(pr.getMinX(), cr.getMinX(), (float)i / div);
		newPos.y = (pr.getMinY() == cr.getMinY()) ? cr.getMinY() : MathUtil::lerp(pr.getMinY(), cr.getMinY(), (float)i / div);

		moveVec = newPos - pos;

		// 埋まらない位置まで移動させてみる
		newCheckWallHit(info, hitX, hitY, infoGroup, moveVec, move);

		if (hitX) {
			objectPrevRect.origin.x = info.boundMin.x;
		}

		if (hitY) {
			objectPrevRect.origin.y = info.boundMin.y;
		}

		// 埋まらない位置まで移動できたと仮定
		if (hitX || hitY) {
			return true;
		}
	}

	// 埋まったままだと思われる
	return false;
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
float pushBackBuriedWallX(cocos2d::Rect& objectRect, float moveX, std::vector<agtk::Tile *> &tileList)
#else
float pushBackBuriedWallX(cocos2d::Rect& objectRect, float moveX, cocos2d::Array* tileList)
#endif
{
	if (moveX == 0) { return 0; }

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
	cocos2d::Ref* ref = nullptr;
#endif
	Rect wallRect;

	float left = objectRect.getMaxX() + 10000;
	float right = objectRect.getMinX() - 10000;
	bool leftUpdated = false;
	bool rightUpdated = false;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	for (auto tile : tileList) {
#else
	CCARRAY_FOREACH(tileList, ref) {
		auto tile = dynamic_cast<agtk::Tile*>(ref);
#endif
		if (tile->getType() == agtk::Tile::kTypeLimitTile) {
			if (tile->getName() == "limitTileUp" || tile->getName() == "limitTileDown")
				continue;
		}

		auto rect = tile->convertToLayerSpaceRect();
		auto pos = rect.origin;
		auto size = rect.size;
		pos.x += TILE_COLLISION_THRESHOLD;
		pos.y += TILE_COLLISION_THRESHOLD;
		size.width -= TILE_COLLISION_THRESHOLD * 2;
		size.height -= TILE_COLLISION_THRESHOLD * 2;

		float x0 = pos.x;
		float x1 = pos.x + size.width;

		wallRect = Rect(pos, size);

			if (left < x0) {
				left = x0;
				leftUpdated = true;
			}
			if (right > x1) {
				right = x1;
				rightUpdated = true;
			}
	}

	if (moveX < 0) {
		if (objectRect.getMinX() <= right && rightUpdated) {
			return right - objectRect.getMinX() + TILE_COLLISION_THRESHOLD;
		}
	}
	else if (moveX > 0) {
		if (objectRect.getMaxX() >= left && leftUpdated) {
			return objectRect.getMaxX() - left - TILE_COLLISION_THRESHOLD;
		}
	}

	return 0;
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
float pushBackBuriedWallY(cocos2d::Rect& objectRect, float moveY, std::vector<agtk::Tile *> &tileList)
#else
float pushBackBuriedWallY(cocos2d::Rect& objectRect, float moveY, cocos2d::Array* tileList)
#endif
{
	if (moveY == 0) { return 0; }

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
	cocos2d::Ref* ref = nullptr;
#endif
	Rect wallRect;

	float tileY = 0;

	float down = objectRect.getMaxY() + 10000;
	float up = objectRect.getMinY() - 10000;
	bool downUpdated = false;
	bool upUpdated = false;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	for (auto tile : tileList) {
#else
	CCARRAY_FOREACH(tileList, ref) {
		auto tile = dynamic_cast<agtk::Tile*>(ref);
#endif
		if (tile->getType() == agtk::Tile::kTypeLimitTile) {
			if (tile->getName() == "limitTileRight" || tile->getName() == "limitTileLeft")
				continue;
		}

		auto rect = tile->convertToLayerSpaceRect();
		auto pos = rect.origin;
		auto size = rect.size;
		pos.x += TILE_COLLISION_THRESHOLD;
		pos.y += TILE_COLLISION_THRESHOLD;
		size.width -= TILE_COLLISION_THRESHOLD * 2;
		size.height -= TILE_COLLISION_THRESHOLD * 2;

		float y0 = pos.y;
		float y1 = pos.y + size.height;

		wallRect = Rect(pos, size);

		if (down > y1) {
			down = y1;
			downUpdated = true;
		}
		if (up < y0) {
			up = y0;
			upUpdated = true;
		}
	}

	if (moveY < 0) {
		if (objectRect.getMinY() <= down && downUpdated) {
			return down - objectRect.getMinY() + TILE_COLLISION_THRESHOLD;
		}
	}
	else if (moveY > 0) {
		if (objectRect.getMaxY() >= up && upUpdated) {
			return objectRect.getMaxY() - up - TILE_COLLISION_THRESHOLD;
		}
	}

	return 0;
}

/**
* 壁判定チェック
* @param	wallHitInfo				壁判定情報(入力/出力)
* @param	hitX					X軸方向での衝突の有無フラグ(出力)
* @param	hitY					Y軸方向での衝突の有無フラグ(出力)
* @param	tileThreshold			タイルとの衝突で利用するしきい値
* @param	tileList				衝突予定のタイルリスト
* @param	slopeList				衝突予定の坂リスト
* @param	passableSlopeList		通過可能な坂リスト
* @apram	moveVec					移動ベクトル
* @param	tileThresholdMoveVec
* @param	oldRect					1フレーム前の矩形
* @param	falling					落下中フラグ
* @param	slope					坂フラグ
*/
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void newCheckWallHit(WallHitInfo &wallHitInfo, bool& hitX, bool& hitY, int tileThreshold, std::vector<agtk::Tile *> &tileList, agtk::MtVector<agtk::Slope *> *slopeList, agtk::MtVector<agtk::Slope *> *passableSlopeList, Vec2& moveVec, Vec2& tileThresholdMoveVec, Rect& oldRect, bool falling, bool slope, agtk::HitWallTiles& hitTiles)
#else
void newCheckWallHit(WallHitInfo &wallHitInfo, bool& hitX, bool& hitY, int tileThreshold, cocos2d::Array* tileList, cocos2d::Array* slopeList, cocos2d::Array* passableSlopeList, Vec2& moveVec, Vec2& tileThresholdMoveVec, Rect& oldRect, bool falling, bool slope, agtk::HitWallTiles& hitTiles)
#endif
{
	Rect rect = Rect(wallHitInfo.boundMin, wallHitInfo.size);
	Rect initRect = rect;
	agtk::Object *object = wallHitInfo.object;

	// 坂確認用矩形を構築
	Rect slopeCheckRect = Rect::ZERO;
	if (object != nullptr) {
		
		// 坂確認用の矩形が設定されている場合
		auto list = object->getObjectCollision()->getSlopeCheckRect()->getInfoGroupDifferenceList();

		if (wallHitInfo.id < static_cast<int>(list.size()) && list.size() > 0) {

			auto slopeRect = object->getObjectCollision()->getSlopeCheckRect()->getRect();

			// 坂確認用の矩形のサイズを設定
			slopeCheckRect.size = slopeRect.size;
			// 坂確認矩形の左下と壁当たりの左下との差を格納
			slopeCheckRect.origin = list[wallHitInfo.id];
		}
	}

	newCheckWallHit(rect, slopeCheckRect, hitX, hitY, tileThreshold, tileList, slopeList, passableSlopeList, moveVec, tileThresholdMoveVec, oldRect, object, falling, slope, hitTiles);

	Vec2 pushBack = rect.origin - initRect.origin;
	wallHitInfo.boundMin += pushBack;
	wallHitInfo.boundMax += pushBack;
}

/**
* 壁判定チェック
* @param	crntRect				壁判定矩形(入力/出力)
* @param	slopeCheckRect			坂判定用矩形
* @param	hitX					X軸方向での衝突の有無フラグ(出力)
* @param	hitY					Y軸方向での衝突の有無フラグ(出力)
* @param	tileThreshold			タイルとの衝突で利用するしきい値
* @param	tileList				衝突予定のタイルリスト
* @param	slopeList				衝突予定の坂リスト
* @param	passableSlopeList		通過可能な坂リスト
* @apram	moveVec					移動ベクトル
* @param	tileThresholdMoveVec
* @param	oldRect					1フレーム前の矩形
* @param	object				    オブジェクト
* @param	falling					落下中フラグ
* @param	slope					坂フラグ
*/
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void newCheckWallHit(Rect& crntRect, Rect slopeCheckRect, bool& hitX, bool& hitY, int tileThreshold, std::vector<agtk::Tile *> &tileList, agtk::MtVector<agtk::Slope *> *slopeList, agtk::MtVector<agtk::Slope *> *passableSlopeList, Vec2& moveVec, Vec2& tileThresholdMoveVec, Rect& oldRect, agtk::Object* object, bool falling, bool slope, agtk::HitWallTiles& hitTiles)
#else
void newCheckWallHit(Rect& crntRect, Rect slopeCheckRect, bool& hitX, bool& hitY, int tileThreshold, cocos2d::Array* tileList, cocos2d::Array* slopeList, cocos2d::Array* passableSlopeList, Vec2& moveVec, Vec2& tileThresholdMoveVec, Rect& oldRect, agtk::Object* object, bool falling, bool slope, agtk::HitWallTiles& hitTiles)
#endif
{
	auto projectData = GameManager::getInstance()->getProjectData();

	bool isHit = false;
	bool isHitLimitTile = false;

	hitX = false;
	hitY = false;
	float pushBackX = 0.0f;
	float pushBackY = 0.0f;

	Vec2 oldPos = crntRect.origin - moveVec;
	Vec2 move = moveVec;
	if (move == cocos2d::Vec2::ZERO) move = crntRect.origin - oldRect.origin;
	Rect checkRect, prevRect, wallRect;

	prevRect = Rect(oldPos, crntRect.size);

	cocos2d::Ref* ref = nullptr;

	//押し戻し方向から壁判定Bitを取得する。
	std::function<int(float x, float y)> getWallBit = [](float pushBackX, float pushBackY) {
		int bit = 0;
		if (pushBackX != 0.0f) {
			bit = (pushBackX > 0.0f) ? agtk::data::ObjectActionLinkConditionData::kWallBitLeft : agtk::data::ObjectActionLinkConditionData::kWallBitRight;
		}
		if (pushBackY != 0.0f) {
			bit = (pushBackY > 0.0f) ? agtk::data::ObjectActionLinkConditionData::kWallBitDown : agtk::data::ObjectActionLinkConditionData::kWallBitUp;
		}
		return bit;
	};

	//壁に衝突したタイル情報と判定bitをリストに追加する。
	std::function<void(agtk::HitWallTiles&, agtk::Tile *, int)> addHitWallTile = [](agtk::HitWallTiles& hitWallTiles, agtk::Tile *tile, int wallBit) {
		if (wallBit == 0) {
			return;
		}
		hitWallTiles.emplace_back(std::make_pair(wallBit, tile));
	};

	// 指定矩形がタイルと接触が発生しているかをチェック
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	std::function<bool(Rect& rect, std::vector<agtk::Tile *> &)> checkWallHit = [&](Rect& objRect, std::vector<agtk::Tile *> &tileList) {
#else
	std::function<bool(Rect& rect, Array*)> checkWallHit = [&](Rect& objRect, Array* tileList) {
#endif
		Rect wallRect;
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		for (auto tile : tileList) {
#else
		cocos2d::Ref *ref = nullptr;

		CCARRAY_FOREACH(tileList, ref) {
			auto tile = dynamic_cast<agtk::Tile*>(ref);
#endif
			if (object && !(object->getObjectData()->getCollideWithTileGroupBit() & tile->getGroupBit())) {
				continue;
			}
			auto rect = tile->convertToLayerSpaceRect();
			auto pos = rect.origin;
			auto size = rect.size;
			pos.x += TILE_COLLISION_THRESHOLD;
			pos.y += TILE_COLLISION_THRESHOLD;
			size.width -= TILE_COLLISION_THRESHOLD * 2;
			size.height -= TILE_COLLISION_THRESHOLD * 2;

			wallRect = Rect(pos, size);

			if (wallRect.intersectsRect(objRect)) {
				return true;
			}
		}

		return false;
	};

	// 指定タイルの上下方向に他のタイルが存在するかをチェック
	std::function<void(Tile*, Rect&, bool&, bool&)> existTileUpDown = [&](Tile* tile, Rect& tileRect, bool& existUp, bool& existDown) {

		existUp = false;
		existDown = false;

		auto scene = GameManager::getInstance()->getCurrentScene();
		auto sceneLayer = scene->getSceneLayer(tile->getLayerId());

		if (sceneLayer) {
			Point boundMin = Point(tileRect.getMinX(), tileRect.getMinY() - tileRect.size.height);
			Point boundMax = Point(tileRect.getMaxX(), tileRect.getMaxY() + tileRect.size.height);
			auto tileList = sceneLayer->getCollisionTileList(boundMin, boundMax);

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			if (tileList.size() <= 1) { return; }
#else
			if (tileList->count() <= 1) { return; }
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			for (auto tile2 : tileList) {
#else
			cocos2d::Ref* ref = nullptr;
			CCARRAY_FOREACH(tileList, ref) {
				auto tile2 = dynamic_cast<agtk::Tile*>(ref);
#endif
				if (object && !(object->getObjectData()->getCollideWithTileGroupBit() & tile2->getGroupBit())) {
					continue;
				}
				if (tile->getTileX() == tile2->getTileX() && tile->getTileY() - 1 == tile2->getTileY()) {
					existUp = true;
				}
				else if (tile->getTileX() == tile2->getTileX() && tile->getTileY() + 1 == tile2->getTileY()) {
					existDown = true;
				}
			}
		}
	};

	// 指定タイルの左右方向に他のタイルが存在するかをチェック
	std::function<void(Tile*, Rect&, bool&, bool&)> existTileLeftRight = [&](Tile* tile, Rect& tileRect, bool& existLeft, bool& existRight) {

		existLeft = false;
		existRight = false;

		auto scene = GameManager::getInstance()->getCurrentScene();
		auto sceneLayer = scene->getSceneLayer(tile->getLayerId());

		if (sceneLayer) {
			Point boundMin = Point(tileRect.getMinX() - tileRect.size.width, tileRect.getMinY());
			Point boundMax = Point(tileRect.getMaxX() + tileRect.size.width, tileRect.getMaxY());
			auto tileList = sceneLayer->getCollisionTileList(boundMin, boundMax);

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			if (tileList.size() <= 1) { return; }
#else
			if (tileList->count() <= 1) { return; }
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			for (auto tile2 : tileList) {
#else
			cocos2d::Ref* ref = nullptr;
			CCARRAY_FOREACH(tileList, ref) {
				auto tile2 = dynamic_cast<agtk::Tile*>(ref);
#endif
				if (object && !(object->getObjectData()->getCollideWithTileGroupBit() & tile2->getGroupBit())) {
					continue;
				}
				if (tile->getTileY() == tile2->getTileY() && tile->getTileX() - 1 == tile2->getTileX()) {
					existLeft = true;
				}
				else if (tile->getTileY() == tile2->getTileY() && tile->getTileX() + 1 == tile2->getTileX()) {
					existRight = true;
				}
			}
		}
	};

	// X方向の押し戻し量を算出
	std::function<bool(agtk::Tile *)> calcPushBackX = [&](agtk::Tile *tile)->bool {

		// 現在の矩形のX座標を更新
		checkRect.origin.x = crntRect.origin.x;

		if (tile->getType() == agtk::Tile::kTypeLimitTile && !(dynamic_cast<LimitTile*>(tile)->isNeedCheck(object))) {
			return false;
		}
		int wallBit = tile->getWallBit();

		Rect rect = tile->convertToLayerSpaceRect();
		Vec2 pos = rect.origin;
		Size size = rect.size;
#if 1//ACT2-4089 タイルの壁判定に接触した際に画面が細かく振動するバグ修正。
		pos.y += TILE_COLLISION_THRESHOLD;
		size.height -= TILE_COLLISION_THRESHOLD * 2;
#else
		pos.x += TILE_COLLISION_THRESHOLD;
		pos.y += TILE_COLLISION_THRESHOLD;
		size.width -= TILE_COLLISION_THRESHOLD * 2;
		size.height -= TILE_COLLISION_THRESHOLD * 2;
#endif

		wallRect = Rect(pos, size);

		bool isThroughTile = false;

		// 坂リストが設定されている場合
		if (slopeList != nullptr) {

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			for (int i = 0; i < (int)slopeList->count(); i++) {
				auto slope = (*slopeList)[i];
#else
			cocos2d::Ref* ref2 = nullptr;
			CCARRAY_FOREACH(slopeList, ref2) {
				auto slope = dynamic_cast<agtk::Slope*>(ref2);
#endif
				// 坂がタイルの上部と接続されている場合
				if (slope->checkConnectTileUp(wallRect, checkRect, moveVec)) {
					// 坂を上りきる際にタイルと接触し綺麗に移動ができないので
					// タイルとの接触確認をスキップする
					isThroughTile = true;
					break;
				}
			}
		}

		// 当たり確認をスルーする場合
		if (isThroughTile) {
			return false;
		}

		float x0 = wallRect.getMinX();
		float x1 = wallRect.getMaxX();
		float diff = 1.0f + TILE_COLLISION_THRESHOLD;

		isHit = false;
		isHitLimitTile = false;

		if (wallRect.intersectsRect(checkRect)) {

			// 通常タイルの場合
			if (tile->getType() == agtk::Tile::kTypeTile) {

				if (object && !(object->getObjectData()->getCollideWithTileGroupBit() & tile->getGroupBit())) {
					return false;
				}

				// 右移動時
				if (move.x > 0) {
					// 判定対象の矩形の右側とタイルの左壁と接触あり
					if (x0 <= checkRect.getMaxX()) {
						// 壁判定が４方向すべて設定されている場合
						if (wallBit == 0x0f) {
							isHit = true;
						}
						// 左方向に設定されている場合
						else if (wallBit & (1 << agtk::data::TilesetData::Left)) {
							if ((oldRect.getMaxX() - diff) <= x0) {
								isHit = true;
							}
						}

						if (isHit) {
							// タイルの右壁を超えている場合
							if (x1 <= checkRect.getMaxX()) {
								if (x1 - TILE_COLLISION_THRESHOLD <= oldRect.getMinX() && oldRect.getMinX() <= x1) {
									// oldRectがタイル閾値内でめり込んでいる場合は右壁へ移動する。
									pushBackX = rect.getMaxX() - checkRect.getMinX();
								} else {
									// めり込む前まで戻す
									pushBackX = oldRect.getMaxX() - checkRect.getMaxX();
								}
							}
							else {
								//pushBackX = (x0 - checkRect.getMaxX()) - TILE_COLLISION_THRESHOLD;
								pushBackX = rect.getMinX() - checkRect.getMaxX();
							}
						}
					}
				}
				// 左移動時
				else if (move.x < 0) {
					// 判定対象の矩形の左側とタイルの右壁と接触あり
					if (checkRect.getMinX() <= x1) {
						// 壁判定が４方向すべて設定されている場合
						if (wallBit == 0x0f) {
							isHit = true;
						}
						// 右方向に設定されている場合
						else if (wallBit & (1 << agtk::data::TilesetData::Right)) {
							if (x1 <= (oldRect.getMinX() + diff)) {
								isHit = true;
							}
						}

						if (isHit) {
							// タイルの左壁を超えている場合
							//! タイルがチェック対象より小さいまたはめり込んだ可能性
							if (checkRect.getMinX() <= x0) {
								if (x0 <= oldRect.getMaxX() && oldRect.getMaxX() <= x0 + TILE_COLLISION_THRESHOLD) {
									// oldRectがタイル閾値内でめり込んでいる場合は左壁へ移動する。
									pushBackX = rect.getMinX() - checkRect.getMaxX();
								} else {
									// めり込む前まで戻す
									pushBackX = oldRect.getMinX() - checkRect.getMinX();
								}
							}
							else {
								//pushBackX = (x1 - checkRect.getMinX()) + TILE_COLLISION_THRESHOLD;
								pushBackX = rect.getMaxX() - checkRect.getMinX();
							}
						}
					}
				}
			}
			// 行動範囲制限の場合
			else {
				auto name = tile->getName();
				// 右に設定された行動範囲制限の場合
				if (name.compare("limitTileRight") == 0) {
					isHitLimitTile = true;
					//pushBackX = (x0 - checkRect.getMaxX()) - TILE_COLLISION_THRESHOLD;
					pushBackX = rect.getMinX() - checkRect.getMaxX();
				}
				// 左に設定された行動範囲制限の場合
				else if (name.compare("limitTileLeft") == 0) {
					isHitLimitTile = true;
					//pushBackX = (x1 - checkRect.getMinX()) + TILE_COLLISION_THRESHOLD;
					pushBackX = rect.getMaxX() - checkRect.getMinX();
				}
				// 左右に移動して上下の行動制限範囲に食い込んだ場合
				else if (name.compare("limitTileDown") == 0 || name.compare("limitTileUp") == 0) {
					isHitLimitTile = true;
					pushBackX = oldRect.getMinX() - checkRect.getMinX();
				}
				// 上下の行動範囲制限に関する処理はここで行わない
			}

			// 接触が発生した場合
			if (isHit || isHitLimitTile) {

				// 閾値が設定されている場合
				if (slope == false && tileThreshold > 0 && (fabsf(moveVec.x) > fabsf(moveVec.y))) {

					// 上下にタイルが存在するかをチェック
					bool existUp, existDown;
					existTileUpDown(tile, wallRect, existUp, existDown);

					float length = tileThreshold + 1;
					if (checkRect.getMidY() > wallRect.getMidY() && !existUp) {

						length = wallRect.getMaxY() - checkRect.getMinY();
					}
					else if (checkRect.getMidY() < wallRect.getMidY() && !existDown) {
						length = checkRect.getMaxY() - wallRect.getMinY();
					}

					// Y座標の調整量が閾値以下の場合
					if (tileThreshold >= length) {
						Rect r = checkRect;
						r.origin.x += pushBackX;

						if (checkRect.getMidY() > wallRect.getMidY()) {
							r.origin.y += length;

							// 自動調節した位置に矩形を配置した場合、タイルに埋まってしまうかをチェックする
							if (!checkWallHit(r, tileList)) {
								float val = (length + TILE_COLLISION_THRESHOLD);

								if (val > fabsf(tileThresholdMoveVec.x)) {
									val = fabsf(tileThresholdMoveVec.x);
								}
								checkRect.origin.y += val;
								crntRect.origin.y += val;
								moveVec.y = 0;
								hitY = true;
								addHitWallTile(hitTiles, tile, getWallBit(0, val));
								//continue;
							}
						}
						else if (checkRect.getMidY() < wallRect.getMidY()) {
							r.origin.y -= length;

							// 自動調節した位置に矩形を配置した場合、タイルに埋まってしまうかをチェックする
							if (!checkWallHit(r, tileList)) {
								float val = (length + TILE_COLLISION_THRESHOLD);

								if (val > fabsf(tileThresholdMoveVec.x)) {
									val = fabsf(tileThresholdMoveVec.x);
								}
								checkRect.origin.y -= val;
								crntRect.origin.y -= val;
								moveVec.y = 0;
								hitY = true;
								addHitWallTile(hitTiles, tile, getWallBit(0, val));
								//continue;
							}
						}
					}
				}

				if (isHit || (isHitLimitTile && pushBackX != 0.0f)) {
					hitX = true;
					//※押し戻しがpushBackX=0の場合は、移動方向から壁判定bitを取得する。
					addHitWallTile(hitTiles, tile, getWallBit((pushBackX != 0.0f) ? pushBackX : -moveVec.x, 0));
					// 押し戻しを行い座標を再度設定する
					crntRect.origin.x += pushBackX;
				}
			}
		}
		return isHit || isHitLimitTile;
	};

	// Y方向の押し戻し量を算出
	std::function<bool(agtk::Tile *)> calcPushBackY = [&](agtk::Tile *tile) {

		// 現在の矩形のY座標を更新
		checkRect.origin.y = crntRect.origin.y;

		if (tile->getType() == agtk::Tile::kTypeLimitTile && !dynamic_cast<LimitTile*>(tile)->isNeedCheck(object)) {
			return false;
		}
		int wallBit = tile->getWallBit();

		Rect rect = tile->convertToLayerSpaceRect();
		Vec2 pos = rect.origin;
		Size size = rect.size;
#if 1//ACT2-4089 タイルの壁判定に接触した際に画面が細かく振動するバグ修正。
		pos.x += TILE_COLLISION_THRESHOLD;
		size.width -= TILE_COLLISION_THRESHOLD * 2;
#else
		pos.x += TILE_COLLISION_THRESHOLD;
		pos.y += TILE_COLLISION_THRESHOLD;
		size.width -= TILE_COLLISION_THRESHOLD * 2;
		size.height -= TILE_COLLISION_THRESHOLD * 2;
#endif

		wallRect = Rect(pos, size);

		bool isThroughTile = false;

		// 坂リストが設定されている場合
		if (slopeList != nullptr) {

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			for (int i = 0; i < (int)slopeList->count(); i++) {
				auto slope = (*slopeList)[i];
#else
			cocos2d::Ref* ref2 = nullptr;
			CCARRAY_FOREACH(slopeList, ref2) {
				auto slope = dynamic_cast<agtk::Slope*>(ref2);
#endif
				// 坂がタイルの上部と接続されている場合
				if (slope->checkConnectTileUp(wallRect, checkRect, moveVec)) {
					// 坂を上りきる際にタイルと接触し綺麗に移動ができないので
					// タイルとの接触確認をスキップする
					isThroughTile = true;
					break;
				}
			}
		}

		// 当たり確認をスルーする場合
		if (isThroughTile) {
			return false;
		}

		float y0 = wallRect.getMinY();
		float y1 = wallRect.getMaxY();
		float diff = 1.0f + TILE_COLLISION_THRESHOLD;

		isHit = false;
		isHitLimitTile = false;

		if (wallRect.intersectsRect(checkRect)) {

			// 通常タイルの場合
			if (tile->getType() == agtk::Tile::kTypeTile) {

				if (object && !(object->getObjectData()->getCollideWithTileGroupBit() & tile->getGroupBit())) {
					return false;
				}

				// 上移動時
				if (moveVec.y > 0) {
					// タイルの下辺と確認用矩形の上辺の接触あり
					if (y0 <= checkRect.getMaxY()) {
						// 壁判定が４方向すべて設定されている場合
						if (wallBit == 0x0f) {
							isHit = true;
						}
						// 下方向に設定されている場合
						else if ((wallBit & (1 << agtk::data::TilesetData::Down)) &&
							(oldRect.getMaxY() - diff) <= y0) {
							isHit = true;
						}

						if (isHit) {
							// タイルの上辺を超える場合
							//! めりこんだ可能性
							if (y1 <= checkRect.getMaxY()) {
								// めり込む前まで戻す
								pushBackY = oldRect.getMaxY() - checkRect.getMaxY();
							}
							else {
								//pushBackY = (y0 - checkRect.getMaxY()) - TILE_COLLISION_THRESHOLD;
								pushBackY = rect.getMinY() - checkRect.getMaxY();
							}
						}
					}
				}
				// 下移動時
				else if (moveVec.y < 0) {
					// 接触あり
					if (checkRect.getMinY() <= y1) {
						// 壁判定が４方向すべて設定されている場合
						if (wallBit == 0x0f) {
							isHit = true;
						}
						// 上方向に設定されている場合
						else if ((wallBit & (1 << agtk::data::TilesetData::Up)) &&
							y1 <= (oldRect.getMinY() + diff)) {
							isHit = true;
						}

						if (isHit) {
							if (checkRect.getMaxY() <= y0 + TILE_COLLISION_THRESHOLD) {
								// オブジェクトがタイルの下からに当たって、タイル閾値以下になった場合は、接触していないと判定する。
								isHit = false;
								} else
								// タイルの下辺を超える場合
								//! めりこんだ可能性
								if (checkRect.getMinY() <= y0) {
									// めり込む前まで戻す
									pushBackY = oldRect.getMinY() - checkRect.getMinY();
								}
								else {
									//pushBackY = (y1 - checkRect.getMinY()) + TILE_COLLISION_THRESHOLD;
									pushBackY = rect.getMaxY() - checkRect.getMinY();
								}
						}
					}
				}
			}
			// 行動範囲制限の場合
			else {
				auto name = tile->getName();
				// 上に設定された行動範囲制限の場合
				if (name.compare("limitTileUp") == 0) {
					isHitLimitTile = true;
					//pushBackY = (y0 - checkRect.getMaxY()) - TILE_COLLISION_THRESHOLD;
					pushBackY = rect.getMinY() - checkRect.getMaxY();
				}
				// 下に設定された行動範囲制限の場合
				else if (name.compare("limitTileDown") == 0) {
					isHitLimitTile = true;
					//pushBackY = (y1 - checkRect.getMinY()) + TILE_COLLISION_THRESHOLD;
					pushBackY = rect.getMaxY() - checkRect.getMinY();
				}

				// 左右の行動範囲制限に関する処理はここで行わない
			}

			// Y方向の移動で接触が発生した場合
			if (isHit || isHitLimitTile) {

				// 落下中でなく閾値が設定されている場合
				if ((falling == false && slope == false) && tileThreshold > 0 && (fabsf(moveVec.x) < fabsf(moveVec.y))) {

					// 左右にタイルが存在するかをチェック
					bool existLeft, existRight;
					existTileLeftRight(tile, wallRect, existLeft, existRight);

					float length = tileThreshold + 1;
					if (checkRect.getMidX() > wallRect.getMidX() && !existRight) {
						length = wallRect.getMaxX() - crntRect.getMinX();
					}
					else if (checkRect.getMidX() < wallRect.getMidX() && !existLeft) {
						length = crntRect.getMaxX() - wallRect.getMinX();
					}

					// X座標の調整量が閾値以下の場合
					if (tileThreshold >= length) {
						Rect r = checkRect;
						r.origin.y += pushBackY;

						if (checkRect.getMidX() > wallRect.getMidX()) {
							r.origin.x += length;

							// 自動調節した位置に矩形を配置した場合、タイルに埋まってしまうかをチェックする
							if (!checkWallHit(r, tileList)) {
								float val = (length + TILE_COLLISION_THRESHOLD);

								if (val > fabsf(tileThresholdMoveVec.y)) {
									val = fabsf(tileThresholdMoveVec.y);
								}
								checkRect.origin.x += val;
								crntRect.origin.x += val;
								hitX = true;
								addHitWallTile(hitTiles, tile, getWallBit(val, 0));
								//continue;
							}
						}
						else if (checkRect.getMidX() < wallRect.getMidX()) {
							r.origin.x -= length;

							// 自動調節した位置に矩形を配置した場合、タイルに埋まってしまうかをチェックする
							if (!checkWallHit(r, tileList)) {
								float val = (length + TILE_COLLISION_THRESHOLD);

								if (val > fabsf(tileThresholdMoveVec.y)) {
									val = fabsf(tileThresholdMoveVec.y);
								}
								checkRect.origin.x -= val;
								crntRect.origin.x -= val;
								hitX = true;
								addHitWallTile(hitTiles, tile, getWallBit(val, 0));
								//continue;
							}
						}
					}
				}

				if (isHit || (isHitLimitTile && pushBackY != 0.0f)) {
					hitY = true;
					//※押し戻しがpushBackY=0の場合は、移動方向から壁判定bitを取得する。
					addHitWallTile(hitTiles, tile, getWallBit(0, (pushBackY != 0.0f) ? pushBackY: -moveVec.y));
					// 押し戻しを行い座標を再度設定する
					crntRect.origin.y += pushBackY;
				}
			}
		}
		return isHit || isHitLimitTile;
	};

	// 矩形を前フレームの位置に設定
	checkRect = prevRect;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	auto tileList2 = tileList;
	do {
		int count = 0;
		for(auto it = tileList2.begin(); it != tileList2.end(); ++it, count++) {
			auto tile = *it;
			//タイルとオブジェクトの壁判定の重なり具合をチェック。
			Rect rect = tile->convertToLayerSpaceRect();
			float minX = (rect.getMinX() < checkRect.getMinX()) ? checkRect.getMinX() : rect.getMinX();
			float maxX = (rect.getMaxX() < checkRect.getMaxX()) ? rect.getMaxX() : checkRect.getMaxX();
			float width = abs(minX - maxX);
			float minY = (rect.getMinY() < checkRect.getMinY()) ? checkRect.getMinY() : rect.getMinY();
			float maxY = (rect.getMaxY() < checkRect.getMaxY()) ? rect.getMaxY() : checkRect.getMaxY();
			float height = abs(minY - maxY);

			cocos2d::Vec2 pos = checkRect.origin;
			bool bPushBackX = false;
			bool bPushBackY = false;

			if (width < height || height < TILE_COLLISION_THRESHOLD) {
				// X方向に移動を行い壁との接触を確認する
				bPushBackX = calcPushBackX(tile);
				checkRect.origin.x = crntRect.origin.x;
				// Y方向に移動を行い壁との接触を確認する
				bPushBackY = calcPushBackY(tile);
				checkRect.origin.y = crntRect.origin.y;
			}
			else {
				// Y方向に移動を行い壁との接触を確認する
				bPushBackY = calcPushBackY(tile);
				checkRect.origin.y = crntRect.origin.y;
				// X方向に移動を行い壁との接触を確認する
				bPushBackX = calcPushBackX(tile);
				checkRect.origin.x = crntRect.origin.x;
			}
			if (bPushBackX == false && bPushBackY == false) {
				checkRect.origin = pos;
				continue;
			}
			tileList2.erase(it);
			break;
		}
		if (count == tileList2.size()) {
			break;
		}
	} while (tileList2.size() > 0);
#else
	for (int i = 0; i < tileList->count(); i++) {
		int step = (moveVec.x <= 0) ? i : (tileList->count() - 1) - i;
		auto tile = dynamic_cast<agtk::Tile*>(tileList->getObjectAtIndex(step));

		//タイルとオブジェクトの壁判定の重なり具合をチェック。
		Rect rect = tile->convertToLayerSpaceRect();
		float minX = (rect.getMinX() < checkRect.getMinX()) ? checkRect.getMinX() : rect.getMinX();
		float maxX = (rect.getMaxX() < checkRect.getMaxX()) ? rect.getMaxX() : checkRect.getMaxX();
		float width = abs(minX - maxX);
		float minY = (rect.getMinY() < checkRect.getMinY()) ? checkRect.getMinY() : rect.getMinY();
		float maxY = (rect.getMaxY() < checkRect.getMaxY()) ? rect.getMaxY() : checkRect.getMaxY();
		float height = abs(minY - maxY);

		if (width < height || height < TILE_COLLISION_THRESHOLD) {
			// X方向に移動を行い壁との接触を確認する
			calcPushBackX(tile);
			checkRect.origin.x = crntRect.origin.x;
			// Y方向に移動を行い壁との接触を確認する
			calcPushBackY(tile);
			checkRect.origin.y = crntRect.origin.y;
		}
		else {
			// Y方向に移動を行い壁との接触を確認する
			calcPushBackY(tile);
			checkRect.origin.y = crntRect.origin.y;
			// X方向に移動を行い壁との接触を確認する
			calcPushBackX(tile);
			checkRect.origin.x = crntRect.origin.x;
		}
	}
#endif

	// 坂が設定されている場合
	if (slopeList != nullptr) {

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		for (int i = 0; i < (int)slopeList->count(); i++) {
			auto slope = (*slopeList)[i];
#else
		CCARRAY_FOREACH(slopeList, ref) {
			auto slope = dynamic_cast<agtk::Slope*>(ref);
#endif
			// 通過中の坂の場合は押し戻しを行わない
			if (passableSlopeList->containsObject(slope)) { continue; }

			// 坂判定矩形情報を基に判定矩形を構築
			auto checkRect = crntRect;
			checkRect.size = slopeCheckRect.size;
			checkRect.origin -= slopeCheckRect.origin;

			// 押し戻しが発生した場合（X方向）
			slope->setObjectTmp(object);
			if (slope->checkPushBackX(object->getObjectData()->isGroupPlayer(), checkRect, oldRect, tileList, moveVec.x, pushBackX, 0.1f)) {
				if (pushBackX != 0.0f) {
					hitX = true;
					crntRect.origin.x += pushBackX;
				}
			}

			// 押し戻しが発生した場合（Y方向）
			if (slope->checkPushBackY(crntRect, oldRect, tileList, moveVec.y, pushBackY, 0.1f)) {
				if (pushBackY != 0.0f) {
					hitY = true;
					crntRect.origin.y += pushBackY;
				}
			}
			slope->setObjectTmp(nullptr);
		}
	}
}

void newCheckWallHit(WallHitInfo &wallHitInfo, bool& hitX, bool& hitY, agtk::WallHitInfoGroup* infoGroup, Vec2& moveVec, Vec2 nowMoveVec)
{
	bool isHit = false;
	hitX = false;
	hitY = false;
	float pushBackX = 0.0f;
	float pushBackY = 0.0f;

	Vec2 oldPos = wallHitInfo.boundMin - moveVec;
	Rect crntRect, prevRect, wallRect;

	prevRect = Rect(oldPos, Size(wallHitInfo.boundMax - wallHitInfo.boundMin));

	std::function<bool(cocos2d::Rect, cocos2d::Rect)> intersectsRectX = [&](cocos2d::Rect rect1, cocos2d::Rect rect2) {
		//X方向の閾値無しで壁と接触判定する。
		cocos2d::Rect rect1x = rect1;
		rect1x.origin.y += TILE_COLLISION_THRESHOLD;
		rect1x.size.height -= TILE_COLLISION_THRESHOLD * 2;
		return rect1x.intersectsRect(rect2);
	};

	// X方向の移動だけで壁と接触するかをチェック
	std::function<void(WallHitInfo &)> movePushBackX = [&](WallHitInfo &hitInfo) {
		// 現在の矩形のX座標を更新
		crntRect.origin.x = wallHitInfo.boundMin.x;

		wallRect = Rect(hitInfo.boundMin, Size(hitInfo.boundMax - hitInfo.boundMin));
		auto tmpWallRect = wallRect;

		//X方向の閾値無しで壁と接触判定する。
		bool ret = intersectsRectX(wallRect, crntRect);

		wallRect.origin.x += TILE_COLLISION_THRESHOLD;
		wallRect.origin.y += TILE_COLLISION_THRESHOLD;
		wallRect.size.width -= TILE_COLLISION_THRESHOLD * 2;
		wallRect.size.height -= TILE_COLLISION_THRESHOLD * 2;

		float x0 = wallRect.getMinX();
		float y0 = wallRect.getMinY();
		float x1 = wallRect.getMaxX();
		float y1 = wallRect.getMaxY();
		float xc = wallRect.getMidX();
		float yc = wallRect.getMidY();

		isHit = false;

		if (ret) {
			// 右移動時
			if (moveVec.x > 0) {
				if (xc >= wallHitInfo.center.x) {
					pushBackX = tmpWallRect.getMinX() - wallHitInfo.boundMax.x;
				}
				else {
					pushBackX = tmpWallRect.getMaxX() - wallHitInfo.boundMin.x;
				}
				isHit = (pushBackX != 0.0f) ? true : false;
			}
			// 左移動時
			else if (moveVec.x < 0) {
				if (xc <= wallHitInfo.center.x) {
					pushBackX = tmpWallRect.getMaxX() - wallHitInfo.boundMin.x;
				}
				else {
					pushBackX = tmpWallRect.getMinX() - wallHitInfo.boundMax.x;
				}
				isHit = (pushBackX != 0.0f) ? true : false;
			}

			if (isHit) {
				hitX = true;
				// 押し戻しを行い座標を再度設定する
				wallHitInfo.boundMin.x += pushBackX;
				wallHitInfo.boundMax.x += pushBackX;
				wallHitInfo.center.x += pushBackX;
			}
		}
	};

	// Y方向の移動だけで壁当たりと接触するかを確認
	std::function<void(WallHitInfo &)> movePushBackY = [&](WallHitInfo &hitInfo) {
		// Y座標を更新
		crntRect.origin.y = wallHitInfo.boundMin.y;

		wallRect = Rect(hitInfo.boundMin, Size(hitInfo.boundMax - hitInfo.boundMin));
		auto wallRect2 = wallRect;
		auto rect = wallRect;

		wallRect.origin.x += TILE_COLLISION_THRESHOLD;
		wallRect.origin.y += TILE_COLLISION_THRESHOLD;
		wallRect.size.width -= TILE_COLLISION_THRESHOLD * 2;
		wallRect.size.height -= TILE_COLLISION_THRESHOLD * 2;
		if (abs(moveVec.y) < 1.0f) {
			wallRect2.origin.x += TILE_COLLISION_THRESHOLD;
			wallRect2.size.width -= TILE_COLLISION_THRESHOLD * 2;
		}

		float x0 = wallRect.getMinX();
		float y0 = wallRect.getMinY();
		float x1 = wallRect.getMaxX();
		float y1 = wallRect.getMaxY();
		float xc = wallRect.getMidX();
		float yc = wallRect.getMidY();

		isHit = false;

		if ((abs(moveVec.y) >= 1.0f && wallRect.intersectsRect(crntRect))
		||  (moveVec.y == 0.0f && !wallRect.intersectsRect(crntRect) && wallRect2.intersectsRect(crntRect))) {

			// 上移動時
			if (moveVec.y > 0) {
				if (yc >= wallHitInfo.center.y) {
					pushBackY = rect.getMinY() - wallHitInfo.boundMax.y;
				}
				else {
					pushBackY = rect.getMaxY() - wallHitInfo.boundMin.y;
				}
				isHit = (pushBackY != 0.0f) ? true : false;
			}
			// 下移動時
			else if (moveVec.y < 0) {
				if (yc <= wallHitInfo.center.y) {
					pushBackY = rect.getMaxY() - wallHitInfo.boundMin.y;
				}
				else {
					pushBackY = rect.getMinY() - wallHitInfo.boundMax.y;
				}
				isHit = (pushBackY != 0.0f) ? true : false;
			}
			// 移動量が無い。
			else {
				if (wallHitInfo.object != nullptr) {
					//食い込んでいる場合は戻す。
					float h1 = std::abs(rect.getMaxY() - wallHitInfo.boundMin.y);
					float h2 = std::abs(rect.getMinY() - wallHitInfo.boundMax.y);
					if (h1 != 0.0f && h2 != 0.0f) {
						if (h1 < h2) {
							pushBackY = rect.getMaxY() - wallHitInfo.boundMin.y;
						}
						else if (h1 > h2) {
							pushBackY = rect.getMinY() - wallHitInfo.boundMax.y;
						}
					}
					isHit = (pushBackY != 0.0f) ? true : false;
				}
			}

			// Y方向の移動で接触が発生した場合
			if (isHit) {
				hitY = true;
				// 押し戻しを行い座標を再度設定する
				wallHitInfo.boundMin.y += pushBackY;
				wallHitInfo.boundMax.y += pushBackY;
				wallHitInfo.center.y += pushBackY;
			}
		}
		else if((abs(moveVec.y) < 1.0f && !wallRect.intersectsRect(crntRect) && wallRect2.intersectsRect(crntRect))) {

			//オブジェクトの上にオブジェクトが乗っているか
			auto object = wallHitInfo.object;
			bool bObjectMoveLift = (object != nullptr) ? object->_objectMoveLift : false;
			// 上移動時
			if (moveVec.y > 0 && !bObjectMoveLift) {
				if (yc >= wallHitInfo.center.y) {
					pushBackY = rect.getMinY() - wallHitInfo.boundMax.y;
				}
				else {
					pushBackY = rect.getMaxY() - wallHitInfo.boundMin.y;
				}
				isHit = (pushBackY != 0.0f) ? true : false;
			}
			// 下移動時
			else if (moveVec.y < 0) {
				if (yc <= wallHitInfo.center.y) {
					pushBackY = rect.getMaxY() - wallHitInfo.boundMin.y;
				}
				else {
					pushBackY = rect.getMinY() - wallHitInfo.boundMax.y;
				}
				isHit = (pushBackY != 0.0f) ? true : false;
			}

			// Y方向の移動で接触が発生した場合
			if (isHit) {
				hitY = true;
				// 押し戻しを行い座標を再度設定する
				wallHitInfo.boundMin.y += pushBackY;
				wallHitInfo.boundMax.y += pushBackY;
				wallHitInfo.center.y += pushBackY;
			}
		}
	};

	crntRect = prevRect;
	crntRect.origin.x = wallHitInfo.boundMin.x;
	crntRect.origin.y = wallHitInfo.boundMin.y;
	for (unsigned int i = 0; i < infoGroup->getWallHitInfoListCount(); i++) {
		auto hitInfo = infoGroup->getWallHitInfo(i);
		auto wallRect = Rect(hitInfo.boundMin, Size(hitInfo.boundMax - hitInfo.boundMin));

		// 上下左右の押し戻し量
		float pushBackLeft = prevRect.getMinX() - wallRect.getMaxX();
		float pushBackRight = prevRect.getMaxX() - wallRect.getMinX();
		float pushBackUp = prevRect.getMaxY() - wallRect.getMinY();
		float pushBackDown = prevRect.getMinY() - wallRect.getMaxY();

		//左右で移動距離が短い方向。
		float valHorz = abs(pushBackLeft) <= abs(pushBackRight) ? pushBackLeft : pushBackRight;//x
		float valVert = abs(pushBackDown) <= abs(pushBackUp) ? pushBackDown : pushBackUp;//y
		if (abs(valHorz) <= abs(valVert)) {//X
			movePushBackX(hitInfo);
		}
		else if (abs(valHorz) > abs(valVert)) {//Y
			movePushBackY(hitInfo);
		}
	}
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
bool newCheckSlopeHit(WallHitInfo &wallHitInfo, Rect& oldRect, bool jumping, bool& hitUp, bool& hitDown, agtk::MtVector<agtk::Slope *> *slopeList, agtk::MtVector<agtk::Slope *> *passableSlopeTouchedList, Vec2& moveVec, bool isSlip, float touchFrame300)
#else
bool newCheckSlopeHit(WallHitInfo &wallHitInfo, Rect& oldRect, bool jumping, bool& hitUp, bool& hitDown, cocos2d::Array* slopeList, cocos2d::Array* passableSlopeTouchedList, Vec2& moveVec, bool isSlip, float touchFrame300)
#endif
{
	auto object = wallHitInfo.object;
	bool falling = false;
	if (object) {
		falling = object->_falling || object->_fallingOld;
	}
	// オブジェクトから近い順になるようにソートを行う
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	auto sortSlopeList = new agtk::MtVector<agtk::Slope *>();
	std::vector<agtk::Tile *> tileList;
#else
	auto sortSlopeList = cocos2d::Array::create();
#endif
	sortSlopeList->addObjectsFromArray(slopeList);
	
	if (sortSlopeList->count() > 1) {

		std::vector<float> distList;
		for (int i = 0; i < (int)sortSlopeList->count(); i++) {
			distList.push_back(99999999.f);
		}

		// 右移動時
		if (moveVec.x > 0) {

			for (int i = 0; i < (int)sortSlopeList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto slope = static_cast<agtk::Slope*>(sortSlopeList->getObjectAtIndex(i));
#else
				auto slope = dynamic_cast<agtk::Slope*>(sortSlopeList->getObjectAtIndex(i));
#endif

				// 坂の中にいる場合
				if (slope->start.x <= oldRect.getMinX() &&
					oldRect.getMaxX() <= slope->end.x) {

					distList[i] = -9999999;
				}
				// 坂の始点を跨いでいる途中の場合
				else if (oldRect.getMinX() <= slope->start.x &&
					slope->start.x <= oldRect.getMaxX()) {

					distList[i] = oldRect.getMaxX() - slope->start.x;
				}
				else {
				}
			}
		}
		// 左移動時
		else if (moveVec.x < 0) {

			for (int i = 0; i < (int)sortSlopeList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto slope = static_cast<agtk::Slope*>(sortSlopeList->getObjectAtIndex(i));
#else
				auto slope = dynamic_cast<agtk::Slope*>(sortSlopeList->getObjectAtIndex(i));
#endif

				// 坂の中にいる場合
				if (slope->start.x <= oldRect.getMinX() &&
					oldRect.getMaxX() <= slope->end.x) {

					distList[i] = -9999999;
				}
				// 坂の終点を跨いでいる途中の場合
				else if (slope->start.x <= oldRect.getMinX() &&
					slope->end.x <= oldRect.getMaxX()) {

					distList[i] = oldRect.getMinX() - slope->end.x;
				}
				else {
				}
			}
		}
		// 停止時
		else {
		}

		// オブジェクトに近い坂から処理を行えるようにバブルソートを行う
		for (int i = 0; i < (int)sortSlopeList->count() - 1; i++) {
			for (int j = sortSlopeList->count() - 1; j > i; j--) {
				auto dist = distList[j];
				auto dist2 = distList[j - 1];

				if (dist < dist2) {
					sortSlopeList->swap(j, j - 1);
					distList[j - 1] = dist;
					distList[j] = dist2;
				}
			}
		}
	}

	hitUp = false;
	hitDown = false;

	cocos2d::Ref* ref = nullptr;

	Rect crntRect = Rect(wallHitInfo.boundMin, Size(wallHitInfo.boundMax - wallHitInfo.boundMin));

	bool hitSlope = false;

	// 接触中の通過可能な坂リストから接触確認を行う坂リストに含まれていない坂を削除する
	for (int i = passableSlopeTouchedList->count() - 1; i >= 0; i--) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		auto slope = (*passableSlopeTouchedList)[i];
#else
		agtk::Slope* slope = dynamic_cast<agtk::Slope*>(passableSlopeTouchedList->getObjectAtIndex(i));
#endif

		if (!sortSlopeList->containsObject(slope)) {
			passableSlopeTouchedList->removeObject(slope);
		}
	}

//	CCLOG("passableSlopeTouchedList : %d", passableSlopeTouchedList->count());

	// ソートされた坂で処理を行う
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	for (int i = 0; i < (int)sortSlopeList->size(); i++) {
		auto slope = (*sortSlopeList)[i];
#else
	CCARRAY_FOREACH(sortSlopeList, ref) {
		agtk::Slope* slope = dynamic_cast<agtk::Slope*>(ref);
#endif

		bool checkHitUp = false;
		bool checkHitDown = false;
		bool touchPassableSlope = false;
		bool touched = passableSlopeTouchedList->containsObject(slope);

		// まだ坂と接触していない場合
		if (!hitSlope) {
			// 坂との接触処理を行う
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
			if (slope->checkHit(crntRect, oldRect, moveVec, touched, jumping, checkHitUp, checkHitDown, touchPassableSlope, isSlip, touchFrame300, object, tileList)) {
#else
			if (slope->checkHit(crntRect, oldRect, moveVec, touched, jumping, checkHitUp, checkHitDown, touchPassableSlope, isSlip, touchFrame300, object, nullptr)) {
#endif
				hitSlope = true;

				if (checkHitUp) {
					hitUp = true;
				}
				if (checkHitDown) {
					hitDown = true;
				}
			}
		}
		// すでに坂と接触済みの場合
		else {
			// 通過可能な坂に接触中か確認を行う
			slope->simpleCheckHit(crntRect, oldRect, moveVec, touched, touchPassableSlope);
		}

		// 通過可能な坂に接触中の場合
		if (touchPassableSlope) {
			// オブジェクトの保持しているリストに未登録の場合は登録する
			if (!touched) {
				passableSlopeTouchedList->addObject(slope);
			}
		}
		// 通過可能な坂に接触していない場合
		else {
			// オブジェクトの保持しているリストに登録している場合はリストから削除する
			if (touched) {
				passableSlopeTouchedList->removeObject(slope);
			}
		}
	}

	return hitSlope;
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
bool newCheckSlopeHit(agtk::Object* object, bool& hitUp, bool& hitDown, agtk::MtVector<agtk::Slope *> *slopeList, Vec2& moveVec, bool isSlip, std::vector<agtk::Tile *> &tileList)
#else
bool newCheckSlopeHit(agtk::Object* object, bool& hitUp, bool& hitDown, cocos2d::Array* slopeList, Vec2& moveVec, bool isSlip, cocos2d::Array *tileList)
#endif
{
	WallHitInfoGroup *wallHitInfoGroup = object->getObjectCollision()->getWallHitInfoGroup();
	WallHitInfoGroup *oldWallHitInfoGroup = object->getObjectCollision()->getOldWallHitInfoGroup();
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	auto passableSlopeTouchedList = object->getPassableSlopeTouchedList();
#else
	cocos2d::Array* passableSlopeTouchedList = object->getPassableSlopeTouchedList();
#endif
	bool falling = object->_falling || object->_fallingOld;

	// オブジェクトから近い順になるようにソートを行う
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	auto sortSlopeList = new agtk::MtVector<agtk::Slope *>();
	AutoDeleter<agtk::MtVector<agtk::Slope *>> deleter(sortSlopeList);
	sortSlopeList->addObjectsFromArray(slopeList);
#else
	auto sortSlopeList = cocos2d::Array::create();
	sortSlopeList->addObjectsFromArray(slopeList);
#endif
	Rect oldRect = oldWallHitInfoGroup->getRect();
// #AGTK-NX #AGTK-WIN
#ifdef USE_SAR_PROVISIONAL_4
	// oldWallHitInfoGroupが空の場合の対応
	if (oldWallHitInfoGroup->getWallHitInfoListCount() == 0) {
		oldRect = wallHitInfoGroup->getRect();
		oldRect.origin -= moveVec;
	}
#endif

	if (sortSlopeList->count() > 1) {

		std::vector<float> distList;
		for (int i = 0; i < (int)sortSlopeList->count(); i++) {
			distList.push_back(99999999.f);
		}

		// 右移動時
		if (moveVec.x > 0) {

			for (int i = 0; i < (int)sortSlopeList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto slope = static_cast<agtk::Slope*>(sortSlopeList->getObjectAtIndex(i));
#else
				auto slope = dynamic_cast<agtk::Slope*>(sortSlopeList->getObjectAtIndex(i));
#endif

				// 坂の中にいる場合、もしくは垂直の坂に触れている場合。
				if ((slope->start.x <= oldRect.getMinX() && oldRect.getMaxX() <= slope->end.x)
				|| (slope->start.x == slope->end.x && oldRect.getMinX() <= slope->start.x && slope->start.x < oldRect.getMaxX())) {

					distList[i] = -9999999 - slope->start.x;
				}
				// 坂の始点を跨いでいる途中の場合
				else if (oldRect.getMinX() <= slope->start.x &&
					slope->start.x <= oldRect.getMaxX()) {

					distList[i] = oldRect.getMaxX() - slope->start.x;
				}
				else {
				}
			}
		}
		// 左移動時
		else if (moveVec.x < 0) {

			for (int i = 0; i < (int)sortSlopeList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto slope = static_cast<agtk::Slope*>(sortSlopeList->getObjectAtIndex(i));
#else
				auto slope = dynamic_cast<agtk::Slope*>(sortSlopeList->getObjectAtIndex(i));
#endif

				// 坂の中にいる場合、もしくは垂直の坂に触れている場合。
				if ((slope->start.x <= oldRect.getMinX() && oldRect.getMaxX() <= slope->end.x)
				|| (slope->start.x == slope->end.x && oldRect.getMinX() <= slope->start.x && slope->start.x < oldRect.getMaxX())) {

					distList[i] = -9999999 + slope->end.x;
				}
				// 坂の終点を跨いでいる途中の場合
				else if (slope->start.x <= oldRect.getMinX() &&
					slope->end.x <= oldRect.getMaxX()) {

					distList[i] = oldRect.getMinX() - slope->end.x;
				}
				else {
				}
			}
		}
		// 停止時
		else {
		}

		// オブジェクトに近い坂から処理を行えるようにバブルソートを行う
		for (int i = 0; i < (int)sortSlopeList->count() - 1; i++) {
			for (int j = sortSlopeList->count() - 1; j > i; j--) {
				auto dist = distList[j];
				auto dist2 = distList[j - 1];

				if (dist < dist2) {
					sortSlopeList->swap(j, j - 1);
					distList[j - 1] = dist;
					distList[j] = dist2;
				}
			}
		}
	}

	hitUp = false;
	hitDown = false;

	cocos2d::Ref* ref = nullptr;

	bool hitSlope = false;

	// 接触中の通過可能な坂リストから接触確認を行う坂リストに含まれていない坂を削除する
	for (int i = passableSlopeTouchedList->count() - 1; i >= 0; i--) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		auto slope = (*passableSlopeTouchedList)[i];
#else
		agtk::Slope* slope = dynamic_cast<agtk::Slope*>(passableSlopeTouchedList->getObjectAtIndex(i));
#endif

		if (!sortSlopeList->containsObject(slope)) {
			passableSlopeTouchedList->removeObject(slope);
		}
	}

	// ソートされた坂で処理を行う
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	for (int i = 0; i < (int)sortSlopeList->size(); i++) {
		auto slope = (*sortSlopeList)[i];
#else
	CCARRAY_FOREACH(sortSlopeList, ref) {
		agtk::Slope* slope = dynamic_cast<agtk::Slope*>(ref);
#endif

		bool checkHitUp = false;
		bool checkHitDown = false;
		bool touchPassableSlope = false;
		bool touched = passableSlopeTouchedList->containsObject(slope);

		// 接触確認を行う壁当たり判定を検索しIDを取得
		int id = slope->getCheckHitWallHitInfoId(wallHitInfoGroup, moveVec);
		Vec2 mv = Vec2::ZERO;
		int oldId = slope->getCheckHitWallHitInfoId(oldWallHitInfoGroup, mv);
//		CCLOG("ID : %d - %d", id, oldId);


		if (id >= 0) {
			auto info = wallHitInfoGroup->getWallHitInfo(id);
			Rect crntRect = Rect(info.boundMin, Size(info.boundMax - info.boundMin));
			crntRect.origin += moveVec;

			// 坂当たり判定用矩形の更新を行う
			object->getObjectCollision()->getSlopeCheckRect()->updateRect(crntRect, wallHitInfoGroup, moveVec);

			if (oldId >= 0) {
				auto oldInfo = oldWallHitInfoGroup->getWallHitInfo(oldId);

				oldRect = Rect(oldInfo.boundMin, Size(oldInfo.boundMax - oldInfo.boundMin));
			}
			else {

				oldRect = crntRect;
				oldRect.origin -= moveVec;
			}

			// まだ坂と接触していない場合
			if (!hitSlope) {
				// 坂との接触処理を行う
				if (slope->checkHit(crntRect, oldRect, moveVec, touched, object->getJumping(), checkHitUp, checkHitDown, touchPassableSlope, isSlip, object->getSlopeTouchedFrame(), object, tileList)) {
					hitSlope = true;

					if (checkHitUp) {
						hitUp = true;
					}
					if (checkHitDown) {
						hitDown = true;
					}
				}
			}
			// すでに坂と接触済みの場合
			else {
				// 通過可能な坂に接触中か確認を行う
				slope->simpleCheckHit(crntRect, oldRect, moveVec, touched, touchPassableSlope);
			}
		}

		// 通過可能な坂に接触中の場合
		if (touchPassableSlope) {
			// オブジェクトの保持しているリストに未登録の場合は登録する
			if (!touched) {
				passableSlopeTouchedList->addObject(slope);
			}
		}
		// 通過可能な坂に接触していない場合
		else {
			// オブジェクトの保持しているリストに登録している場合はリストから削除する
			if (touched) {
				passableSlopeTouchedList->removeObject(slope);
			}
		}
	}

	return hitSlope;
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void newCheckSlopeHit(WallHitInfo &wallHitInfo, agtk::MtVector<agtk::Slope *> *slopeList, agtk::MtVector<agtk::Slope *> *touchedSlopeList)
#else
void newCheckSlopeHit(WallHitInfo &wallHitInfo, cocos2d::Array* slopeList, cocos2d::Array* touchedSlopeList)
#endif
{
	cocos2d::Ref* ref = nullptr;
	Rect rect = Rect(wallHitInfo.boundMin, Size(wallHitInfo.boundMax - wallHitInfo.boundMin));
	rect.origin.x -= TILE_COLLISION_THRESHOLD;
	rect.origin.y -= TILE_COLLISION_THRESHOLD;
	rect.size.width += TILE_COLLISION_THRESHOLD * 2;
	rect.size.height += TILE_COLLISION_THRESHOLD * 2;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	for (int i = 0; i < (int)slopeList->size(); i++) {
		auto slope = (*slopeList)[i];
#else
	CCARRAY_FOREACH(slopeList, ref) {
		auto slope = dynamic_cast<agtk::Slope*>(ref);
#endif

		// すでにリストに追加されている坂の場合は処理しない
		if (touchedSlopeList->containsObject(slope)) {
			continue;
		}

		// 接触チェック
		if (slope->checkHitRect(rect)) {
			touchedSlopeList->addObject(slope);
		}
	}
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
bool checkGroundAndWallHit(Rect &objRect, Vec2& dist, std::vector<agtk::Tile *> &tileList, Rect& oldRect, agtk::Object* object, bool checkWall, bool jumping)
#else
bool checkGroundAndWallHit(Rect &objRect, Vec2& dist, cocos2d::Array* tileList, Rect& oldRect, agtk::Object* object, bool checkWall, bool jumping)
#endif
{
	auto projectData = GameManager::getInstance()->getProjectData();
	dist = Vec2::ZERO;
	bool result = false;
	bool isHit = false;
	cocos2d::Ref* ref = nullptr;
	Rect wallRect, checkDownRect, checkLeftRect, checkRightRect;

	checkDownRect = objRect;
	checkDownRect.origin.y -= (TILE_COLLISION_THRESHOLD + 1.0f);

	checkLeftRect = objRect;
	checkLeftRect.origin.x -= (TILE_COLLISION_THRESHOLD + 1.0f);

	checkRightRect = objRect;
	checkRightRect.origin.x += (TILE_COLLISION_THRESHOLD + 1.0f);

	// タイルとの接触チェック
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	for (auto tile : tileList) {
#else
	CCARRAY_FOREACH(tileList, ref) {
		auto tile = dynamic_cast<agtk::Tile*>(ref);
#endif
		if (tile->getType() == agtk::Tile::kTypeLimitTile && !dynamic_cast<LimitTile*>(tile)->isNeedCheck(object)) {
			continue;
		}
		int wallBit = tile->getWallBit();

		Rect rect = tile->convertToLayerSpaceRect();
		Vec2 pos = rect.origin;
		Size size = rect.size;
		pos.x += TILE_COLLISION_THRESHOLD;
		pos.y += TILE_COLLISION_THRESHOLD;
		size.width -= TILE_COLLISION_THRESHOLD * 2;
		size.height -= TILE_COLLISION_THRESHOLD * 2;

		wallRect = Rect(pos, size);

		float y1 = wallRect.getMaxY();

		// 地面接触チェック & トップビュー以外
		isHit = false;
		if (wallRect.intersectsRect(checkDownRect) && (projectData->getGameType() != agtk::data::ProjectData::kGameTypeTopView) && !jumping) {
			if (checkDownRect.getMinY() <= y1) {
				// 壁判定が４方向すべて設定されている場合
				if (wallBit == 0x0f) {
					isHit = true;
				}
				// 上方向に設定されている場合
				else if ((wallBit & (1 << agtk::data::TilesetData::Up)) &&
					y1 <= (oldRect.getMinY() + 1.0f)) {
					isHit = true;
				}

				if (isHit) {
					//float pushBackY = (y1 - checkDownRect.getMinY()) + TILE_COLLISION_THRESHOLD;
					float pushBackY = (rect.getMaxY() - checkDownRect.getMinY());
					pushBackY = objRect.getMinY() - (checkDownRect.getMinY() + pushBackY);

					// 予期せぬ接触により座標が吹き飛ぶ可能性もあるので、座標変更に制限を設ける
					if (-TILE_COLLISION_THRESHOLD < pushBackY && pushBackY < TILE_COLLISION_THRESHOLD) {
						if (fabsf(dist.y) < fabsf(pushBackY)) {
							dist.y = pushBackY;
						}
						result = true;
					}
				}
			}
		}

		// 地面だけチェックを行いたい場合はこの先の処理は行わない
		if (!checkWall) { continue; }

		float x0 = wallRect.getMinX();
		float x1 = wallRect.getMaxX();

		// 左壁接触チェック
		isHit = false;
		if (wallRect.intersectsRect(checkLeftRect)) {
			if (checkLeftRect.getMinX() <= x1) {
				// 壁判定が４方向すべて設定されている場合
				if (wallBit == 0x0f) {
					isHit = true;
				}
				// 右方向に設定されている場合
				else if (wallBit & (1 << agtk::data::TilesetData::Right)) {
					if (x1 <= (oldRect.getMinX() + 1.0f)) {
						isHit = true;
					}
				}

				if (isHit) {
					//float pushBackX = (x1 - checkLeftRect.getMinX()) + TILE_COLLISION_THRESHOLD;
					float pushBackX = (rect.getMaxX() - checkLeftRect.getMinX());
					pushBackX = objRect.getMinX() - (checkLeftRect.getMinX() + pushBackX);

					if (-TILE_COLLISION_THRESHOLD < pushBackX && pushBackX < 0) {
						if (fabsf(dist.x) < fabsf(pushBackX)) {
							dist.x = pushBackX;
						}
						result = true;
					}
				}

			}
		}

		// 右壁接触チェック
		isHit = false;
		if (wallRect.intersectsRect(checkRightRect)) {
			if (x0 <= checkRightRect.getMaxX()) {
				// 壁判定が４方向すべて設定されている場合
				if (wallBit == 0x0f) {
					isHit = true;
				}
				// 左方向に設定されている場合
				else if (wallBit & (1 << agtk::data::TilesetData::Left)) {
					if ((oldRect.getMaxX() - 1.0f) <= x0) {
						isHit = true;
					}
				}

				if (isHit) {
					//float pushBackX = (x0 - checkRightRect.getMaxX()) - TILE_COLLISION_THRESHOLD;
					float pushBackX = (rect.getMinX() - checkLeftRect.getMaxX());
					pushBackX = objRect.getMaxX() - (checkRightRect.getMaxX() + pushBackX);

					if (0 < pushBackX && pushBackX < TILE_COLLISION_THRESHOLD) {
						if (fabsf(dist.x) < fabsf(pushBackX)) {
							dist.x = pushBackX;
						}

						result = true;
					}
				}
			}
		}
	}

	return result;
}

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
bool checkGroundSlope(Rect &objRect, float& dist, agtk::MtVector<agtk::Slope *> *slopeList)
#else
bool checkGroundSlope(Rect &objRect, float& dist, cocos2d::Array* slopeList)
#endif
{
	bool result = false;

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	for (int i = 0; i < (int)slopeList->size(); i++) {
		auto slope = (*slopeList)[i];
#else
	cocos2d::Ref* ref = nullptr;

	CCARRAY_FOREACH(slopeList, ref) {
		auto slope = dynamic_cast<agtk::Slope*>(ref);
#endif

		// 上から通過できる坂は処理しない
		if (slope->getSlopeData()->getPassableFromUpper()) {
			continue;
		}

		float d = 0;
		if (slope->calcDistToUpper(d, objRect)) {
			dist = d;

			result = true;
		}
	}

	return result;
}


cocos2d::Vec2 getPushObjectVec(cocos2d::Vec2 pushObjMoveVec, cocos2d::Rect& pushObjRect, agtk::WallHitInfoGroup* pushedObjInfoGroup, bool& bBuriedInWallFlag, bool bPushedMove)
{
	Vec2 pushObjOldPos = pushObjRect.origin - pushObjMoveVec;
	Rect pushObjPrevRect = Rect(pushObjOldPos, pushObjRect.size);
	Rect pushObjCrntRect = pushObjPrevRect;
	Rect pushedObjRect;

	float moveX = 0;
	float moveY = 0;

	//押された側が移動している場合。
	cocos2d::Vec2 pushedMove = cocos2d::Vec2::ZERO;
	if (bPushedMove) {
		auto pushedObject = pushedObjInfoGroup->getObject();
		if (pushedObject != nullptr) {
			pushedMove = pushedObject->getPosition() - pushedObject->getOldPosition();
			pushedMove.y = -1.0f * pushedMove.y;
		}
		//床にいる場合はY軸が下方向の場合は０とする。
		if (pushedObject->_floor && pushedMove.y < 0.0f) {
			pushedMove.y = 0.0f;
		}
		//床にいる場合、Y軸が上方向の場合は、オブジェクト移動量を確認して０以下の場合は０とする。
		if (pushedObject->_floor && pushedMove.y > 0) {
			auto movement = pushedObject->getObjectMovement();
			if (movement->getMoveY() <= 0) {
				pushedMove.y = 0.0f;
			}
		}
	}

	bBuriedInWallFlag = false;

	// 移動前からオブジェクト同士が接触するかをチェック
	for (unsigned int i = 0; i < pushedObjInfoGroup->getWallHitInfoListCount(); i++) {
		auto wallInfo = pushedObjInfoGroup->getWallHitInfo(i);

		pushedObjRect = Rect(wallInfo.boundMin - pushedMove, Size(wallInfo.boundMax - wallInfo.boundMin));

		// 押されるオブジェクトの矩形のサイズを閾値に合わせて補正
		pushedObjRect.origin.x += TILE_COLLISION_THRESHOLD;
		pushedObjRect.origin.y += TILE_COLLISION_THRESHOLD;
		pushedObjRect.size.width -= TILE_COLLISION_THRESHOLD * 2;
		pushedObjRect.size.height -= TILE_COLLISION_THRESHOLD * 2;

		pushedObjRect.origin.x += moveX;
		pushedObjRect.origin.y += moveY;

		if (pushedObjRect.intersectsRect(pushObjCrntRect)) {
			bool right = false;
			bool left = false;
			bool down = false;
			bool up = false;

			//完全に埋まっている。
			std::function<bool(cocos2d::Rect, cocos2d::Rect)> checkBuriedInWall = [](cocos2d::Rect rect1, cocos2d::Rect rect2) {
				return (rect1.containsPoint(cocos2d::Vec2(rect2.getMinX(), rect2.getMinY()))
					&& rect1.containsPoint(cocos2d::Vec2(rect2.getMinX(), rect2.getMaxY()))
					&& rect1.containsPoint(cocos2d::Vec2(rect2.getMaxX(), rect2.getMinY()))
					&& rect1.containsPoint(cocos2d::Vec2(rect2.getMaxX(), rect2.getMaxY())));
			};
			//互いに中心点が埋まっている。
			std::function<bool(cocos2d::Rect, cocos2d::Rect)> checkBuriedInEachMiddlePoint = [](cocos2d::Rect rect1, cocos2d::Rect rect2) {
				return (rect1.containsPoint(cocos2d::Vec2(rect2.getMidX(), rect2.getMidY()))
					&& rect2.containsPoint(cocos2d::Vec2(rect1.getMidX(), rect1.getMidY())));
			};
			//rect1にrect2の中心点が埋まっている。
			std::function<bool(cocos2d::Rect, cocos2d::Rect)> checkBuriedInMiddlePoint = [](cocos2d::Rect rect1, cocos2d::Rect rect2) {
				return (rect1.containsPoint(cocos2d::Vec2(rect2.getMidX(), rect2.getMidY())));
			};

			//完全に埋まっているか、互いに中心点が埋まっている。
			if (checkBuriedInWall(pushObjCrntRect, pushedObjRect) || checkBuriedInWall(pushedObjRect, pushObjCrntRect)//完全に埋まっている
			|| checkBuriedInEachMiddlePoint(pushObjCrntRect, pushedObjRect)) {//互い中心点が埋まっている
				//右側
				if (pushedObjRect.getMaxX() < pushObjCrntRect.getMidX()) {
					right = true;
				}
				//左側
				else if (pushedObjRect.getMinX() >= pushObjCrntRect.getMidX()) {
					left = true;
				}
				//左右両側
				else {
					float w1 = pushObjCrntRect.getMidX() - pushedObjRect.getMinX();
					float w2 = pushedObjRect.getMaxX() - pushObjCrntRect.getMidX();
					CC_ASSERT(w1 >= 0 && w2 >= 0);
					if (w1 < w2) {
						left = true;
					}
					else if (w1 > w2) {
						right = true;
					}
				}
				//上側
				if (pushedObjRect.getMaxY() < pushObjCrntRect.getMidY()) {
					up = true;
				}
				//下側
				else if (pushedObjRect.getMinY() >= pushObjCrntRect.getMidY()) {
					down = true;
				}
				//上下両側
				else {
					float h1 = pushObjCrntRect.getMidY() - pushedObjRect.getMinY();
					float h2 = pushedObjRect.getMaxY() - pushObjCrntRect.getMidY();
					CC_ASSERT(h1 >= 0 && h2 >= 0);
					if (h1 < h2) {
						down = true;
					}
					else if (h1 > h2) {
						up = true;
					}
				}
				//埋まった。
				bBuriedInWallFlag = true;
			}
			//中心が埋まっている。
			else if (checkBuriedInMiddlePoint(pushObjCrntRect, pushedObjRect) || checkBuriedInMiddlePoint(pushedObjRect, pushObjCrntRect)) {
				//上側
				if (pushObjCrntRect.containsPoint(cocos2d::Vec2(pushedObjRect.getMinX(), pushedObjRect.getMaxY()))
				&& pushObjCrntRect.containsPoint(cocos2d::Vec2(pushedObjRect.getMaxX(), pushedObjRect.getMaxY()))) {
					up = true;
					if (pushObjCrntRect.getMidX() < pushedObjRect.getMidX()) {
						left = true;
					}
					else if (pushObjCrntRect.getMidX() > pushedObjRect.getMidX()) {
						right = true;
					}
				}
				//下側
				else if(pushObjCrntRect.containsPoint(cocos2d::Vec2(pushedObjRect.getMinX(), pushedObjRect.getMinY()))
				&& pushObjCrntRect.containsPoint(cocos2d::Vec2(pushedObjRect.getMaxX(), pushedObjRect.getMinY()))) {
					down = true;
					if (pushObjCrntRect.getMidX() < pushedObjRect.getMidX()) {
						left = true;
					}
					else if (pushObjCrntRect.getMidX() > pushedObjRect.getMidX()) {
						right = true;
					}
				}
				//左側
				else if (pushObjCrntRect.containsPoint(cocos2d::Vec2(pushedObjRect.getMinX(), pushedObjRect.getMinY()))
				&& pushObjCrntRect.containsPoint(cocos2d::Vec2(pushedObjRect.getMinX(), pushedObjRect.getMaxY()))) {
					left = true;
					if (pushObjCrntRect.getMidY() < pushedObjRect.getMidY()) {
						down = true;
					}
					else if (pushObjCrntRect.getMidY() > pushedObjRect.getMidY()) {
						up = true;
					}
				}
				//右側
				else if (pushObjCrntRect.containsPoint(cocos2d::Vec2(pushedObjRect.getMaxX(), pushedObjRect.getMinY()))
				&& pushObjCrntRect.containsPoint(cocos2d::Vec2(pushedObjRect.getMaxX(), pushedObjRect.getMaxY()))) {
					right = true;
					if (pushObjCrntRect.getMidY() < pushedObjRect.getMidY()) {
						down = true;
					}
					else if (pushObjCrntRect.getMidY() > pushedObjRect.getMidY()) {
						up = true;
					}
				}
				//左上
				else if (pushObjCrntRect.containsPoint(cocos2d::Vec2(pushedObjRect.getMinX(), pushedObjRect.getMaxY()))) {
					left = true;
					up = true;
				}
				//右上
				else if (pushObjCrntRect.containsPoint(cocos2d::Vec2(pushedObjRect.getMaxX(), pushedObjRect.getMaxY()))) {
					right = true;
					up = true;
				}
				//左下
				else if (pushObjCrntRect.containsPoint(cocos2d::Vec2(pushedObjRect.getMinX(), pushedObjRect.getMinY()))) {
					left = true;
					down = true;
				}
				//右下
				else if (pushObjCrntRect.containsPoint(cocos2d::Vec2(pushedObjRect.getMaxX(), pushedObjRect.getMinY()))) {
					right = true;
					down = true;
				}
				//埋まった。
				bBuriedInWallFlag = true;
			}
			//その他
			else {
				right = pushObjCrntRect.getMidX() > pushedObjRect.getMidX();
				left = pushObjCrntRect.getMidX() < pushedObjRect.getMidX();
				down = pushObjCrntRect.getMidY() < pushedObjRect.getMidY();
				up = pushObjCrntRect.getMidY() > pushedObjRect.getMidY();
			}

			// 同じ位置にいる場合
			if (!right && !left && !down && !up) {
				continue;
			}

			// 上下左右の押し戻し量
			float pushBackLeft = pushObjCrntRect.getMinX() - pushedObjRect.getMaxX() - 0.1f;
			float pushBackRight = pushObjCrntRect.getMaxX() - pushedObjRect.getMinX() + 0.1f;
			float pushBackUp = pushObjCrntRect.getMaxY() - pushedObjRect.getMinY() + 0.1f;
			float pushBackDown = pushObjCrntRect.getMinY() - pushedObjRect.getMaxY() - 0.1f;

			if (!right && !left) {
				if (up) {
					// 下へ押し出す
					moveY += pushBackDown;
				}
				else if (down) {
					// 上へ押し出す
					moveY += pushBackUp;
				}
				continue;
			}

			if (!down && !up) {
				if (left) {
					// 右への押し出す
					moveX += pushBackRight;
				}
				else if (right) {
					// 左へ押し出す
					moveX += pushBackLeft;
				}
				continue;
			}

			// 移動が発生している場合
			/*if (pushObjMoveVec.x != 0 && pushObjMoveVec.y != 0) */{
			
				float valVert = 0.0f;
				float valHorz = 0.0f;

				if (left) {
					valHorz = abs(pushBackRight);
				}
				else if (right) {
					valHorz = abs(pushBackLeft);
				}

				if (up) {
					valVert = abs(pushBackDown);
				}
				else if(down) {
					valVert = abs(pushBackUp);
				}

#if 1//sakihama-h, 2019.02.27 ACT2-4066
				if (valVert < valHorz) {
					pushObjMoveVec.add(cocos2d::Vec2(0, valVert));
				}
				else if (valVert > valHorz) {
					pushObjMoveVec.add(cocos2d::Vec2(valHorz, 0));
				}
				else {
					cocos2d::Vec2 mv = cocos2d::Vec2(
						abs(pushObjCrntRect.getMidX() - pushedObjRect.getMidX()),
						abs(pushObjCrntRect.getMidY() - pushedObjRect.getMidY())
					);
					//中心同士の距離が遠い方向が優先。
					if (mv.x < mv.y) {
						pushObjMoveVec.add(cocos2d::Vec2(0, valVert));
					}
					else if(mv.x > mv.y) {
						pushObjMoveVec.add(cocos2d::Vec2(valHorz, 0));
					}
					else {
						pushObjMoveVec.add(cocos2d::Vec2(valHorz, valVert));
					}
				}
				//移動量がサイズ半分より大きい場合は、埋まっているとする。
				if (pushObjRect.size.width * 0.5f < abs(pushObjMoveVec.x) || pushObjRect.size.height * 0.5f < abs(pushObjMoveVec.y)) {
					//埋まった。
					bBuriedInWallFlag = true;
				}
#elif 0//sakihama-h, 2018.09.03 ACT2-2294
				if (valVert < valHorz) {
					pushObjMoveVec.add(cocos2d::Vec2(0, up ? pushBackDown : pushBackUp));
				}
				else {
					pushObjMoveVec.add(cocos2d::Vec2(left ? pushBackRight : pushBackLeft, 0));
				}
#else
				float val = 0;
				if (valVert < valHorz) {
					val = valVert;
				}
				else {
					val = valHorz;
				}

				pushObjMoveVec.add(pushObjMoveVec * val);
#endif

				pushObjOldPos = pushObjRect.origin - pushObjMoveVec;
				pushObjPrevRect = Rect(pushObjOldPos, pushObjRect.size);
				pushObjCrntRect = pushObjPrevRect;


				break;
			}
			/*
			// 移動が発生していない場合
			else {
				Vec2 vec1;

				if (left) {
					vec1.x = pushedObjRect.getMinX() - pushedObjRect.getMidX();
				}
				else if (right) {
					vec1.x = pushedObjRect.getMaxX() - pushedObjRect.getMidX();
				}
				if (down) {
					vec1.y = pushedObjRect.getMinY() - pushedObjRect.getMidY();
				}
				else if (up) {
					vec1.y = pushedObjRect.getMaxY() - pushedObjRect.getMidY();
				}

				// 押し出すオブジェクトから押されるオブジェクトへのベクトル
				Vec2 vec2 = Vec2(pushObjCrntRect.getMidX(), pushObjCrntRect.getMidY()) - Vec2(pushedObjRect.getMidX(), pushedObjRect.getMidY());

				vec1.normalize();
				vec2.normalize();

				float cross = vec1.cross(vec2);

				if (left && down) {
					if (cross > 0) {
						// 上へ押し出す
						moveY += pushBackUp;
					}
					else if (cross < 0) {
						// 右への押し出す
						moveX += pushBackRight;
					}
				}
				else if (left && up) {
					if (cross > 0) {
						// 右への押し出す
						moveX += pushBackRight;
					}
					else if (cross < 0) {
						// 下へ押し出す
						moveY += pushBackDown;
					}
				}
				else if (right && down) {
					if (cross > 0) {
						// 左へ押し出す
						moveX += pushBackLeft;
					}
					else if (cross < 0) {
						// 上へ押し出す
						moveY += pushBackUp;
					}
				}
				else if (right && up) {
					if (cross > 0) {
						// 下へ押し出す
						moveY += pushBackDown;
					}
					else if (cross < 0) {
						// 左へ押し出す
						moveX += pushBackLeft;
					}
				}
			}
			*/
		}
	}
	
#if 1//ACT2-4072で、オブジェクト同士が重なると押し出す移動量が大きい方になることがあるので、移動量が少ない方を優先するように修正。
	if (pushObjMoveVec.x != 0 && pushObjMoveVec.y != 0) {
		for (unsigned int i = 0; i < pushedObjInfoGroup->getWallHitInfoListCount(); i++) {
			auto wallInfo = pushedObjInfoGroup->getWallHitInfo(i);

			// X,Y方向でオブジェクト同士が接触するかをチェック
			pushObjCrntRect.origin.x = pushObjRect.origin.x;
			pushObjCrntRect.origin.y = pushObjRect.origin.y;

			pushedObjRect = Rect(wallInfo.boundMin - pushedMove, Size(wallInfo.boundMax - wallInfo.boundMin));
			pushedObjRect.origin.x += TILE_COLLISION_THRESHOLD;
			pushedObjRect.origin.y += TILE_COLLISION_THRESHOLD;
			pushedObjRect.size.width -= TILE_COLLISION_THRESHOLD * 2;
			pushedObjRect.size.height -= TILE_COLLISION_THRESHOLD * 2;

			pushedObjRect.origin.x += moveX;
			pushedObjRect.origin.y += moveY;

			float _moveX = 0.0f;
			if (pushedObjRect.intersectsRect(pushObjCrntRect)) {
				// 押し出すオブジェクトが押してくるオブジェクトの左にいる場合
				if (pushObjRect.getMidX() > pushedObjRect.getMidX()) {
					// 左への押し出し量を算出
					_moveX = pushObjRect.getMinX() - pushedObjRect.getMaxX() - TILE_COLLISION_THRESHOLD;
				}
				// 押し出すオブジェクトが押してくるオブジェクトの右にいる場合
				else if (pushObjRect.getMidX() < pushedObjRect.getMidX()) {
					// 右への押し出し量を算出
					_moveX = pushObjRect.getMaxX() - pushedObjRect.getMinX() + TILE_COLLISION_THRESHOLD;
				}
			}

			float _moveY = 0.0f;
			if (pushedObjRect.intersectsRect(pushObjCrntRect)) {
				if (pushObjRect.getMidY() > pushedObjRect.getMidY()) {
					_moveY = pushObjRect.getMinY() - pushedObjRect.getMaxY() - TILE_COLLISION_THRESHOLD;
				}
				else if (pushObjRect.getMidY() < pushedObjRect.getMidY()) {
					_moveY = pushObjRect.getMaxY() - pushedObjRect.getMinY() + TILE_COLLISION_THRESHOLD;
				}
			}

			//移動量が小さい方を優先する。
			if (_moveX != 0.0f || _moveY != 0.0f) {
				if (abs(_moveX) < abs(_moveY)) {
					moveX += _moveX;
				}
				else if (abs(_moveX) > abs(_moveY)) {
					moveY += _moveY;
				}
				else {
					moveX += _moveX;
					moveY += _moveY;
				}
			}
		}
	}
	else {
		pushObjCrntRect.origin.x = pushObjRect.origin.x;
		pushObjCrntRect.origin.y = pushObjRect.origin.y;

		for (unsigned int i = 0; i < pushedObjInfoGroup->getWallHitInfoListCount(); i++) {
			auto wallInfo = pushedObjInfoGroup->getWallHitInfo(i);

			pushedObjRect = Rect(wallInfo.boundMin - pushedMove, Size(wallInfo.boundMax - wallInfo.boundMin));
			pushedObjRect.origin.x += TILE_COLLISION_THRESHOLD;
			pushedObjRect.origin.y += TILE_COLLISION_THRESHOLD;
			pushedObjRect.size.width -= TILE_COLLISION_THRESHOLD * 2;
			pushedObjRect.size.height -= TILE_COLLISION_THRESHOLD * 2;

			pushedObjRect.origin.x += moveX;
			pushedObjRect.origin.y += moveY;

			if (pushedObjRect.intersectsRect(pushObjCrntRect)) {
				// 上下左右の押し戻し量
				float pushBackLeft = pushObjCrntRect.getMinX() - pushedObjRect.getMaxX();
				float pushBackRight = pushObjCrntRect.getMaxX() - pushedObjRect.getMinX();
				float pushBackUp = pushObjCrntRect.getMaxY() - pushedObjRect.getMinY();
				float pushBackDown = pushObjCrntRect.getMinY() - pushedObjRect.getMaxY();

				//左右で移動距離が短い方向。
				float valHorz = abs(pushBackLeft) <= abs(pushBackRight) ? pushBackLeft - TILE_COLLISION_THRESHOLD : pushBackRight + TILE_COLLISION_THRESHOLD;//x
				float valVert = abs(pushBackDown) <= abs(pushBackUp) ? pushBackDown - TILE_COLLISION_THRESHOLD : pushBackUp + TILE_COLLISION_THRESHOLD;//y
				if (abs(valHorz) < abs(valVert)) {
					moveX += valHorz;
				}
				else if (abs(valHorz) > abs(valVert)) {
					moveY += valVert;
				}
				else {
					moveX += valHorz;
					moveY += valVert;
				}
			}
		}
	}
#else
	// X方向の移動だけでオブジェクト同士が接触するかをチェック
	pushObjCrntRect.origin.x = pushObjRect.origin.x;

	if (pushObjMoveVec.x != 0 && pushObjMoveVec.y != 0 || 
		pushObjMoveVec.x != 0 && pushObjMoveVec.y == 0 ||
		pushObjMoveVec.x == 0 && pushObjMoveVec.y == 0)
	{
		for (unsigned int i = 0; i < pushedObjInfoGroup->getWallHitInfoListCount(); i++) {
			auto wallInfo = pushedObjInfoGroup->getWallHitInfo(i);

			pushedObjRect = Rect(wallInfo.boundMin, Size(wallInfo.boundMax - wallInfo.boundMin));
			pushedObjRect.origin.x += TILE_COLLISION_THRESHOLD;
			pushedObjRect.origin.y += TILE_COLLISION_THRESHOLD;
			pushedObjRect.size.width -= TILE_COLLISION_THRESHOLD * 2;
			pushedObjRect.size.height -= TILE_COLLISION_THRESHOLD * 2;

			pushedObjRect.origin.x += moveX;
			pushedObjRect.origin.y += moveY;

			if (pushedObjRect.intersectsRect(pushObjCrntRect)) {
				// 押し出すオブジェクトが押してくるオブジェクトの左にいる場合
				if (pushObjRect.getMidX() > pushedObjRect.getMidX()) {
					// 左への押し出し量を算出
					moveX += pushObjRect.getMinX() - pushedObjRect.getMaxX() - TILE_COLLISION_THRESHOLD;
				}
				// 押し出すオブジェクトが押してくるオブジェクトの右にいる場合
				else if (pushObjRect.getMidX() < pushedObjRect.getMidX()) {
					// 右への押し出し量を算出
					moveX += pushObjRect.getMaxX() - pushedObjRect.getMinX() + TILE_COLLISION_THRESHOLD;
				}
			}
		}
	}


	// Y方向の移動だけでオブジェクト同士が接触するかをチェック
	pushObjCrntRect.origin.y = pushObjRect.origin.y;

	if (pushObjMoveVec.x != 0 && pushObjMoveVec.y != 0 ||
		pushObjMoveVec.x == 0 && pushObjMoveVec.y != 0 ||
		pushObjMoveVec.x == 0 && pushObjMoveVec.y == 0)
	{

		for (unsigned int i = 0; i < pushedObjInfoGroup->getWallHitInfoListCount(); i++) {
			auto wallInfo = pushedObjInfoGroup->getWallHitInfo(i);

			pushedObjRect = Rect(wallInfo.boundMin, Size(wallInfo.boundMax - wallInfo.boundMin));
			pushedObjRect.origin.x += TILE_COLLISION_THRESHOLD;
			pushedObjRect.origin.y += TILE_COLLISION_THRESHOLD;
			pushedObjRect.size.width -= TILE_COLLISION_THRESHOLD * 2;
			pushedObjRect.size.height -= TILE_COLLISION_THRESHOLD * 2;

			pushedObjRect.origin.x += moveX;
			pushedObjRect.origin.y += moveY;

			if (pushedObjRect.intersectsRect(pushObjCrntRect)) {

				if (pushObjRect.getMidY() > pushedObjRect.getMidY()) {
					moveY += pushObjRect.getMinY() - pushedObjRect.getMaxY() - TILE_COLLISION_THRESHOLD;
				}
				else if (pushObjRect.getMidY() < pushedObjRect.getMidY()) {
					moveY += pushObjRect.getMaxY() - pushedObjRect.getMinY() + TILE_COLLISION_THRESHOLD;
				}
			}
		}
	}
#endif

	/*
	if (moveX != 0 && moveY != 0) {
		CCLOG("おかしい可能性あり");
	}

	if(moveX != 0 || moveY != 0) {
		CCLOG("push : %f, %f", moveX, moveY);
	}
	*/
	return Vec2(moveX, moveY);
}

// #AGTK-NX #AGTK-WIN
#ifdef USE_SAR_FIX_AND_OPTIMIZE_1
int checkWallHitDivIndex(agtk::Object* object, Rect& crntRect, cocos2d::Vec2 move, int wallHitInfoId, int crntIndex, int div, std::vector<agtk::Tile *> &tileList, agtk::MtVector<agtk::Slope *> *slopeList, agtk::MtVector<agtk::Slope *> *passableSlopeList)
{
	// 移動しない場合はヒットしない
	if (move == Vec2::ZERO) {
		return div + 1;
	}

	// 移動方向により8方向に分類(テンキー方向)
	int checkRectType = 5;
	if (move.x >= 0.0f)
	{
		if (move.x == 0.0f)
		{
			if (move.y > 0.0f) {
				checkRectType = 8;
			}
			else {
				checkRectType = 2;
			}
		}
		else {
			if (move.y == 0.0f) {
				checkRectType = 6;
			} else if (move.y > 0.0f) {
				checkRectType = 9;
			}
			else {
				checkRectType = 3;
			}
		}
	}
	else {
		if (move.y == 0.0f) {
			checkRectType = 4;
		}
		else if (move.y > 0.0f) {
			checkRectType = 7;
		}
		else {
			checkRectType = 1;
		}
	}

	cocos2d::Point sp1[3];
	cocos2d::Point ep1[3];
	CollisionLine lines1[3] = { { &sp1[0], &ep1[0] },{ &sp1[1], &ep1[1] },{ &sp1[2], &ep1[2] } };
	cocos2d::Point originOfs1[3];
	int lines1Num = 0;

	// カレント矩形の移動方向に伸びるレイを設定
	std::function<void(cocos2d::Rect)> setupRectLay = [&](cocos2d::Rect rect1) {
		switch (checkRectType)
		{
			case 1:
				lines1Num = 3;
				sp1[0] = cocos2d::Point(rect1.getMinX(), rect1.getMaxY());
				ep1[0] = sp1[0] + move;
				sp1[1] = cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				ep1[1] = sp1[1] + move;
				sp1[2] = cocos2d::Point(rect1.getMaxX(), rect1.getMinY());
				ep1[2] = sp1[2] + move;
				originOfs1[0] = sp1[0] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[1] = sp1[1] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[2] = sp1[2] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				break;
			case 3:
				lines1Num = 3;
				sp1[0] = cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				ep1[0] = sp1[0] + move;
				sp1[1] = cocos2d::Point(rect1.getMaxX(), rect1.getMinY());
				ep1[1] = sp1[1] + move;
				sp1[2] = cocos2d::Point(rect1.getMaxX(), rect1.getMaxY());
				ep1[2] = sp1[2] + move;
				originOfs1[0] = sp1[0] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[1] = sp1[1] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[2] = sp1[2] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				break;
			case 7:
				lines1Num = 3;
				sp1[0] = cocos2d::Point(rect1.getMaxX(), rect1.getMaxY());
				ep1[0] = sp1[0] + move;
				sp1[1] = cocos2d::Point(rect1.getMinX(), rect1.getMaxY());
				ep1[1] = sp1[1] + move;
				sp1[2] = cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				ep1[2] = sp1[2] + move;
				originOfs1[0] = sp1[0] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[1] = sp1[1] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[2] = sp1[2] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				break;
			case 9:
				lines1Num = 3;
				sp1[0] = cocos2d::Point(rect1.getMaxX(), rect1.getMinY());
				ep1[0] = sp1[0] + move;
				sp1[1] = cocos2d::Point(rect1.getMaxX(), rect1.getMaxY());
				ep1[1] = sp1[1] + move;
				sp1[2] = cocos2d::Point(rect1.getMinX(), rect1.getMaxY());
				ep1[2] = sp1[2] + move;
				originOfs1[0] = sp1[0] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[1] = sp1[1] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[2] = sp1[2] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				break;
			case 2:
				lines1Num = 2;
				sp1[0] = cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				ep1[0] = sp1[0] + move;
				sp1[1] = cocos2d::Point(rect1.getMaxX(), rect1.getMinY());
				ep1[1] = sp1[1] + move;
				originOfs1[0] = sp1[0] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[1] = sp1[1] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				break;
			case 4:
				lines1Num = 2;
				sp1[0] = cocos2d::Point(rect1.getMinX(), rect1.getMaxY());
				ep1[0] = sp1[0] + move;
				sp1[1] = cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				ep1[1] = sp1[1] + move;
				originOfs1[0] = sp1[0] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[1] = sp1[1] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				break;
			case 6:
				lines1Num = 2;
				sp1[0] = cocos2d::Point(rect1.getMaxX(), rect1.getMinY());
				ep1[0] = sp1[0] + move;
				sp1[1] = cocos2d::Point(rect1.getMaxX(), rect1.getMaxY());
				ep1[1] = sp1[1] + move;
				originOfs1[0] = sp1[0] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[1] = sp1[1] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				break;
			case 8:
				lines1Num = 2;
				sp1[0] = cocos2d::Point(rect1.getMinX(), rect1.getMaxY());
				ep1[0] = sp1[0] + move;
				sp1[1] = cocos2d::Point(rect1.getMaxX(), rect1.getMaxY());
				ep1[1] = sp1[1] + move;
				originOfs1[0] = sp1[0] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				originOfs1[1] = sp1[1] - cocos2d::Point(rect1.getMinX(), rect1.getMinY());
				break;
		}
	};

	float minHitIndex = div + 1;

	// rect1がmove分移動するときのrect2との接触インデックスを取得 ヒットしなければfalseを返す
	std::function<void(cocos2d::Rect, cocos2d::Rect, cocos2d::Vec2, int)> checkHit = [&](cocos2d::Rect rect1, cocos2d::Rect rect2, cocos2d::Vec2 moveVec, int checkType) {

		cocos2d::Point sp2[2];
		cocos2d::Point ep2[2];
		CollisionLine lines2[2] = { { &sp2[0], &ep2[0] },{ &sp2[1], &ep2[1] } };
		int lines2Num = 0;

		switch (checkType)
		{
			case 1:
				lines2Num = 2;
				sp2[0] = cocos2d::Point(rect2.getMaxX(), rect2.getMinY() - rect1.size.height);
				ep2[0] = cocos2d::Point(rect2.getMaxX(), rect2.getMaxY() + rect1.size.height);
				sp2[1] = cocos2d::Point(rect2.getMaxX() + rect1.size.width, rect2.getMaxY());
				ep2[1] = cocos2d::Point(rect2.getMinX() - rect1.size.width, rect2.getMaxY());
				break;
			case 3:
				lines2Num = 2;
				sp2[0] = cocos2d::Point(rect2.getMaxX() + rect1.size.width, rect2.getMaxY());
				ep2[0] = cocos2d::Point(rect2.getMinX() - rect1.size.width, rect2.getMaxY());
				sp2[1] = cocos2d::Point(rect2.getMinX(), rect2.getMaxY() + rect1.size.height);
				ep2[1] = cocos2d::Point(rect2.getMinX(), rect2.getMinY() - rect1.size.height);
				break;
			case 7:
				lines2Num = 2;
				sp2[0] = cocos2d::Point(rect2.getMinX() - rect1.size.width, rect2.getMinY());
				ep2[0] = cocos2d::Point(rect2.getMaxX() + rect1.size.width, rect2.getMinY());
				sp2[1] = cocos2d::Point(rect2.getMaxX(), rect2.getMinY() - rect1.size.height);
				ep2[1] = cocos2d::Point(rect2.getMaxX(), rect2.getMaxY() + rect1.size.height);
				break;
			case 9:
				lines2Num = 2;
				sp2[0] = cocos2d::Point(rect2.getMinX(), rect2.getMaxY() + rect1.size.height);
				ep2[0] = cocos2d::Point(rect2.getMinX(), rect2.getMinY() - rect1.size.height);
				sp2[1] = cocos2d::Point(rect2.getMinX() - rect1.size.width, rect2.getMinY());
				ep2[1] = cocos2d::Point(rect2.getMaxX() + rect1.size.width, rect2.getMinY());
				break;
			case 2:
				lines2Num = 1;
				sp2[0] = cocos2d::Point(rect2.getMaxX() + rect1.size.width, rect2.getMaxY());
				ep2[0] = cocos2d::Point(rect2.getMinX() - rect1.size.width, rect2.getMaxY());
				break;
			case 4:
				lines2Num = 1;
				sp2[0] = cocos2d::Point(rect2.getMaxX(), rect2.getMinY() - rect1.size.height);
				ep2[0] = cocos2d::Point(rect2.getMaxX(), rect2.getMaxY() + rect1.size.height);
				break;
			case 6:
				lines2Num = 1;
				sp2[0] = cocos2d::Point(rect2.getMinX(), rect2.getMaxY() + rect1.size.height);
				ep2[0] = cocos2d::Point(rect2.getMinX(), rect2.getMinY() - rect1.size.height);
				break;
			case 8:
				lines2Num = 1;
				sp2[0] = cocos2d::Point(rect2.getMinX() - rect1.size.width, rect2.getMinY());
				ep2[0] = cocos2d::Point(rect2.getMaxX() + rect1.size.width, rect2.getMinY());
				break;
		}

		for (int i = 0; i < lines1Num; i++) {
			for (int j = 0; j < lines2Num; j++) {
				cocos2d::Point cross;
				float t;
				if (CollisionUtils::checkPushCross2(&lines2[j], &lines1[i], &cross)) {
					// 実際に当たるか判定
					cocos2d::Point origin = cross + originOfs1[i];
					if (lines2[j].start->y == lines2[j].end->y) {
						// horizontal line
						if (origin.x > rect2.getMaxX() || origin.x + rect1.size.width < rect2.getMinX()) {
							// ヒットしない
							continue;
						}
					}
					else {
						// vertical line
						if (origin.y > rect2.getMaxY() || origin.y + rect1.size.height < rect2.getMinY()) {
							// ヒットしない
							continue;
						}
					}

					// 衝突する分割インデックスを求める
					if (abs(moveVec.x) > abs(moveVec.y)) {
						t = (cross.x - sp1[i].x) / moveVec.x;
					}
					else {
						t = (cross.y - sp1[i].y) / moveVec.y;
					}
					float index = (t * (div - crntIndex)) + crntIndex;
					if (index < minHitIndex) {
						minHitIndex = index;
					}
					break;
				}
			}
		}
	};

	std::function<void(cocos2d::Rect, CollisionLine*, cocos2d::Vec2)> checkSlopeHit = [&](cocos2d::Rect rect1, CollisionLine* slopeLine, cocos2d::Vec2 moveVec) {

		for (int i = 0; i < lines1Num; i++) {
			cocos2d::Point cross;
			float t;
			if (CollisionUtils::checkPushCross2(slopeLine, &lines1[i], &cross)) {
				if (abs(moveVec.x) > abs(moveVec.y)) {
					t = (cross.x - sp1[i].x) / moveVec.x;
				}
				else {
					t = (cross.y - sp1[i].y) / moveVec.y;
				}
				float index = (t * (div - crntIndex)) + crntIndex;
				if (index < minHitIndex) {
					minHitIndex = index;
				}
			}
		}
	};

	// タイルとの接触から最初にヒットする分割インデックスを更新
	setupRectLay(crntRect);
	for (auto tile : tileList) {
		if (object && !(object->getObjectData()->getCollideWithTileGroupBit() & tile->getGroupBit())) {
			continue;
		}
		auto rect = tile->convertToLayerSpaceRect();
		auto pos = rect.origin;
		auto size = rect.size;
		pos.x += TILE_COLLISION_THRESHOLD;
		pos.y += TILE_COLLISION_THRESHOLD;
		size.width -= TILE_COLLISION_THRESHOLD * 2;
		size.height -= TILE_COLLISION_THRESHOLD * 2;

		cocos2d::Rect targetRect = cocos2d::Rect(pos, size);
		checkHit(crntRect, targetRect, move, checkRectType);
	}

	// スロープとの接触から最初にヒットする分割インデックスを更新
	if (slopeList) {
		// 坂確認用矩形を取得
		Rect slopeCheckRect = Rect::ZERO;
		if (object != nullptr) {
			// 坂確認用の矩形が設定されている場合
			auto list = object->getObjectCollision()->getSlopeCheckRect()->getInfoGroupDifferenceList();
			if (wallHitInfoId < static_cast<int>(list.size()) && list.size() > 0) {

				auto slopeRect = object->getObjectCollision()->getSlopeCheckRect()->getRect();

				// 坂確認用の矩形のサイズを設定
				slopeCheckRect.size = slopeRect.size;
				// 坂確認矩形の左下と壁当たりの左下との差を格納
				slopeCheckRect.origin = crntRect.origin - list[wallHitInfoId];
			}
		}

		setupRectLay(slopeCheckRect);
		for (int i = 0; i < slopeList->count(); i++) {
			auto slope = (*slopeList)[i];
			if (passableSlopeList->containsObject(slope)) {
				continue;
			}

			cocos2d::Point sp = slope->getWorldStartPoint();
			cocos2d::Point ep = slope->getWorldEndPoint();
			float dy = (ep.y - sp.y) / (ep.x - sp.x);
			if (ep.x >= sp.x) {
				sp.x -= crntRect.size.width;
				sp.y -= dy * crntRect.size.width;
				ep.x += crntRect.size.width;
				ep.y += dy * crntRect.size.width;
			}
			else {
				sp.x += crntRect.size.width;
				sp.y += dy * crntRect.size.width;
				ep.x -= crntRect.size.width;
				ep.y -= dy * crntRect.size.width;
			}

			CollisionLine slopeLine = { &sp, &ep };
			checkSlopeHit(slopeCheckRect, &slopeLine, move);
		}
	}

	// この時点でminHitIndexは minHitIndex > crntIndex && minHitIndex <= divとなっている
	// もしも誤差によりインデックス値がdiv以上にオーバーしているならばdiv以下に制限
	if(minHitIndex < (float)div + 0.5f && minHitIndex > div)
	{
		minHitIndex = div;
	}
	// 実際に判定をとるタイミングは衝突インデックス値を切り上げたインデックスとなる
	minHitIndex = std::ceilf(minHitIndex);

	if (minHitIndex < crntIndex + 1) {
		minHitIndex = crntIndex + 1;
	}
	return minHitIndex;
}
#endif

//-------------------------------------------------------------------------------------------------------------------
void showDebugLine(cocos2d::Vec2 &p1, cocos2d::Vec2 &p2, cocos2d::Color4F &color, float duration, int localZOrder)
{
	auto scene = GameManager::getInstance()->getCurrentScene();
	if (scene == nullptr) {
		return;
	}
	auto sceneLayer = scene->getSceneLayerFront();
	if (sceneLayer == nullptr) {
		return;
	}
	auto primitiveManager = PrimitiveManager::getInstance();
	auto poly = primitiveManager->createLine(p1.x, p1.y, p2.x, p2.y, color);
	primitiveManager->setTimer(poly, duration);
	sceneLayer->addChild(poly, localZOrder);
}

void showDebugRect(cocos2d::Rect &rect, cocos2d::Color4F &color, float duration, int localZOrder)
{
	auto scene = GameManager::getInstance()->getCurrentScene();
	if (scene == nullptr) {
		return;
	}
	auto sceneLayer = scene->getSceneLayerFront();
	if (sceneLayer == nullptr) {
		return;
	}
	auto primitiveManager = PrimitiveManager::getInstance();
	auto poly = primitiveManager->createRectangle(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height, color);
	primitiveManager->setTimer(poly, duration);
	sceneLayer->addChild(poly, localZOrder);
}

void showDebugPolygon(cocos2d::Vec2 *vertices, int length, cocos2d::Color4F &fillColor, cocos2d::Color4F &borderColor, float duration, int localZOrder)
{
	auto scene = GameManager::getInstance()->getCurrentScene();
	if (scene == nullptr) {
		return;
	}
	auto sceneLayer = scene->getSceneLayerFront();
	if (sceneLayer == nullptr) {
		return;
	}
	auto primitiveManager = PrimitiveManager::getInstance();
	auto poly = primitiveManager->createPolygon(0, 0, vertices, length, fillColor, borderColor);
	primitiveManager->setTimer(poly, duration);
	sceneLayer->addChild(poly, localZOrder);
}

void showDebugCircle(cocos2d::Vec2 pos, float radius, cocos2d::Color4F &color, float duration, int localZOrder)
{
	auto scene = GameManager::getInstance()->getCurrentScene();
	if (scene == nullptr) {
		return;
	}
	auto sceneLayer = scene->getSceneLayerFront();
	if (sceneLayer == nullptr) {
		return;
	}
	auto primitiveManager = PrimitiveManager::getInstance();
	auto poly = primitiveManager->createCircle(pos.x, pos.y, radius, color);
	primitiveManager->setTimer(poly, duration);
	sceneLayer->addChild(poly, localZOrder);
}

NS_AGTK_END
