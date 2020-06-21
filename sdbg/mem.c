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
        printf("[+] vm_read_overwrite result: 0x%lx - as int: %lu\n", readOut, readOut); }
    else if (kret != KERN_SUCCESS) {
        printf("[!] Error! vm_read_overwrite failed: %s\n", mach_error_string(kret)); }

}

//reads memory
void read_lines(pid_t pid, mach_port_t port, char address[20], int lines) {
    vm_address_t addr = (vm_address_t) strtol(address, NULL, 0);
    address[strlen(address)-1] = '0';

    vm_address_t zeroaddr = (vm_address_t) strtol(address, NULL, 0);
    
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &port);
    if (kret != KERN_SUCCESS) {
        printf("[!] Error! Could not obtain task_for_pid - pid: %d\n", pid);
        printf("[!] Mach error: %s\n", mach_error_string(kret)); }
    
    printf("[+] Reading %d lines of memory from address: 0x%lx\n", lines, addr);
    printf(GREEN "0x%lx " WHITE "| ", addr);
    printf(YELLOW "00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F |\n");
    
    vm_address_t readOut;
    vm_size_t bytes = 1;
    
    for (int j = 0; j < lines; j++) {
        printf(CYAN "0x%lx " WHITE "| ", zeroaddr);
        for (int i = 0; i <= 0xf; i++) {
            kret = vm_read_overwrite(port, zeroaddr, bytes, (vm_offset_t) &readOut, &bytes);

            if (kret == KERN_SUCCESS) {
                if (readOut <= 0xf) {
                    printf("0%lx ", readOut); }
                else {
                    printf("%lx ", readOut); }
                zeroaddr += 0x1;
            }
        
            
            else if (kret != KERN_SUCCESS) {
                printf("[!] Error! vm_read_overwrite failed: %s\n", mach_error_string(kret));
                break;
            }
        }
        printf("|\n");
    }
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
        printf("[+] vm_read_overwrite result: 0x%lx - as int: %lu\n", readOut, readOut); }
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

kern_return_t register_read(pid_t pid, mach_port_t port, char reg[10]) {
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &port);
    if (kret != KERN_SUCCESS) {
        printf("[!] Error! Could not obtain task_for_pid - pid: %d\n", pid);
        printf("[!] Mach error: %s\n", mach_error_string(kret));
        return KERN_FAILURE;
    }

    //guided by https://www.exploit-db.com/papers/13176 although it was meant for PPC
    //WARNING: ARM64 REGISTERS ONLY!
    //developement moved to an iPad Air (arm64)
    thread_act_port_array_t thread_list;
    mach_msg_type_number_t thread_count;
    kret = task_threads(port, &thread_list, &thread_count);
    if (kret != KERN_SUCCESS) {
        printf("[!] Error! Could not get task_threads with error: %s\n", mach_error_string(kret));
        return KERN_FAILURE;
    }
    
    //https://opensource.apple.com/source/cctools/cctools-877.8/include/mach/arm/_structs.h.auto.html
    arm_thread_state64_t state;
    mach_msg_type_number_t scount = ARM_THREAD_STATE64_COUNT;
    
    thread_get_state(thread_list[0], ARM_THREAD_STATE64, (thread_state_t) &state, &scount);
    if (strcmp(reg, "all\n") == 0) {
        printf("[!] Reading all registers...\n");
        for (int i = 0; i < 30; i++) {
            printf(GREEN "x%d " WHITE "= 0x%x\n", i, state.__x[i]); }
        printf(YELLOW "fp " WHITE "= 0x%x\n", state.__fp);
        printf(YELLOW "lr " WHITE "= 0x%x\n", state.__lr);
        printf(YELLOW "sp " WHITE "= 0x%x\n", state.__sp);
        printf(RED "pc " WHITE "= 0x%x\n", state.__pc);
        printf(YELLOW "cpsr " WHITE "= 0x%x\n", state.__cpsr);
        printf(YELLOW "pad " WHITE "= 0x%x\n", state.__pad);
    }
    
    if (strcmp(reg, "all\n") != 0) {
        if (strcmp(reg, "x0\n") == 0) { printf(GREEN "x0 " WHITE "= 0x%x\n", state.__x[0]); }         // x0
        else if (strcmp(reg, "x1\n") == 0) { printf(GREEN "x1 " WHITE "= 0x%x\n", state.__x[1]); }    // x1
        else if (strcmp(reg, "x2\n") == 0) { printf(GREEN "x2 " WHITE "= 0x%x\n", state.__x[2]); }    // x2
        else if (strcmp(reg, "x3\n") == 0) { printf(GREEN "x3 " WHITE "= 0x%x\n", state.__x[3]); }    // x3
        else if (strcmp(reg, "x4\n") == 0) { printf(GREEN "x4 " WHITE "= 0x%x\n", state.__x[4]); }    // x4
        else if (strcmp(reg, "x5\n") == 0) { printf(GREEN "x5 " WHITE "= 0x%x\n", state.__x[5]); }    // x5
        else if (strcmp(reg, "x6\n") == 0) { printf(GREEN "x6 " WHITE "= 0x%x\n", state.__x[6]); }    // x6
        else if (strcmp(reg, "x7\n") == 0) { printf(GREEN "x7 " WHITE "= 0x%x\n", state.__x[7]); }    // x7
        else if (strcmp(reg, "x8\n") == 0) { printf(GREEN "x8 " WHITE "= 0x%x\n", state.__x[8]); }    // x8
        else if (strcmp(reg, "x9\n") == 0) { printf(GREEN "x9 " WHITE "= 0x%x\n", state.__x[9]); }    // x9
        else if (strcmp(reg, "x10\n") == 0) { printf(GREEN "x10 " WHITE "= 0x%x\n", state.__x[10]); } // x10
        else if (strcmp(reg, "x11\n") == 0) { printf(GREEN "x11 " WHITE "= 0x%x\n", state.__x[11]); } // x11
        else if (strcmp(reg, "x12\n") == 0) { printf(GREEN "x12 " WHITE "= 0x%x\n", state.__x[12]); } // x12
        else if (strcmp(reg, "x13\n") == 0) { printf(GREEN "x13 " WHITE "= 0x%x\n", state.__x[13]); } // x13
        else if (strcmp(reg, "x14\n") == 0) { printf(GREEN "x14 " WHITE "= 0x%x\n", state.__x[14]); } // x14
        else if (strcmp(reg, "x15\n") == 0) { printf(GREEN "x15 " WHITE "= 0x%x\n", state.__x[15]); } // x15
        else if (strcmp(reg, "x16\n") == 0) { printf(GREEN "x16 " WHITE "= 0x%x\n", state.__x[16]); } // x16
        else if (strcmp(reg, "x17\n") == 0) { printf(GREEN "x17 " WHITE "= 0x%x\n", state.__x[17]); } // x17
        else if (strcmp(reg, "x18\n") == 0) { printf(GREEN "x18 " WHITE "= 0x%x\n", state.__x[18]); } // x18
        else if (strcmp(reg, "x19\n") == 0) { printf(GREEN "x19 " WHITE "= 0x%x\n", state.__x[19]); } // x19
        else if (strcmp(reg, "x20\n") == 0) { printf(GREEN "x20 " WHITE "= 0x%x\n", state.__x[20]); } // x20
        else if (strcmp(reg, "x21\n") == 0) { printf(GREEN "x21 " WHITE "= 0x%x\n", state.__x[21]); } // x21
        else if (strcmp(reg, "x22\n") == 0) { printf(GREEN "x22 " WHITE "= 0x%x\n", state.__x[22]); } // x22
        else if (strcmp(reg, "x23\n") == 0) { printf(GREEN "x23 " WHITE "= 0x%x\n", state.__x[23]); } // x23
        else if (strcmp(reg, "x24\n") == 0) { printf(GREEN "x24 " WHITE "= 0x%x\n", state.__x[24]); } // x24
        else if (strcmp(reg, "x25\n") == 0) { printf(GREEN "x25 " WHITE "= 0x%x\n", state.__x[25]); } // x25
        else if (strcmp(reg, "x26\n") == 0) { printf(GREEN "x26 " WHITE "= 0x%x\n", state.__x[26]); } // x26
        else if (strcmp(reg, "x27\n") == 0) { printf(GREEN "x27 " WHITE "= 0x%x\n", state.__x[27]); } // x27
        else if (strcmp(reg, "x28\n") == 0) { printf(GREEN "x28 " WHITE "= 0x%x\n", state.__x[28]); } // x28
        else if (strcmp(reg, "x29\n") == 0) { printf(GREEN "x29 " WHITE "= 0x%x\n", state.__x[29]); } // x29
        else if (strcmp(reg, "fp\n") == 0) { printf(YELLOW "fp " WHITE "= 0x%x\n", state.__fp); }      // fp
        else if (strcmp(reg, "lr\n") == 0) { printf(YELLOW "lr " WHITE "= 0x%x\n", state.__lr); }      // lr
        else if (strcmp(reg, "sp\n") == 0) { printf(YELLOW "sp " WHITE "= 0x%x\n", state.__sp); }      // sp
        else if (strcmp(reg, "pc\n") == 0) { printf(RED "pc " WHITE "= 0x%x\n", state.__pc); }         // pc
        else if (strcmp(reg, "cpsr\n") == 0) { printf(YELLOW "cpsr " WHITE "= 0x%x\n", state.__cpsr); }// cpsr
        else if (strcmp(reg, "pad\n") == 0) { printf(YELLOW "pad " WHITE "= 0x%x\n", state.__pad); }   //pad
        else { printf("[!] Error! Invalid register.\n"); }
    }
    return KERN_SUCCESS;
}

kern_return_t register_write(pid_t pid, mach_port_t port, char reg[10], vm_address_t val) {
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &port);
    if (kret != KERN_SUCCESS) {
        printf("[!] Error! Could not obtain task_for_pid - pid: %d\n", pid);
        printf("[!] Mach error: %s\n", mach_error_string(kret));
        return KERN_FAILURE;
    }

    //guided by https://www.exploit-db.com/papers/13176 although it was meant for PPC
    //WARNING: ARM64 REGISTERS ONLY!
    //developement moved to an iPad Air (arm64)
    thread_act_port_array_t thread_list;
    mach_msg_type_number_t thread_count;
    kret = task_threads(port, &thread_list, &thread_count);
    if (kret != KERN_SUCCESS) {
        printf("[!] Error! Could not get task_threads with error: %s\n", mach_error_string(kret));
        return KERN_FAILURE;
    }
    
    //https://opensource.apple.com/source/cctools/cctools-877.8/include/mach/arm/_structs.h.auto.html
    arm_thread_state64_t state;
    mach_msg_type_number_t scount = ARM_THREAD_STATE64_COUNT;
    
    thread_get_state(thread_list[0], ARM_THREAD_STATE64, (thread_state_t) &state, &scount);

    
    if (strcmp(reg, "x0") == 0) { printf(GREEN "x0 " WHITE "written with 0x%lx\n", val); state.__x[0] = val; }         // x0
    else if (strcmp(reg, "x1") == 0) { printf(GREEN "x1 " WHITE "written with 0x%lx\n", val); state.__x[1] = val; }    // x1
    else if (strcmp(reg, "x2") == 0) { printf(GREEN "x2 " WHITE "written with 0x%lx\n", val); state.__x[2] = val; }    // x2
    else if (strcmp(reg, "x3") == 0) { printf(GREEN "x3 " WHITE "written with 0x%lx\n", val); state.__x[3] = val; }    // x3
    else if (strcmp(reg, "x4") == 0) { printf(GREEN "x4 " WHITE "written with 0x%lx\n", val); state.__x[4] = val; }    // x4
    else if (strcmp(reg, "x5") == 0) { printf(GREEN "x5 " WHITE "written with 0x%lx\n", val); state.__x[5] = val; }    // x5
    else if (strcmp(reg, "x6") == 0) { printf(GREEN "x6 " WHITE "written with 0x%lx\n", val); state.__x[6] = val; }    // x6
    else if (strcmp(reg, "x7") == 0) { printf(GREEN "x7 " WHITE "written with 0x%lx\n", val); state.__x[7] = val; }    // x7
    else if (strcmp(reg, "x8") == 0) { printf(GREEN "x8 " WHITE "written with 0x%lx\n", val); state.__x[8] = val; }    // x8
    else if (strcmp(reg, "x9") == 0) { printf(GREEN "x9 " WHITE "written with 0x%lx\n", val); state.__x[9] = val; }    // x9
    else if (strcmp(reg, "x10") == 0) { printf(GREEN "x10 " WHITE "written with 0x%lx\n", val); state.__x[10] = val; } // x10
    else if (strcmp(reg, "x11") == 0) { printf(GREEN "x11 " WHITE "written with 0x%lx\n", val); state.__x[11] = val; } // x11
    else if (strcmp(reg, "x12") == 0) { printf(GREEN "x12 " WHITE "written with 0x%lx\n", val); state.__x[12] = val; } // x12
    else if (strcmp(reg, "x13") == 0) { printf(GREEN "x13 " WHITE "written with 0x%lx\n", val); state.__x[13] = val; } // x13
    else if (strcmp(reg, "x14") == 0) { printf(GREEN "x14 " WHITE "written with 0x%lx\n", val); state.__x[14] = val; } // x14
    else if (strcmp(reg, "x15") == 0) { printf(GREEN "x15 " WHITE "written with 0x%lx\n", val); state.__x[15] = val; } // x15
    else if (strcmp(reg, "x16") == 0) { printf(GREEN "x16 " WHITE "written with 0x%lx\n", val); state.__x[16] = val; } // x16
    else if (strcmp(reg, "x17") == 0) { printf(GREEN "x17 " WHITE "written with 0x%lx\n", val); state.__x[17] = val; } // x17
    else if (strcmp(reg, "x18") == 0) { printf(GREEN "x18 " WHITE "written with 0x%lx\n", val); state.__x[18] = val; } // x18
    else if (strcmp(reg, "x19") == 0) { printf(GREEN "x19 " WHITE "written with 0x%lx\n", val); state.__x[19] = val; } // x19
    else if (strcmp(reg, "x20") == 0) { printf(GREEN "x20 " WHITE "written with 0x%lx\n", val); state.__x[20] = val; } // x20
    else if (strcmp(reg, "x21") == 0) { printf(GREEN "x21 " WHITE "written with 0x%lx\n", val); state.__x[21] = val; } // x21
    else if (strcmp(reg, "x22") == 0) { printf(GREEN "x22 " WHITE "written with 0x%lx\n", val); state.__x[22] = val; } // x22
    else if (strcmp(reg, "x23") == 0) { printf(GREEN "x23 " WHITE "written with 0x%lx\n", val); state.__x[23] = val; } // x23
    else if (strcmp(reg, "x24") == 0) { printf(GREEN "x24 " WHITE "written with 0x%lx\n", val); state.__x[24] = val; } // x24
    else if (strcmp(reg, "x25") == 0) { printf(GREEN "x25 " WHITE "written with 0x%lx\n", val); state.__x[25] = val; } // x25
    else if (strcmp(reg, "x26") == 0) { printf(GREEN "x26 " WHITE "written with 0x%lx\n", val); state.__x[26] = val; } // x26
    else if (strcmp(reg, "x27") == 0) { printf(GREEN "x27 " WHITE "written with 0x%lx\n", val); state.__x[27] = val; } // x27
    else if (strcmp(reg, "x28") == 0) { printf(GREEN "x28 " WHITE "written with 0x%lx\n", val); state.__x[28] = val; } // x28
    else if (strcmp(reg, "x29") == 0) { printf(GREEN "x29 " WHITE "written with 0x%lx\n", val); state.__x[29] = val; } // x29
    else if (strcmp(reg, "fp") == 0) { printf(YELLOW "fp " WHITE "written with 0x%lx\n", val); state.__fp = val; }      // fp
    else if (strcmp(reg, "lr") == 0) { printf(YELLOW "lr " WHITE "written with 0x%lx\n", val); state.__lr = val; }      // lr
    else if (strcmp(reg, "sp") == 0) { printf(YELLOW "sp " WHITE "written with 0x%lx\n", val); state.__sp = val; }      // sp
    else if (strcmp(reg, "pc") == 0) { printf(RED "pc " WHITE "written with 0x%lx\n", val); state.__pc = val; }         // pc
    else if (strcmp(reg, "cpsr") == 0) { printf(YELLOW "cpsr " WHITE "written with 0x%lx\n", val); state.__cpsr = val; }// cpsr
    else if (strcmp(reg, "pad") == 0) { printf(YELLOW "pad " WHITE "written with 0x%lx\n", val); state.__pad = val; }    //pad
    else { printf("[!] Error! Invalid register.\n"); }

    thread_set_state(thread_list[0], ARM_THREAD_STATE64, &state, scount);
    
    return KERN_SUCCESS;
}

#endif


