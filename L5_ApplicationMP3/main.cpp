#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include "printf_lib.h"
#include "str.hpp"

#include "Flash.h"
#include "storage.hpp"
#include "VS1011Drv.hpp"

TaskHandle_t flashHandle = NULL;
TaskHandle_t decoderHandle = NULL;

void vQueueMp3Files(void * ) 
{
	Flash::instance()->send_mp3_file_2_decoder();
}

void vSendToDecoder(void *)
{
	VS1011Drv::instance()->SendToDecoder();
}

void vStartNextSong(void *)
{
	for(uint8_t i=0; i < Flash::instance()->get_number_of_songs(); i++)
	{
		xQueueSend(*Flash::instance()->songQueue, Flash::instance()->mp3Files[i], portMAX_DELAY);
	}
}
int main(int argc, char const *argv[]){
	char songTitles[10][20];
	char artists[10][20];

	// uint8_t numSongs = obj.get_number_of_songs();
	// for (uint8_t i = 0; i < numSongs; i++) {
	// 	printf("Artist: ");
	// 	printf(artists[i]);
	// 	printf("\n");
	// 	printf("Song Title: ");
	// 	printf(songTitles[i]);
	// 	printf("\n");
	// }

	//xTaskCreate(vprintMp3Files, "printMp3Files", 4096, (void*) 0, 1, NULL);
	QueueHandle_t decoderQueue = xQueueCreate(10, 512);	//buffer for sending mp3 to decoder
	QueueHandle_t songQueue = xQueueCreate(1, 20);		//holds the song title we requested

	Flash::instance()->mp3_init((char*)songTitles, (char*)artists, (uint8_t)10, (uint8_t)20);
	Flash::instance()->setQueueHandlers(decoderQueue, songQueue);
	VS1011Drv::instance()->initialize();
	VS1011Drv::instance()->setQueueHandler(decoderQueue);
	printf("end of decoder init \n");
	xTaskCreate(vSendToDecoder, "vSendToDecoder", 1024, NULL, 3, &flashHandle);
	xTaskCreate(vQueueMp3Files, "vqMp3", 1024, NULL, 4, &decoderHandle);
	xTaskCreate(vStartNextSong, "vStartNext", 1024, NULL, 2, NULL);
	vTaskStartScheduler();

	while(1);

	return 0;
}