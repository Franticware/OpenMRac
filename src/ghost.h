#include <cstring>
#include <vector>

struct Ghost {
    Ghost(bool frames = true);
    bool load(int track, int reverse);
    void save();
    void copyFrom(const Ghost& gnew);
    
    int m_version; // verze souboru
    int m_track; // číslo tratě
    int m_reverse; 
    int m_car;
    int m_carcolor;
    float m_seconds;
    int m_maxnum;
    int m_num; // počet uložených snímků při 10 fps
    std::vector<float> m_frames;
    
    void getfname(char* buff); // 1024 bajtů
};
