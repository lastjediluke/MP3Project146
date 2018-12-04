#pragma once
#ifndef FLASH_H
#define FLASH_H

#include "queue.h"

#include "vector.hpp"
#include "str.hpp"

class Flash {
    private:
        // for dislay output
        char mp3Files[10][20];
        uint8_t numFiles;
    public:
        Flash();
        // to be used with LCD
        void mp3_init(char *songs, char *artists, uint8_t rows, uint8_t cols);
        void get_mp3_files();
        void get_mp3_metadata(char *songs, char *artists, uint8_t rows, uint8_t cols);
        uint8_t get_number_of_songs();
        // if user is ready to switch, or start, the song,
        // have user specify the song to send to decoder with
        // file number (index inside the vector)
        bool send_mp3_file_2_decoder(uint8_t songIndex, QueueHandle_t *queue);
};
#endif