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
    rpi->ctrl_pipe = USBH_AllocPipe(phost, 0);

    USBH_OpenPipe(phost, rpi->out_pipe, rpi->out_ep, phost->device.address, phost->device.speed, USB_EP_TYPE_BULK, rpi->out_size);
    USBH_OpenPipe(phost, rpi->in_pipe, rpi->in_ep, phost->device.address, phost->device.speed, USB_EP_TYPE_BULK, rpi->in_size);
    USBH_OpenPipe(phost, rpi->ctrl_pipe, 0, phost->device.address, phost->device.speed, USB_EP_TYPE_CTRL, phost->device.CfgDesc.Itf_Desc[rpi->interface].Ep_Desc[0].wMaxPacketSize);

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

static void ep_write(USBH_HandleTypeDef *phost, void * buffer, size_t size){
    //eheh
    RPI_USB_status * rpi = phost->pActiveClass->pData;
    usb_setup_packet data = {.bRequest=0, .bmRequestType=(0x02<<5), .wIndex=(size >> 16), .wLength=0, .wValue=size&0xffff};
    if(USBH_CtlSendSetup(phost, (uint8_t *) (&data), rpi->ctrl_pipe) != USBH_OK) Error_Handler(); //is it on endpoint 0?
    USBH_Delay(500);

    while(size > 0){
        size_t snd = size < rpi->out_size ? size : rpi->out_size;
        if(USBH_BulkSendData(phost, buffer, snd, rpi->out_ep, 0) != USBH_OK) Error_Handler();
        size -= snd;
        buffer += snd;
    }
}

static void ep_read(USBH_HandleTypeDef *phost, uint8_t * buffer, size_t size){
    //eheh
    RPI_USB_status * rpi = phost->pActiveClass->pData;
    usb_setup_packet data = {.bRequest=0, .bmRequestType=(0x02<<5) | 0x80, .wIndex=(size >> 16), .wLength=0, .wValue=size&0xffff};
    if(USBH_CtlSendSetup(phost, (uint8_t *) (&data), rpi->ctrl_pipe) != USBH_OK) Error_Handler(); //is it on endpoint 0?
    USBH_Delay(500);
    if(USBH_CtlReceiveData(phost, buffer, size, rpi->ctrl_pipe) != USBH_OK) Error_Handler();
    USBH_Delay(500);
}

static USBH_StatusTypeDef USBH_RPI_Process(USBH_HandleTypeDef *phost) {
    RPI_USB_status * s = phost->pActiveClass->pData;
    switch (s->status) {
        case RPI_IDLE:
            break;
        case RPI_SND_BOOTLOADER:
            write_code(0x41);

            //todo read bootcode4.bin
            // to reduce ram usage, read and send chunks
            size_t bootloader_size = 100;
            uint8_t * txbuf = malloc(bootloader_size);
            boot_message_t boot_msg = {.length = bootloader_size, /*.signature = get signature*/};
            //todo verify signature

            //send boot_message
            ep_write(phost, (uint8_t *) &boot_msg, sizeof(boot_msg));
            //send file - return > 0
            ep_write(phost, txbuf, boot_msg.length);
            //read return code -- should be 0
            int retcode;
            ep_read(phost, (uint8_t *) &retcode, sizeof(retcode));
            free(txbuf); //LOL non funzionerá mai
            break;
        default:
            Error_Handler();
    }
    return USBH_OK;
}

static USBH_StatusTypeDef USBH_RPI_SOFProcess(USBH_HandleTypeDef *phost) {
    return USBH_OK;
}