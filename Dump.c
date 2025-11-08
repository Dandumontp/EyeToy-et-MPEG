#include <fcntl.h>
#include <libusb-1.0/libusb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
 
#define TIMEOUT 500
 
/*
 
>cat /etc/udev/rules.d/50-usb.rules
...................................
# Sony EyeToy
SUBSYSTEM=="usb", ATTRS{idVendor}=="054c", ATTRS{idProduct}=="0155", MODE="0666"
 
>cat /etc/modprobe.d/blacklist.conf
...................................
# Sony EyeToy
blacklist gspca_ov519
blacklist snd-usb-audio
 
>update-initramfs -u
 
>reboot
 
*/
 
 
// int libusb_control_transfer2(void* dev_handle,
//  uint8_t     bmRequestType,
//  uint8_t     bRequest,
//  uint16_t        wValue,
//  uint16_t        wIndex,
//  unsigned char*  data,
//  uint16_t        wLength,
//  unsigned int    timeout) {
//  printf("control = req:%02x, val:%02x, idx:%02x, len:%02x, data: ",
//              bmRequestType << 8 | bRequest, wValue, wIndex, wLength);
//  for (int i = 0; i < wLength; i++) {
//      printf("%02x ", data[i]);
//  }
//  printf("\n");
// }
 
int reg_write(void* dev_handle, int reg, int val) {
    uint8_t data = val;
    int length = sizeof(data);
    int ret = libusb_control_transfer(dev_handle, 0x40, 0x01, 0x00, reg, &data, length, TIMEOUT);
    if (ret != length) {
        printf("%s:%d ret=%d(%s)\n", __FUNCTION__, __LINE__, ret, libusb_error_name(ret));
        exit(1);
    }
}
 
int reg_read(void* dev_handle, int reg, int def) {
    uint8_t data = def;
    int length = sizeof(data);
    int ret = libusb_control_transfer(dev_handle, 0xc0, 0x01, 0x00, reg, &data, length, TIMEOUT);
    if (ret != length) {
        printf("%s:%d ret=%d(%s)\n", __FUNCTION__, __LINE__, ret, libusb_error_name(ret));
        exit(1);
    }
    return data;
}
 
int i2c_write(void* dev_handle, int reg, int val) {
    reg_write(dev_handle, 0x42, reg);
    reg_write(dev_handle, 0x45, val);
    reg_write(dev_handle, 0x47, 0x01);
}
 
int i2c_read(void* dev_handle, int reg, int def) {
    uint8_t data = def;
    reg_write(dev_handle, 0x43, reg);
    reg_write(dev_handle, 0x47, 0x03);
    reg_write(dev_handle, 0x47, 0x05);
    reg_read(dev_handle, 0x45, def);
    return data;
}
 
int eyetoy1_init_3(void* dev_handle, int arg1, int arg2, int arg3, int arg4, int arg5) {
    reg_write(dev_handle, 0x16, 0x00); // DIVIDER - Vertical Divide by 1 / Horizontal Divide by 1 
    
        i2c_write(dev_handle, 0x24, 0x30); // AEW  - AGC/AEC - Stable Operating Region - Upper Limit
        i2c_write(dev_handle, 0x25, 0x7a); // AEB  - AGC/AEC - Stable Operating Region - Lower Limit
        i2c_write(dev_handle, 0x67, 0x14); // RSVD - Reserved
        i2c_write(dev_handle, 0x74, 0x20); // COMM - Common Mode Control M - default
    
    reg_write(dev_handle, 0x59, arg1); // CAMERA_CLOCK - Camera Clock Divider - CCLK = 48MHz/CAMERA_CLOCK
 
        i2c_write(dev_handle, 0x28, arg2); // COMH - Common Control H
        i2c_write(dev_handle, 0x2d, arg3); // COMJ - Common Control J (bit4 = Auto-brightness)
        i2c_write(dev_handle, 0x11, 0x00); // CLKRC - Data Format and Internal Clock - default
    
    reg_write(dev_handle, 0xa2, 0x20); // Reserved
    reg_write(dev_handle, 0xa3, 0x20); // Reserved
    reg_write(dev_handle, 0xe1, 0x30); // Reserved
    reg_write(dev_handle, 0xe2, 0x78); // Reserved
    reg_write(dev_handle, 0xa4, 0x04); // Reserved (Compression level)
    reg_write(dev_handle, 0xa5, 0x20); // Reserved (Compression level)
    reg_write(dev_handle, 0xe0, 0x00); // Reserved
    reg_write(dev_handle, 0xe3, 0xef); // Reserved
    reg_write(dev_handle, 0xe4, 0x82); // Reserved
    reg_write(dev_handle, 0xe5, 0x82); // Reserved
    reg_write(dev_handle, 0xe7, 0x04); // Reserved
    reg_write(dev_handle, 0xe8, 0x04); // Reserved
 
        i2c_write(dev_handle, 0x2a, arg4); // FRARH - Output Format - Frame Rate Adjust High
        i2c_write(dev_handle, 0x2b, arg5); // FRARL - Data Format - Frame Rate Adjust Setting LSB
        i2c_write(dev_handle, 0x2a, arg4); // FRARH - Output Format - Frame Rate Adjust High
        i2c_write(dev_handle, 0x2b, arg5); // FRARL - Data Format - Frame Rate Adjust Setting LSB
 
    reg_write(dev_handle, 0x51, 0x00); // RESET1 - Reset Control Register 0 - clear flags
 
    reg_write(dev_handle, 0x10, 0x14); // H_SIZE - Image Width  : 0x14*16 = 0x0140 (320)
    reg_write(dev_handle, 0x11, 0x1e); // V_SIZE - Image Height : 0x1e* 8 = 0x00f0 (240)
    reg_write(dev_handle, 0x12, 0x00); // X_OFFSETL - Windows top-left X coordinate (Low)
    reg_write(dev_handle, 0x13, 0x00); // X_OFFSETH - Windows top-left X coordinate (High)
    reg_write(dev_handle, 0x14, 0x00); // Y_OFFSETL - Windows top-left Y coordinate (Low)
    reg_write(dev_handle, 0x15, 0x00); // Y_OFFSETH - Windows top-left Y coordinate (High)
    reg_write(dev_handle, 0x25, 0x01); // Format[2:0] - Image format - YUV411
}
 
int eyetoy1_init_2(void* dev_handle) {
    reg_write(dev_handle, 0x5a, 0x6d); // YS_CTRL - System Control
    reg_write(dev_handle, 0x53, 0xff); // EN_CLK0 - Clock Enable 0
    reg_write(dev_handle, 0x54, 0xff); // EN_CLK1 - Clock Enable 1
    reg_write(dev_handle, 0x5d, 0x03); // PWDN - Power-Down Control
    reg_write(dev_handle, 0x51, 0x0f); // RESET1 - Reset Control Register 0 - FIFO+JPEG+SFIFO+CIF
    reg_write(dev_handle, 0x51, 0x00); // RESET1 - Reset Control Register 0 - clear flags
    reg_write(dev_handle, 0x5d, 0x03); // PWDN - Power-Down Control
    reg_write(dev_handle, 0x53, 0x9f); // EN_CLK0 - Clock Enable 0
    reg_write(dev_handle, 0x54, 0x0f); // EN_CLK1 - Clock Enable 1
    reg_write(dev_handle, 0x37, 0x00); // Reserved
    reg_write(dev_handle, 0x20, 0x0d); // DFR - Data Formatter - Swap U0<>V0, Swap Y0<>Y1, Swap Y0<>U0
    reg_write(dev_handle, 0x21, 0x38); // SR - Syncronization Register - BufSync 8bit, BufSyncEnabl
    reg_write(dev_handle, 0x55, 0x02); // AUDIO_CLK - Audio Clock Control -  Clock Select 2.048 MHz
    reg_write(dev_handle, 0x22, 0x1d); // FRAR - Frame Rate Adjuster Register - YUV, EvenKeep, OddKeep, MaxFrameCount=5
    reg_write(dev_handle, 0x17, 0x50); // Reserved
    reg_write(dev_handle, 0x37, 0x01); // Reserved
    reg_write(dev_handle, 0x40, 0xff); // Reserved
    reg_write(dev_handle, 0x46, 0x00); // Reserved
    reg_write(dev_handle, 0x48, 0x00); // Reserved
    reg_write(dev_handle, 0x72, 0xee); // GPIO_IO_CTRL0 - GPIO Innput/Output Control0
    reg_write(dev_handle, 0x71, 0x00); // GPIO_DATA_OUT0 - GPIO Output Data0 - [bit0]=Red LED
    reg_write(dev_handle, 0x26, 0x00); // Reserved
    reg_write(dev_handle, 0x23, 0xff); // Reserved
    reg_write(dev_handle, 0x23, 0xff); // Reserved
 
    reg_write(dev_handle, 0x41, 0x42); // Reserved
    reg_write(dev_handle, 0x44, 0x43); // Reserved
 
        i2c_write(dev_handle, 0x12, 0x80); // COMA - Common Control A - Reset all registers to default values
 
        i2c_read(dev_handle, 0x00, 0x00); // AGC - Gain control gain setting
 
        i2c_read(dev_handle, 0x1c, 0x7f); // MIDH - Manufacturer ID Byte – High
        i2c_read(dev_handle, 0x1d, 0xa2); // MIDL - Manufacturer ID Byte – Low
        i2c_read(dev_handle, 0x1c, 0x7f); // MIDH - Manufacturer ID Byte – High
        i2c_read(dev_handle, 0x1d, 0xa2); // MIDL - Manufacturer ID Byte – Low
 
        i2c_write(dev_handle, 0x12, 0x80); // COMA - Common Control A - Reset all registers to default values
        i2c_write(dev_handle, 0x12, 0x80); // COMA - Common Control A - Reset all registers to default values
        i2c_write(dev_handle, 0x12, 0x80); // COMA - Common Control A - Reset all registers to default values
 
        i2c_write(dev_handle, 0x03, 0xa4); // SAT - Image Format – Color saturation value
        i2c_write(dev_handle, 0x04, 0x30); // HUE - Image Format – Color hue control
        i2c_write(dev_handle, 0x05, 0x88); // CWF - AWB – Red/Blue Pre-Amplifier gain setting
        i2c_write(dev_handle, 0x06, 0x60); // BRT - ABC – Brightness setting
        i2c_write(dev_handle, 0x11, 0x00); // CLKRC - Data Format and Internal Clock
        i2c_write(dev_handle, 0x12, 0x05); // COMA - Common Control A - AWB-Enable + Reserved
 
        i2c_read(dev_handle, 0x00, 0x00); // AGC - Gain control gain setting
 
        i2c_write(dev_handle, 0x00, 0x00); // AGC - Gain control gain setting
 
        i2c_write(dev_handle, 0x10, 0x82); // AECH - Exposure Value
        i2c_write(dev_handle, 0x13, 0xa3); // COMB - Common Control B - default
        i2c_write(dev_handle, 0x14, 0x24); // COMC - Common Control C - QVGA
        i2c_write(dev_handle, 0x15, 0x14); // COMD - Common Control D - Reserved
        i2c_write(dev_handle, 0x1f, 0x41); // FACT - Output Format - Format Control - Reserved
        i2c_write(dev_handle, 0x23, 0xde); // RSVD - Reserved
        i2c_write(dev_handle, 0x26, 0xa2); // COMF - Common Control F - default
        i2c_write(dev_handle, 0x27, 0xe2); // COMG - Common Control G - default
        i2c_write(dev_handle, 0x28, 0x20); // COMH - Common Control H - Output Format - Progressive
        i2c_write(dev_handle, 0x2f, 0x9d); // RSVD - Reserved
        i2c_write(dev_handle, 0x30, 0x00); // RSVD - Reserved
        i2c_write(dev_handle, 0x31, 0xc4); // RSVD - Reserved
        i2c_write(dev_handle, 0x60, 0xa6); // SPCB - Signal Process Control B - AGC + Reserved
        i2c_write(dev_handle, 0x61, 0xe0); // RSVD - Reserved
        i2c_write(dev_handle, 0x62, 0x88); // RSVD - Reserved
        i2c_write(dev_handle, 0x63, 0x11); // RSVD - Reserved
        i2c_write(dev_handle, 0x64, 0x89); // RSVD - Reserved
        i2c_write(dev_handle, 0x65, 0x00); // RSVD - Reserved
        i2c_write(dev_handle, 0x67, 0x94); // RSVD - Reserved
        i2c_write(dev_handle, 0x68, 0x7a); // RSVD - Reserved (bit 0:2 = Audio gain)
        i2c_write(dev_handle, 0x69, 0x08); // RSVD - Reserved
        i2c_write(dev_handle, 0x6c, 0x11); // RMCO - Color Matrix - RGB Crosstalk Compensation - R Channel
        i2c_write(dev_handle, 0x6d, 0x33); // GMCO - Color Matrix - RGB Crosstalk Compensation - G Channel
        i2c_write(dev_handle, 0x6e, 0x22); // BMCO - Color Matrix - RGB Crosstalk Compensation - B Channel
        i2c_write(dev_handle, 0x6f, 0x00); // RSVD - Reserved
        i2c_write(dev_handle, 0x74, 0x20); // COMM - Common Mode Control M - default
        i2c_write(dev_handle, 0x75, 0x06); // COMN - Common Mode Control N - Reserved
        i2c_write(dev_handle, 0x77, 0xc4); // RSVD - Reserved
 
        i2c_write(dev_handle, 0x14, 0x24); // COMC - Common Control C - QVGA
        i2c_write(dev_handle, 0x26, 0xa2); // COMF - Common Control F - default
 
        i2c_write(dev_handle, 0x17, 0x1c); // HSTART - Output Format - Horizontal Frame (HREF Column) Start
        i2c_write(dev_handle, 0x18, 0xbc); // HSTOP  - Output Format - Horizontal Frame (HREF Column) Stop
    
        i2c_write(dev_handle, 0x12, 0x45); // COMA - Common Control A - AWB-Enable + Reserved + Mirror Image Enable
        i2c_write(dev_handle, 0x20, 0xd2); // COME - Common Control E - Reserved + Edge Enh + AEC + Reserved
        i2c_write(dev_handle, 0x15, 0x00); // COMD - Common Control D - clear reserved flags
 
    eyetoy1_init_3(dev_handle, 0x00, 0x20, 0x01, 0x10, 0x00);
 
    reg_write(dev_handle, 0xa0, 0x42); // Reserved
    reg_write(dev_handle, 0x51, 0x00); // RESET1 - Reset Control Register 0 - clear flags
}
 
int eyetoy1_init_1(void* dev_handle) {
    int ret = 0;
    struct libusb_config_descriptor *config = NULL;
    libusb_device *dev = libusb_get_device(dev_handle);

    ret = libusb_get_active_config_descriptor(dev, &config);
    if (ret != LIBUSB_SUCCESS) {
        printf("%s:%d ret=%d(%s)\n", __FUNCTION__, __LINE__, ret, libusb_error_name(ret));
        exit(1);
    }

    printf("Device has %d interface(s)\n", config->bNumInterfaces);

    // Interface 0 — caméra principale
    if (config->bNumInterfaces > 0) {
        ret = libusb_detach_kernel_driver(dev_handle, 0);
        if (ret != LIBUSB_SUCCESS && ret != LIBUSB_ERROR_NOT_FOUND) {
            printf("%s:%d ret=%d(%s)\n", __FUNCTION__, __LINE__, ret, libusb_error_name(ret));
        }

        ret = libusb_claim_interface(dev_handle, 0);
        if (ret != LIBUSB_SUCCESS) {
            printf("%s:%d ret=%d(%s)\n", __FUNCTION__, __LINE__, ret, libusb_error_name(ret));
            exit(1);
        }

        // Si l’interface 0 a des alternate settings, on prend le plus haut
        const struct libusb_interface *iface0 = &config->interface[0];
        int alt = iface0->num_altsetting > 1 ? iface0->altsetting[iface0->num_altsetting - 1].bAlternateSetting : 0;
        printf("Interface 0 alternate setting = %d\n", alt);
        ret = libusb_set_interface_alt_setting(dev_handle, 0, alt);
        if (ret != LIBUSB_SUCCESS) {
            printf("%s:%d ret=%d(%s)\n", __FUNCTION__, __LINE__, ret, libusb_error_name(ret));
        }
    }

    // Interface 1 — souvent audio (microphone EyeToy)
    if (config->bNumInterfaces > 1) {
        ret = libusb_detach_kernel_driver(dev_handle, 1);
        if (ret != LIBUSB_SUCCESS && ret != LIBUSB_ERROR_NOT_FOUND) {
            printf("%s:%d ret=%d(%s)\n", __FUNCTION__, __LINE__, ret, libusb_error_name(ret));
        }

        ret = libusb_claim_interface(dev_handle, 1);
        if (ret == LIBUSB_SUCCESS) {
            const struct libusb_interface *iface1 = &config->interface[1];
            int alt = iface1->num_altsetting > 1 ? iface1->altsetting[iface1->num_altsetting - 1].bAlternateSetting : 0;
            printf("Interface 1 alternate setting = %d\n", alt);
            ret = libusb_set_interface_alt_setting(dev_handle, 1, alt);
            if (ret != LIBUSB_SUCCESS) {
                printf("%s:%d ret=%d(%s)\n", __FUNCTION__, __LINE__, ret, libusb_error_name(ret));
            }
        } else {
            printf("%s:%d cannot claim interface 1: %s\n", __FUNCTION__, __LINE__, libusb_error_name(ret));
        }
    }

    // Appelle les séquences d’initialisation du capteur
    eyetoy1_init_2(dev_handle);
    eyetoy1_init_2(dev_handle);
    eyetoy1_init_3(dev_handle, 0x02, 0x00, 0x41, 0x90, 0x98);

    libusb_free_config_descriptor(config);
    return 0;
}

 
struct isoc_trans {
    struct libusb_transfer *transfer_buff;
    uint8_t isoc_buffer[896];
    int is_pending_flag;
};
 
void isoc_transfer_completion_handler(struct libusb_transfer *transfer) {
    // printf("iso comp : %d/%d\n", transfer->length, transfer->actual_length);
    unsigned char *b = transfer->buffer;
    static int file_index = 0;
    static int fd = -1;
 
    if (transfer->buffer[0] == 0xff
     && transfer->buffer[1] == 0xff
     && transfer->buffer[2] == 0xff
     && transfer->buffer[3] == 0x50) {
        char file_name[80] = {0};
        sprintf(file_name, "dumps/file_%03d", file_index++);
        printf("open file = %s\n", file_name);
        fd = open(file_name, O_RDWR | O_CREAT );// | O_SYNC);
 
        for (int i = 0; i < transfer->length; i+=0x10) {
            if (b[i+0]!=0xFF || b[i+1]!=0xFF || b[i+2]!=0xFF || b[i+3]!=0xFF
             && b[i+4]!=0xFF || b[i+5]!=0xFF || b[i+6]!=0xFF || b[i+7]!=0xFF 
             && b[i+8]!=0xFF || b[i+9]!=0xFF || b[i+0xa]!=0xFF || b[i+0xb]!=0xFF 
             && b[i+0xc]!=0xFF || b[i+0xd]!=0xFF || b[i+0xe]!=0xFF || b[i+0xf]!=0xFF) {
                write(fd, b+i, 0x10);
            }
        }
    } else if (transfer->buffer[0] == 0xff
     && transfer->buffer[1] == 0xff
     && transfer->buffer[2] == 0xff
     && transfer->buffer[3] == 0x51) {
        write(fd, b, 0x10);
        close(fd);
        fd = -1;
    } else {
        for (int i = 0; i < transfer->length; i+=0x10) {
            if (b[i+0]!=0xFF || b[i+1]!=0xFF || b[i+2]!=0xFF || b[i+3]!=0xFF
             && b[i+4]!=0xFF || b[i+5]!=0xFF || b[i+6]!=0xFF || b[i+7]!=0xFF 
             && b[i+8]!=0xFF || b[i+9]!=0xFF || b[i+0xa]!=0xFF || b[i+0xb]!=0xFF 
             && b[i+0xc]!=0xFF || b[i+0xd]!=0xFF || b[i+0xe]!=0xFF || b[i+0xf]!=0xFF) {
                write(fd, b+i, 0x10);
            }
        }
    }
 
    if (transfer->user_data != NULL) {
        struct isoc_trans *sp = (struct isoc_trans*) transfer->user_data;
        sp->is_pending_flag = 0;
    }
}
 
int main(int argc, char **argv) {
    libusb_context *ctx = NULL;
    int ret = libusb_init(&ctx);
    if (ret != LIBUSB_SUCCESS) {
        printf("%s:%d ret=%d(%s)\n", __FUNCTION__, __LINE__, ret, libusb_error_name(ret));
        exit(1);
    }
 
    void* dev_handle = libusb_open_device_with_vid_pid(ctx, 0x054c, 0x0155);
    if (dev_handle == NULL) {
        printf("%s:%d ret=%p\n", __FUNCTION__, __LINE__, dev_handle);
        exit(1);
    }
 
    eyetoy1_init_1(dev_handle);
 
 
    // int pk_len = 896;
    int pk_len = 896;
    int loop = 1000;
 
    struct isoc_trans transfer_array[50];
    for(int i = 0; i < sizeof(transfer_array) / sizeof(transfer_array[0]); i++) {
        transfer_array[i].is_pending_flag = 0;
        transfer_array[i].transfer_buff = libusb_alloc_transfer(1);
    }
 
    struct timeval poll_timeout;
    poll_timeout.tv_sec = 0;
    poll_timeout.tv_usec = 1000;
 
    while (loop --> 0) {
        for(int i = 0; i < sizeof(transfer_array) / sizeof(transfer_array[0]); i++) {
            if(transfer_array[i].is_pending_flag) {
                libusb_handle_events_timeout(NULL, &poll_timeout);
                break;
            }
        }
        for(int i = 0; i < sizeof(transfer_array) / sizeof(transfer_array[0]); i++) {
            if(transfer_array[i].is_pending_flag) {
                libusb_handle_events_timeout(NULL, &poll_timeout);
                printf("pending...\n");
                continue;
            }
            memset(transfer_array[i].transfer_buff, 0, sizeof(transfer_array[i].transfer_buff));
 
            libusb_fill_iso_transfer(
                transfer_array[i].transfer_buff,
                dev_handle,
                LIBUSB_ENDPOINT_IN | 1,
                transfer_array[i].isoc_buffer,
                pk_len,
                1,
                isoc_transfer_completion_handler,
                &transfer_array[i],
                1000);
            libusb_set_iso_packet_lengths(transfer_array[i].transfer_buff, pk_len);
 
            transfer_array[i].is_pending_flag = 1;
            
            // À la place du break;
ret = libusb_submit_transfer(transfer_array[i].transfer_buff);
if (ret == LIBUSB_SUCCESS) {
    transfer_array[i].is_pending_flag = 1;
}

          //  ret = libusb_submit_transfer(transfer_array[i].transfer_buff);
 
          //  break;
        }
        libusb_handle_events(NULL);
    }
}