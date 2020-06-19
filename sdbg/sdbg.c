#include "mem.h"




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




//cli
void interact(pid_t pid, mach_port_t port) {
    char input[128];
    char args[10][30];
    
    printf("[+] Entering CLI... for a list of commands, type 'help'\n");
    
    while (1) {
    
        memset(args, 0, sizeof(args[0][0])*10*30);
    
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
        
#ifdef __arm64
    
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
        
#else
    
        //HELP
        if (strcmp(args[0], "help\n") == 0) {
            printf("\n[!] List of commands:\n"
                   "write 0x[address] 0x[data] - writes 0x[data] to 0x[address]\n"
                   "writeoffset 0x[offset] 0x[data] - writes 0x[data] to 0x[offset] + slide\n"
                   "read 0x[address] [bytes] - reads [bytes] from 0x[address]\n"
                   "readoffset 0x[offset] [bytes] - reads [bytes] from 0x[offset] + slide\n"
                   "slide - gets current slide as 0x[slide]\n"
                   "protect 0x[address] [bytes] - sets R|W|X permissions at 0x[address] for [bytes]\n"
                   "exit - self explanatory\n"); }

        
#endif
        
        //EXIT
        else if (strcmp(args[0], "exit\n") == 0) {
            printf("[!] Exiting SDBG...\n"); exit(0); }
        else if (strcmp(args[0], "quit\n") == 0) {
            printf("[!] Exiting SDBG...\n"); exit(0); }
        
        //get slide
        else if (strcmp(args[0], "slide\n") == 0) {
            printf("[!] Current slide is: 0x%lx\n", (vm_address_t) _dyld_get_image_vmaddr_slide(0)); }
   
#ifdef __arm64
        
        //register_magic (2 args)
        else if (strcmp(args[0], "regread") == 0 && args[1][0] != '\0') {
            register_magic(pid, port, false, args[1]); }
        else if (strcmp(args[0], "regread\n") == 0) {
            printf("[!] Error! Not enough arguments for regread!\n"); }
        else if (strcmp(args[0], "regwrite") == 0 && args[1][0] != '\0') {
            printf("[!] REG WRITE\n"); }
        else if (strcmp(args[0], "regwrite\n") == 0) {
            printf("[!] Error! Not enough arguments for regwrite!\n"); }
            
#endif
        
        //set_region_protection
        else if (strcmp(args[0], "protect") == 0 && args[1][0] != '\0' && args[2][0] != '\0') {
            set_region_protection(pid, port, (vm_address_t) strtol(args[1], NULL, 0), (vm_size_t) strtol(args[2], NULL, 0)); }
        else if (strcmp(args[0], "protect") == 0 && args[1][0] != '\0' && args[2][0] == '\0') {
            printf("[!] Error! Not enough arguments for protect!\n"); }
        else if (strcmp(args[0], "protect\n") == 0) {
            printf("[!] Error! Not enough arguments for protect!\n"); }
        
        //write_memory (3 args)
        else if (strcmp(args[0], "write") == 0 && args[1][0] != '\0' && args[2][0] != '\0') {
            write_memory(pid, port, (vm_address_t) strtol(args[1], NULL, 0), (vm_address_t) strtol(args[2], NULL, 0)); }
        else if (strcmp(args[0], "write") == 0 && args[1][0] != '\0' && args[2][0] == '\0') {
            printf("[!] Error! Not enough arguments for write!\n"); }
        else if (strcmp(args[0], "write\n") == 0) {
            printf("[!] Error! Not enough arguments for write!\n"); }
        
        //write_offset (3 args)
        else if (strcmp(args[0], "writeoffset") == 0 && args[1][0] != '\0' && args[2][0] != '\0') {
            write_offset(pid, port, (vm_address_t) strtol(args[1], NULL, 0), (vm_address_t) strtol(args[2], NULL, 0)); }
        else if (strcmp(args[0], "writeoffset") == 0 && args[1][0] != '\0' && args[2][0] == '\0') {
            printf("[!] Error! Not enough arguments for writeoffset!\n"); }
        else if (strcmp(args[0], "writeoffset\n") == 0) {
            printf("[!] Error! Not enough arguments for writeoffset!\n"); }
        
        //read_memory (3 args)
        else if (strcmp(args[0], "read") == 0 && args[1][0] != '\0' && args[2][0] != '\0') {
            read_memory(pid, port, (vm_address_t) strtol(args[1], NULL, 0), (vm_address_t) strtol(args[2], NULL, 0)); }
        else if (strcmp(args[0], "read") == 0 && args[1][0] != '\0' && args[2][0] == '\0') {
            printf("[!] Error! Not enough arguments for read!\n"); }
        else if (strcmp(args[0], "read\n") == 0) {
            printf("[!] Error! Not enough arguments for read!\n"); }
        
        //read_offset (3 args)
        else if (strcmp(args[0], "readoffset") == 0 && args[1][0] != '\0' && args[2][0] != '\0') {
            read_offset(pid, port, (vm_address_t) strtol(args[1], NULL, 0), (vm_address_t) strtol(args[2], NULL, 0)); }
        else if (strcmp(args[0], "readoffset") == 0 && args[1][0] != '\0' && args[2][0] == '\0') {
            printf("[!] Error! Not enough arguments for readoffset!\n"); }
        else if (strcmp(args[0], "readoffset\n") == 0) {
            printf("[!] Error! Not enough arguments for readoffset!\n"); }

        //UNKNOWN COMMAND
        else { printf("[!] Error! Unknown command\n"); }
    }
}


int main() {
    pid_t pid;
    mach_port_t port;
    
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
        printf("[+] Obtained task_for_pid! - pid:%d\n", pid); }
    
    interact(pid, port);
    return 0;
}
