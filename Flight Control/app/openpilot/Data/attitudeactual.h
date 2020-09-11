#ifndef ATTITUDEACTUAL_H
#define ATTITUDEACTUAL_H
#include "common.h"
struct AttitudeActualData
{
    float q1;
    float q2;
    float q3;
    float q4;
    float Roll;
    float Pitch;
    float Yaw;
};
typedef struct AttitudeActualData AttitudeActualData;
#endif // ATTITUDEACTUAL_H
