#if defined(AGTK_PLAYER)
#include <cocos2d.h>
#include <string.h>
#else
#include <QDebug>
#endif
#include "SplineInterp.h"

////////////////////////////////////////////////////////////
void SplineMakeTable(SplineInterp::PointF v[], SplineInterp::PointF z[], int num)
{
	int i;
	float t;
    SplineInterp::PointF *h, *d;

	h = new SplineInterp::PointF[num];
	d = new SplineInterp::PointF[num];

	z[0].x = 0; z[num - 1].x = 0;
	for(i = 0; i < num - 1; i++){
		h[i].x = v[i + 1].x - v[i].x;
		d[i + 1].x = (v[i + 1].y - v[i].y) / h[i].x;
	}
	z[1].x = d[2].x - d[1].x -h[0].x * z[0].x;
	d[1].x = 2 * (v[2].x - v[0].x);
	for(i = 1; i < num - 2; i++){
		t = h[i].x / d[i].x;
		z[i + 1].x = d[i + 2].x - d[i + 1].x - z[i].x * t;
		d[i + 1].x = 2 * (v[i + 2].x - v[i].x) - h[i].x * t;
	}
	z[num - 2].x -= h[num - 2].x * z[num - 1].x;
	for(i = num - 2; i > 0; i--){
		z[i].x = (z[i].x - h[i].x * z[i + 1].x) / d[i].x;
	}

	delete[] h;
	delete[] d;
}

float SplineInterpolate(float t, SplineInterp::PointF v[], SplineInterp::PointF z[], int num)
{
	int i, j, k;
	float d, h;
	float result;

	i = 0;  j = num - 1;
	while(i < j){
		k = (i + j) / 2;
		if(v[k].x < t){
			i = k + 1;
		} else {
			j = k;
		}
	}
	if(i > 0) i--;
	h = v[i + 1].x - v[i].x;
	d = t - v[i].x;
	result = (((z[i + 1].x - z[i].x) * d / h + z[i].x * 3) * d + ((v[i + 1].y - v[i].y) / h - (z[i].x * 2 + z[i + 1].x) * h)) * d + v[i].y;
	return result;
}

////////////////////////////////////////////////////////////
SplineInterp::SplineInterp()
: mPoints(0)
, mSize(0)
, mPointList(nullptr)
, mTable(nullptr)
, mTableUpdated(false)
{
}

SplineInterp::~SplineInterp()
{
    if(mPointList){
        delete[] mPointList;
        mPointList = nullptr;
    }
    if(mTable){
        delete[] mTable;
        mTable = nullptr;
    }
}

void SplineInterp::AddKey(int x, float v)
{
    if(mPointList == nullptr || mPoints + 1 >= mSize){
        auto newSize = (mSize == 0 ? 16 : mSize * 3 / 2);
        auto newList = new PointF[newSize];
        if(mPointList){
            memcpy(newList, mPointList, sizeof(*mPointList) * mPoints);
            delete[] mPointList;
        }
        mPointList = newList;
        mSize = newSize;
    }
	mPointList[mPoints].x = (float)x;
	mPointList[mPoints].y = v;
	mPoints++;
    mTableUpdated = false;
}

float SplineInterp::GetInterpolated(float x)
{
	float result;
	if(mPoints <= 0){
#if defined(AGTK_PLAYER)
		CCLOG("%s,%s,%d", __FILE__, __FUNCTION__, __LINE__);
#else
        qWarning() << __FILE__ << __FUNCTION__ << __LINE__;
#endif
        return 0;
    }
    if(mPoints == 1){
		return mPointList[0].y;
	}
    if(mPoints == 2){
		return mPointList[0].y + (mPointList[1].y - mPointList[0].y) * (x - mPointList[0].x) / (mPointList[1].x - mPointList[0].x);
	}
    if(!mTableUpdated){
        if(mTable){
            delete[] mTable;
            mTable = nullptr;
        }
        mTable = new PointF[mPoints];
        SplineMakeTable(mPointList, mTable, mPoints);
    }
	result = SplineInterpolate(x, mPointList, mTable, mPoints);
	return result;
}

