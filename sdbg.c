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

//#define DBG_ARM
//let me compile on my mac pls

//returns your offset / app addr + slide like magic!
uint32_t getRealOffset(vm_address_t offset) {
    return offset + _dyld_get_image_vmaddr_slide(0);
}

//reads memory
void readMemory(mach_port_t port, vm_address_t addr, vm_size_t bytes) {
    printf("[+] Reading %d bytes from memory from address: 0x%lx\n", (int) bytes, addr);
    unsigned char *readOut = malloc(sizeof(char)*bytes);
    kern_return_t kret;
    kret = vm_read_overwrite(port, addr, bytes, (vm_offset_t) &readOut, &bytes);

    if (kret == KERN_SUCCESS) {
        printf("[+] vm_read_overwrite result: %u\n", (unsigned int)&readOut);
    }
    if (kret != KERN_SUCCESS) {
        printf("[!] Error! vm_read_overwrite failed!\n");
    }

}

//reads memory of offset
//see getReadOffset()
void readOffset(mach_port_t port, vm_address_t addr, vm_size_t bytes) {
    printf("[+] Reading %d bytes from memory from offset: 0x%lx\n", (int) bytes, addr);
    unsigned char *readOut = malloc(sizeof(char)*bytes);
    kern_return_t kret;
    kret = vm_read_overwrite(port, getRealOffset(addr), bytes, (vm_offset_t) &readOut, &bytes);

    if (kret == KERN_SUCCESS) {
        printf("[+] vm_read_overwrite (offset) result: %u\n", (unsigned int) &readOut);
    }
    if (kret != KERN_SUCCESS) {
        printf("[!] Error! vm_read_overwrite (offset) failed!\n");
    }

}

//write memory to address
void writeMemory(mach_port_t port, vm_address_t addr, vm_address_t data) {
    printf("%lu\n", addr);
    printf("%lu\n", data);
    printf("[+] Writing memory: 0x%lx with data: 0x%lx\n", addr, data);
    kern_return_t kret;
    vm_write(port, addr, (vm_offset_t) &data, sizeof(data));
    if (kret == KERN_SUCCESS) {
        printf("[+] vm_write success at: 0x%lx\n", addr);
    }
    else if (kret != KERN_SUCCESS) {
        printf("[!] Error! vm_write failed!\n");
    }

}

//write memory to offset
//see getRealOffset()
void writeOffset(mach_port_t port, vm_address_t offset, vm_address_t data) {
    printf("[+] Writing offset: 0x%lx with data: 0x%lx\n", offset, data);
    kern_return_t kret;
    vm_write(port, (vm_address_t) getRealOffset(offset), (vm_offset_t) &data, sizeof(data));
    if (kret == KERN_SUCCESS) {
        printf("[+] vm_write (offset) success: %x\n", getRealOffset(offset));
    }
    else if (kret != KERN_SUCCESS) {
        printf("[!] Error! vm_write (offset) failed!\n");
    }
}



#if defined(DBG_ARM)

kern_return_t get_reg(char[] reg) {
    kern_return_t kret;

    thread_act_port_array_t thread_list;
    mach_msg_type_number_t thread_count;
    task_threads(port, &thread_list, &thread_count);

    //mit documentation
    thread_act_t reg;
    arm_thread_state_t *state;
    mach_msg_type_number_t scount = ARM_THREAD_STATE_COUNT;

    kret = thread_get_state(thread_list[0], ARM_THREAD_STATE, (thread_state_t) state, &scount);

    if (kret != KERN_SUCCESS) {
        return KERN_FAILURE;
    }
    printf("")

    return KERN_SUCCESS;
}

kern_return_t set_reg(thread_t reg) {
    kern_return_t kret;

    return KERN_SUCCESS;
}

kern_return_t get_all_reg(thread_t reg) {
    kern_return_t kret;
    //thanks MIT!
     thread_act_port_array_t thread_list;
     mach_msg_type_number_t thread_count;
     kret = task_threads(port, &thread_list, &thread_count);
     if (kret != KERN_SUCCESS) {
         return KERN_FAILURE;
     }

     arm_thread_state_t *state;
     mach_msg_type_number_t scount = ARM_THREAD_STATE_COUNT;

    thread_get_state(thread_list[0], ARM_THREAD_STATE, (thread_state_t) state, &thread_count);

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
                   "read 0x[address] 0x[bytes] - reads 0x[bytes] from 0x[address]\n"
                   "read 0x[offset] 0x[bytes] - reads 0x[bytes] from 0x[offset] + slide\n"
                   "exit - self explanatory\n");
        }
        
        //EXIT
        else if (strcmp(args[0], "exit\n") == 0) {
            printf("[!] Exiting SDBG...\n"); exit(0); }
        else if (strcmp(args[0], "quit\n") == 0) {
            printf("[!] Exiting SDBG...\n"); exit(0); }
        
        //writeMemory (3 args)
        else if (strcmp(args[0], "write") == 0 && args[1][0] != '\0' && args[2][0] != '\0') {
            writeMemory(port, (vm_address_t) strtol(args[1], NULL, 0), (vm_address_t) strtol(args[2], NULL, 0)); }
        else if (strcmp(args[0], "write") == 0 && args[1][0] != '\0' && args[2][0] == '\0') {
            printf("[!] Error! Not enough arguments for write!\n"); }
        else if (strcmp(args[0], "write\n") == 0) {
            printf("[!] Error! Not enough arguments for write!\n"); }
        
        //writeOffset (3 args)
        else if (strcmp(args[0], "writeoffset") == 0 && args[1][0] != '\0' && args[2][0] != '\0') {
            writeOffset(port, (vm_address_t) strtol(args[1], NULL, 0), (vm_address_t) strtol(args[2], NULL, 0)); }
        else if (strcmp(args[0], "writeoffset") == 0 && args[1][0] != '\0' && args[2][0] == '\0') {
            printf("[!] Error! Not enough arguments for writeoffset!\n"); }
        else if (strcmp(args[0], "writeoffset\n") == 0) {
            printf("[!] Error! Not enough arguments for writeoffset!\n"); }
        
        //readMemory (3 args)
        else if (strcmp(args[0], "read") == 0 && args[1][0] != '\0' && args[2][0] != '\0') {
            readMemory(port, (vm_address_t) strtol(args[1], NULL, 0), (vm_address_t) strtol(args[2], NULL, 0)); }
        else if (strcmp(args[0], "read") == 0 && args[1][0] != '\0' && args[2][0] == '\0') {
            printf("[!] Error! Not enough arguments for read!\n"); }
        else if (strcmp(args[0], "read\n") == 0) {
            printf("[!] Error! Not enough arguments for read!\n"); }
        
        //readOffset (3 args)
        else if (strcmp(args[0], "readoffset") == 0 && args[1][0] != '\0' && args[2][0] != '\0') {
            readMemory(port, (vm_address_t) strtol(args[1], NULL, 0), (vm_address_t) strtol(args[2], NULL, 0)); }
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
    int pid = 0;
    printf("[!] PID to attach: ");
    scanf("%d", &pid);
    printf("[+] Attaching to pid: %d\n", pid);
    getchar();

    //task_for_pid
    mach_port_t port;
    kern_return_t kret;
    kret = task_for_pid(mach_task_self(), pid, &port);

    if (kret != KERN_SUCCESS) {
        printf("[!] Error! Couldn't obtain task_for_pid: %d\n[!] Do you have the proper entitlements?\n[!] Exiting SDBG...\n", pid);
        exit(0);
    }
    else {
        printf("[+] Obtained task_for_pid: %d!\n", pid);
    }

    //writeOffset(0x10092DEE8, 0x1F2003D5);
    //NOP^^ test works!!
    writeMemory(port, 0x174797800, 0xfffffff);

    interact(port);
    return 0;
}
