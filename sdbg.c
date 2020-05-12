#include <stdio.h>
#include <mach/mach.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>
#include <mach-o/dyld.h>
#include <string.h>
#include <stdlib.h>


int pid, addr, data = 0;
mach_port_t port;

//check if program is running as root
bool isRoot() {
    if (getuid() && geteuid()) {
        return false;
    }
    return true;
}

//checking task_for_pid for target process. parameter is process id
//readMemory/writeMemory will not work if get_task_for_pid != KERN_SUCCESS
kern_return_t get_task_for_pid(int pid) {
    kern_return_t kret;
    kret = task_for_pid(mach_task_self(), pid, &port);
    
    if (kret != KERN_SUCCESS) {
        return KERN_FAILURE;
    }
    return KERN_SUCCESS;
}

//returns your offset + current aslr slide like magic!
uint32_t getRealOffset(uint32_t offset) {
    return offset + _dyld_get_image_vmaddr_slide(0);
}

//reads memory
void readMemory(uint32_t addr, vm_size_t bytes) {
    unsigned char *readOut = malloc(sizeof(char)*2);
    kern_return_t kret;
    kret = vm_read_overwrite(port, (vm_address_t) addr, bytes, (vm_offset_t) &readOut, &bytes);
    
    if (kret == KERN_SUCCESS) {
        printf("vm_read_overwite success!\n");
    }
    if (kret != KERN_SUCCESS) {
        printf("Error! vm_read_overwrite failed!\n");
    }
    
}

//reads memory of offset
//see getReadOffset()
void readOffset(uint32_t addr, vm_size_t bytes) {
    unsigned char *readOut = malloc(sizeof(char)*2);
    kern_return_t kret;
    kret = vm_read_overwrite(port, (vm_address_t) getRealOffset(addr), bytes, (vm_offset_t) &readOut, &bytes);
    
    if (kret == KERN_SUCCESS) {
        printf("vm_read_overwite (offset) success!\n");
    }
    if (kret != KERN_SUCCESS) {
        printf("Error! vm_read_overwrite (offset) failed!\n");
    }
    
}

//write memory to address
void writeMemory(uint32_t addr, uint32_t data) {
    kern_return_t kret;
    vm_write(port, (vm_address_t) addr, (vm_address_t)data, sizeof(data));
    if (kret == KERN_SUCCESS) {
        printf("vm_write success!\n");
    }
    else if (kret != KERN_SUCCESS) {
        printf("Error! vm_write failed!\n");
    }
    
}

//write memory to offset
//see getRealOffset()
void writeOffset(uint32_t offset, uint32_t data) {
    kern_return_t kret;
    vm_write(port, (vm_address_t) getRealOffset(offset), (vm_address_t)data, sizeof(data));
    if (kret == KERN_SUCCESS) {
        printf("vm_write (offset) success!\n");
    }
    else if (kret != KERN_SUCCESS) {
        printf("Error! vm_write (offset) failed!\n");
    }
}


thread_state_t get_reg(thread_t reg) {
    kern_return_t kret;
    
    return KERN_SUCCESS;
}

thread_state_t set_reg(thread_t reg) {
    kern_return_t kret;
    
    return KERN_SUCCESS;
}

thread_state_t get_all_reg(thread_t reg) {
    kern_return_t kret;
    
    return KERN_SUCCESS;
}


//cli
void interact() {
    char input[128];
    char instr[5][30];
    
    printf("For list of commands, type 'help'\n");
    while (1) {
        
        printf("\n(SDBG) ");
        fgets(input, sizeof input, stdin);
        
        
        int k=0;
        int cmd = 0;
        for(int i=0; i<=(strlen(input)); i++)
        {
            
            if(input[i]==' '||input[i]=='\0')
            {
                instr[cmd][k]='\0';
                cmd++;
                k=0;
            }
            else
            {
                instr[cmd][k]=input[i];
                k++;
            }
        }
        
        
        if (strcmp(instr[0], "help\n") == 0) {
            printf("\nList of commands:\n"
                   "write [address] [data] - writes [data] to [address]\n"
                   "writeoffset [offset] [data] - writes [data] to [offset] + slide\n"
                   "read [address] [bytes] - reads [bytes] from [address]\n"
                   "read [offset] [bytes] - reads [bytes] from [offset] + slide\n"
                   "exit - self explanatory\n");
            strcpy(instr[0], "");
        }
        
        else if (strcmp(instr[0], "write") == 0 && strcmp(instr[1], "\0") != 0 && strcmp(instr[2], "\0") != 0) {
            writeMemory((uint32_t) instr[1], (uint32_t) instr[2]);
        }
        else if (strcmp(instr[0], "write\n") == 0) {
            printf("Not enough parameters for write!\n");
        }
        else if (strcmp(instr[0], "writeoffset") == 0 && strcmp(instr[1], "\0") != 0 && strcmp(instr[2], "\0") != 0) {
            writeOffset((uint32_t) instr[1], (uint32_t) instr[2]);
        }
        else if (strcmp(instr[0], "writeoffset\n") == 0) {
            printf("Not enough parameters for writeoffset!\n");
        }
        else if (strcmp(instr[0], "read") == 0 && strcmp(instr[1], "\0") != 0 && strcmp(instr[2], "\0") != 0) {
            readMemory((uint32_t) instr[1], (int) instr[2]);
        }
        else if (strcmp(instr[0], "read\n") == 0) {
            printf("Not enough parameters for read!\n");
        }
        else if (strcmp(instr[0], "readoffset") == 0 && strcmp(instr[1], "\0") != 0 && strcmp(instr[2], "\0") != 0) {
            readOffset((uint32_t) instr[1], (int) instr[2]);
        }
        else if (strcmp(instr[0], "readoffset\n") == 0) {
            printf("Not enough parameters for readoffset!\n");
        }
        else if (strcmp(instr[0], "exit\n") == 0) {
            printf("Exiting SDBG...\n");
            exit(0);
        }
        else {
            printf("Unknown command\n");
        }

    }
}


int main() {
    printf("Welcome to SDBG!\n\n");
    
    //check if root
    if (!isRoot()) {
        printf("Error! Run SDBG as root. Exiting SDBG...\n");
        return -1;
    }
    
    //get pid to attach
    printf("PID to attach: ");
    scanf("%d", &pid);
    getchar();
    
    //task_for_pid
    //get_task_for_pid returns KERN_SUCCESS on success and KERN_FAILURE on...
    if (get_task_for_pid(pid) == KERN_FAILURE) {
        printf("Error! Couldn't obtain task_for_pid(%d) - Exiting SDBG...\n", pid);
        return -1;
    }
    
    //writeOffset(0x10092DEE8, 0x1F2003D5);
    //NOP^^ test works!!
    
    interact();
    return 0;
}
