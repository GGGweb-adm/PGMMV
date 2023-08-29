#if !defined(_SPLINE_INTERP_H_)
#define _SPLINE_INTERP_H_

#ifdef __cplusplus
class SplineInterp
{
public:
	SplineInterp();
	~SplineInterp();
	void AddKey(int x, float v);
	float GetInterpolated(float x);
    class PointF
    {
    public:
        float x, y;
    };

protected:
	int mPoints;
	int mSize;
	PointF *mPointList;
    PointF *mTable;
    bool mTableUpdated;
};
#endif

#endif	//_SPLINE_INTERP_H_
