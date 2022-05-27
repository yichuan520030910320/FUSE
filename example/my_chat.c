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
    char *filename;
    char *contents;
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
    char *data;                     /* File content *///maybe useless
    struct options *option;
    // u8 free_on_delete;
    int dir_or_file;            //1 stand for dir 2 stand for file

    struct stat *vstat;              /* File stat */

    pthread_mutex_t lock;
    pthread_cond_t write_finished;

    struct rb_node node;
};

static int __insert(struct rb_root *root, struct memfs_file *pf) {
    struct rb_node **new = &(root->rb_node), *parent = NULL;

    /* Figure out where to put new node */
    while (*new) {
        struct memfs_file *this = container_of(*new, struct memfs_file, node);
        int result = strcmp(pf->path, this->path);
        parent = *new;
        if (result < 0) {
            new = &((*new)->rb_left);
        } else if (result > 0) {
            new = &((*new)->rb_right);
        } else {
            printf("%s is exist!!!\n", pf->path);
            return -1;
        }
    }

    /* Add new node and rebalance tree. */
    rb_link_node(&pf->node, parent, new);
    rb_insert_color(&pf->node, root);
    return 0;
}

static struct memfs_file *__search(struct rb_root *root, const char *path) {
    struct memfs_file *pf = NULL;
    struct rb_node *node = root->rb_node;

    while (node) {
        pf = container_of(node, struct memfs_file, node);

        int result = strcmp(path, pf->path);
        if (result < 0) {
            node = node->rb_left;
        } else if (result > 0) {
            node = node->rb_right;
        } else {
            return pf;
        }
    }

//  printf("%s not found!!!\n", path);
    return NULL;
}

static inline void __free(struct memfs_file *pf) {
    {
        if (pf->data) {
            free(pf->data);
        }
        if (pf->path) {
            free(pf->path);
        }
        free(pf);
    }
}

static int __delete(struct rb_root *root, const char *path) {
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
                          struct fuse_config *cfg) {
    (void) conn;
    cfg->kernel_cache = 1;
    return NULL;
}

static int my_chat_truncate(const char *path, off_t size,
                            struct fuse_file_info *fi) {
    (void) size;
    (void) fi;

    // if(strcmp(path, "/") != 0)
    // 	return -ENOENT;

    return 0;
}

static int my_chat_utimens(const char *path, const struct timespec ts[2],
                           struct fuse_file_info *fi) {
    (void) fi;


    return 0;
}

static int my_chat_getattr(const char *path, struct stat *stbuf,
                           struct fuse_file_info *fi) {


    (void) fi;
    int res = 0;

    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else {

        const struct memfs_file *pf = __search(&root, path);

        if (!pf) {
            //todo the file doent't exsit
            // strcat (options.contents ,"strings 404 can't getattr");

            return -ENOENT;

        }


        // stbuf=pf->vstat;
        if (pf->dir_or_file == 2 && strcmp(path, pf->option->filename) == 0) {
            stbuf->st_mode = S_IFREG | 0444;
            stbuf->st_nlink = 1;
            stbuf->st_size = strlen(pf->option->contents);
        } else if (pf->dir_or_file == 1) {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
        } else { res = -ENOENT; }
    }

    return res;
}


static inline const char *__is_parent(const char *parent, const char *path) {
    const char delim = '/';

    if (parent[1] == '\0' && parent[0] == '/' && path[0] == '/') {
        return path;
    }

    while (*parent != '\0' && *path != '\0' && *parent == *path) {
        ++parent, ++path;
    }
    return (*parent == '\0' && *path == delim) ? path : NULL;
}

// get a utility from https://blog.csdn.net/stayneckwind2/article/details/82876330

static int my_chat_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                           off_t offset, struct fuse_file_info *fi,
                           enum fuse_readdir_flags flags) {
    (void) offset;
    (void) fi;
    (void) flags;

    // if (strcmp(path, "/") != 0)
    // 	return -ENOENT;

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);
    // filler(buf, options.filename, NULL, 0, 0);

    struct rb_node *node = NULL;


    if (strcmp(path, "/") == 0) {
        for (node = rb_first(&root); node; node = rb_next(node)) {
            const struct memfs_file *pf = rb_entry(node, struct memfs_file, node);
            if (strchr(pf->option->filename + 1, '/')) {
                continue;
            }
            filler(buf, pf->option->filename + 1, NULL, 0, 0);
        }
    } else {

//todo read in dir


        struct memfs_file *pentry = __search(&root, path);
        for (node = rb_next(&pentry->node); node; node = rb_next(node)) {
            //todo break
            const struct memfs_file *pf = rb_entry(node, struct memfs_file, node);

            const char *basename = __is_parent(path, pf->path);

            if (!basename) {
                break;
            }
            //avoid /xxx/xxx/xxx
            if (strchr(basename + 1, '/')) {
                continue;
            }

            filler(buf, basename + 1, NULL, 0, 0);
        }


    }

    return 0;
}

static int my_chat_open(const char *path, struct fuse_file_info *fi) {


    return 0;
}


static int my_chat_read(const char *path, char *buf, size_t size, off_t offset,
                        struct fuse_file_info *fi) {
    size_t len;
    (void) fi;
    const struct memfs_file *pf = __search(&root, path);
    if (pf) {
        strcpy(buf, pf->option->contents);
        return size;

    } else {
        return -ENOENT;
    }
}

//

static int my_chat_write(const char *path, char *buf, size_t size, off_t offset,
                         struct fuse_file_info *fi) {


    const struct memfs_file *pf = __search(&root, path);
    if (pf) {
        if (strchr(path + 1, '/')) {
            char *pre = malloc(strlen(pf->option->contents));
            strcpy(pre, pf->option->contents);
            pf->option->contents = malloc(strlen(pre) + strlen(buf) + 1);
            strcpy(pf->option->contents, pre);
            strcat(pf->option->contents, buf);
//another path
            const char *str = path;
            const char ch = '/';
            char *ret;
            char *str1 = malloc(strlen(str));
            ret = strchr(str + 1, ch);
            strcpy(str1, ret);
            strncat(str1, str, strlen(str) - strlen(ret));
            const struct memfs_file *pf1 = __search(&root, str1);
            if (!pf1) {
                return -ENOENT;

            }
            char *pre1 = malloc(strlen(pf1->option->contents));
            strcpy(pre1, pf1->option->contents);
            pf1->option->contents = malloc(strlen(pre1) + strlen(buf) + 1);
            strcpy(pf1->option->contents, pre1);
            strcat(pf1->option->contents, buf);


            return size;
        }
        char *pre = malloc(strlen(pf->option->contents));
        strcpy(pre, pf->option->contents);
        pf->option->contents = malloc(strlen(pre) + strlen(buf) + 1);
        strcpy(pf->option->contents, pre);
        strcat(pf->option->contents, buf);
        return size;

    } else {

        //the file doent't exsit

        return -ENOENT;
    }
}

static int my_chat_mknod(const char *path, mode_t mode, dev_t rdev) {

    const struct memfs_file *pf = __search(&root, path);
    if (!pf) {
        if (strchr(path + 1, '/')) {
            struct options *new_option = malloc(sizeof(struct options));
            new_option->filename = strdup(path);
            new_option->contents = "";
            struct memfs_file *memnode = malloc(sizeof(struct memfs_file));
            memnode->path = strdup(path);
            memnode->dir_or_file = 2;
            memnode->option = new_option;
            __insert(&root, memnode);
            const char *str = path;
            const char ch = '/';
            char *ret;
            char *str1 = malloc(strlen(str));
            ret = strchr(str + 1, ch);
            strcpy(str1, ret);
            strncat(str1, str, strlen(str) - strlen(ret));
            new_option = malloc(sizeof(struct options));
            new_option->filename = strdup(str1);
            new_option->contents = strdup("");
            memnode = malloc(sizeof(struct memfs_file));
            memnode->path = strdup(str1);
            memnode->dir_or_file = 2;
            memnode->option = new_option;
            __insert(&root, memnode);
            return 0;
        }
        struct options *new_option = malloc(sizeof(struct options));
        new_option->filename = strdup(path);
        new_option->contents = strdup("");
        struct memfs_file *memnode = malloc(sizeof(struct memfs_file));
        memnode->path = strdup(path);
        memnode->dir_or_file = 2;
        memnode->option = new_option;
        __insert(&root, memnode);
        return 0;
    } else {

        return -EEXIST;
    }
}


static int my_chat_release(const char *path, struct fuse_file_info *fi) {
    return 0;
}

static int my_chat__unlink(const char *path) {
    const struct memfs_file *pf = __search(&root, path);
    if (pf) {
        __delete(&root, path);
        return 0;
    } else {
        return -ENOENT;
    }
}

static int my_chat_mkdir(const char *path, mode_t mode) {
    //todo
    const struct memfs_file *pf = __search(&root, path);
    if (!pf) {
        struct options *new_option = malloc(sizeof(struct options));
        new_option->filename = strdup(path);
        struct memfs_file *memnode = malloc(sizeof(struct memfs_file));
        memnode->path = strdup(path);
        memnode->dir_or_file = 1;
        memnode->option = new_option;
        __insert(&root, memnode);
        return 0;
    } else {
        return -EEXIST;
    }
    res = mkdir(path, mode);
}

int is_in(char *s, char *c) {
    int i = 0, j = 0, flag = -1;
    while (i < strlen(s) && j < strlen(c)) {
        if (s[i] == c[j]) {//如果字符相同则两个字符都增加
            i++;
            j++;
        } else {
            i = i - j + 1; //主串字符回到比较最开始比较的后一个字符
            j = 0;     //字串字符重新开始
        }
        if (j == strlen(c)) { //如果匹配成功
            flag = 1;  //字串出现
            break;
        }
    }
    return flag;
}
static int my_chat_rmdir(const char *path) {
    const struct memfs_file *pf = __search(&root, path);
    if (pf) {
        __delete(&root, path);
        /* code */
        struct rb_node *node = NULL;
        int remove_cnt = 0;
        char **pps8Output = (char **) malloc(300 * sizeof(char *));
        for (node = rb_first(&root); node; node = rb_next(node)) {
            //todo break
            const struct memfs_file *pf1 = rb_entry(node, struct memfs_file, node);
            //todo
            if (pf1->dir_or_file == 2) {
                char *ret;
                *ret = NULL;
                char *tmp_path = strdup(pf1->path);;
                char *tmp_path1 = strdup(path);
                if (is_in(tmp_path, tmp_path1) == 1) {
                    pps8Output[remove_cnt++] = strdup(pf1->path);
                }
            }
        }
        for (int i = 0; i < remove_cnt; i++) {


            __delete(&root, pps8Output[i]);
        }

        for (int i = 0; i < remove_cnt; i++) {
            free(pps8Output[i]);
        }
        free(pps8Output);

        return 0;

    } else {

        return -ENOENT;
    }

}

static int my_chat_access(const char *path, int mask) {


    return 0;
}

static const struct fuse_operations my_chat_oper = {
        .init           = my_chat_init,
        .getattr    = my_chat_getattr,
        .readdir    = my_chat_readdir,
        .open        = my_chat_open,
        .read        = my_chat_read,
        .write        = my_chat_write,
        .truncate    =my_chat_truncate,
        .utimens    = my_chat_utimens,
        .mknod        = my_chat_mknod,
        .release    = my_chat_release,
        .unlink        = my_chat__unlink,
        .mkdir        =my_chat_mkdir,
        .rmdir        = my_chat_rmdir,
        .access        = my_chat_access,


};

static void show_help(const char *progname) {
    printf("usage: %s [options] <mountpoint>\n\n", progname);
    printf("File-system specific options:\n"
           "    --name=<s>          Name of the \"my_chat\" file\n"
           "                        (default: \"my_chat\")\n"
           "    --contents=<s>      Contents \"my_chat\" file\n"
           "                        (default \"my_chat, World!\\n\")\n"
           "\n");
}

int main(int argc, char *argv[]) {
    int ret;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    /* Parse options */
    if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
        return 1;
    if (options.show_help) {
        show_help(argv[0]);
        assert(fuse_opt_add_arg(&args, "--help") == 0);
        args.argv[0][0] = '\0';
    }

    ret = fuse_main(args.argc, args.argv, &my_chat_oper, NULL);
    fuse_opt_free_args(&args);
    return ret;
}
















