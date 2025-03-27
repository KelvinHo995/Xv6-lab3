#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  uint64 start_va; // địa chỉ ảo bắt đầu
  int num_pages;   // số trang cần quét
  uint64 mask_ptr; // địa chỉ user để lưu bitmask

  argaddr(0, &start_va);
  argint(1, &num_pages);
  argaddr(2, &mask_ptr);

  struct proc *p = myproc();
  pagetable_t pagetable = p->pagetable;
  if (!pagetable) return -1;

  // Tạo bitmask
  uint32 mask = 0;

  for(int i = 0; i < num_pages; i++){
    uint64 va = start_va + i*PGSIZE;

    // Tìm pte tương ứng
    pte_t *pte = walk(pagetable, va, 0);
    if (pte == 0) {
      // Khong co PTE, khong set bit
      continue;
    }

    // Kiểm tra PTE có valid không và bit A có set không
    if((*pte & PTE_V) && (*pte & PTE_A)) {
      // Set bit thứ i
      mask |= (1 << i);

      // Xóa bit PTE_A
      *pte &= ~PTE_A;
    }
  }

  // Sao chép bitmask về user space
  copyout(pagetable, mask_ptr, (char *)&mask, sizeof(mask));
 
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
