
/*

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/input.h>
#include <sys/select.h>
#include <sys/ioctl.h>
//#include <stdio.h>
#include <termios.h>
//#include <unistd.h>

*/

#define MAX_DEVICES 16
#define DEVICE_NAME_LENGTH 256
#define RESULT_FORMAT "%s (%s)"
static int keyboard_fd = -1;  // Store keyboard file descriptor

typedef struct {
    int fd;
    char name[DEVICE_NAME_LENGTH];
    char path[DEVICE_NAME_LENGTH];
} InputDevice;

char* get_keyboard_id() {
    DIR *dir;
    struct dirent *entry;
    InputDevice devices[MAX_DEVICES];
    int device_count = 0;

    // Scan input devices
    if ((dir = opendir("/dev/input")) == NULL) {
        perror("opendir failed");
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL && device_count < MAX_DEVICES) {
        if (strncmp(entry->d_name, "event", 5) != 0) continue;

        char path[256];
        snprintf(path, sizeof(path), "/dev/input/%s", entry->d_name);

        int fd = open(path, O_RDONLY | O_NONBLOCK);
        if (fd == -1) continue;

        // Check if device supports key events
        unsigned long ev_bits = 0;
        if (ioctl(fd, EVIOCGBIT(0, sizeof(ev_bits)), &ev_bits) < 0) {
            close(fd);
            continue;
        }

        if (!(ev_bits & (1 << EV_KEY))) {
            close(fd);
            continue;
        }

        // Get device name
        char name[DEVICE_NAME_LENGTH] = "Unknown";
        ioctl(fd, EVIOCGNAME(sizeof(name)), name);

        devices[device_count].fd = fd;
        strncpy(devices[device_count].path, path, sizeof(path));
        strncpy(devices[device_count].name, name, sizeof(name));
        device_count++;
    }
    closedir(dir);

    if (device_count == 0) {
        fprintf(stderr, "No input devices found\n");
        return NULL;
    }

    printf("Press the 'a' key now...\n");

    // Set up file descriptor monitoring
    fd_set readfds;
    FD_ZERO(&readfds);
    int max_fd = 0;

    for (int i = 0; i < device_count; i++) {
        FD_SET(devices[i].fd, &readfds);
        if (devices[i].fd > max_fd) max_fd = devices[i].fd;
    }

    // Wait for key press
    int result = select(max_fd + 1, &readfds, NULL, NULL, NULL);
    if (result == -1) {
        perror("select failed");
        for (int i = 0; i < device_count; i++) close(devices[i].fd);
        return NULL;
    }

    // Check which device triggered the event
    for (int i = 0; i < device_count; i++) {
        if (!FD_ISSET(devices[i].fd, &readfds)) continue;

        struct input_event ev;
	//int runn = 1;

	printf("aaaaaaaa\n");

        while (read(devices[i].fd, &ev, sizeof(ev)) == sizeof(ev)){
	      if (ev.type == EV_KEY && ev.value == 1 && ev.code == KEY_A){
                char *result_str;
		//runn = 0;
	    
	    
                // Combine device name and path into result string
                //if (asprintf(&result_str, RESULT_FORMAT, 
                //           devices[i].name, devices[i].path) == -1) {
                //    result_str = NULL;
		//}
		//printf("%s  %d\n",devices[i].name,devices[i].fd);
		 
		if (asprintf(&result_str, "%s", devices[i].path) == -1) { // Format now only includes the path
    result_str = NULL;
}
		//strcpy(result_str,devices[i].path);

                // Cleanup
                for (int j = 0; j < device_count; j++) close(devices[j].fd);
                return result_str;
		}
		
            }
        }
    

    // Cleanup if no device found
    for (int i = 0; i < device_count; i++) close(devices[i].fd);
    
    return NULL;
}


// Connect to keyboard device
int connect_to_keyboard(const char *path) {
    // Open device in read-only mode
    keyboard_fd = open(path, O_RDONLY);
    if (keyboard_fd == -1) return -1;
    return 0;
}

// Check if specific key is currently pressed
int is_key_down(int key) {
    if (keyboard_fd == -1) return 0;  // Not connected
    
    // Buffer to hold key states (large enough for all standard keys)
    unsigned char key_states[KEY_MAX/8 + 1];
    memset(key_states, 0, sizeof(key_states));

    // Get current key states from driver
    if (ioctl(keyboard_fd, EVIOCGKEY(sizeof(key_states)), key_states) == -1) {
        return 0;
    }

    // Calculate byte and bit position for this key
    int byte_index = key / 8;
    int bit_index = key % 8;

    // Check if the relevant bit is set
    return (key_states[byte_index] & (1 << bit_index)) ? 1 : 0;
}

// Cleanup and disconnect
void disconnect_keyboard() {
    if (keyboard_fd != -1) {
        close(keyboard_fd);
        keyboard_fd = -1;
    }
}







/*

int main() {
    char *keyboard_info = get_keyboard_id();
    if (keyboard_info) {
	printf("%s\n", keyboard_info);
	if (connect_to_keyboard(keyboard_info) == 0) {


	  int loop = 1;

	  while(loop){

	    printf("\r%s", is_key_down(KEY_B) ? "DOWN" : "UP");
	    fflush(stdout);
	    if(is_key_down(KEY_Q)){loop = 0;}

	  }    
	disconnect_keyboard();
	  



	}
        free(keyboard_info);
    }
    return 0;
}


*/
