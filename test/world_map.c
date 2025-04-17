#include "world_map.h"

#include "stdio.h"
#include "stdlib.h"
#include "raylib.h"
#include <math.h>

const int SWIDTH = 1400;
const int SHEIGHT = 1000;


#define NUM_CITIES 3


typedef struct {
    Vector3 position;
    float radius;
    Color color;
    const char *name;
} City;

typedef struct {
    float r;
    int rings;
    int slices;

} World;

Camera initCamera(){
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 16.0f, 16.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    return camera;
} 


int main(void)
{
    World world = {
        world.r = 10.0f,
        world.rings = 32,
        world.slices = 32,
    };

    City worldCities[NUM_CITIES];


    InitWindow(SWIDTH, SHEIGHT, "World Map Test");
    SetTargetFPS(60);


    Camera camera;
    camera = initCamera();

    float offset = 48;
    float azimuthA = 45.0f;
    float zenithA = 20.0f;

    Mesh sphereMesh = GenMeshSphere(world.r, world.slices, world.rings);
    Model sphereModel = LoadModelFromMesh(sphereMesh);

    const char* objPath = "E:\\Projects\\Project-Red\\assets\\Home.obj";
    const char* textPath = "E:\\Projects\\Project-Red\\assets\\FastFood_Restaurant_Color.png";
    Model building = LoadModel(objPath);
    Texture2D buildingT = LoadTexture(textPath);
    building.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = buildingT;

    float zoom = 1.0f;

    while (!WindowShouldClose())
    {
        BeginDrawing();

        float zoomSpeed = 1.0f;
        float wheel = GetMouseWheelMove();
        float rotation = 64;
        
        camera.position = Vector3Add(camera.position,
        Vector3Scale(Vector3Normalize(Vector3Subtract(camera.target, camera.position)), wheel * zoomSpeed));
        offset = sqrt(pow(camera.position.x, 2) + pow(camera.position.y, 2) + pow(camera.position.z, 2));

        if (IsKeyDown(KEY_L)) azimuthA += PI/rotation;
        if (IsKeyDown(KEY_H)) azimuthA -= PI/rotation;
        if (IsKeyDown(KEY_K)) zenithA += PI/rotation;
        if (IsKeyDown(KEY_J)) zenithA -= PI/rotation;

        if (zenithA > PI/2 - PI/16){
            zenithA = PI/2 - PI/16;
        }

        if (zenithA < -PI/2 + PI/16){
            zenithA = -PI/2 + PI/16;
        }

        camera.position.x = offset * cosf(zenithA) * sinf(azimuthA);
        camera.position.y = offset * sinf(zenithA);
        camera.position.z = offset * cosf(zenithA) * cosf(azimuthA); 

        ClearBackground(BLACK);

        BeginMode3D(camera);

        DrawModel(building, (Vector3){ 0.0f, 10.0f, 0.0f }, 0.01f, WHITE);
        
        DrawGrid(10, 1.0f);

        DrawModel(sphereModel, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, RED);
        /**/
        /*/*debug*/
        /*DrawModelWires(sphereModel, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, GRAY);*/
        /*DrawBoundingBox(GetMeshBoundingBox(sphereMesh), YELLOW);*/

        EndMode3D();

        DrawFPS(10, 10);

        EndDrawing();
    }

    UnloadModel(sphereModel);
    CloseWindow();

    return 0;
}

