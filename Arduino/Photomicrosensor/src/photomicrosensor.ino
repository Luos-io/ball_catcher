#include <Arduino.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include <luos_engine.h>
#include "photomicrosensor.h"
#include "ball_catcher.h"
#ifdef __cplusplus
}
#endif

/******************************************************************************
 * @brief Setup ardiuno
 * @param None
 * @return None
 ******************************************************************************/
void setup()
{
    Luos_Init();
    PhotoSensor_Init();
    BallCatcher_Init();
}
/******************************************************************************
 * @brief Loop Arduino
 * @param None
 * @return None
 ******************************************************************************/
void loop()
{
    Luos_Loop();
    PhotoSensor_Loop();
    BallCatcher_Loop();
}
