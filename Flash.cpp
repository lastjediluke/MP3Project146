#include <stdio.h>              // printf()
#include "lpc_sys.h"
#include "printf_lib.h"
#include "str.hpp"

#include "storage.hpp"          // Get Storage Device instances
#include "ff.h"

#include "vector.hpp"

Flash::Flash() {
    num_files = 0;
}

void Flash::getMp3Files() {
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
        return true;
    }

    u0_dbg_printf("Directory listing of: %s\n\n", dirPath);
    // endless loop
    for (;;) {
        #if _USE_LFN
            Finfo.lfname = Lfname;
            Finfo.lfsize = sizeof(Lfname);
        #endif

        returnCode = f_readdir(&Dir, &Finfo);
        
        if ( (FR_OK != returnCode) || !Finfo.fname[0]) {
            break;
        }
        else {
            // check if currently reading mp3 file
            string file_name = Finfo.fname[0];
            if (Finfo.fname[0].find('3')) {
                num_files++;
                mp3_files.push_back(Finfo);
            }
        }

        // // move information into display
        // output.printf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %10lu %13s",
        //         (Finfo.fattrib & AM_DIR) ? 'D' : '-',
        //         (Finfo.fattrib & AM_RDO) ? 'R' : '-',
        //         (Finfo.fattrib & AM_HID) ? 'H' : '-',
        //         (Finfo.fattrib & AM_SYS) ? 'S' : '-',
        //         (Finfo.fattrib & AM_ARC) ? 'A' : '-',
        //         (Finfo.fdate >> 9) + 1980, (Finfo.fdate >> 5) & 15, Finfo.fdate & 31,
        //         (Finfo.ftime >> 11), (Finfo.ftime >> 5) & 63,
        //         // file size
        //         Finfo.fsize,
        //         // file type 
        //         &(Finfo.fname[0]));

        // // LFN names tend to increase memory requirements for output str, enable with caution
        // #if (_USE_LFN)
        // output.put(" - ");
        // // file name
        // output.put(Lfname);
        // #endif
        // output.putline("");
    }
    f_closedir(&Dir);
    return true;
}

void Flash::readMP3Files() {
    for (auto i : num_files){
        u0_dbg_printf("File name: \n", mp3_files[i].fname);
    }
}