/**
 * @file Shader.cpp
 * @brief シェーダーを管理
*/
#include "Shader.h"

NS_AGTK_BEGIN //---------------------------------------------------------------------------------//

const GLchar *ccShaderDefault_vert =
"                                                   \n\
attribute vec4 a_position;                          \n\
attribute vec2 a_texCoord;                          \n\
attribute vec4 a_color;                             \n\
                                                    \n\
#ifdef GL_ES                                        \n\
varying lowp vec4 v_fragmentColor;                  \n\
varying mediump vec2 v_texCoord;                    \n\
#else                                               \n\
varying vec4 v_fragmentColor;                       \n\
varying vec2 v_texCoord;                            \n\
#endif                                              \n\
                                                    \n\
void main()                                         \n\
{                                                   \n\
    gl_Position = CC_PMatrix * a_position;          \n\
    v_fragmentColor = a_color;                      \n\
    v_texCoord = a_texCoord;                        \n\
}                                                   \n\
";

const GLchar *ccShaderDefault_frag =
"                                           \n\
#ifdef GL_ES                                \n\
precision lowp float;                       \n\
#endif                                      \n\
                                            \n\
varying vec4 v_fragmentColor;               \n\
varying vec2 v_texCoord;                    \n\
uniform sampler2D u_texture;                \n\
                                            \n\
void main()                                                                       \n\
{                                                                                 \n\
    vec4 texColor = texture2D(u_texture, v_texCoord);                             \n\
    if ( texColor.a <= 0.0 )                                                      \n\
        discard;                                                                  \n\
    gl_FragColor = texColor * v_fragmentColor;                                    \n\
}                                                                                 \n\
";

const GLchar *ccShaderColorGray_frag =
"                                                \n\
#ifdef GL_ES                                     \n\
precision lowp float;                            \n\
#endif                                           \n\
                                                 \n\
varying vec4      v_fragmentColor;               \n\
varying vec4      v_fragmentOffsetColor;         \n\
varying vec2      v_texCoord;                    \n\
uniform sampler2D u_texture;                     \n\
uniform float     Intensity;                     \n\
                                                 \n\
void main()                                                                   \n\
{                                                                             \n\
    vec4 texColor = texture2D(u_texture, v_texCoord);                         \n\
    if ( texColor.a <= 0.0 )                                                  \n\
        discard;                                                              \n\
    texColor = texColor * v_fragmentColor;                                    \n\
	float Y = texColor.r * 0.299 + texColor.g * 0.587 + texColor.b * 0.114;   \n\
    vec3 grayColor = mix(texColor.rgb, vec3(Y, Y, Y), Intensity);             \n\
    gl_FragColor = vec4(grayColor.rgb, texColor.a);                           \n\
}                                                                             \n\
";

const GLchar *ccShaderColorSepia_frag =
"                                                \n\
#ifdef GL_ES                                     \n\
precision lowp float;                            \n\
#endif                                           \n\
                                                 \n\
varying vec4      v_fragmentColor;               \n\
varying vec4      v_fragmentOffsetColor;         \n\
varying vec2      v_texCoord;                    \n\
uniform sampler2D u_texture;                     \n\
uniform float     Intensity;                     \n\
                                                 \n\
void main()                                                                                     \n\
{                                                                                               \n\
    vec4 texColor = texture2D(u_texture, v_texCoord);                                           \n\
    if ( texColor.a <= 0.0 )                                                                    \n\
        discard;                                                                                \n\
    texColor = texColor * v_fragmentColor;                                                      \n\
	float Y = texColor.r * 0.299 + texColor.g * 0.587 + texColor.b * 0.114;                     \n\
    vec3 sepiaColor = mix(texColor.rgb, vec3(Y, Y * 0.691588785, Y * 0.401869), Intensity);     \n\
    gl_FragColor = vec4(sepiaColor.rgb, texColor.a);                                            \n\
}                                                                                               \n\
";

const GLchar *ccShaderColorNega_frag =
"                                                \n\
#ifdef GL_ES                                     \n\
precision lowp float;                            \n\
#endif                                           \n\
                                                 \n\
varying vec4      v_fragmentColor;               \n\
varying vec4      v_fragmentOffsetColor;         \n\
varying vec2      v_texCoord;                    \n\
uniform sampler2D u_texture;                     \n\
uniform float     Intensity;                     \n\
                                                 \n\
void main()                                                                                                     \n\
{                                                                                                               \n\
    vec4 texColor = texture2D(u_texture, v_texCoord);                                                           \n\
    if ( texColor.a <= 0.0 )                                                                                    \n\
        discard;                                                                                                \n\
    float r = mix(texColor.r, (1.0 - texColor.r) * texColor.a, Intensity);                                      \n\
	float g = mix(texColor.g, (1.0 - texColor.g) * texColor.a, Intensity);                                      \n\
	float b = mix(texColor.b, (1.0 - texColor.b) * texColor.a, Intensity);                                      \n\
	gl_FragColor = vec4(vec3(r, g, b), texColor.a) * v_fragmentColor;                                           \n\
}                                                                                                               \n\
";

const GLchar *ccShaderColorGameBoyPalette_frag =
"                                                                       \n\
#ifdef GL_ES                                                            \n\
precision mediump float;                                                \n\
#endif                                                                  \n\
                                                                        \n\
varying vec4 v_fragmentColor;                                           \n\
varying vec2 v_texCoord;                                                \n\
uniform sampler2D u_texture;                                            \n\
uniform float     Intensity;                                            \n\
                                                                        \n\
void main()                                                             \n\
{                                                                       \n\
	vec4 texColor = texture2D(u_texture, v_texCoord);                   \n\
    if ( texColor.a <= 0.0 )                                            \n\
	    discard;                                                        \n\
    vec3 color = texColor.rgb;                                          \n\
                                                                        \n\
	float gamma = 1.5;                                                  \n\
	color.r = pow(color.r, gamma);                                      \n\
	color.g = pow(color.g, gamma);                                      \n\
	color.b = pow(color.b, gamma);                                      \n\
                                                                        \n\
	vec3 col1 = vec3(0.612, 0.725, 0.086);                              \n\
	vec3 col2 = vec3(0.549, 0.667, 0.078);                              \n\
	vec3 col3 = vec3(0.188, 0.392, 0.188);                              \n\
	vec3 col4 = vec3(0.063, 0.247, 0.063);                              \n\
                                                                        \n\
	float dist1 = length(color - col1);                                 \n\
	float dist2 = length(color - col2);                                 \n\
	float dist3 = length(color - col3);                                 \n\
	float dist4 = length(color - col4);                                 \n\
                                                                        \n\
	float d = min(dist1, dist2);                                        \n\
	d = min(d, dist3);                                                  \n\
	d = min(d, dist4);                                                  \n\
                                                                        \n\
	if (d == dist1) {                                                   \n\
		color = col1;                                                   \n\
	}                                                                   \n\
	else if (d == dist2) {                                              \n\
		color = col2;                                                   \n\
	}                                                                   \n\
	else if (d == dist3) {                                              \n\
		color = col3;                                                   \n\
	}                                                                   \n\
	else {                                                              \n\
		color = col4;                                                   \n\
	}                                                                   \n\
                                                                        \n\
    vec3 gbpColor = mix(texColor.rgb, color, Intensity);                \n\
	gl_FragColor = vec4(gbpColor, texColor.a) * v_fragmentColor;        \n\
}                                                                       \n\
";

const GLchar *ccShaderColorChromaticAberration_frag =
"                                           \n\
#ifdef GL_ES                                \n\
precision lowp float;                       \n\
#endif                                      \n\
                                            \n\
varying vec4      v_fragmentColor;          \n\
varying vec2      v_texCoord;               \n\
uniform sampler2D u_texture;                \n\
uniform vec2      Resolution;               \n\
uniform float     Shift;                    \n\
uniform float     Intensity;                \n\
                                            \n\
void main()                                                                              \n\
{                                                                                        \n\
    vec4 texColor = texture2D(u_texture, v_texCoord);                                    \n\
    vec2 unit = 1.0 / Resolution.xy;                                                     \n\
    float r = texture2D(u_texture, v_texCoord - vec2(unit.x * Shift * Intensity, 0)).r;  \n\
    float b = texture2D(u_texture, v_texCoord + vec2(unit.x * Shift * Intensity, 0)).b;  \n\
	gl_FragColor = vec4(r, texColor.g, b, texColor.a) * v_fragmentColor;                 \n\
}                                                                                        \n\
";

const GLchar *ccShaderColorDark_frag =
"                                           \n\
#ifdef GL_ES                                \n\
precision lowp float;                       \n\
#endif                                      \n\
                                            \n\
varying vec4 v_fragmentColor;               \n\
varying vec2 v_texCoord;                    \n\
uniform sampler2D u_texture;                \n\
uniform float     Intensity;                \n\
                                            \n\
void main()                                                                      \n\
{                                                                                \n\
    vec4 texColor = texture2D(CC_Texture0, v_texCoord);                          \n\
    if ( texColor.a <= 0.0 )                                                     \n\
        discard;                                                                 \n\
    vec3 darkColor = mix(texColor.rgb, vec3(0.0, 0.0, 0.0), Intensity);          \n\
    gl_FragColor = vec4(darkColor, texColor.a) * v_fragmentColor;                \n\
}                                                                                \n\
";

const GLchar *ccShaderColorDarkMask_frag =
"                                           \n\
#ifdef GL_ES                                \n\
precision lowp float;                       \n\
#endif                                      \n\
                                            \n\
varying vec4 v_fragmentColor;               \n\
varying vec2 v_texCoord;                    \n\
//uniform sampler2D u_texture;              \n\
uniform sampler2D maskTexture;              \n\
uniform float     Intensity;                \n\
                                            \n\
void main()                                                                      \n\
{                                                                                \n\
    vec4 texColor = texture2D(CC_Texture0, v_texCoord);                          \n\
    if ( texColor.a <= 0.0 )                                                     \n\
        discard;                                                                 \n\
    vec3 darkColor = mix(texColor.rgb, vec3(0.0, 0.0, 0.0), Intensity);          \n\
    vec4 maskTexColor = texture2D(maskTexture, v_texCoord);                      \n\
    if (maskTexColor.a >= 0.1) {                                                 \n\
        darkColor = vec3(texColor.r, texColor.g, texColor.b);                    \n\
    } else if (maskTexColor.a > 0.0) {                                           \n\
        darkColor = mix(texColor.rgb, vec3(0.0, 0.0, 0.0), (1.0 - maskTexColor.a * 10) * Intensity); \n\
    }                                                                            \n\
    gl_FragColor = vec4(darkColor, texColor.a) * v_fragmentColor;                \n\
}                                                                                \n\
";

const GLchar *ccShaderTransparency_frag =
"                                           \n\
#ifdef GL_ES                                \n\
precision lowp float;                       \n\
#endif                                      \n\
                                            \n\
varying vec4 v_fragmentColor;               \n\
varying vec2 v_texCoord;                    \n\
uniform sampler2D u_texture;                \n\
uniform float     Intensity;                \n\
                                            \n\
void main()                                                                      \n\
{                                                                                \n\
    vec4 texColor = texture2D(CC_Texture0, v_texCoord);                          \n\
    if ( texColor.a <= 0.0 )                                                     \n\
        discard;                                                                 \n\
    texColor *= (1 - Intensity);                                                 \n\
    gl_FragColor = texColor * v_fragmentColor;                                   \n\
}                                                                                \n\
";

const GLchar *ccShaderColorSolid_frag =
"                                           \n\
#ifdef GL_ES                                \n\
precision lowp float;                       \n\
#endif                                      \n\
                                            \n\
varying vec4 v_fragmentColor;               \n\
varying vec2 v_texCoord;                    \n\
uniform sampler2D u_texture;                \n\
uniform vec3      SolidColor;               \n\
uniform float     Intensity;                \n\
                                            \n\
void main()                                                                      \n\
{                                                                                \n\
    vec4 texColor = texture2D(u_texture, v_texCoord);                            \n\
    if ( texColor.a <= 0.0 )                                                     \n\
        discard;                                                                 \n\
    vec3 color = mix(texColor.rgb, SolidColor, Intensity);                       \n\
    gl_FragColor = vec4(color, texColor.a) * v_fragmentColor;                    \n\
}                                                                                \n\
";

const GLchar *ccShaderTextureRepeat_frag =
"                                           \n\
#ifdef GL_ES                                \n\
precision lowp float;                       \n\
#endif                                      \n\
                                            \n\
varying vec4      v_fragmentColor;          \n\
varying vec2      v_texCoord;               \n\
uniform sampler2D u_texture;                \n\
uniform vec2      Resolution;               \n\
uniform vec2      TexSize;                  \n\
                                            \n\
float mod(float v, float m) {               \n\
    if (v < m) {                            \n\
        return v;                           \n\
    }                                       \n\
    do {                                    \n\
        v -= m;                             \n\
    } while (v > m);                        \n\
    return v;                               \n\
}                                           \n\
                                            \n\
void main()                                                                      \n\
{                                                                                \n\
    vec2 unit = Resolution.xy / TexSize.xy;                                      \n\
    vec2 texCoord = vec2(mod(v_texCoord.x, unit.x), mod(v_texCoord.y, unit.y));  \n\
    vec4 texColor = texture2D(u_texture, texCoord);                              \n\
	gl_FragColor = texColor * v_fragmentColor;                                   \n\
}                                                                                \n\
";

const GLchar *ccShaderColorRgba_frag =
"                                           \n\
#ifdef GL_ES                                \n\
precision lowp float;                       \n\
#endif                                      \n\
                                            \n\
varying vec4 v_fragmentColor;               \n\
varying vec2 v_texCoord;                    \n\
uniform sampler2D u_texture;                \n\
uniform vec4      RgbaColor;                \n\
uniform float     Intensity;                \n\
                                            \n\
void main()                                                                     \n\
{                                                                               \n\
    vec4 texColor = texture2D(u_texture, v_texCoord);                           \n\
    if ( texColor.a <= 0.0 )                                                    \n\
        discard;                                                                \n\
    float r = texColor.r;                                                       \n\
    if (texColor.r < RgbaColor.r) {                                             \n\
        r = texColor.r + (RgbaColor.r - texColor.r) * RgbaColor.a * Intensity;  \n\
    } else if (texColor.r > RgbaColor.r) {                                      \n\
        r = texColor.r - (texColor.r - RgbaColor.r) * RgbaColor.a * Intensity;  \n\
    }                                                                           \n\
    float g = texColor.g;                                                       \n\
    if (texColor.g < RgbaColor.g) {                                             \n\
        g = texColor.g + (RgbaColor.g - texColor.g) * RgbaColor.a * Intensity;  \n\
    } else if (texColor.g > RgbaColor.g) {                                      \n\
        g = texColor.g - (texColor.g - RgbaColor.g) * RgbaColor.a * Intensity;  \n\
    }                                                                           \n\
    float b = texColor.b;                                                       \n\
    if (texColor.b < RgbaColor.b) {                                             \n\
        b = texColor.b + (RgbaColor.b - texColor.b) * RgbaColor.a * Intensity;  \n\
    } else if (texColor.b > RgbaColor.b) {                                      \n\
        b = texColor.b - (texColor.b - RgbaColor.b) * RgbaColor.a * Intensity;  \n\
    }                                                                           \n\
    gl_FragColor = vec4(r, g, b, texColor.a) * v_fragmentColor;                 \n\
}                                                                               \n\
";

const GLchar *ccShaderMosaic_frag =
"                                                                       \n\
#ifdef GL_ES                                                            \n\
precision mediump float;                                                \n\
#endif                                                                  \n\
                                                                        \n\
varying vec4        v_fragmentColor;                                    \n\
varying vec2        v_texCoord;                                         \n\
uniform sampler2D   u_texture;                                          \n\
uniform vec2        u_texSize;                                          \n\
uniform float       u_mosaicLevel;                                      \n\
uniform float       Intensity;                                          \n\
                                                                        \n\
void main()                                                             \n\
{                                                                       \n\
    vec4 color;                                                         \n\
	float level = u_mosaicLevel * Intensity;                            \n\
    if(level > 1.0) {                                                   \n\
        vec2 target;                                                    \n\
        target.x = float(int(v_texCoord.x / level * u_texSize.x + 0.5)) * level / u_texSize.x;    \n\
        target.y = float(int(v_texCoord.y / level * u_texSize.y + 0.5)) * level / u_texSize.y;    \n\
        color = texture2D(u_texture, target);                           \n\
    } else {                                                            \n\
        color = texture2D(u_texture, v_texCoord);                       \n\
    }                                                                   \n\
    gl_FragColor = color * v_fragmentColor;                             \n\
}                                                                       \n\
";

const GLchar *ccShaderNoisy_frag =
"                                                                       \n\
#ifdef GL_ES                                                            \n\
precision lowp float;                                                   \n\
#endif                                                                  \n\
                                                                        \n\
varying vec4      v_fragmentColor;                                      \n\
varying vec2      v_texCoord;                                           \n\
uniform sampler2D u_texture;                                            \n\
uniform float Time;                                                     \n\
uniform float Intensity;                                                \n\
                                                                        \n\
float rand(vec2 co){                                                    \n\
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);     \n\
}                                                                       \n\
                                                                        \n\
void main()                                                             \n\
{                                                                       \n\
  vec4 color = texture2D(u_texture, v_texCoord);                        \n\
  if ( color.a <= 0.0 )                                                 \n\
	  discard;                                                          \n\
  vec2 pos = v_texCoord;                                                \n\
  pos *= sin(Time);                                                     \n\
  float r = rand(pos);                                                  \n\
  vec3 noise = vec3(r);                                                 \n\
  vec3 noiseColor = mix(color.rgb, noise, Intensity);                   \n\
  float a = color.a * (1.0 - Intensity) + Intensity;                    \n\
  gl_FragColor = vec4(noiseColor.rgb, a) * v_fragmentColor;             \n\
}                                                                       \n\
";

const GLchar *ccShaderCRTMonitor_frag =
"                                                                       \n\
#ifdef GL_ES                                                            \n\
precision lowp float;                                                   \n\
#endif                                                                  \n\
                                                                        \n\
varying vec4      v_fragmentColor;                                      \n\
varying vec2      v_texCoord;                                           \n\
uniform sampler2D u_texture;                                            \n\
uniform float     Time;                                                 \n\
uniform float     Intensity;                                            \n\
                                                                        \n\
void main()                                                             \n\
{                                                                       \n\
  vec4 texColor = texture2D(u_texture, v_texCoord);                     \n\
  vec3 color = texColor.rgb;                                            \n\
                                                                        \n\
  color.rgb -= abs(sin(v_texCoord.y * 100.0 - Time * 5.0)) * 0.08;      \n\
  color.rgb -= abs(sin(v_texCoord.y * 300.0 - Time * 10.0)) * 0.05;     \n\
                                                                        \n\
  vec3 crtmColor = mix(texColor.rgb, color, Intensity);                 \n\
  gl_FragColor = vec4(crtmColor, texColor.a) * v_fragmentColor;         \n\
}                                                                       \n\
";

const GLchar *ccShaderBlur_frag =
"                                                                       \n\
#ifdef GL_ES                                                            \n\
precision lowp float;                                                   \n\
#endif                                                                  \n\
                                                                        \n\
varying vec4 v_fragmentColor;                                           \n\
varying vec2 v_texCoord;                                                \n\
uniform sampler2D u_texture;                                            \n\
                                                                        \n\
uniform vec2 resolution;                                                \n\
uniform float blurRadius;                                               \n\
uniform float Intensity;                                                \n\
                                                                        \n\
vec4 blur(vec2);                                                        \n\
                                                                        \n\
void main(void)                                                         \n\
{                                                                       \n\
    vec4 col = blur(v_texCoord);                                        \n\
    gl_FragColor = vec4(col) * v_fragmentColor;                         \n\
}                                                                       \n\
                                                                        \n\
vec4 blur(vec2 p)                                                       \n\
{                                                                       \n\
    if (blurRadius > 0.0 && Intensity > 0.0)                            \n\
    {                                                                   \n\
        vec4 col = vec4(0);                                             \n\
        vec2 unit = 1.0 / resolution.xy;                                \n\
                                                                        \n\
        float r = blurRadius * Intensity;                               \n\
        float sampleStep = Intensity;                                   \n\
                                                                        \n\
        float count = 0.0;                                              \n\
                                                                        \n\
        for(float x = -r; x <= r; x += sampleStep)                      \n\
        {                                                               \n\
            for(float y = -r; y <= r; y += sampleStep)                  \n\
            {                                                           \n\
                float weight = (r - abs(x)) * (r - abs(y));             \n\
                col += texture2D(u_texture, p + vec2(x * unit.x, y * unit.y)) * weight;    \n\
                count += weight;                                        \n\
            }                                                           \n\
        }                                                               \n\
                                                                        \n\
        return col / count;                                             \n\
    }                                                                   \n\
                                                                        \n\
    return texture2D(u_texture, p);                                     \n\
}                                                                       \n\
";

const GLchar *ccShaderColorAfterimageRgba_frag =
"                                           \n\
#ifdef GL_ES                                \n\
precision lowp float;                       \n\
#endif                                      \n\
                                            \n\
varying vec4 v_fragmentColor;               \n\
varying vec2 v_texCoord;                    \n\
uniform sampler2D u_texture;                \n\
uniform vec4 RgbaColor;                     \n\
uniform float Intensity;                    \n\
uniform float Alpha;                        \n\
void main()                                              \n\
{                                                        \n\
	vec4 texColor = texture2D(u_texture, v_texCoord);    \n\
	if ( (texColor.a * Alpha)<= 0.0)                     \n\
        discard;                                         \n\
	texColor = texColor * v_fragmentColor;				 \n\
	vec4 color = mix(texColor, RgbaColor, RgbaColor.a);  \n\
	color *= texColor.a;                                 \n\
	gl_FragColor = color * Alpha;                        \n\
}                                                        \n\
";

const GLchar *ccShaderColorSilhouetteimageRgba_frag =
"                                           \n\
#ifdef GL_ES                                \n\
precision lowp float;                       \n\
#endif                                      \n\
                                            \n\
varying vec4 v_fragmentColor;               \n\
varying vec2 v_texCoord;                    \n\
uniform sampler2D u_texture;                \n\
uniform vec4 RgbaColor;                     \n\
uniform float Intensity;                    \n\
uniform float Alpha;                        \n\
void main()                                              \n\
{                                                        \n\
	vec4 texColor = texture2D(u_texture, v_texCoord);    \n\
	if ( texColor.a <= 0.0)                              \n\
        discard;                                         \n\
	gl_FragColor = RgbaColor;                            \n\
}                                                        \n\
";

const GLchar *ccShaderColorSilhouetteimageRgbaMultiply_frag =
"                                           \n\
#ifdef GL_ES                                \n\
precision lowp float;                       \n\
#endif                                      \n\
                                            \n\
varying vec4 v_fragmentColor;               \n\
varying vec2 v_texCoord;                    \n\
uniform sampler2D u_texture;                \n\
uniform vec4 RgbaColor;                     \n\
uniform float Intensity;                    \n\
uniform float Alpha;                        \n\
void main()                                              \n\
{                                                        \n\
	vec4 texColor = texture2D(u_texture, v_texCoord);    \n\
	if ( texColor.a <= 0.0)                              \n\
        discard;                                         \n\
	gl_FragColor.rgb = vec3(1, 1, 1) * (1 - RgbaColor.a) + RgbaColor.rgb * RgbaColor.a; \n\
	gl_FragColor.a = RgbaColor.a;                        \n\
}                                                        \n\
";

const GLchar *ccShaderAlphaMask_frag =
"                                           \n\
#ifdef GL_ES                                \n\
precision lowp float;                       \n\
#endif                                      \n\
                                            \n\
varying vec4 v_fragmentColor;               \n\
varying vec2 v_texCoord;                    \n\
uniform sampler2D maskTexture;              \n\
                                            \n\
void main()                                                                      \n\
{                                                                                \n\
    vec4 texColor = texture2D(CC_Texture0, v_texCoord);                          \n\
    if ( texColor.a <= 0.0 )                                                     \n\
        discard;                                                                 \n\
    vec4 maskTexColor = texture2D(maskTexture, v_texCoord);                      \n\
    if ( maskTexColor.a <= 0.0 )                                                 \n\
        discard;                                                                 \n\
    gl_FragColor = texColor * v_fragmentColor;                                   \n\
}                                                                                \n\
";

const GLchar *ccShaderImage_frag =
"                                                                                         \n\
varying vec4 v_fragmentColor;                                                             \n\
varying vec2 v_texCoord;                                                                  \n\
uniform sampler2D maskTexture;                                                            \n\
uniform float Intensity;                                                                  \n\
uniform int imgPlacement;                                                                 \n\
uniform vec2 resolution;                                                                  \n\
uniform vec2 imgResolution;                                                               \n\
uniform vec2 imgSizeRate;                                                                 \n\
uniform vec2 imgXy;                                                                       \n\
uniform vec2 sxy;                                                                         \n\
                                                                                          \n\
float mod(float v, float m) {                                                             \n\
    if (v < m) {                                                                          \n\
        return v;                                                                         \n\
    }                                                                                     \n\
    do {                                                                                  \n\
        v -= m;                                                                           \n\
    } while (v > m);                                                                      \n\
    return v;                                                                             \n\
}                                                                                         \n\
                                                                                          \n\
void main()                                                                               \n\
{                                                                                         \n\
    vec4 texColor = texture2D(CC_Texture0, v_texCoord);                                   \n\
    if ( texColor.a <= 0.0f )                                                             \n\
        discard;                                                                          \n\
    vec2 imgCoord = v_texCoord;                                                           \n\
    imgCoord.y = 1.0f - imgCoord.y;//※Y軸を反転                                          \n\
    if (imgPlacement == 0) {//中央                                                        \n\
        imgCoord = imgCoord.xy * sxy + imgXy;                                             \n\
    } else if(imgPlacement == 1) {//拡大                                                  \n\
        imgCoord = imgCoord.xy + imgXy;                                                   \n\
    } else if(imgPlacement == 2) {//タイル                                                \n\
        imgCoord.xy = imgCoord.xy * (resolution.xy / imgResolution.xy) + imgXy;           \n\
        imgCoord = vec2(mod(imgCoord.x, imgSizeRate.x), mod(imgCoord.y, imgSizeRate.y));  \n\
    } else if(imgPlacement == 3) {//比率を維持して拡大                                    \n\
        imgCoord = imgCoord.xy * sxy.xy + imgXy;                                          \n\
    }                                                                                     \n\
    if (imgCoord.x < 0.0 || imgCoord.x >= 1.0 || imgCoord.y < 0.0 || imgCoord.y >= 1.0) { \n\
        gl_FragColor = texColor * v_fragmentColor;                                        \n\
    } else {                                                                              \n\
        vec4 maskTexColor = texture2D(maskTexture, imgCoord);                             \n\
        vec3 col;                                                                         \n\
        if (maskTexColor.a <= 0) {                                                        \n\
            col = texColor.rgb;                                                           \n\
        } else {                                                                          \n\
            col = mix(texColor.rgb, maskTexColor.rgb, Intensity);                         \n\
        }                                                                                 \n\
        gl_FragColor = vec4(col, texColor.a) * v_fragmentColor;                           \n\
    }                                                                                     \n\
}                                                                                         \n\
";

//-------------------------------------------------------------------------------------------------------------------
ShaderValue::ShaderValue() : agtk::EventTimer()
{
	_value = 0.0f;
	_prevValue = 0.0f;
	_nextValue = 0.0f;
	_ignored = false;
	_seconds = 0.0f;
}

ShaderValue::~ShaderValue()
{
}

bool ShaderValue::init(float value, bool ignored)
{
	if (agtk::EventTimer::init() == false) {
		return false;
	}
	_value = value;
	_prevValue = value;
	_nextValue = value;
	_oldValue = value;
	_ignored = ignored;

	this->setProcessingFunc([&](float dt) {
		_oldValue = _value;
		_value = AGTK_LINEAR_INTERPOLATE(_prevValue, _nextValue, _seconds, _timer);
	});
	this->setEndFunc([&]() {
		_oldValue = _value;
		_value = _nextValue;
	});

	return true;
}

float ShaderValue::setValue(float value, float seconds)
{
	_nextValue = value;
	_prevValue = _value;
	_seconds = seconds;
	this->start(seconds);
	return _value;
}

float ShaderValue::getValue()
{
	return this->getIgnored() ? 0.0f : _value;
}

bool ShaderValue::isChanged()
{
	return _value != _oldValue ? true : false;
}

ShaderValue &ShaderValue::operator=(const ShaderValue &shaderValue)
{
	//this->_state = shaderValue._state;	// _stateがkStateIdleに上書きされると、Shader::updateIntensity()で更新が行われずACT2-5632の不具合が発生するため、コメントアウト。
	this->_timer = shaderValue._timer;
	this->EventTimer::_seconds = shaderValue.EventTimer::_seconds;

	this->_value = shaderValue._value;
	this->_nextValue = shaderValue._nextValue;
	this->_prevValue = shaderValue._prevValue;
	this->_oldValue = shaderValue._oldValue;
	this->_ignored = shaderValue._ignored;
	this->_seconds = shaderValue._seconds;
	return *this;
}

void ShaderValue::getJsonData(rapidjson::Value& jsonData, rapidjson::Document::AllocatorType& allocator)
{
	//jsonData.AddMember("state", this->_state, allocator);
	jsonData.AddMember("value", this->_value, allocator);
	jsonData.AddMember("nextValue", this->_nextValue, allocator);
	jsonData.AddMember("prevValue", this->_prevValue, allocator);
	jsonData.AddMember("oldValue", this->_oldValue, allocator);
	jsonData.AddMember("ignored", this->_ignored, allocator);
	jsonData.AddMember("timer", this->_timer, allocator);
	jsonData.AddMember("seconds", this->_seconds, allocator);
}

void ShaderValue::setJsonData(const rapidjson::Value& jsonData)
{
	//if (jsonData.HasMember("state")) this->_state = (agtk::EventTimer::EnumState)jsonData["state"].GetInt();
	if (jsonData.HasMember("value")) this->_value = jsonData["value"].GetFloat();
	if (jsonData.HasMember("nextValue")) this->_nextValue = jsonData["nextValue"].GetFloat();
	if (jsonData.HasMember("prevValue")) this->_prevValue = jsonData["prevValue"].GetFloat();
	if (jsonData.HasMember("oldValue")) this->_oldValue = jsonData["oldValue"].GetFloat();
	if (jsonData.HasMember("ignored")) this->_ignored = jsonData["ignored"].GetBool();
	if (jsonData.HasMember("timer")) this->_timer = jsonData["timer"].GetFloat();
	if (jsonData.HasMember("seconds")) this->_seconds = jsonData["seconds"].GetFloat();
	this->EventTimer::_seconds = this->_seconds;
}

//-------------------------------------------------------------------------------------------------------------------
Shader::Shader()
{
	_programState = nullptr;
	_valueList = nullptr;
	_counter = 0.0f;
	_kind = ShaderKind::kShaderDefault;
	_value = nullptr;
	_shaderSize = cocos2d::Size::ZERO;
	_maskTexture = nullptr;
	_pauseFlag = false;
	_resumeFlag = false;
	_updateFlag = true;
	_glProgram = nullptr;
	_updateUniformsFlag = false;
	_userData = nullptr;
}

Shader::~Shader()
{
//	auto s = cocos2d::Director::getInstance()->getScheduler();
//	s->unscheduleUpdate(this);
	CC_SAFE_RELEASE_NULL(_programState);
	CC_SAFE_RELEASE_NULL(_valueList);
	CC_SAFE_RELEASE_NULL(_value);
	CC_SAFE_RELEASE_NULL(_maskTexture);
	CC_SAFE_RELEASE_NULL(_glProgram);
	CC_SAFE_RELEASE_NULL(_userData);
}

Shader *Shader::create(cocos2d::Size size, ShaderKind kind)
{
	auto p = new (std::nothrow) Shader();
	if (p && p->init(size, kind)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

Shader *Shader::createShaderKind(cocos2d::Node *node, cocos2d::Size size, ShaderKind kind, int hierarchyLimit)
{
	auto p = new (std::nothrow) Shader();
	if (p && p->initShaderKind(node, size, kind, hierarchyLimit)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

//--------------------------------------------------------------------------------------------------------------------
// 残像用のパーティクルに使用するシェーダーを生成する
//--------------------------------------------------------------------------------------------------------------------
cocos2d::GLProgramState *Shader::createShaderParticleAfterimage()
{
	auto program = new cocos2d::GLProgram();
	auto programState = cocos2d::GLProgramState::getOrCreateWithGLProgram(program);
	program->initWithByteArrays(ccShaderDefault_vert, ccShaderColorAfterimageRgba_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);

	program->link();
	program->updateUniforms();
	program->setUniformLocationWith4f(program->getUniformLocationForName("RgbaColor"), 1.0f, 1.0f, 1.0f, 0.0f);
	program->setUniformLocationWith1f(program->getUniformLocationForName("Alpha"), 1.0f);

	return programState;
}

bool Shader::init(cocos2d::Size size, ShaderKind kind)
{
	this->setShaderSize(size);
	auto value = agtk::ShaderValue::create(0.0f, true);
	if (value == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setValue(value);
	_blurParam.init(5.0f, 5.0f, this->getIntensity());
	this->createShader(kind);
	return true;
}

bool Shader::initShaderKind(cocos2d::Node *node, cocos2d::Size size, ShaderKind kind, int hierarchyLimit)
{
	this->setShaderSize(size);
	auto value = agtk::ShaderValue::create(0.0f, true);
	if (value == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setValue(value);
	_blurParam.init(5.0f, 5.0f, this->getIntensity());

	auto programState = this->createShader(kind);
	setShaderProgramRecursive(node, programState, 0, hierarchyLimit);
	return true;
}

void Shader::setShaderProgram(cocos2d::Node *node, int hierarchyLimit)
{
	auto programState = this->getProgramState();
	setShaderProgramRecursive(node, programState, 0, hierarchyLimit);
}

void Shader::setShaderProgramRecursive(cocos2d::Node *node, cocos2d::GLProgramState *programState, int hierarchy, int hierarchyLimit)
{
	if(node == nullptr || programState == nullptr){
		return;
	}
	//指定階層を超えた場合は処理無。
	if (hierarchyLimit >= 0 && hierarchy > hierarchyLimit) {
		return;
	}
	node->setGLProgramState(programState);
	auto children = node->getChildren();
	for(int i = 0; i < node->getChildrenCount(); i++){
		auto n = children.at(i);
		setShaderProgramRecursive(n, programState, hierarchy + 1, hierarchyLimit);
	}
}

cocos2d::GLProgramState *Shader::createShader(ShaderKind kind)
{
	cocos2d::GLProgramState *programState = nullptr;
	switch (kind) {
	case kShaderCRTMonitor:
		programState = this->createShaderCRTMonitor();
		break;
	case kShaderBlur:
		programState = this->createShaderBlur();
		break;
	case kShaderNoisy:
		programState = this->createShaderNoisy();
		break;
	case kShaderMosaic:
		programState = this->createShaderMosaic();
		break;
	case kShaderColorGray:
		programState = this->createShaderColorGray();
		break;
	case kShaderColorSepia:
		programState = this->createShaderColorSepia();
		break;
	case kShaderColorNega:
		programState = this->createShaderColorNega();
		break;
	case kShaderColorGameboy:
		programState = this->createShaderColorGameBoy();
		break;
	case kShaderColorDark:
		programState = this->createShaderColorDark();
		break;
	case kShaderColorDarkMask:
		programState = this->createShaderColorDarkMask();
		break;
	case kShaderColorChromaticAberration:
		programState = this->createShaderColorChromaticAberration();
		break;
	case kShaderColorSolidColor:
		programState = this->createShaderColorSolidColor();
		break;
	case kShaderColorRgba:
		programState = this->createShaderColorRgba();
		break;
	case kShaderColorAfterimageRbga:
		programState = this->createShaderColorAfterimageAlpha();
		break;
	case kShaderColorSilhouetteimageRbga:
		programState = this->createShaderColorShilhouetteimage();
		break;
	case kShaderColorSilhouetteimageRbgaMultiply:
		programState = this->createShaderColorMultiplyShilhouetteimage();
		break;
	case kShaderTextureRepeat:
		programState = this->createShaderTextureRepeat();
		break;
	case kShaderAlphaMask:
		programState = this->createShaderAlphaMask();
		break;
	case kShaderImage:
		programState = this->createShaderImage();
		break;
	case kShaderTransparency:
		programState = this->createShaderTransparency();
		break;
	case kShaderDefault:
		programState = this->createShaderDefault();
		break;
	default:CC_ASSERT(0);
	}
	CC_ASSERT(programState);
	_kind = kind;
	this->setProgramState(programState);
	return programState;
}

static cocos2d::GLProgram* createGLProgram()
{
	auto ret = new (std::nothrow) cocos2d::GLProgram();
	if (ret) {
		ret->autorelease();
		return ret;
	}
	CC_SAFE_DELETE(ret);
	return nullptr;
}

cocos2d::GLProgramState *Shader::createShaderDefault()
{
	auto program = createGLProgram();
	auto programState = cocos2d::GLProgramState::create(program);

	program->initWithByteArrays(ccShaderDefault_vert, ccShaderDefault_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);

	program->link();
	program->updateUniforms();

	return programState;
}

cocos2d::GLProgramState *Shader::createShaderColorGray()
{
	auto program = createGLProgram();
	auto programState = cocos2d::GLProgramState::create(program);

	program->initWithByteArrays(ccShaderDefault_vert, ccShaderColorGray_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);

	program->link();
	program->updateUniforms();
	program->setUniformLocationWith1f(program->getUniformLocationForName("Intensity"), this->getIntensity());

	return programState;
}

cocos2d::GLProgramState *Shader::createShaderColorSepia()
{
	auto program = createGLProgram();
	auto programState = cocos2d::GLProgramState::create(program);

	program->initWithByteArrays(ccShaderDefault_vert, ccShaderColorSepia_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);

	program->link();
	program->updateUniforms();
	program->setUniformLocationWith1f(program->getUniformLocationForName("Intensity"), this->getIntensity());

	return programState;
}

cocos2d::GLProgramState *Shader::createShaderColorNega()
{
	auto program = createGLProgram();
	auto programState = cocos2d::GLProgramState::create(program);

	program->initWithByteArrays(ccShaderDefault_vert, ccShaderColorNega_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);

	program->link();
	program->updateUniforms();
	program->setUniformLocationWith1f(program->getUniformLocationForName("Intensity"), this->getIntensity());

	return programState;
}

cocos2d::GLProgramState *Shader::createShaderColorGameBoy()
{
	auto program = createGLProgram();
	auto programState = cocos2d::GLProgramState::create(program);

	program->initWithByteArrays(ccShaderDefault_vert, ccShaderColorGameBoyPalette_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);

	program->link();
	program->updateUniforms();
	program->setUniformLocationWith1f(program->getUniformLocationForName("Intensity"), this->getIntensity());

	return programState;
}

cocos2d::GLProgramState *Shader::createShaderColorDark()
{
	auto program = createGLProgram();
	auto programState = cocos2d::GLProgramState::create(program);

	program->initWithByteArrays(ccShaderDefault_vert, ccShaderColorDark_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);
	program->link();
	program->updateUniforms();
	program->setUniformLocationWith1f(program->getUniformLocationForName("Intensity"), this->getIntensity());

	return programState;
}

cocos2d::GLProgramState *Shader::createShaderTransparency()
{
	auto program = createGLProgram();
	auto programState = cocos2d::GLProgramState::create(program);

	program->initWithByteArrays(ccShaderDefault_vert, ccShaderTransparency_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);
	program->link();
	program->updateUniforms();
	program->setUniformLocationWith1f(program->getUniformLocationForName("Intensity"), this->getIntensity());

	return programState;
}

cocos2d::GLProgramState *Shader::createShaderColorDarkMask()
{
	auto program = createGLProgram();
	program->initWithByteArrays(ccShaderDefault_vert, ccShaderColorDarkMask_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);
	program->link();
	program->updateUniforms();
	program->setUniformLocationWith1f(program->getUniformLocationForName("Intensity"), this->getIntensity());

	auto programState = cocos2d::GLProgramState::create(program);
	return programState;
}

cocos2d::GLProgramState *Shader::createShaderColorChromaticAberration()
{
	auto program = createGLProgram();
	auto programState = cocos2d::GLProgramState::create(program);

	program->initWithByteArrays(ccShaderDefault_vert, ccShaderColorChromaticAberration_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);

	program->link();
	program->updateUniforms();
	cocos2d::Size shaderSize = this->getShaderSize();
	if (shaderSize.width == 0 || shaderSize.height == 0) {
		//※サイズ設定がいない場合は、描画サイズにする。
		shaderSize = cocos2d::Director::getInstance()->getVisibleSize();
	}
	program->setUniformLocationWith2f(program->getUniformLocationForName("Resolution"), shaderSize.width, shaderSize.height);
	program->setUniformLocationWith1f(program->getUniformLocationForName("Shift"), 5.0f);
	program->setUniformLocationWith1f(program->getUniformLocationForName("Intensity"), this->getIntensity());

	return programState;
}

cocos2d::GLProgramState *Shader::createShaderColorSolidColor()
{
	auto program = createGLProgram();
	auto programState = cocos2d::GLProgramState::create(program);

	program->initWithByteArrays(ccShaderDefault_vert, ccShaderColorSolid_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);

	program->link();
	program->updateUniforms();
	program->setUniformLocationWith3f(program->getUniformLocationForName("SolidColor"), 1.0f, 0.0f, 0.0f);
	program->setUniformLocationWith1f(program->getUniformLocationForName("Intensity"), this->getIntensity());

	return programState;
}

cocos2d::GLProgramState *Shader::createShaderColorRgba()
{
	auto program = createGLProgram();
	auto programState = cocos2d::GLProgramState::create(program);

	program->initWithByteArrays(ccShaderDefault_vert, ccShaderColorRgba_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);

	program->link();
	program->updateUniforms();
	program->setUniformLocationWith4f(program->getUniformLocationForName("RgbaColor"), 1.0f, 1.0f, 1.0f, 1.0f);
	program->setUniformLocationWith1f(program->getUniformLocationForName("Intensity"), this->getIntensity());

	return programState;
}

cocos2d::GLProgramState *Shader::createShaderColorAfterimageAlpha()
{
	return createShaderParticleAfterimage();
}

cocos2d::GLProgramState *Shader::createShaderTextureRepeat()
{
	auto program = createGLProgram();
	auto programState = cocos2d::GLProgramState::create(program);

	program->initWithByteArrays(ccShaderDefault_vert, ccShaderTextureRepeat_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);

	program->link();
	program->updateUniforms();
	cocos2d::Size shaderSize = this->getShaderSize();
	if (shaderSize.width == 0 || shaderSize.height == 0) {
		//※サイズ設定がいない場合は、描画サイズにする。
		shaderSize = cocos2d::Director::getInstance()->getVisibleSize();
	}
	program->setUniformLocationWith2f(program->getUniformLocationForName("TexSize"), shaderSize.width, shaderSize.height);
	program->setUniformLocationWith2f(program->getUniformLocationForName("Resolution"), shaderSize.width, shaderSize.height);

	return programState;
}

cocos2d::GLProgramState *Shader::createShaderCRTMonitor()
{
	auto program = createGLProgram();
	auto programState = cocos2d::GLProgramState::create(program);

	program->initWithByteArrays(ccShaderDefault_vert, ccShaderCRTMonitor_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);

	program->link();
	program->updateUniforms();
	program->setUniformLocationWith1f(program->getUniformLocationForName("Time"), 0.0f);
	program->setUniformLocationWith1f(program->getUniformLocationForName("Intensity"), this->getIntensity());

	return programState;
}

cocos2d::GLProgramState *Shader::createShaderBlur()
{
	auto program = createGLProgram();
	auto programState = cocos2d::GLProgramState::create(program);

	program->initWithByteArrays(ccShaderDefault_vert, ccShaderBlur_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);

	program->link();
	program->updateUniforms();
	program->setUniformLocationWith1f(program->getUniformLocationForName("blurRadius"), _blurParam.blurRadius());
	program->setUniformLocationWith1f(program->getUniformLocationForName("Intensity"), _blurParam.intensity());
	cocos2d::Size shaderSize = this->getShaderSize();
	if (shaderSize.width == 0 || shaderSize.height == 0) {
		//※サイズ設定がいない場合は、描画サイズにする。
		shaderSize = cocos2d::Director::getInstance()->getVisibleSize();
	}
	program->setUniformLocationWith2f(program->getUniformLocationForName("resolution"), shaderSize.width, shaderSize.height);

	return programState;
}

cocos2d::GLProgramState *Shader::createShaderNoisy()
{
	auto program = createGLProgram();
	auto programState = cocos2d::GLProgramState::create(program);

	program->initWithByteArrays(ccShaderDefault_vert, ccShaderNoisy_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);
	program->link();
	program->updateUniforms();
	program->setUniformLocationWith1f(program->getUniformLocationForName("Time"), 0.0f);
	program->setUniformLocationWith1f(program->getUniformLocationForName("Intensity"), this->getIntensity());

	return programState;
}

cocos2d::GLProgramState *Shader::createShaderMosaic()
{
	auto program = createGLProgram();
	auto programState = cocos2d::GLProgramState::create(program);

	program->initWithByteArrays(ccShaderDefault_vert, ccShaderMosaic_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);

	program->link();
	program->updateUniforms();
	cocos2d::Size shaderSize = this->getShaderSize();
	if (shaderSize.width == 0 || shaderSize.height == 0) {
		//※サイズ設定がいない場合は、描画サイズにする。
		shaderSize = cocos2d::Director::getInstance()->getVisibleSize();
	}
	program->setUniformLocationWith1f(program->getUniformLocationForName("u_mosaicLevel"), 10.0);
	program->setUniformLocationWith2f(program->getUniformLocationForName("u_texSize"), shaderSize.width, shaderSize.height);
	program->setUniformLocationWith1f(program->getUniformLocationForName("Intensity"), this->getIntensity());

	return programState;
}

//--------------------------------------------------------------------------------------------------------------------
// シルエットに使用するシェーダーを生成する
//--------------------------------------------------------------------------------------------------------------------
cocos2d::GLProgramState *Shader::createShaderColorShilhouetteimage()
{
	auto program = createGLProgram();
	auto programState = cocos2d::GLProgramState::create(program);

	program->initWithByteArrays(ccShaderDefault_vert, ccShaderColorSilhouetteimageRgba_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);

	program->link();
	program->updateUniforms();
	program->setUniformLocationWith4f(program->getUniformLocationForName("RgbaColor"), 1.0f, 1.0f, 1.0f, 0.0f);
	program->setUniformLocationWith1f(program->getUniformLocationForName("Alpha"), 1.0f);

	return programState;
}

cocos2d::GLProgramState *Shader::createShaderColorMultiplyShilhouetteimage()
{
	auto program = createGLProgram();
	auto programState = cocos2d::GLProgramState::create(program);

	program->initWithByteArrays(ccShaderDefault_vert, ccShaderColorSilhouetteimageRgbaMultiply_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);

	program->link();
	program->updateUniforms();
	program->setUniformLocationWith4f(program->getUniformLocationForName("RgbaColor"), 1.0f, 1.0f, 1.0f, 0.0f);
	program->setUniformLocationWith1f(program->getUniformLocationForName("Alpha"), 1.0f);

	return programState;
}

cocos2d::GLProgramState *Shader::createShaderAlphaMask()
{
	auto program = createGLProgram();
	program->initWithByteArrays(ccShaderDefault_vert, ccShaderAlphaMask_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);
	program->link();
	program->updateUniforms();

	auto programState = cocos2d::GLProgramState::create(program);
	return programState;
}

cocos2d::GLProgramState *Shader::createShaderImage()
{
	auto program = createGLProgram();
	program->initWithByteArrays(ccShaderDefault_vert, ccShaderImage_frag);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_POSITION, cocos2d::GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_COLOR, cocos2d::GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(cocos2d::GLProgram::ATTRIBUTE_NAME_TEX_COORD, cocos2d::GLProgram::VERTEX_ATTRIB_TEX_COORD);
	program->link();
	program->updateUniforms();

	auto programState = cocos2d::GLProgramState::create(program);
	programState->setUniformFloat("Intensity", this->getIntensity());
	programState->setUniformInt("imgPlacement", -1);

	return programState;
}

void Shader::updateShader(float delta)
{
	auto kind = this->getKind();
	if (kind != ShaderKind::kShaderCRTMonitor && kind != ShaderKind::kShaderNoisy) {
		return;
	}
	cocos2d::GLProgram *program;
	if (this->updateUniforms(program)) {
		program->setUniformLocationWith1f(program->getUniformLocationForName("Time"), delta);
	}
}

float Shader::setIntensity(float v, float seconds)
{
	auto value = this->getValue();
	CC_ASSERT(value);
	return value->setValue(v, seconds);
}

float Shader::getIntensity()
{
	auto value = this->getValue();
	CC_ASSERT(value);
	return value->getValue();
}

bool Shader::setIgnored(bool ignored)
{
	auto value = this->getValue();
	CC_ASSERT(value);
	bool tmpIgnored = value->getIgnored();
	if (tmpIgnored != ignored) {
		value->setIgnored(ignored);
		this->updateIntensity(); 
	}
	return tmpIgnored;
}

bool Shader::getIgnored()
{
	auto value = this->getValue();
	CC_ASSERT(value);
	return value->getIgnored();
}

void Shader::updateIntensity()
{
	auto value = this->getValue();
	auto state = value->getState();
	if (state == agtk::EventTimer::kStateIdle) {
		return;
	}
	_blurParam.set(value->getValue());
	cocos2d::GLProgram *program;
	if (this->updateUniforms(program)) {
		float intensity = this->getIgnored() ? 0.0f : value->getValue();
		if (this->getKind() == ShaderKind::kShaderColorDarkMask || this->getKind() == ShaderKind::kShaderImage) {
			auto programState = this->getProgramState();
			programState->setUniformFloat("Intensity", intensity);
			if (this->getMaskTexture()) {
				programState->setUniformTexture("maskTexture", this->getMaskTexture());
			}
		}
		else if (this->getKind() == ShaderKind::kShaderBlur) {
			program->setUniformLocationWith1f(program->getUniformLocationForName("blurRadius"), _blurParam.blurRadius());
			program->setUniformLocationWith1f(program->getUniformLocationForName("Intensity"), _blurParam.intensity());
		}
		else {
			program->setUniformLocationWith1f(program->getUniformLocationForName("Intensity"), intensity);
		}
	}
}

void Shader::setShaderSolidColor(cocos2d::Color3B color)
{
	if (this->getKind() != ShaderKind::kShaderColorSolidColor) {
		return;
	}
	cocos2d::GLProgram *program;
	if (this->updateUniforms(program)) {
		program->setUniformLocationWith3f(program->getUniformLocationForName("SolidColor"), (float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f);
	}
	_updateUniformsFlag = false;
}

void Shader::setShaderRgbaColor(cocos2d::Color4B color)
{
	if (this->getKind() != ShaderKind::kShaderColorRgba &&
		this->getKind() != ShaderKind::kShaderColorAfterimageRbga &&
		this->getKind() != ShaderKind::kShaderColorSilhouetteimageRbga &&
		this->getKind() != ShaderKind::kShaderColorSilhouetteimageRbgaMultiply) {
		return;
	}

	_rgbaColor = color;

	cocos2d::GLProgram *program;
	if (this->updateUniforms(program)) {
		program->setUniformLocationWith4f(program->getUniformLocationForName("RgbaColor"), (float)color.r / 255.0f, (float)color.g / 255.0f, (float)color.b / 255.0f, (float)color.a / 255.0f);
	}
	_updateUniformsFlag = false;
}

void Shader::setShaderAlpha(float alpha)
{
	if (this->getKind() != ShaderKind::kShaderColorAfterimageRbga &&
		this->getKind() != ShaderKind::kShaderColorSilhouetteimageRbga &&
		this->getKind() != ShaderKind::kShaderColorSilhouetteimageRbgaMultiply) {
		return;
	}
	cocos2d::GLProgram *program;
	if (updateUniforms(program)) {
		program->setUniformLocationWith1f(program->getUniformLocationForName("Alpha"), alpha);
	}
	_updateUniformsFlag = false;
}

void Shader::setShaderTextureRepeat(cocos2d::Size textureSize, cocos2d::Size resolutionSize)
{
	if (this->getKind() != ShaderKind::kShaderTextureRepeat) {
		return;
	}
	cocos2d::GLProgram *program;
	if (this->updateUniforms(program)) {
		program->setUniformLocationWith2f(program->getUniformLocationForName("TexSize"), textureSize.width, textureSize.height);
		program->setUniformLocationWith2f(program->getUniformLocationForName("Resolution"), resolutionSize.width, resolutionSize.height);
	}
	_updateUniformsFlag = false;
}

cocos2d::Color4B Shader::getShaderRgbaColor()
{
	if (this->getKind() != ShaderKind::kShaderColorRgba) {
		return cocos2d::Color4B(255, 255, 255, 255);
	}
	
	float intensity = getIntensity();
	return cocos2d::Color4B(
		_rgbaColor.r * intensity,
		_rgbaColor.g * intensity,
		_rgbaColor.b * intensity,
		_rgbaColor.a * intensity );
}

void Shader::setMaskTexture(cocos2d::Texture2D *texture2d)
{
	if (this->getKind() != ShaderKind::kShaderColorDarkMask
	&& this->getKind() != ShaderKind::kShaderAlphaMask
	&& this->getKind() != ShaderKind::kShaderImage) {
		CC_ASSERT(0);
		return;
	}
	if (texture2d) {
		CC_SAFE_RETAIN(texture2d);
		_maskTexture = texture2d;
	}
	auto programState = this->getProgramState();
	if (programState == nullptr) {
		return;
	}
	cocos2d::GLProgram *program;
	if (this->updateUniforms(program)) {
		auto programState = this->getProgramState();
		programState->setUniformTexture("maskTexture", texture2d);
		float intensity = this->getIgnored() ? 0.0f : this->getIntensity();
		programState->setUniformFloat("Intensity", intensity);
	}
	_updateUniformsFlag = false;
}

void Shader::resetMaskTexture()
{
	if (_maskTexture) {
		CC_SAFE_RELEASE(_maskTexture);
		_maskTexture = nullptr;
	}
}

void Shader::pause()
{
	_pauseFlag = true;
	_resumeFlag = false;
}

void Shader::resume()
{
	_resumeFlag = true;
	_pauseFlag = false;
}

void Shader::update(float delta)
{
	if (_pauseFlag) {
		_pauseFlag = false;
		_updateFlag = false;
	}
	if (_resumeFlag) {
		_resumeFlag = false;
		_updateFlag = true;
	}
	if (!_updateFlag) {
		return;
	}

	_counter += delta;
	//value
	auto value = this->getValue();
	value->update(delta);

	this->updateIntensity();
	this->updateShader(_counter);

	_updateUniformsFlag = false;
}

bool Shader::updateUniforms(cocos2d::GLProgram* &program)
{
	program = nullptr;
	auto programState = this->getProgramState();
	if (programState == nullptr) {
		return false;
	}
	program = programState->getGLProgram();
	CC_ASSERT(program);
	if (_updateUniformsFlag) {
		return true;
	}
	program->updateUniforms();
	_updateUniformsFlag = true;
	return true;
}

NS_AGTK_END //-----------------------------------------------------------------------------------//
