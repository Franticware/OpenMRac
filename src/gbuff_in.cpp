#include "gbuff_in.h"

Gbuff_in gbuff_in;

bool Gbuff_in::init_dat(const char* fname)
{
    p_bdat = true;
    return p_datdec.init(fname);
}

void Gbuff_in::rewind()
{
    if (p_bactive)
    {
        if (p_bdat)
        {
            p_buff_pos = 0;
        } else {
            if (!p_bbin)
                ::rewind(p_fin);
        }
    }
}

char* Gbuff_in::fgets(char* str, int num)
{
    if (p_bactive)
    {
        if (p_bdat)
        {
            if (p_buff_pos == p_datdec.p_buff.size())
                return 0;
            int i = 0;
            while (i+1 < num && p_buff_pos < p_datdec.p_buff.size())
            {
                if (p_datdec.p_buff[p_buff_pos] != '\r')
                {
                    str[i] = p_datdec.p_buff[p_buff_pos];
                    ++i;
                    if (p_datdec.p_buff[p_buff_pos] == '\n')
                    {
                        ++p_buff_pos;
                        break;
                    }
                }
                ++p_buff_pos;
            }
            if (num > 0)
                str[i] = '\0';
            if (strlen(str) == 0)
                return 0;
            return str;
        } else {
            if (!p_bbin)
            {
                return ::fgets(str, num, p_fin);
            } else {
                return 0;
            }
        }
    }
    return 0;
}

/*const*/ uint8_t* Gbuff_in::fbuffptr()
{
    if (p_bactive && p_bbin)
    {
        if (p_bdat)
        {
            return p_datdec.p_buff.data();
        } else {
            return p_stdin_buff_ptr.data();
        }
    }
    return 0;
}

unsigned int Gbuff_in::fbuffsz()
{
    if (p_bactive && p_bbin)
    {
        if (p_bdat)
        {
            return p_datdec.p_buff.size();
        } else {
            return p_stdin_buff_sz;
        }
    }
    return 0;
}

bool Gbuff_in::f_open(const char* fname, const char* mode) /* "r" nebo "rb" */
{
    if (p_bactive) // pokud je soubor otevřený, neotevře se jiný a vrátí se false (chyba)
        return false;
    // parametr mode je "r"
    if (strlen(mode) == 1 && *mode == 'r')
    {
        if (p_bdat)
        {
            if (!p_datdec.load(fname))
                return false;
        } else {
            p_fin = ::fopen(fname, mode);
            //fprintf(stderr, "%s %s\n", __PRETTY_FUNCTION__, fname);
            if (p_fin == NULL)
                return false;
        }
        p_bactive = true;
        p_bbin = false;
        return true;
    }
    // parametr mode je "rb"
    else if (strlen(mode) == 2 && *mode == 'r' && mode[1] == 'b')
    {
        if (p_bdat) // otevření z paku
        {
            if (!p_datdec.load(fname))
                return false;
        } else { // otevření normálního souboru
            FILE* fin = ::fopen(fname, mode);
            //fprintf(stderr, "%s %s\n", __PRETTY_FUNCTION__, fname);
            if (fin == NULL)
                return false;
            ::fseek(fin, 0, SEEK_END);
            p_stdin_buff_sz = ::ftell(fin);
            ::rewind(fin);
            p_stdin_buff_ptr.resize(p_stdin_buff_sz);
            ::fread(p_stdin_buff_ptr.data(), 1, p_stdin_buff_sz, fin);
            ::fclose(fin);
        }
        p_bactive = true;
        p_bbin = true; // je otevřený binární soubor
        return true;
    }
    else return false;
}

void Gbuff_in::fclose()
{
    p_bactive = false;
    if (p_bbin)
    {
        if (p_bdat)
        {
            p_stdin_buff_ptr.clear();
            p_stdin_buff_sz = 0;
        } else { // normální soubor
           p_stdin_buff_ptr.clear();
            p_stdin_buff_sz = 0;
        }
    } else { // textový soubor
        if (p_bdat)
        {
            p_buff_pos = 0;
        } else { // normální soubor
            if (p_fin)
                ::fclose(p_fin);
            p_fin = 0;
        }
    }
}
