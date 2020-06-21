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
#define WHITE   "\033[37m"


vm_address_t get_real_addr( vm_address_t offset );


void read_memory(
                 pid_t pid,
                 mach_port_t port,
                 vm_address_t addr,
                 vm_size_t bytes);

void read_offset(
                 pid_t pid,
                 mach_port_t port,
                 vm_address_t offset,
                 vm_size_t bytes);

void read_lines(
                pid_t pid,
                mach_port_t port,
                vm_address_t addr,
                int lines);

void write_memory(
                  pid_t pid,
                  mach_port_t port,
                  vm_address_t addr,
                  vm_address_t data);

void write_offset(
                  pid_t pid,
                  mach_port_t port,
                  vm_address_t offset,
                  vm_address_t data);

void set_region_protection(
                           pid_t pid,
                           mach_port_t port,
                           vm_address_t addr,
                           vm_size_t size);


#ifdef __arm64

kern_return_t register_read(
                             pid_t pid,
                             mach_port_t port,
                             char reg[10]);

kern_return_t register_write(
                             pid_t pid,
                             mach_port_t port,
                             char reg[10],
                             vm_address_t val);

#endif
