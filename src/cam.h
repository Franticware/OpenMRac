#ifndef HLIDAC_CAM_H
#define HLIDAC_CAM_H

class Cam {
public:
    void init(const float* pos, float ax, float ay, float speed_transf, float speed_rot);
    void move_l(float T); // move left
    void move_r(float T); // move right
    void move_b(float T); // move back
    void move_f(float T); // move forward
    void move_d(float T); // move down - currently does nothing
    void move_u(float T); // move up - currently does nothing
    void turn_l(float T);
    void turn_r(float T);
    void turn_d(float T);
    void turn_u(float T);

    /*void save();
    void load();*/

    void transform();

    float p_pos[3];
    float p_ax;
    float p_ay;
    float p_vx;
    float p_va;
};

#endif
