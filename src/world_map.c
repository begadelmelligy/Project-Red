#include "world_map.h"

#include "stdio.h"
#include "stdlib.h"
#include "raylib.h"

const int SWIDTH = 1400;
const int SHEIGHT = 1000;


#define NUM_CITIES 3


typedef struct {
    Vector3 position;
    float radius;
    Color color;
    const char *name;
} City;

Camera initCamera(){
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 16.0f, 16.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    return camera;
} 

typedef struct {
    float r;
    int rings;
    int slices;

} World;

int main(void)
{
    World world = {
        world.r = 1.0f,
        world.rings = 32,
        world.slices = 32,
    };


    InitWindow(SWIDTH, SHEIGHT, "World Map Test");
    SetTargetFPS(60);


    Camera camera;
    camera = initCamera();

    float offset = 16;
    float azimuthA = 45.0f;
    float zenithA = 20.0f;

    Mesh sphereMesh = GenMeshSphere(world.r, world.slices, world.rings);
    Model sphereModel = LoadModelFromMesh(sphereMesh);

    sphereMesh.colors = (unsigned char *)malloc(sphereMesh.vertexCount * 4); // 4 bytes per vertex

    for (int i = 0; i < sphereMesh.vertexCount; i++) {
        // Assign colors based on some rule — for example, thirds
        if (i < sphereMesh.vertexCount / 3) {
            sphereMesh.colors[i*4 + 0] = 255;  // R
            sphereMesh.colors[i*4 + 1] = 0;    // G
            sphereMesh.colors[i*4 + 2] = 0;    // B
            sphereMesh.colors[i*4 + 3] = 255;  // A
        } else if (i < 2 * sphereMesh.vertexCount / 3) {
            sphereMesh.colors[i*4 + 0] = 0;
            sphereMesh.colors[i*4 + 1] = 255;
            sphereMesh.colors[i*4 + 2] = 0;
            sphereMesh.colors[i*4 + 3] = 255;
        } else {
            sphereMesh.colors[i*4 + 0] = 0;
            sphereMesh.colors[i*4 + 1] = 0;
            sphereMesh.colors[i*4 + 2] = 255;
            sphereMesh.colors[i*4 + 3] = 255;
        }
    }

    UploadMesh(&sphereMesh, false); // false = don't keep CPU copy

    Model model = LoadModelFromMesh(sphereMesh);
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture.id = 0;

    while (!WindowShouldClose())
    {
        BeginDrawing();

        if (IsKeyDown(KEY_L)) azimuthA += PI/32;
        if (IsKeyDown(KEY_H)) azimuthA -= PI/32;
        if (IsKeyDown(KEY_K)) zenithA += PI/32;
        if (IsKeyDown(KEY_J)) zenithA -= PI/32;

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

        DrawModel(sphereModel, (Vector3){ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);
        

        /*debug*/
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

