#include "drv.h"

br_token brt_sdl_surface_h = BRT_NONE;

/*
 * Main entry point for device - this may get redefined by the build system.
 */
br_device *BR_EXPORT BrDrv1SDL3Begin(const char *arguments)
{
    br_device *device;
    (void)arguments;

    /*
     * Set up device
     */
    if((device = DeviceSDL3Allocate("SDL3")) == NULL)
        return NULL;

    brt_sdl_surface_h = BrTokenCreate("SDL_SURFACE_H", BR_NULL_TOKEN);

    if(OutputFacilitySDL3CreateGeneric(device) == NULL) {
        /*
         * If nothing is available, then don't admit to being a device
         */
        ObjectFree(device);
        return NULL;
    }

    return device;
}

#ifdef DEFINE_BR_ENTRY_POINT
br_device *BR_EXPORT BrDrv1Begin(const char *arguments)
{
    return BrDrv1SDL3Begin(arguments);
}
#endif
