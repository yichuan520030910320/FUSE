/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPLv2.
  See the file COPYING.
*/

/** @file
 *
 * minimal example filesystem using high-level API
 *
 * Compile with:
 *
 *     gcc -Wall rbtree.c my_chat.c `pkg-config fuse3 --cflags --libs` -o my_chat
 *
 * ## Source code ##
 * \include my_chat.c
 */


#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>

#include "rbtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define NUM_NODES 500

/*
 * Command line options
 *
 * We can't set default values for the char* fields here because
 * fuse_opt_parse would attempt to free() them when the user specifies
 * different values on the command line.
 */
struct options {
	const char *filename;
	const char *contents;
	int show_help;
} option[NUM_NODES];
struct options options;
#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--name=%s", filename),
	OPTION("--contents=%s", contents),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};


// option


//rbtree



#include "rbtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#include <assert.h>






struct rb_root root = RB_ROOT;



struct memfs_file {
    char *path;                     /* File path */
    void *data;                     /* File content *///maybe useless
	struct options* option;
    // u8 free_on_delete;
 
    // struct stat vstat;              /* File stat */
 
    pthread_mutex_t lock;
    pthread_cond_t write_finished;
 
    struct rb_node node;
};

static int __insert(struct rb_root *root, struct memfs_file *pf)
{
    struct rb_node **new = &(root->rb_node), *parent = NULL;
 
    /* Figure out where to put new node */
    while (*new) {
        struct memfs_file *this = container_of(*new, struct memfs_file, node);
        int result = strcmp(pf->path, this->path);
        parent = *new;
        if (result < 0) {
            new = &((*new)->rb_left);
        }
        else if (result > 0) {
            new = &((*new)->rb_right);
        }
        else {
            printf("%s is exist!!!\n", pf->path);
            return -1;
        }
    }
 
    /* Add new node and rebalance tree. */
    rb_link_node(&pf->node, parent, new);
    rb_insert_color(&pf->node, root);
    return 0;
}
static struct memfs_file *__search(struct rb_root *root, const char *path)
{
    struct memfs_file *pf = NULL;
    struct rb_node *node = root->rb_node;
 
    while (node) {
        pf = container_of(node, struct memfs_file, node);
 
        int result = strcmp(path, pf->path);
        if (result < 0) {
            node = node->rb_left;
        }
        else if (result > 0) {
            node = node->rb_right;
        }
        else {
            return pf;
        }
    }
 
//  printf("%s not found!!!\n", path);
    return NULL;
}
static inline void __free(struct memfs_file *pf)
{ {
        if (pf->data) {
            free(pf->data);
        }
        if (pf->path) {
            free(pf->path);
        }
        free(pf);
    }
}
 
static int __delete(struct rb_root *root, const char *path)
{
    struct memfs_file *pf = __search(root, path);
    if (!pf) {
        return -1;
    }   
    
    int blocks = 0;
    rb_erase(&pf->node, root);
   // __free(pf);
 
    return blocks;
} 






//the fuction in my_chat


//private



//interface

static void *my_chat_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	(void) conn;
	cfg->kernel_cache = 1;
	return NULL;
}

static int my_chat_truncate(const char *path, off_t size,
			 struct fuse_file_info *fi)
{
	(void) size;
	(void) fi;

	if(strcmp(path, "/") != 0)
		return -ENOENT;

	return 0;
}
static int my_chat_utimens(const char *path, const struct timespec ts[2],
		       struct fuse_file_info *fi)
{
	(void) fi;


	return 0;
}

static int my_chat_getattr(const char *path, struct stat *stbuf,
			 struct fuse_file_info *fi)
{
	
	
	// (void) fi;
	// int res = 0;

	// memset(stbuf, 0, sizeof(struct stat));
	// if (strcmp(path, "/") == 0) {
	// 	stbuf->st_mode = S_IFDIR | 0755;
	// 	stbuf->st_nlink = 2;
	// } else if (strcmp(path+1, options.filename) == 0) {
	// 	stbuf->st_mode = S_IFREG | 0444;
	// 	stbuf->st_nlink = 1;
	// 	stbuf->st_size = strlen(options.contents);
	// } else
	// 	res = -ENOENT;

	// return res;
	
	
	
	
	
	
	(void) fi;
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else {
		
		const struct memfs_file *pf = __search(&root,path+1);

if (!pf)
	{
		//todo the file doent't exsit
    // strcat (options.contents ,"strings 404 can't getattr");

		return -ENOENT;

	}
		if (strcmp(path+1, pf->option->filename) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(pf->option->contents);

		
		
	}

	else
	{	res = -ENOENT;}
}
	return res;
}

static int my_chat_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi,
			 enum fuse_readdir_flags flags)
{
	(void) offset;
	(void) fi;
	(void) flags;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0, 0);
	filler(buf, "..", NULL, 0, 0);
	// filler(buf, options.filename, NULL, 0, 0);

	struct rb_node *node = NULL;
        for (node = rb_first(&root); node; node = rb_next(node)) {
            const struct memfs_file *pf = rb_entry(node, struct memfs_file, node);
            filler(buf,pf->option->filename,NULL,0,0);
        }

	return 0;
}

static int my_chat_open(const char *path, struct fuse_file_info *fi)
{
	if (strcmp(path+1, options.filename) != 0)
		return -ENOENT;

	if ((fi->flags & O_ACCMODE) != O_RDONLY)
		return -EACCES;
	const struct memfs_file *pf = __search(&root,path+1);
	if (pf)
	{
	// 	int res;
	// 	//todo ?

	// res = open(path, fi->flags);
	// if (res == -1)
	// 	return -errno;

	// fi->fh = res;
		

		return 0;
		

	}
	else{

		//the file doent't exsit
		return -ENOENT;
	}




	return 0;
}

static int my_chat_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;

	const struct memfs_file *pf = __search(&root,path+1);
	if (pf)
	{
		
	if(strcmp(path+1, pf->option->filename) != 0)
		{
    strcat (options.contents ,"strings 404 can't read 1");
			
			return -ENOENT;}

	len = strlen(pf->option->contents);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, pf->option->contents+ offset, size);

	} else
		size = 0;

	return size;

	}
	else{
    strcat (options.contents ,"strings 404 can't read 2");


		//the file doent't exsit
		return -ENOENT;
	}


}

static int my_chat_create(const char *path, mode_t mode,
		      struct fuse_file_info *fi)
{
	
	
	const struct memfs_file *pf = __search(&root,path+1);
	if (!pf)
	{
		
		struct options *new_option=malloc(sizeof(struct options));
		new_option->filename = strdup(path+1);
		new_option->contents = strdup("");

		struct memfs_file *memnode=malloc(sizeof(struct memfs_file));
		memnode->path=strdup(path+1);
		memnode->option=new_option;
		__insert(&root, memnode);
		return 0;
		

	}
	else{

		return -EEXIST;
	}
	
	return 0;
}


static int my_chat_write(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{


	const struct memfs_file *pf = __search(&root,path+1);
	if (pf)
	{
	memcpy(pf->option->contents+ offset, buf, size);
		
	

	// len = strlen(pf->option->contents);
	// if (offset < len) {
	// 	if (offset + size > len)
	// 		size = len - offset;
	// 	memcpy(buf, pf->option->contents+ offset, size);

	// } else
	// 	size = 0;

	return size;

	}
	else{

		//the file doent't exsit

    strcat (options.contents ,"strings 404 can't write ");
		return -ENOENT;
	}
}

static const struct fuse_operations my_chat_oper = {
	.init           = my_chat_init,
	.getattr	= my_chat_getattr,
	.readdir	= my_chat_readdir,
	.open		= my_chat_open,
	.read		= my_chat_read,
	.write		= my_chat_write,
	.create		=my_chat_create,
	.truncate 	=my_chat_truncate,
	.utimens	= my_chat_utimens,
};

static void show_help(const char *progname)
{
	printf("usage: %s [options] <mountpoint>\n\n", progname);
	printf("File-system specific options:\n"
	       "    --name=<s>          Name of the \"my_chat\" file\n"
	       "                        (default: \"my_chat\")\n"
	       "    --contents=<s>      Contents \"my_chat\" file\n"
	       "                        (default \"my_chat, World!\\n\")\n"
	       "\n");
}

int main(int argc, char *argv[])
{
	int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	/* Set defaults -- we have to use strdup so that
	   fuse_opt_parse can free the defaults if other
	   values are specified */
	options.filename = strdup("my_chat_debug");
	options.contents = strdup("my_chat World!qwqq\n");
	struct memfs_file *memnode=malloc(sizeof(struct memfs_file));
		memnode->path=strdup(options.filename );
		memnode->option=&options;
		__insert(&root, memnode);

	/* Parse options */
	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
		return 1;

	/* When --help is specified, first print our own file-system
	   specific help text, then signal fuse_main to show
	   additional help (by adding `--help` to the options again)
	   without usage: line (by setting argv[0] to the empty
	   string) */
	if (options.show_help) {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0][0] = '\0';
	}

	ret = fuse_main(args.argc, args.argv, &my_chat_oper, NULL);
	fuse_opt_free_args(&args);
	return ret;
}
















