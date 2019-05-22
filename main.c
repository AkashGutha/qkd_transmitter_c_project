#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "hwlib.h"
#include "soc_cv_av/socal/socal.h"
#include "soc_cv_av/socal/hps.h"
#include "soc_cv_av/socal/alt_gpio.h"

// pointers for virtual bases
void *h2p_lw_axi_vbase;
void *h2p_hw_axi_vbase;

int main(int argc, char const *argv[])
{

    // file descriptor
    int fd;

    // OPEN "/dev/mem" -> Map the device peripherals into a linux vbase
    if (fd = open("/dev/mem", (O_RDWR | O_SYNC)) == -1)
    {
        printf("ERROR: Could not open \"/dev/mem\"...\n");
        return 1;
    }

    // get the base address for LW axi bridge
    h2p_lw_vbase = mmap(
        NULL,
        HW_REGS_SPAN,
        (PROT_READ | PROT_WRITE),
        MAP_SHARED,
        fd,
        HW_REGS_BASE);

    if (h2p_lw_vbase == MAP_FAILED)
    {
        printf("ERROR: mmap failed... \n");
        close(fd);
        return 1;
    }

    // get the base address for the HW axi bridge

    return 0;
}
