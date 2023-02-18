#include "cam.h"
#include <cstring>
#include <cmath>
#include "gl1.h"
#include <cstdio>

//void Cam::save()
//{
//    FILE* fout = fopen("cam.dat", "wb");
//    if (!fout) return;
//    fwrite(p_pos, sizeof(float), 3, fout);
//    fwrite(&p_ax, sizeof(float), 1, fout);
//    fwrite(&p_ay, sizeof(float), 1, fout);
//    fclose(fout);
//}

//void Cam::load()
//{
//    FILE* fin = fopen("cam.dat", "rb");
//    if (!fin) return;
//    fread(p_pos, sizeof(float), 3, fin);
//    fread(&p_ax, sizeof(float), 1, fin);
//    fread(&p_ay, sizeof(float), 1, fin);
//    fclose(fin);
//}

void Cam::init(float x, float y, float z, float ax, float ay, float speed_transf, float speed_rot)
{
    p_pos[0] = x;
    p_pos[1] = y;
    p_pos[2] = z;
    p_ax = ax;
    p_ay = ay;
    p_vx = speed_transf;
    p_va = speed_rot;
}

void Cam::move_l(float T)
{
    p_pos[0] -= cosf(p_ay)*p_vx*T;
    p_pos[2] += sinf(p_ay)*p_vx*T;
}

void Cam::move_r(float T)
{
    p_pos[0] += cosf(p_ay)*p_vx*T;
    p_pos[2] -= sinf(p_ay)*p_vx*T;
}

void Cam::move_b(float T)
{
    p_pos[0] += sinf(p_ay)*cosf(p_ax)*p_vx*T;
    p_pos[2] += cosf(p_ay)*cosf(p_ax)*p_vx*T;
    p_pos[1] -= sinf(p_ax)*p_vx*T;
}

void Cam::move_f(float T)
{
    p_pos[0] -= sinf(p_ay)*cosf(p_ax)*p_vx*T;
    p_pos[2] -= cosf(p_ay)*cosf(p_ax)*p_vx*T;
    p_pos[1] += sinf(p_ax)*p_vx*T;
}

void Cam::move_d(float T)
{
    p_pos[1] -= p_vx*T;
}

void Cam::move_u(float T)
{
    p_pos[1] += p_vx*T;
}

void Cam::turn_l(float T)
{
    p_ay += p_va*T;
}

void Cam::turn_r(float T)
{
    p_ay -= p_va*T;
}

void Cam::turn_d(float T)
{
    p_ax -= p_va*T;
}

void Cam::turn_u(float T)
{
    p_ax += p_va*T;
}

glm::mat4 Cam::transform() const
{
    //printf("%f, %f, %f, %f, %f\n", p_pos[0], p_pos[1], p_pos[2], p_ax, p_ay); fflush(stdout);
    glm::mat4 ret(1);
    ret = glm::rotate(ret, -p_ax, glm::vec3(1.f, 0.f, 0.f));
    ret = glm::rotate(ret, -p_ay, glm::vec3(0.f, 1.f, 0.f));
    ret = glm::translate(ret, glm::vec3(-p_pos[0], -p_pos[1], -p_pos[2]));
    return ret;
}
