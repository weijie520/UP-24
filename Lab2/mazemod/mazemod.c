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
#include "maze.h"

DEFINE_MUTEX(mylock);

static dev_t devnum;
static struct cdev c_dev;
static struct class *clazz;

// static struct user_data {
//   int id;
//   coord_t pos;
//   maze_t* maze;
// }users[3];
static user_d *users[3];
// static maze_t *maze[3];
static int cur = 0;

// char buf[_MAZE_MAXX*_MAZE_MAXY];
void maze_create(maze_t *m, coord_t coord){
  m->w = coord.x;
  m->h = coord.y;
  m->sx = 1;
  m->sy = 1;
  m->ex = coord.x-2;
  m->ey = coord.y-2;
  for(int i = 0; i < m->h; i++){
    for(int j = 0; j < m->w; j++){
      if(i == 0 || i == m->h-1 || j == 0 || j == m->w-1)
        m->blk[i][j] = 1;
      else m->blk[i][j] = 0;
    }
  }
}

void move(user_d *u, coord_t coord){
  int nextx = u->pos.x+coord.x;
  int nexty = u->pos.y+coord.y;
  if(u->maze->blk[nexty][nextx] != 1){
    u->pos.x = nextx;
    u->pos.y = nexty;
  }
}

static int mazemod_dev_open(struct inode *i, struct file *f) {
	// printk(KERN_INFO "mazemod: device opened.\n");
  mutex_lock(&mylock);
    if(cur < _MAZE_MAXUSER){
      for(int i = 0; i < _MAZE_MAXUSER; i++){
        if(!users[i]){
          users[i] = kzalloc(sizeof(user_d), GFP_KERNEL);
          users[i]->id = current->pid;
          users[i]->maze = NULL;
          break;
        }
      }
      cur++;
    }
    else{
      mutex_unlock(&mylock);
      return -ENOMEM;
    }
  mutex_unlock(&mylock);
	return 0;
}

static int mazemod_dev_close(struct inode *i, struct file *f) {

  for(int i = 0; i < _MAZE_MAXUSER; i++){
    if(users[i] && users[i]->id == current->pid){
      if(users[i]->maze){
        kfree(users[i]->maze);
        users[i]->maze = NULL;
      }
      kfree(users[i]);
      users[i] = NULL;
      break;
    }
  }
  mutex_lock(&mylock);
  cur--;
  mutex_unlock(&mylock);
	// printk(KERN_INFO "mazemod: device closed.\n");
	return 0;
}

static ssize_t mazemod_dev_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
	// if(maze){
  //   // char buf[]
  //   // printk(KERN_INFO "vacancy");
  // }
  // printk(KERN_INFO "mazemod: read %zu bytes @ %llu.\n", len, *off);
	return len;
}

static ssize_t mazemod_dev_write(struct file *f, const char __user *buf, size_t len, loff_t *off) {
	// printk(KERN_INFO "mazemod: write %zu bytes @ %llu.\n", len, *off);
	return len;
}

static long mazemod_dev_ioctl(struct file *fp, unsigned int cmd, unsigned long arg) {
	// printk(KERN_INFO "mazemod: ioctl cmd=%u arg=%lu.\n", cmd, arg);
  // int flag = 0;
  int index = 0;
  coord_t coord;
  // mutex_lock(&mylock);
  for(int i = 0; i < _MAZE_MAXUSER; i++){
    if(users[i] && users[i]->id == current->pid){ // already exist
      index = i;
      // flag = 1;
      break;
    }
    // else if(users[i]->id == -1)
    //   index = index < i ? index : i;
  }
  // mutex_unlock(&mylock);
  switch(cmd){
    case MAZE_CREATE:
      if (copy_from_user(&coord, (coord_t __user *)arg, sizeof(coord_t)) != 0) {
        return -EBUSY;
      }
      if(coord.x < 0 || coord.y < 0)
        return -EINVAL;

      if(users[index]->maze) // already exist
        return -EEXIST;
      // index = 0;
      mutex_lock(&mylock);
        // for(int i = 0; i < _MAZE_MAXUSER; i++){
        //   if(!users[i]->maze){
        //     index = i;
        //     break;
        //   }
        // }
        // if(cur < _MAZE_MAXUSER){
          users[index]->id = current->pid;
          users[index]->maze = kzalloc(sizeof(maze_t), GFP_KERNEL);

          maze_create(users[index]->maze, coord);
          users[index]->pos.x = users[index]->maze->sx;
          users[index]->pos.y = users[index]->maze->sy;
          // flag = 1;
          // cur++;
        // }
        // else { // already 3 people
        //   mutex_unlock(&mylock);
        //   return -ENOMEM;
        // }
      mutex_unlock(&mylock);
      break;
    case MAZE_RESET:
      if(users[index]->maze){
        users[index]->pos.x = users[index]->maze->sx;
        users[index]->pos.y = users[index]->maze->sy;
      }
      else return -ENOENT;

      break;
    case MAZE_DESTROY:
      if(users[index]->maze){
        // users[index]->id = -1;
        kfree(users[index]->maze);
        users[index]->maze = NULL;
      }
      else return -ENOENT;

      break;
    case MAZE_GETSIZE:
      if(users[index]->maze){
        coord.x = users[index]->maze->w;
        coord.y = users[index]->maze->h;
        if(copy_to_user((coord_t __user *)arg, &coord, sizeof(coord_t)) != 0){
          return -EBUSY;
        }
      }
      else return -ENOENT;

      break;
    case MAZE_MOVE:
      if(users[index]->maze){
        if (copy_from_user(&coord, (coord_t __user *)arg, sizeof(coord_t)) != 0) {
          return -EBUSY;
        }
        move(users[index], coord);
      }
      else return -ENOENT;

      break;
    case MAZE_GETPOS:
      if(users[index]->maze){
        coord.x = users[index]->pos.x;
        coord.y = users[index]->pos.y;
        if(copy_to_user((coord_t __user *)arg, &coord, sizeof(coord_t)) != 0){
          return -EBUSY;
        }
      }
      else return -ENOENT;

      break;
    case MAZE_GETSTART:
      if(users[index]->maze){
        coord.x = users[index]->maze->sx;
        coord.y = users[index]->maze->sy;
        if(copy_to_user((coord_t __user *)arg, &coord, sizeof(coord_t)) != 0){
          return -EBUSY;
        }
      }
      else return -ENOENT;

      break;
    case MAZE_GETEND:
      if(users[index]->maze){
        coord.x = users[index]->maze->ex;
        coord.y = users[index]->maze->ey;
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
	char index_buf[] = "#00: ";
  for(int i = 0; i < _MAZE_MAXUSER; i++){
    seq_printf(m, index_buf);
    if(!users[i]){
      seq_printf(m, "vacancy\n");
    }
    else{
      char line_buf[60];
      snprintf(line_buf, 60, "pid %d - [%d x %d]: (%d, %d) -> (%d, %d) @ (%d, %d)\n",
                users[i]->id, users[i]->maze->w, users[i]->maze->h,
                users[i]->maze->sx, users[i]->maze->sy, users[i]->maze->ex,
                users[i]->maze->ey, users[i]->pos.x, users[i]->pos.y);
      seq_printf(m, line_buf);
      for(int j = 0; j < users[i]->maze->h; j++){
        snprintf(line_buf, 60, "- %03d: ", j);
        seq_printf(m, line_buf);
        for(int k = 0; k < users[i]->maze->w; k++){
          if(users[i]->maze->blk[j][k] == 1)
            seq_printf(m, "#");
          else{
            if(j == users[i]->pos.y && k == users[i]->pos.x)
              seq_printf(m, "*");
            else if(j == users[i]->maze->sy && k == users[i]->maze->sx)
              seq_printf(m, "S");
            else if(j == users[i]->maze->ey && k == users[i]->maze->ex)
              seq_printf(m, "E");
            else seq_printf(m, ".");
          }
        }
        seq_printf(m, "\n");
      }
      // continue;
    }
    seq_printf(m, "\n");
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
