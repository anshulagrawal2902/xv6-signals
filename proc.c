#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "signals.h"
struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);
extern int pause_chan;
extern int stop_chan;

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);
  curproc->parent->signalState.pendingSignals |= 1 << (31 - SIGCHLD);  // sends sigchld to parent of dead process

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;

  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      p->state = RUNNING;

      // Running loop for default handlers
      for (int i = 0; i < MAX_SIGNALS; i++)
      {
        if( (c->proc->signalState.pendingSignals & (1 << (31-i)))  == 1 << (31-i)  &&  
        (c->proc->signalState.blockedSignals | (0 << (31-i))) == 0 )
        {
          p->chan = 0;
          // if (c->proc->hasUserHandler[i] == 0)
          if( (c->proc->signalState.hasUserHandler | (0 << (31-i))) == 0)
          {
            // c->proc->pendingSignals[i] = 0;
            c->proc->signalState.pendingSignals &= ~(1 <<  (31-i));
            doDefaultSignal(i);
          }
        }
      }

      // Running loop for user handlers
      for (int i = 0; i < MAX_SIGNALS; i++)
      {
        if((c->proc->signalState.pendingSignals & (1 << (31-i))) == 1 << (31-i)  &&  
        (c->proc->signalState.blockedSignals | (0 << (31-i))) == 0 )
        {
          p->chan = 0;
          if( (c->proc->signalState.hasUserHandler & (1 << (31-i))) == 1 << (31-i))
          {
            c->proc->signalState.pendingSignals &= ~(1 <<  (31-i));

            // int* temp = (int*)p->tf->esp;
            // cprintf("user stack eip value %x\n" , p->tf->eip) ;
            p->tf->eip = (uint) c->proc->signalState.signalHandlers[i];
            // cprintf("user stack sp value %x\n" , p->tf->esp) ;
            // cprintf("user stack bp value %x\n" , p->tf->ebp) ;
            // // cprintf("value of temp %d \n", *temp);
            // cprintf("user stac %x\n" , (c->proc->signalHandlers[i]));
            // cprintf("user stac %x\n" , c->proc->signalHandlers[i] - 4) ;
            // cprintf("user stack sp value %x\n" , p->tf->ebp) ;
            // cprintf("user stack sp value %x\n" , p->tf->ebp) ;
            // cprintf("user stack sp value %x\n" , p->tf->ebp) ;
            // cprintf("user stack sp value %x\n" , p->tf->ebp) ;
            break;
          }
        }
      }

      if(p->state == SLEEPING || p->state == ZOMBIE ){
        continue;
      }

      switchuvm(p);

      swtch(&(c->scheduler), p->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
    release(&ptable.lock);
  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  if (p->chan != &pause_chan)
    p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

void dh_sigkill(int signo)
{
  cprintf("Running default sigkill\n");
  struct proc *curproc = myproc();
  curproc->killed = 1;
  wakeup1(curproc->parent);
  curproc->parent->signalState.pendingSignals |= 1 << (31 - SIGCHLD); //sent sigchld to parent of dead process

}

void dh_sigstop(int signo)
{
  struct proc *curproc = myproc();
  cprintf("Running default sigstop\n");
  curproc->state = SLEEPING;
  curproc->chan = &stop_chan; 
  curproc->parent->signalState.pendingSignals |= 1 << (31 - SIGCHLD); //sent sigchld to parent of stopped process
}

void dh_sigcont(int signo)
{
  struct proc *curproc = myproc();
  cprintf("Running default sigcont\n");
  if (curproc->chan == &stop_chan)
  {
    curproc->state = RUNNABLE;
    curproc->chan = 0;
  }
}

void dh_sigterm(int signo)
{
  //basically perform exit for a process without calling sched but returning to it
  release(&ptable.lock);
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);
  curproc->parent->signalState.pendingSignals |= 1 << (31 - SIGCHLD); //sent sigchld to parent of zombied process

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }
  curproc->state = ZOMBIE;
}

void dh_sigint(int signo)
{
  dh_sigterm(SIGTERM);
}

void dh_sigusr1(int){
 
}

void dh_sigsegv(int){
  dh_sigterm(SIGTERM);
}

void dh_sigchld(int){
}

void dh_sigill(int){
  cprintf("your process accessed a illegal memory location\n");
  dh_sigterm(SIGTERM);
}

void dh_sigvtalrm(int){

}



int pause(int *pause_chan)
{
  struct proc *curproc = myproc();
  acquire(&ptable.lock);
  curproc->chan = pause_chan;
  curproc->state = SLEEPING;
  while (curproc->chan == pause_chan)
    sleep(&pause_chan, &ptable.lock);
  release(&ptable.lock);
  return 0;
}

// send the process with given pid signal signum
int kill1(int pid, int signum)
{
  struct proc *p;
  acquire(&ptable.lock);
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  {
    if (p->pid == pid)
    {
      p->signalState.pendingSignals |= 1 << (31 - signum);   // adding signals to pending list
      // Wake process from sleep if necessary.
      if (p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

int signal(int signum, signalHandler fn)
{
  struct proc *curproc = myproc();
  if (fn == SIG_DFL)
  {
    // Restore the default signal handler
      curproc->signalState.hasUserHandler &= ~(1 << (31 - signum));
      return 0;
  }
  else if(signum == SIGKILL || signum == SIGSTOP || signum == SIGSEGV){
    return 0;
  }
  else
  {
    curproc->signalState.hasUserHandler |= 1 << (31 - signum);
    curproc->signalState.signalHandlers[signum] = fn;
  }
  return 0;
}

int sigprocmask(int how, struct sigset_t *set, struct sigset_t *oldset)
{
  struct proc *curproc = myproc();
  for (int i = 0; i < MAX_SIGNALS; i++)
  {
    if( (curproc->signalState.blockedSignals & (1 << (31 - i))) == (1 << (31 - i)) )
    {
      oldset->mask |= 1 << (31 - i);
    }
    else
    {
      oldset->mask &= ~(1 << (31 - i));
    }
  }
  if (how == SIG_BLOCK)
  {
    for (int i = 0; i < MAX_SIGNALS; i++)
    {
      if((set->mask & (1 << (31 - i))) == (1 << (31 - i)) )
      {
        curproc->signalState.blockedSignals |= (1 << (31 - i));
      }
    }
  }
  else if (how == SIG_UNBLOCK)
  {
    for (int i = 0; i < MAX_SIGNALS; i++)
    {
      if ((set->mask & (1 << (31 - i))) == (1 << (31 - i)))
      {
        curproc->signalState.blockedSignals &= ~(1 << (31 - i));
      }
    }
  }
  else if (how == SIG_SETMASK)
  {
    // TODO : behaviour of SIG_SETMASK
    for (int i = 0; i < MAX_SIGNALS; i++)
    {
      if ((oldset->mask & (1 << (31 - i))) == (1 << (31 - i)))
      {
        curproc->signalState.blockedSignals |= (1 << (31 - i));
      }
      else
      {
        curproc->signalState.blockedSignals &= ~(1 << (31 - i));
      }
    }
  }
  return 0;
}

int
procSigState(int pid, int bitmapNum, int signo){  //pid, bitmapNum, signo
  struct proc *p;
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  {
    if (p->pid == pid)
    {
      if(bitmapNum == 0 && (p->signalState.pendingSignals & (1<< (31 - signo))) == (1<< (31 - signo))){

        return 1;
      } 
      else if(bitmapNum == 1 && (p->signalState.blockedSignals & (1<< (31 - signo))) == (1<< (31 - signo))){

        return 1;
      }
      else if(bitmapNum == 2 && (p->signalState.blockedSignals & (1<< (31 - signo))) == (1<< (31 - signo))){

        return 1;
      }
      else return 0;
    }
  }
  return 0;
}

int
getitimer(int which, struct itimerval* value){
  return 0;
}

int
setitimer(int which, struct itimerval* value, struct itimerval* ovalue){
  return 0;
}
//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}
