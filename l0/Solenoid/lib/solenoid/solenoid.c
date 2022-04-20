/******************************************************************************
 * @file solenoid
 * @brief driver example a solenoid
 * @author Luos
 * @version 0.0.0
 ******************************************************************************/
#include "solenoid.h"
#include "timestamp.h"
#include "solenoid_drv.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t val       = 0;
time_luos_t timer = 0;
service_t *solenoid;
/*******************************************************************************
 * Function
 ******************************************************************************/
static void Solenoid_MsgHandler(service_t *service, msg_t *msg);
/******************************************************************************
 * @brief init must be call in project init
 * @param None
 * @return None
 ******************************************************************************/
void Solenoid_Init(void)
{
    revision_t revision = {.major = 1, .minor = 0, .build = 0};
    solenoid_drv_init();
    solenoid = Luos_CreateService(Solenoid_MsgHandler, LUOS_LAST_TYPE, "solenoid", revision);
}
/******************************************************************************
 * @brief loop must be call in project loop
 * @param None
 * @return None
 ******************************************************************************/
void Solenoid_Loop(void)
{
    if (Luos_IsNodeDetected())
    {
        if (Timestamp_now() >= timer)
        {
            solenoid_drv_write((bool)val);
        }
    }
}
/******************************************************************************
 * @brief Msg Handler call back when a msg receive for this service
 * @param Service destination
 * @param Msg receive
 * @return None
 ******************************************************************************/
static void Solenoid_MsgHandler(service_t *service, msg_t *msg)
{
    if (Timestamp_IsTimestampMsg(msg) == true)
    {
        timer = Timestamp_GetTimestamp(msg);
    }
    else
    {
        timer = 0;
    }
    if (msg->header.cmd == IO_STATE)
    {
        val = msg->data[0];
    }
}
