/* 8c0207d4 */
#include "0207d4.h"

float FUN_8c0207d4(Point3f *param1, Point3f *param2, Point3f *param3)
{
    float a = param2->x_0x00 - param1->x_0x00;
    float b = param3->x_0x00 - param1->x_0x00;
    float c = param2->z_0x08 - param1->z_0x08;
    float d = param3->y_0x04 - param1->z_0x08;

    return c * d + b * a;
}

float FUN_8c0207fa(Point3f *param1, Point3f *param2, Point3f *param3)
{
    float a = param2->x_0x00 - param1->x_0x00;
    float b = param3->y_0x04 - param1->z_0x08;
    float c = param2->y_0x04 - param1->z_0x08;
    c *= param3->x_0x00 - param1->x_0x00;

    return a * b - c;
}
