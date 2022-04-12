/******************************************************************************
 * @file app
 * @brief app that controls photosensor and solenoid
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <Arduino.h>
#include "ball_catcher.h"
#include "timestamp.h"

/*******************************************************************************
 * Definitions
 *****************************************************************************/
#define DIST   0.20
#define HEIGHT 0.55

typedef enum
{
    SOLENOID_ACTIVATE,
    GOAL,
    FAIL_CHECK,
    WAITING,
    IDLE
} state_t;
/*******************************************************************************
 * Variables
 ******************************************************************************/
service_t *app;
search_result_t solenoid;
search_result_t sensor;
search_result_t lcd;

uint64_t wait_timer = 0;
// uint64_t velocity   = 0;
uint8_t state = IDLE;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void BallCatcher_MsgHandler(service_t *service, msg_t *msg);

/* Not yet in use
uint64_t velocity_calc(uint64_t dist, uint64_t time);
int64_t delay_calc(uint64_t dist, uint64_t vel);
*/
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void BallCatcher_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    app                 = Luos_CreateService(BallCatcher_MsgHandler, LUOS_LAST_TYPE, "app", revision);
    // delay to setup all the systems components
    Luos_Detect(app);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void BallCatcher_Loop(void)
{
    if (Luos_IsNodeDetected())
    {
        switch (state)
        {
            case (SOLENOID_ACTIVATE):
            {
                // uint64_t date = delay_calc(HEIGHT, velocity);
                uint64_t date    = 50 * 1000000;
                uint8_t power_on = 1;
                timestamp_token_t token1;

                Timestamp_CreateEvent(date, &token1, &power_on);
                if (solenoid.result_nbr > 0)
                {
                    msg_t msg;
                    msg.header.target      = solenoid.result_table[0]->id;
                    msg.header.target_mode = IDACK;
                    msg.header.cmd         = IO_STATE;
                    msg.header.size        = 1;
                    msg.data[0]            = 1;
                    if (Timestamp_GetToken(&power_on))
                    {
                        Timestamp_EncodeMsg(&msg, &power_on);
                    }
                    Luos_SendTimestampMsg(app, &msg);
                }

                date     = 550 * 1000000;
                power_on = 0;
                timestamp_token_t token2;

                Timestamp_CreateEvent(date, &token2, &power_on);
                if (solenoid.result_nbr > 0)
                {
                    msg_t msg;
                    msg.header.target      = solenoid.result_table[0]->id;
                    msg.header.target_mode = IDACK;
                    msg.header.cmd         = IO_STATE;
                    msg.header.size        = 1;
                    msg.data[0]            = 0;
                    if (Timestamp_GetToken(&power_on))
                    {
                        Timestamp_EncodeMsg(&msg, &power_on);
                    }
                    Luos_SendTimestampMsg(app, &msg);
                }
                wait_timer = Luos_GetSystick();
                state      = FAIL_CHECK;
                break;
            }
            case (GOAL):
            {
                if (lcd.result_nbr > 0)
                {
                    msg_t msg;
                    msg.header.target      = lcd.result_table[0]->id;
                    msg.header.target_mode = IDACK;
                    msg.header.cmd         = TEXT;
                    msg.header.size        = sizeof("GOAL!!!\0");
                    memcpy(&msg.data[0], "GOAL!!!\0", sizeof("GOAL!!!\0"));
                    Luos_SendMsg(app, &msg);
                }
                wait_timer = Luos_GetSystick();
                state      = WAITING;
                break;
            }
            case (FAIL_CHECK):
            {
                if (Luos_GetSystick() > wait_timer + 3000)
                {
                    if (lcd.result_nbr > 0)
                    {
                        msg_t msg;
                        msg.header.target      = lcd.result_table[0]->id;
                        msg.header.target_mode = IDACK;
                        msg.header.cmd         = TEXT;
                        msg.header.size        = sizeof("FAILED!\0");
                        memcpy(&msg.data[0], "FAILED!\0", sizeof("FAILED!\0"));
                        Luos_SendMsg(app, &msg);
                        wait_timer = Luos_GetSystick();
                        state      = WAITING;
                    }
                }
            }
            case (WAITING):
            {

                if (Luos_GetSystick() > wait_timer + 2000)
                {
                    if (lcd.result_nbr > 0)
                    {
                        msg_t msg;
                        msg.header.target      = lcd.result_table[0]->id;
                        msg.header.target_mode = IDACK;
                        msg.header.cmd         = TEXT;
                        msg.header.size        = sizeof("WAITING...\0");
                        memcpy(&msg.data[0], "WAITING...\0", sizeof("WAITING...\0"));
                        Luos_SendMsg(app, &msg);
                    }
                    state = IDLE;
                }
            }
            default:
                break;
        }
    }
    else
    {
        state = IDLE;
    }
}
/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void BallCatcher_MsgHandler(service_t *service, msg_t *msg)
{
    static uint8_t sensor_state1 = 0;
    static uint8_t sensor_state2 = 0;
    static uint8_t sensor_state3 = 0;
    static int64_t timestamp1    = 0;
    static int64_t timestamp2    = 0;

    if (msg->header.cmd == IO_STATE)
    {
        search_result_t result;
        RTFilter_ID(RTFilter_Reset(&result), msg->header.source);
        if (strstr(result.result_table[0]->alias, "sensor1") != 0)
        {
            if ((sensor_state1 == 0) && (msg->data[0] != sensor_state1))
            {
                if (Timestamp_IsTimestampMsg(msg) == true)
                {
                    Timestamp_DecodeMsg(msg, &timestamp1);
                }
            }
            sensor_state1 = msg->data[0];
        }
        else if (strstr(result.result_table[0]->alias, "sensor3") != 0)
        {
            if ((sensor_state3 == 0) && (msg->data[0] != sensor_state3))
            {
                if (Timestamp_IsTimestampMsg(msg) == true)
                {
                    Timestamp_DecodeMsg(msg, &timestamp2);
                }
                // uint64_t delay = timestamp2 - timestamp1;
                // velocity       = velocity_calc(DIST, delay);
                state = SOLENOID_ACTIVATE;
            }
            sensor_state3 = msg->data[0];
        }
        else
        {
            if ((sensor_state2 == 0) && (msg->data[0] != sensor_state2))
            {
                state = GOAL;
            }
            sensor_state2 = msg->data[0];
        }
    }
    else if (msg->header.cmd == END_DETECTION)
    {
        RTFilter_Alias(RTFilter_Reset(&sensor), "sensor");
        for (uint8_t i = 0; i < sensor.result_nbr; i++)
        {
            msg_t pub_msg;
            pub_msg.header.target = sensor.result_table[i]->id;
            time_luos_t time      = TimeOD_TimeFrom_s(0.002f);
            TimeOD_TimeToMsg(&time, &pub_msg);
            pub_msg.header.cmd = UPDATE_PUB;
            while (Luos_SendMsg(service, &pub_msg) != SUCCEED)
                ;
        }
        RTFilter_Alias(RTFilter_Reset(&solenoid), "solenoid");
        RTFilter_Alias(RTFilter_Reset(&lcd), "lcd");
        if (lcd.result_nbr > 0)
        {
            msg_t pub_msg;
            pub_msg.header.target      = lcd.result_table[0]->id;
            pub_msg.header.target_mode = ID;
            pub_msg.header.size        = 0;
            pub_msg.header.cmd         = REINIT;
            while (Luos_SendMsg(app, &pub_msg) != SUCCEED)
                ;
            msg_t new_msg;
            new_msg.header.target      = lcd.result_table[0]->id;
            new_msg.header.target_mode = ID;
            new_msg.header.cmd         = TEXT;
            new_msg.header.size        = sizeof("WAITING...\0");
            memcpy(&new_msg.data[0], "WAITING...\0", sizeof("WAITING...\0"));
            Luos_SendMsg(app, &new_msg);
        }
    }
}
/* NOT YET IN USE
uint64_t velocity_calc(uint64_t dist, uint64_t time)
{
    return ((float)((dist - 10 * (time ^ 3)) / (time + (float)((time ^ 2) / 2))) + 10 * time);
}

int64_t delay_calc(uint64_t dist, uint64_t vel)
{
    for (uint64_t i = 1; i < 100000; i++)
    {
        if (((10 * (i ^ 3) + (velocity * (i ^ 2)) + velocity * i - dist) > 0) && ((10 * (i ^ 3) + (velocity * (i ^ 2)) + velocity * i - dist) < 1))
        {
            return i;
        }
    }
    return 0;
}
*/