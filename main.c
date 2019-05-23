#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "errno.h"
#include "hwlib.h"
#include "soc_cv_av/socal/socal.h"
#include "soc_cv_av/socal/hps.h"
#include "soc_cv_av/socal/alt_gpio.h"

#include "hps.h"
#include "base_addresses.h"
#include "quartus.h"

// pointers for virtual bases
void *h2p_lw_axi_vbase;
void *h2p_hw_axi_vbase;

// pointers for ram
void *ram_on_axi_base;

// FILE pointer and other requirements for the input file
FILE *file;
UINT32 buffer;

int main(int argc, char const *argv[])
{
    // file descriptor
    int fd;
    unsigned int i = 0;

    // get the file path from arguments and open the file
    if (argc != 2)
    {
        printf("Please pass in 2 arguments instead of %d arguments\r\n", argc);
        return -1;
    }
    file = fopen(argv[1], "r");

    if (file == NULL)
    {
        printf("ERROR: Opening the file at %s \r\nError code: %d \r\n", argv[1], errno);
        return -1;
    }

    // OPEN "/dev/mem" -> Map the device peripherals into a linux vbase
    fd = open("/dev/mem", (O_RDWR | O_SYNC));
    if (fd == -1)
    {
        printf("ERROR: Could not open \"/dev/mem\"...\n");
        return 1;
    }

    // get the base address for the LW axi bridge
    h2p_lw_axi_vbase = mmap(NULL, HW_REGS_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, HW_REGS_BASE);

    if (h2p_lw_axi_vbase == MAP_FAILED)
    {
        printf("ERROR: lw axi mmap() failed..................\r\n");
        close(fd);
        return 1;
    }

    // get the base address for the HW axi bridge
    h2p_hw_axi_vbase = mmap(NULL, HW_FPGA_AXI_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, ALT_AXI_FPGASLVS_OFST);

    if (h2p_hw_axi_vbase == MAP_FAILED)
    {
        printf("ERROR: hw axi mmap() failed....................\r\n");
        close(fd);
        return 1;
    }

    //-------------------------------------------------------------
    // Read from the sd card and write into ram on the fpga
    //-------------------------------------------------------------

    printf("Starting the write process...........................\r\n");
    ram_on_axi_base = h2p_hw_axi_vbase + ((ULONG)ONCHIP_MEMORY2_0_BASE & (ULONG)HW_FPGA_AXI_MASK);
    for (i = 0; i < 256; i++)
    {
        fread(&buffer, sizeof(UINT32), 1, file);
        *((UP32)ram_on_axi_base + i) = buffer;
    }
    printf("Completed writing data to the ram \r\n");
    printf("Reading data from the ram : \r\n");
    for (i = 0; i < 256; i++)
    {
        printf("%u => %zu \r\n", i, *((UP32)ram_on_axi_base + i));
    }

    //-------------------------------------------------------------
    // clean up our memory mapping and exit
    //-------------------------------------------------------------
    if (munmap(h2p_hw_axi_vbase, HW_FPGA_AXI_SPAN) != 0)
    {
        printf("ERROR: axi munmap() failed.............................\r\n");
        close(fd);
        return 1;
    }

    fclose(file);
    close(fd);

    return 0;
}
