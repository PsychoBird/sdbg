#include "mem.h"


//returns your offset / app addr + slide like magic!
vm_address_t get_real_addr(vm_address_t offset) {
    vm_address_t slide = _dyld_get_image_vmaddr_slide(0);
    return offset + slide;
}

//reads memory
void read_memory(pid_t pid, mach_port_t port, vm_address_t addr, vm_size_t bytes) {
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &port);
    if (kret != KERN_SUCCESS) {
        printf("[!] Error! Could not obtain task_for_pid - pid: %d\n", pid);
        printf("[!] Mach error: %s\n", mach_error_string(kret)); }
    
    printf("[+] Reading %d bytes from memory from address: 0x%lx\n", (int) bytes, addr);
    vm_address_t readOut;
    kret = vm_read_overwrite(port, addr, bytes, (vm_offset_t) &readOut, &bytes);

    if (kret == KERN_SUCCESS) {
        printf("[+] vm_read_overwrite return: 0x%lx - as int: %lu\n", readOut, readOut); }
    else if (kret != KERN_SUCCESS) {
        printf("[!] Error! vm_read_overwrite failed: %s\n", mach_error_string(kret)); }

}

//reads memory of offset
//see get_real_addr()
void read_offset(pid_t pid, mach_port_t port, vm_address_t offset, vm_size_t bytes) {
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &port);
    if (kret != KERN_SUCCESS) {
        printf("[!] Error! Could not obtain task_for_pid - pid: %d\n", pid);
        printf("[!] Mach error: %s\n", mach_error_string(kret)); }
    
    printf("[!] Current slide is: 0x%lx\n", (vm_address_t) _dyld_get_image_vmaddr_slide(0));
    printf("[+] Reading %d bytes from memory from offset: 0x%lx\n", (int) bytes, get_real_addr(offset));
    vm_address_t readOut;
    kret = vm_read_overwrite(port, get_real_addr(offset), bytes, (vm_offset_t) &readOut, &bytes);

    if (kret == KERN_SUCCESS) {
        printf("[+] vm_read_overwrite return: 0x%lx - as int: %lu\n", readOut, readOut); }
    else if (kret != KERN_SUCCESS) {
        printf("[!] Error! vm_read_overwrite (offset) failed: %s\n", mach_error_string(kret)); }

}

//write memory to address
void write_memory(pid_t pid, mach_port_t port, vm_address_t addr, vm_address_t data) {
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &port);
    if (kret != KERN_SUCCESS) {
        printf("[!] Error! Could not obtain task_for_pid - pid: %d\n", pid);
        printf("[!] Mach error: %s\n", mach_error_string(kret)); }

    printf("[+] Writing memory: 0x%lx with data: 0x%lx\n", addr, data);
    kret = vm_write(port, addr, (vm_offset_t) &data, sizeof(data));
    
    if (kret == KERN_SUCCESS) {
        printf("[+] vm_write success at: 0x%lx\n", addr); }
    else if (kret != KERN_SUCCESS) {
        printf("[!] Error! vm_write failed: %s\n", mach_error_string(kret)); }

}

//write memory to offset
//see get_real_addr()
//may not work as expected, works in theory but permission changes could be required
void write_offset(pid_t pid, mach_port_t port, vm_address_t offset, vm_address_t data) {
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &port);
    if (kret != KERN_SUCCESS) {
        printf("[!] Error! Could not obtain task_for_pid - pid: %d\n", pid);
        printf("[!] Mach error: %s\n", mach_error_string(kret)); }
    
    printf("[!] Current slide is: 0x%lx\n", (vm_address_t) _dyld_get_image_vmaddr_slide(0));
    printf("[+] Writing address: 0x%lx with data: 0x%lx\n", get_real_addr(offset), data);
    kret = vm_write(port, get_real_addr(offset), (vm_offset_t) &data, sizeof(data));
    
    if (kret == KERN_SUCCESS) {
        printf("[+] vm_write (offset) success at: %lx\n", get_real_addr(offset)); }
    else if (kret != KERN_SUCCESS) {
        printf("[!] Error! vm_write (offset) failed: %s\n", mach_error_string(kret)); }
}


//vm_protect
void set_region_protection(pid_t pid, mach_port_t port, vm_address_t addr, vm_size_t size) {
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &port);
    if (kret != KERN_SUCCESS) {
        printf("[!] Error! Could not obtain task_for_pid - pid: %d\n", pid);
        printf("[!] Mach error: %s\n", mach_error_string(kret)); }
    
    printf("[!] Setting %d byte region permissions at 0x%lx as R|W|X\n", (int) size, addr);
    kret = vm_protect(port, addr, size, 0, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE);
        
    if (kret == KERN_SUCCESS) {
        printf("[+] vm_protect success!\n"); }
    else if (kret != KERN_SUCCESS) {
        printf("[!] Error! vm_protect failure with: %s\n", mach_error_string(kret)); }
}


#ifdef __arm64

kern_return_t register_magic(pid_t pid, mach_port_t port, bool isWrite, char reg[40]) {
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &port);
    if (kret != KERN_SUCCESS) {
        printf("[!] Error! Could not obtain task_for_pid - pid: %d\n", pid);
        printf("[!] Mach error: %s\n", mach_error_string(kret)); }
    
    if (strcmp(reg, "all") == 0) {
        printf("[!] Reading all registers...\n");
    }

    //guided by https://www.exploit-db.com/papers/13176 although it was meant for PPC
    thread_act_port_array_t thread_list;
    mach_msg_type_number_t thread_count;
    kret = task_threads(port, &thread_list, &thread_count);
    if (kret != KERN_SUCCESS) {
        printf("[!] Error! Could not get task_threads with error: %s\n", mach_error_string(kret));
        return KERN_FAILURE;
    }

    arm_thread_state_t *state;
    mach_msg_type_number_t scount = ARM_THREAD_STATE_COUNT;
    
    //https://opensource.apple.com/source/cctools/cctools-877.8/include/mach/arm/_structs.h.auto.html
    //not arm64!
    thread_get_state(thread_list[0], ARM_THREAD_STATE, (thread_state_t) state, &scount);
    printf("[!] r3 = 0x%x\n", state->__r[2]);
    
    return KERN_SUCCESS;
}

#endif


