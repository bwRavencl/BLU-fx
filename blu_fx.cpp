/* Copyright 2014 Matteo Hausner */

#include "XPLMDataAccess.h"
#include "XPLMDefs.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMMenus.h"
#include "XPLMPlugin.h"
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
#define VERSION "0.4"

// define config file path
#if IBM
#define CONFIG_PATH ".\\Resources\\plugins\\"NAME_LOWERCASE"\\"NAME_LOWERCASE".ini"
#else
#define CONFIG_PATH "./Resources/plugins/"NAME_LOWERCASE"/"NAME_LOWERCASE".ini"
#endif

#define DEFAULT_POST_PROCESSING_ENABLED 1
#define DEFAULT_FPS_LIMITER_ENABLED 0
#define DEFAULT_CONTROL_CINEMA_VERITE_ENABLED 1
#define DEFAULT_RALEIGH_SCALE 13.0f
#define DEFAULT_MAX_FRAME_RATE 35.0f
#define DEFAULT_DISABLE_CINEMA_VERITE_TIME 5.0f

enum BLUfxPresets_t { PRESET_DEFAULT, PRESET_POLAROID, PRESET_FOGGED_UP,
                      PRESET_HIGH_DYNAMIC_RANGE, PRESET_EDITORS_CHOICE,
                      PRESET_SLIGHTLY_ENHANCED, PRESET_EXTRA_GLOOMY, PRESET_RED_ISH,
                      PRESET_GREEN_ISH, PRESET_BLUE_ISH, PRESET_SHINY_CALIFORNIA,
                      PRESET_DUSTY_DRY, PRESET_GRAY_WINTER, PRESET_FANCY_IMAGINATION,
                      PRESET_SIXTIES, PRESET_COLD_WINTER, PRESET_VINTAGE_FILM, PRESET_COLORLESS,
                      PRESET_MONOCHROME, PRESET_MAX
                    };

struct BLUfxPreset_t
{
    /* Basic */
    float brightness;
    float contrast;
    float saturation;
    /* Scale */
    float redScale;
    float greenScale;
    float blueScale;
    /* Offset */
    float redOffset;
    float greenOffset;
    float blueOffset;
    /* Misc */
    float vignette;
};
typedef BLUfxPreset_t BLUfxPreset;

BLUfxPreset BLUfxPresets [PRESET_MAX] =
{
    // PRESET_DEFAULT
    {
        .brightness = 0.0f,
        .contrast = 1.0f,
        .saturation = 1.0f,
        .redScale = 0.0f,
        .greenScale = 0.0f,
        .blueScale = 0.0f,
        .redOffset = 0.0f,
        .greenOffset = 0.0f,
        .blueOffset = 0.0f,
        .vignette = 0.0f
    },
    // PRESET_POLAROID
    {
        .brightness = 0.05f,
        .contrast = 1.1f,
        .saturation = 1.4f,
        .redScale = 0.0f,
        .greenScale = 0.0f,
        .blueScale = -0.2f,
        .redOffset = 0.0f,
        .greenOffset = 0.0f,
        .blueOffset = 0.0f,
        .vignette = 0.6f
    },
    // PRESET_FOGGED_UP
    {
        .brightness = 0.05f,
        .contrast = 1.2f,
        .saturation = 0.7f,
        .redScale = 0.15f,
        .greenScale = 0.15f,
        .blueScale = 0.15f,
        .redOffset = 0.0f,
        .greenOffset = 0.0f,
        .blueOffset = 0.0f,
        .vignette = 0.3f
    },
    // PRESET_HIGH_DYNAMIC_RANGE
    {
        .brightness = 0.0f,
        .contrast = 1.15f,
        .saturation = 0.9f,
        .redScale = 0.0f,
        .greenScale = 0.0f,
        .blueScale = 0.0f,
        .redOffset = 0.0f,
        .greenOffset = 0.0f,
        .blueOffset = 0.0f,
        .vignette = 0.6f
    },
    // PRESET_EDITORS_CHOICE
    {
        .brightness = 0.05f,
        .contrast = 1.1f,
        .saturation = 1.3f,
        .redScale = 0.0f,
        .greenScale = 0.0f,
        .blueScale = 0.0f,
        .redOffset = 0.0f,
        .greenOffset = 0.0f,
        .blueOffset = 0.0f,
        .vignette = 0.3f
    },
    // PRESET_SLIGHTLY_ENHANCED
    {
        .brightness = 0.05f,
        .contrast = 1.1f,
        .saturation = 1.1f,
        .redScale = 0.0f,
        .greenScale = 0.0f,
        .blueScale = 0.0f,
        .redOffset = 0.0f,
        .greenOffset = 0.0f,
        .blueOffset = 0.0f,
        .vignette = 0.0f
    },
    // PRESET_EXTRA_GLOOMY
    {
        .brightness = -0.15f,
        .contrast = 1.3f,
        .saturation = 1.0f,
        .redScale = 0.0f,
        .greenScale = 0.0f,
        .blueScale = 0.0f,
        .redOffset = 0.0f,
        .greenOffset = 0.0f,
        .blueOffset = 0.0f,
        .vignette = 0.0f
    },
    // PRESET_RED_ISH
    {
        .brightness = 0.0f,
        .contrast = 1.1f,
        .saturation = 1.1f,
        .redScale = 0.1f,
        .greenScale = 0.0f,
        .blueScale = 0.0f,
        .redOffset = 0.0f,
        .greenOffset = 0.0f,
        .blueOffset = 0.0f,
        .vignette = 0.0f
    },
    // PRESET_GREEN_ISH
    {
        .brightness = 0.0f,
        .contrast = 1.1f,
        .saturation = 1.1f,
        .redScale = 0.0f,
        .greenScale = 0.0f,
        .blueScale = 0.1f,
        .redOffset = 0.0f,
        .greenOffset = 0.0f,
        .blueOffset = 0.0f,
        .vignette = 0.0f
    },
    // PRESET_BLUE_ISH
    {
        .brightness = 0.0f,
        .contrast = 1.1f,
        .saturation = 1.1f,
        .redScale = 0.0f,
        .greenScale = 0.1f,
        .blueScale = 0.0f,
        .redOffset = 0.0f,
        .greenOffset = 0.0f,
        .blueOffset = 0.0f,
        .vignette = 0.0f
    },
    // PRESET_SHINY_CALIFORNIA
    {
        .brightness = 0.1f,
        .contrast = 1.5f,
        .saturation = 1.3f,
        .redScale = 0.0f,
        .greenScale = 0.0f,
        .blueScale = 0.0f,
        .redOffset = 0.0f,
        .greenOffset = 0.0f,
        .blueOffset = -0.1f,
        .vignette = 0.0f
    },
    // PRESET_DUSTY_DRY
    {
        .brightness = 0.0f,
        .contrast = 1.3f,
        .saturation = 1.3f,
        .redScale = 0.2f,
        .greenScale = 0.0f,
        .blueScale = 0.0f,
        .redOffset = 0.0f,
        .greenOffset = 0.0f,
        .blueOffset = 0.0f,
        .vignette = 0.6f
    },
    // PRESET_GRAY_WINTER
    {
        .brightness = 0.07f,
        .contrast = 1.15f,
        .saturation = 1.3f,
        .redScale = 0.0f,
        .greenScale = 0.0f,
        .blueScale = 0.2f,
        .redOffset = 0.0f,
        .greenOffset = 0.05f,
        .blueOffset = 0.0f,
        .vignette = 0.0f
    },
    // PRESET_FANCY_IMAGINATION
    {
        .brightness = 0.0f,
        .contrast = 1.6f,
        .saturation = 1.5f,
        .redScale = 0.0f,
        .greenScale = 0.0f,
        .blueScale = -0.1f,
        .redOffset = 0.0f,
        .greenOffset = 0.05f,
        .blueOffset = 0.0f,
        .vignette = 0.6f
    },
    // PRESET_SIXTIES
    {
        .brightness = 0.0f,
        .contrast = 1.6f,
        .saturation = 1.5f,
        .redScale = 0.2f,
        .greenScale = 0.0f,
        .blueScale = -0.1f,
        .redOffset = 0.0f,
        .greenOffset = 0.05f,
        .blueOffset = 0.0f,
        .vignette = 0.65f
    },
    // PRESET_COLD_WINTER
    {
        .brightness = 0.0f,
        .contrast = 1.55f,
        .saturation = 0.6f,
        .redScale = 0.0f,
        .greenScale = 0.05f,
        .blueScale = 0.2f,
        .redOffset = 0.0f,
        .greenOffset = 0.05f,
        .blueOffset = 0.0f,
        .vignette = 0.25f
    },
    // PRESET_VINTAGE_FILM
    {
        .brightness = 0.0f,
        .contrast = 1.05f,
        .saturation = 0.0f,
        .redScale = 0.0f,
        .greenScale = 0.0f,
        .blueScale = 0.07f,
        .redOffset = 0.07f,
        .greenOffset = 0.03f,
        .blueOffset = 0.0f,
        .vignette = 0.0f
    },
    // PRESET_COLORLESS
    {
        .brightness = -0.03f,
        .contrast = 1.3f,
        .saturation = 0.0f,
        .redScale = 0.0f,
        .greenScale = 0.0f,
        .blueScale = 0.0f,
        .redOffset = 0.0f,
        .greenOffset = 0.03f,
        .blueOffset = 0.0f,
        .vignette = 0.65f
    },
    // PRESET_MONOCHROME
    {
        .brightness = -0.13f,
        .contrast = 1.2f,
        .saturation = 0.0f,
        .redScale = 0.0f,
        .greenScale = 0.0f,
        .blueScale = 0.0f,
        .redOffset = 0.0f,
        .greenOffset = 0.03f,
        .blueOffset = 0.0f,
        .vignette = 0.7f
    }
};

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
static float maxFps = DEFAULT_MAX_FRAME_RATE,
             disableCinemaVeriteTime = DEFAULT_DISABLE_CINEMA_VERITE_TIME;
static float brightness = BLUfxPresets[PRESET_DEFAULT].brightness,
             contrast = BLUfxPresets[PRESET_DEFAULT].contrast,
             saturation = BLUfxPresets[PRESET_DEFAULT].saturation,
             redScale = BLUfxPresets[PRESET_DEFAULT].redScale,
             greenScale = BLUfxPresets[PRESET_DEFAULT].greenScale,
             blueScale = BLUfxPresets[PRESET_DEFAULT].blueScale,
             redOffset = BLUfxPresets[PRESET_DEFAULT].redOffset,
             greenOffset = BLUfxPresets[PRESET_DEFAULT].greenOffset,
             blueOffset = BLUfxPresets[PRESET_DEFAULT].blueOffset,
             vignette = BLUfxPresets[PRESET_DEFAULT].vignette,
             raleighScale = DEFAULT_RALEIGH_SCALE;

// global internal variables
static int lastMouseX = 0, lastMouseY = 0, lastResolutionX = 0, lastResolutionY = 0;
static GLuint textureId = 0, program = 0, fragmentShader = 0;
static float startTimeFlight = 0.0f, endTimeFlight = 0.0f, startTimeDraw = 0.0f, endTimeDraw = 0.0f, lastMouseMovementTime = 0.0f;
#if LIN
static Display *display = NULL;
#endif

// global dataref variables
static XPLMDataRef cinemaVeriteDataRef, viewTypeDataRef, raleighScaleDataRef;

// global widget variables
static XPWidgetID settingsWidget, postProcessingCheckbox, fpsLimiterCheckbox, controlCinemaVeriteCheckbox, brightnessCaption, contrastCaption, saturationCaption, redScaleCaption, greenScaleCaption, blueScaleCaption, redOffsetCaption, greenOffsetCaption, blueOffsetCaption, vignetteCaption, raleighScaleCaption, maxFpsCaption, disableCinemaVeriteTimeCaption, brightnessSlider, contrastSlider, saturationSlider, redScaleSlider, greenScaleSlider, blueScaleSlider, redOffsetSlider, greenOffsetSlider, blueOffsetSlider, vignetteSlider, raleighScaleSlider, maxFpsSlider, disableCinemaVeriteTimeSlider, presetButtons[PRESET_MAX], resetRaleighScaleButton;

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

    if (mouseButtonDown == 1 || currentMouseX != lastMouseX || currentMouseY != lastMouseY)
    {
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
    if (isFragmentShaderCompiled == GL_FALSE)
    {
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
    if (isProgramLinked == GL_FALSE)
    {
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

// sets the raleigh scale dataref to the selected raleigh scale value, passing reset = 1 resets the dataref to its default value
void UpdateRaleighScale(int reset)
{
    if (raleighScaleDataRef == NULL)
        raleighScaleDataRef = XPLMFindDataRef("sim/private/controls/atmo/atmo_scale_raleigh");
    if (raleighScaleDataRef != NULL)
        XPLMSetDataf(raleighScaleDataRef, reset == 0 ? raleighScale : DEFAULT_RALEIGH_SCALE);
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

    char stringRaleighScale[32];
    sprintf(stringRaleighScale, "Raleigh Scale: %.2f", raleighScale);
    XPSetWidgetDescriptor(raleighScaleCaption, stringRaleighScale);

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
    XPSetWidgetProperty(raleighScaleSlider, xpProperty_ScrollBarSliderPosition, (intptr_t) raleighScale);
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
        file << "raleighScale=" << raleighScale << std::endl;
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
            else if(line.find("raleighScale") != -1)
                iss >> raleighScale;
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
            {
                XPLMUnregisterDrawCallback(PostProcessingCallback, xplm_Phase_Window, 1, NULL);
                UpdateRaleighScale(1);
            }
            else
            {
                XPLMRegisterDrawCallback(PostProcessingCallback, xplm_Phase_Window, 1, NULL);
                UpdateRaleighScale(0);
            }

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
        else if (inParam1 == (long) raleighScaleSlider)
        {
            raleighScale = Round(XPGetWidgetProperty(raleighScaleSlider, xpProperty_ScrollBarSliderPosition, 0));
            UpdateRaleighScale(0);
        }
        else if (inParam1 == (long) maxFpsSlider)
            maxFps = (float) (int) XPGetWidgetProperty(maxFpsSlider, xpProperty_ScrollBarSliderPosition, 0);
        else if (inParam1 == (long) disableCinemaVeriteTimeSlider)
            disableCinemaVeriteTime = (float) (int) XPGetWidgetProperty(disableCinemaVeriteTimeSlider, xpProperty_ScrollBarSliderPosition, 0);

        UpdateSettingsWidgets();
    }
    else if (inMessage == xpMsg_PushButtonPressed)
    {
        if ((long) inParam1 == (long) resetRaleighScaleButton)
        {
            raleighScale = DEFAULT_RALEIGH_SCALE;
            UpdateRaleighScale(1);
        }
        else
        {
            int i;
            for (i=0; i < PRESET_MAX; i++)
            {
                if ((long) presetButtons[i] == (long) inParam1)
                {
                    brightness = BLUfxPresets[i].brightness;
                    contrast = BLUfxPresets[i].contrast;
                    saturation = BLUfxPresets[i].saturation;
                    redScale = BLUfxPresets[i].redScale;
                    greenScale = BLUfxPresets[i].greenScale;
                    blueScale = BLUfxPresets[i].blueScale;
                    redOffset = BLUfxPresets[i].redOffset;
                    greenOffset = BLUfxPresets[i].greenOffset;
                    blueOffset = BLUfxPresets[i].blueOffset;
                    vignette = BLUfxPresets[i].vignette;

                    break;
                }
            }
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
    presetButtons[PRESET_DEFAULT] = XPCreateWidget(x + 30, y - 300, x + 30 + 80, y - 315, 1, "Reset", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(presetButtons[PRESET_DEFAULT], xpProperty_ButtonType, xpPushButton);

    // add post-processing presets caption
    XPCreateWidget(x + 10, y - 330, x2 - 20, y - 345, 1, "Post-Processing Presets:", 0, settingsWidget, xpWidgetClass_Caption);

    // first preset button column

    // add polaroid preset button
    presetButtons[PRESET_POLAROID] = XPCreateWidget(x + 20, y - 360, x + 20 + 125, y - 375, 1, "Polaroid", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(presetButtons[PRESET_POLAROID], xpProperty_ButtonType, xpPushButton);

    // add fogged up preset button
    presetButtons[PRESET_FOGGED_UP] = XPCreateWidget(x + 20, y - 385, x + 20 + 125, y - 400, 1, "Fogged Up", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(presetButtons[PRESET_FOGGED_UP], xpProperty_ButtonType, xpPushButton);

    // add high dynamic range preset button
    presetButtons[PRESET_HIGH_DYNAMIC_RANGE] = XPCreateWidget(x + 20, y - 410, x + 20 + 125, y - 425, 1, "High Dynamic Range", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(presetButtons[PRESET_HIGH_DYNAMIC_RANGE], xpProperty_ButtonType, xpPushButton);

    // add editor's choice drab preset button
    presetButtons[PRESET_EDITORS_CHOICE] = XPCreateWidget(x + 20, y - 435, x + 20 + 125, y - 450, 1, "Editor's Choice", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(presetButtons[PRESET_EDITORS_CHOICE], xpProperty_ButtonType, xpPushButton);

    // add slightly enhanced preset button
    presetButtons[PRESET_SLIGHTLY_ENHANCED] = XPCreateWidget(x + 20, y - 460, x + 20 + 125, y - 475, 1, "Slightly Enhanced", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(presetButtons[PRESET_SLIGHTLY_ENHANCED], xpProperty_ButtonType, xpPushButton);

    // add extra gloomy preset button
    presetButtons[PRESET_EXTRA_GLOOMY] = XPCreateWidget(x + 20, y - 485, x + 20 + 125, y - 500, 1, "Extra Gloomy", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(presetButtons[PRESET_EXTRA_GLOOMY], xpProperty_ButtonType, xpPushButton);

    // add red shift preset button
    presetButtons[PRESET_RED_ISH] = XPCreateWidget(x + 20, y - 510, x + 20 + 125, y - 525, 1, "Red-ish", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(presetButtons[PRESET_RED_ISH], xpProperty_ButtonType, xpPushButton);

    // add green shift preset button
    presetButtons[PRESET_GREEN_ISH] = XPCreateWidget(x + 20, y - 535, x + 20 + 125, y - 550, 1, "Green-ish", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(presetButtons[PRESET_GREEN_ISH], xpProperty_ButtonType, xpPushButton);

    // add blue shift preset button
    presetButtons[PRESET_BLUE_ISH] = XPCreateWidget(x + 20, y - 560, x + 20 + 125, y - 575, 1, "Blue-ish", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(presetButtons[PRESET_BLUE_ISH], xpProperty_ButtonType, xpPushButton);

    // second preset button column

    // add shiny california preset button
    presetButtons[PRESET_SHINY_CALIFORNIA] = XPCreateWidget(x2 - 20 - 125, y - 360, x2 - 20, y - 375, 1, "Shiny California", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(presetButtons[PRESET_SHINY_CALIFORNIA], xpProperty_ButtonType, xpPushButton);

    // add dusty dry preset button
    presetButtons[PRESET_DUSTY_DRY] = XPCreateWidget(x2 - 20 - 125, y - 385, x2 - 20, y - 400, 1, "Dusty Dry", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(presetButtons[PRESET_DUSTY_DRY], xpProperty_ButtonType, xpPushButton);

    // add gray winter preset button
    presetButtons[PRESET_GRAY_WINTER] = XPCreateWidget(x2 - 20 - 125, y - 410, x2 - 20, y - 425, 1, "Gray Winter", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(presetButtons[PRESET_GRAY_WINTER], xpProperty_ButtonType, xpPushButton);

    // add fancy imagination dreams preset button
    presetButtons[PRESET_FANCY_IMAGINATION] = XPCreateWidget(x2 - 20 - 125, y - 435, x2 - 20, y - 450, 1, "Fancy Imagination", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(presetButtons[PRESET_FANCY_IMAGINATION], xpProperty_ButtonType, xpPushButton);

    // add sixties normal preset button
    presetButtons[PRESET_SIXTIES] = XPCreateWidget(x2 - 20 - 125, y - 460, x2 - 20, y - 475, 1, "Sixties", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(presetButtons[PRESET_SIXTIES], xpProperty_ButtonType, xpPushButton);

    // add cold winter preset button
    presetButtons[PRESET_COLD_WINTER] = XPCreateWidget(x2 - 20 - 125, y - 485, x2 - 20, y - 500, 1, "Cold Winter", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(presetButtons[PRESET_COLD_WINTER], xpProperty_ButtonType, xpPushButton);

    // add vintage film preset button
    presetButtons[PRESET_VINTAGE_FILM] = XPCreateWidget(x2 - 20 - 125, y - 510, x2 - 20, y - 525, 1, "Vintage Film", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(presetButtons[PRESET_VINTAGE_FILM], xpProperty_ButtonType, xpPushButton);

    // add colorless preset button
    presetButtons[PRESET_COLORLESS] = XPCreateWidget(x2 - 20 - 125, y - 535, x2 - 20, y - 550, 1, "Colorless", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(presetButtons[PRESET_COLORLESS], xpProperty_ButtonType, xpPushButton);

    // add monochrome preset button
    presetButtons[PRESET_MONOCHROME] = XPCreateWidget(x2 - 20 - 125, y - 560, x2 - 20, y - 575, 1, "Monochrome", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(presetButtons[PRESET_MONOCHROME], xpProperty_ButtonType, xpPushButton);

    // add raleigh scale sub window
    XPCreateWidget(x + 10, y - 600, x2 - 10, y - 675 - 10, 1, "Raleigh Scale:", 0, settingsWidget, xpWidgetClass_SubWindow);

    // add raleigh scale caption
    XPCreateWidget(x + 10, y - 600, x2 - 20, y - 615, 1, "Raleigh Scale:", 0, settingsWidget, xpWidgetClass_Caption);

    // add raleigh scale caption
    char stringRaleighScale[32];
    sprintf(stringRaleighScale, "Raleigh Scale: %.0f", raleighScale);
    raleighScaleCaption = XPCreateWidget(x + 30, y - 630, x2 - 50, y - 645, 1, stringRaleighScale, 0, settingsWidget, xpWidgetClass_Caption);

    // add raleigh scale slider
    raleighScaleSlider = XPCreateWidget(x + 195, y - 630, x2 - 15, y - 645, 1, "Raleigh Scale", 0, settingsWidget, xpWidgetClass_ScrollBar);
    XPSetWidgetProperty(raleighScaleSlider, xpProperty_ScrollBarMin, 1);
    XPSetWidgetProperty(raleighScaleSlider, xpProperty_ScrollBarMax, 100);

    // add raleigh scale reset button
    resetRaleighScaleButton = XPCreateWidget(x + 30, y - 660, x + 30 + 80, y - 675, 1, "Reset", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(resetRaleighScaleButton, xpProperty_ButtonType, xpPushButton);

    // add fps-limiter sub window
    XPCreateWidget(x + 10, y - 700, x2 - 10, y - 775 - 10, 1, "FPS-Limiter:", 0, settingsWidget, xpWidgetClass_SubWindow);

    // add fps-limiter caption
    XPCreateWidget(x + 10, y - 700, x2 - 20, y - 715, 1, "FPS-Limiter:", 0, settingsWidget, xpWidgetClass_Caption);

    // add fps-limiter checkbox
    fpsLimiterCheckbox = XPCreateWidget(x + 20, y - 730, x2 - 20, y - 745, 1, "Enable FPS-Limiter", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(fpsLimiterCheckbox, xpProperty_ButtonType, xpRadioButton);
    XPSetWidgetProperty(fpsLimiterCheckbox, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);

    // add max fps caption
    char stringMaxFps[32];
    sprintf(stringMaxFps, "Max FPS: %.0f", maxFps);
    maxFpsCaption = XPCreateWidget(x + 30, y - 760, x2 - 50, y - 775, 1, stringMaxFps, 0, settingsWidget, xpWidgetClass_Caption);

    // add max fps slider
    maxFpsSlider = XPCreateWidget(x + 195, y - 760, x2 - 15, y - 775, 1, "Max FPS", 0, settingsWidget, xpWidgetClass_ScrollBar);
    XPSetWidgetProperty(maxFpsSlider, xpProperty_ScrollBarMin, 20);
    XPSetWidgetProperty(maxFpsSlider, xpProperty_ScrollBarMax, 200);

    // add auto disable enable cinema verite sub window
    XPCreateWidget(x + 10, y - 800, x2 - 10, y - 875 - 10, 1, "Auto disable / enable Cinema Verite:", 0, settingsWidget, xpWidgetClass_SubWindow);

    // add auto disable enable cinema verite caption
    XPCreateWidget(x + 10, y - 800, x2 - 20, y - 815, 1, "Auto disable / enable Cinema Verite:", 0, settingsWidget, xpWidgetClass_Caption);

    // add control cinema verite checkbox
    controlCinemaVeriteCheckbox = XPCreateWidget(x + 20, y - 830, x2 - 20, y - 845, 1, "Control Cinema Verite", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(controlCinemaVeriteCheckbox, xpProperty_ButtonType, xpRadioButton);
    XPSetWidgetProperty(controlCinemaVeriteCheckbox, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);

    // add disable cinema verite time caption
    char stringDisableCinemaVeriteTime[32];
    sprintf(stringDisableCinemaVeriteTime, "On input disable for: %.0f sec", disableCinemaVeriteTime);
    disableCinemaVeriteTimeCaption = XPCreateWidget(x + 30, y - 850, x2 - 50, y - 865, 1, stringDisableCinemaVeriteTime, 0, settingsWidget, xpWidgetClass_Caption);

    // add disable cinema verite time slider
    disableCinemaVeriteTimeSlider = XPCreateWidget(x + 195, y - 850, x2 - 15, y - 865, 1, "Disable Cinema Verite Timer", 0, settingsWidget, xpWidgetClass_ScrollBar);
    XPSetWidgetProperty(disableCinemaVeriteTimeSlider, xpProperty_ScrollBarMin, 1);
    XPSetWidgetProperty(disableCinemaVeriteTimeSlider, xpProperty_ScrollBarMax, 30);

    // add about sub window
    XPCreateWidget(x + 10, y - 900, x2 - 10, y - 960 - 10, 1, "About:", 0, settingsWidget, xpWidgetClass_SubWindow);

    // add about caption
    XPCreateWidget(x + 10, y - 900, x2 - 20, y - 915, 1, NAME" "VERSION, 0, settingsWidget, xpWidgetClass_Caption);
    XPCreateWidget(x + 10, y - 915, x2 - 20, y - 930, 1, "Thank you for using "NAME" by Matteo Hausner", 0, settingsWidget, xpWidgetClass_Caption);
    XPCreateWidget(x + 10, y - 930, x2 - 20, y - 945, 1, "Copyright 2014 for non-commerical use only!", 0, settingsWidget, xpWidgetClass_Caption);
    XPCreateWidget(x + 10, y - 945, x2 - 20, y - 960, 1, "Contact: matteo.hausner@gmail.com or www.bwravencl.de", 0, settingsWidget, xpWidgetClass_Caption);

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

            CreateSettingsWidget(10, y - 100, 350, 980);
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

    // init glew
#if IBM
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        XPLMDebugString(NAME": The following error occured while initializing GLEW:\n");
        XPLMDebugString((const char*) glewGetErrorString(err));
    }
#endif

    // prepare fragment-shader
    InitShader(FRAGMENT_SHADER);

    // obtain datarefs
    cinemaVeriteDataRef = XPLMFindDataRef("sim/graphics/view/cinema_verite");
    viewTypeDataRef = XPLMFindDataRef("sim/graphics/view/view_type");

    // create menu-entries
    int SubMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), NAME, 0, 1);
    XPLMMenuID Menu = XPLMCreateMenu(NAME, XPLMFindPluginsMenu(), SubMenuItem, MenuHandlerCallback, 0);
    XPLMAppendMenuItem(Menu, "Settings", (void*) 0, 1);

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
    UpdateRaleighScale(1);
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
    if (inMessage == XPLM_MSG_SCENERY_LOADED)
        UpdateRaleighScale(0);
}
