#include "mapper.h"
#include "VL53L0X.h"
#include "mbed.h"
#include "HALLFX_ENCODER.h"


// Serial pc(USBTX, USBRX);

Mapper::Mapper():
_wheel_l(MOTOR_PWM_LEFT, MOTOR_FWD_LEFT, MOTOR_REV_LEFT),
_wheel_r(MOTOR_PWM_RIGHT, MOTOR_FWD_RIGHT, MOTOR_REV_RIGHT),
_encoder_left(ENCODER_LEFT_PIN),
_encoder_right(ENCODER_RIGHT_PIN),
_i2c(I2C_SDA_PIN, I2C_SCL_PIN),
_lidar_shdn_center(LIDAR_SHDN_CENTER),
_lidar_shdn_left(LIDAR_SHDN_LEFT),
_lidar_shdn_right(LIDAR_SHDN_RIGHT)
{
    _init_lidar();
}

Mapper::~Mapper() {
    delete _lidars.center;
    delete _lidars.left;
    delete _lidars.right;
}

int Mapper::drive(float speed) {
    _wheel_l.speed(speed);
    _wheel_r.speed(speed);
    return 0;
}

bool Mapper::check_moved_distance(uint32_t dist) {
    int lr = _encoder_left.read();
    int rr = _encoder_right.read();
    float ld = 0.5672320068 * (lr); //unit in mm
    float rd = 0.5672320068 * (rr);
    float total_d = (ld + rd) / 2;
    x = 0;
    y = total_d;
    return y >= dist - 45;
}


int Mapper::move_forward(uint32_t dist) {
    //pc.printf("total_d: %f\n\r",total_d);
    _encoder_left.reset();
    _encoder_right.reset();
    _wheel_l.speed(0.3);
    _wheel_r.speed(0.3);
    while (!check_moved_distance(dist));
    _wheel_l.speed(0);
    _wheel_r.speed(0);
    return 0;
}


// Returns 0 on map of object
// Returns -1 on fail to map an object
int Mapper::plot_object(LIDAR_DIRECTION dir, Point &p) {
    uint32_t dist;
    int status = read_dist(dir, dist);
    if (status == VL53L0X_ERROR_NONE && dist <= _map_thresh_mm) {
        float l_theta = 0;
        switch (dir) {
            case CENTER:
                l_theta = theta;
                break;
            case LEFT:
                l_theta = theta + M_PI / 2;
                break;
            case RIGHT:
                l_theta = theta - M_PI / 2;
                break;
            default:
                error("INVALID LIDAR DIRECTION\r\n");
        };
        p.x = x + cos(l_theta) * dist;
        p.y = y + sin(l_theta) * dist;
        return 0;
    }
    return -1;
}

int Mapper::read_dist(LIDAR_DIRECTION dir, uint32_t &d) {
    uint32_t distance;
    int status = -1;
    switch (dir) {
        case CENTER:
            status = _lidars.center->get_distance(&distance);
            break;
        case LEFT:
            status = _lidars.left->get_distance(&distance);
            break;
        case RIGHT:
            status = _lidars.right->get_distance(&distance);
            break;
        default:
            error("INVALID LIDAR DIRECTION\r\n");
    };
    if (status == VL53L0X_ERROR_NONE) {
        d = distance;
    } else if (status != VL53L0X_ERROR_RANGE_ERROR) {
        error("LIDAR DIRECTION %d: ERROR %d", dir, status);
    }
    return status;
}

// Create the lidar objects, initialize boards, assign the boards unique I2C addresses
void Mapper::_init_lidar() {
    _lidars.center = new VL53L0X(_i2c, _lidar_shdn_center, p15);
    _lidars.left = new VL53L0X(_i2c, _lidar_shdn_left, p16);
    _lidars.right  = new VL53L0X(_i2c, _lidar_shdn_right, p17);
    int status;
    status = _lidars.center->init_sensor(0x01);
    if (status != 0) {
        // error("FAILED TO INITIALIZE CENTER LIDAR\r\n");
    }
    status = _lidars.left->init_sensor(0x03);
    if (status != 0) {
        // error("FAILED TO INITIALIZE LEFT LIDAR\r\n");
    }
    status = _lidars.right->init_sensor(0x05);
    if (status != 0) {
        // error("FAILED TO INITIALIZE RIGHT LIDAR\r\n");
    }
}
