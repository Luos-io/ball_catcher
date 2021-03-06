/******************************************************************************
 * @file photomicrosensor
 * @brief driver example a photomicrosensor
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include <Arduino.h>
#include "photomicrosensor.h"
#include "timestamp.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PHOTOIN_PIN1 8
#define PHOTOIN_PIN2 9
/*******************************************************************************
 * Variables
 ******************************************************************************/
/*******************************************************************************
 * Function
 ******************************************************************************/
static void PhotoSensor_MsgHandler1(service_t *service, msg_t *msg);
static void PhotoSensor_MsgHandler2(service_t *service, msg_t *msg);
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void PhotoSensor_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    pinMode(PHOTOIN_PIN1, INPUT_PULLUP);
    pinMode(PHOTOIN_PIN2, INPUT_PULLUP);
    Luos_CreateService(PhotoSensor_MsgHandler1, STATE_TYPE, "photo_sensor1", revision);
    Luos_CreateService(PhotoSensor_MsgHandler2, STATE_TYPE, "photo_sensor2", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void PhotoSensor_Loop(void)
{
    if (!Luos_IsNodeDetected())
    {
        pinMode(PHOTOIN_PIN1, INPUT_PULLUP);
        pinMode(PHOTOIN_PIN2, INPUT_PULLUP);
    }
}
/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void PhotoSensor_MsgHandler1(service_t *service, msg_t *msg)
{
    time_luos_t timestamp = 0;
    if (msg->header.cmd == GET_CMD)
    {
        uint8_t value = (bool)digitalRead(PHOTOIN_PIN1);

        timestamp = Timestamp_now();
        //  fill the message infos
        msg_t pub_msg;
        pub_msg.header.cmd         = IO_STATE;
        pub_msg.header.target_mode = ID;
        pub_msg.header.target      = msg->header.source;
        pub_msg.header.size        = sizeof(uint8_t);
        pub_msg.data[0]            = value;
        Luos_SendTimestampMsg(service, &pub_msg, timestamp);
        return;
    }
}
/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void PhotoSensor_MsgHandler2(service_t *service, msg_t *msg)
{
    if (msg->header.cmd == GET_CMD)
    {
        // fill the message infos
        msg_t pub_msg;
        pub_msg.header.cmd         = IO_STATE;
        pub_msg.header.target_mode = ID;
        pub_msg.header.target      = msg->header.source;
        pub_msg.header.size        = sizeof(uint8_t);
        pub_msg.data[0]            = (bool)digitalRead(PHOTOIN_PIN2);
        Luos_SendMsg(service, &pub_msg);
        return;
    }
}
