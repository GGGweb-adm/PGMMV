/**
 ForTreeCollision v0.0.1
 CollisionUtils.cpp

 Copyright (c) 2016 Kazuki Oda

 This software is released under the MIT License.
 http://opensource.org/licenses/mit-license.php
*/

#include "CollisionUtils.hpp"
USING_NS_CC;

bool CollisionUtils::containsPoint(cocos2d::Node *target,
                                   const cocos2d::Point &point,
                                   const cocos2d::Size &wide) {
    if (target == nullptr) {
        return false;
    }
    // ざっくりと判定。BoundingBoxでfalseの場合はfalseを返す
    if (target->getBoundingBox().containsPoint(point) == false) {
        return false;
    }
    // もう少し細かく判定する。

    Point anchor(target->getAnchorPoint());
    Point pos(target->getPosition());
    Size size(target->getContentSize());
    size.width *= target->getScaleX();
    size.height *= target->getScaleY();

    double x1 = pos.x - size.width * anchor.x;
    double y1 = pos.y + size.height * (1 - anchor.y);
    double cx1 = x1 + (size.width / 2);
    double cy1 = y1 - (size.height / 2);
    double l = sqrt(pow(point.x - cx1, 2) + pow(point.y - cy1, 2));
    double r = CC_DEGREES_TO_RADIANS(-target->getRotation());
    double r2 = atan((point.y - cy1) / (point.x - cx1)) - r;
    double tx2 = l * cos(r2) + cx1;
    double ty2 = l * sin(r2) + cy1;

#if 0//
	double digit = 1000;
	if(floor((x1 - wide.width) * digit) <= floor(tx2 * digit) && floor(tx2 * digit) <= floor((x1 + size.width + wide.width) * digit)
	&& floor((y1 + wide.height) * digit) >= floor(ty2 * digit) && floor(ty2 * digit) >= floor((y1 - size.height - wide.height) * digit)){
		return true;
	}
#else
    if (x1 - wide.width <= tx2 &&               //
        tx2 <= x1 + size.width + wide.width &&  //
        y1 + wide.height >= ty2 &&              //
        ty2 >= y1 - size.height - wide.height) {
        return true;
    }
#endif
    return false;
}

bool CollisionUtils::intersectRect(cocos2d::Node *target, cocos2d::Node *self) {
    if (!target->getBoundingBox().intersectsRect(self->getBoundingBox())) {
        return false;
    }

    Size tSize(target->getContentSize());
    Size sSize(self->getContentSize());
    double tl = sqrt(pow(tSize.width, 2) + pow(tSize.height, 2));
    double sl = sqrt(pow(sSize.width, 2) + pow(sSize.height, 2));
    auto s = tl >= sl ? self : target;
    auto b = tl >= sl ? target : self;
    auto sCorner = getCorner(s);
    auto bCorner = getCorner(b);

    // 線分に入っているかどうかを見る
    for (auto line : sCorner.lines()) {
        for (auto line2 : bCorner.lines()) {
            if (cross(&line, &line2)) {
                return true;
            }
        }
    }
    return containsPoint(b, s->getPosition(), Size::ZERO) ||
           containsPoint(s, b->getPosition(), Size::ZERO);
}

CollisionCorner CollisionUtils::getCorner(cocos2d::Node *node) {
    Size size(node->getContentSize());
    size.width *= node->getScaleX();
    size.height *= node->getScaleY();

    double _atan1 = atan2(size.height / 2, size.width / 2);
    double _atan2 = atan2(size.height / 2, -size.width / 2);
    double _atan3 = -_atan1;
    double _atan4 = -_atan2;
    double l = sqrt(pow(size.width / 2, 2) + pow(size.height / 2, 2));

    double radian = CC_DEGREES_TO_RADIANS(-node->getRotation());
    Point pos(node->getPosition());

	// anchorを考慮
	pos.x += (0.5f - node->getAnchorPoint().x) * size.width;
	pos.y += (0.5f - node->getAnchorPoint().y) * size.height;

    return {Point(pos.x + l * cosf(radian + _atan1),
                  pos.y + l * sinf(radian + _atan1)),
            Point(pos.x + l * cosf(radian + _atan2),
                  pos.y + l * sinf(radian + _atan2)),
            Point(pos.x + l * cosf(radian + _atan3),
                  pos.y + l * sinf(radian + _atan3)),
            Point(pos.x + l * cosf(radian + _atan4),
                  pos.y + l * sinf(radian + _atan4)),
            pos};
}

bool CollisionUtils::cross(CollisionLine *one, CollisionLine *two) {
    float ax = one->start->x;
    float ay = one->start->y;
    float bx = one->end->x;
    float by = one->end->y;

    float cx = two->start->x;
    float cy = two->start->y;
    float dx = two->end->x;
    float dy = two->end->y;

    float ta = (cx - dx) * (ay - cy) + (cy - dy) * (cx - ax);
    float tb = (cx - dx) * (by - cy) + (cy - dy) * (cx - bx);
    float tc = (ax - bx) * (cy - ay) + (ay - by) * (ax - cx);
    float td = (ax - bx) * (dy - ay) + (ay - by) * (ax - dx);

    return tc * td < 0 && ta * tb < 0;
}

/**
* 円と壁の線分の衝突判定
* @param	wallList			壁の線分リスト(右回り必須)
* @param	objLine				衝突物の線分
* @param	wallBit				壁ビット
* @param	size				衝突物のサイズ
* @param	repulsion			反発係数
* @param	crossPos			交点座標(出力)
* @param	reflection			反射ベクトル(出力)
* @param	checkInverseNormal	壁線分の法線チェックを行うか？
* @return						True:衝突 / false:衝突していない
*/
bool CollisionUtils::checkCircleHitSegmentAndGetIntersectionAndReflection(std::vector<CollisionLine>& wallList, CollisionLine* objLine, int wallBit, float size, float repulsion, const cocos2d::Vec2* speed, cocos2d::Vec2* crossPos, cocos2d::Vec2* reflection, bool checkInverseNormal/* = true */)
{
	bool isHit = false;

	auto radius = size * 0.5f;
	auto radiusPow = radius * radius;

	auto objSX = objLine->start->x;
	auto objSY = objLine->start->y;
	auto objEX = objLine->end->x;
	auto objEY = objLine->end->y;

	auto objLineNormalized = Vec2(objEX - objSX, objEY - objSY).getNormalized();

	for (auto wallLine : wallList) {

		// 壁判定を持たない線分の場合
		if (!(wallLine.wallBit & wallBit)) {
			// スキップ
			continue;
		}

		auto wallSX = wallLine.start->x;
		auto wallSY = wallLine.start->y;
		auto wallEX = wallLine.end->x;
		auto wallEY = wallLine.end->y;

		// 壁用線分のベクトル(pq)
		auto pqVec = Vec2(wallEX - wallSX, wallEY - wallSY);

		// -----------------------------------------------------
		// 先に壁線分と円の移動線分の交差判定を行い
		// 交差している場合は交点を円の終点とする
		// ※円の移動線分の始点を a 終点を b 
		// ※壁線分の始点を p 終点を q
		// -----------------------------------------------------

		// 線分が交差している場合
		if (cross(&wallLine, objLine)) {

			// 壁線分の始点(p)から円の移動線分の始点(a)へのベクトル
			Vec2 paVec = Vec2(objSX - wallSX, objSY - wallSY);

			// 円の移動線分のベクトル
			Vec2 abVec = Vec2(objEX - objSX, objEY - objSY);

			float pqabCross = pqVec.cross(abVec);

			// 線分 pq と線分 ab が平行でない場合
			if (pqabCross != 0.0f) {

				// paVec と abVec の外積を求める
				float paabCross = paVec.cross(abVec);

				// 壁の始点から交点までの長さの割合
				float t = paabCross / pqabCross;

				// 交点を終点に変更
				objEX = wallSX + t * pqVec.x;
				objEY = wallSY + t * pqVec.y;
			}
		}

		// -----------------------------------------------------
		// 壁線分の始点と終点をそれぞれ p, q とし、
		// 円の移動線分の終点 c を中心点とする円が壁線分 pq に衝突しているかは
		// 1. c からベクトル pqVec への垂線の長さ d を求め [d = (pcVec×pqVec) / |pqVec|]
		// 2. d が円の半径未満であり                       [d^2 <= radius^2]
		// 3. 長さが d の垂線が pq 上にある                [(pcVec・pqVec) * (qcVec・pqVec) < 0]
		// 4. 3が偽である場合は c から p への長さもしくは c から q への長さが円の半径以下であれば衝突
		// -----------------------------------------------------

		// 壁用線分の始点から円の移動線分の終点のベクトル(pc)
		auto pcVec = Vec2(objEX - wallSX, objEY - wallSY);

		// 壁用線分の終点から円の移動線分の終点のベクトル(qc)
		auto qcVec = Vec2(objEX - wallEX, objEY - wallEY);

		// pc と pq の外積を求める
		auto pcpqCross = pcVec.cross(pqVec);

		// 距離 d の二乗の値を求める
		auto d2 = pcpqCross * pcpqCross / pqVec.getLengthSq();

		// 衝突する可能性がある場合
		if (d2 <= radiusPow) {
			auto pcpqDot = Vec2::dot(pcVec, pqVec);
			auto qcpqDot = Vec2::dot(qcVec, pqVec);
			auto cpVec = Vec2(wallSX - objEX, wallSY - objEY);// 円の中心点(c)から壁線分の始点(p)へのベクトル
			auto cqVec = Vec2(wallEX - objEX, wallEY - objEY);// 円の中心点(c)から壁線分の始点(q)へのベクトル

			// c から pq への垂線が pq 上にある
			// または
			// cpVec の長さもしくは cqVec の長さが円の半径以下
			if (pcpqDot * qcpqDot <= 0 || cpVec.getLengthSq() <= radiusPow || cqVec.getLengthSq() <= radiusPow) {
				// --------------------------------------------------------------
				// 衝突処理
				// --------------------------------------------------------------
				// 壁線分 pq の法線を取得
				Vec2 pqNormal = Vec2(-pqVec.y, pqVec.x).getNormalized();

				// 壁線分 pq の法線ベクトルと円の移動方向が異なる場合
				if (!checkInverseNormal || speed->dot(pqNormal) < 0) {

					// 反射ベクトルを算出
					auto reflect = Vec2::ZERO;

					// 反発する場合
					if (repulsion > 0) {
						reflect = (*speed + (2.0f * Vec2::dot(-*speed, pqNormal) * pqNormal)) * repulsion;
					}
					// 壁ずりになる場合
					else {
						reflect = *speed + (Vec2::dot(-*speed, pqNormal) * pqNormal);
					}

					reflection->x = reflect.x;
					reflection->y = reflect.y;

					// 押し出し先を求める
					// 円の移動線分の終点から壁の法線方向へ
					auto diff = radius - sqrtf(d2);
					crossPos->x = objEX + pqNormal.x * diff;
					crossPos->y = objEY + pqNormal.y * diff;

					isHit = true;
					break;
				}
			}
		}
	}

	return isHit;
}

/**
* 線分交差判定
* @param	wallLine	壁の線分
* @param	objLine		衝突物の線分
* @return				wallLineとobjLineが交差しているか？
*/
bool CollisionUtils::checkPushCross2(CollisionLine* wallLine, CollisionLine* objLine, cocos2d::Vec2* crossPos)
{
	float wallSX = wallLine->start->x;
	float wallSY = wallLine->start->y;
	float wallEX = wallLine->end->x;
	float wallEY = wallLine->end->y;

	float objSX = objLine->start->x;
	float objSY = objLine->start->y;
	float objEX = objLine->end->x;
	float objEY = objLine->end->y;

	// 壁用線分のベクトル
	Vec2 wallVec = Vec2(wallEX - wallSX, wallEY - wallSY);

	// 衝突物線分のベクトル
	Vec2 objVec = Vec2(objEX - objSX, objEY - objSY);

	// 壁と衝突物の始点を結ぶベクトル
	Vec2 wallToObjVec = Vec2(objSX - wallSX, objSY - wallSY);

	// 壁と衝突物の外積
	float cross_w_o = wallVec.cross(objVec);

	// 並行な場合
	if (cross_w_o == 0.0f) {
		return false;
	}

	// 交点を求める
	float cross_wo_w = wallToObjVec.cross(wallVec);
	float cross_wo_o = wallToObjVec.cross(objVec);

	float wall_t = cross_wo_o / cross_w_o;
	float obj_t = cross_wo_w / cross_w_o;

	// 壁及び衝突物のベクトルの交点がそれぞれの線分の中に無い場合
	if (wall_t < 0 || wall_t > 1 || obj_t < 0 || obj_t > 1) {
		return false;
	}

	crossPos->x = wallSX + (wallVec.x * wall_t);
	crossPos->y = wallSY + (wallVec.y * wall_t);

	return true;
}