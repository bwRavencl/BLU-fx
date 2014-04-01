#include "XPLMDataAccess.h"
#include "XPLMDefs.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"

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

#define FRAME_RATE 35.0f

#define DISABLE_CINEMA_VERITE_TIME 5.0f

static int lastMouseX = 0, lastMouseY = 0;
static float startTimeFlight = 0.0f, endTimeFlight = 0.0f, startTimeDraw = 0.0f, endTimeDraw = 0.0f, lastMouseMovementTime = 0.0f;
static XPLMDataRef cinemaVeriteDataRef, viewTypeDataRef;
static GLuint tex_id = 0, program;

float LimiterFlightCallback(
                            float                inElapsedSinceLastCall,
                            float                inElapsedTimeSinceLastFlightLoop,
                            int                  inCounter,
                            void *               inRefcon)
{
    endTimeFlight = XPLMGetElapsedTime();
    float dt = endTimeFlight - startTimeFlight;
    
    float t = 1.0f / FRAME_RATE - dt;
    
    if(t > 0.0f)
        usleep((useconds_t) (t * 1000000.0f));
    
    startTimeFlight = XPLMGetElapsedTime();
    
    return -1.0f;
}

static int LimiterDrawCallback(
                               XPLMDrawingPhase     inPhase,
                               int                  inIsBefore,
                               void *               inRefcon)
{
    endTimeDraw = XPLMGetElapsedTime();
    float dt = endTimeDraw - startTimeDraw;
    
    float t = 1.0f / FRAME_RATE - dt;
    
    if(t > 0.0f)
        usleep((useconds_t) (t * 1000000.0f));
    
    startTimeDraw = XPLMGetElapsedTime();
    
    return 1;
}

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
        
        if (mouseButtonDown || elapsedTime <= DISABLE_CINEMA_VERITE_TIME)
            XPLMSetDatai(cinemaVeriteDataRef, 0);
        else
            XPLMSetDatai(cinemaVeriteDataRef, 1);
    }
    
    return -1.0f;
}

static int RipAndRedrawCallback(
                                XPLMDrawingPhase     inPhase,
                                int                  inIsBefore,
                                void *               inRefcon)
{
    int x, y;
	XPLMGetScreenSize(&x, &y);
    
    glUseProgram(program);
    
	if(tex_id == 0)
	{
		XPLMGenerateTextureNumbers((int *) &tex_id, 1);
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, tex_id);
		//XPLMBindTexture2d(tex_id, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	} else {
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, tex_id);
//		XPLMBindTexture2d(tex_id, 0);
    }
    
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, x, y);
	XPLMSetGraphicsState(0, 1, 0, 0, 0,  0, 0);
    
    int brightnessLocation = glGetUniformLocation(program, "brightness");
    glUniform1f(brightnessLocation, 0.0f);
    
    int contrastLocation = glGetUniformLocation(program, "contrast");
    glUniform1f(contrastLocation, 1.5f);
    
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

GLint initShader(const GLchar *vertexShaderString, const GLchar *fragmentShaderString)
{
    program = glCreateProgram();

    /*GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderString, 0);
    glCompileShader(vertexShader);
    glAttachShader(program, vertexShader);
    GLint isVertexShaderCompiled = GL_FALSE;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isVertexShaderCompiled);*/
    
    GLsizei maxLength = 2048;
    /*GLchar vertexErrorLog[maxLength];
    glGetShaderInfoLog(vertexShader, maxLength, &maxLength, vertexErrorLog);
    XPLMDebugString("VERTEX SHADER LOG:\n");
    XPLMDebugString(vertexErrorLog);*/
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderString, 0);
    glCompileShader(fragmentShader);
    glAttachShader(program, fragmentShader);
    GLint isFragmentShaderCompiled = GL_FALSE;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isFragmentShaderCompiled);
    
    GLchar fragmentErrorLog[maxLength];
    glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, fragmentErrorLog);
    XPLMDebugString("FRAGMENT SHADER LOG:\n");
    XPLMDebugString(fragmentErrorLog);
    
    glLinkProgram(program);
    GLint isProgramLinked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, (int *)&isProgramLinked);
    
    /*glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);*/
    
    char out[512];
    sprintf(out, ">>>>>>>>>>>>>>>> SHADER STATE: %d %d %d\n", 0/*isVertexShaderCompiled*/, isFragmentShaderCompiled, isProgramLinked);
    XPLMDebugString(out);
    
    return isProgramLinked;
}

PLUGIN_API int XPluginStart(
                            char *		outName,
                            char *		outSig,
                            char *		outDesc)
{
    
	strcpy(outName, "SuperPlugin");
	strcpy(outSig, "de.bwravencl.superplugin");
	strcpy(outDesc, "Enhances your X-Plane experience!");
    
    initShader("", "#version 120\nuniform float brightness; uniform float contrast; uniform sampler2D scene; void main() { vec3 color = texture2D(scene, gl_TexCoord[0].st).rgb; vec3 colorContrasted = (color) * contrast; vec3 bright = colorContrasted + vec3(brightness,brightness,brightness); gl_FragColor.rgb = bright; }");
    
    cinemaVeriteDataRef = XPLMFindDataRef("sim/graphics/view/cinema_verite");
    viewTypeDataRef = XPLMFindDataRef("sim/graphics/view/view_type");
    
    XPLMRegisterFlightLoopCallback(LimiterFlightCallback, -1, NULL);
    XPLMRegisterFlightLoopCallback(ControlCinemaVeriteCallback, -1, NULL);
    
    XPLMRegisterDrawCallback(LimiterDrawCallback, xplm_Phase_Terrain, 1, NULL);
    XPLMRegisterDrawCallback(RipAndRedrawCallback, xplm_Phase_Window, 1, NULL);
    
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
