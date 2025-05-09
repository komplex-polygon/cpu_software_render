//#include <stdbool.h>
//#include <math.h>

/*

int lines_intersect(line a, line b) {
    
    float a_x1 = a.x1;
    float a_y1 = a.y1;
    float a_x2 = a.x2;
    float a_y2 = a.y2;

    float b_x1 = b.x1;
    float b_y1 = b.y1;
    float b_x2 = b.x2;
    float b_y2 = b.y2;

    float dx_a = a_x2 - a_x1;
    float dy_a = a_y2 - a_y1;
    float dx_b = b_x2 - b_x1;
    float dy_b = b_y2 - b_y1;

    float cross = dx_a * dy_b - dy_a * dx_b;
    const float epsilon = 1e-8f;

    if (fabsf(cross) < epsilon) {
        return 1;
    }

    float numerator_t = (b_y1 - a_y1) * dx_b - (b_x1 - a_x1) * dy_b;
    float numerator_s = (b_y1 - a_y1) * dx_a - (b_x1 - a_x1) * dy_a;

    float t = numerator_t / (-cross);
    float s = numerator_s / (-cross);

    if(t >= 0.0f && t <= 1.0f && s >= 0.0f && s <= 1.0f){
      return 1;
    }
    else{
      return 0;
    }
}



*/

/*

int pointInTriangle(float px, float py, float ax, float ay, float bx, float by, float cx, float cy) {
    float cross1, cross2;

    // Check edge AB against point C and P
    cross1 = (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
    cross2 = (bx - ax) * (py - ay) - (by - ay) * (px - ax);
    if (cross1 * cross2 < 0) return 0;

    // Check edge BC against point A and P
    cross1 = (cx - bx) * (ay - by) - (cy - by) * (ax - bx);
    cross2 = (cx - bx) * (py - by) - (cy - by) * (px - bx);
    if (cross1 * cross2 < 0) return 0;

    // Check edge CA against point B and P
    cross1 = (ax - cx) * (by - cy) - (ay - cy) * (bx - cx);
    cross2 = (ax - cx) * (py - cy) - (ay - cy) * (px - cx);
    if (cross1 * cross2 < 0) return 0;

    return 1;
}


*/

int is_point_inside_triangle(triangle a, point b) {



    float ax = a.a.x;
    float ay = a.a.y;
    float bx = a.b.x;
    float by = a.b.y;
    float cx = a.c.x;
    float cy = a.c.y;

    float px = b.x;
    float py = b.y;



    // Calculate cross products for each edge
    float cross1 = (bx - ax) * (py - ay) - (by - ay) * (px - ax);
    float cross2 = (cx - bx) * (py - by) - (cy - by) * (px - bx);
    float cross3 = (ax - cx) * (py - cy) - (ay - cy) * (px - cx);

    // Check if all cross products are non-negative or all non-positive
    return ((cross1 >= 0 && cross2 >= 0 && cross3 >= 0) ||
            (cross1 <= 0 && cross2 <= 0 && cross3 <= 0));
}





point calculatePoint(point A, point B, point C, float a, float b, float c) {
    point p;
    p.x = a * A.x + b * B.x + c * C.x;
    p.y = a * A.y + b * B.y + c * C.y;
    return p;
}


void calculate_barycentric(
    point a, point b, point c, point p,
    float *a_weight, float *b_weight, float *c_weight
) {
    // Calculate denominator (twice the signed area of triangle ABC)
    float denom = ( (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y) );
    
    // Check if the triangle is degenerate (using a small epsilon to avoid floating-point precision issues)
    const float epsilon = 1e-8f;
    if (fabsf(denom) < epsilon) {
        *a_weight = *b_weight = *c_weight = 0.0f;
        return;
    }
    
    // Calculate numerators for barycentric coordinates b and c
    float b_num = ( (p.x - a.x) * (c.y - a.y) - (c.x - a.x) * (p.y - a.y) );
    float c_num = ( (b.x - a.x) * (p.y - a.y) - (p.x - a.x) * (b.y - a.y) );
    
    // Compute barycentric coordinates
    float b_coord = b_num / denom;
    float c_coord = c_num / denom;
    float a_coord = 1.0f - b_coord - c_coord;
    
    *a_weight = a_coord;
    *b_weight = b_coord;
    *c_weight = c_coord;
}







point_3d project(const camera *cam, const point_3d *p) {
    // Translate the point to camera space
    float dx = p->x - cam->pos.x;
    float dy = p->y - cam->pos.y;
    float dz = p->z - cam->pos.z;

    // Compute distance from camera (projected z)
    float distance = sqrtf(dx * dx + dy * dy + dz * dz);

    // Rotate around X-axis by negative pitch
    float cp = cosf(-cam->pitch);
    float sp = sinf(-cam->pitch);
    float y_rot = dy * cp - dz * sp;
    float z_rot = dy * sp + dz * cp;

    // Update dz after X rotation
    dz = z_rot;

    // Rotate around Y-axis by negative yaw
    float cy = cosf(-cam->yaw);
    float sy = sinf(-cam->yaw);
    float x_proj = dx * cy + dz * sy;
    float z_proj = -dx * sy + dz * cy;

    // Create and return the projected point
    point_3d projected = {
        .x = x_proj,
        .y = y_rot,
        .z = distance
    };
    return projected;
}


/*
#include <math.h>

typedef struct {
    float x;
    float y;
    float z;
} point_3d;

typedef struct {
    point_3d pos;
    float yaw;   // Left to right angle (around world Z-axis)
    float pitch; // Up and down angle (around camera's local X-axis)
} camera;


*/

point_3d project_orthographic(const point_3d *p, const camera *cam) {
    // Translate the point by the camera's position
    point_3d translated = {
        p->x - cam->pos.x,
        p->y - cam->pos.y,
        p->z - cam->pos.z
    };

    // Precompute trigonometric values for yaw and pitch
    float cos_yaw = cosf(cam->yaw);
    float sin_yaw = sinf(cam->yaw);
    float cos_pitch = cosf(cam->pitch);
    float sin_pitch = sinf(cam->pitch);

    // Compute the right vector (x-axis in camera space)
    point_3d right = { cos_yaw, -sin_yaw, 0.0f };

    // Compute the forward vector (direction the camera is facing)
    point_3d forward = {
        sin_yaw * cos_pitch,
        cos_yaw * cos_pitch,
        sin_pitch
    };

    // Compute the up vector (y-axis in camera space, perpendicular to right and forward)
    point_3d up = {
        -sin_yaw * sin_pitch,
        -cos_yaw * sin_pitch,
        cos_pitch
    };

    // Apply the rotation matrix to the translated point
    point_3d projected;
    projected.x = translated.x * right.x + translated.y * right.y + translated.z * right.z;
    projected.y = translated.x * up.x + translated.y * up.y + translated.z * up.z;
    projected.z = translated.x * forward.x + translated.y * forward.y + translated.z * forward.z;

    return projected;
}




point_3d project_point(point_3d world_point, camera cam, float focal_length) {
    // Translate to camera-relative coordinates
    point_3d translated = {
        world_point.x - cam.pos.x,
        world_point.y - cam.pos.y,
        world_point.z - cam.pos.z
    };

    // Precompute trigonometric values
    float cos_yaw = cosf(cam.yaw);
    float sin_yaw = sinf(cam.yaw);
    float cos_pitch = cosf(cam.pitch);
    float sin_pitch = sinf(cam.pitch);

    // Apply view rotation matrix
    point_3d rotated;
    rotated.x = translated.x * cos_yaw - translated.y * sin_yaw;
    rotated.y = translated.x * (-sin_yaw * sin_pitch) +
                translated.y * (-cos_yaw * sin_pitch) +
                translated.z * cos_pitch;
    rotated.z = translated.x * (-sin_yaw * cos_pitch) +
                translated.y * (-cos_yaw * cos_pitch) -
                translated.z * sin_pitch;

    // Compute distance from the camera
    float distance = sqrtf(rotated.x * rotated.x +
                           rotated.y * rotated.y +
                           rotated.z * rotated.z);

    // Handle division by zero
    if (distance == 0) {
        return (point_3d){0, 0, 0};
    }

    // Apply perspective projection
    float inv_dist = 1.0f / distance;
    point_3d projected = {
        rotated.x * inv_dist * focal_length,
        rotated.y * inv_dist * focal_length,
        distance
    };

    return projected;
}



void project_point2(const point_3d *world_point, const camera *cam, point_3d *projected) {
    // Translate to camera-relative coordinates
    float tx = world_point->x - cam->pos.x;
    float ty = world_point->y - cam->pos.y;
    float tz = world_point->z - cam->pos.z;

    // Compute Euclidean distance from the camera
    projected->z = sqrtf(tx * tx + ty * ty + tz * tz);

    // Precompute trigonometric values
    float cos_yaw = cosf(cam->yaw);
    float sin_yaw = sinf(cam->yaw);
    float cos_pitch = cosf(cam->pitch);
    float sin_pitch = sinf(cam->pitch);

    // Compute view space coordinates
    float view_x = tx * sin_yaw - ty * cos_yaw;
    float view_y = (-tx * cos_yaw * sin_pitch) - (ty * sin_yaw * sin_pitch) + (tz * cos_pitch);
    float view_z = tx * cos_yaw * cos_pitch + ty * sin_yaw * cos_pitch + tz * sin_pitch;

    // Apply perspective projection
    if (view_z != 0.0f) {
        projected->x = view_x / view_z;
        projected->y = view_y / view_z;
    } else {
        // Handle division by zero by setting x and y to 0
        projected->x = 0.0f;
        projected->y = 0.0f;
    }
}


point_3d project_point3(point_3d world_point, camera cam) {
    point_3d translated = {
        world_point.x - cam.pos.x,
        world_point.y - cam.pos.y,
        world_point.z - cam.pos.z
    };

    // Convert angles to radians
    float yaw = cam.yaw * (M_PI / 180.0f);
    float pitch = cam.pitch * (M_PI / 180.0f);

    // Compute forward vector
    point_3d forward = {
        cosf(yaw) * cosf(pitch),
        sinf(pitch),
        sinf(yaw) * cosf(pitch)
    };

    // Compute right vector (cross product of forward and world up [0,1,0])
    point_3d right = {
        forward.z,
        0.0f,
        -forward.x
    };
    // Normalize right vector
    float right_len = sqrtf(right.x * right.x + right.y * right.y + right.z * right.z);
    if (right_len > 0) {
        right.x /= right_len;
        right.y /= right_len;
        right.z /= right_len;
    } else {
        right = (point_3d){1.0f, 0.0f, 0.0f}; // Fallback to prevent division by zero
    }

    // Compute up vector (cross product of right and forward)
    point_3d up = {
        right.y * forward.z - right.z * forward.y,
        right.z * forward.x - right.x * forward.z,
        right.x * forward.y - right.y * forward.x
    };
    // Normalize up vector
    float up_len = sqrtf(up.x * up.x + up.y * up.y + up.z * up.z);
    if (up_len > 0) {
        up.x /= up_len;
        up.y /= up_len;
        up.z /= up_len;
    } else {
        up = (point_3d){0.0f, 1.0f, 0.0f}; // Fallback
    }

    // Rotate the translated point using the view matrix (R, U, -F as rows)
    point_3d rotated = {
        translated.x * right.x + translated.y * right.y + translated.z * right.z,
        translated.x * up.x + translated.y * up.y + translated.z * up.z,
        -(translated.x * forward.x + translated.y * forward.y + translated.z * forward.z)
    };

    // Check if the point is behind the camera
    if (rotated.z <= 0) {
        point_3d invalid = {NAN, NAN, -rotated.z};
        return invalid;
    }

    // Compute perspective projection
    float fov_rad = cam.fov * (M_PI / 180.0f);
    float scale = 1.0f / tanf(fov_rad / 2.0f);

    point_3d projected = {
        (rotated.x / rotated.z) * scale,
        (rotated.y / rotated.z) * scale,
        rotated.z
    };

    return projected;
}
