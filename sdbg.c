#include <stdio.h>
#include <mach/mach.h>
#include <unistd.h>
#include <sys/types.h>
#include <mach-o/dyld.h>
#include <string.h>
#include <stdlib.h>

/*
 sign with these entitlements
 ldid -Sent.xml sdbg


 <?xml version="1.0" encoding="UTF-8"?>
 <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd …">
 <plist version="1.0">
 <dict>
 <key>task_for_pid-allow</key><true/>
 <key>get-task-allow</key> <true/>
 <key>proc_info-allow</key> <true/>
 <key>run-unsigned-code</key><true/>
 </dict>
 </plist>
 */

int pid = 0;
mach_port_t port;

//#define DBG_ARM
//let me compile on my mac pls



//returns your offset / app addr + slide like magic!
vm_address_t get_real_addr(vm_address_t offset) {
    vm_address_t slide = _dyld_get_image_vmaddr_slide(0);
    return offset + slide;
}

//reads memory
void read_memory(mach_port_t port, vm_address_t addr, vm_size_t bytes) {
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &port);
    
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
void read_offset(mach_port_t port, vm_address_t offset, vm_size_t bytes) {
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &port);
    
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
void write_memory(mach_port_t port, vm_address_t addr, vm_address_t data) {
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &port);

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
void write_offset(mach_port_t port, vm_address_t offset, vm_address_t data) {
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &port);
    
    printf("[!] Current slide is: 0x%lx\n", (vm_address_t) _dyld_get_image_vmaddr_slide(0));
    printf("[+] Writing address: 0x%lx with data: 0x%lx\n", get_real_addr(offset), data);
    kret = vm_write(port, get_real_addr(offset), (vm_offset_t) &data, sizeof(data));
    
    if (kret == KERN_SUCCESS) {
        printf("[+] vm_write (offset) success at: %lx\n", get_real_addr(offset)); }
    else if (kret != KERN_SUCCESS) {
        printf("[!] Error! vm_write (offset) failed: %s\n", mach_error_string(kret)); }
}


//vm_protect
void set_region_protection(mach_port_t port, vm_address_t addr, vm_size_t size) {
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &port);
    
    printf("[!] Setting %d byte region permissions at 0x%lx as R|W|X\n", (int) size, addr);
    kret = vm_protect(port, addr, size, 0, VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE);
        
    if (kret == KERN_SUCCESS) {
        printf("[+] vm_protect success!\n"); }
    else if (kret != KERN_SUCCESS) {
        printf("[!] Error! vm_protect failure with: %s\n", mach_error_string(kret)); }
}


#if defined(DBG_ARM)

kern_return_t register_magic(mach_port_t port, bool isWrite, char reg[40]) {
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &port);
    
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


//cli
void interact(mach_port_t port) {
    char input[128];
    char args[10][30];
    
    printf("[+] Entering CLI... for a list of commands, type 'help'\n");
    
    while (1) {
    
        for (int i=0; i<10; i++) {
            memset(args[i],0,sizeof(args));
        }
    
        printf("(SDBG) ");
        fgets(input, 128, stdin);
    
        int charIndex = 0;
        int argIndex = 0;
        int argCharIndex = 0;
    
        input[strlen(input)] = ' ';
    
        for (int i = 0; i < strlen(input); i++) {
            if (input[i] == ' ') {
                argCharIndex = 0;
                for (; charIndex < i; charIndex++) {
                    args[argIndex][argCharIndex] = input[charIndex];
                    argCharIndex++;
                }
                //printf("Argument #%d = %s\n", argIndex, args[argIndex]); //DEBUG
                argIndex++;
                charIndex++;
            }
        }
        
    
        //HELP
        if (strcmp(args[0], "help\n") == 0) {
            printf("\n[!] List of commands:\n"
                   "write 0x[address] 0x[data] - writes 0x[data] to 0x[address]\n"
                   "writeoffset 0x[offset] 0x[data] - writes 0x[data] to 0x[offset] + slide\n"
                   "read 0x[address] [bytes] - reads [bytes] from 0x[address]\n"
                   "readoffset 0x[offset] [bytes] - reads [bytes] from 0x[offset] + slide\n"
                   "slide - gets current slide as 0x[slide]\n"
                   "protect 0x[address] [bytes] - sets R|W|X permissions at 0x[address] for [bytes]\n"
                   "regread [register] - reads register - pass \"all\" to get all registers\n"
                   "regwrite [register] - writes register\n"
                   "exit - self explanatory\n"); }
        
        //EXIT
        else if (strcmp(args[0], "exit\n") == 0) {
            printf("[!] Exiting SDBG...\n"); exit(0); }
        else if (strcmp(args[0], "quit\n") == 0) {
            printf("[!] Exiting SDBG...\n"); exit(0); }
        
        //get slide
        else if (strcmp(args[0], "slide\n") == 0) {
            printf("[!] Current slide is: 0x%lx\n", (vm_address_t) _dyld_get_image_vmaddr_slide(0)); }
   
#if defined(DBG_ARM)
        
        //register_magic (2 args)
        else if (strcmp(args[0], "regread") == 0 && args[1][0] != '\0') {
            register_magic(port, false, args[1]); }
        else if (strcmp(args[0], "regread\n") == 0) {
            printf("[!] Error! Not enough arguments for regread!\n"); }
        else if (strcmp(args[0], "regwrite") == 0 && args[1][0] != '\0') {
            printf("[!] REG WRITE\n"); }
        else if (strcmp(args[0], "regwrite\n") == 0) {
            printf("[!] Error! Not enough arguments for regwrite!\n"); }
            
#endif
        
        //set_region_protection
        else if (strcmp(args[0], "protect") == 0 && args[1][0] != '\0' && args[2][0] != '\0') {
            set_region_protection(port, (vm_address_t) strtol(args[1], NULL, 0), (vm_size_t) strtol(args[2], NULL, 0)); }
        else if (strcmp(args[0], "protect") == 0 && args[1][0] != '\0' && args[2][0] == '\0') {
            printf("[!] Error! Not enough arguments for protect!\n"); }
        else if (strcmp(args[0], "protect\n") == 0) {
            printf("[!] Error! Not enough arguments for protect!\n"); }
        
        //write_memory (3 args)
        else if (strcmp(args[0], "write") == 0 && args[1][0] != '\0' && args[2][0] != '\0') {
            write_memory(port, (vm_address_t) strtol(args[1], NULL, 0), (vm_address_t) strtol(args[2], NULL, 0)); }
        else if (strcmp(args[0], "write") == 0 && args[1][0] != '\0' && args[2][0] == '\0') {
            printf("[!] Error! Not enough arguments for write!\n"); }
        else if (strcmp(args[0], "write\n") == 0) {
            printf("[!] Error! Not enough arguments for write!\n"); }
        
        //write_offset (3 args)
        else if (strcmp(args[0], "writeoffset") == 0 && args[1][0] != '\0' && args[2][0] != '\0') {
            write_offset(port, (vm_address_t) strtol(args[1], NULL, 0), (vm_address_t) strtol(args[2], NULL, 0)); }
        else if (strcmp(args[0], "writeoffset") == 0 && args[1][0] != '\0' && args[2][0] == '\0') {
            printf("[!] Error! Not enough arguments for writeoffset!\n"); }
        else if (strcmp(args[0], "writeoffset\n") == 0) {
            printf("[!] Error! Not enough arguments for writeoffset!\n"); }
        
        //read_memory (3 args)
        else if (strcmp(args[0], "read") == 0 && args[1][0] != '\0' && args[2][0] != '\0') {
            read_memory(port, (vm_address_t) strtol(args[1], NULL, 0), (vm_address_t) strtol(args[2], NULL, 0)); }
        else if (strcmp(args[0], "read") == 0 && args[1][0] != '\0' && args[2][0] == '\0') {
            printf("[!] Error! Not enough arguments for read!\n"); }
        else if (strcmp(args[0], "read\n") == 0) {
            printf("[!] Error! Not enough arguments for read!\n"); }
        
        //read_offset (3 args)
        else if (strcmp(args[0], "readoffset") == 0 && args[1][0] != '\0' && args[2][0] != '\0') {
            read_offset(port, (vm_address_t) strtol(args[1], NULL, 0), (vm_address_t) strtol(args[2], NULL, 0)); }
        else if (strcmp(args[0], "readoffset") == 0 && args[1][0] != '\0' && args[2][0] == '\0') {
            printf("[!] Error! Not enough arguments for readoffset!\n"); }
        else if (strcmp(args[0], "readoffset\n") == 0) {
            printf("[!] Error! Not enough arguments for readoffset!\n"); }

        //UNKNOWN COMMAND
        else { printf("[!] Error! Unknown command\n"); }
    }
}


int main() {
    printf("[!] Welcome to SDBG!\n\n");

    //check if root
    if (geteuid() && getuid()) {
        printf("[!] Run SDBG as root.\n[!] Do you have the proper entitlements?\n[!] Exiting SDBG...\n");
        exit(0);
    }

    //get pid to attach
    printf("[!] PID to attach: ");
    scanf("%d", &pid);
    printf("[+] Attaching to pid: %d\n", pid);
    getchar();

    //task_for_pid through get_task_for_pid wrapper
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &port);
    if (kret != KERN_SUCCESS) {
        printf("[!] Error! Couldn't obtain task_for_pid: %d\n[!] Do you have the proper entitlements?\n[!] Exiting SDBG...\n", pid);
        exit(0);
    }
    else {
        printf("[+] Obtained task_for_pid: %d!\n", pid); }
    
    interact(port);
    return 0;
}
