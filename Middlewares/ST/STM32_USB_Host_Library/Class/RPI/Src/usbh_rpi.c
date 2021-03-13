#include <bootcode4.h>
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

    RPI_USB_status * rpi;
    rpi = (RPI_USB_status *) malloc(sizeof(RPI_USB_status));
    if (rpi == NULL) Error_Handler();
    phost->pActiveClass->pData = rpi;
    rpi->sending_await = 0;
    rpi->send_buffer = NULL;
    rpi->receiving_len = 0;
    rpi->recv_buffer = NULL;

    if(phost->device.CfgDesc.bNumInterfaces == 1){
        rpi->interface = 0;
        rpi->in_ep = 2;
        rpi->out_ep = 1;
    } else {
        rpi->interface = 1;
        rpi->in_ep = 4;
        rpi->out_ep = 3;
    }
    if (USBH_SelectInterface(phost, rpi->interface) != USBH_OK) Error_Handler();
    rpi->in_pipe = USBH_AllocPipe(phost, rpi->in_ep);
    rpi->in_size = phost->device.CfgDesc.Itf_Desc[rpi->interface].Ep_Desc[rpi->in_ep].wMaxPacketSize;
    rpi->out_pipe = USBH_AllocPipe(phost, rpi->out_ep);
    rpi->out_size = phost->device.CfgDesc.Itf_Desc[rpi->interface].Ep_Desc[rpi->out_ep].wMaxPacketSize;

    USBH_OpenPipe(phost, rpi->out_pipe, rpi->out_ep, phost->device.address, phost->device.speed, USB_EP_TYPE_BULK, rpi->out_size);
    USBH_OpenPipe(phost, rpi->in_pipe, rpi->in_ep, phost->device.address, phost->device.speed, USB_EP_TYPE_BULK, rpi->in_size);

    return USBH_OK;
}

static USBH_StatusTypeDef USBH_RPI_InterfaceDeInit(USBH_HandleTypeDef *phost) {
    USBH_free(phost->pActiveClass->pData);
    return USBH_OK;
}

static USBH_StatusTypeDef USBH_RPI_ClassRequest(USBH_HandleTypeDef *phost) {
    if (phost->device.DevDesc.iSerialNumber == 3 || phost->device.DevDesc.iSerialNumber == 0)
        ((RPI_USB_status *) phost->pActiveClass->pData)->status = RPI_SND_BOOTLOADER_HEADER;
    else
        ((RPI_USB_status *) phost->pActiveClass->pData)->status = RPI_FILE_SERVER;
    return USBH_OK;
}

static void rpi_handle_ep_read(USBH_HandleTypeDef * phost){
    RPI_USB_status * rpi = phost->pActiveClass->pData;
    switch (rpi->status) {
        case EP_READ_INIT:
            //send ctrl and start bulk
            phost->Control.setup.b.bmRequestType = USB_REQ_TYPE_VENDOR | USB_D2H;
            phost->Control.setup.b.bRequest = 0;
            phost->Control.setup.b.wValue.w = rpi->sending_await & 0xffff;
            phost->Control.setup.b.wIndex.w = rpi->sending_await >> 16;
            phost->Control.setup.b.wLength.w = rpi->receiving_len;
            rpi->status = EP_READ;

            if(rpi->recv_buffer != NULL) free(rpi->recv_buffer);
            rpi->recv_buffer = malloc(rpi->receiving_len);

            break;
        case EP_READ: {
            if (USBH_CtlReq(phost, rpi->recv_buffer, rpi->receiving_len) == USBH_OK)
                rpi->status = rpi->next;
            break;
        }
        default:
            rpi->status = EP_READ_INIT;
    }
}

static void rpi_handle_ep_write(USBH_HandleTypeDef * phost){
    RPI_USB_status * rpi = phost->pActiveClass->pData;
    switch (rpi->status){
        case EP_WRITE_INIT:
            //send ctrl and start bulk
            phost->Control.setup.b.bmRequestType = USB_REQ_TYPE_VENDOR;
            phost->Control.setup.b.bRequest = 0;
            phost->Control.setup.b.wValue.w = rpi->sending_await & 0xffff;
            phost->Control.setup.b.wIndex.w = rpi->sending_await >> 16;
            phost->Control.setup.b.wLength.w = 0;
            rpi->status = EP_WRITE;
            break;
        case EP_WRITE: {
            int x = 0;
            if(USBH_CtlReq(phost, NULL, 0) != USBH_OK) break;
            while (rpi->sending_await > 0){
                size_t snd = rpi->sending_await < 0x140 ? rpi->sending_await : 0x140;
                USBH_Delay(5);
                if(USBH_BulkSendData(phost, rpi->send_buffer, snd, rpi->out_pipe, 0) != USBH_OK)
                    error_handler_code(((++x>>4))%256);
                write_code(((++x>>4))%256);
                rpi->sending_await -= snd;
                rpi->send_buffer += snd;
            }
            rpi->status = rpi->next;
            break;
        }
        default:
            rpi->status = EP_WRITE_INIT;
    }
}

static USBH_StatusTypeDef USBH_RPI_Process(USBH_HandleTypeDef *phost) {
    RPI_USB_status * rpi = phost->pActiveClass->pData;
    switch (rpi->status) {
        case RPI_IDLE:
            break;

        case EP_READ_INIT:
        case EP_READ:
            rpi_handle_ep_read(phost);
            break;
        case EP_WRITE_INIT:
        case EP_WRITE:
            rpi_handle_ep_write(phost);
            break;

        case RPI_SND_BOOTLOADER_HEADER: {
            write_code(0x41);
            size_t bootloader_size = msd_bootcode4_bin_len;
            boot_message_t boot_msg = {.length = bootloader_size, /*.signature = get signature*/};
            rpi->sending_await = sizeof(boot_message_t);
            rpi->send_buffer = &boot_msg;
            rpi->next = RPI_SND_BOOTLOADER_DATA;
            rpi_handle_ep_write(phost);
            break;
        }
        case RPI_SND_BOOTLOADER_DATA: {
            write_code(0x42);
            rpi->sending_await = msd_bootcode4_bin_len;
            rpi->send_buffer = msd_bootcode4_bin;
            rpi->next = RPI_SND_BOOTLOADER_RESPONSE;
            rpi_handle_ep_write(phost);
            break;
        }
        case RPI_SND_BOOTLOADER_RESPONSE:{
            write_code(0x43);
            rpi->next = RPI_FILE_SERVER;
            rpi->receiving_len = 4;
            rpi_handle_ep_read(phost);
            break;
        }
        case RPI_FILE_SERVER:{
            write_code(0x44);
            if(*((int *) rpi->recv_buffer) != 0) Error_Handler();
            error_handler_code(0);
            USBH_Delay(10000);
            break;
        }
        default:
            Error_Handler();
    }
    return USBH_OK;
}

static USBH_StatusTypeDef USBH_RPI_SOFProcess(USBH_HandleTypeDef *phost) {
    return USBH_OK;
}