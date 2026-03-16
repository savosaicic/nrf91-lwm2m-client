#include <zephyr/kernel.h>

int main(void)
{
  while (1) {
    k_sleep(K_FOREVER);
  }

  return 0;
}
