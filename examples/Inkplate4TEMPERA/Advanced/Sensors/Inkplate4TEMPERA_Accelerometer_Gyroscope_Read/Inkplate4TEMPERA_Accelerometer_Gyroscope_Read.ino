/**
 **************************************************
 * @file        Inkplate4TEMPERA_Accelerometer_Gyroscope_Read_LVGL.ino
 * @brief       Reads the on-board LSM6DS3 accelerometer/gyroscope and visualizes
 *              motion by rotating a wireframe 3D cube on the e-paper display.
 *
 * @details     LVGL rewrite:
 *              - LVGL is used for text labels
 *              - custom pixel-line drawing is used for the cube
 *              - cube drawing area is explicitly defined and fully cleared
 *              - projection scale is reduced so the cube stays inside the area
 *              - rotation math is corrected
 *              - display rotation is explicitly set
 *
 **************************************************/
#ifndef ARDUINO_INKPLATE4TEMPERA
#error "Wrong board selection for this example, please select Inkplate 4 TEMPERA in the boards menu."
#endif

#include <Inkplate-LVGL.h>
#include <math.h>

Inkplate inkplate(INKPLATE_1BIT);

static const int SCREEN_W = 600;
static const int SCREEN_H = 600;

// Cube drawing region
static const int CUBE_X = 100;
static const int CUBE_Y = 100;
static const int CUBE_W = 540;
static const int CUBE_H = 330;
static const int CUBE_CENTER_X = CUBE_X + CUBE_W / 2;
static const int CUBE_CENTER_Y = CUBE_Y + CUBE_H / 2;
static const float CUBE_SCALE = 80.0f;

// Variable that keeps count on how many times the screen has been partially updated
int numRefreshes = 0;

// How many partial updates we want before doing a full refresh
#define NUM_PARTIAL_UPDATES_BEFORE_FULL_REFRESH 10

// Cube vertices
float cube[8][3] = {
    {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
    {-1, -1,  1}, {1, -1,  1}, {1, 1,  1}, {-1, 1,  1}
};

// Cube edges
int edges[12][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0},
    {4, 5}, {5, 6}, {6, 7}, {7, 4},
    {0, 4}, {1, 5}, {2, 6}, {3, 7}
};

// Rotation angles
float angleX = 0.0f;
float angleY = 0.0f;
float angleZ = 0.0f;

float previousAngleX = 0.0f;
float previousAngleY = 0.0f;

// LVGL labels
static lv_obj_t *lbl_title = nullptr;
static lv_obj_t *lbl_acc_x = nullptr;
static lv_obj_t *lbl_acc_y = nullptr;
static lv_obj_t *lbl_acc_z = nullptr;
static lv_obj_t *lbl_gyro_x = nullptr;
static lv_obj_t *lbl_gyro_y = nullptr;
static lv_obj_t *lbl_gyro_z = nullptr;
static lv_obj_t *lbl_error = nullptr;

// Forward declarations
void createUI();
void refreshLVGL();
void project(float *v, float angleX, float angleY, float angleZ, int *x, int *y);
void drawCube();
void updateSensorLabels(float accelX, float accelY, float accelZ, float gyroX, float gyroY, float gyroZ);
void drawLineFast(int x0, int y0, int x1, int y1);
void clearCubeArea();

void refreshLVGL()
{
    lv_tick_inc(20);
    lv_timer_handler();
}

void createUI()
{
    lv_obj_clean(lv_screen_active());

    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);

    lbl_title = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_22, 0);
    lv_label_set_text(lbl_title, "LSM6DS3 IMU Cube Demo");
    lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 10);

    lbl_acc_x = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_acc_x, &lv_font_montserrat_18, 0);
    lv_label_set_text(lbl_acc_x, "ACC X: 0.0000");
    lv_obj_set_pos(lbl_acc_x, 40, 430);

    lbl_acc_y = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_acc_y, &lv_font_montserrat_18, 0);
    lv_label_set_text(lbl_acc_y, "ACC Y: 0.0000");
    lv_obj_set_pos(lbl_acc_y, 40, 455);

    lbl_acc_z = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_acc_z, &lv_font_montserrat_18, 0);
    lv_label_set_text(lbl_acc_z, "ACC Z: 0.0000");
    lv_obj_set_pos(lbl_acc_z, 40, 480);

    lbl_gyro_x = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_gyro_x, &lv_font_montserrat_18, 0);
    lv_label_set_text(lbl_gyro_x, "GYRO X: 0.0000");
    lv_obj_set_pos(lbl_gyro_x, 40, 505);

    lbl_gyro_y = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_gyro_y, &lv_font_montserrat_18, 0);
    lv_label_set_text(lbl_gyro_y, "GYRO Y: 0.0000");
    lv_obj_set_pos(lbl_gyro_y, 40, 530);

    lbl_gyro_z = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_gyro_z, &lv_font_montserrat_18, 0);
    lv_label_set_text(lbl_gyro_z, "GYRO Z: 0.0000");
    lv_obj_set_pos(lbl_gyro_z, 40, 555);

    lbl_error = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_error, &lv_font_montserrat_20, 0);
    lv_label_set_text(lbl_error, "");
    lv_obj_set_pos(lbl_error, 40, 300);
}

void setup()
{
    Serial.begin(115200);
    delay(200);

    inkplate.begin(LV_DISPLAY_RENDER_MODE_PARTIAL);
    inkplate.selectDisplayMode(INKPLATE_1BIT);
    inkplate.setRotation(0);

    createUI();

    inkplate.wakePeripheral(INKPLATE_ACCELEROMETER);

    if (inkplate.lsm6ds3.begin() != 0)
    {
        lv_label_set_text(lbl_error, "ERROR: can't init LSM6DS3!");
        refreshLVGL();
        inkplate.display();
        esp_deep_sleep_start();
    }
    

    refreshLVGL();
    clearCubeArea();
    drawCube();
    inkplate.display();
}

void loop()
{
    float accelX = inkplate.lsm6ds3.readRawAccelX();
    float accelY = inkplate.lsm6ds3.readRawAccelY();
    float accelZ = inkplate.lsm6ds3.readRawAccelZ();

    float gyroX = inkplate.lsm6ds3.readFloatGyroX();
    float gyroY = inkplate.lsm6ds3.readFloatGyroY();
    float gyroZ = inkplate.lsm6ds3.readFloatGyroZ();

    updateSensorLabels(accelX, accelY, accelZ, gyroX, gyroY, gyroZ);

    float roll = atan2(accelY, accelZ);
    float pitch = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ));

    angleX = 0.8f * previousAngleX + 0.2f * roll;
    angleY = 0.8f * previousAngleY + 0.2f * pitch;
    angleZ += gyroZ * 0.0015f;

    previousAngleX = angleX;
    previousAngleY = angleY;

    refreshLVGL();

    clearCubeArea();
    drawCube();

    if (numRefreshes >= NUM_PARTIAL_UPDATES_BEFORE_FULL_REFRESH)
    {
        inkplate.display();
        numRefreshes = 0;
    }
    else
    {
        inkplate.partialUpdate(false, true);
        numRefreshes++;
    }

    delay(40);
}

void updateSensorLabels(float accelX, float accelY, float accelZ, float gyroX, float gyroY, float gyroZ)
{
    char buf[48];

    snprintf(buf, sizeof(buf), "ACC X: %.4f", accelX);
    lv_label_set_text(lbl_acc_x, buf);

    snprintf(buf, sizeof(buf), "ACC Y: %.4f", accelY);
    lv_label_set_text(lbl_acc_y, buf);

    snprintf(buf, sizeof(buf), "ACC Z: %.4f", accelZ);
    lv_label_set_text(lbl_acc_z, buf);

    snprintf(buf, sizeof(buf), "GYRO X: %.4f", gyroX);
    lv_label_set_text(lbl_gyro_x, buf);

    snprintf(buf, sizeof(buf), "GYRO Y: %.4f", gyroY);
    lv_label_set_text(lbl_gyro_y, buf);

    snprintf(buf, sizeof(buf), "GYRO Z: %.4f", gyroZ);
    lv_label_set_text(lbl_gyro_z, buf);
}

void clearCubeArea()
{
    for (int y = CUBE_Y; y < CUBE_Y + CUBE_H; y++)
    {
        for (int x = CUBE_X; x < CUBE_X + CUBE_W; x++)
        {
            inkplate.drawPixel(x, y, WHITE);
        }
    }
}

void drawCube()
{
    for (int i = 0; i < 12; i++)
    {
        float *v1 = cube[edges[i][0]];
        float *v2 = cube[edges[i][1]];

        int x1, y1, x2, y2;

        project(v1, angleX, angleY, angleZ, &x1, &y1);
        project(v2, angleX, angleY, angleZ, &x2, &y2);

        drawLineFast(x1, y1, x2, y2);
    }
}

void drawLineFast(int x0, int y0, int x1, int y1)
{
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (true)
    {
        if (x0 >= 0 && x0 < SCREEN_W && y0 >= 0 && y0 < SCREEN_H)
        {
            inkplate.drawPixel(x0, y0, BLACK);
        }

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;

        if (e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void project(float *v, float angleX, float angleY, float angleZ, int *x, int *y)
{
    float xr = v[0];
    float yr = v[1] * cos(angleX) - v[2] * sin(angleX);
    float zr = v[1] * sin(angleX) + v[2] * cos(angleX);

    float xrr = xr * cos(angleY) + zr * sin(angleY);
    float yrr = yr;
    float zrr = -xr * sin(angleY) + zr * cos(angleY);

    float xrrr = xrr * cos(angleZ) - yrr * sin(angleZ);
    float yrrr = xrr * sin(angleZ) + yrr * cos(angleZ);
    float zrrr = zrr;

    float persp = 4.0f / (4.0f + zrrr);

    *x = (int)(xrrr * persp * CUBE_SCALE + CUBE_CENTER_X);
    *y = (int)(yrrr * persp * CUBE_SCALE + CUBE_CENTER_Y);
}