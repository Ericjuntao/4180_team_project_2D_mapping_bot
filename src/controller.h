#include "mbed.h"
#include "mapper.h"
#include <climits>


extern Serial pc;
Mapper robot;
BusOut leds(LED1, LED2, LED3, LED4);


volatile int d_center = INT_MAX;
volatile int d_left = INT_MAX;
volatile int d_right = INT_MAX;
void plot_surrounding() {
    Point measured_point = {0, 0};
    if (!robot.plot_object(CENTER, measured_point)) {
        pc.printf("%d, %d\r\n", measured_point.x, measured_point.y);
        d_center = sqrt(pow(measured_point.x - robot.state.x, 2) + pow(measured_point.y - robot.state.y, 2));
    } else d_center = INT_MAX;
    if (!robot.plot_object(LEFT, measured_point)) {
        pc.printf("%d, %d\r\n", measured_point.x, measured_point.y);
        d_left = sqrt(pow(measured_point.x - robot.state.x, 2) + pow(measured_point.y - robot.state.y, 2));
    } else d_left = INT_MAX;
    if (!robot.plot_object(RIGHT, measured_point)) {
        pc.printf("%d, %d\r\n", measured_point.x, measured_point.y);
        d_right = sqrt(pow(measured_point.x - robot.state.x, 2) + pow(measured_point.y - robot.state.y, 2));
    } else d_right = INT_MAX;

    // for (auto dir : dirs) {
    //     if (!robot.plot_object(dir, measured_point)) {
    //         pc.printf("%d, %d\r\n", measured_point.x, measured_point.y);
    //     }
    // }
    //
}

void print_state() {
    pc.printf("X: %d, Y: %d, THETA: %.f, TARGET THETA: %.f\r\n", robot.state.x, robot.state.y, robot.state.theta * 180 / M_PI, robot.target_theta * 180 / M_PI);
    pc.printf("LV: %d, RV: %d\r\n", robot.state.lv, robot.state.rv);
    pc.printf("PWML: %f, PWMR: %f\r\n", robot._pwm_l, robot._pwm_r);
    pc.printf("VOFF: %d, PWMADDL: %.2f, PWMADDR: %.2f\r\n\r\n", robot._v_off, robot._pwm_add_l, robot._pwm_add_r);
}

void print_cal() {
    std::map<float, int32_t>::iterator itr;
    pc.printf("Left samples:\r\n");
    for (itr = robot._pwm_speed_map_l.begin(); itr != robot._pwm_speed_map_l.end(); ++itr) {
        pc.printf("robot._pwm_speed_map_l.insert(std::pair<float, int32_t>(%.1f, %d));\r\n", itr->first, itr->second);
    }
    pc.printf("Right samples:\r\n");
    for (itr = robot._pwm_speed_map_r.begin(); itr != robot._pwm_speed_map_r.end(); ++itr) {
        pc.printf("robot._pwm_speed_map_r.insert(std::pair<float, int32_t>(%.1f, %d));\r\n", itr->first, itr->second);
    }
    pc.printf("\r\nLEFT:\r\n");
    pc.printf("\t%f (mm/s) / V\r\n", robot._pwm_speed_m_l);
    pc.printf("\t%f mm/s at 0 V\r\n", robot._pwm_speed_b_l);
    pc.printf("RIGHT:\r\n");
    pc.printf("\t%f (mm/s) / v\r\n", robot._pwm_speed_m_r);
    pc.printf("\t%f mm/s at 0 V\r\n\r\n", robot._pwm_speed_b_r);
}

void turn_left() {
    if (robot.target_speed > 0) robot.target_speed = 50;
    robot.target_theta = bound_theta(robot.target_theta + M_PI / 2);
}

void turn_right() {
    if (robot.target_speed > 0) robot.target_speed = 50;
    robot.target_theta = bound_theta(robot.target_theta - M_PI / 2);
}

void forward() {
    robot.target_speed = 350;
}

void stop() {
    robot.target_speed = 0;
}

void dispatch() {
    while (pc.readable()) {
        char IN = pc.getc();
        switch (IN) {
            case 13:  // enter key
            case 'p':
                print_state();
                break;
            case 24:  // up arrow
                forward();
            case 25:  // back arrow
                stop();
            case 26:  // right arrow
                turn_left();
                break;
            case 27:  // left arrow
                turn_right();
                break;
            case 'c':
                robot.control = false;
                robot.calibrate_wheel_speed();
                print_cal();
                break;
            default:
                break;
        }
    }
}


int main() {
    robot.start_state_update(0.05);
    robot.control = true;
    pc.printf("reset\r\n");
    pc.attach(Callback<void()>(dispatch));
    while (1) {
        // print_state();
        // plot_surrounding();
        wait(1);
    }
}