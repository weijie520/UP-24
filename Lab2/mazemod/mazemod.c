/*
 * Lab problem set for UNIX programming course
 * by Chun-Ying Huang <chuang@cs.nctu.edu.tw>
 * License: GPLv2
 */
#include <linux/module.h>	// included for all kernel modules
#include <linux/kernel.h>	// included for KERN_INFO
#include <linux/init.h>		// included for __init and __exit macros
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/errno.h>
#include <linux/sched.h>	// task_struct requried for cur_uid()
#include <linux/cred.h>		// for cur_uid();
#include <linux/slab.h>		// for kmalloc/kfree
#include <linux/uaccess.h>	// copy_to_user
#include <linux/string.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/random.h>
#include "maze.h"

DEFINE_MUTEX(mylock);
DEFINE_MUTEX(numlock);

static dev_t devnum;
static struct cdev c_dev;
static struct class *clazz;

// static user_d *users[3];
static user_d *users_head = NULL;
static int cur = 0;

typedef struct Point{
  int prevx;
  int prevy;
  int x;
  int y;
}Point;

typedef struct Stack{
  Point point;
  struct Stack* next;
}Stack;

// void maze_init(maze_t *m, int **is_visited, int prevx, int prevy, int x, int y){
//   if(is_visited[y][x])
//     return;

//   m->blk[y][x] = 0;
//   is_visited[y][x] = 1;

//   m->blk[(y + prevy)/2][(x + prevx)/2] = 0;
//   is_visited[(y + prevy)/2][(x + prevx)/2] = 1;

//   int dx[] = {2,-2,0,0};
//   int dy[] = {0,0,2,-2};
//   int rand = get_random_u32() % 4;

//   for(int i = 0; i < 4; i++){
//     int nextx = x + dx[(i+rand)%4];
//     int nexty = y + dy[(i+rand)%4];
//     if(nextx > 0 && nextx < m->w-1 && nexty > 0 && nexty < m->h-1)
//       maze_init(m, is_visited, x, y, nextx, nexty);
//     else continue;
//   }
//   return;
// }

void push(Stack **stack, Stack* node){
  node->next = *stack;
  *stack = node;
}

Stack* pop(Stack **stack){
  Stack *top = *stack;
  if(!top)
    return NULL;
  *stack = top->next;
  return top;
}

void maze_create(maze_t *m, int **is_visited, Stack *stack, int top){
  int dx[] = {2,-2,0,0};
  int dy[] = {0,0,2,-2};

  while(stack){
    Stack *tmp = pop(&stack);
    top--;

    // if(!stack)
    //   printk(KERN_INFO "stack is empty now\n");
    // else printk(KERN_INFO "stack is not empty\n");
    // printk(KERN_INFO "top: %d\n", top);
    if(is_visited[tmp->point.y][tmp->point.x]){
      kfree(tmp);
      continue;
    }
    // printk(KERN_INFO "top: %d %d %d %d %d\n", top, tmp->point.x, tmp->point.y, tmp->point.prevx, tmp->point.prevy);
    m->blk[tmp->point.y][tmp->point.x] = 0;
    is_visited[tmp->point.y][tmp->point.x] = 1;

    m->blk[(tmp->point.y + tmp->point.prevy)/2][(tmp->point.x + tmp->point.prevx)/2] = 0;
    is_visited[(tmp->point.y + tmp->point.prevy)/2][(tmp->point.x + tmp->point.prevx)/2] = 1;

    int rand = get_random_u32() % 4;
    for(int i = 0; i < 4; i++){
      int nextx = tmp->point.x + dx[(i+rand)%4];
      int nexty = tmp->point.y + dy[(i+rand)%4];
      if(nextx > 0 && nextx < m->w-1 && nexty > 0 && nexty < m->h-1){
        Stack *s = kzalloc(sizeof(Stack), GFP_KERNEL);
        s->point.prevx = tmp->point.x;
        s->point.prevy = tmp->point.y;
        s->point.x = nextx;
        s->point.y = nexty;
        s->next = NULL;
        push(&stack, s);
        top++;
      }
      else continue;
    }
    kfree(tmp);
  }
}

void maze_init(maze_t *m, coord_t coord){
  m->w = coord.x;
  m->h = coord.y;
  // m->sx = 1;
  // m->sy = 1;
  // m->ex = coord.x-2;
  // m->ey = coord.y-2;
  int **is_visited = kzalloc(m->h*sizeof(int*), GFP_KERNEL);

  for(int i = 0; i < m->h; i++){
    is_visited[i] = kzalloc(m->w*sizeof(int), GFP_KERNEL);
    for(int j = 0; j < m->w; j++){
      // if(i == 0 || i == m->h-1 || j == 0 || j == m->w-1)
      is_visited[i][j] = 0;
      m->blk[i][j] = 1;
      // else m->blk[i][j] = 0;
    }
  }

  Stack *s = kzalloc(sizeof(Stack), GFP_KERNEL);
  s->point.prevx = 1;
  s->point.prevy = 1;
  s->point.x = 1;
  s->point.y = 1;
  s->next = NULL;
  maze_create(m, is_visited, s, 1);
  for(int i = 0; i < m->h; i++){
    kfree(is_visited[i]);
  }
  kfree(is_visited);

  do{
    m->sx = get_random_u32() % (m->w-2) + 1;
    m->sy = get_random_u32() % (m->h-2) + 1;
  }while(m->blk[m->sy][m->sx] == 1);

  do{
    m->ex = get_random_u32() % (m->w-2) + 1;
    m->ey = get_random_u32() % (m->h-2) + 1;
  }while((m->blk[m->ey][m->ex]==1) || ((m->ex==m->sx) && (m->ey==m->sy)));
  // kfree(s);
  // maze_init(m, is_visited, 1, 1, 1, 1);

}

void move(user_d *u, coord_t coord){
  int nextx = u->pos.x+coord.x;
  int nexty = u->pos.y+coord.y;
  if(u->maze->blk[nexty][nextx] != 1){
    u->pos.x = nextx;
    u->pos.y = nexty;
  }
}

user_d* search_front_user(user_d *head, int id){
  user_d *tmp = head;
  if(!tmp || tmp->id == id)
    return NULL;
  while(tmp->next_user){
    if(tmp->next_user->id == id){
      return tmp;
    }
    tmp = tmp->next_user;
  }
  return NULL;
}

user_d* search_user(user_d *head, int id){
  user_d *tmp = head;
  while(tmp){
    if(tmp->id == id){
      return tmp;
    }
    tmp = tmp->next_user;
  }
  return NULL;
}

static int mazemod_dev_open(struct inode *i, struct file *f) {
  user_d *newnode = kzalloc(sizeof(user_d), GFP_KERNEL);
  newnode->id = current->pid;
  newnode->maze = NULL;
  newnode->next_user = NULL;

  mutex_lock(&mylock);
  if(!users_head){
    users_head = newnode;
  }
  else{
    user_d *now = users_head;
    while(now->next_user){
      now = now->next_user;
    }
    now->next_user = newnode;
  }
  mutex_unlock(&mylock);
	return 0;
}

static int mazemod_dev_close(struct inode *i, struct file *f) {
  mutex_lock(&mylock);
  user_d* front = search_front_user(users_head, current->pid);
  // printk(KERN_INFO "%d\n", current->pid);
  user_d* now = search_user(users_head, current->pid);
  if(now && !front){
    if(now->maze){
      kfree(now->maze);
      now->maze = NULL;
      cur--;
    }
    users_head = now->next_user;
    kfree(now);
  }
  else if(now && front){
    front->next_user = now->next_user;
    if(now->maze){
      kfree(now->maze);
      now->maze = NULL;
      cur--;
    }
    kfree(now);
    now = NULL;
  }
  mutex_unlock(&mylock);
	return 0;
}

static ssize_t mazemod_dev_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
  int bytes_read = 0;
  mutex_lock(&mylock);
  user_d *now = search_user(users_head, current->pid);
  mutex_unlock(&mylock);
  if(!now->maze)
    return -EBADFD;

  for(int i = 0; i < now->maze->h; i++){
    if(copy_to_user(buf + bytes_read, now->maze->blk[i], now->maze->w) != 0)
      return -EBUSY;
    bytes_read += now->maze->w;
  }
	return bytes_read;
}

static ssize_t mazemod_dev_write(struct file *f, const char __user *buf, size_t len, loff_t *off) {

  mutex_lock(&mylock);
  user_d* now = search_user(users_head, current->pid);
  mutex_unlock(&mylock);
  if(!now->maze)
    return -EBADFD;

  if(len%sizeof(coord_t) != 0)
    return -EINVAL;

  coord_t *coords = NULL;
  int steps = len/sizeof(coord_t);
  coords = kzalloc(steps*sizeof(coord_t), GFP_KERNEL);

  if (copy_from_user(coords, buf, len) != 0){
    kfree(coords);
    return -EBUSY;
  }

  for(int i = 0; i < steps; i++){
    move(now, coords[i]);
  }

	return len;
}

static long mazemod_dev_ioctl(struct file *fp, unsigned int cmd, unsigned long arg) {
  coord_t coord;
  mutex_lock(&mylock);
  user_d* now = search_user(users_head, current->pid);
  mutex_unlock(&mylock);

  switch(cmd){
    case MAZE_CREATE:
      if (copy_from_user(&coord, (coord_t __user *)arg, sizeof(coord_t)) != 0)
        return -EBUSY;
      if(coord.x < 0 || coord.y < 0)
        return -EINVAL;
      if(now->maze) // already exist
        return -EEXIST;
      mutex_lock(&mylock);
        if(cur < _MAZE_MAXUSER)
          cur++;
        else{
          mutex_unlock(&mylock);
          return -ENOMEM;
        }

      now->maze = kzalloc(sizeof(maze_t), GFP_KERNEL);
      maze_init(now->maze, coord);
      mutex_unlock(&mylock);
      now->pos.x = now->maze->sx;
      now->pos.y = now->maze->sy;
      break;
    case MAZE_RESET:
      if(now->maze){
        now->pos.x = now->maze->sx;
        now->pos.y = now->maze->sy;
      }
      else return -ENOENT;
      break;
    case MAZE_DESTROY:
      if(now->maze){
        kfree(now->maze);
        now->maze = NULL;
        mutex_lock(&mylock);
        cur--;
        mutex_unlock(&mylock);
      }
      else return -ENOENT;
      break;
    case MAZE_GETSIZE:
      if(now->maze){
        coord.x = now->maze->w;
        coord.y = now->maze->h;
        if(copy_to_user((coord_t __user *)arg, &coord, sizeof(coord_t)) != 0){
          return -EBUSY;
        }
      }
      else return -ENOENT;
      break;
    case MAZE_MOVE:
      if(now->maze){
        if (copy_from_user(&coord, (coord_t __user *)arg, sizeof(coord_t)) != 0) {
          return -EBUSY;
        }
        move(now, coord);
      }
      else return -ENOENT;
      break;
    case MAZE_GETPOS:
      if(now->maze){
        coord.x = now->pos.x;
        coord.y = now->pos.y;
        if(copy_to_user((coord_t __user *)arg, &coord, sizeof(coord_t)) != 0){
          return -EBUSY;
        }
      }
      else return -ENOENT;
      break;
    case MAZE_GETSTART:
      if(now->maze){
        coord.x = now->maze->sx;
        coord.y = now->maze->sy;
        if(copy_to_user((coord_t __user *)arg, &coord, sizeof(coord_t)) != 0){
          return -EBUSY;
        }
      }
      else return -ENOENT;
      break;
    case MAZE_GETEND:
      if(now->maze){
        coord.x = now->maze->ex;
        coord.y = now->maze->ey;
        if(copy_to_user((coord_t __user *)arg, &coord, sizeof(coord_t)) != 0){
          return -EBUSY;
        }
      }
      else return -ENOENT;
      break;
  }
	return 0;
}

static const struct file_operations mazemod_dev_fops = {
	.owner = THIS_MODULE,
	.open = mazemod_dev_open,
	.read = mazemod_dev_read,
	.write = mazemod_dev_write,
	.unlocked_ioctl = mazemod_dev_ioctl,
	.release = mazemod_dev_close
};

static int mazemod_proc_read(struct seq_file *m, void *v) {
  mutex_lock(&mylock);
  int cnt = cur, extra = _MAZE_MAXUSER - cur;
  user_d *now = users_head;

  char index_buf[] = "#00: ";
  char line_buf[60];

  while(cnt--){
    if(now->maze){
      seq_printf(m, index_buf);
      snprintf(line_buf, 60, "pid %d - [%d x %d]: (%d, %d) -> (%d, %d) @ (%d, %d)\n",
                now->id, now->maze->w, now->maze->h,
                now->maze->sx, now->maze->sy, now->maze->ex,
                now->maze->ey, now->pos.x, now->pos.y);
      seq_printf(m, line_buf);
      for(int j = 0; j < now->maze->h; j++){
        snprintf(line_buf, 60, "- %03d: ", j);
        seq_printf(m, line_buf);
        for(int k = 0; k < now->maze->w; k++){
          if(now->maze->blk[j][k] == 1)
            seq_printf(m, "#");
          else{
            if(j == now->pos.y && k == now->pos.x)
              seq_printf(m, "*");
            else if(j == now->maze->sy && k == now->maze->sx)
              seq_printf(m, "S");
            else if(j == now->maze->ey && k == now->maze->ex)
              seq_printf(m, "E");
            else seq_printf(m, ".");
          }
        }
        seq_printf(m, "\n");
      }
      seq_printf(m, "\n");
      index_buf[2]++;
    }
    if(now->next_user)
      now = now->next_user;
    else break;
  }
  mutex_unlock(&mylock);
  while(extra--){
    // printk(KERN_INFO "%d!!!\n", extra);
    seq_printf(m, index_buf);
    seq_printf(m, "vacancy\n\n");
    index_buf[2]++;
  }
	return 0;
}

static int mazemod_proc_open(struct inode *inode, struct file *file) {
	return single_open(file, mazemod_proc_read, NULL);
}

static const struct proc_ops mazemod_proc_fops = {
	.proc_open = mazemod_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

static char *mazemod_devnode(const struct device *dev, umode_t *mode) {
	if(mode == NULL) return NULL;
	*mode = 0666;
	return NULL;
}

static int __init mazemod_init(void)
{
	// create char dev
	if(alloc_chrdev_region(&devnum, 0, 1, "updev") < 0)
		return -1;
	if((clazz = class_create("upclass")) == NULL)
		goto release_region;
	clazz->devnode = mazemod_devnode;
	if(device_create(clazz, NULL, devnum, NULL, "maze") == NULL)
		goto release_class;
	cdev_init(&c_dev, &mazemod_dev_fops);
	if(cdev_add(&c_dev, devnum, 1) == -1)
		goto release_device;

	// create proc
	proc_create("maze", 0, NULL, &mazemod_proc_fops);

	printk(KERN_INFO "mazemod: initialized.\n");
	return 0;    // Non-zero return means that the module couldn't be loaded.

release_device:
	device_destroy(clazz, devnum);
release_class:
	class_destroy(clazz);
release_region:
	unregister_chrdev_region(devnum, 1);
	return -1;
}

static void __exit mazemod_cleanup(void)
{
	remove_proc_entry("maze", NULL);

	cdev_del(&c_dev);
	device_destroy(clazz, devnum);
	class_destroy(clazz);
	unregister_chrdev_region(devnum, 1);

	printk(KERN_INFO "mazemod: cleaned up.\n");
}

module_init(mazemod_init);
module_exit(mazemod_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wei-Jie Wu");
MODULE_DESCRIPTION("The unix programming course demo kernel module.");
