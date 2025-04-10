/* A simple real time physical simulation */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <brdemo.h>

#define SPF (1.0f / 60.0f)

#define S0  BR_SCALAR(0)
#define S1  BR_SCALAR(1)

/* Specify number of vertices in sheet */

#define SHEETX              9
#define SHEETY              9
#define XSTART              BR_SCALAR(-0.8)
#define YSTART              BR_SCALAR(-0.8)

#define VERTEX(model, i, j) (model->vertices[(i) + (j) * SHEETX])

struct br_vertex vertices[SHEETX * SHEETY];
struct br_face   faces[4 * (SHEETX - 1) * (SHEETY - 1)];

enum {
    S_FIXED = 1
};

br_vector3 velocity[SHEETX][SHEETY];
br_vector3 position[SHEETX][SHEETY];
br_vector3 spring_extension[SHEETX][SHEETY][2];
enum {
    SPRING_X,
    SPRING_Z
};

int Frozen = 0;

#define FORCE_CONSTANT  BR_SCALAR(0.015)
#define NATURAL_LENGTH  BR_SCALAR(0.18)
#define NATURAL_LENGTH2 BR_SCALAR(0.18 * 0.18)
#define MAX_LENGTH      BR_SCALAR(0.20)
#define GRAVITY         BR_SCALAR(-0.0001)
#define FRICTION        BR_SCALAR(0.3)
#define CYCLES          20
#define RADIUS          BR_SCALAR(0.3)
#define RADIUS2         BR_SCALAR(0.3 * 0.3)
#define FORCE           BR_SCALAR(0.7)
#define PLANE_FORCE     BR_SCALAR(0.3)
#define DAMPING         BR_SCALAR(0.8)
#define DT              BR_SCALAR(1)

void InitialiseSheetModel(br_model *model)
{
    int          i, j;
    br_vertex   *vertex;
    br_face     *face;
    br_material *design_1, *design_2;

    design_1 = BrMaterialFind("plain_red");
    design_2 = BrMaterialFind("sheet_top");

    vertex = model->vertices;

    for(j = 0; j < SHEETY; j++)
        for(i = 0; i < SHEETX; i++) {
            vertex->p.v[0] = i * NATURAL_LENGTH + XSTART;
            vertex->p.v[1] = BR_SCALAR(0.9);
            vertex->p.v[2] = j * NATURAL_LENGTH + YSTART;

            vertex->map.v[0] = BR_SCALAR((float)i / SHEETX);
            vertex->map.v[1] = BR_SCALAR((float)j / SHEETY);

            BrVector3Copy(&position[i][j], &vertex->p);

            vertex++;
        }

    BrVector3Set(&vertices[(SHEETY - 1) * SHEETX].p, BR_SCALAR(-0.5), BR_SCALAR(-0.5), BR_SCALAR(-0.5));

    BrVector3Set(&vertices[SHEETY - 1 + (SHEETY - 1) * SHEETX].p, BR_SCALAR(-0.5), BR_SCALAR(-0.5), BR_SCALAR(-0.5));

    face = model->faces;

    /* Position vertices of rubber sheet */
    /* The reverse side of each face uses a different texture */

    for(i = 0; i < SHEETX - 1; i++)
        for(j = 0; j < SHEETY - 1; j++) {

            face->material = design_1;

            face->vertices[0] = i + j * SHEETX;
            face->vertices[1] = i + 1 + j * SHEETX;
            face->vertices[2] = i + 1 + (j + 1) * SHEETX;
            face->smoothing   = 1;

            face++;

            face->material = design_1;

            face->vertices[0] = i + j * SHEETX;
            face->vertices[1] = i + 1 + (j + 1) * SHEETX;
            face->vertices[2] = i + (j + 1) * SHEETX;
            face->smoothing   = 1;

            face++;

            face->material = design_2;

            face->vertices[0] = i + j * SHEETX;
            face->vertices[2] = i + 1 + j * SHEETX;
            face->vertices[1] = i + 1 + (j + 1) * SHEETX;
            face->smoothing   = 2;

            face++;

            face->material = design_2;

            face->vertices[0] = i + j * SHEETX;
            face->vertices[2] = i + 1 + (j + 1) * SHEETX;
            face->vertices[1] = i + (j + 1) * SHEETX;
            face->smoothing   = 2;

            face++;
        }
}

/* The centres of the three sphere */

br_vector3 centre1 = BR_VECTOR3(0.4, 0.4, 0.4);
br_vector3 centre2 = BR_VECTOR3(0, 0, 0);
br_vector3 centre3 = BR_VECTOR3(-0.4, -0.4, -0.4);

br_vector3 normal1 = BR_VECTOR3(1, 0, 0);
br_vector3 normal2 = BR_VECTOR3(0, 1, 0);
br_vector3 normal3 = BR_VECTOR3(0, 0, 1);
br_vector3 normal4 = BR_VECTOR3(-1, 0, 0);
br_vector3 normal5 = BR_VECTOR3(0, -1, 0);
br_vector3 normal6 = BR_VECTOR3(0, 0, -1);
br_scalar  k1      = BR_SCALAR(-1);
br_scalar  k2      = BR_SCALAR(-1);
br_scalar  k3      = BR_SCALAR(-1);
br_scalar  k4      = BR_SCALAR(-1);
br_scalar  k5      = BR_SCALAR(-1);
br_scalar  k6      = BR_SCALAR(-1);

/* Calculate force due to fields centred on spheres. Force is only */
/* applied to points within a threshold radius */
/* Return 1 if point is within threshold */

int AccumulateSphereField(br_vector3 *a, br_vector3 *pos, br_vector3 *centre)
{
    br_vector3 tmp1, tmp2;
    br_scalar  length;

    BrVector3Sub(&tmp1, pos, centre);
    length = BrVector3LengthSquared(&tmp1);
    if(length < RADIUS2) {

        /* Force is proportional to square of distance over threshold */

        length = BR_SUB(RADIUS, BR_SQRT(length));
        BrVector3Normalise(&tmp2, &tmp1);
        BrVector3Scale(&tmp1, &tmp2, FORCE);
        BrVector3Scale(&tmp2, &tmp1, length);
        BrVector3Accumulate(a, &tmp2);
        return 1;
    }
    return 0;
}

int AccumulatePlaneField(br_vector3 *a, br_vector3 *pos, br_vector3 *normal, br_scalar k)
{
    br_vector3 tmp1, tmp2;
    br_scalar  length;

    length = BR_SUB(k, BrVector3Dot(normal, pos));
    if(length > 0) {

        /* Force is proportional to distance over threshold */

        BrVector3Scale(&tmp1, normal, PLANE_FORCE);
        BrVector3Scale(&tmp2, &tmp1, length);
        BrVector3Accumulate(a, &tmp2);
        return 1;
    }
    return 0;
}

br_vector3 init_gravity = {0, BR_SCALAR(1.0), 0};
br_vector3 gravity      = {0, BR_SCALAR(1.0), 0};

/* The sheet is made up of springs. CalculateSpring calculates the */
/* extension of each spring and the resultant force if it is stretched */
/* longer than its natural length */

int CalculateSpring(int i, int j, int d, br_vector3 *pos1, br_vector3 *pos2)
{
    br_vector3 displacement, natural;
    br_scalar  length;

    BrVector3Sub(&displacement, pos1, pos2);
    length = BrVector3LengthSquared(&displacement);
    if(length > NATURAL_LENGTH2) {
        BrVector3Normalise(&natural, &displacement);
        BrVector3Scale(&natural, &displacement, NATURAL_LENGTH);
        BrVector3Sub(&spring_extension[i][j][d], &displacement, &natural);
        return 1;
    }
    BrVector3Set(&spring_extension[i][j][d], 0, 0, 0);
    return 0;
}

void AccumulateForceFromSpring(br_vector3 *force, int i, int j, int d, int r)
{
    if(r > 0) {
        BrVector3Accumulate(force, &spring_extension[i][j][d]);
    } else {
        BrVector3Sub(force, force, &spring_extension[i][j][d]);
    }
}

/*
   The main dynamics routine.

   This is simple first order approximation hard coded to deal
   specifically with a rubber sheet.
*/

void UpdateDynamics(br_model *model, br_vector3 *gravity)
{
    int         i, j;
    br_vector3  force, current, acceleration, tmp;
    br_vertex  *vertex;
    br_scalar   length;
    br_matrix34 inverse;

    for(i = 0; i < SHEETX - 1; i++) {
        for(j = 0; j < SHEETY; j++) {
            CalculateSpring(i, j, 0, &VERTEX(model, i, j).p, &VERTEX(model, i + 1, j).p);
        }
    }

    for(i = 0; i < SHEETX; i++) {
        for(j = 0; j < SHEETY - 1; j++) {
            CalculateSpring(i, j, 1, &VERTEX(model, i, j).p, &VERTEX(model, i, j + 1).p);
        }
    }

    for(j = 0; j < SHEETY; j++)
        for(i = 0; i < SHEETX; i++) {
            vertex = &VERTEX(model, i, j);
            BrVector3Copy(&current, &vertex->p);

            BrVector3Set(&force, 0, 0, 0);

            if(i < SHEETX - 1)
                AccumulateForceFromSpring(&force, i, j, SPRING_X, -1);
            if(j < SHEETY - 1)
                AccumulateForceFromSpring(&force, i, j, SPRING_Z, -1);
            if(i > 0)
                AccumulateForceFromSpring(&force, i - 1, j, SPRING_X, 1);
            if(j > 0)
                AccumulateForceFromSpring(&force, i, j - 1, SPRING_Z, 1);

            BrVector3Scale(&acceleration, &force, FORCE_CONSTANT);
            if(AccumulateSphereField(&acceleration, &VERTEX(model, i, j).p, &centre1) ||
               AccumulateSphereField(&acceleration, &VERTEX(model, i, j).p, &centre2) ||
               AccumulateSphereField(&acceleration, &VERTEX(model, i, j).p, &centre3) ||
               (AccumulatePlaneField(&acceleration, &VERTEX(model, i, j).p, &normal1, k1) |
                AccumulatePlaneField(&acceleration, &VERTEX(model, i, j).p, &normal2, k2) |
                AccumulatePlaneField(&acceleration, &VERTEX(model, i, j).p, &normal3, k3) |
                AccumulatePlaneField(&acceleration, &VERTEX(model, i, j).p, &normal4, k4) |
                AccumulatePlaneField(&acceleration, &VERTEX(model, i, j).p, &normal5, k5) |
                AccumulatePlaneField(&acceleration, &VERTEX(model, i, j).p, &normal6, k6)))
                BrVector3Scale(&velocity[i][j], &velocity[i][j], FRICTION);

            BrVector3Scale(&tmp, gravity, GRAVITY);
            BrVector3Accumulate(&acceleration, &tmp);

            BrVector3Scale(&acceleration, &acceleration, DT);
            BrVector3Accumulate(&velocity[i][j], &acceleration);

            BrVector3Scale(&velocity[i][j], &velocity[i][j], DAMPING);
            BrVector3Scale(&tmp, &velocity[i][j], DT);
            BrVector3Add(&position[i][j], &current, &tmp);
        }

    for(i = 0; i < SHEETX; i++)
        for(j = 0; j < SHEETY; j++) {
            BrVector3Copy(&model->vertices[i + j * SHEETX].p, &position[i][j]);
        }
}

#define VIEW_RADIUS BR_SCALAR(0.26)

void PlaceSphere(br_actor *parent, br_model *model, br_vector3 *position, char *colour)
{
    br_actor *sphere;

    sphere           = BrActorAdd(parent, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    sphere->model    = model;
    sphere->material = BrMaterialFind(colour);
    sphere->t.type   = BR_TRANSFORM_MATRIX34;
    BrMatrix34Scale(&sphere->t.t.mat, VIEW_RADIUS, VIEW_RADIUS, VIEW_RADIUS);
    BrMatrix34PostTranslate(&sphere->t.t.mat, position->v[0], position->v[1], position->v[2]);
}

typedef struct br_demo_sheet {
    br_actor  *sheet;
    br_boolean frozen;
    float      accum;
} br_demo_sheet;

static br_error SheetInit(br_demo *demo)
{
    br_demo_sheet *sheet;
    br_actor      *pivot, *system, *box;
    br_model      *sphere_model;
    br_camera     *camera_data;
    br_actor      *light1, *light2;
    br_material   *mats[6];
    br_uint_32     nmats;

    if((demo->palette = BrPixelmapLoad("std.pal")) == NULL) {
        BrLogError("DEMO", "Error loading std.pal");
        return BRE_FAIL;
    }
    BrMapAdd(demo->palette);

    /*
     * If indexed, load and set the palette.
     */
    if(demo->colour_buffer->type == BR_PMT_INDEX_8) {
        br_pixelmap *std_pal;

        if((std_pal = BrPixelmapLoad("std.pal")) == NULL) {
            BrLogError("DEMO", "Unable to load std.pal");
            return BRE_FAIL;
        }

        BrPixelmapPaletteSet(demo->colour_buffer, std_pal);
    }

    if((sphere_model = BrModelLoad("sph32.dat")) == NULL) {
        BrLogError("DEMO", "Error loading sph32.dat.");
        return BRE_FAIL;
    }
    BrModelAdd(sphere_model);

    if((nmats = BrFmtScriptMaterialLoadMany("sheet.mat", mats, BR_ASIZE(mats))) != 6) {
        BrLogError("DEMO", "Error loading sheet.mat");
        return BRE_FAIL;
    }
    BrMaterialAddMany(mats, nmats);

    sheet        = BrResAllocate(demo, sizeof(br_demo_sheet), BR_MEMORY_APPLICATION);
    sheet->accum = SPF;

    demo->world->t.type = BR_TRANSFORM_MATRIX34;
    BrMatrix34PostTranslate(&demo->world->t.t.mat, 0, BR_SCALAR(0.5), 0);

    /*
     * The Actors
     */
    pivot  = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_NONE, NULL));
    system = BrActorAdd(pivot, BrActorAllocate(BR_ACTOR_NONE, NULL));

    sheet->sheet        = BrActorAdd(system, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    sheet->sheet->model = BrModelAllocate("sheet", SHEETX * SHEETY, 4 * (SHEETX - 1) * (SHEETY - 1));
    InitialiseSheetModel(sheet->sheet->model);
    sheet->sheet->model->flags = BR_MODF_KEEP_ORIGINAL | BR_MODF_GENERATE_TAGS;
    BrModelAdd(sheet->sheet->model);

    BrModelUpdate(sheet->sheet->model, BR_MODU_ALL);

    box               = BrActorAdd(system, BrActorAllocate(BR_ACTOR_MODEL, NULL));
    box->material     = BrMaterialFind("plain_white");
    box->render_style = BR_RSTYLE_EDGES;
    BrMatrix34PostScale(&box->t.t.mat, BR_SCALAR(1.0), BR_SCALAR(1.0), BR_SCALAR(1.0));

    sphere_model = BrModelLoad("sph32.dat");
    BrModelAdd(sphere_model);

    PlaceSphere(system, sphere_model, &centre1, "ball8");
    PlaceSphere(system, sphere_model, &centre2, "plain_green");
    PlaceSphere(system, sphere_model, &centre3, "earth");

    /*
     * The Camera
     */
    demo->camera = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_CAMERA, NULL));

    camera_data                = demo->camera->type_data;
    camera_data->type          = BR_CAMERA_PERSPECTIVE;
    camera_data->field_of_view = BR_ANGLE_DEG(45.0);
    camera_data->hither_z      = BR_SCALAR(0.1);
    camera_data->yon_z         = BR_SCALAR(20.0);
    BrMatrix34Translate(&demo->camera->t.t.mat, BR_SCALAR(0.0), BR_SCALAR(0.0), BR_SCALAR(4.0));

    demo->order_table->min_z = camera_data->hither_z;
    demo->order_table->max_z = camera_data->yon_z;

    /*
     * The lights
     */
    light1                                = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_LIGHT, NULL));
    ((br_light *)light1->type_data)->type = BR_LIGHT_DIRECT;
    BrMatrix34RotateZ(&light1->t.t.mat, BR_ANGLE_DEG(55));
    BrMatrix34PostRotateX(&light1->t.t.mat, BR_ANGLE_DEG(-45));
    BrLightEnable(light1);

    light2                                = BrActorAdd(demo->world, BrActorAllocate(BR_ACTOR_LIGHT, NULL));
    ((br_light *)light2->type_data)->type = BR_LIGHT_DIRECT;
    BrMatrix34PostTranslate(&light2->t.t.mat, BR_SCALAR(0), BR_SCALAR(1), BR_SCALAR(1));
    BrMatrix34PostRotateX(&light2->t.t.mat, BR_ANGLE_DEG(45));
    BrLightEnable(light2);

    demo->user = sheet;
    return BRE_OK;
}

void SheetUpdate(br_demo *demo, br_scalar dt)
{
    br_demo_sheet *sheet = demo->user;

    if(sheet->frozen) {
        sheet->accum = SPF;
        return;
    }

    sheet->accum += dt;

    while(sheet->accum >= SPF) {
        for(int i = 0; i < CYCLES; i++)
            UpdateDynamics(sheet->sheet->model, &gravity);

        /* If the sheet has changed we need to tell BRender to */
        /* update the normals, bounding box and radius */

        BrModelUpdate(sheet->sheet->model, BR_MODU_NORMALS | BR_MODU_BOUNDING_BOX | BR_MODU_RADIUS);

        sheet->accum -= SPF;
    }
}

const static br_demo_dispatch dispatch = {
    .init          = SheetInit,
    .process_event = BrDemoDefaultProcessEvent,
    .update        = SheetUpdate,
    .render        = BrDemoDefaultRender,
    .on_resize     = BrDemoDefaultOnResize,
    .destroy       = BrDemoDefaultDestroy,
};

int main(int argc, char **argv)
{
    return BrDemoRunArgv("BRender Rubber Sheet Demo", &dispatch, argc, argv);
}
