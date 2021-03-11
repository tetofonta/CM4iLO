/* Define to prevent recursive  ----------------------------------------------*/
#ifndef __USBH_RPI_H
#define __USBH_RPI_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbh_core.h"

typedef enum {
    RPI_IDLE = 0U,
    RPI_SND_BOOTLOADER,
    RPI_WAIT_BOOTLOADER,
    RPI_FILE_SERVER,
    RPI_FILE_SERVER_SND,
    RPI_FILE_SERVER_WAIT
} RPI_DataStateTypeDef;

extern USBH_ClassTypeDef RPI_Class;
#define USBH_RPI_CLASS                                          &RPI_Class
#define USB_RPI_CLASS                                           0xFFU
#define BROADCOM_VENDOR_ID                                      0x0a5c
#define PI4_CM_SOC                                              0x2711

typedef struct RPI_USB_status{
    RPI_DataStateTypeDef status;
} RPI_USB_status;

#ifdef __cplusplus
}
#endif

#endif