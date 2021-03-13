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

    EP_WRITE_INIT,
    EP_WRITE,
    EP_READ_INIT,
    EP_READ,

    RPI_SND_BOOTLOADER_HEADER,
    RPI_SND_BOOTLOADER_DATA,
    RPI_SND_BOOTLOADER_RESPONSE,

    RPI_FILE_SERVER
} RPI_DataStateTypeDef;

extern USBH_ClassTypeDef RPI_Class;
#define USBH_RPI_CLASS                                          &RPI_Class
#define USB_RPI_CLASS                                           0xFFU
#define BROADCOM_VENDOR_ID                                      0x0a5c
#define PI4_CM_SOC                                              0x2711

typedef struct RPI_USB_status{
    RPI_DataStateTypeDef status;
    uint8_t interface;
    uint8_t in_ep, in_pipe;
    uint16_t in_size;
    uint8_t out_ep, out_pipe;
    uint16_t out_size;

    size_t sending_await;
    void * send_buffer;
    RPI_DataStateTypeDef next;

    size_t receiving_len;
    void * recv_buffer;
} RPI_USB_status;

typedef struct MESSAGE_S {
    int length;
    unsigned char signature[20];
} boot_message_t;

#ifdef __cplusplus
}
#endif

#endif