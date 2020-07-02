/*
 sign with these entitlements
 ldid -Sent.xml sdbg
 -Sent.xml is not a typo - there's no space

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

#include "mem.h"

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
        
        //HELP
        if (strcmp(args[0], "help\n") == 0) {
            printf("[!] List of commands:\n"
                   YELLOW "write " GREEN "0x[address] 0x[data] " WHITE "- writes " GREEN "0x[data] " WHITE "to " GREEN "0x[address]\n"
                   YELLOW "writeoffset " GREEN "0x[offset] 0x[data] " WHITE "- writes " GREEN "0x[data] " WHITE "to " GREEN "0x[offset] " WHITE "+ slide\n"
                   YELLOW "read " GREEN "0x[address] [bytes] " WHITE "- reads " GREEN "[bytes] " WHITE "from " GREEN "0x[address]\n"
                   YELLOW "readoffset " GREEN "0x[offset] [bytes] " WHITE "- reads " GREEN "[bytes] " WHITE "from " GREEN "0x[offset] " WHITE "+ slide\n"
                   YELLOW "readlines " GREEN "0x[address] [lines] " WHITE "- reads " GREEN "[lines] " WHITE "of memory from " GREEN "0x[address]\n"
                   YELLOW "slide " WHITE "- gets current slide as address\n"
                   YELLOW "protect " GREEN "0x[address] [bytes] " WHITE "- sets R|W|X permissions at " GREEN "0x[address] " WHITE "for " GREEN"[bytes]\n"
                   YELLOW "pid " GREEN "[pid] " WHITE "- changes pid to new [pid]\n"
#ifdef __arm64
                   YELLOW "regread " GREEN "[register] " WHITE "- reads value of " GREEN "[register] " WHITE "- pass \"all\" to get all registers\n"
                   YELLOW "regwrite " GREEN "[register] 0x[value] " WHITE "- writes " GREEN "[register] " WHITE "with " GREEN "0x[value]\n"
#endif
                   YELLOW "exit " WHITE "- self explanatory\n"); }
    
        
        //EXIT
        else if (strcmp(args[0], "exit\n") == 0) {
            printf("[!] Exiting SDBG...\n"); exit(0); }
        else if (strcmp(args[0], "quit\n") == 0) {
            printf("[!] Exiting SDBG...\n"); exit(0); }
        
        //get slide
        else if (strcmp(args[0], "slide\n") == 0) {
            printf(GREEN "0x%lx " WHITE "\n", (vm_address_t) _dyld_get_image_vmaddr_slide(0)); }
        
        else if (strcmp(args[0], "pid\n") == 0) {
            printf(GREEN "%d" WHITE "\n", pid); }
        else if (strcmp(args[0], "pid") == 0 && args[1][0] != '\0') {
            pid = (int) strtol(args[1], NULL, 0); printf("[!] PID changed to %d\n", pid); }
        
        //set_region_protection (3 args)
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
        
        //read_memory (3 args)
        else if (strcmp(args[0], "readlines") == 0 && args[1][0] != '\0' && args[2][0] != '\0') {
            read_lines(pid, port, args[1], (int) strtol(args[2], NULL, 0)); }
        else if (strcmp(args[0], "readlines") == 0 && args[1][0] != '\0' && args[2][0] == '\0') {
            printf("[!] Error! Not enough arguments for readlines!\n"); }
        else if (strcmp(args[0], "readlines\n") == 0) {
            printf("[!] Error! Not enough arguments for readlines!\n"); }
        
#ifdef __arm64
                
        //register_read (2 args)
        else if (strcmp(args[0], "regread") == 0 && args[1][0] != '\0') {
            register_read(pid, port, args[1]); }
        else if (strcmp(args[0], "regread\n") == 0) {
            printf("[!] Error! Not enough arguments for regread!\n"); }
                
        //register_write (3 args)
        else if (strcmp(args[0], "regwrite") == 0 && args[1][0] != '\0' && args[2][0] != '\0') {
            register_write(pid, port, args[1], (vm_address_t) strtol(args[2], NULL, 0)); }
        else if (strcmp(args[0], "regwrite") == 0 && args[1][0] != '\0' && args[2][0] == '\0') {
            printf("[!] Error! Not enough arguments for regwrite!\n"); }
        else if (strcmp(args[0], "regwrite\n") == 0) {
            printf("[!] Error! Not enough arguments for regwrite!\n"); }
                    
#endif

        //UNKNOWN COMMAND
        else { printf("[!] Error! Unknown command\n"); }
    }
}


int main() {
    pid_t pid;
    mach_port_t port;
    
    printf("[!] Welcome to SDBG!\n");

    //check if root
    if (geteuid() && getuid()) {
        printf("[!] Error! Run SDBG as root.\n[!] Do you have the proper entitlements?\n[!] Exiting SDBG...\n");
        exit(0);
    }

    //get pid to attach
    printf("[!] PID to attach: ");
    scanf("%d", &pid);
    printf("[+] Attaching to pid: %d\n", pid);
    getchar();

    //task_for_pid
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &port);
    if (kret != KERN_SUCCESS) {
        printf("[!] Error! Couldn't obtain task_for_pid: %d\n[!] Do you have the proper entitlements?\n[!] Exiting SDBG...\n", pid);
        exit(0);
    }
    else {
        printf("[+] Obtained task_for_pid!\n"); }
    
    interact(pid, port);
    return 0;
}
