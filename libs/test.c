//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

#define MAX_LINE_LENGTH 2048
#define MAX_VERTICES 100000
#define MAX_UVS 100000
#define MAX_FACE_VERTICES 64

//convert boy.png -colorspace gray -depth 8 gray:boy.bin
//xxd -i boy.bin > boy.h

/*

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

*/

typedef struct { float x, y, z; } qVertex;
typedef struct { float u, v; } UV;

typedef struct {
    int v_idx;
    int uv_idx;
} FaceVertex;

void parse_face_component(const char *comp, FaceVertex *result) {
    char buf[256];
    strncpy(buf, comp, sizeof(buf));
    buf[255] = '\0';

    result->v_idx = -1;
    result->uv_idx = -1;

    char *slash1 = strchr(buf, '/');
    if (slash1) {
        *slash1 = '\0';
        result->v_idx = atoi(buf);
        
        char *slash2 = strchr(slash1 + 1, '/');
        if (slash2) {
            *slash2 = '\0';
        }
        result->uv_idx = atoi(slash1 + 1);
    } else {
        result->v_idx = atoi(buf);
    }
}

void process_obj(const char *filename,polygon *test, int image_size, int *len,int texg) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }

    qVertex vertices[MAX_VERTICES];
    UV uvs[MAX_UVS];
    int v_count = 0, uv_count = 0;
    char line[MAX_LINE_LENGTH];
    int face_num = 0;

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v') {
            if (line[1] == ' ' && v_count < MAX_VERTICES) {
                sscanf(line + 2, "%f %f %f",
                       &vertices[v_count].x,
                       &vertices[v_count].y,
                       &vertices[v_count].z);
                v_count++;
            }
            else if (line[1] == 't' && uv_count < MAX_UVS) {
                sscanf(line + 3, "%f %f",
                       &uvs[uv_count].u,
                       &uvs[uv_count].v);
                uv_count++;
            }
        }
        else if (line[0] == 'f') {
            face_num++;
            FaceVertex fvs[MAX_FACE_VERTICES];
            int fv_count = 0;
            int has_uv = 1;

            char *token = strtok(line + 2, " ");
            while (token && fv_count < MAX_FACE_VERTICES) {
                parse_face_component(token, &fvs[fv_count]);
                if (fvs[fv_count].uv_idx == -1) has_uv = 0;
                fv_count++;
                token = strtok(NULL, " ");
            }

            // Triangulate polygon
            for (int j = 2; j < fv_count; j++) {
                //printf("Face %d Triangle %d:\n", face_num, j-1);
                
                // Indices for current triangle
                int indices[3] = {0, j-1, j};

		point tty[3];
		point_3d tty2[3];
                
                for (int k = 0; k < 3; k++) {
                    FaceVertex fv = fvs[indices[k]];
                    qVertex v = vertices[fv.v_idx - 1];
                    
                    //printf("  v%d: (%8.4f, %8.4f, %8.4f)",
                    //       fv.v_idx, v.x, v.y, v.z);

		    tty2[k].x = v.x*400;
		    tty2[k].y = v.z*400;
		    tty2[k].z = v.y*400;
                    
                    if (has_uv && fv.uv_idx > 0) {
                        UV uv = uvs[fv.uv_idx - 1];
                        //printf("  uv%d: (%6.4f, %6.4f)",
                        //     fv.uv_idx, uv.u, uv.v);
			tty[k].x = uv.u;// * image_size;
			tty[k].y = uv.v;// * image_size;


                    }
                    //printf("\n");
                }

		polygon testtt;

		vertex trr = {{tty2[0].x,tty2[0].y,tty2[0].z},{tty2[1].x,tty2[1].y,tty2[1].z},{tty2[2].x,tty2[2].y,tty2[2].z}};

		triangle trr2 = {{tty[0].x,tty[0].y},{tty[1].x,tty[1].y},{tty[2].x,tty[2].y}};

		testtt.vertex = trr;
		testtt.uv = trr2;
        testtt.texture = texg;

		test[*len] = testtt;
		*len += 1;
		

                printf("\n");
            }
        }
    }
    fclose(file);
}

/*

int main(int argc, char *argv[]) {
    if (argc != 2) {
        //printf("Usage: %s <file.obj>\n", argv[0]);
        return 1;
    }
    process_obj(argv[1]);
    return 0;
}

*/
