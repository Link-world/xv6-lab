// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define HASHNUM 13

struct {
  struct spinlock lock[HASHNUM];
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head[HASHNUM];
} bcache;

int
hash(uint num)
{
  return num % HASHNUM;
}

void
binit(void)
{
  struct buf *b;
  int i;
  for (i = 0; i < HASHNUM; i++)
  {
    initlock(&(bcache.lock[i]), "bcache");//initialize the locks
  

    // Create linked list of buffers
    bcache.head[i].prev = &(bcache.head[i]);
    bcache.head[i].next = &(bcache.head[i]);
  }
  
  i = 0;
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.head[i].next;
    b->prev = &bcache.head[i];
    initsleeplock(&b->lock, "buffer");
    bcache.head[i].next->prev = b;
    bcache.head[i].next = b;
    i = (i + 1) % HASHNUM;
  }//initialize the bcaches
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int i = hash(blockno);
  int j, k;
  acquire(&bcache.lock[i]);
  
  // Is the block already cached?
  for(b = bcache.head[i].next; b != &bcache.head[i]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[i]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for(b = bcache.head[i].prev; b != &bcache.head[i]; b = b->prev){
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache.lock[i]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  
  //When no unused buffer,look for buffer in other linked lists
  for(k = 1; k < HASHNUM; k++)
  {
    j = (i+k) % HASHNUM;
    acquire(&bcache.lock[j]);
    for(b = bcache.head[j].prev; b != &bcache.head[j]; b = b->prev){
      if(b->refcnt == 0) {
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;
        b->prev->next = b->next;
        b->next->prev = b->prev;
        release(&bcache.lock[j]);

        
        b->next = bcache.head[i].next;
        b->prev = &bcache.head[i];
        bcache.head[i].next->prev = b;
        bcache.head[i].next = b;
        release(&bcache.lock[i]);

        acquiresleep(&b->lock);
        return b;
        }
      }
    release(&bcache.lock[j]);
  }
  release(&bcache.lock[i]);
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  int i = hash(b->blockno);
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  acquire(&bcache.lock[i]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head[i].next;
    b->prev = &bcache.head[i];
    bcache.head[i].next->prev = b;
    bcache.head[i].next = b;
  }
  
  release(&bcache.lock[i]);
}

void
bpin(struct buf *b) {
  int i = hash(b->blockno);
  acquire(&bcache.lock[i]);
  b->refcnt++;
  release(&bcache.lock[i]);
}

void
bunpin(struct buf *b) {
  int i = hash(b->blockno);
  acquire(&bcache.lock[i]);
  b->refcnt--;
  release(&bcache.lock[i]);
}


