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
    uint8_t interface;
    uint8_t in_ep, in_pipe, in_size;
    uint8_t out_ep, out_pipe, out_size;
    uint8_t ctrl_pipe;
} RPI_USB_status;

typedef struct MESSAGE_S {
    int length;
    unsigned char signature[20];
} boot_message_t;

typedef struct {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
}usb_setup_packet __attribute__((packed));


#ifdef __cplusplus
}
#endif

#endif