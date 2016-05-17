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
 * Copyright (C) 2011,2013 HiKoB.
 */

/*
 * eeprom_config.c
 *
 *  Last Modified: Jan 08, 2014
 *      Author: burindes
 *              harter
 */

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ftdi.h>

#define VENDOR 0x0403
#define PRODUCT_2232 0x6010
#define PRODUCT_4232 0x6011
enum ftdi_product {
    FTDI_2232 = PRODUCT_2232,
    FTDI_4232 = PRODUCT_4232,
};


static int update_name(char* current_name, char* new_name, char* new_serial,
                       int product);

static void print_usage_exit(char * bin)
{
    printf("Usage: %s [-t ftdi_type] [-e current_name] [-s new_serial] " \
           "new_name\n", bin);
    printf("  %c: ftdi_type in [2232, 4232] default 2232\n", 't');
    printf("  %c: FTDI current name\n", 'e');
    printf("  %c: SerialNumber\n", 's');

    exit(1);
}

int main(int argc, char *argv[])
{
    printf("FTx232 EEPROM writer by IoT-LAB\n");

    char* new_name = NULL;
    char* current_name = NULL;
    char* new_serial = NULL;
    enum ftdi_product ftdi_type = FTDI_2232;

    char c;
    opterr = 0;
    while ((c = getopt(argc, argv, "t:e:s:")) != (char)-1) {
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
            case 'e':
                current_name = optarg;
                break;
            case 's':
                new_serial = optarg;
                break;
            case '?':
                if (optopt == 't' || optopt == 'e' || optopt == 's')
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
    if (optind == argc) {
            printf("Missing new_name\n");
            print_usage_exit(argv[0]);
    }
    // Take the first of the remaining argument as new_name, others are ignored
    new_name = argv[optind];


    if (FTDI_2232 == ftdi_type)
        printf("Updating an FT2232...\n");
    else if (FTDI_4232 == ftdi_type)
        printf("Updating an FT4232...\n");

    int ret = update_name(current_name, new_name, new_serial, ftdi_type);
    if (ret)
        printf("No matching FTx232 found\n");
    else
        printf("Done!\n");
    return ret;
}

static int update_name(char* current_name, char* new_name, char* new_serial,
                       int product)
{
    int ret;
    struct ftdi_eeprom eeprom;
    unsigned char eeprom_buf[256];
    struct ftdi_context ftdi_struct;
    // too lazy to replace to &ftdi in all code
    struct ftdi_context *ftdi = &ftdi_struct;

    // Initialize the context
    // set ftdi->module_detach_mode = AUTO_DETACH_SIO_MODULE;
    // prevents "Unable to claim USB device"
    ftdi_init(ftdi);

    // Initialize the EEPROM values
    ftdi_eeprom_initdefaults(&eeprom);

    // Set other values
    eeprom.vendor_id = VENDOR;
    eeprom.product_id = product;
    eeprom.self_powered = 0;
    eeprom.remote_wakeup = 0;
    eeprom.chip_type = product == PRODUCT_2232 ? TYPE_2232H : TYPE_4232H;
    eeprom.max_power = 50;

    eeprom.manufacturer = "IoT-LAB";
    eeprom.product = new_name;
    eeprom.serial = new_serial;

    // Open ftdi device
    if (current_name) {
        printf("Opening device with name \"%s\"\n", current_name);
        ret = ftdi_usb_open_desc(ftdi, VENDOR, product, current_name, NULL);
    } else {
        printf("Opening device...\n");
        ret = ftdi_usb_open(ftdi, VENDOR, product);
    }
    if (ret != 0) {
        printf("Unable to find FTDI device\n");
        printf("Error: %s\n", ftdi->error_str);
        return 1;
    }

    // Force EEPROM size
    ftdi_eeprom_setsize(ftdi, &eeprom, 256);

    // Erase the EEPROM
    printf("Erasing eeprom...\n");
    ret = ftdi_erase_eeprom(ftdi);
    if (ret != 0)
    {
        printf("Unable to erase EEPROM\n");
        printf("Error: %s\n", ftdi->error_str);
        exit(-1);
    }

    // Build the eeprom content
    ftdi_eeprom_build(&eeprom, eeprom_buf);

    // Store it
    printf("Writing eeprom...\n");
    ret = ftdi_write_eeprom(ftdi, eeprom_buf);
    if (ret != 0)
    {
        printf("Unable to write EEPROM\n");
        printf("Error: %s\n", ftdi->error_str);
        exit(-1);
    }

    // An FTDI has been updated
    return 0;
}
