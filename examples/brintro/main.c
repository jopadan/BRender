#include <brdemo.h>
#include "intro.h"

static br_error IntroInit(br_demo *demo)
{
    br_intro *intro;

    if((intro = BrIntroCreate()) == NULL) {
        BrLogError("DEMO", "Error loading intro resources.");
        return BRE_FAIL;
    }

    if(demo->colour_buffer->type == BR_PMT_INDEX_8) {
        BrPixelmapPaletteSet(demo->colour_buffer, BrMapFind("intro_palette"));
    }

    BrIntroSetAspect(intro, BR_DIV(BR_SCALAR(demo->colour_buffer->width), BR_SCALAR(demo->colour_buffer->height)));

    demo->user   = intro;
    demo->camera = BrIntroGetCamera(intro);
    BrActorAdd(demo->world, BrIntroGetWorld(intro));

    return BRE_OK;
}

static void IntroUpdate(br_demo *demo, br_scalar dt)
{
    br_intro *intro = demo->user;
    BrIntroUpdate(intro, dt);
}

static void IntroDestroy(br_demo *demo)
{
    BrIntroDestroy(demo->user);
}

const static br_demo_dispatch dispatch = {
    .init          = IntroInit,
    .process_event = BrDemoDefaultProcessEvent,
    .update        = IntroUpdate,
    .render        = BrDemoDefaultRender,
    .on_resize     = BrDemoDefaultOnResize,
    .destroy       = IntroDestroy,
};

int main(int argc, char **argv)
{
    return BrDemoRunArgv("BRender Intro Sequence", &dispatch, argc, argv);
}
