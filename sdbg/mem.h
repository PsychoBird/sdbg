#include <stdio.h>
#include <mach/mach.h>
#include <unistd.h>
#include <sys/types.h>
#include <mach-o/dyld.h>
#include <string.h>
#include <stdlib.h>



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

kern_return_t register_magic(
                             pid_t pid,
                             mach_port_t port,
                             bool isWrite,
                             char reg[40]);

#endif
