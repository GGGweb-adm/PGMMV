#ifndef __JSB_AGTK_OBJECT_INSTANCES_H__
#define	__JSB_AGTK_OBJECT_INSTANCES_H__

#include "cocos2d.h"
#include "jsapi.h"
#include "js/RootingAPI.h"
#include "mozilla/Maybe.h"
#include "Lib/Macros.h"

extern void jsb_register_agtk_objectInstances(JSContext *_cx,JS::HandleObject object);

#endif	//__JSB_AGTK_OBJECT_INSTANCES_H__
