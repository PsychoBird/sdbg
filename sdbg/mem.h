#include <stdio.h>
#include <mach/mach.h>
#include <unistd.h>
#include <sys/types.h>
#include <mach-o/dyld.h>
#include <string.h>
#include <stdlib.h>

#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[00m"

#define GOOD "\033[32m # " WHITE
#define ERROR "\033[31m # " WHITE

#define EXIT printf(ERROR"Exiting SDBG...\n"); exit(0);

typedef unsigned char byte_t;

vm_address_t get_real_addr( vm_address_t offset );


void read_memory(
                 mach_port_t task,
                 vm_address_t addr,
                 vm_size_t bytes);

void read_offset(
                 mach_port_t task,
                 vm_address_t offset,
                 vm_size_t bytes);

void read_lines(
                mach_port_t task,
                char address[20],
                int lines,
                bool printchar);

void write_memory(
                  mach_port_t task,
                  vm_address_t addr,
                  vm_address_t data);

void write_offset(
                  mach_port_t task,
                  vm_address_t offset,
                  vm_address_t data);

void set_region_protection(
                           mach_port_t task,
                           vm_address_t addr,
                           vm_size_t size);


#ifdef __arm64

kern_return_t register_read(
                             mach_port_t task,
                             char reg[10]);

kern_return_t register_write(
                             mach_port_t task,
                             char reg[10],
                             uint64_t val);

#endif
