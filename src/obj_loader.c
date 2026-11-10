#include "obj_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* --- tiny OBJ parser: vertices + triangles only --- */

typedef struct { float x, y, z; } Vec3f;
typedef struct { int v[3]; } Face3;

static char* skip_ws(char* p) { while (*p==' '||*p=='\t') p++; return p; }
static float parse_float(char** p) {
    char* end; float v = strtof(*p, &end); *p = end; return v;
}
static int parse_int(char** p) {
    char* end; long v = strtol(*p, &end, 10); *p = end; return (int)v;
}

int obj_load_mesh(const char* path, Mesh* out){
    FILE* f = fopen(path, "r");
    if(!f) return -1;

    /* dynamic arrays */
    int vcap=4096, fcap=1024;
    Vec3f* verts = malloc(sizeof(Vec3f)*vcap);
    Face3* faces = malloc(sizeof(Face3)*fcap);
    int nv=0, nf=0;
    char line[1024];

    while(fgets(line, sizeof(line), f)){
        char* p = skip_ws(line);
        if(*p=='#'||*p=='\0'||*p=='\n'||*p=='\r') continue;

        if(p[0]=='v' && p[1]==' '){
            if(nv>=vcap){ vcap*=2; verts=realloc(verts,sizeof(Vec3f)*vcap); }
            p+=2;
            verts[nv].x = parse_float(&p);
            verts[nv].y = parse_float(&p);
            verts[nv].z = parse_float(&p);
            nv++;
        }
        else if(p[0]=='f'){
            if(nf>=fcap){ fcap*=2; faces=realloc(faces,sizeof(Face3)*fcap); }
            p++;
            int vi[3];
            for(int k=0;k<3;k++){
                p = skip_ws(p);
                vi[k] = parse_int(&p);
                /* skip /uv/normal */
                while(*p && *p!=' ' && *p!='\t' && *p!='\n' && *p!='\r'){ if(*p=='/'){ p++; while(*p && *p!=' ' && *p!='\t' && *p!='\n' && *p!='\r') p++; } else p++; }
            }
            faces[nf].v[0]=vi[0]; faces[nf].v[1]=vi[1]; faces[nf].v[2]=vi[2];
            nf++;
        }
    }
    fclose(f);

    if(nv==0 || nf==0){ free(verts); free(faces); return -1; }

    /* build interleaved vertex array: 3 floats pos + 3 floats normal + 2 floats uv */
    int vcount = nf*3;
    Vertex* v = malloc(sizeof(Vertex)*vcount);
    uint32_t* idx = malloc(sizeof(uint32_t)*vcount);
    int vi=0;

    /* compute face normals */
    for(int i=0;i<nf;i++){
        Face3* fc = &faces[i];
        /* OBJ indices are 1-based, negative means relative to end */
        int iv[3];
        for(int k=0;k<3;k++){
            int raw = fc->v[k];
            iv[k] = (raw>0) ? raw-1 : (nv+raw);
            if(iv[k]<0||iv[k]>=nv){ free(verts); free(faces); free(v); free(idx); return -1; }
        }
        Vec3f* a = &verts[iv[0]];
        Vec3f* b = &verts[iv[1]];
        Vec3f* c = &verts[iv[2]];
        /* normal = (b-a) x (c-a) */
        float ux=b->x-a->x, uy=b->y-a->y, uz=b->z-a->z;
        float vx=c->x-a->x, vy=c->y-a->y, vz=c->z-a->z;
        float nx=uy*vz-uz*vy, ny=uz*vx-ux*vz, nz=ux*vy-uy*vx;
        float len=nx*nx+ny*ny+nz*nz;
        if(len>0.0f){ len=1.0f/sqrtf(len); nx*=len; ny*=len; nz*=len; }
        else { nx=0; ny=1; nz=0; }

        for(int k=0;k<3;k++){
            Vec3f* pv = &verts[iv[k]];
            v[vi].pos = v3(pv->x, pv->y, pv->z);
            v[vi].nrm = v3(nx, ny, nz);
            v[vi].uv  = v2(0.5f, 0.5f);
            idx[vi] = vi;
            vi++;
        }
    }

    Mesh m = mesh_make(v, vcount, idx, vcount);
    *out = m;
    free(verts); free(faces); free(v); free(idx);
    return 0;
}

void obj_draw_mesh(Renderer* r, Mesh* m, Vec3 pos, Vec3 scale, float yaw, Color color, int material, float emissive){
    Mat4 model = mat4_from_trs(pos, v3(0, yaw, 0), scale);
    renderer_draw_model(r, m, model, color, material, emissive, 1.0f, false);
}
