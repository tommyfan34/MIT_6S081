// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"
#define PA2IDX(pa) (((uint64)pa) >> 12)

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

// struct to maintain the ref counts
struct {
  struct spinlock lock;
  int count[PGROUNDUP(PHYSTOP) / PGSIZE];
} refc;

void
refcinit()
{
  initlock(&refc.lock, "refc");
  for (int i = 0; i < PGROUNDUP(PHYSTOP) / PGSIZE; i++) {
    refc.count[i] = 0;
  }
}

void
refcinc(void *pa)
{
  acquire(&refc.lock);
  refc.count[PA2IDX(pa)]++;
  release(&refc.lock);
}

void
refcdec(void *pa)
{
  acquire(&refc.lock);
  refc.count[PA2IDX(pa)]--;
  release(&refc.lock);
}

int
getrefc(void *pa)
{
  return refc.count[PA2IDX(pa)];
}

void
kinit()
{
  refcinit();
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
  char *p;
  p = (char*)PGROUNDUP((uint64)end);
  for(; p + PGSIZE <= (char*)PHYSTOP; p += PGSIZE) {
    refcinc((void*)p);
  }
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
    kfree(p);
}
    
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");
  refcdec(pa);
  if (getrefc(pa) > 0) return;
  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r) {
    memset((char*)r, 5, PGSIZE); // fill with junk
    refcinc((void*)r);
  }
  return (void*)r;
}
