#include <iostream>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <elf.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <capstone/capstone.h>
#include <map>
using namespace std;

#define int3 0xcc

#define	PEEKSIZE	8
// char *text = NULL;
unsigned long text_section_end = 0;

class instruction1 {
public:
	unsigned char bytes[16];
	int size;
	string opr, opnd;
};

class breakpoint {
  public:
    unsigned long long addr;
    unsigned char data;
};

int brk_id = 0;
static map<int, breakpoint> breakpoints;

static csh cshandle = 0;
static map<long long, instruction1> instructions;

breakpoint* find_brk(unsigned long long addr){
  for(auto it = breakpoints.begin(); it != breakpoints.end(); it++){
    if(it->second.addr == addr){
      return &it->second;
    }
  }
  return NULL;
}

void errquit(const char *msg) {
	perror(msg);
	exit(-1);
}

void print_instruction(long long addr, instruction1 *in, const char *module) {
	int i;
	char bytes[128] = "";
	if(in == NULL) {
		fprintf(stderr, "%llx:\t<cannot disassemble>\n", addr);
	} else {
		for(i = 0; i < in->size; i++) {
      snprintf(&bytes[i*3], 4, "%2.2x ", in->bytes[i]);
		}
		fprintf(stderr, "\t%llx: %-32s\t%-10s%s\n", addr, bytes, in->opr.c_str(), in->opnd.c_str());
	}
}

static void brk_dumpcheck(char *text, unsigned long long rip, int size){
  for(auto it = breakpoints.begin(); it != breakpoints.end(); it++){
    if(it->second.addr >= rip && it->second.addr < rip+size){
      text[it->second.addr - rip]  = it->second.data;
    }
  }
}

void disassemble(pid_t proc, unsigned long long rip, const char *module){
  int count;
	char buf[64] = { 0 };
	unsigned long long ptr = rip;
	cs_insn *insn;
	map<long long, instruction1>::iterator mi; // from memory addr to instruction

  for(ptr = rip; ptr < rip + sizeof(buf); ptr += PEEKSIZE) {
		long long peek;
		errno = 0;
		peek = ptrace(PTRACE_PEEKTEXT, proc, ptr, NULL);
		if(errno != 0) break;
		memcpy(&buf[ptr-rip], &peek, PEEKSIZE);
	}

  brk_dumpcheck(buf, rip, sizeof(buf));

	if((count = cs_disasm(cshandle, (uint8_t*) buf, rip-ptr, rip, 5, &insn)) > 0) {
		int i;
		for(i = 0; i < count; i++) {
      if(insn[i].address >= text_section_end){
        printf("** the address is out of the range of the text section.\n");
        break;
      }
			instruction1 in;
			in.size = insn[i].size;
			in.opr  = insn[i].mnemonic;
			in.opnd = insn[i].op_str;
			memcpy(in.bytes, insn[i].bytes, insn[i].size);
      print_instruction(insn[i].address, &in, module);
		}
		cs_free(insn, count);
	}
}

int main(int argc, char *argv[]){

	setvbuf(stderr, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stdin,  NULL, _IONBF, 0);
  string load_program;
  string command;
  pid_t child;
  if(argc == 2){
    load_program = argv[1];
  }

  // load program
  while(load_program.empty()){
    cout << "(sdb) ";
    getline(cin, command);
    if(command.substr(0, 5) == "load "){
        load_program = command.substr(5);
    }else{
        cout << "** please load a program first.\n";
    }
  }


  if((child = fork()) == 0){
    if(ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) errquit("ptrace@child");
    execvp(load_program.c_str(), argv+1);
    errquit("execvp");
  }
  else{
    if(cs_open(CS_ARCH_X86, CS_MODE_64, &cshandle) != CS_ERR_OK)
			return -1;

    int fd = open(load_program.c_str(), O_RDONLY);
    Elf64_Ehdr ehdr;
    read(fd, &ehdr, sizeof(ehdr));
    if(memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0) errquit("not an ELF file");

    lseek(fd, ehdr.e_shoff, SEEK_SET); // move to the section header table
    Elf64_Shdr shdr[ehdr.e_shnum];
    read(fd, shdr, sizeof(shdr));
    // Seek to the section header string table
    if (lseek(fd, shdr[ehdr.e_shstrndx].sh_offset, SEEK_SET) < 0) errquit("lseek");

    // Read section header string table
    char shstrtab[shdr[ehdr.e_shstrndx].sh_size];
    read(fd, shstrtab, shdr[ehdr.e_shstrndx].sh_size);

    for (int i = 0; i < ehdr.e_shnum; i++) {
      if (strcmp(&shstrtab[shdr[i].sh_name], ".text") == 0) {
        text_section_end = shdr[i].sh_addr + shdr[i].sh_size;
        break;
      }
    }
    close(fd);

    int wait_status;
    struct user_regs_struct regs;
    uint64_t entry = ehdr.e_entry;
    breakpoint* reset_breakpoint = NULL;
    int enter = 0x01;
    unsigned long long sys_entry = 0;

    if(waitpid(child, &wait_status, 0) < 0) errquit("waitpid");
    ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_EXITKILL|PTRACE_O_TRACESYSGOOD);
    printf("** program '%s' loaded. entry point 0x%lx.\n", load_program.c_str(), entry);
    ptrace(PTRACE_GETREGS, child, 0, &regs);
    disassemble(child, regs.rip, "child");

    do{
      cout << "(sdb) ";
      getline(cin, command);
      if(command == "si"){
        if(ptrace(PTRACE_SINGLESTEP, child, 0, 0) < 0) errquit("si");
        if(waitpid(child, &wait_status, 0) < 0) errquit("waitpid");
        if(reset_breakpoint){
          long ret = ptrace(PTRACE_PEEKTEXT, child, reset_breakpoint->addr, 0);
          ptrace(PTRACE_POKETEXT, child, reset_breakpoint->addr, ((ret & ~(0xff)) | int3));
          reset_breakpoint = NULL;
        }
        if(!enter){
          ptrace(PTRACE_GETREGS, child, 0, &regs);
          if(sys_entry == regs.rip-2){
            enter ^= 0x01;
          }
        }
      }
      else if(command == "cont"){
        if(reset_breakpoint){
          ptrace(PTRACE_SINGLESTEP, child, 0, 0);
          waitpid(child, &wait_status, 0);
          long ret = ptrace(PTRACE_PEEKTEXT, child, reset_breakpoint->addr, 0);
          ptrace(PTRACE_POKETEXT, child, reset_breakpoint->addr, ((ret & ~(0xff)) | int3));
          reset_breakpoint = NULL;
        }
        if(!enter){
          ptrace(PTRACE_SYSCALL, child, 0, 0);
          waitpid(child, &wait_status, 0);
          enter ^= 0x01;
        }

        if(ptrace(PTRACE_CONT, child, 0, 0) < 0) errquit("ptrace@cont");
        if(waitpid(child, &wait_status, 0) < 0) errquit("waitpid");
      }
      else if(command == "info reg"){
        if(ptrace(PTRACE_GETREGS, child, 0, &regs) == 0){
          printf("$rax 0x%016llx\t$rbx 0x%016llx\t$rcx 0x%016llx\n", regs.rax, regs.rbx, regs.rcx);
          printf("$rdx 0x%016llx\t$rsi 0x%016llx\t$rdi 0x%016llx\n", regs.rdx, regs.rsi, regs.rdi);
          printf("$rbp 0x%016llx\t$rsp 0x%016llx\t$r8  0x%016llx\n", regs.rbp, regs.rsp, regs.r8);
          printf("$r9  0x%016llx\t$r10 0x%016llx\t$r11 0x%016llx\n", regs.r9, regs.r10, regs.r11);
          printf("$r12 0x%016llx\t$r13 0x%016llx\t$r14 0x%016llx\n", regs.r12, regs.r13, regs.r14);
          printf("$r15 0x%016llx\t$rip 0x%016llx\t$eflags 0x%016llx\n", regs.r15, regs.rip, regs.eflags);
        }
        continue;
      }
      else if(command.substr(0, 6) == "break "){
        // break
        unsigned long long addr = stoll(command.substr(6), 0, 16);
        breakpoint bp;
        bp.addr = addr;
        long ret = ptrace(PTRACE_PEEKTEXT, child, addr, 0);
        bp.data = ret & 0xff;
        ptrace(PTRACE_POKETEXT, child, addr, ((ret & ~(0xff)) | int3));
        breakpoints[brk_id++] = bp;
        printf("** set a breakpoint at 0x%llx.\n", addr);
        continue;
      }
      else if(command == "info break"){
        printf("Num\tAddress\n");
        for(auto it = breakpoints.begin(); it != breakpoints.end(); it++){
          printf("%d\t0x%llx\n", it->first, it->second.addr);
        }
        continue;
      }
      else if(command.substr(0, 7) == "delete "){
        // delete
        int num = stoi(command.substr(7));
        auto it = breakpoints.find(num);
        if(it != breakpoints.end()){
          long ret = ptrace(PTRACE_PEEKTEXT, child, it->second.addr, 0);
          ptrace(PTRACE_POKETEXT, child, it->second.addr, ((ret & ~(0xff)) | it->second.data));
          breakpoints.erase(it);
          printf("** delete breakpoint %d.\n", num);
        }
        else{
          printf("** breakpoint %d does not exist.\n", num);
        }
        continue;
      }
      else if(command.substr(0, 6) == "patch "){
        string data = command.substr(6);
        unsigned long long addr = stoll(strtok(&data[0], " "), 0, 16);
        long value = stoll(strtok(NULL, " "), 0, 16);
        int len = stoi(strtok(NULL, " "));

        for(int i = 0; i < len; i++){
          breakpoint *bp = find_brk(addr+i);
          if(bp)
            bp->data = (value >> (i*8)) & 0xff;
          else{
            long ret = ptrace(PTRACE_PEEKTEXT, child, addr+i, 0);
            ptrace(PTRACE_POKETEXT, child, addr+i, ((ret & ~(0xff)) | ((value >> (i*8)) & 0xff)));
          }
        }
        printf("** patch memory at address 0x%llx.\n", addr);
        continue;
      }
      else if(command == "syscall"){
        ptrace(PTRACE_SYSCALL, child, 0, 0);
        if(waitpid(child, &wait_status, 0) < 0) errquit("waitpid");
        if(reset_breakpoint){
          long ret = ptrace(PTRACE_PEEKTEXT, child, reset_breakpoint->addr, 0);
          ptrace(PTRACE_POKETEXT, child, reset_breakpoint->addr, ((ret & ~(0xff)) | int3));
          reset_breakpoint = NULL;
        }
      }
      else{
        cout << "** unknown command.\n";
        continue;
      }

      if(WIFSTOPPED(wait_status)){
        ptrace(PTRACE_GETREGS, child, 0, &regs);
        if(WSTOPSIG(wait_status) & 0x80){
          regs.rip -= 2;
          if(enter){
            sys_entry = regs.rip;
            printf("** enter a syscall(%d) at 0x%llx.\n", (int)regs.orig_rax, regs.rip);
          }else{
            printf("** leave a syscall(%d) = %d at 0x%llx.\n", (int)regs.orig_rax, (int)regs.rax, regs.rip);
          }
          enter ^= 0x01;
        }
        else{
          long brk = ptrace(PTRACE_PEEKTEXT, child, regs.rip - 1, 0);
          if(((unsigned char*)&brk)[0] == int3){ // cont
            regs.rip--;
            printf("** hit a breakpoint at 0x%llx.\n", regs.rip);
            // put the original data back
            reset_breakpoint = find_brk(regs.rip);
            unsigned char data = reset_breakpoint->data;
            long ret = ptrace(PTRACE_PEEKTEXT, child, regs.rip, 0);
            ptrace(PTRACE_POKETEXT, child, regs.rip, ((ret & ~(0xff)) | data));
            // put the new register value
            ptrace(PTRACE_SETREGS, child, 0, &regs);
          }
          else if(((unsigned char*)&brk)[1] == int3){ // si
            printf("** hit a breakpoint at 0x%llx.\n", regs.rip);
            // put the original data back
            reset_breakpoint = find_brk(regs.rip);
            unsigned char data = reset_breakpoint->data;
            long ret = ptrace(PTRACE_PEEKTEXT, child, regs.rip, 0);
            ptrace(PTRACE_POKETEXT, child, regs.rip, ((ret & ~(0xff)) | data));
            // put the new register value
            ptrace(PTRACE_SETREGS, child, 0, &regs);
          }
        }
        disassemble(child, regs.rip, "child");
      }
    }while(!WIFEXITED(wait_status));
    cout << "** the target program terminated.\n";
  }

  return 0;
}