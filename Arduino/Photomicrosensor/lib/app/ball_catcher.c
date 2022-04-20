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
#define DATE 58

typedef enum
{
    SOLENOID_ACTIVATE,
    SOLENOID_DEACTIVATE,
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
uint8_t state       = IDLE;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void BallCatcher_MsgHandler(service_t *service, msg_t *msg);

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
    time_luos_t date = 0;
    if (Luos_IsNodeDetected())
    {
        switch (state)
        {
            case (SOLENOID_ACTIVATE):
            {

                date = Timestamp_now() + TimeOD_TimeFrom_ms(DATE);
                // date = Timestamp_now();
                if (solenoid.result_nbr > 0)
                {
                    msg_t msg;
                    msg.header.target      = solenoid.result_table[0]->id;
                    msg.header.target_mode = IDACK;
                    msg.header.cmd         = IO_STATE;
                    msg.header.size        = 1;
                    msg.data[0]            = 1;
                    while (Luos_SendTimestampMsg(app, &msg, date) != SUCCEED)
                        ;
                }
                state      = SOLENOID_DEACTIVATE;
                wait_timer = Luos_GetSystick();
                break;
            }
            case (SOLENOID_DEACTIVATE):
            {
                if (Luos_GetSystick() >= wait_timer + 60)
                {
                    date = Timestamp_now() + TimeOD_TimeFrom_ms(500);
                    if (solenoid.result_nbr > 0)
                    {
                        msg_t msg;
                        msg.header.target      = solenoid.result_table[0]->id;
                        msg.header.target_mode = IDACK;
                        msg.header.cmd         = IO_STATE;
                        msg.header.size        = 1;
                        msg.data[0]            = 0;
                        while (Luos_SendTimestampMsg(app, &msg, date) != SUCCEED)
                            ;
                    }
                    wait_timer = Luos_GetSystick();
                    state      = FAIL_CHECK;
                }
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
                break;
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
                break;
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

    if (msg->header.cmd == IO_STATE)
    {
        search_result_t result;
        RTFilter_ID(RTFilter_Reset(&result), msg->header.source);
        if (strstr(result.result_table[0]->alias, "sensor1") != 0)
        {
            if ((sensor_state1 == 0) && (msg->data[0] != sensor_state1))
            {
                wait_timer = Luos_GetSystick();
                state      = SOLENOID_ACTIVATE;
            }
            sensor_state1 = msg->data[0];
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
