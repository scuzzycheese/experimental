#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h>

#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/uio.h>
#include <linux/smp_lock.h>
#include <linux/fsnotify.h>
#include <linux/security.h>
#include <linux/syscalls.h>
#include <linux/pagemap.h>
#include <linux/compat.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/net.h>
#include <linux/sockios.h>
#include <linux/netdevice.h>
#include <linux/vmalloc.h>
#include <net/sock.h>

#include <linux/sched.h>
#include <asm/uaccess.h>


/**
 * netlink stuff
 */

struct sock *netSock = NULL;

/**
 * this is just temporary, but I'll write code to scan memory
 * and find the syscall table
 */
void **sys_call_table = (void *)0xc02fc540;

static int uid;
module_param(uid, int, 0644);

asmlinkage int (*original_call) (const char *, int, int);

void netDataArrv(struct sock *sk, int len)
{
	struct sk_buff *skb;
	struct nlmsghdr *nlh = NULL;
	u8 *payload = NULL;
	
	while((skb = skb_dequeue(&sk->sk_receive_queue)) != NULL)
	{
		nlh = (struct nlmsghdr *)skb->data;
		payload = NLMSG_DATA(nlh);
	}
}

asmlinkage int our_sys_open(const char *filename, int flags, int mode)
{
	int i = 0;
	char ch;

	if(uid == current->uid)
	{
		if(strcmp(filename, "test.af") == 0)
		{
			netSock = netlink_kernel_create(17, 1, netDataArrv, NULL, THIS_MODULE);
		}

		printk("Opened file by %d: ", uid);
		do
		{
			get_user(ch, filename + i);
			i++;
			printk("%c", ch);
		} while (ch != 0);
		printk("\n");
	}

	return original_call(filename, flags, mode);
}

static inline loff_t file_pos_read(struct file *file)
{
	return file->f_pos;
}

static inline void file_pos_write(struct file *file, loff_t pos)
{
	file->f_pos = pos;
}


struct file fastcall *fget_light(unsigned int fd, int *fput_needed)
{
	struct file *file;
	struct files_struct *files = current->files;

	*fput_needed = 0;
	if(likely((atomic_read(&files->count) == 1)))
	{
		file = fcheck_files(files, fd);
	}
	else
	{
		rcu_read_lock();
		file = fcheck_files(files, fd);
		if(file)
		{
			if (atomic_inc_not_zero(&file->f_count)) *fput_needed = 1;
			else file = NULL;
		}
		rcu_read_unlock();
	}
	return file;
}


asmlinkage int (*sys_execve_old)(struct pt_regs regs);
asmlinkage int sys_execve_new(struct pt_regs regs)
{
	if(strcmp((const char *)regs.ebx, "/home/scuzzy/devel/temp/activeFiles/test.af") == 0)
	{
		printk(KERN_ALERT "FILE EXECUTED: %s\n", (const char *)regs.ebx);
		char **argv = (char **)regs.ecx;
		while(*argv)
		{
			printk(KERN_ALERT "\tARG: %s\n", *argv);
			argv ++;
		}
		char **env = (char **)regs.edx;
		while(*env)
		{
			printk(KERN_ALERT "\tENV: %s\n", *env);
			env ++;
		}
	}
	return sys_execve_old(regs);
}


asmlinkage ssize_t (*sys_read_old)(unsigned int fd, char __user * buf, size_t count);
asmlinkage ssize_t sys_read_new(unsigned int fd, char __user * buf, size_t count)
{
	struct file *file;
	ssize_t ret = -EBADF;
	int fput_needed;

	file = fget_light(fd, &fput_needed);
	if(file)
	{


		//adding
		struct dentry *fdEnt = file->f_path.dentry;
		if(strcmp(fdEnt->d_name.name, "test.af") == 0)
		{
			int length = 0;
			//we loop through the nodes untill we equal our parents
			while(fdEnt != fdEnt->d_parent)
			{
				//printk(KERN_ALERT "PATH: %s\n", fdEnt->d_name.name);
				//We add an extra one for the /
				length += strlen(fdEnt->d_name.name) + 1;
				fdEnt = fdEnt->d_parent;
			}

			//allocate us some memory to hold this string
			char unsigned *filePath = (char *)vmalloc_32_user(length);
			char unsigned *filePathTemp = filePath + length;
			//Null the end of this string
			*filePathTemp = 0x00;
			//Reset and loop again
			fdEnt = file->f_path.dentry;
			while(fdEnt != fdEnt->d_parent)
			{
				filePathTemp -= strlen(fdEnt->d_name.name);
				memcpy(filePathTemp, fdEnt->d_name.name, strlen(fdEnt->d_name.name));
				filePathTemp --;
				memcpy(filePathTemp, "/", 1);
				fdEnt = fdEnt->d_parent;
			}
			printk(KERN_ALERT "PATH: %s\n", filePath);

			//YAY, I got this working!
			call_usermodehelper_keys(filePath, NULL, NULL, NULL, 0);
			
			/*
			//Allocate and set up args
			char **myArgs = (char **)vmalloc_32_user(sizeof(char *) * 1);
			myArgs[0] = (char *)vmalloc_32_user(strlen(file->f_path.dentry->d_name.name) + 1);
			memcpy(myArgs[0], file->f_path.dentry->d_name.name, strlen(file->f_path.dentry->d_name.name) + 1);

			//Soohrt says this should work, I dunno why it's not happening
			struct pt_regs myApp;
			//I probably don't have to memset, because vmalloc mashes everything to 0
			memset(&myApp, 0x00, sizeof(myApp));
			myApp.ebx = (unsigned long)filePath;
			myApp.ecx = (unsigned long)myArgs;
			int (*sys_execve)(struct pt_regs) = sys_call_table[__NR_execve];
			sys_execve(myApp);

			//Better free this!
			vfree(filePath);
			//these are just temporary
			vfree(myArgs[0]);
			vfree(myArgs);
			*/
				
			//We add an extra one for the NUL
			length ++;
		}
		//finished adding
		

		loff_t pos = file_pos_read(file);
		ret = vfs_read(file, buf, count, &pos);
		file_pos_write(file, pos);
		fput_light(file, fput_needed);
	}

	return ret;
}

int init_module()
{
	printk(KERN_ALERT "I'm dangerous. I hope you did a ");
	printk(KERN_ALERT "sync before you insmod'ed me.\n");
	printk(KERN_ALERT "My counterpart, cleanup_module(), is even");
	printk(KERN_ALERT "more dangerous. If\n");
	printk(KERN_ALERT "you value your file system, it will ");
	printk(KERN_ALERT "be \"sync; rmmod\" \n");
	printk(KERN_ALERT "when you remove this module.\n");

	original_call = sys_call_table[__NR_open];
	sys_call_table[__NR_open] = our_sys_open;

	sys_read_old = sys_call_table[__NR_read];
	sys_call_table[__NR_read] = sys_read_new;

//	sys_execve_old = sys_call_table[__NR_execve];
//	sys_call_table[__NR_execve] = sys_execve_new;

	printk(KERN_INFO "Spying on UID:%d\n", uid);

	return 0;
}

void cleanup_module()
{
	if(sys_call_table[__NR_open] != our_sys_open)
	{
		printk(KERN_ALERT "Somebody else also played with the ");
		printk(KERN_ALERT "open system call\n");
		printk(KERN_ALERT "The system may be left in ");
		printk(KERN_ALERT "an unstable state.\n");
	}

	//It is VERY important when you unload your module, to replace the syscalls
	//with their origional counterpart
	sys_call_table[__NR_open] = original_call;
	sys_call_table[__NR_read] = sys_read_old;
//	sys_call_table[__NR_execve] = sys_execve_old;
}
