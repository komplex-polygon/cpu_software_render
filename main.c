



//gcc main.c libs/lodepng.c -lm -O3 -o exec


//struct particle {
//  float x;
//  float y;
//  float xa;
//  float ya;
//};

// gcc -lm -O3 -o exec main.c

#include <sys/ioctl.h>
//#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <termios.h>
#include <signal.h>
#include <time.h>

#include <stdbool.h>
#include <fcntl.h>

#include <dirent.h>
#include <linux/input.h>
#include <sys/select.h>
#include <sys/ioctl.h>

#include <linux/fb.h>
#include <sys/mman.h>

typedef struct {
  float x;
  float y;
} point;

typedef struct {
  float x;
  float y;
  float z;
} point_3d;

typedef struct {
  point_3d a;
  point_3d b;
  point_3d c;
} vertex;

typedef struct {
  point a;
  point b;
  point c;
} triangle;

typedef struct {
  point_3d pos;
  float yaw; //the left to right angle.
  float pitch; //the up and down angle.
  float fov;
} camera;


typedef struct {
  vertex vertex;
  triangle uv;
  int texture;
} polygon;

typedef struct {
  unsigned char *image;
  unsigned width;
  unsigned height;
} texture;

typedef struct {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} color;

int key_bord = true;
int opt_dith_size = 4;
int opt_monitor_mode = 0; //0 = 8px (monochrome), 1 = 2px (color), 2 = 2px (monochrome); 3 = ascii; 4 = framebuffer
float opt_zoom = 1;
int opt_spinn = 0;

int polygon_buffer_len = 0;
polygon *polygon_buffer = NULL;
float pi = 3.141592;

#include "libs/lodepng.h"

#include "libs/keybord.c"
#include "libs/lines.c"
#include "libs/test.c"
//#include "model/tv/bake_crt.h"
//#include "model/tv/bake_floor.h"

texture textures[16];

static struct termios original;

#define EXIT_SEQUENCE "\033[?25h\033[0m\033[3J\033[2J\033c\033[HHave a good day :3\n"

static const char exit_sequence[] = EXIT_SEQUENCE;
static const size_t exit_len = sizeof(EXIT_SEQUENCE) - 1; // exclude null terminator

  int fbfd;
  int fb_width;// = vinfo.xres;
  int fb_height;// = vinfo.yres;
  int fb_bpp;// = vinfo.bits_per_pixel;
  int fb_bytes;// = fb_bpp / 8;
  int stride;// = finfo.line_length; 
  int fb_data_size;// = fb_height * stride;
  char *fbdata;

void restore_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &original);
    write(STDOUT_FILENO, exit_sequence, exit_len);
    tcdrain(STDOUT_FILENO);
    disconnect_keyboard();
    munmap (fbdata, fb_data_size);
    close (fbfd);

}


void sigint_handler(int sig) {
    restore_terminal();
    _exit(EXIT_SUCCESS); // Exit immediately without running atexit() handlers
}


float dist(float x, float y){
  return sqrt(x*x+y*y);
}

void print_braille_char(unsigned int code_point) {
  // Check if the code point is in the Braille range (10240-10495)
  if (code_point >= 10240 && code_point <= 10495) {
    // UTF-8 3-byte encoding
    putchar(0xE0 | (code_point >> 12));         // Leading byte
    putchar(0x80 | ((code_point >> 6) & 0x3F)); // Middle byte
    putchar(0x80 | (code_point & 0x3F));        // Trailing byte
  } else {
    printf("?"); // Fallback for out-of-range characters
  }
}


int main(int argc, char *argv[]) {

  

  for(int logs = 1;logs < argc;logs++){
    //printf("#%s#\n",argv[logs]);
    /*if (strcmp(argv[logs],"-h") == 0) {
	printf("list of parameters:\n-no-root //used when using ssh. removes som featches for conpatebility.\n-mode # //what rendering mode to use mono or color\n");
	return 1;
    }
    */
    if (strcmp(argv[logs],"-no-root") == 0) {
	printf("+ (-no-root)\n");
	//return 1;
	key_bord = 2;
    }

    if (strcmp(argv[logs],"-mode") == 0) {
            int dithg = 69;

	if(logs + 1 < argc){    

	  if(strcmp(argv[logs + 1],"mono") == 0){dithg = 0;}
	  if(strcmp(argv[logs + 1],"color") == 0){dithg = 1;}
	  if(strcmp(argv[logs + 1],"buffer") == 0){dithg = 4;}
	}

      if(dithg != 69){
	opt_monitor_mode = dithg;
	printf("+ (-mode %d)\n",dithg);
	logs += 1;
      }
      else{
	printf("- (-mode) //missing parameters.\n");
	return 1;
      }
      	//printf("list of parameters:\n-ssh //used when using ssh. removes som featches for conpatebility.\n");
	//return 1;
    }
    if (strcmp(argv[logs],"-zoom") == 0) {
            //int dithg = 69;

	if(logs + 1 < argc){    

	  opt_zoom = atof(argv[logs+1]);
	  printf("+ (-zoom %f)\n",opt_zoom);
	  //if(strcmp(argv[logs + 1],"mono") == 0){dithg = 0;}
	  //if(strcmp(argv[logs + 1],"color") == 0){dithg = 1;}
	  //if(strcmp(argv[logs + 1],"buffer") == 0){dithg = 4;}
	}

      /*if(dithg != 69){
	opt_monitor_mode = dithg;
	printf("+ (-mode %d)\n",dithg);
	logs += 1;
      }
      else{
	printf("- (-mode) //missing parameters.\n");
	return 1;
      }
      */
      	//printf("list of parameters:\n-ssh //used when using ssh. removes som featches for conpatebility.\n");
	//return 1;
    }

    if (strcmp(argv[logs],"-spin") == 0) {
	printf("+ (-spin)\n");
	//return 1;
	opt_spinn = 1;
    }



  }

  usleep(500000);


  

  if(opt_monitor_mode == 4){
    fbfd = open ("/dev/fb0", O_RDWR);
    if (fbfd >= 0){

      struct fb_fix_screeninfo finfo;
      struct fb_var_screeninfo vinfo;

      ioctl (fbfd, FBIOGET_FSCREENINFO, &finfo);
      ioctl (fbfd, FBIOGET_VSCREENINFO, &vinfo);

      fb_width = vinfo.xres;
      fb_height = vinfo.yres;
      fb_bpp = vinfo.bits_per_pixel;
      fb_bytes = fb_bpp / 8;
      stride = finfo.line_length; // In bytes, not pixels

//int fb_data_size = fb_width * fb_height * fb_bytes;
//
      //printf("x:%d y:%d \n",fb_width,fb_height);
      //return 0;

      fb_data_size = fb_height * stride;

      fbdata = mmap (0, fb_data_size, 
        PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, (off_t)0);


    }
    else{
      printf("can not read framebuffer :( try sudo...\n");
      return 0;
    }

  }

  
  if(key_bord == 1){  
   char *keyboard_info = get_keyboard_id();

  if (keyboard_info) {

    if (connect_to_keyboard(keyboard_info) != 0) {
      return 0;
    }
  }

  free(keyboard_info);

  }


   // srand(time(0));
    struct termios new;

    // Get current terminal settings
    tcgetattr(STDIN_FILENO, &original);

    // Register restore function to run at exit
    atexit(restore_terminal);


    // Copy settings to modify
    new = original;


    new.c_lflag &= ~(ECHO | ICANON);
    // Set minimum characters and timeout (non-blocking read)
    new.c_cc[VMIN] = 0;   // Read 1 character at a time
    new.c_cc[VTIME] = 0;  // No timeout

    // Apply new settings immediately
    tcsetattr(STDIN_FILENO, TCSANOW, &new);


    struct sigaction sa = {
        .sa_handler = sigint_handler,
        .sa_flags = SA_RESTART
    };
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
  

   
  // [  ░░▒▒▓▓██] 



  int screen_x = 0;
  int screen_y = 0;
  int old_col = 0;
  int old_row = 0;
  int *screen_old = NULL;
  color *screen_old_color = NULL;
  color *screen = NULL;
  float *screen_depth = NULL;
  camera cam = {.pos = {1024,1024,1024}, .yaw = -pi/4, .pitch = -pi/4, .fov = 70};
  polygon_buffer = malloc(sizeof(polygon) * 1024);


  struct timespec start, end;

  /*

  vertex test = {.a = {0,0,0}, .b = {200,200,0}, .c = {0,200,0}};
  triangle test2 = {.a = {0,0}, .b = {512,512}, .c = {0,512}};
  polygon_buffer[0].vertex = test;
  polygon_buffer[0].uv = test2;

  vertex rtest = {.a = {0,0,0}, .b = {200,200,0}, .c = {200,0,0}};
  triangle rtest2 = {.a = {0,0}, .b = {512,512}, .c = {512,0}};
  polygon_buffer[1].vertex = rtest;
  polygon_buffer[1].uv = rtest2;


  //memcpy(&triangle_buffer[0],&test,sizeof(triangle));

  polygon_buffer_len += 2;

  */

  //unsigned char* image = 0;
  //unsigned width, height;

  lodepng_decode32_file(&textures[0].image, &textures[0].width, &textures[0].height, "models/bake_crt2.png");

  printf("%d, %d\n",textures[0].width,textures[0].height);

  process_obj("models/tv.obj",polygon_buffer,1,&polygon_buffer_len,0);

  lodepng_decode32_file(&textures[1].image, &textures[1].width, &textures[1].height, "models/bake_flor.png");

  printf("%d, %d\n",textures[1].width,textures[1].height);

  process_obj("models/floor.obj",polygon_buffer,1,&polygon_buffer_len,1);
  //textures[0] = lode;


  //process_obj("model/tv/floor.obj",polygon_buffer,1024,&polygon_buffer_len,1);


  //textures[1] = bake_floor_bin;



  printf("%d\n",polygon_buffer_len);

  //return 1;


  struct winsize ws;

//ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
//screen_x = (ws.ws_col) * 2;
//screen_y = (ws.ws_row) * 4;



  printf("\x1b[?25l");


  float player_x = 0;
  float player_y = 0;   

 
    float up = -pi/4;
    float left = pi/4;
    float distss = 2;

    float zoom;
    switch (opt_monitor_mode){
      case 0:
	zoom = 0.5 * opt_zoom;
      break;
      case 1:
	zoom = 0.25 * opt_zoom;
      break;
      case 4:
	zoom = 16 * opt_zoom;
      break;
    }




    if(opt_monitor_mode == 4){

      screen_x = fb_width;
      screen_y = fb_height;
      //screen_old_color = realloc(screen_old_color, (screen_x * screen_y) * sizeof(color));
      //memset(screen_old_color, 0, (screen_x * screen_y) * sizeof(color));


      screen = realloc(screen, (screen_y * screen_x) * sizeof(color));
      screen_depth = realloc(screen_depth, (screen_y * screen_x) * sizeof(float));
      
      //memset (fbdata, 0, fb_data_size);

    }






    int loop = 1;

  while (loop) {

    clock_gettime(CLOCK_MONOTONIC, &start);

    if(key_bord == 1){
    if(is_key_down(KEY_O)){
      loop = 0;
      break;
    }
    }
  
    
    //float acs_x = 0;
    //float acs_y = 0;

    /*


    if(is_key_down(KEY_UP)){
      //player_y += 0.1;
      cam.pitch += 0.1;
    }
    if(is_key_down(KEY_DOWN)){
      //player_y -= 0.1;
      cam.pitch -= 0.1;
    }

    */

   /* 

    if(is_key_down(KEY_W)){
      //player_y += 0.1;
      player_y += 0.5;
    }
    if(is_key_down(KEY_S)){
      //player_y -= 0.1;
      player_y -= 0.5;
    }
*/



    if(opt_spinn == 1){
      left += 0.1;

    }

   if(key_bord == 1){ 

    if(is_key_down(KEY_M)){
      //player_y += 0.1;
      //player_x += 0.5;
      zoom *= 0.95;
    }
    if(is_key_down(KEY_N)){
      //player_y -= 0.1;
      //player_x -= 0.5;
      //
      //
      zoom *= 1.05;

    }
    
    
    
    if(is_key_down(KEY_RIGHT)){
      //player_x += 0.1;
      left += 0.1;
    }   
    if(is_key_down(KEY_LEFT)){
      //player_x -= 0.1;
      left -= 0.1;
    } 

    if(is_key_down(KEY_UP)){
      //player_x += 0.1;
      up -= 0.1;
    }   
    if(is_key_down(KEY_DOWN)){
      //player_x -= 0.1;
      up += 0.1;
    }

  }

    float cam_h = sin(up);
    float cam_d = cos(up);

    float cam_ly = sin(left) * cam_d;
    float cam_lx = cos(left) * cam_d;

    cam.pos.x = cam_lx + player_x;
    cam.pos.y = cam_ly + player_y;
    cam.pos.z = cam_h;

    cam.yaw = left + pi * 2;
    cam.pitch = up + pi * 2;
    


    //float dissss = sqrt(acs_x*acs_x+acs_y*acs_y);

    //if(dissss > 0.01){
//
    //  player_x += acs_x / dissss * 0.5;
    //  player_y += acs_y / dissss * 0.5;

    //}

    
    //camera_hist[camera_init].x = player_x;
    //camera_hist[camera_init].y = player_y;
    //camera_init += 1;
    /*

    if(camera_init >= 128){camera_init = 0;}

    float camera_temp_x = 0;
    float camera_temp_y = 0;

    for(int camera_temp_init = 0; camera_temp_init < 128; camera_temp_init++){
      camera_temp_x += camera_hist[camera_temp_init].x;
      camera_temp_y += camera_hist[camera_temp_init].y;
    }

    camera_x = camera_temp_x / 128;
    camera_y = camera_temp_y / 128;

    */


    if(opt_monitor_mode != 4){

      



    

      ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);

        if (old_row != ws.ws_row || old_col != ws.ws_col)
        {
            //system("clear");
	    printf("\033[3J\033[2J");
	    //printf("\e[1;1H\e[2J");
	    //
	    switch (opt_monitor_mode){

	    case 0:
            screen_x = (ws.ws_col) * 2;
            screen_y = (ws.ws_row) * 4;
	    screen_old = realloc(screen_old, (ws.ws_row * ws.ws_col) * sizeof(int));
	    memset(screen_old, 0, (ws.ws_row * ws.ws_col) * sizeof(int));
	    break;
	    
	    case 1:
            screen_x = (ws.ws_col) * 1;
            screen_y = (ws.ws_row) * 2;
	    screen_old_color = realloc(screen_old_color, (screen_x * screen_y) * sizeof(color));
	    memset(screen_old_color, 0, (screen_x * screen_y) * sizeof(color));
	    break;

	    case 2:
            screen_x = (ws.ws_col) * 1;
            screen_y = (ws.ws_row) * 2;
	    screen_old = realloc(screen_old, (screen_x * screen_y) * sizeof(int));
	    memset(screen_old, 0, (screen_x * screen_y) * sizeof(int));
	    break;

	    }


            screen = realloc(screen, (screen_y * screen_x) * sizeof(color));
	    screen_depth = realloc(screen_depth, (screen_y * screen_x) * sizeof(float));



            //memset(screen_old, 0, (ws.ws_row * ws.ws_col) * sizeof(int));
        }

        old_row = ws.ws_row;
        old_col = ws.ws_col;

	}

        memset(screen, 0, (screen_y * screen_x) * sizeof(color));
	memset(screen_depth, 0, (screen_y * screen_x) * sizeof(float));

	/*
	for(int fill_y = 0;fill_y < screen_y; fill_y++){
	  for(int fill_x = 0;fill_x < screen_x; fill_x++){
	    float virt_x = fill_x - screen_x / 2.0 + camera_x;
	    float virt_y = fill_y - screen_y / 2.0 + camera_y;

	    int value2 = 2;

	    if( virt_x - floor(virt_x/16)*16 < 1.0){value2 = 1;}
	    if( virt_y - floor(virt_y/16)*16 < 1.0){value2 = 1;}
	      


	    screen[fill_x + fill_y * screen_x] = value2;
	  }
	} 

	*/


	




      //screen[player_x + player_y * screen_x] = 1;

      /*

      polygon_buffer[0].vertex.b.x = player_x;
      polygon_buffer[0].vertex.b.y = player_y;

      polygon_buffer[1].vertex.b.x = player_x;
      polygon_buffer[1].vertex.b.y = player_y;

      */


      for(int inss = 0;inss < polygon_buffer_len;inss++){


	vertex temp;
	triangle temp_2d;

	temp.a = project_orthographic(&polygon_buffer[inss].vertex.a,&cam);
	temp.b = project_orthographic(&polygon_buffer[inss].vertex.b,&cam);
	temp.c = project_orthographic(&polygon_buffer[inss].vertex.c,&cam);

	//float fov =  1.0f / tanf(cam.fov / 2.0f);

	//temp.a = project_point3(polygon_buffer[inss].vertex.a,cam);
	//temp.b = project_point3(polygon_buffer[inss].vertex.b,cam);
	//temp.c = project_point3(polygon_buffer[inss].vertex.c,cam);

	//project_point2(&polygon_buffer[inss].vertex.a,&cam,&temp.a);
	//project_point2(&polygon_buffer[inss].vertex.b,&cam,&temp.b);
	//project_point2(&polygon_buffer[inss].vertex.c,&cam,&temp.c);


	temp_2d.a.x = screen_x - (temp.a.x * zoom + screen_x / 2);
	temp_2d.a.y = screen_y - (temp.a.y * zoom + screen_y / 2);

	temp_2d.b.x = screen_x - (temp.b.x * zoom + screen_x / 2);
	temp_2d.b.y = screen_y - (temp.b.y * zoom + screen_y / 2);
	
	temp_2d.c.x = screen_x - (temp.c.x * zoom + screen_x / 2);
	temp_2d.c.y = screen_y - (temp.c.y * zoom + screen_y / 2);



	int s_x = screen_x;

	if(s_x > floor(temp_2d.a.x)){s_x = floor(temp_2d.a.x);}
	if(s_x > floor(temp_2d.b.x)){s_x = floor(temp_2d.b.x);}
	if(s_x > floor(temp_2d.c.x)){s_x = floor(temp_2d.c.x);}

	int s_y = screen_y;

	if(s_y > floor(temp_2d.a.y)){s_y = floor(temp_2d.a.y);}
	if(s_y > floor(temp_2d.b.y)){s_y = floor(temp_2d.b.y);}
	if(s_y > floor(temp_2d.c.y)){s_y = floor(temp_2d.c.y);}

	int e_x = 0;

	if(e_x < floor(temp_2d.a.x)+1){e_x = floor(temp_2d.a.x)+1;}
	if(e_x < floor(temp_2d.b.x)+1){e_x = floor(temp_2d.b.x)+1;}
	if(e_x < floor(temp_2d.c.x)+1){e_x = floor(temp_2d.c.x)+1;}

	int e_y = 0;

	if(e_y < floor(temp_2d.a.y)+1){e_y = floor(temp_2d.a.y)+1;}
	if(e_y < floor(temp_2d.b.y)+1){e_y = floor(temp_2d.b.y)+1;}
	if(e_y < floor(temp_2d.c.y)+1){e_y = floor(temp_2d.c.y)+1;}

	if(s_x < screen_x && s_y < screen_y && e_x > 0 && e_y > 0){

	if(s_x < 0){s_x = 0;}
	if(s_y < 0){s_y = 0;}
	if(e_x > screen_x){e_x = screen_x;}
	if(e_y > screen_y){e_y = screen_y;}

	for(int tri_y = s_y;tri_y < e_y;tri_y++){
	  for(int tri_x = s_x;tri_x < e_x;tri_x++){
	    point screens = {tri_x,tri_y};
	    if(is_point_inside_triangle(temp_2d,screens)){
	      //screen[tri_x + tri_y * screen_x] = 1;
	      //float ad = dist(polygon_buffer[inss].vertex.a.x - tri_x, polygon_buffer[inss].vertex.a.y - tri_y);
	      //float bd = dist(polygon_buffer[inss].vertex.b.x - tri_x, polygon_buffer[inss].vertex.b.y - tri_y);
	      //float cd = dist(polygon_buffer[inss].vertex.c.x - tri_x, polygon_buffer[inss].vertex.c.y - tri_y);

	      //float totol = ad + bd + cd;
	      //float totol2 = (1 - ad / totol) + (1 - bd / totol) + (1 - cd / totol); 

	      float ua, vb, wc;
	      calculate_barycentric(temp_2d.a, temp_2d.b, temp_2d.c, screens, &ua, &vb, &wc);

	      float depth = temp.a.z * ua + temp.b.z * vb + temp.c.z * wc;

	      if(screen_depth[tri_x + tri_y * screen_x] == 0 || screen_depth[tri_x + tri_y * screen_x] > depth){

		screen_depth[tri_x + tri_y * screen_x] = depth;

		point uv_cord = calculatePoint(polygon_buffer[inss].uv.a,polygon_buffer[inss].uv.b,polygon_buffer[inss].uv.c,ua,vb,wc);

		if(uv_cord.x >= 0 && uv_cord.x < 1 && uv_cord.y >= 0 && uv_cord.y < 1){
		  int uv_int_x = floor(uv_cord.x * textures[polygon_buffer[inss].texture].width);
		  int uv_int_y = textures[polygon_buffer[inss].texture].height - floor(uv_cord.y * textures[polygon_buffer[inss].texture].height);

		  //printf("goooog %d %d %f %f\n",uv_int_x,uv_int_y,uv_cord.x,uv_cord.y); 
		  screen[tri_x + tri_y * screen_x].r = textures[polygon_buffer[inss].texture].image[(uv_int_x + uv_int_y * textures[polygon_buffer[inss].texture].width)*4+0]; 
		  screen[tri_x + tri_y * screen_x].g = textures[polygon_buffer[inss].texture].image[(uv_int_x + uv_int_y * textures[polygon_buffer[inss].texture].width)*4+1];
		  screen[tri_x + tri_y * screen_x].b = textures[polygon_buffer[inss].texture].image[(uv_int_x + uv_int_y * textures[polygon_buffer[inss].texture].width)*4+2];
		

		}
	      }

	    }

	  }
	}


	}
      }

      //printf("gaaaaa\n");


      int dith[16] = {
      0,128,32,160,
      192,64,224,96,
      48,176,16,144,
      240,112,208,80
      };

      int dith2[4] = {
      0,128,
      192,64
      };



      /*

      for(int dith_y = 0;dith_y < screen_y;dith_y++){
	for(int dith_x = 0;dith_x < screen_x;dith_x++){

	    int local_x;
	    int local_y;

	  switch (opt_dith_size) {
	    case 4:

	    local_x = dith_x % 4;
	    local_y = dith_y % 4;

	    if(screen[dith_x + dith_y * screen_x] > dith[local_x + local_y * 4]){
	      screen[dith_x + dith_y * screen_x] = 1;
	    }
	    else{
	      screen[dith_x + dith_y * screen_x] = 0;
	    }
	    break;

	    case 2:

	    local_x = dith_x % 2;
	    local_y = dith_y % 2;

	    if(screen[dith_x + dith_y * screen_x] > dith2[local_x + local_y * 2]){
	      screen[dith_x + dith_y * screen_x] = 1;
	    }
	    else{
	      screen[dith_x + dith_y * screen_x] = 0;
	    }

	    break;

	    default:
	      screen[dith_x + dith_y * screen_x] = 1;
	    break;
	  

	  }
	  

	}
      }
	*/


     if(opt_monitor_mode == 4){
      for(int gg_fy = 0;gg_fy < screen_y;gg_fy++){
	for(int gg_fx = 0;gg_fx < screen_x;gg_fx++){
	  int offset = (gg_fy * stride) + (gg_fx * fb_bytes);

	  color debug = {.b = 255};

	  if(gg_fx > screen_x / 2){debug.r = 255;}
	  if(gg_fy > screen_y / 2){debug.g = 255;}


	  fbdata [offset + 0] = screen[gg_fx + gg_fy * screen_x].b;
	  fbdata [offset + 1] = screen[gg_fx + gg_fy * screen_x].g;
	  fbdata [offset + 2] = screen[gg_fx + gg_fy * screen_x].r;
	  fbdata [offset + 3] = 0;

	}
      }

     }
     else{

	//printf("\033[H");
	for (int row = 0; row < old_row; row++)
        {
            for (int col = 0; col < old_col; col++)
            {

		  switch (opt_monitor_mode){
		  case 0:

		  int bufferl[8];

		  for(int jjy = 0; jjy < 4;jjy++){
		    for(int jjx = 0; jjx < 2;jjx++){
		      int pri = floor((screen[(col*2+jjx) + (row*4+jjy) * screen_x].r + screen[(col*2+jjx) + (row*4+jjy) * screen_x].g + screen[(col*2+jjx) + (row*4+jjy) * screen_x].b) / 3);
		      int local_x = (col*2+jjx) % 4;
		      int local_y = (row*4+jjy) % 4;

		      if(pri > dith[local_x + local_y * 4]){

			bufferl[jjx + jjy * 2] = 1;

		      }
		      else{
			bufferl[jjx + jjy * 2] = 0;
		      }

		    }

		  }
		  

		  int value = 0;

		  int bx = col * 2;
		  int by = row * 4;

		  value += 1 * bufferl[(0)+(0)*2];
		  value += 2 * bufferl[(0)+(1)*2];
		  value += 4 * bufferl[(0)+(2)*2];
		  value += 8 * bufferl[(1)+(0)*2];
		  value += 16 * bufferl[(1)+(1)*2];
		  value += 32 * bufferl[(1)+(2)*2];
		  value += 64 * bufferl[(0)+(3)*2];
		  value += 128 * bufferl[(1)+(3)*2];

		  if(value != screen_old[col + row * old_col]){
		    printf("\x1b[%d;%dH",row + 1,col + 1);
		    print_braille_char(10240 + value);
		    screen_old[col + row * old_col] = value;
		  }

		  break;
		  
		  case 1:

		  int upgy = (row * 2) * screen_x + col;
		  

		  int downgy = (row * 2 + 1) * screen_x + col;
		  

		  if(screen[upgy].r != screen_old_color[upgy].r || screen[upgy].g != screen_old_color[upgy].g || screen[upgy].b != screen_old_color[upgy].b || screen[downgy].r != screen_old_color[downgy].r || screen[downgy].g != screen_old_color[downgy].g || screen[downgy].b != screen_old_color[downgy].b){

		   printf("\x1b[%d;%dH\x1b[38;2;%d;%d;%dm\x1b[48;2;%d;%d;%dm▀\x1b[0m",row + 1,col + 1,screen[upgy].r,screen[upgy].g,screen[upgy].b,screen[downgy].r,screen[downgy].g,screen[downgy].b); 

		   screen_old_color[upgy].r = screen[upgy].r;
		   screen_old_color[upgy].g = screen[upgy].g;
		   screen_old_color[upgy].b = screen[upgy].b;

		   screen_old_color[downgy].r = screen[downgy].r;
		   screen_old_color[downgy].g = screen[downgy].g;
		   screen_old_color[downgy].b = screen[downgy].b;

		  }


		  break;
	







    
		}

	    }

	    //printf("\n");

	}
	fflush(stdout);
	
	}

	clock_gettime(CLOCK_MONOTONIC, &end);

	long long elapsed_sec = end.tv_sec - start.tv_sec;
	long long elapsed_nsec = end.tv_nsec - start.tv_nsec;
	long long elapsed_usec = (elapsed_sec * 1000000) + (elapsed_nsec / 1000);

	

	//memcpy(screen_old, screen, screen_y * screen_x * sizeof(int));
	//screen_old = screen;
	long long delay = 1000000/30-elapsed_usec;
	if(delay > 10){
	  //printf("rendering frame to long...\n");
	  //return 0;
	
	  usleep(delay);
	}

  }

  

  return 1;
}
