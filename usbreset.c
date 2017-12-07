/* Author Jacob Psimos 2017 */
/* usbreset - reset the usb busses */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#define MAX_NUM_BUSSES 8
#define BUS_PATH_BUFFER_SIZE 128

static const ssize_t locate_busses(char (*busses)[BUS_PATH_BUFFER_SIZE]);
static void reset_bus_power(const char *bus_path, unsigned int toggle_delay_s);

int main(int argc, char **argv){

	if(argc == 2){
		if((const char*)strstr(argv[1], "-h") != (const char*)NULL || 
			(const char*)strstr(argv[1], "--help") != (const char*)NULL){

			printf("usbreset - reset the usb busses\nby Jacob Psimos 2017\n");
			printf("\t-h --help  display this message\n");
			printf("Returns 0 if successful or 1 for error.\nRequires root privileges.\n");
		}
	}

	char busses[MAX_NUM_BUSSES][BUS_PATH_BUFFER_SIZE];
	const ssize_t number_of_busses = locate_busses(busses);
	int i;

	if(number_of_busses > 0){
		for(i = 0; i < number_of_busses; i++){
			reset_bus_power(busses[i], 2);
//			printf("Resetting: %s\n", busses[i]);
		}
	}
	return 0;
}

static const ssize_t locate_busses(char (*busses)[BUS_PATH_BUFFER_SIZE]){
	if(busses == (char(*)[BUS_PATH_BUFFER_SIZE])NULL){
		return -1;
	}

	const char *search_base = "/sys/devices/platform/soc";

	ssize_t number_found = 0;

	DIR *dir_handle;
	struct dirent *dir_pointer;

	if((dir_handle = opendir(search_base)) == NULL){
		return -1;
	}

	while((dir_pointer = readdir(dir_handle)) != NULL){
		const char *dirname = (const char*)dir_pointer->d_name;
		const int length = strlen(dirname);

		if(length < 5){ continue; }

		if(number_found < MAX_NUM_BUSSES && (const char*)strstr(dirname, ".usb") != (const char*)NULL){
			snprintf(busses[number_found], BUS_PATH_BUFFER_SIZE, "%s/%s/buspower", search_base, dirname);
			number_found = number_found + 1;
		}
	}

	closedir(dir_handle);
	return number_found;
}

static void reset_bus_power(const char *bus_path, const unsigned int toggle_delay_s){
	if(bus_path == (const char*)NULL){ return; }
	if(!access(bus_path, R_OK | W_OK)){
		int fp = open(bus_path, O_WRONLY | O_SYNC);
		if(fp > 0){
			write(fp, "0", 1);
			if(toggle_delay_s > 0){
				sleep(toggle_delay_s);
			}
			write(fp, "1", 1);
			close(fp);
		}
	}else{
		//Probbaly missing root privileges.
		exit(1);
	}
}
