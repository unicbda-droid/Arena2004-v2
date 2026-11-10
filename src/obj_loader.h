#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include "math3d.h"
#include "render.h"

/* Loads a Wavefront .obj file (triangles only) into a Mesh.
 * Returns 0 on success, -1 on failure. Materials/colors are ignored;
 * use renderer_draw_model() with a Color/tint afterwards. */
int obj_load_mesh(const char* path, Mesh* out);

/* Convenience: draw an OBJ mesh at a position with a color. */
void obj_draw_mesh(Renderer* r, Mesh* m, Vec3 pos, Vec3 scale, float yaw, Color color, int material, float emissive);

#endif
