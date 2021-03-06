/*
 * This file is part of HiKoB Openlab.
 *
 * HiKoB Openlab is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, version 3.
 *
 * HiKoB Openlab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with HiKoB Openlab. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2011,2012,2013 HiKoB.
 */

/*
 * devices_list.c
 *
 *  Created on: Aug 3, 2012
 *      Author: burindes
 */

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <ftdi.h>

#define VENDOR 0x0403
#define PRODUCT_2232 0x6010
#define PRODUCT_4232 0x6011
enum ftdi_product {
    FTDI_2232 = PRODUCT_2232,
    FTDI_4232 = PRODUCT_4232,
};

#if LIBFTDI1
#define USB_DEVICE struct libusb_device
#else /* LIBFTDI1 */
#define USB_DEVICE struct usb_device
#endif

static void print_usage_exit(char * bin)
{
    printf("usage: %s [-t ftdi_type] [-r]\n", bin);
    printf("  %c: ftdi_type in [2232, 4232] default 2232\n", 't');
    printf("  %c: reset ftdi devices\n", 'r');
    exit(1);
}

static void do_list(struct ftdi_context *ftdi, int product, int reset);

int main(int argc, char *argv[])
{
    printf("FTx232 devices lister by IoT-LAB\n");

    struct ftdi_context ftdi;
    enum ftdi_product ftdi_type = FTDI_2232;
    int reset = 0;

    char c;
    opterr = 0;
    while ((c = getopt(argc, argv, "t:r")) != (char)-1) {
        switch (c) {
            case 't':
                if (strcmp(optarg, "2232") == 0) {
                    ftdi_type = FTDI_2232;
                } else if (strcmp(optarg, "4232") == 0) {
                    ftdi_type = FTDI_4232;
                } else {
                    printf("Option -t requires 2232 or 4232 argument.\n");
                    print_usage_exit(argv[0]);
                }
                break;
            case 'r':
                reset = 1;
                break;
            case '?':
                if (optopt == 't')
                    printf("Option -%c requires an argument.\n", optopt);
                else if (isprint(optopt))
                    printf("Unknown option `-%c`.\n", optopt);
                else
                    printf("Unknown option character `\\x%x`.\n", optopt);
            default:
                print_usage_exit(argv[0]);
                return EINVAL;
        }
    }

    // Initialize the context
    ftdi_init(&ftdi);

    if (FTDI_2232 == ftdi_type)
        printf("Listing FT2232 devices...\n");
    else if (FTDI_4232 == ftdi_type)
        printf("Listing FT4232 devices...\n");
    do_list(&ftdi, ftdi_type, reset);

    // De-initialize the context
    ftdi_deinit(&ftdi);

    printf("All done, success!\n");
    return 0;
}


static void _device_reset(struct ftdi_context *ftdi, USB_DEVICE *usbdevice)
{
    // Open the USB device
    ftdi_usb_open_dev(ftdi, usbdevice);

    // Reset it
    ftdi_usb_reset(ftdi);

    // And close the handle
    ftdi_usb_close(ftdi);
}


static void _device_print(struct ftdi_context *ftdi, USB_DEVICE *usbdevice)
{
    char manufacturer[128];
    char description[128];
    char serial[128];
    ftdi_usb_get_strings(ftdi, usbdevice,
                         manufacturer, sizeof(manufacturer),
                         description, sizeof(description),
                         serial, sizeof(serial));
    printf("\tManufacturer: %s \n", manufacturer);
    printf("\tDescription: %s \n", description);
    printf("\tSerial: %s\n", serial);
}


static void do_list(struct ftdi_context *ftdi, int product, int reset)
{
    int ret;
    struct ftdi_device_list* device_list = NULL;
    ret = ftdi_usb_find_all(ftdi, &device_list, VENDOR, product);

    if (ret < 0) {
        printf("Failed to list FTDI devices\n");
        printf("Error: %s\n", ftdi->error_str);
        exit(-1);
    } else if (ret == 0) {
        printf("No FTDI device found\n");
    } else {
        printf("Found %u device(s)\n", ret);
        int num = 0;
        struct ftdi_device_list* dev;
        for (dev = device_list; dev != NULL; dev = dev->next, num++) {

            if (reset) {
                printf("\tResetting device...\n");
                _device_reset(ftdi, dev->dev);
                printf("\t\tOK\n");
            }

            printf("Device %u: \n", num);
            _device_print(ftdi, dev->dev);
        }
    }

    // Free the list
    ftdi_list_free(&device_list);
}
