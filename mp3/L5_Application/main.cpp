#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include "printf_lib.h"
#include "str.hpp"

#include "Flash.h"
#include "storage.hpp"

Flash obj;

void vprintMp3Files(void * pvParam) {
	obj.get_mp3_meta_data();=
	char buffTmp[75];
	int i = 0;
	char songTitle[40];
	char artist[40];

	// Storage::read("1:PapaYaSunny.mp3", &buffTmp, strlen(buffTmp), 0);
	Storage::read("1:regularShow.mp3", &buffTmp, strlen(buffTmp), 0);
	// Storage::read("1:tameImpala.mp3", &buffTmp, strlen(buffTmp), 0);

	// clean buffer by replacing weird characters with space
	for (i = 0; i < 75; i++) {
		if (buff2[i] > 122 || (buff2[i] < 97 && buff2[i] > 90) || 
			(buff2[i] < 65 && buff2[i] > 57) || (buff2[i] < 48 && buff2[i] > 32) || 
			buff2[i] < 32)
			buff2[i] = ' ';
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
				songTitle[j] = buffTmp[i];
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
				artist[j] = buffTmp[i];
				i++;
				j++;
				if ((buffTmp[i] == 'P') && (buffTmp[i + 1] == 'O')) {
					break;
				}
			}

			u0_dbg_printf("Artist Char: ");
			for (i = 0; i < 40; i++) {
				u0_dbg_printf("%c", artist[i]);
			}
			u0_dbg_printf("\n");

			u0_dbg_printf("Title Char: ");
			for (i = 0; i < 40; i++) {
				u0_dbg_printf("%c", songTitle[i]);
			}
			u0_dbg_printf("\n");

			break;
		}
	}

	while (1) {
		vTaskDelay(50);
	}
}

int main(int argc, char const *argv[]){
	xTaskCreate(vprintMp3Files, "printMp3Files", 4096, (void*) 0, 1, NULL);

	vTaskStartScheduler();

	while(1);

	return 0;
}