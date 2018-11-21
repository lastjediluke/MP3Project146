#pragma once
#ifndef FLASH_H
#define FLASH_H

#include "vector.hpp"

class Flash {
    private:
        VECTOR<FILINFO> mp3_files;
        int num_files;
    public:
        Flash();
        void getMp3Files();
        void readMP3Files();
};
#endif