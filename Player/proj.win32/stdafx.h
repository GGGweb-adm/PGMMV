// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifndef __STDAFX_H__
#define __STDAFX_H__

// C RunTime Header Files
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <iostream>
#include <string>

// TODO: reference additional headers your program requires here

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
#include "ImGUI/IMGUIGLViewImpl.h"
#include "ImGUI/ImGuiLayer.h"
#include "ImGUI/imgui.h"
#endif

#include "vlc/vlc.h"
#ifdef OPENAL_PLAIN_INCLUDES
#include "alc.h"
#include "alext.h"
#else
#include "AL/alc.h"
#include "AL/alext.h"
#endif

#include "External/gif/gif_lib.h"//※ビルドできないため、Externl/Gif以下のcファイルを「プリコンパイルヘッダーを使用しない」、強制的にstdafx.hをインクルードしないようにしています。
#include "External/SSPlayer/SS6Player.h"
#include "External/SSPlayer/SS6PlayerData.h"
#include "External/collision/CollisionComponent.hpp"
#include "External/collision/CollisionUtils.hpp"

#endif //__STDAFX_H__
