#ifndef __SSPLAYER_CELLMAP__
#define __SSPLAYER_CELLMAP__

#include "../../SS6PlayerTypes.h"

namespace ss
{

/**
* SsCell
*/
struct SsCell
{
	float		pivot_X;		//原点補正
	float		pivot_Y;		//原点補正
	TextuerData texture;
	ss::SSRect	rect;
	std::string texname;
	int			cellIndex;
	std::string cellName;
	float		u1;
	float		v1;
	float		u2;
	float		v2;

	SsCell():
		  pivot_X(0)
		, pivot_Y(0)
		, cellIndex(-1)
		, u1(0)
		, v1(0)
		, u2(0)
		, v2(0)
	{}
};

///パーツが使用するセルの情報
struct SsCellValue
{
	SsRenderBlendType::_enum    blendType;
	SsCell						refCell;



	SsCellValue() :  
		blendType(SsRenderBlendType::_enum::Add)
		{}
};

};
#endif
