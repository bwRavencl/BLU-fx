/* Copyright 2014 Matteo Hausner */

#include "XPLMDataAccess.h"
#include "XPLMDefs.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMMenus.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"
#include "XPStandardWidgets.h"
#include "XPWidgets.h"

#include <fstream>
#include <string.h>
#include <sstream>

#if !IBM
#include <unistd.h>
#endif

#if APL
#include "ApplicationServices/ApplicationServices.h"
#include <OpenGL/gl.h>
#elif IBM
#include <GL/glew.h>
#include <windows.h>
#elif LIN
#include <GL/gl.h>
#include <X11/Xlib.h>
#endif

// define name
#define NAME "BLU-fx"
#define NAME_LOWERCASE "blu_fx"

// define version
#define VERSION "0.1"

// define config file path
#if IBM
#define CONFIG_PATH ".\\Resources\\plugins\\"NAME_LOWERCASE"\\"NAME_LOWERCASE".ini"
#else
#define CONFIG_PATH "./Resources/plugins/"NAME_LOWERCASE"/"NAME_LOWERCASE".ini"
#endif

// define default values for settings
#define DEFAULT_POST_PROCESSING_ENABLED 1
#define DEFAULT_FPS_LIMITER_ENABLED 0
#define DEFAULT_CONTROL_CINEMA_VERITE_ENABLED 1
#define DEFAULT_BRIGHTNESS 0.0f
#define DEFAULT_CONTRAST 1.0f
#define DEFAULT_SATURATION 1.0f
#define DEFAULT_RED_SCALE 0.0f
#define DEFAULT_GREEN_SCALE 0.0f
#define DEFAULT_BLUE_SCALE 0.0f
#define DEFAULT_RED_OFFSET 0.0f
#define DEFAULT_GREEN_OFFSET 0.0f
#define DEFAULT_BLUE_OFFSET 0.0f
#define DEFAULT_VIGNETTE 0.0f
#define DEFAULT_MAX_FRAME_RATE 35.0f
#define DEFAULT_DISABLE_CINEMA_VERITE_TIME 5.0f

// define polarized post-processing preset values
#define PRESET_POLARIZED_BRIGHTNESS 0.05f
#define PRESET_POLARIZED_CONTRAST 1.1f
#define PRESET_POLARIZED_SATURATION 1.4f
#define PRESET_POLARIZED_RED_SCALE 0.0f
#define PRESET_POLARIZED_GREEN_SCALE 0.0f
#define PRESET_POLARIZED_BLUE_SCALE -0.2f
#define PRESET_POLARIZED_RED_OFFSET 0.0f
#define PRESET_POLARIZED_GREEN_OFFSET 0.0f
#define PRESET_POLARIZED_BLUE_OFFSET 0.0f
#define PRESET_POLARIZED_VIGNETTE 0.6f

// define crazy hazy post-processing preset values
#define PRESET_CRAZY_HAZY_BRIGHTNESS 0.05f
#define PRESET_CRAZY_HAZY_CONTRAST 1.2f
#define PRESET_CRAZY_HAZY_SATURATION 0.7f
#define PRESET_CRAZY_HAZY_RED_SCALE 0.15f
#define PRESET_CRAZY_HAZY_GREEN_SCALE 0.15f
#define PRESET_CRAZY_HAZY_BLUE_SCALE 0.15f
#define PRESET_CRAZY_HAZY_RED_OFFSET 0.0f
#define PRESET_CRAZY_HAZY_GREEN_OFFSET 0.0f
#define PRESET_CRAZY_HAZY_BLUE_OFFSET 0.0f
#define PRESET_CRAZY_HAZY_VIGNETTE 0.3f

// define hdr-ish post-processing preset values
#define PRESET_HDR_ISH_BRIGHTNESS 0.0f
#define PRESET_HDR_ISH_CONTRAST 1.15f
#define PRESET_HDR_ISH_SATURATION 0.9f
#define PRESET_HDR_ISH_RED_SCALE 0.0f
#define PRESET_HDR_ISH_GREEN_SCALE 0.0f
#define PRESET_HDR_ISH_BLUE_SCALE 0.0f
#define PRESET_HDR_ISH_RED_OFFSET 0.0f
#define PRESET_HDR_ISH_GREEN_OFFSET 0.0f
#define PRESET_HDR_ISH_BLUE_OFFSET 0.0f
#define PRESET_HDR_ISH_VIGNETTE 0.6f

// define negative drab post-processing preset values
#define PRESET_NEGATIVE_DRAB_BRIGHTNESS 0.05f
#define PRESET_NEGATIVE_DRAB_CONTRAST 1.1f
#define PRESET_NEGATIVE_DRAB_SATURATION 1.3f
#define PRESET_NEGATIVE_DRAB_RED_SCALE 0.0f
#define PRESET_NEGATIVE_DRAB_GREEN_SCALE 0.0f
#define PRESET_NEGATIVE_DRAB_BLUE_SCALE 0.0f
#define PRESET_NEGATIVE_DRAB_RED_OFFSET 0.0f
#define PRESET_NEGATIVE_DRAB_GREEN_OFFSET 0.0f
#define PRESET_NEGATIVE_DRAB_BLUE_OFFSET 0.0f
#define PRESET_NEGATIVE_DRAB_VIGNETTE 0.3f

// define extra normal post-processing preset values
#define PRESET_EXTRA_NORMAL_BRIGHTNESS 0.05f
#define PRESET_EXTRA_NORMAL_CONTRAST 1.1f
#define PRESET_EXTRA_NORMAL_SATURATION 1.1f
#define PRESET_EXTRA_NORMAL_RED_SCALE 0.0f
#define PRESET_EXTRA_NORMAL_GREEN_SCALE 0.0f
#define PRESET_EXTRA_NORMAL_BLUE_SCALE 0.0f
#define PRESET_EXTRA_NORMAL_RED_OFFSET 0.0f
#define PRESET_EXTRA_NORMAL_GREEN_OFFSET 0.0f
#define PRESET_EXTRA_NORMAL_BLUE_OFFSET 0.0f
#define PRESET_EXTRA_NORMAL_VIGNETTE 0.0f

// define shadowhancer post-processing preset values
#define PRESET_SHADOWHANCER_BRIGHTNESS -0.15f
#define PRESET_SHADOWHANCER_CONTRAST 1.3f
#define PRESET_SHADOWHANCER_SATURATION 1.0f
#define PRESET_SHADOWHANCER_RED_SCALE 0.0f
#define PRESET_SHADOWHANCER_GREEN_SCALE 0.0f
#define PRESET_SHADOWHANCER_BLUE_SCALE 0.0f
#define PRESET_SHADOWHANCER_RED_OFFSET 0.0f
#define PRESET_SHADOWHANCER_GREEN_OFFSET 0.0f
#define PRESET_SHADOWHANCER_BLUE_OFFSET 0.0f
#define PRESET_SHADOWHANCER_VIGNETTE 0.0f

// define red shift post-processing preset values
#define PRESET_RED_SHIFT_BRIGHTNESS 0.0f
#define PRESET_RED_SHIFT_CONTRAST 1.1f
#define PRESET_RED_SHIFT_SATURATION 1.1f
#define PRESET_RED_SHIFT_RED_SCALE 0.1f
#define PRESET_RED_SHIFT_GREEN_SCALE 0.0f
#define PRESET_RED_SHIFT_BLUE_SCALE 0.0f
#define PRESET_RED_SHIFT_RED_OFFSET 0.0f
#define PRESET_RED_SHIFT_GREEN_OFFSET 0.0f
#define PRESET_RED_SHIFT_BLUE_OFFSET 0.0f
#define PRESET_RED_SHIFT_VIGNETTE 0.0f

// define green shift post-processing preset values
#define PRESET_GREEN_SHIFT_BRIGHTNESS 0.0f
#define PRESET_GREEN_SHIFT_CONTRAST 1.1f
#define PRESET_GREEN_SHIFT_SATURATION 1.1f
#define PRESET_GREEN_SHIFT_RED_SCALE 0.0f
#define PRESET_GREEN_SHIFT_GREEN_SCALE 0.1f
#define PRESET_GREEN_SHIFT_BLUE_SCALE 0.0f
#define PRESET_GREEN_SHIFT_RED_OFFSET 0.0f
#define PRESET_GREEN_SHIFT_GREEN_OFFSET 0.0f
#define PRESET_GREEN_SHIFT_BLUE_OFFSET 0.0f
#define PRESET_GREEN_SHIFT_VIGNETTE 0.0f

// define blue shift post-processing preset values
#define PRESET_BLUE_SHIFT_BRIGHTNESS 0.0f
#define PRESET_BLUE_SHIFT_CONTRAST 1.1f
#define PRESET_BLUE_SHIFT_SATURATION 1.1f
#define PRESET_BLUE_SHIFT_RED_SCALE 0.0f
#define PRESET_BLUE_SHIFT_GREEN_SCALE 0.0f
#define PRESET_BLUE_SHIFT_BLUE_SCALE 0.1f
#define PRESET_BLUE_SHIFT_RED_OFFSET 0.0f
#define PRESET_BLUE_SHIFT_GREEN_OFFSET 0.0f
#define PRESET_BLUE_SHIFT_BLUE_OFFSET 0.0f
#define PRESET_BLUE_SHIFT_VIGNETTE 0.0f

// define sunny florida post-processing preset values
#define PRESET_SUNNY_FLORIDA_BRIGHTNESS 0.1f
#define PRESET_SUNNY_FLORIDA_CONTRAST 1.5f
#define PRESET_SUNNY_FLORIDA_SATURATION 1.3f
#define PRESET_SUNNY_FLORIDA_RED_SCALE 0.0f
#define PRESET_SUNNY_FLORIDA_GREEN_SCALE 0.0f
#define PRESET_SUNNY_FLORIDA_BLUE_SCALE 0.0f
#define PRESET_SUNNY_FLORIDA_RED_OFFSET 0.0f
#define PRESET_SUNNY_FLORIDA_GREEN_OFFSET 0.0f
#define PRESET_SUNNY_FLORIDA_BLUE_OFFSET -0.1f
#define PRESET_SUNNY_FLORIDA_VIGNETTE 0.0f

// define mojave desert post-processing preset values
#define PRESET_MOJAVE_DESERT_BRIGHTNESS 0.0f
#define PRESET_MOJAVE_DESERT_CONTRAST 1.3f
#define PRESET_MOJAVE_DESERT_SATURATION 1.3f
#define PRESET_MOJAVE_DESERT_RED_SCALE 0.2f
#define PRESET_MOJAVE_DESERT_GREEN_SCALE 0.0f
#define PRESET_MOJAVE_DESERT_BLUE_SCALE 0.0f
#define PRESET_MOJAVE_DESERT_RED_OFFSET 0.0f
#define PRESET_MOJAVE_DESERT_GREEN_OFFSET 0.0f
#define PRESET_MOJAVE_DESERT_BLUE_OFFSET 0.0f
#define PRESET_MOJAVE_DESERT_VIGNETTE 0.6f

// define winter springs post-processing preset values
#define PRESET_WINTER_SPRINGS_BRIGHTNESS 0.07f
#define PRESET_WINTER_SPRINGS_CONTRAST 1.15f
#define PRESET_WINTER_SPRINGS_SATURATION 1.3f
#define PRESET_WINTER_SPRINGS_RED_SCALE 0.0f
#define PRESET_WINTER_SPRINGS_GREEN_SCALE 0.0f
#define PRESET_WINTER_SPRINGS_BLUE_SCALE 0.2f
#define PRESET_WINTER_SPRINGS_RED_OFFSET 0.0f
#define PRESET_WINTER_SPRINGS_GREEN_OFFSET 0.05f
#define PRESET_WINTER_SPRINGS_BLUE_OFFSET 0.0f
#define PRESET_WINTER_SPRINGS_VIGNETTE 0.0f

// define vivid dreams post-processing preset values
#define PRESET_VIVID_DREAMS_BRIGHTNESS 0.0f
#define PRESET_VIVID_DREAMS_CONTRAST 1.6f
#define PRESET_VIVID_DREAMS_SATURATION 1.5f
#define PRESET_VIVID_DREAMS_RED_SCALE 0.0f
#define PRESET_VIVID_DREAMS_GREEN_SCALE 0.0f
#define PRESET_VIVID_DREAMS_BLUE_SCALE -0.1f
#define PRESET_VIVID_DREAMS_RED_OFFSET 0.0f
#define PRESET_VIVID_DREAMS_GREEN_OFFSET 0.05f
#define PRESET_VIVID_DREAMS_BLUE_OFFSET 0.0f
#define PRESET_VIVID_DREAMS_VIGNETTE 0.6f

// define technocolor post-processing preset values
#define PRESET_TECHNOCOLOR_BRIGHTNESS 0.0f
#define PRESET_TECHNOCOLOR_CONTRAST 1.6f
#define PRESET_TECHNOCOLOR_SATURATION 1.5f
#define PRESET_TECHNOCOLOR_RED_SCALE 0.2f
#define PRESET_TECHNOCOLOR_GREEN_SCALE 0.0f
#define PRESET_TECHNOCOLOR_BLUE_SCALE -0.1f
#define PRESET_TECHNOCOLOR_RED_OFFSET 0.0f
#define PRESET_TECHNOCOLOR_GREEN_OFFSET 0.05f
#define PRESET_TECHNOCOLOR_BLUE_OFFSET 0.0f
#define PRESET_TECHNOCOLOR_VIGNETTE 0.65f

// define gun metal post-processing preset values
#define PRESET_GUN_METAL_BRIGHTNESS 0.0f
#define PRESET_GUN_METAL_CONTRAST 1.55f
#define PRESET_GUN_METAL_SATURATION 0.6f
#define PRESET_GUN_METAL_RED_SCALE 0.0f
#define PRESET_GUN_METAL_GREEN_SCALE 0.05f
#define PRESET_GUN_METAL_BLUE_SCALE 0.2f
#define PRESET_GUN_METAL_RED_OFFSET 0.0f
#define PRESET_GUN_METAL_GREEN_OFFSET 0.05f
#define PRESET_GUN_METAL_BLUE_OFFSET 0.0f
#define PRESET_GUN_METAL_VIGNETTE 0.25f

// define old movie post-processing preset values
#define PRESET_OLD_MOVIE_BRIGHTNESS 0.0f
#define PRESET_OLD_MOVIE_CONTRAST 1.05f
#define PRESET_OLD_MOVIE_SATURATION 0.0f
#define PRESET_OLD_MOVIE_RED_SCALE 0.0f
#define PRESET_OLD_MOVIE_GREEN_SCALE 0.0f
#define PRESET_OLD_MOVIE_BLUE_SCALE 0.07f
#define PRESET_OLD_MOVIE_RED_OFFSET 0.07f
#define PRESET_OLD_MOVIE_GREEN_OFFSET 0.03f
#define PRESET_OLD_MOVIE_BLUE_OFFSET 0.0f
#define PRESET_OLD_MOVIE_VIGNETTE 0.0f

// define black & white post-processing preset values
#define PRESET_BLACK_AND_WHITE_BRIGHTNESS -0.03f
#define PRESET_BLACK_AND_WHITE_CONTRAST 1.3f
#define PRESET_BLACK_AND_WHITE_SATURATION 0.0f
#define PRESET_BLACK_AND_WHITE_RED_SCALE 0.0f
#define PRESET_BLACK_AND_WHITE_GREEN_SCALE 0.0f
#define PRESET_BLACK_AND_WHITE_BLUE_SCALE 0.0f
#define PRESET_BLACK_AND_WHITE_RED_OFFSET 0.0f
#define PRESET_BLACK_AND_WHITE_GREEN_OFFSET 0.00f
#define PRESET_BLACK_AND_WHITE_BLUE_OFFSET 0.0f
#define PRESET_BLACK_AND_WHITE_VIGNETTE 0.65f

// define ansel adams post-processing preset values
#define PRESET_ANSEL_ADAMS_BRIGHTNESS -0.13f
#define PRESET_ANSEL_ADAMS_CONTRAST 1.2f
#define PRESET_ANSEL_ADAMS_SATURATION 0.0f
#define PRESET_ANSEL_ADAMS_RED_SCALE 0.0f
#define PRESET_ANSEL_ADAMS_GREEN_SCALE 0.0f
#define PRESET_ANSEL_ADAMS_BLUE_SCALE 0.0f
#define PRESET_ANSEL_ADAMS_RED_OFFSET 0.0f
#define PRESET_ANSEL_ADAMS_GREEN_OFFSET 0.00f
#define PRESET_ANSEL_ADAMS_BLUE_OFFSET 0.0f
#define PRESET_ANSEL_ADAMS_VIGNETTE 0.7f

// fragment-shader code
#define FRAGMENT_SHADER "#version 120\n"\
                        "const vec3 lumCoeff = vec3(0.2125, 0.7154, 0.0721);"\
                        "uniform float brightness;"\
                        "uniform float contrast;"\
                        "uniform float saturation;"\
                        "uniform float redScale;"\
                        "uniform float greenScale;"\
                        "uniform float blueScale;"\
                        "uniform float redOffset;"\
                        "uniform float greenOffset;"\
                        "uniform float blueOffset;"\
                        "uniform vec2 resolution;"\
                        "uniform float vignette;"\
                        "uniform sampler2D scene;"\
                        "void main()"\
                        "{"\
                            "vec3 color = texture2D(scene, gl_TexCoord[0].st).rgb;"\
                            "color *= contrast;"\
                            "color += vec3(brightness, brightness, brightness);"\
                            "vec3 intensity = vec3(dot(color, lumCoeff));"\
                            "color = mix(intensity, color, saturation);"\
                            "vec3 newColor = (color.rgb - 0.5) * 2.0;"\
                            "newColor.r = 2.0 / 3.0 * (1.0 - (newColor.r * newColor.r));"\
                            "newColor.g = 2.0 / 3.0 * (1.0 - (newColor.g * newColor.g));"\
                            "newColor.b = 2.0 / 3.0 * (1.0 - (newColor.b * newColor.b));"\
                            "newColor.r = clamp(color.r + redScale * newColor.r + redOffset, 0.0, 1.0);"\
                            "newColor.g = clamp(color.g + greenScale * newColor.g + greenOffset, 0.0, 1.0);"\
                            "newColor.b = clamp(color.b + blueScale * newColor.b + blueOffset, 0.0, 1.0);"\
                            "color = newColor;"\
                            "vec2 position = (gl_FragCoord.xy / resolution.xy) - vec2(0.5);"\
                            "float len = length(position);"\
                            "float vig = smoothstep(0.75, 0.75 - 0.45, len);"\
                            "color = mix(color, color * vig, vignette);"\
                            "gl_FragColor = vec4(color, 1.0);"\
                        "}"

// global settings variables
static int postProcesssingEnabled = DEFAULT_POST_PROCESSING_ENABLED, fpsLimiterEnabled = DEFAULT_FPS_LIMITER_ENABLED, controlCinemaVeriteEnabled = DEFAULT_CONTROL_CINEMA_VERITE_ENABLED;
static float brightness = DEFAULT_BRIGHTNESS, contrast = DEFAULT_CONTRAST, saturation = DEFAULT_SATURATION, redScale = DEFAULT_RED_SCALE, greenScale = DEFAULT_GREEN_SCALE, blueScale = DEFAULT_BLUE_SCALE, redOffset = DEFAULT_RED_OFFSET, greenOffset = DEFAULT_GREEN_OFFSET, blueOffset = DEFAULT_BLUE_OFFSET, vignette = DEFAULT_VIGNETTE, maxFps = DEFAULT_MAX_FRAME_RATE, disableCinemaVeriteTime = DEFAULT_DISABLE_CINEMA_VERITE_TIME;

// global internal variables
static int lastMouseX = 0, lastMouseY = 0, lastResolutionX = 0, lastResolutionY = 0;
static GLuint textureId = 0, program = 0, fragmentShader = 0;
static float startTimeFlight = 0.0f, endTimeFlight = 0.0f, startTimeDraw = 0.0f, endTimeDraw = 0.0f, lastMouseMovementTime = 0.0f;
#if LIN
static Display *display = NULL;
#endif

// global dataref variables
static XPLMDataRef cinemaVeriteDataRef, viewTypeDataRef;

// global widget variables
static XPWidgetID settingsWidget, postProcessingCheckbox, fpsLimiterCheckbox, controlCinemaVeriteCheckbox, brightnessCaption, contrastCaption, saturationCaption, redScaleCaption, greenScaleCaption, blueScaleCaption, redOffsetCaption, greenOffsetCaption, blueOffsetCaption, vignetteCaption, maxFpsCaption, disableCinemaVeriteTimeCaption, brightnessSlider, contrastSlider, saturationSlider, redScaleSlider, greenScaleSlider, blueScaleSlider, redOffsetSlider, greenOffsetSlider, blueOffsetSlider, vignetteSlider, maxFpsSlider, disableCinemaVeriteTimeSlider, resetButton, polarizedPresetButton, crazyHazyPresetButton, hdrIshPresetButton, negativeDrabPresetButton, extraNormalPresetButton, shadowhancerPresetButton, redShiftPresetButton, greenShiftPresetButton, blueShiftPresetButton, sunnyFloridaPresetButton, mojaveDesertPresentButton, winterSpringsPresetButton, vividDreamsPresetButton, technoColorPresetButton, gunMetalPresetButton, oldMoviePresetButton, blackAndWhitePresetButton, anselAdamsPresetButton;

// draw-callback that adds post-processing
static int PostProcessingCallback(
                                  XPLMDrawingPhase     inPhase,
                                  int                  inIsBefore,
                                  void *               inRefcon)
{
    int x, y;
	XPLMGetScreenSize(&x, &y);
    
	if(textureId == 0 || lastResolutionX != x || lastResolutionY != y)
	{
		XPLMGenerateTextureNumbers((int *) &textureId, 1);
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        lastResolutionX = x;
        lastResolutionY = y;
	}
    else
    {
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, textureId);
    }
    
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, x, y);
	XPLMSetGraphicsState(0, 1, 0, 0, 0,  0, 0);
    
    glUseProgram(program);
    
    int brightnessLocation = glGetUniformLocation(program, "brightness");
    glUniform1f(brightnessLocation, brightness);
    
    int contrastLocation = glGetUniformLocation(program, "contrast");
    glUniform1f(contrastLocation, contrast);
    
    int saturationLocation = glGetUniformLocation(program, "saturation");
    glUniform1f(saturationLocation, saturation);
    
    int redScaleLocation = glGetUniformLocation(program, "redScale");
    glUniform1f(redScaleLocation, redScale);
    
    int greenScaleLocation = glGetUniformLocation(program, "greenScale");
    glUniform1f(greenScaleLocation, greenScale);
    
    int blueScaleLocation = glGetUniformLocation(program, "blueScale");
    glUniform1f(blueScaleLocation, blueScale);
    
    int redOffsetLocation = glGetUniformLocation(program, "redOffset");
    glUniform1f(redOffsetLocation, redOffset);
    
    int greenOffsetLocation = glGetUniformLocation(program, "greenOffset");
    glUniform1f(greenOffsetLocation, greenOffset);
    
    int blueOffsetLocation = glGetUniformLocation(program, "blueOffset");
    glUniform1f(blueOffsetLocation, blueOffset);
    
    int resolutionLocation = glGetUniformLocation(program, "resolution");
    glUniform2f(resolutionLocation, (float) x, (float) y);
    
    int vignetteLocation = glGetUniformLocation(program, "vignette");
    glUniform1f(vignetteLocation, vignette);
    
    int sceneLocation = glGetUniformLocation(program, "scene");
    glUniform1i(sceneLocation, 0);
    
	glPushAttrib(GL_VIEWPORT_BIT);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0f, x, 0.0f, y, -1.0f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glViewport(0, 0, x, y);
    
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
    int settingsWindowOpen = XPIsWidgetVisible(settingsWidget);
	glTexCoord2f(((settingsWindowOpen == 0) ? 0.0f : 0.5f), 0.0f);
    glVertex2f(((settingsWindowOpen == 0) ? 0.0f : (GLfloat) (x / 2.0f)), 0.0f);
	glTexCoord2f(((settingsWindowOpen == 0) ? 0.0f : 0.5f), 1.0f);
    glVertex2f(((settingsWindowOpen == 0) ? 0.0f : (GLfloat) (x / 2.0f)), (GLfloat) y);
	glTexCoord2f(1.0f, 1.0f);
    glVertex2f((GLfloat) x, (GLfloat) y);
	glTexCoord2f(1.0f, 0.0f);
    glVertex2f((GLfloat) x, 0.0f);
	glEnd();
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
    
    glUseProgram(0);
	
	return 1;
}

// flightloop-callback that limits the number of flightcycles
float LimiterFlightCallback(
                            float                inElapsedSinceLastCall,
                            float                inElapsedTimeSinceLastFlightLoop,
                            int                  inCounter,
                            void *               inRefcon)
{
    endTimeFlight = XPLMGetElapsedTime();
    float dt = endTimeFlight - startTimeFlight;
    
    float t = 1.0f / maxFps - dt;
    
    if(t > 0.0f)
#if IBM
		Sleep((DWORD) (t * 1000.0f));
#else
        usleep((useconds_t) (t * 1000000.0f));
#endif
    
    startTimeFlight = XPLMGetElapsedTime();
    
    return -1.0f;
}

// draw-callback that limits the number of drawcycles
static int LimiterDrawCallback(
                               XPLMDrawingPhase     inPhase,
                               int                  inIsBefore,
                               void *               inRefcon)
{
    endTimeDraw = XPLMGetElapsedTime();
    float dt = endTimeDraw - startTimeDraw;
    
    float t = 1.0f / maxFps - dt;
    
    if(t > 0.0f)
#if IBM
		Sleep((DWORD) (t * 1000.0f));
#else
        usleep((useconds_t) (t * 1000000.0f));
#endif
    
    startTimeDraw = XPLMGetElapsedTime();
    
    return 1;
}

// flightloop-callback that auto-controls cinema-verite
float ControlCinemaVeriteCallback(
                                  float                inElapsedSinceLastCall,
                                  float                inElapsedTimeSinceLastFlightLoop,
                                  int                  inCounter,
                                  void *               inRefcon)
{
#if APL
    int mouseButtonDown = CGEventSourceButtonState(kCGEventSourceStateCombinedSessionState, kCGMouseButtonLeft);
#elif IBM
    // if the most significant bit is set, the key is down
    SHORT state = GetAsyncKeyState(VK_LBUTTON);
    SHORT msb = state >> 15;
    int mouseButtonDown = (int) msb;
#elif LIN
    if (display == NULL)
        display = XOpenDisplay(NULL);
    Window root, child;
    int rootX, rootY, winX, winY;
    unsigned int mask;
    XQueryPointer(display, DefaultRootWindow(display), &root, &child, &rootX, &rootY, &winX, &winY, &mask);
    int mouseButtonDown = (mask & Button1Mask) >> 8;
#endif
    
    int currentMouseX, currentMouseY;
    XPLMGetMouseLocation(&currentMouseX, &currentMouseY);
    
    if (mouseButtonDown == 1 || currentMouseX != lastMouseX || currentMouseY != lastMouseY) {
        lastMouseX = currentMouseX;
        lastMouseY = currentMouseY;
        
        lastMouseMovementTime = XPLMGetElapsedTime();
    }
    
    int viewType = XPLMGetDatai(viewTypeDataRef);
    
    if (viewType != 1026) // not 3D Cockpit+
        XPLMSetDatai(cinemaVeriteDataRef, 1);
    else
    {
        float currentTime = XPLMGetElapsedTime();
        float elapsedTime = currentTime - lastMouseMovementTime;
        
        if (mouseButtonDown || elapsedTime <= disableCinemaVeriteTime)
            XPLMSetDatai(cinemaVeriteDataRef, 0);
        else
            XPLMSetDatai(cinemaVeriteDataRef, 1);
    }
    
    return -1.0f;
}

// removes the fragment-shader from video memory, if deleteProgram is set the shader-program is also removed
void CleanupShader(int deleteProgram = 0)
{
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);
    
    if (deleteProgram != 0)
        glDeleteProgram(program);
}

// function to load, compile and link the fragment-shader
void InitShader(const char *fragmentShaderString)
{
    program = glCreateProgram();
    
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderString, 0);
    glCompileShader(fragmentShader);
    glAttachShader(program, fragmentShader);
    GLint isFragmentShaderCompiled = GL_FALSE;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isFragmentShaderCompiled);
    if (isFragmentShaderCompiled == GL_FALSE) {
        GLsizei maxLength = 2048;
        GLchar *log = new GLchar[maxLength];
        glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, log);
        XPLMDebugString(NAME": The following error occured while compiling the fragment shader:\n");
        XPLMDebugString(log);
		delete[] log;
        
        CleanupShader(1);
        
        return;
    }
    
    glLinkProgram(program);
    GLint isProgramLinked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &isProgramLinked);
    if (isProgramLinked == GL_FALSE) {
        GLsizei maxLength = 2048;
        GLchar *log = new GLchar[maxLength];
        glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, log);
        XPLMDebugString(NAME": The following error occured while linking the shader program:\n");
        XPLMDebugString(log);
		delete[] log;
        
        CleanupShader(1);
        
        return;
    }
    
    CleanupShader(0);
}

// returns a float rounded to two decimal places
float Round(const float f)
{
    return ((int) (f * 100.0f)) / 100.0f;
}

// updates all caption widgets and slider positions associated with settings variables
void UpdateSettingsWidgets(void)
{
    XPSetWidgetProperty(postProcessingCheckbox, xpProperty_ButtonState, postProcesssingEnabled);
    XPSetWidgetProperty(fpsLimiterCheckbox, xpProperty_ButtonState, fpsLimiterEnabled);
    XPSetWidgetProperty(controlCinemaVeriteCheckbox, xpProperty_ButtonState, controlCinemaVeriteEnabled);
    
    char stringBrightness[32];
    sprintf(stringBrightness, "Brightness: %.2f", brightness);
    XPSetWidgetDescriptor(brightnessCaption, stringBrightness);
    
    char stringContrast[32];
    sprintf(stringContrast, "Contrast: %.2f", contrast);
    XPSetWidgetDescriptor(contrastCaption, stringContrast);
    
    char stringSaturation[32];
    sprintf(stringSaturation, "Saturation: %.2f", saturation);
    XPSetWidgetDescriptor(saturationCaption, stringSaturation);
    
    char stringRedScale[32];
    sprintf(stringRedScale, "Red Scale: %.2f", redScale);
    XPSetWidgetDescriptor(redScaleCaption, stringRedScale);
    
    char stringGreenScale[32];
    sprintf(stringGreenScale, "Green Scale: %.2f", greenScale);
    XPSetWidgetDescriptor(greenScaleCaption, stringGreenScale);
    
    char stringBlueScale[32];
    sprintf(stringBlueScale, "Blue Scale: %.2f", blueScale);
    XPSetWidgetDescriptor(blueScaleCaption, stringBlueScale);
    
    char stringRedOffset[32];
    sprintf(stringRedOffset, "Red Offset: %.2f", redOffset);
    XPSetWidgetDescriptor(redOffsetCaption, stringRedOffset);
    
    char stringGreenOffset[32];
    sprintf(stringGreenOffset, "Green Offset: %.2f", greenOffset);
    XPSetWidgetDescriptor(greenOffsetCaption, stringGreenOffset);
    
    char stringBlueOffset[32];
    sprintf(stringBlueOffset, "Blue Offset: %.2f", blueOffset);
    XPSetWidgetDescriptor(blueOffsetCaption, stringBlueOffset);
    
    char stringVignette[32];
    sprintf(stringVignette, "Vignette: %.2f", vignette);
    XPSetWidgetDescriptor(vignetteCaption, stringVignette);
    
    char stringMaxFps[32];
    sprintf(stringMaxFps, "Max FPS: %.0f", maxFps);
    XPSetWidgetDescriptor(maxFpsCaption, stringMaxFps);
    
    char stringDisableCinemaVeriteTime[32];
    sprintf(stringDisableCinemaVeriteTime, "On input disable for: %.0f sec", disableCinemaVeriteTime);
    XPSetWidgetDescriptor(disableCinemaVeriteTimeCaption, stringDisableCinemaVeriteTime);
    
   	XPSetWidgetProperty(brightnessSlider, xpProperty_ScrollBarSliderPosition, (intptr_t) (brightness * 1000.0f));
   	XPSetWidgetProperty(contrastSlider, xpProperty_ScrollBarSliderPosition, (intptr_t) (contrast * 100.0f));
    XPSetWidgetProperty(saturationSlider, xpProperty_ScrollBarSliderPosition, (intptr_t) (saturation * 100.0f));
    XPSetWidgetProperty(redScaleSlider, xpProperty_ScrollBarSliderPosition, (intptr_t) (redScale * 100.0f));
    XPSetWidgetProperty(greenScaleSlider, xpProperty_ScrollBarSliderPosition, (intptr_t) (greenScale * 100.0f));
    XPSetWidgetProperty(blueScaleSlider, xpProperty_ScrollBarSliderPosition, (intptr_t) (blueScale * 100.0f));
    XPSetWidgetProperty(redOffsetSlider, xpProperty_ScrollBarSliderPosition, (intptr_t) (redOffset * 100.0f));
    XPSetWidgetProperty(greenOffsetSlider, xpProperty_ScrollBarSliderPosition, (intptr_t) (greenOffset * 100.0f));
	XPSetWidgetProperty(blueOffsetSlider, xpProperty_ScrollBarSliderPosition, (intptr_t) (blueOffset * 100.0f));
    XPSetWidgetProperty(vignetteSlider, xpProperty_ScrollBarSliderPosition, (intptr_t) (vignette * 100.0f));
    XPSetWidgetProperty(maxFpsSlider, xpProperty_ScrollBarSliderPosition, (intptr_t) (maxFps));
    XPSetWidgetProperty(disableCinemaVeriteTimeSlider, xpProperty_ScrollBarSliderPosition, (intptr_t) (disableCinemaVeriteTime));
}

// saves current settings to the config file
void SaveSettings(void)
{
    std::fstream file;
    file.open(CONFIG_PATH, std::ios_base::out | std::ios_base::trunc);
    
    if(file.is_open())
    {        
        file << "postProcesssingEnabled=" << postProcesssingEnabled << std::endl;
        file << "fpsLimiterEnabled=" << fpsLimiterEnabled << std::endl;
        file << "controlCinemaVeriteEnabled=" << controlCinemaVeriteEnabled << std::endl;
        file << "brightness=" << brightness << std::endl;
        file << "contrast=" << contrast << std::endl;
        file << "saturation=" << saturation << std::endl;
        file << "redScale=" << redScale << std::endl;
        file << "greenScale=" << greenScale << std::endl;
        file << "blueScale=" << blueScale << std::endl;
        file << "redOffset=" << redOffset << std::endl;
        file << "greenOffset=" << greenOffset << std::endl;
        file << "blueOffset=" << blueOffset << std::endl;
        file << "vignette=" << vignette << std::endl;
        file << "maxFps=" << maxFps << std::endl;
        file << "disableCinemaVeriteTime=" << disableCinemaVeriteTime << std::endl;
        
        file.close();
    }
}

// loads settings from the config file
void LoadSettings(void)
{
    std::ifstream file;
    file.open(CONFIG_PATH);
    
	if(file.is_open())
	{
        std::string line;
        
		while(getline(file, line))
		{
            std::string val = line.substr(line.find("=") + 1);
            std::istringstream iss(val);
            
			if(line.find("postProcesssingEnabled") != -1)
				iss >> postProcesssingEnabled;
			else if(line.find("fpsLimiterEnabled") != -1)
				iss >> fpsLimiterEnabled;
			else if(line.find("controlCinemaVeriteEnabled") != -1)
				iss >> controlCinemaVeriteEnabled;
			else if(line.find("brightness") != -1)
				iss >> brightness;
			else if(line.find("contrast") != -1)
				iss >> contrast;
			else if(line.find("saturation") != -1)
				iss >> saturation;
			else if(line.find("redScale") != -1)
				iss >> redScale;
			else if(line.find("greenScale") != -1)
				iss >> greenScale;
            else if(line.find("blueScale") != -1)
				iss >> blueScale;
            else if(line.find("redOffset") != -1)
				iss >> redOffset;
            else if(line.find("greenOffset") != -1)
				iss >> greenOffset;
            else if(line.find("blueOffset") != -1)
				iss >> blueOffset;
            else if(line.find("vignette") != -1)
				iss >> vignette;
            else if(line.find("maxFps") != -1)
				iss >> maxFps;
            else if(line.find("disableCinemaVeriteTime") != -1)
				iss >> disableCinemaVeriteTime;
		}
        
		file.close();
	}
}

// handles the settings widget
int SettingsWidgetHandler(XPWidgetMessage inMessage, XPWidgetID inWidget, long inParam1, long inParam2)
{
	if (inMessage == xpMessage_CloseButtonPushed)
	{
		if (XPIsWidgetVisible(settingsWidget))
		{
            SaveSettings();
			XPHideWidget(settingsWidget);
		}
	}
    else if (inMessage == xpMsg_ButtonStateChanged)
    {
        if (inParam1 == (long) postProcessingCheckbox)
        {
            postProcesssingEnabled = (int) XPGetWidgetProperty(postProcessingCheckbox, xpProperty_ButtonState, 0);
            
            if (postProcesssingEnabled == 0)
                XPLMUnregisterDrawCallback(PostProcessingCallback, xplm_Phase_Window, 1, NULL);
            else
                XPLMRegisterDrawCallback(PostProcessingCallback, xplm_Phase_Window, 1, NULL);
            
        }
        else if (inParam1 == (long) fpsLimiterCheckbox)
        {
            fpsLimiterEnabled = (int) XPGetWidgetProperty(fpsLimiterCheckbox, xpProperty_ButtonState, 0);
            
            if (fpsLimiterEnabled == 0)
            {
                XPLMUnregisterFlightLoopCallback(LimiterFlightCallback, NULL);
                XPLMUnregisterDrawCallback(LimiterDrawCallback, xplm_Phase_Terrain, 1, NULL);
            }
            else
            {
                XPLMRegisterFlightLoopCallback(LimiterFlightCallback, -1, NULL);
                XPLMRegisterDrawCallback(LimiterDrawCallback, xplm_Phase_Terrain, 1, NULL);
            }
            
        }
        else if (inParam1 == (long) controlCinemaVeriteCheckbox)
        {
            controlCinemaVeriteEnabled = (int) XPGetWidgetProperty(controlCinemaVeriteCheckbox, xpProperty_ButtonState, 0);
            
            if (controlCinemaVeriteEnabled == 0)
                XPLMUnregisterFlightLoopCallback(ControlCinemaVeriteCallback, NULL);
            else
                XPLMRegisterFlightLoopCallback(ControlCinemaVeriteCallback, -1, NULL);
            
        }
    }
	else if (inMessage == xpMsg_ScrollBarSliderPositionChanged)
	{
        if (inParam1 == (long) brightnessSlider)
            brightness = Round(XPGetWidgetProperty(brightnessSlider, xpProperty_ScrollBarSliderPosition, 0) / 1000.0f);
        else if (inParam1 == (long) contrastSlider)
            contrast = Round(XPGetWidgetProperty(contrastSlider, xpProperty_ScrollBarSliderPosition, 0) / 100.0f);
        else if (inParam1 == (long) saturationSlider)
            saturation = Round(XPGetWidgetProperty(saturationSlider, xpProperty_ScrollBarSliderPosition, 0) / 100.0f);
        else if (inParam1 == (long) redScaleSlider)
            redScale = Round(XPGetWidgetProperty(redScaleSlider, xpProperty_ScrollBarSliderPosition, 0) / 100.0f);
        else if (inParam1 == (long) greenScaleSlider)
            greenScale = Round(XPGetWidgetProperty(greenScaleSlider, xpProperty_ScrollBarSliderPosition, 0) / 100.0f);
        else if (inParam1 == (long) blueScaleSlider)
            blueScale = Round(XPGetWidgetProperty(blueScaleSlider, xpProperty_ScrollBarSliderPosition, 0) / 100.0f);
        else if (inParam1 == (long) redOffsetSlider)
            redOffset = Round(XPGetWidgetProperty(redOffsetSlider, xpProperty_ScrollBarSliderPosition, 0) / 100.0f);
        else if (inParam1 == (long) greenOffsetSlider)
            greenOffset = Round(XPGetWidgetProperty(greenOffsetSlider, xpProperty_ScrollBarSliderPosition, 0) / 100.0f);
        else if (inParam1 == (long) blueOffsetSlider)
            blueOffset = Round(XPGetWidgetProperty(blueOffsetSlider, xpProperty_ScrollBarSliderPosition, 0) / 100.0f);
        else if (inParam1 == (long) vignetteSlider)
            vignette = Round(XPGetWidgetProperty(vignetteSlider, xpProperty_ScrollBarSliderPosition, 0) / 100.0f);
        else if (inParam1 == (long) maxFpsSlider)
            maxFps = (float) (int) XPGetWidgetProperty(maxFpsSlider, xpProperty_ScrollBarSliderPosition, 0);
        else if (inParam1 == (long) disableCinemaVeriteTimeSlider)
            disableCinemaVeriteTime = (float) (int) XPGetWidgetProperty(disableCinemaVeriteTimeSlider, xpProperty_ScrollBarSliderPosition, 0);
        
        UpdateSettingsWidgets();
	}
    else if (inMessage == xpMsg_PushButtonPressed)
    {
        if (inParam1 == (long) resetButton)
        {
            brightness = DEFAULT_BRIGHTNESS;
            contrast = DEFAULT_CONTRAST;
            saturation = DEFAULT_SATURATION;
            redScale = DEFAULT_RED_SCALE;
            greenScale = DEFAULT_GREEN_SCALE;
            blueScale = DEFAULT_BLUE_SCALE;
            redOffset = DEFAULT_RED_OFFSET;
            greenOffset = DEFAULT_GREEN_OFFSET;
            blueOffset = DEFAULT_BLUE_OFFSET;
            vignette = DEFAULT_VIGNETTE;
        }
        else if (inParam1 == (long) polarizedPresetButton)
        {
            brightness = PRESET_POLARIZED_BRIGHTNESS;
            contrast = PRESET_POLARIZED_CONTRAST;
            saturation = PRESET_POLARIZED_SATURATION;
            redScale = PRESET_POLARIZED_RED_SCALE;
            greenScale = PRESET_POLARIZED_GREEN_SCALE;
            blueScale = PRESET_POLARIZED_BLUE_SCALE;
            redOffset = PRESET_POLARIZED_RED_OFFSET;
            greenOffset = PRESET_POLARIZED_GREEN_OFFSET;
            blueOffset = PRESET_POLARIZED_BLUE_OFFSET;
            vignette = PRESET_POLARIZED_VIGNETTE;
        }
        else if (inParam1 == (long) crazyHazyPresetButton)
        {
            brightness = PRESET_CRAZY_HAZY_BRIGHTNESS;
            contrast = PRESET_CRAZY_HAZY_CONTRAST;
            saturation = PRESET_CRAZY_HAZY_SATURATION;
            redScale = PRESET_CRAZY_HAZY_RED_SCALE;
            greenScale = PRESET_CRAZY_HAZY_GREEN_SCALE;
            blueScale = PRESET_CRAZY_HAZY_BLUE_SCALE;
            redOffset = PRESET_CRAZY_HAZY_RED_OFFSET;
            greenOffset = PRESET_CRAZY_HAZY_GREEN_OFFSET;
            blueOffset = PRESET_CRAZY_HAZY_BLUE_OFFSET;
            vignette = PRESET_CRAZY_HAZY_VIGNETTE;
        }
        else if (inParam1 == (long) hdrIshPresetButton)
        {
            brightness = PRESET_HDR_ISH_BRIGHTNESS;
            contrast = PRESET_HDR_ISH_CONTRAST;
            saturation = PRESET_HDR_ISH_SATURATION;
            redScale = PRESET_HDR_ISH_RED_SCALE;
            greenScale = PRESET_HDR_ISH_GREEN_SCALE;
            blueScale = PRESET_HDR_ISH_BLUE_SCALE;
            redOffset = PRESET_HDR_ISH_RED_OFFSET;
            greenOffset = PRESET_HDR_ISH_GREEN_OFFSET;
            blueOffset = PRESET_HDR_ISH_BLUE_OFFSET;
            vignette = PRESET_HDR_ISH_VIGNETTE;
        }
        else if (inParam1 == (long) negativeDrabPresetButton)
        {
            brightness = PRESET_NEGATIVE_DRAB_BRIGHTNESS;
            contrast = PRESET_NEGATIVE_DRAB_CONTRAST;
            saturation = PRESET_NEGATIVE_DRAB_SATURATION;
            redScale = PRESET_NEGATIVE_DRAB_RED_SCALE;
            greenScale = PRESET_NEGATIVE_DRAB_GREEN_SCALE;
            blueScale = PRESET_NEGATIVE_DRAB_BLUE_SCALE;
            redOffset = PRESET_NEGATIVE_DRAB_RED_OFFSET;
            greenOffset = PRESET_NEGATIVE_DRAB_GREEN_OFFSET;
            blueOffset = PRESET_NEGATIVE_DRAB_BLUE_OFFSET;
            vignette = PRESET_NEGATIVE_DRAB_VIGNETTE;
        }
        else if (inParam1 == (long) extraNormalPresetButton)
        {
            brightness = PRESET_EXTRA_NORMAL_BRIGHTNESS;
            contrast = PRESET_EXTRA_NORMAL_CONTRAST;
            saturation = PRESET_EXTRA_NORMAL_SATURATION;
            redScale = PRESET_EXTRA_NORMAL_RED_SCALE;
            greenScale = PRESET_EXTRA_NORMAL_GREEN_SCALE;
            blueScale = PRESET_EXTRA_NORMAL_BLUE_SCALE;
            redOffset = PRESET_EXTRA_NORMAL_RED_OFFSET;
            greenOffset = PRESET_EXTRA_NORMAL_GREEN_OFFSET;
            blueOffset = PRESET_EXTRA_NORMAL_BLUE_OFFSET;
            vignette = PRESET_EXTRA_NORMAL_VIGNETTE;
        }
        else if (inParam1 == (long) shadowhancerPresetButton)
        {
            brightness = PRESET_SHADOWHANCER_BRIGHTNESS;
            contrast = PRESET_SHADOWHANCER_CONTRAST;
            saturation = PRESET_SHADOWHANCER_SATURATION;
            redScale = PRESET_SHADOWHANCER_RED_SCALE;
            greenScale = PRESET_SHADOWHANCER_GREEN_SCALE;
            blueScale = PRESET_SHADOWHANCER_BLUE_SCALE;
            redOffset = PRESET_SHADOWHANCER_RED_OFFSET;
            greenOffset = PRESET_SHADOWHANCER_GREEN_OFFSET;
            blueOffset = PRESET_SHADOWHANCER_BLUE_OFFSET;
            vignette = PRESET_SHADOWHANCER_VIGNETTE;
        }
        else if (inParam1 == (long) redShiftPresetButton)
        {
            brightness = PRESET_RED_SHIFT_BRIGHTNESS;
            contrast = PRESET_RED_SHIFT_CONTRAST;
            saturation = PRESET_RED_SHIFT_SATURATION;
            redScale = PRESET_RED_SHIFT_RED_SCALE;
            greenScale = PRESET_RED_SHIFT_GREEN_SCALE;
            blueScale = PRESET_RED_SHIFT_BLUE_SCALE;
            redOffset = PRESET_RED_SHIFT_RED_OFFSET;
            greenOffset = PRESET_RED_SHIFT_GREEN_OFFSET;
            blueOffset = PRESET_RED_SHIFT_BLUE_OFFSET;
            vignette = PRESET_RED_SHIFT_VIGNETTE;
        }
        else if (inParam1 == (long) greenShiftPresetButton)
        {
            brightness = PRESET_GREEN_SHIFT_BRIGHTNESS;
            contrast = PRESET_GREEN_SHIFT_CONTRAST;
            saturation = PRESET_GREEN_SHIFT_SATURATION;
            redScale = PRESET_GREEN_SHIFT_RED_SCALE;
            greenScale = PRESET_GREEN_SHIFT_GREEN_SCALE;
            blueScale = PRESET_GREEN_SHIFT_BLUE_SCALE;
            redOffset = PRESET_GREEN_SHIFT_RED_OFFSET;
            greenOffset = PRESET_GREEN_SHIFT_GREEN_OFFSET;
            blueOffset = PRESET_GREEN_SHIFT_BLUE_OFFSET;
            vignette = PRESET_GREEN_SHIFT_VIGNETTE;
        }
        else if (inParam1 == (long) blueShiftPresetButton)
        {
            brightness = PRESET_BLUE_SHIFT_BRIGHTNESS;
            contrast = PRESET_BLUE_SHIFT_CONTRAST;
            saturation = PRESET_BLUE_SHIFT_SATURATION;
            redScale = PRESET_BLUE_SHIFT_RED_SCALE;
            greenScale = PRESET_BLUE_SHIFT_GREEN_SCALE;
            blueScale = PRESET_BLUE_SHIFT_BLUE_SCALE;
            redOffset = PRESET_BLUE_SHIFT_RED_OFFSET;
            greenOffset = PRESET_BLUE_SHIFT_GREEN_OFFSET;
            blueOffset = PRESET_BLUE_SHIFT_BLUE_OFFSET;
            vignette = PRESET_BLUE_SHIFT_VIGNETTE;
        }
        else if (inParam1 == (long) sunnyFloridaPresetButton)
        {
            brightness = PRESET_SUNNY_FLORIDA_BRIGHTNESS;
            contrast = PRESET_SUNNY_FLORIDA_CONTRAST;
            saturation = PRESET_SUNNY_FLORIDA_SATURATION;
            redScale = PRESET_SUNNY_FLORIDA_RED_SCALE;
            greenScale = PRESET_SUNNY_FLORIDA_GREEN_SCALE;
            blueScale = PRESET_SUNNY_FLORIDA_BLUE_SCALE;
            redOffset = PRESET_SUNNY_FLORIDA_RED_OFFSET;
            greenOffset = PRESET_SUNNY_FLORIDA_GREEN_OFFSET;
            blueOffset = PRESET_SUNNY_FLORIDA_BLUE_OFFSET;
            vignette = PRESET_SUNNY_FLORIDA_VIGNETTE;
        }
        else if (inParam1 == (long) mojaveDesertPresentButton)
        {
            brightness = PRESET_MOJAVE_DESERT_BRIGHTNESS;
            contrast = PRESET_MOJAVE_DESERT_CONTRAST;
            saturation = PRESET_MOJAVE_DESERT_SATURATION;
            redScale = PRESET_MOJAVE_DESERT_RED_SCALE;
            greenScale = PRESET_MOJAVE_DESERT_GREEN_SCALE;
            blueScale = PRESET_MOJAVE_DESERT_BLUE_SCALE;
            redOffset = PRESET_MOJAVE_DESERT_RED_OFFSET;
            greenOffset = PRESET_MOJAVE_DESERT_GREEN_OFFSET;
            blueOffset = PRESET_MOJAVE_DESERT_BLUE_OFFSET;
            vignette = PRESET_MOJAVE_DESERT_VIGNETTE;
        }
        else if (inParam1 == (long) winterSpringsPresetButton)
        {
            brightness = PRESET_WINTER_SPRINGS_BRIGHTNESS;
            contrast = PRESET_WINTER_SPRINGS_CONTRAST;
            saturation = PRESET_WINTER_SPRINGS_SATURATION;
            redScale = PRESET_WINTER_SPRINGS_RED_SCALE;
            greenScale = PRESET_WINTER_SPRINGS_GREEN_SCALE;
            blueScale = PRESET_WINTER_SPRINGS_BLUE_SCALE;
            redOffset = PRESET_WINTER_SPRINGS_RED_OFFSET;
            greenOffset = PRESET_WINTER_SPRINGS_GREEN_OFFSET;
            blueOffset = PRESET_WINTER_SPRINGS_BLUE_OFFSET;
            vignette = PRESET_WINTER_SPRINGS_VIGNETTE;
        }
        else if (inParam1 == (long) vividDreamsPresetButton)
        {
            brightness = PRESET_VIVID_DREAMS_BRIGHTNESS;
            contrast = PRESET_VIVID_DREAMS_CONTRAST;
            saturation = PRESET_VIVID_DREAMS_SATURATION;
            redScale = PRESET_VIVID_DREAMS_RED_SCALE;
            greenScale = PRESET_VIVID_DREAMS_GREEN_SCALE;
            blueScale = PRESET_VIVID_DREAMS_BLUE_SCALE;
            redOffset = PRESET_VIVID_DREAMS_RED_OFFSET;
            greenOffset = PRESET_VIVID_DREAMS_GREEN_OFFSET;
            blueOffset = PRESET_VIVID_DREAMS_BLUE_OFFSET;
            vignette = PRESET_VIVID_DREAMS_VIGNETTE;
        }
        else if (inParam1 == (long) technoColorPresetButton)
        {
            brightness = PRESET_TECHNOCOLOR_BRIGHTNESS;
            contrast = PRESET_TECHNOCOLOR_CONTRAST;
            saturation = PRESET_TECHNOCOLOR_SATURATION;
            redScale = PRESET_TECHNOCOLOR_RED_SCALE;
            greenScale = PRESET_TECHNOCOLOR_GREEN_SCALE;
            blueScale = PRESET_TECHNOCOLOR_BLUE_SCALE;
            redOffset = PRESET_TECHNOCOLOR_RED_OFFSET;
            greenOffset = PRESET_TECHNOCOLOR_GREEN_OFFSET;
            blueOffset = PRESET_TECHNOCOLOR_BLUE_OFFSET;
            vignette = PRESET_TECHNOCOLOR_VIGNETTE;
        }
        else if (inParam1 == (long) gunMetalPresetButton)
        {
            brightness = PRESET_GUN_METAL_BRIGHTNESS;
            contrast = PRESET_GUN_METAL_CONTRAST;
            saturation = PRESET_GUN_METAL_SATURATION;
            redScale = PRESET_GUN_METAL_RED_SCALE;
            greenScale = PRESET_GUN_METAL_GREEN_SCALE;
            blueScale = PRESET_GUN_METAL_BLUE_SCALE;
            redOffset = PRESET_GUN_METAL_RED_OFFSET;
            greenOffset = PRESET_GUN_METAL_GREEN_OFFSET;
            blueOffset = PRESET_GUN_METAL_BLUE_OFFSET;
            vignette = PRESET_GUN_METAL_VIGNETTE;
        }
        else if (inParam1 == (long) oldMoviePresetButton)
        {
            brightness = PRESET_OLD_MOVIE_BRIGHTNESS;
            contrast = PRESET_OLD_MOVIE_CONTRAST;
            saturation = PRESET_OLD_MOVIE_SATURATION;
            redScale = PRESET_OLD_MOVIE_RED_SCALE;
            greenScale = PRESET_OLD_MOVIE_GREEN_SCALE;
            blueScale = PRESET_OLD_MOVIE_BLUE_SCALE;
            redOffset = PRESET_OLD_MOVIE_RED_OFFSET;
            greenOffset = PRESET_OLD_MOVIE_GREEN_OFFSET;
            blueOffset = PRESET_OLD_MOVIE_BLUE_OFFSET;
            vignette = PRESET_OLD_MOVIE_VIGNETTE;
        }
        else if (inParam1 == (long) blackAndWhitePresetButton)
        {
            brightness = PRESET_BLACK_AND_WHITE_BRIGHTNESS;
            contrast = PRESET_BLACK_AND_WHITE_CONTRAST;
            saturation = PRESET_BLACK_AND_WHITE_SATURATION;
            redScale = PRESET_BLACK_AND_WHITE_RED_SCALE;
            greenScale = PRESET_BLACK_AND_WHITE_GREEN_SCALE;
            blueScale = PRESET_BLACK_AND_WHITE_BLUE_SCALE;
            redOffset = PRESET_BLACK_AND_WHITE_RED_OFFSET;
            greenOffset = PRESET_BLACK_AND_WHITE_GREEN_OFFSET;
            blueOffset = PRESET_BLACK_AND_WHITE_BLUE_OFFSET;
            vignette = PRESET_BLACK_AND_WHITE_VIGNETTE;
        }
        else if (inParam1 == (long) anselAdamsPresetButton)
        {
            brightness = PRESET_ANSEL_ADAMS_BRIGHTNESS;
            contrast = PRESET_ANSEL_ADAMS_CONTRAST;
            saturation = PRESET_ANSEL_ADAMS_SATURATION;
            redScale = PRESET_ANSEL_ADAMS_RED_SCALE;
            greenScale = PRESET_ANSEL_ADAMS_GREEN_SCALE;
            blueScale = PRESET_ANSEL_ADAMS_BLUE_SCALE;
            redOffset = PRESET_ANSEL_ADAMS_RED_OFFSET;
            greenOffset = PRESET_ANSEL_ADAMS_GREEN_OFFSET;
            blueOffset = PRESET_ANSEL_ADAMS_BLUE_OFFSET;
            vignette = PRESET_ANSEL_ADAMS_VIGNETTE;
        }
        
        UpdateSettingsWidgets();
    }
    
    return 0;
}

// creates the settings widget
void CreateSettingsWidget(int x, int y, int w, int h)
{
	int x2 = x + w;
	int y2 = y - h;
    
	// widget window
	settingsWidget = XPCreateWidget(x, y, x2, y2, 1, NAME" Settings", 1, 0, xpWidgetClass_MainWindow);
    
	// add close box
	XPSetWidgetProperty(settingsWidget, xpProperty_MainWindowHasCloseBoxes, 1);
    
    // add post-processing sub window
    XPCreateWidget(x + 10, y - 30, x2 - 10, y - 575 - 10, 1, "Post-Processing Settings:", 0, settingsWidget, xpWidgetClass_SubWindow);
    
    // add post-processing settings caption
    XPCreateWidget(x + 10, y - 30, x2 - 20, y - 45, 1, "Post-Processing Settings:", 0, settingsWidget, xpWidgetClass_Caption);
    
    // add post-processing checkbox
    postProcessingCheckbox = XPCreateWidget(x + 20, y - 60, x2 - 20, y - 75, 1, "Enable Post-Processing", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(postProcessingCheckbox, xpProperty_ButtonType, xpRadioButton);
    XPSetWidgetProperty(postProcessingCheckbox, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
    
	// add brightness caption
    char stringBrightness[32];
    sprintf(stringBrightness, "Brightness: %.2f", brightness);
	brightnessCaption = XPCreateWidget(x + 30, y - 90, x2 - 50, y - 105, 1, stringBrightness, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add brightness slider
	brightnessSlider = XPCreateWidget(x + 195, y - 90, x2 - 15, y - 105, 1, "Brightness", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(brightnessSlider, xpProperty_ScrollBarMin, -500);
	XPSetWidgetProperty(brightnessSlider, xpProperty_ScrollBarMax, 500);
    
    // add contrast caption
    char stringContrast[32];
    sprintf(stringContrast, "Contrast: %.2f", contrast);
	contrastCaption = XPCreateWidget(x + 30, y - 110, x2 - 50, y - 125, 1, stringContrast, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add contrast slider
	contrastSlider = XPCreateWidget(x + 195, y - 110, x2 - 15, y - 125, 1, "Contrast", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(contrastSlider, xpProperty_ScrollBarMin, 5);
	XPSetWidgetProperty(contrastSlider, xpProperty_ScrollBarMax, 200);
    
    // add saturation caption
    char stringSaturation[32];
    sprintf(stringSaturation, "Saturation: %.2f", saturation);
	saturationCaption = XPCreateWidget(x + 30, y - 130, x2 - 50, y - 145, 1, stringSaturation, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add saturation slider
	saturationSlider = XPCreateWidget(x + 195, y - 130, x2 - 15, y - 145, 1, "Saturation", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(saturationSlider, xpProperty_ScrollBarMin, 0);
	XPSetWidgetProperty(saturationSlider, xpProperty_ScrollBarMax, 250);
    
    // add red scale caption
    char stringRedScale[32];
    sprintf(stringRedScale, "Red Scale: %.2f", redScale);
	redScaleCaption = XPCreateWidget(x + 30, y - 150, x2 - 50, y - 165, 1, stringRedScale, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add red scale slider
	redScaleSlider = XPCreateWidget(x + 195, y - 150, x2 - 15, y - 165, 1, "Red Scale", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(redScaleSlider, xpProperty_ScrollBarMin, -75);
	XPSetWidgetProperty(redScaleSlider, xpProperty_ScrollBarMax, 75);
    
    // add green scale caption
    char stringGreenScale[32];
    sprintf(stringGreenScale, "Green Scale: %.2f", greenScale);
	greenScaleCaption = XPCreateWidget(x + 30, y - 170, x2 - 50, y - 185, 1, stringGreenScale, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add green scale slider
	greenScaleSlider = XPCreateWidget(x + 195, y - 170, x2 - 15, y - 185, 1, "Green Scale", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(greenScaleSlider, xpProperty_ScrollBarMin, -75);
	XPSetWidgetProperty(greenScaleSlider, xpProperty_ScrollBarMax, 75);
    
    // add blue scale caption
    char stringBlueScale[32];
    sprintf(stringBlueScale, "Blue Scale: %.2f", blueScale);
	blueScaleCaption = XPCreateWidget(x + 30, y - 190, x2 - 50, y - 205, 1, stringBlueScale, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add blue scale slider
	blueScaleSlider = XPCreateWidget(x + 195, y - 190, x2 - 15, y - 205, 1, "Blue Scale", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(blueScaleSlider, xpProperty_ScrollBarMin, -75);
	XPSetWidgetProperty(blueScaleSlider, xpProperty_ScrollBarMax, 75);
    
    // add red offset caption
    char stringRedOffset[32];
    sprintf(stringRedOffset, "Red Offset: %.2f", redOffset);
	redOffsetCaption = XPCreateWidget(x + 30, y - 210, x2 - 50, y - 225, 1, stringRedOffset, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add red offset slider
	redOffsetSlider = XPCreateWidget(x + 195, y - 210, x2 - 15, y - 225, 1, "Red Offset", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(redOffsetSlider, xpProperty_ScrollBarMin, -50);
	XPSetWidgetProperty(redOffsetSlider, xpProperty_ScrollBarMax, 50);
    
    // add green offset caption
    char stringGreenOffset[32];
    sprintf(stringGreenOffset, "Green Offset: %.2f", greenOffset);
	greenOffsetCaption = XPCreateWidget(x + 30, y - 230, x2 - 50, y - 245, 1, stringGreenOffset, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add green offset slider
	greenOffsetSlider = XPCreateWidget(x + 195, y - 230, x2 - 15, y - 245, 1, "Green Offset", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(greenOffsetSlider, xpProperty_ScrollBarMin, -50);
	XPSetWidgetProperty(greenOffsetSlider, xpProperty_ScrollBarMax, 50);
    
    // add blue offset caption
    char stringBlueOffset[32];
    sprintf(stringBlueOffset, "Blue Offset: %.2f", blueOffset);
	blueOffsetCaption = XPCreateWidget(x + 30, y - 250, x2 - 50, y - 265, 1, stringBlueOffset, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add blue offset slider
	blueOffsetSlider = XPCreateWidget(x + 195, y - 250, x2 - 15, y - 265, 1, "Blue Offset", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(blueOffsetSlider, xpProperty_ScrollBarMin, -50);
	XPSetWidgetProperty(blueOffsetSlider, xpProperty_ScrollBarMax, 50);
    
    // add vignette caption
    char stringVignette[32];
    sprintf(stringVignette, "Vignette: %.2f", vignette);
	vignetteCaption = XPCreateWidget(x + 30, y - 270, x2 - 50, y - 285, 1, stringVignette, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add vignette slider
	vignetteSlider = XPCreateWidget(x + 195, y - 270, x2 - 15, y - 285, 1, "Vignette", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(vignetteSlider, xpProperty_ScrollBarMin, 0);
	XPSetWidgetProperty(vignetteSlider, xpProperty_ScrollBarMax, 100);
    
    // add reset button
    resetButton = XPCreateWidget(x + 30, y - 300, x + 30 + 80, y - 315, 1, "Reset", 0, settingsWidget, xpWidgetClass_Button);
	XPSetWidgetProperty(resetButton, xpProperty_ButtonType, xpPushButton);
    
    // add post-processing presets caption
    XPCreateWidget(x + 10, y - 330, x2 - 20, y - 345, 1, "Post-Processing Presets:", 0, settingsWidget, xpWidgetClass_Caption);
    
    // first preset button column
    
    // add polarized preset button
    polarizedPresetButton = XPCreateWidget(x + 20, y - 360, x + 20 + 125, y - 375, 1, "Polarized", 0, settingsWidget, xpWidgetClass_Button);
	XPSetWidgetProperty(polarizedPresetButton, xpProperty_ButtonType, xpPushButton);
    
    // add crazy hazy preset button
    crazyHazyPresetButton = XPCreateWidget(x + 20, y - 385, x + 20 + 125, y - 400, 1, "Crazy Hazy", 0, settingsWidget, xpWidgetClass_Button);
	XPSetWidgetProperty(crazyHazyPresetButton, xpProperty_ButtonType, xpPushButton);

    // add hdr-ish preset button
    hdrIshPresetButton = XPCreateWidget(x + 20, y - 410, x + 20 + 125, y - 425, 1, "HDR-ish", 0, settingsWidget, xpWidgetClass_Button);
	XPSetWidgetProperty(hdrIshPresetButton, xpProperty_ButtonType, xpPushButton);
    
    // add negative drab preset button
    negativeDrabPresetButton = XPCreateWidget(x + 20, y - 435, x + 20 + 125, y - 450, 1, "Negative Drab", 0, settingsWidget, xpWidgetClass_Button);
	XPSetWidgetProperty(negativeDrabPresetButton, xpProperty_ButtonType, xpPushButton);
    
    // add extra normal preset button
    extraNormalPresetButton = XPCreateWidget(x + 20, y - 460, x + 20 + 125, y - 475, 1, "Extra Normal", 0, settingsWidget, xpWidgetClass_Button);
	XPSetWidgetProperty(extraNormalPresetButton, xpProperty_ButtonType, xpPushButton);
    
    // add shadowhancer preset button
    shadowhancerPresetButton = XPCreateWidget(x + 20, y - 485, x + 20 + 125, y - 500, 1, "Shadowhancer", 0, settingsWidget, xpWidgetClass_Button);
	XPSetWidgetProperty(shadowhancerPresetButton, xpProperty_ButtonType, xpPushButton);
    
    // add red shift preset button
    redShiftPresetButton = XPCreateWidget(x + 20, y - 510, x + 20 + 125, y - 525, 1, "Red Shift", 0, settingsWidget, xpWidgetClass_Button);
	XPSetWidgetProperty(redShiftPresetButton, xpProperty_ButtonType, xpPushButton);
    
    // add green shift preset button
    greenShiftPresetButton = XPCreateWidget(x + 20, y - 535, x + 20 + 125, y - 550, 1, "Green Shift", 0, settingsWidget, xpWidgetClass_Button);
	XPSetWidgetProperty(greenShiftPresetButton, xpProperty_ButtonType, xpPushButton);
    
    // add blue shift preset button
    blueShiftPresetButton = XPCreateWidget(x + 20, y - 560, x + 20 + 125, y - 575, 1, "Blue Shift", 0, settingsWidget, xpWidgetClass_Button);
	XPSetWidgetProperty(blueShiftPresetButton, xpProperty_ButtonType, xpPushButton);
    
    // second preset button column
    
    // add sunny florida preset button
    sunnyFloridaPresetButton = XPCreateWidget(x2 - 20 - 125, y - 360, x2 - 20, y - 375, 1, "Sunny Florida", 0, settingsWidget, xpWidgetClass_Button);
	XPSetWidgetProperty(polarizedPresetButton, xpProperty_ButtonType, xpPushButton);
    
    // add mojave desert preset button
    mojaveDesertPresentButton = XPCreateWidget(x2 - 20 - 125, y - 385, x2 - 20, y - 400, 1, "Mojave Desert", 0, settingsWidget, xpWidgetClass_Button);
	XPSetWidgetProperty(crazyHazyPresetButton, xpProperty_ButtonType, xpPushButton);
    
    // add winter springs preset button
    winterSpringsPresetButton = XPCreateWidget(x2 - 20 - 125, y - 410, x2 - 20, y - 425, 1, "Winter Springs", 0, settingsWidget, xpWidgetClass_Button);
	XPSetWidgetProperty(winterSpringsPresetButton, xpProperty_ButtonType, xpPushButton);
    
    // add vivid dreams preset button
    vividDreamsPresetButton = XPCreateWidget(x2 - 20 - 125, y - 435, x2 - 20, y - 450, 1, "Vivid Dreams", 0, settingsWidget, xpWidgetClass_Button);
	XPSetWidgetProperty(vividDreamsPresetButton, xpProperty_ButtonType, xpPushButton);
    
    // add extra normal preset button
    technoColorPresetButton = XPCreateWidget(x2 - 20 - 125, y - 460, x2 - 20, y - 475, 1, "Technocolor", 0, settingsWidget, xpWidgetClass_Button);
	XPSetWidgetProperty(technoColorPresetButton, xpProperty_ButtonType, xpPushButton);
    
    // add gun metal preset button
    gunMetalPresetButton = XPCreateWidget(x2 - 20 - 125, y - 485, x2 - 20, y - 500, 1, "Gun Metal", 0, settingsWidget, xpWidgetClass_Button);
	XPSetWidgetProperty(gunMetalPresetButton, xpProperty_ButtonType, xpPushButton);
    
    // add old movie preset button
    oldMoviePresetButton = XPCreateWidget(x2 - 20 - 125, y - 510, x2 - 20, y - 525, 1, "Old Movie", 0, settingsWidget, xpWidgetClass_Button);
	XPSetWidgetProperty(oldMoviePresetButton, xpProperty_ButtonType, xpPushButton);
    
    // add black & white preset button
    blackAndWhitePresetButton = XPCreateWidget(x2 - 20 - 125, y - 535, x2 - 20, y - 550, 1, "Black & White", 0, settingsWidget, xpWidgetClass_Button);
	XPSetWidgetProperty(blackAndWhitePresetButton, xpProperty_ButtonType, xpPushButton);
    
    // add ansel adams preset button
    anselAdamsPresetButton = XPCreateWidget(x2 - 20 - 125, y - 560, x2 - 20, y - 575, 1, "Ansel Adams", 0, settingsWidget, xpWidgetClass_Button);
	XPSetWidgetProperty(anselAdamsPresetButton, xpProperty_ButtonType, xpPushButton);
    
    // add fps-limiter sub window
    XPCreateWidget(x + 10, y - 600, x2 - 10, y - 675 - 10, 1, "FPS-Limiter:", 0, settingsWidget, xpWidgetClass_SubWindow);
    
    // add fps-limiter caption
    XPCreateWidget(x + 10, y - 600, x2 - 20, y - 615, 1, "FPS-Limiter:", 0, settingsWidget, xpWidgetClass_Caption);
    
    // add fps-limiter checkbox
    fpsLimiterCheckbox = XPCreateWidget(x + 20, y - 630, x2 - 20, y - 645, 1, "Enable FPS-Limiter", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(fpsLimiterCheckbox, xpProperty_ButtonType, xpRadioButton);
    XPSetWidgetProperty(fpsLimiterCheckbox, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
    
    // add max fps caption
    char stringMaxFps[32];
    sprintf(stringMaxFps, "Max FPS: %.0f", maxFps);
	maxFpsCaption = XPCreateWidget(x + 30, y - 660, x2 - 50, y - 675, 1, stringMaxFps, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add max fps slider
	maxFpsSlider = XPCreateWidget(x + 195, y - 660, x2 - 15, y - 675, 1, "Max FPS", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(maxFpsSlider, xpProperty_ScrollBarMin, 20);
	XPSetWidgetProperty(maxFpsSlider, xpProperty_ScrollBarMax, 200);
    
    // add auto disable enable cinema verite sub window
    XPCreateWidget(x + 10, y - 700, x2 - 10, y - 775 - 10, 1, "Auto disable / enable Cinema Verite:", 0, settingsWidget, xpWidgetClass_SubWindow);
    
    // add auto disable enable cinema verite caption
    XPCreateWidget(x + 10, y - 700, x2 - 20, y - 715, 1, "Auto disable / enable Cinema Verite:", 0, settingsWidget, xpWidgetClass_Caption);
    
    // add control cinema verite checkbox
    controlCinemaVeriteCheckbox = XPCreateWidget(x + 20, y - 730, x2 - 20, y - 745, 1, "Control Cinema Verite", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(controlCinemaVeriteCheckbox, xpProperty_ButtonType, xpRadioButton);
    XPSetWidgetProperty(controlCinemaVeriteCheckbox, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
    
    // add disable cinema verite time caption
    char stringDisableCinemaVeriteTime[32];
    sprintf(stringDisableCinemaVeriteTime, "On input disable for: %.0f sec", disableCinemaVeriteTime);
	disableCinemaVeriteTimeCaption = XPCreateWidget(x + 30, y - 760, x2 - 50, y - 775, 1, stringDisableCinemaVeriteTime, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add disable cinema verite time slider
	disableCinemaVeriteTimeSlider = XPCreateWidget(x + 195, y - 760, x2 - 15, y - 775, 1, "Disable Cinema Verite Timer", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(disableCinemaVeriteTimeSlider, xpProperty_ScrollBarMin, 1);
	XPSetWidgetProperty(disableCinemaVeriteTimeSlider, xpProperty_ScrollBarMax, 30);
    
    // add about sub window
    XPCreateWidget(x + 10, y - 800, x2 - 10, y - 860 - 10, 1, "About:", 0, settingsWidget, xpWidgetClass_SubWindow);
    
    // add about caption
    XPCreateWidget(x + 10, y - 800, x2 - 20, y - 815, 1, NAME" "VERSION, 0, settingsWidget, xpWidgetClass_Caption);
    XPCreateWidget(x + 10, y - 815, x2 - 20, y - 830, 1, "Thank you for using "NAME" by Matteo Hausner", 0, settingsWidget, xpWidgetClass_Caption);
    XPCreateWidget(x + 10, y - 830, x2 - 20, y - 845, 1, "Copyright 2014 for non-commerical use only!", 0, settingsWidget, xpWidgetClass_Caption);
    XPCreateWidget(x + 10, y - 845, x2 - 20, y - 860, 1, "Contact: matteo.hausner@gmail.com or www.bwravencl.de", 0, settingsWidget, xpWidgetClass_Caption);
    
    // init checkbox and slider positions
    UpdateSettingsWidgets();
    
	// register widget handler
	XPAddWidgetCallback(settingsWidget, (XPWidgetFunc_t) SettingsWidgetHandler);
}

// handles the menu-entries
void MenuHandlerCallback(void* inMenuRef, void* inItemRef)
{
	// settings menu entry
	if ((long) inItemRef == 0)
	{
		if (settingsWidget == 0) // settings not created yet
		{
            int x, y;
            XPLMGetScreenSize(&x, &y);
            
			CreateSettingsWidget(10, y - 100, 350, 880);
		}
		else // settings already created
		{
			if (!XPIsWidgetVisible(settingsWidget))
                XPShowWidget(settingsWidget);
		}
	}
}

PLUGIN_API int XPluginStart(
                            char *		outName,
                            char *		outSig,
                            char *		outDesc)
{
    // set plugin info
	strcpy(outName, NAME);
	strcpy(outSig, "de.bwravencl."NAME_LOWERCASE);
	strcpy(outDesc, NAME" enhances your X-Plane experience!");
    
    // prepare fragment-shader
    InitShader(FRAGMENT_SHADER);
    
    // obtain datarefs
    cinemaVeriteDataRef = XPLMFindDataRef("sim/graphics/view/cinema_verite");
    viewTypeDataRef = XPLMFindDataRef("sim/graphics/view/view_type");
    
    // create menu-entries
	int SubMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), NAME, 0, 1);
	XPLMMenuID Menu = XPLMCreateMenu(NAME, XPLMFindPluginsMenu(), SubMenuItem, MenuHandlerCallback, 0);
	XPLMAppendMenuItem(Menu, "Settings", (void*) 0, 1); // settings menu entry with ItemRef = 0
    
    // read and apply config file
    LoadSettings();
    
    // register flightloop-callbacks
    if (fpsLimiterEnabled != 0)
        XPLMRegisterFlightLoopCallback(LimiterFlightCallback, -1, NULL);
    if (controlCinemaVeriteEnabled != 0)
        XPLMRegisterFlightLoopCallback(ControlCinemaVeriteCallback, -1, NULL);
    
    // register draw-callbacks
    if (postProcesssingEnabled != 0)
        XPLMRegisterDrawCallback(PostProcessingCallback, xplm_Phase_Window, 1, NULL);
    if (fpsLimiterEnabled != 0)
        XPLMRegisterDrawCallback(LimiterDrawCallback, xplm_Phase_Terrain, 1, NULL);
    
	return 1;
}


PLUGIN_API void	XPluginStop(void)
{
    CleanupShader(1);
}

PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

PLUGIN_API void XPluginReceiveMessage(
                                      XPLMPluginID	inFromWho,
                                      long			inMessage,
                                      void *			inParam)
{
}
