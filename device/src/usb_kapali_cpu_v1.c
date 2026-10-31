/* Yalnız gerçek RISC-V CPU'da 4B return stub ABI testi; USB/MMIO/ALSA yok. */
#include <stdio.h>
#include <stdint.h>
extern long usb_kapali_ret(long);
__asm__(".text\n.balign 2\n.global usb_kapali_ret\nusb_kapali_ret:\n"
        ".byte 0x7d,0x55,0x82,0x80\n");
int main(void)
{
    const long a[] = {0, 1, 2, -1, INT32_MAX};
    for (unsigned i = 0; i < sizeof(a)/sizeof(a[0]); i++) {
        long r = usb_kapali_ret(a[i]);
        printf("arg=%ld return=%ld\n", a[i], r);
        if (r != -1) return 1;
    }
    puts("CPU STUB PASS: compressed li -1 / ret; USB/MMIO/ALSA acilmadi");
    return 0;
}
