#include "XPLMDataAccess.h"
#include "XPLMDefs.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMMenus.h"
#include "XPLMProcessing.h"
#include "XPStandardWidgets.h"
#include "XPWidgets.h"

#include <fstream>
#include <string.h>
#include <unistd.h>

#ifdef APL
#include "ApplicationServices/ApplicationServices.h"
#endif

#if APL
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <stdio.h>
#include "XPLMUtilities.h"

// version
#define VERSION "0.1"

// default values
#define DEFAULT_POST_PROCESSING_ENABLED 1
#define DEFAULT_LIMIT_FRAMES_ENABLED 1
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
#define DEFAULT_NOISE 0.0f
#define DEFAULT_MAX_FRAME_RATE 35.0f
#define DEFAULT_DISABLE_CINEMA_VERITE_TIME 5.0f

// global variables
static int postProcesssingEnabled = DEFAULT_POST_PROCESSING_ENABLED, limitFramesEnabled = DEFAULT_LIMIT_FRAMES_ENABLED, controlCinemaVeriteEnabled = DEFAULT_CONTROL_CINEMA_VERITE_ENABLED,lastMouseX = 0, lastMouseY = 0, settingsWindowOpen = 0, aboutWindowOpen = 0;
static GLuint textureId = 0, program;
static float brightness = DEFAULT_BRIGHTNESS, contrast = DEFAULT_CONTRAST, saturation = DEFAULT_SATURATION, redScale = DEFAULT_RED_SCALE, greenScale = DEFAULT_GREEN_SCALE, blueScale = DEFAULT_BLUE_SCALE, redOffset = DEFAULT_RED_OFFSET, greenOffset = DEFAULT_GREEN_OFFSET, blueOffset = DEFAULT_BLUE_OFFSET, vignette = DEFAULT_VIGNETTE, noise = DEFAULT_NOISE,maxFps = DEFAULT_MAX_FRAME_RATE, disableCinemaVeriteTime = DEFAULT_DISABLE_CINEMA_VERITE_TIME, startTimeFlight = 0.0f, endTimeFlight = 0.0f, startTimeDraw = 0.0f, endTimeDraw = 0.0f, lastMouseMovementTime = 0.0f;
static XPLMDataRef cinemaVeriteDataRef, viewTypeDataRef;
static XPWidgetID settingsWidget, aboutWidget, postProcessingCheckbox, limitFramesCheckbox, controlCinemaVeriteCheckbox, brightnessCaption, contrastCaption, saturationCaption, redScaleCaption, greenScaleCaption, blueScaleCaption, redOffsetCaption, greenOffsetCaption, blueOffsetCaption, vignetteCaption, noiseCaption, brightnessSlider, contrastSlider, saturationSlider, redScaleSlider, greenScaleSlider, blueScaleSlider, redOffsetSlider, greenOffsetSlider, blueOffsetSlider, vignetteSlider, noiseSlider;

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
        usleep((useconds_t) (t * 1000000.0f));
    
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
        usleep((useconds_t) (t * 1000000.0f));
    
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
    int mouseButtonDown = 0;
#ifdef APL
    mouseButtonDown = CGEventSourceButtonState(kCGEventSourceStateCombinedSessionState, kCGMouseButtonLeft);
#elif IBM
    // TODO implement for Windows
#elif LIN
    // TODO implement for Linux
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

// draw-callback that adds post-processing
static int PostProcessingCallback(
                                  XPLMDrawingPhase     inPhase,
                                  int                  inIsBefore,
                                  void *               inRefcon)
{
    int x, y;
	XPLMGetScreenSize(&x, &y);
    
    glUseProgram(program);
    
	if(textureId == 0)
	{
		XPLMGenerateTextureNumbers((int *) &textureId, 1);
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, textureId);
		//XPLMBindTexture2d(tex_id, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	} else {
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, textureId);
        //		XPLMBindTexture2d(tex_id, 0);
    }
    
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, x, y);
	XPLMSetGraphicsState(0, 1, 0, 0, 0,  0, 0);
    
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

    int randomLocation = glGetUniformLocation(program, "random");
    glUniform2f(randomLocation, XPLMGetElapsedTime(), (float) XPLMGetCycleNumber());
    
    int noiseLocation = glGetUniformLocation(program, "noise");
    glUniform1f(noiseLocation, noise);
    
    int sceneLocation = glGetUniformLocation(program, "scene");
    glUniform1i(sceneLocation, 0);
    
	glPushAttrib(GL_VIEWPORT_BIT);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, x, 0, y, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glViewport(0, 0, x, y);
    
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
	glTexCoord2f(0,0);			glVertex2f(0,0);
	glTexCoord2f(0,1);			glVertex2f(0,y);
	glTexCoord2f(1,1);			glVertex2f(x,y);
	glTexCoord2f(1,0);			glVertex2f(x,0);
	glEnd();
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
    
    glUseProgram(0);
	
	return 1;
}

// function to load, compile and link the fragment-shader
GLint InitShader(const GLchar *fragmentShaderString)
{
    program = glCreateProgram();
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderString, 0);
    glCompileShader(fragmentShader);
    glAttachShader(program, fragmentShader);
    GLint isFragmentShaderCompiled = GL_FALSE;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isFragmentShaderCompiled);
    
    GLsizei maxLength = 2048;
    GLchar fragmentErrorLog[maxLength];
    glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, fragmentErrorLog);
    XPLMDebugString("FRAGMENT SHADER LOG:\n");
    XPLMDebugString(fragmentErrorLog);
    
    glLinkProgram(program);
    GLint isProgramLinked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, (int *)&isProgramLinked);
    
    glDetachShader(program, fragmentShader);
    
    char out[512];
    sprintf(out, ">>>>>>>>>>>>>>>> SHADER STATE: %d %d\n", isFragmentShaderCompiled, isProgramLinked);
    XPLMDebugString(out);
    
    return isProgramLinked;
}

// handles the settings widget
int SettingsWidgetHandler(XPWidgetMessage inMessage, XPWidgetID inWidget, long inParam1, long inParam2)
{
	if (inMessage == xpMessage_CloseButtonPushed)
	{
		if (settingsWindowOpen == 1)
		{
			// save config to file
			std::fstream file;
#if IBM
			file.open(".\\Resources\\plugins\\BLU-fx\\blu_fx.ini", std::ios_base::out | std::ios_base::trunc);
#else
			file.open("./Resources/plugins/BLU-fx/blu_fx.ini", std::ios_base::out | std::ios_base::trunc);
#endif
			if(file.is_open())
			{
				/*file << "gmax=" << gmax << std::endl;
                 file << "gmin=" << gmin << std::endl;
                 file << "aoa_max=" << aoa_max << std::endl;
                 file << "aoa_min=" << aoa_min << std::endl;
                 file << "aoa_demand=" << aoa_demand << std::endl;
                 file << "x_ptch=" << x_ptch << std::endl;
                 file << "x_roll=" << x_roll << std::endl;
                 file << "x_yaw=" << x_yaw << std::endl;*/
				file.close();
			}
            
			XPHideWidget(settingsWidget);
		}
        
		return 1;
	}
    else if (inMessage == xpMsg_ButtonStateChanged)
    {
        if (inParam1 == (long) postProcessingCheckbox)
        {
            postProcesssingEnabled = XPGetWidgetProperty(postProcessingCheckbox, xpProperty_ButtonState, 0);
            
            if (postProcesssingEnabled == 0)
                XPLMUnregisterDrawCallback(PostProcessingCallback, xplm_Phase_Window, 1, NULL);
            else
                XPLMRegisterDrawCallback(PostProcessingCallback, xplm_Phase_Window, 1, NULL);
            
        }
        else if (inParam1 == (long) limitFramesCheckbox)
        {
            limitFramesEnabled = XPGetWidgetProperty(limitFramesCheckbox, xpProperty_ButtonState, 0);
            
            if (limitFramesEnabled == 0)
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
            controlCinemaVeriteEnabled = XPGetWidgetProperty(controlCinemaVeriteCheckbox, xpProperty_ButtonState, 0);
            
            if (controlCinemaVeriteEnabled == 0)
                XPLMUnregisterFlightLoopCallback(ControlCinemaVeriteCallback, NULL);
            else
                XPLMRegisterFlightLoopCallback(ControlCinemaVeriteCallback, -1, NULL);
            
        }
    }
	else if (inMessage == xpMsg_ScrollBarSliderPositionChanged)
	{
        if (inParam1 == (long) brightnessSlider)
        {
            brightness = XPGetWidgetProperty(brightnessSlider, xpProperty_ScrollBarSliderPosition, 0) / 1000.0f;
            char stringBrigthness[32];
            sprintf(stringBrigthness, "Brightness: %.2f", brightness);
            XPSetWidgetDescriptor(brightnessCaption, stringBrigthness);
        }
        else if (inParam1 == (long) contrastSlider)
        {
            contrast = XPGetWidgetProperty(contrastSlider, xpProperty_ScrollBarSliderPosition, 0) / 100.0f;
            char stringContrast[32];
            sprintf(stringContrast, "Contrast: %.2f", contrast);
            XPSetWidgetDescriptor(contrastCaption, stringContrast);
        }
        else if (inParam1 == (long) saturationSlider)
        {
            saturation = XPGetWidgetProperty(saturationSlider, xpProperty_ScrollBarSliderPosition, 0) / 100.0f;
            char stringSaturation[32];
            sprintf(stringSaturation, "Saturation: %.2f", saturation);
            XPSetWidgetDescriptor(saturationCaption, stringSaturation);
        }
        else if (inParam1 == (long) redScaleSlider)
        {
            redScale = XPGetWidgetProperty(redScaleSlider, xpProperty_ScrollBarSliderPosition, 0) / 100.0f;
            char stringRedScale[32];
            sprintf(stringRedScale, "Red Scale: %.2f", redScale);
            XPSetWidgetDescriptor(redScaleCaption, stringRedScale);
        }
        else if (inParam1 == (long) greenScaleSlider)
        {
            greenScale = XPGetWidgetProperty(greenScaleSlider, xpProperty_ScrollBarSliderPosition, 0) / 100.0f;
            char stringGreenScale[32];
            sprintf(stringGreenScale, "Green Scale: %.2f", greenScale);
            XPSetWidgetDescriptor(greenScaleCaption, stringGreenScale);
        }
        else if (inParam1 == (long) blueScaleSlider)
        {
            blueScale = XPGetWidgetProperty(blueScaleSlider, xpProperty_ScrollBarSliderPosition, 0) / 100.0f;
            char stringBlueScale[32];
            sprintf(stringBlueScale, "Blue Scale: %.2f", blueScale);
            XPSetWidgetDescriptor(blueScaleCaption, stringBlueScale);
        }
        else if (inParam1 == (long) redOffsetSlider)
        {
            redOffset = XPGetWidgetProperty(redOffsetSlider, xpProperty_ScrollBarSliderPosition, 0) / 100.0f;
            char stringRedOffset[32];
            sprintf(stringRedOffset, "Red Offset: %.2f", redOffset);
            XPSetWidgetDescriptor(redOffsetCaption, stringRedOffset);
        }
        else if (inParam1 == (long) greenOffsetSlider)
        {
            greenOffset = XPGetWidgetProperty(greenOffsetSlider, xpProperty_ScrollBarSliderPosition, 0) / 100.0f;
            char stringGreenOffset[32];
            sprintf(stringGreenOffset, "Green Offset: %.2f", greenOffset);
            XPSetWidgetDescriptor(greenOffsetCaption, stringGreenOffset);
        }
        else if (inParam1 == (long) blueOffsetSlider)
        {
            blueOffset = XPGetWidgetProperty(blueOffsetSlider, xpProperty_ScrollBarSliderPosition, 0) / 100.0f;
            char stringBlueOffset[32];
            sprintf(stringBlueOffset, "Blue Offset: %.2f", blueOffset);
            XPSetWidgetDescriptor(blueOffsetCaption, stringBlueOffset);
        }
        else if (inParam1 == (long) vignetteSlider)
        {
            vignette = XPGetWidgetProperty(vignetteSlider, xpProperty_ScrollBarSliderPosition, 0) / 100.0f;
            char stringVignette[32];
            sprintf(stringVignette, "Vignette: %.2f", vignette);
            XPSetWidgetDescriptor(vignetteCaption, stringVignette);
        }
        else if (inParam1 == (long) noiseSlider)
        {
            noise = XPGetWidgetProperty(noiseSlider, xpProperty_ScrollBarSliderPosition, 0) / 100.0f;
            char stringNoise[32];
            sprintf(stringNoise, "Noise: %.2f", noise);
            XPSetWidgetDescriptor(noiseCaption, stringNoise);
        }
        
		return 1;
	}
    
    return 0;
}

// creates the settings widget
void CreateSettingsWidget(int x, int y, int w, int h)
{
	int x2 = x + w;
	int y2 = y - h;
    
	// widget window
	settingsWidget = XPCreateWidget(x, y, x2, y2, 1, "BLU-fx Settings", 1, 0, xpWidgetClass_MainWindow);
    
	// add close box
	XPSetWidgetProperty(settingsWidget, xpProperty_MainWindowHasCloseBoxes, 1);
    
    // add post-processing settings caption
    XPCreateWidget(x + 10, y - 30, x2 - 20, y - 45, 1, "Post-Processing Settings:", 0, settingsWidget, xpWidgetClass_Caption);
    
    // add post-processing checkbox
    postProcessingCheckbox = XPCreateWidget(x + 20, y - 50, x2 - 20, y - 65, 1, "Enable Post-Processing", 0, settingsWidget, xpWidgetClass_Button);
    XPSetWidgetProperty(postProcessingCheckbox, xpProperty_ButtonType, xpRadioButton);
    XPSetWidgetProperty(postProcessingCheckbox, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);
    XPSetWidgetProperty(postProcessingCheckbox, xpProperty_ButtonState, postProcesssingEnabled);
    
	// add brightness caption
    char stringBrigthness[32];
    sprintf(stringBrigthness, "Brightness: %.2f", brightness);
	brightnessCaption = XPCreateWidget(x + 30, y - 70, x2 - 50, y - 85, 1, stringBrigthness, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add brightness slider
	brightnessSlider = XPCreateWidget(x + 195, y - 70, x2 - 15, y - 85, 1, "Brightness", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(brightnessSlider, xpProperty_ScrollBarMin, -500);
	XPSetWidgetProperty(brightnessSlider, xpProperty_ScrollBarMax, 500);
	XPSetWidgetProperty(brightnessSlider, xpProperty_ScrollBarSliderPosition, brightness * 1000.0f);
    
    // add contrast caption
    char stringContrast[32];
    sprintf(stringContrast, "Contrast: %.2f", contrast);
	contrastCaption = XPCreateWidget(x + 30, y - 90, x2 - 50, y - 105, 1, stringContrast, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add contrast slider
	contrastSlider = XPCreateWidget(x + 195, y - 90, x2 - 15, y - 105, 1, "Contrast", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(contrastSlider, xpProperty_ScrollBarMin, 0);
	XPSetWidgetProperty(contrastSlider, xpProperty_ScrollBarMax, 300);
	XPSetWidgetProperty(contrastSlider, xpProperty_ScrollBarSliderPosition, contrast * 100.0f);
    
    // add saturation caption
    char stringSaturation[32];
    sprintf(stringSaturation, "Saturation: %.2f", saturation);
	saturationCaption = XPCreateWidget(x + 30, y - 110, x2 - 50, y - 125, 1, stringSaturation, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add saturation slider
	saturationSlider = XPCreateWidget(x + 195, y - 110, x2 - 15, y - 125, 1, "Saturation", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(saturationSlider, xpProperty_ScrollBarMin, 0);
	XPSetWidgetProperty(saturationSlider, xpProperty_ScrollBarMax, 500);
	XPSetWidgetProperty(saturationSlider, xpProperty_ScrollBarSliderPosition, saturation * 100.0f);
    
    // add red scale caption
    char stringRedScale[32];
    sprintf(stringRedScale, "Red Scale: %.2f", redScale);
	redScaleCaption = XPCreateWidget(x + 30, y - 130, x2 - 50, y - 145, 1, stringRedScale, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add red scale slider
	redScaleSlider = XPCreateWidget(x + 195, y - 130, x2 - 15, y - 145, 1, "Red Scale", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(redScaleSlider, xpProperty_ScrollBarMin, -100);
	XPSetWidgetProperty(redScaleSlider, xpProperty_ScrollBarMax, 100);
	XPSetWidgetProperty(redScaleSlider, xpProperty_ScrollBarSliderPosition, redScale * 100.0f);
    
    // add green scale caption
    char stringGreenScale[32];
    sprintf(stringGreenScale, "Green Scale: %.2f", greenScale);
	greenScaleCaption = XPCreateWidget(x + 30, y - 150, x2 - 50, y - 165, 1, stringGreenScale, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add green scale slider
	greenScaleSlider = XPCreateWidget(x + 195, y - 150, x2 - 15, y - 165, 1, "Green Scale", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(greenScaleSlider, xpProperty_ScrollBarMin, -100);
	XPSetWidgetProperty(greenScaleSlider, xpProperty_ScrollBarMax, 100);
	XPSetWidgetProperty(greenScaleSlider, xpProperty_ScrollBarSliderPosition, greenScale * 100.0f);
    
    // add blue scale caption
    char stringBlueScale[32];
    sprintf(stringBlueScale, "Blue Scale: %.2f", blueScale);
	blueScaleCaption = XPCreateWidget(x + 30, y - 170, x2 - 50, y - 185, 1, stringBlueScale, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add blue scale slider
	blueScaleSlider = XPCreateWidget(x + 195, y - 170, x2 - 15, y - 185, 1, "Blue Scale", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(blueScaleSlider, xpProperty_ScrollBarMin, -100);
	XPSetWidgetProperty(blueScaleSlider, xpProperty_ScrollBarMax, 100);
	XPSetWidgetProperty(blueScaleSlider, xpProperty_ScrollBarSliderPosition, blueScale * 100.0f);
    
    // add red offset caption
    char stringRedOffset[32];
    sprintf(stringRedOffset, "Red Offset: %.2f", redOffset);
	redOffsetCaption = XPCreateWidget(x + 30, y - 190, x2 - 50, y - 205, 1, stringRedOffset, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add red offset slider
	redOffsetSlider = XPCreateWidget(x + 195, y - 190, x2 - 15, y - 205, 1, "Red Offset", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(redOffsetSlider, xpProperty_ScrollBarMin, -100);
	XPSetWidgetProperty(redOffsetSlider, xpProperty_ScrollBarMax, 100);
	XPSetWidgetProperty(redOffsetSlider, xpProperty_ScrollBarSliderPosition, redOffset * 100.0f);
    
    // add green offset caption
    char stringGreenOffset[32];
    sprintf(stringGreenOffset, "Green Offset: %.2f", greenOffset);
	greenOffsetCaption = XPCreateWidget(x + 30, y - 210, x2 - 50, y - 225, 1, stringGreenOffset, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add green offset slider
	greenOffsetSlider = XPCreateWidget(x + 195, y - 210, x2 - 15, y - 225, 1, "Green Offset", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(greenOffsetSlider, xpProperty_ScrollBarMin, -100);
	XPSetWidgetProperty(greenOffsetSlider, xpProperty_ScrollBarMax, 100);
	XPSetWidgetProperty(greenOffsetSlider, xpProperty_ScrollBarSliderPosition, greenOffset * 100.0f);
    
    // add blue offset caption
    char stringBlueOffset[32];
    sprintf(stringBlueOffset, "Blue Offset: %.2f", blueOffset);
	blueOffsetCaption = XPCreateWidget(x + 30, y - 230, x2 - 50, y - 245, 1, stringBlueOffset, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add blue offset slider
	blueOffsetSlider = XPCreateWidget(x + 195, y - 230, x2 - 15, y - 245, 1, "Blue Offset", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(blueOffsetSlider, xpProperty_ScrollBarMin, -100);
	XPSetWidgetProperty(blueOffsetSlider, xpProperty_ScrollBarMax, 100);
	XPSetWidgetProperty(blueOffsetSlider, xpProperty_ScrollBarSliderPosition, blueOffset * 100.0f);
    
    // add vignette caption
    char stringVignette[32];
    sprintf(stringVignette, "Vignette: %.2f", vignette);
	vignetteCaption = XPCreateWidget(x + 30, y - 250, x2 - 50, y - 265, 1, stringVignette, 0, settingsWidget, xpWidgetClass_Caption);
    
	// add vignette slider
	vignetteSlider = XPCreateWidget(x + 195, y - 250, x2 - 15, y - 265, 1, "Vignette", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(vignetteSlider, xpProperty_ScrollBarMin, 0);
	XPSetWidgetProperty(vignetteSlider, xpProperty_ScrollBarMax, 100);
	XPSetWidgetProperty(vignetteSlider, xpProperty_ScrollBarSliderPosition, vignette * 100.0f);
    
    // add noise caption
    char stringNoise[32];
    sprintf(stringNoise, "Noise: %.2f", noise);
	vignetteCaption = XPCreateWidget(x + 30, y - 270, x2 - 50, y - 285, 1, stringNoise, 0, settingsWidget, xpWidgetClass_Caption);
    
    // add noise slider
	noiseSlider = XPCreateWidget(x + 195, y - 270, x2 - 15, y - 285, 1, "Noise", 0, settingsWidget, xpWidgetClass_ScrollBar);
	XPSetWidgetProperty(noiseSlider, xpProperty_ScrollBarMin, 0);
	XPSetWidgetProperty(noiseSlider, xpProperty_ScrollBarMax, 500);
	XPSetWidgetProperty(noiseSlider, xpProperty_ScrollBarSliderPosition, noise * 100.0f);
    
	// register widget handler
	XPAddWidgetCallback(settingsWidget, SettingsWidgetHandler);
}

// handles the about widget
int AboutWidgetHandler(XPWidgetMessage inMessage, XPWidgetID inWidget, long inParam1, long inParam2)
{
	if (inMessage == xpMessage_CloseButtonPushed)
	{
		if (aboutWindowOpen == 1)
			XPHideWidget(aboutWidget);
        
		return 1;
	}
    
	return 0;
}

// creates the about widget
void CreateAboutWidget(int x, int y, int w, int h)
{
	int x2 = x + w;
	int y2 = y - h;
    
	// widget window
	aboutWidget = XPCreateWidget(x, y, x2, y2, 1, "About BLU-fx", 1, 0, xpWidgetClass_MainWindow);
    
	// add close box
	XPSetWidgetProperty(aboutWidget, xpProperty_MainWindowHasCloseBoxes, 1);
    
	// add caption widgets
	XPCreateWidget(x + 30, y - 30, x2 - 30, y - 45, 1, "BLU-fx Plugin", 0, aboutWidget, xpWidgetClass_Caption);
	XPCreateWidget(x + 30, y - 70, x2 - 30, y - 85, 1, "by Matteo Hausner", 0, aboutWidget, xpWidgetClass_Caption);
    char vers[16];
	sprintf(vers, "Version: %s", VERSION);
	XPCreateWidget(x + 30, y - 110, x2 - 30, y - 125, 1, vers, 0, aboutWidget, xpWidgetClass_Caption);
    
	// register widget handler
	XPAddWidgetCallback(aboutWidget, AboutWidgetHandler);
}

// handles the menu-entries
void MenuHandlerCallback(void* inMenuRef, void* inItemRef)
{
	// settings menu entry
	if ((long) inItemRef == 0)
	{
		if (settingsWindowOpen == 0) // settings not open yet
		{
			CreateSettingsWidget(600, 600, 350, 405);
			settingsWindowOpen = 1;
		}
		else // settings already open
		{
			if (!XPIsWidgetVisible(settingsWidget))
                XPShowWidget(settingsWidget);
		}
	}
    
	// about menu entry
	else if ((long) inItemRef == 1)
	{
		if (aboutWindowOpen == 0) // about not open yet
		{
			CreateAboutWidget(600, 600, 180, 155);
			aboutWindowOpen = 1;
		}
		else // about already open
		{
			if (!XPIsWidgetVisible(aboutWidget))
                XPShowWidget(aboutWidget);
		}
	}
}

PLUGIN_API int XPluginStart(
                            char *		outName,
                            char *		outSig,
                            char *		outDesc)
{
    // set plugin info
	strcpy(outName, "BLU-fx");
	strcpy(outSig, "de.bwravencl.blu_fx");
	strcpy(outDesc, "BLU-fx enhances your X-Plane experience!");
    
    // prepare fragment-shader
    InitShader("#version 120\nconst vec3 lumCoeff = vec3(0.2125, 0.7154, 0.0721); uniform float brightness; uniform float contrast; uniform float saturation; uniform float redScale; uniform float greenScale; uniform float blueScale; uniform float redOffset; uniform float greenOffset; uniform float blueOffset; uniform vec2 resolution; uniform float vignette; uniform vec2 random; uniform float noise; uniform sampler2D scene; void main() { vec3 color = texture2D(scene, gl_TexCoord[0].st).rgb; vec3 colorContrasted = (color) * contrast; vec3 bright = colorContrasted + vec3(brightness,brightness,brightness); vec3 intensity = vec3(dot(bright, lumCoeff)); vec3 col = mix(intensity, bright, saturation); vec3 newColor = (col.rgb - 0.5) * 2.0; newColor.r = 2.0/3.0 * (1.0 - (newColor.r * newColor.r)); newColor.g = 2.0/3.0 * (1.0 - (newColor.g * newColor.g)); newColor.b = 2.0/3.0 * (1.0 - (newColor.b * newColor.b)); newColor.r = clamp(col.r + redScale * newColor.r + redOffset, 0.0, 1.0); newColor.g = clamp(col.g + greenScale * newColor.g + greenOffset, 0.0, 1.0); newColor.b = clamp(col.b + blueScale * newColor.b + blueOffset, 0.0, 1.0); vec2 position = (gl_FragCoord.xy / resolution.xy) - vec2(0.5); float len = length(position); float vig = smoothstep(0.75, 0.75-0.45, len); newColor = mix(newColor, newColor * vig, vignette); float rand = 0.5 + 0.5 * fract(sin(dot(random.xy, vec2(12.9898, 78.233)))* 43758.5453); newColor = mix(newColor, newColor * rand, noise);  gl_FragColor = vec4(newColor, 1.0); }");
    
    // obtain datarefs
    cinemaVeriteDataRef = XPLMFindDataRef("sim/graphics/view/cinema_verite");
    viewTypeDataRef = XPLMFindDataRef("sim/graphics/view/view_type");
    
    // create menu-entries
	int SubMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "BLU-fx", 0, 1);
	XPLMMenuID Menu = XPLMCreateMenu("BLU-fx", XPLMFindPluginsMenu(), SubMenuItem, MenuHandlerCallback, 0);
	XPLMAppendMenuItem(Menu, "Settings", (void*) 0, 1); // settings menu entry with ItemRef = 0
	XPLMAppendMenuItem(Menu, "About", (void*) 1, 1); // about menu entry with ItemRef = 1
    
    // register flightloop-callbacks
    if (limitFramesEnabled == 1)
        XPLMRegisterFlightLoopCallback(LimiterFlightCallback, -1, NULL);
    if (controlCinemaVeriteEnabled == 1)
        XPLMRegisterFlightLoopCallback(ControlCinemaVeriteCallback, -1, NULL);
    
    // register draw-callbacks
    if (limitFramesEnabled == 1)
        XPLMRegisterDrawCallback(LimiterDrawCallback, xplm_Phase_Terrain, 1, NULL);
    if (postProcesssingEnabled == 1)
        XPLMRegisterDrawCallback(PostProcessingCallback, xplm_Phase_Window, 1, NULL);
    
	return 1;
}


PLUGIN_API void	XPluginStop(void)
{
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
