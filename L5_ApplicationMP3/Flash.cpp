#include <stdio.h>              // printf(), sprintf()
#include "lpc_sys.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "printf_lib.h"

#include "storage.hpp"          // Get Storage Device instances
#include "ff.h"

#include "vector.hpp"
#include "Flash.h"

#include <string.h>
#include "str.hpp"

Flash::Flash() {
    numFiles = 0;
}

Flash * Flash::m_instance = NULL;

Flash * Flash::instance()
{
    if(!m_instance)
    {
        m_instance = new Flash();
    }
    return m_instance;
}
uint8_t Flash::get_number_of_songs(){
    return numFiles;
}

void Flash::get_mp3_files() {
    DIR Dir;
    FILINFO Finfo;
    FRESULT returnCode = FR_OK;
    
    #if _USE_LFN
        char Lfname[_MAX_LFN];
    #endif
    // access SD directory directly
    const char *dirPath = "1:";
    if (FR_OK != (returnCode = f_opendir(&Dir, dirPath))) {
        u0_dbg_printf("Invalid directory: |%s| (Error %i)\n", dirPath, returnCode);
        return;
    }

    u0_dbg_printf("Directory listing of: %s\n", dirPath);
    // endless loop
    uint8_t iter = 0;
    for (;;) {
        #if _USE_LFN
            Finfo.lfname = Lfname;
            Finfo.lfsize = sizeof(Lfname);
        #endif

        returnCode = f_readdir(&Dir, &Finfo);
        
        if ((FR_OK != returnCode) || !Finfo.fname[0]) {
            break;
        }
        else {
            char tmp[20];
            sprintf(tmp, Finfo.fname);
            str name = tmp;
            // check if currently reading mp3 file
            if (name.contains("MP3")) {
                ++numFiles;
                sprintf(mp3Files[iter], Finfo.lfname);

                ++iter;
            }
        }
    }
    f_closedir(&Dir);
    return;
}

void Flash::get_mp3_metadata(char *songs, char *arts, uint8_t rows, uint8_t cols) {
    for (uint8_t k = 0; k < numFiles; k++) {
        char fileName[75];
        char buffTmp[75];
        uint16_t i = 0;

        str file = "1:";
        str tmp1(mp3Files[k]);
        file.append(tmp1);
        strcpy(fileName, file.c_str());
        printf("%s\n",fileName);
        // fixes weird bug that ignores metadata of first mp3 file
        for (i = 0; i < 75; i++) {
            buffTmp[i] = ' ';
        }

        Storage::read(fileName, &buffTmp, strlen(buffTmp), 0);

        // clean buffer by replacing weird characters with space
        for (i = 0; i < 75; i++) {
            if (buffTmp[i] > 122 || (buffTmp[i] < 97 && buffTmp[i] > 90) || 
                (buffTmp[i] < 65 && buffTmp[i] > 57) || (buffTmp[i] < 48 && buffTmp[i] > 32) || 
                buffTmp[i] < 32)
                buffTmp[i] = ' ';
        }

        for (i = 0; i < 75; i++) {
            // navigate to Song Title
            if (buffTmp[i] == '2' && buffTmp[i - 1] == 'T') {
                // skip non alpha
                while (buffTmp[i] > 122 || buffTmp[i] < 65) {
                    i++;
                }

                // get song title
                uint16_t j = 0;
                while (1) {
                    *((songs + k * cols) + j) = buffTmp[i];
                    i++;
                    j++;
                    if ((buffTmp[i] == 'T') && (buffTmp[i + 1] == 'P') && (buffTmp[i + 2] == 'E')) {
                        break;
                    }
                }

                // skip tag TPE2, currently at "T"
                i += 4;
                // skip non alpha
                while (buffTmp[i] > 122 || buffTmp[i] < 65) {
                    i++;
                }

                // get artist
                j = 0;
                while (1) {
                    *((arts + k * cols) + j) = buffTmp[i];
                    i++;
                    j++;
                    if ((buffTmp[i] == 'P') && (buffTmp[i + 1] == 'O')) {
                        break;
                    }
                }

                break;
            }
        }
    }
    return;
}

void Flash::mp3_init(char *songs, char *artists, uint8_t rows, uint8_t cols) {
    get_mp3_files();
    get_mp3_metadata(songs, artists, rows, cols);
    printf("end of flash init \n");
}

void Flash::send_mp3_file_2_decoder() {
    // if (songIndex < 0 || songIndex > (numFiles - 1)){
    //     return false;
    // }
    while(1)
    {
        char songName[20];
        if(xQueueReceive(*songQueue, songName, portMAX_DELAY))
        {
            FIL src_file;
            // Open existing file
            str fileName = "1:";
            fileName.append(songName);
            strcpy(songName, fileName.c_str());
            printf("songName:%s w\n",songName);
            FRESULT status = f_open(&src_file, songName , FA_OPEN_EXISTING | FA_READ);
            if (status == FR_OK) 
            {
                // returns number of DWORDs, so 1 DWORD = 4 bytes
                int file_size = f_size(&src_file);
                printf("size of mp3: %d \n",file_size);
                // read and send file byte by byte to decoder, may be slower than by taking the whole file
                // f_forward: forward data to the stream
                // f_lseek: move file pointer of  afile

                // read and send entire file to decoder, may be faster than by going byte by byte
                char buffer[512] = {0};     // extra char for null terminator
                unsigned int bytesRead = 0;
                unsigned int totalBytesRead = 0;
                // start sending bytes to decoder
                while (FR_OK == f_read(&src_file, buffer, sizeof(buffer), &bytesRead) && itsUIcontrol->getCurrentState() != MP3State::STOPPED) 
                {
                    if(itsUIcontrol->getCurrentState() == MP3State::PAUSED)
                    {
                        xTaskSuspend(NULL);
                    }
                    xQueueSend(*decoderQueue, buffer, portMAX_DELAY);
                    totalBytesRead += bytesRead;
                    if(totalBytesRead >= file_size)
                    {
                        break;
                    }
                }

                VS1011Drv::instance()->stopSong();
                if(itsUIcontrol->getCurrentState() == MP3State::STOPPED)    //We want to play another song. Clear queue
                {
                    xQueueReset(*decoderQueue);     //clear the buffer of the previously playing song
                    f_close(&src_file);
                }
            }
            else 
            {
                u0_dbg_printf("Error reading file");
            }
            f_close(&src_file);
        }
    }
}

void Flash::setQueueHandlers(QueueHandle_t &dQueue, QueueHandle_t &sQueue)
{
    decoderQueue = &dQueue;
    songQueue = &sQueue;
}