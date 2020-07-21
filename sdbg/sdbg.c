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
void interact(pid_t pid, mach_port_t task) {
    char input[128];
    char args[10][30];
    
    printf(GOOD"Entering CLI... for a list of commands, type 'help'\n");
    
    while (1) {
    
        memset(args, 0, sizeof(args[0][0])*10*30);
    
        printf(CYAN">> " WHITE);
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
            printf(GOOD"List of commands:\n"
                   MAGENTA "write (w) " GREEN "0x[address] 0x[data] " WHITE "- writes " GREEN "0x[data] " WHITE "to " GREEN "0x[address]\n"
                   MAGENTA "writeoffset (wo) " GREEN "0x[offset] 0x[data] " WHITE "- writes " GREEN "0x[data] " WHITE "to " GREEN "0x[offset] " WHITE "+ slide\n"
                   MAGENTA "read (r) " GREEN "0x[address] [bytes] " WHITE "- reads " GREEN "[bytes] " WHITE "from " GREEN "0x[address]\n"
                   MAGENTA "readoffset (ro) " GREEN "0x[offset] [bytes] " WHITE "- reads " GREEN "[bytes] " WHITE "from " GREEN "0x[offset] " WHITE "+ slide\n"
                   MAGENTA "readlines (rl) " GREEN "0x[address] [lines] " WHITE "- reads " GREEN "[lines] " WHITE "of memory from " GREEN "0x[address]\n"
                   MAGENTA "readlineschar (rlc) " GREEN "0x[address] [lines] " WHITE "- reads " GREEN "[lines] " WHITE "of memory from " GREEN "0x[address] as chars\n"
                   MAGENTA "slide (s) " WHITE "- gets current slide as address\n"
                   MAGENTA "protect (pr) " GREEN "0x[address] [bytes] " WHITE "- sets R|W|X permissions at " GREEN "0x[address] " WHITE "for " GREEN"[bytes]\n"
                   MAGENTA "pid " GREEN "[pid] " WHITE "- changes pid to new [pid]\n"
                   MAGENTA "pause (p) " WHITE "- pauses task\n"
                   MAGENTA "resume (re) " WHITE "- resumes task\n"
#ifdef __arm64
                   MAGENTA "regread (rr) " GREEN "[register] " WHITE "- reads value of " GREEN "[register] " WHITE "- pass \"all\" to get all registers\n"
                   MAGENTA "regwrite (rw) " GREEN "[register] 0x[value] " WHITE "- writes " GREEN "[register] " WHITE "with " GREEN "0x[value]\n"
#endif
                   MAGENTA "exit (e) / quit (q) " WHITE "- self explanatory\n"); }
    
        
        //EXIT
        else if (strcmp(args[0], "exit\n") == 0 || strcmp(args[0], "e\n") == 0) {
            EXIT }
        else if (strcmp(args[0], "quit\n") == 0 || strcmp(args[0], "q\n") == 0) {
            EXIT }
        
        else if (strcmp(args[0], "\n") == 0) { ; }
        
        //pause
        else if (strcmp(args[0], "pause\n") == 0 || strcmp(args[0], "p\n") == 0) {
            printf(GOOD"Pausing task...\n"); task_suspend(task); }
        
        //resume
        else if (strcmp(args[0], "resume\n") == 0 || strcmp(args[0], "re\n") == 0) {
            printf(GOOD"Resuming task...\n"); task_resume(task); }
        
        //get slide
        else if (strcmp(args[0], "slide\n") == 0 || strcmp(args[0], "s\n") == 0) {
            printf(GREEN "0x%lx " WHITE "\n", (vm_address_t) _dyld_get_image_vmaddr_slide(0)); }
        
        else if (strcmp(args[0], "pid\n") == 0) {
            printf(GREEN "%d" WHITE "\n", pid); }
        else if (strcmp(args[0], "pid") == 0 && args[1][0] != '\0') {
            pid = (int) strtol(args[1], NULL, 0);
            printf(GOOD "PID changed to %d\n", pid);
            kern_return_t changetfp = task_for_pid(mach_task_self(), pid, &task);
            if (changetfp == KERN_SUCCESS) {
                printf(GOOD"Changed pid to %d\n", pid);
            }
            else {
                printf(ERROR"Could not change PID\n");
            }
        }
        
        //set_region_protection (3 args)
        else if ((strcmp(args[0], "protect") == 0 || strcmp(args[0], "pr") == 0) && args[1][0] != '\0' && args[2][0] != '\0') {
            set_region_protection(task, (vm_address_t) strtol(args[1], NULL, 0), (vm_size_t) strtol(args[2], NULL, 0)); }
        else if ((strcmp(args[0], "protect") == 0 || strcmp(args[0], "pr") == 0) && args[1][0] != '\0' && args[2][0] == '\0') {
            printf(ERROR"Not enough arguments for protect!\n"); }
        else if (strcmp(args[0], "protect\n") == 0 || strcmp(args[0], "pr\n") == 0) {
            printf(ERROR"Not enough arguments for protect!\n"); }
        
        //write_memory (3 args)
        else if ((strcmp(args[0], "write") == 0 || strcmp(args[0], "w") == 0) && args[1][0] != '\0' && args[2][0] != '\0') {
            write_memory(task, (vm_address_t) strtol(args[1], NULL, 0), (vm_address_t) strtol(args[2], NULL, 0)); }
        else if ((strcmp(args[0], "write") == 0 || strcmp(args[0], "w") == 0) && args[1][0] != '\0' && args[2][0] == '\0') {
            printf(ERROR"Not enough arguments for write!\n"); }
        else if (strcmp(args[0], "write\n") == 0 || strcmp(args[0], "w\n") == 0) {
            printf(ERROR"Not enough arguments for write!\n"); }
        
        //write_offset (3 args)
        else if ((strcmp(args[0], "writeoffset") == 0 || strcmp(args[0], "wo") == 0) && args[1][0] != '\0' && args[2][0] != '\0') {
            write_offset(task, (vm_address_t) strtol(args[1], NULL, 0), (vm_address_t) strtol(args[2], NULL, 0)); }
        else if ((strcmp(args[0], "writeoffset") == 0 || strcmp(args[0], "wo") == 0) && args[1][0] != '\0' && args[2][0] == '\0') {
            printf(ERROR"Not enough arguments for writeoffset!\n"); }
        else if (strcmp(args[0], "writeoffset\n") == 0 || strcmp(args[0], "wo\n") == 0) {
            printf(ERROR"Not enough arguments for writeoffset!\n"); }
        
        //read_memory (3 args)
        else if ((strcmp(args[0], "read") == 0 || strcmp(args[0], "r") == 0) && args[1][0] != '\0' && args[2][0] != '\0') {
            read_memory(task, (vm_address_t) strtol(args[1], NULL, 0), (vm_address_t) strtol(args[2], NULL, 0)); }
        else if ((strcmp(args[0], "read") == 0 || strcmp(args[0], "r") == 0) && args[1][0] != '\0' && args[2][0] == '\0') {
            printf(ERROR"Not enough arguments for read!\n"); }
        else if (strcmp(args[0], "read\n") == 0 || strcmp(args[0], "r\n") == 0) {
            printf(ERROR"Not enough arguments for read!\n"); }
        
        //read_offset (3 args)
        else if ((strcmp(args[0], "readoffset") == 0 || strcmp(args[0], "ro") == 0) && args[1][0] != '\0' && args[2][0] != '\0') {
            read_offset(task, (vm_address_t) strtol(args[1], NULL, 0), (vm_address_t) strtol(args[2], NULL, 0)); }
        else if ((strcmp(args[0], "readoffset") == 0 || strcmp(args[0], "ro") == 0) && args[1][0] != '\0' && args[2][0] == '\0') {
            printf(ERROR"Not enough arguments for readoffset!\n"); }
        else if (strcmp(args[0], "readoffset\n") == 0 || strcmp(args[0], "ro\n") == 0) {
            printf(ERROR"Not enough arguments for readoffset!\n"); }
        
        //read_lines (3 args)
        else if ((strcmp(args[0], "readlines") == 0 || strcmp(args[0], "rl") == 0) && args[1][0] != '\0' && args[2][0] != '\0') {
            read_lines(task, args[1], (int) strtol(args[2], NULL, 0), false); }
        else if ((strcmp(args[0], "readlines") == 0 || strcmp(args[0], "rl") == 0) && args[1][0] != '\0' && args[2][0] == '\0') {
            printf(ERROR"Not enough arguments for readlines!\n"); }
        else if (strcmp(args[0], "readlines\n") == 0 || strcmp(args[0], "rl\n") == 0) {
            printf(ERROR"Not enough arguments for readlines!\n"); }
        
        //read_lines (3 args)
        else if ((strcmp(args[0], "readlineschar") == 0 || strcmp(args[0], "rlc") == 0) && args[1][0] != '\0' && args[2][0] != '\0') {
            read_lines(task, args[1], (int) strtol(args[2], NULL, 0), true); }
        else if ((strcmp(args[0], "readlineschar") == 0 || strcmp(args[0], "rlc") == 0) && args[1][0] != '\0' && args[2][0] == '\0') {
            printf(ERROR"Not enough arguments for readlines!\n"); }
        else if (strcmp(args[0], "readlineschar\n") == 0 || strcmp(args[0], "rlc\n") == 0) {
            printf(ERROR"Not enough arguments for readlines!\n"); }
        
#ifdef __arm64
                
        //register_read (2 args)
        else if ((strcmp(args[0], "regread") == 0 || strcmp(args[0], "rr") == 0) && args[1][0] != '\0') {
            register_read(task, args[1]); }
        else if (strcmp(args[0], "regread\n") == 0 || strcmp(args[0], "rr\n") == 0) {
            printf(ERROR"Not enough arguments for regread!\n"); }
                
        //register_write (3 args)
        else if ((strcmp(args[0], "regwrite") == 0 || strcmp(args[0], "rw") == 0) && args[1][0] != '\0' && args[2][0] != '\0') {
            register_write(task, args[1], (uint64_t) strtol(args[2], NULL, 0)); }
        else if ((strcmp(args[0], "regwrite") == 0 || strcmp(args[0], "rw") == 0) && args[1][0] != '\0' && args[2][0] == '\0') {
            printf(ERROR"Not enough arguments for regwrite!\n"); }
        else if (strcmp(args[0], "regwrite\n") == 0 || strcmp(args[0], "rw\n") == 0) {
            printf(ERROR"Not enough arguments for regwrite!\n"); }
                    
#endif

        //UNKNOWN COMMAND
        else { printf(ERROR"Unknown command\n"); }
    }
}


int main() {
    pid_t pid;
    mach_port_t task;
    
    printf(YELLOW" # Welcome to SDBG!\n");

    //check if root
    if (geteuid() && getuid()) {
        printf(ERROR"Run SDBG as root.\n");
        EXIT
    }

    //get pid to attach
    printf(GOOD"PID to attach: ");
    scanf("%d", &pid);
    getchar();

    //task_for_pid
    kern_return_t kret = task_for_pid(mach_task_self(), pid, &task);
    if (kret != KERN_SUCCESS) {
        printf(ERROR"Couldn't obtain task_for_pid(%d)\n", pid);
        printf(ERROR"Do you have proper entitlements?\n");
        EXIT
    }
    else {
        printf(GOOD"Obtained task_for_pid(%d)\n", pid); }
    
    interact(pid, task);
    return 0;
}
