#include <string.h>
#include <unistd.h>

#include "XPLMDataAccess.h"
#include "XPLMDefs.h"
#include "XPLMDisplay.h"
#include "XPLMProcessing.h"

#define FRAME_RATE 33.0f
#define DISABLE_CINEMA_VERITE_TIME 5.0f

static int lastMouseX = 0, lastMouseY = 0;
static float startTimeFlight = 0.0f, endTimeFlight = 0.0f, startTimeDraw = 0.0f, endTimeDraw = 0.0f, lastMouseMovementTime = 0.0f;
static XPLMDataRef cinemaVeriteDataRef, viewTypeDataRef;

float FlightCallback(
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

static int DrawCallback(
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
    float currentTime = XPLMGetElapsedTime();
    
    int currentMouseX, currentMouseY;
    XPLMGetMouseLocation(&currentMouseX, &currentMouseY);
    
    if (currentMouseX != lastMouseX || currentMouseY != lastMouseY) {
        lastMouseX = currentMouseX;
        lastMouseY = currentMouseY;
        
        lastMouseMovementTime = XPLMGetElapsedTime();
    }
    
    int viewType = XPLMGetDatai(viewTypeDataRef);
    
    if (viewType != 1026) // not 3D Cockpit+
        XPLMSetDatai(cinemaVeriteDataRef, 1);
    else
    {
        float elapsedTime = currentTime - lastMouseMovementTime;
        
        if (elapsedTime <= DISABLE_CINEMA_VERITE_TIME)
            XPLMSetDatai(cinemaVeriteDataRef, 0);
        else
            XPLMSetDatai(cinemaVeriteDataRef, 1);
    }
    
    return 0.25f;
}

PLUGIN_API int XPluginStart(
						char *		outName,
						char *		outSig,
						char *		outDesc)
{

	strcpy(outName, "FPS Limiter");
	strcpy(outSig, "de.bwravencl.fps_limiter");
	strcpy(outDesc, "Limits the frame rate.");

    cinemaVeriteDataRef = XPLMFindDataRef("sim/graphics/view/cinema_verite");
    viewTypeDataRef = XPLMFindDataRef("sim/graphics/view/view_type");
    
    XPLMRegisterFlightLoopCallback(FlightCallback, -1, NULL);
    XPLMRegisterFlightLoopCallback(ControlCinemaVeriteCallback, -1, NULL);
    XPLMRegisterDrawCallback(DrawCallback, xplm_Phase_Terrain, 1, NULL);

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
