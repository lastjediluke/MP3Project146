#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include "printf_lib.h"
#include "str.hpp"

#include "Flash.h"
#include "storage.hpp"

void vprintMp3Files(void * pvParam) {
	while (1) {
		vTaskDelay(50);
	}
}

int main(int argc, char const *argv[]){
	char songTitles[10][20];
	char artists[10][20];
	Flash obj;
	obj.mp3_init((char*)songTitles, (char*)artists, (uint8_t)10, (uint8_t)20);

	uint8_t numSongs = obj.get_number_of_songs();
	for (uint8_t i = 0; i < numSongs; i++) {
		printf("Artist: ");
		printf(artists[i]);
		printf("\n");
		printf("Song Title: ");
		printf(songTitles[i]);
		printf("\n");
	}

	xTaskCreate(vprintMp3Files, "printMp3Files", 4096, (void*) 0, 1, NULL);

	vTaskStartScheduler();

	while(1);

	return 0;
}