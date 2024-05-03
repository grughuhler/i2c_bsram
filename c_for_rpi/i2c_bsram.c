/* Simple bsram test using i2c.  See README.
 *
 * SPDX: BSD 2-Clause "Simplified" License: BSD-2-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <i2c/smbus.h>

#define ADDR_H 0
#define ADDR_L 1
#define DATA_W 2
#define DATA_R 3

unsigned char do_read(int file, unsigned short addr)
{
  unsigned char addr_h, addr_l, data_r;

  addr_h = (0x3f00 & addr) >> 8;
  addr_l = 0xff & addr;

  if (i2c_smbus_write_byte_data(file, ADDR_H, addr_h)) {
    perror("Write to slave failed");
  }
  if (i2c_smbus_write_byte_data(file, ADDR_L, addr_l)) {
    perror("Write to slave failed");
  }

  data_r = i2c_smbus_read_byte_data(file, DATA_R);
  
  return data_r;
}


void do_write(int file, unsigned short addr, unsigned char data_w)
{
  unsigned char addr_h, addr_l;

  addr_h = (0x3f00 & addr) >> 8;
  addr_l = 0xff & addr;

  if (i2c_smbus_write_byte_data(file, ADDR_H, addr_h)) {
    perror("Write to slave failed");
  }
  if (i2c_smbus_write_byte_data(file, ADDR_L, addr_l)) {
    perror("Write to slave failed");
  }

  if (i2c_smbus_write_byte_data(file, DATA_W, data_w)) {
    perror("Write to slave failed");
  }
}


int do_test(int file, int quick)
{
  int errors;
  unsigned int addr, addr_inc;
  unsigned char data;

  if (quick) addr_inc = 32;
  else addr_inc = 1;
  
  errors = 0;
  srandom(0);
  for (addr = 0; addr < 16384; addr += addr_inc) {
    data = random()&0xff;
    if (addr % 1024 == 0) printf("write %x to %hx\n", data, addr);
    do_write(file, addr, data);
    data += 1;
  }

  srandom(0);
  for (addr = 0; addr < 16384; addr += addr_inc) {
    data = random()&0xff;
    if (do_read(file, addr) != data) {
      errors += 1;
    }
    if (addr % 1024 == 0) printf("read from %hx\n", addr);
    data += 1;
  }

  return errors;
}

unsigned long get_val(char *str)
{
  char *tmp;
  unsigned long val;

  val = strtoul(str, &tmp, 0);
  if(*tmp != 0) {
    fprintf(stderr, "Error: %s is not a number\n", str);
    exit(EXIT_FAILURE);
  }

  return val;
}


int main(int argc, char **argv)
{
  int file;
  char *fname = "/dev/i2c-1";
  int dev_addr = 0x31;
  int opt;
  extern char *optarg;
  extern int optind;
  unsigned int addr;
  unsigned char data;

  /* First open the correct i2c bus */
  if ((file = open(fname, O_RDWR)) < 0) {
    perror("Cannot open i2c bus");
    exit(EXIT_FAILURE);
  }

  /* Set address of i2c target to be read/written */
  if (ioctl(file, I2C_SLAVE, dev_addr) < 0) {
    perror("Cannot access slave device");
    close(file);
    exit(EXIT_FAILURE);
  }

  while ((opt = getopt(argc, argv, "hstr:w:")) != -1) {
    switch (opt) {
    case 't':
      printf("test completed with %d errors\n", do_test(file, 0));
      break;
    case 's':
      printf("test completed with %d errors\n", do_test(file, 1));
      break;
    case 'r':
      addr = get_val(optarg) & 0x3fff;
      printf("mem[0x%x] = %x\n", addr, do_read(file, addr));
      break;
    case 'w':
      addr = 0x3fff & get_val(optarg);
      if (optind < argc && *argv[optind] != '-') {
        data = get_val(argv[optind]);
	do_write(file, addr, data);
        optind++;
      } else {
        fprintf(stderr, "-w requires two arguments\n");
        exit(EXIT_FAILURE);
      }
      break;
    case 'h':
    default:
      fprintf(stderr, "Usage: i2c_bsram [-t] [-s] [-r addr] [-w addr val]\n");
      exit(EXIT_FAILURE);
    }
  }

  close(file);

  return EXIT_SUCCESS;
}
