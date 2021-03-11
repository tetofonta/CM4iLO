#include "../Inc/usbh_rpi.h"

static USBH_StatusTypeDef USBH_RPI_InterfaceInit(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_RPI_InterfaceDeInit(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_RPI_Process(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_RPI_SOFProcess(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_RPI_ClassRequest(USBH_HandleTypeDef *phost);

USBH_ClassTypeDef RPI_Class =
        {
                "RPI",
                USB_RPI_CLASS,
                USBH_RPI_InterfaceInit,
                USBH_RPI_InterfaceDeInit,
                USBH_RPI_ClassRequest,
                USBH_RPI_Process,
                USBH_RPI_SOFProcess,
                NULL,
        };

static USBH_StatusTypeDef USBH_RPI_InterfaceInit(USBH_HandleTypeDef *phost) {
    if (phost->device.DevDesc.idVendor != BROADCOM_VENDOR_ID) Error_Handler();

    uint8_t interface = USBH_FindInterface(phost, USB_RPI_CLASS, 0, 0);

    if ((interface == 0xFFU) || (interface >= USBH_MAX_NUM_INTERFACES)) Error_Handler();
    if (USBH_SelectInterface(phost, interface) != USBH_OK) Error_Handler();

    (phost->pActiveClass->pData) = (RPI_USB_status *) malloc(sizeof(RPI_USB_status));
    if(phost->pActiveClass->pData == NULL) Error_Handler();

    ((RPI_USB_status *)phost->pActiveClass->pData)->status = RPI_IDLE;
    return USBH_OK;
}

static USBH_StatusTypeDef USBH_RPI_InterfaceDeInit(USBH_HandleTypeDef *phost) {
    USBH_free(phost->pActiveClass->pData);
    return USBH_OK;
}

static USBH_StatusTypeDef USBH_RPI_ClassRequest(USBH_HandleTypeDef *phost) {
    if (phost->device.DevDesc.iSerialNumber == 3 || phost->device.DevDesc.iSerialNumber == 0)
        ((RPI_USB_status *) phost->pActiveClass->pData)->status = RPI_SND_BOOTLOADER;
    else
        ((RPI_USB_status *) phost->pActiveClass->pData)->status = RPI_FILE_SERVER;
    return USBH_OK;
}

static USBH_StatusTypeDef USBH_RPI_Process(USBH_HandleTypeDef *phost) {
    RPI_USB_status * s = phost->pActiveClass->pData;
    switch (s->status) {
        case RPI_IDLE:
            break;
        case RPI_SND_BOOTLOADER:
            write_code(0x41);
            break;
        default:
            Error_Handler();
    }
    return USBH_OK;
}

static USBH_StatusTypeDef USBH_RPI_SOFProcess(USBH_HandleTypeDef *phost) {
    return USBH_OK;
}