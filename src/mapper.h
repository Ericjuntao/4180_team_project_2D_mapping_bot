#include "mbed.h"
#include "Motor.h"
#include "VL53L0X.h"
#include "HALLFX_ENCODER.h"

#define MOTOR_PWM_LEFT p21
#define MOTOR_PWM_RIGHT p22
#define MOTOR_FWD_LEFT p6
#define MOTOR_FWD_RIGHT p7
#define MOTOR_REV_LEFT p5
#define MOTOR_REV_RIGHT p8
#define ENCODER_LEFT_PIN p11
#define ENCODER_RIGHT_PIN p12
#define I2C_SDA_PIN p28
#define I2C_SCL_PIN p27
#define LIDAR_SHDN_CENTER p26
#define LIDAR_SHDN_LEFT p25
#define LIDAR_SHDN_RIGHT p24

typedef enum {
    CENTER = 0,
    LEFT = 1,
    RIGHT = 2
} LIDAR_DIRECTION;

typedef struct lidars {
    VL53L0X *center;
    VL53L0X *left;
    VL53L0X *right;
} lidars;

typedef struct point {
        int32_t x;
        int32_t y;
} Point;


class Mapper {
public:
    Mapper();
    ~Mapper();
    int drive(float speed);
    bool check_moved_distance(uint32_t dist);
    int move_forward(uint32_t dist);
    int plot_object(LIDAR_DIRECTION dir, Point &p);
    int read_dist(LIDAR_DIRECTION dir, uint32_t &dist);
    int32_t x = 0;  // X coordinate relative to start in millimiters
    int32_t y = 0;  // Y coordinate relative to start in millimiters
    float theta = M_PI / 2;  // Orientation of robot in radians
private:
    uint32_t old_L = 0;
    uint32_t old_R = 0;
    Motor _wheel_l;
    Motor _wheel_r;
    HALLFX_ENCODER _encoder_left;
    HALLFX_ENCODER _encoder_right;
    DevI2C _i2c;
    DigitalOut _lidar_shdn_center;
    DigitalOut _lidar_shdn_left;
    DigitalOut _lidar_shdn_right;
    lidars _lidars;
    void _init_lidar();
    uint32_t _map_thresh_mm = 500;  // Objects must be this close to be mapped
};
