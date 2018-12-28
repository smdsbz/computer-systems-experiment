#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <dirent.h>

#include <pwd.h>
#include <grp.h>

#include <time.h>

typedef char bool;


/* Helper Functions */

/*
 * Split out name from path
 * :param path: Path to file or directory.
 * :return: Name (statically allocated, DO NOT free), NULL on invalid path or error.
 */
const char *splitname(const char *path) {
    static char retbuf[256];
    // safe-copy path to pathbak
    if (strnlen(path, 255) == 255) {
        return NULL;
    }
    char pathbak[256];
    strcpy(pathbak, path);
    pathbak[255] = '\0';
    int lastslash = strlen(pathbak) - 1;
    // validation
    if (lastslash == -1) {      // empty
        return NULL;
    } else if (strcmp("/", pathbak) == 0) {     // "/"
        return NULL;
    } else if (pathbak[lastslash] == '/') {     // "xxxx/"
        pathbak[lastslash] = '\0';
    }
    // get index of last '/'
    for (; lastslash && pathbak[lastslash] != '/'; --lastslash) ;
    ++lastslash;
    strcpy(retbuf, pathbak + lastslash);
    return retbuf;
}


/*
 * Translate permission bits to string
 * :param perm: Permission bits.
 * :return: "rwx......" formatted string (NO free)
 */
const char *getpermstr(u_int16_t bits) {
    static char retstr[10];
    strcpy(retstr, "---------");
    int plc = 0;
    for (; plc != 3; ++plc, bits <<= 3) {
        if (bits & 0400) {
            retstr[0 + plc * 3] = 'r';
        }
        if (bits & 0200) {
            retstr[1 + plc * 3] = 'w';
        }
        if (bits & 0100) {
            retstr[2 + plc * 3] = 'x';
        }
    }
    return retstr;
}


/*
 * Translate file size in byte to human readable
 * :param size:
 * :return: c-string (NO free)
 */
const char *gethsize(size_t size) {
    static char retbuf[33];
    static char units[] = "BKMGT";
    int unitidx = 0;
    double filesize = size;
    for (; filesize > 1024.0 && unitidx != 4; filesize /= 1024.0, ++unitidx) ;
    sprintf(retbuf, "%.1f%c", filesize, units[unitidx]);
    return retbuf;
}


/*
 * Print status of the file system node.
 * :param path: File path.
 * :param is_dir: Valid if not NULL.
 * :param filsiz: Valid if not NULL, this is calculated recursively.
 */
void printstat(const char *path, bool *is_dir, size_t *filsiz) {
    static struct stat sb;
    if (is_dir != NULL) {
        *is_dir = 0;
    }
    // get absolute path
    char *abspath = realpath(path, NULL);   // no trailing '/', even with a directory
    if (stat(abspath, &sb) == -1) {
        perror("fstat");
        exit(EXIT_FAILURE);
    }
    if (filsiz != NULL) {
        *filsiz = sb.st_size;
    }
    // filetype
    char typechar;
    switch (sb.st_mode & S_IFMT) {
        case S_IFBLK: { typechar = 'b'; break; }
        case S_IFCHR: { typechar = 'c'; break; }
        case S_IFDIR: {
            typechar = 'd';
            if (is_dir != NULL) {
                *is_dir = 1;
            }
            break;
        }
        case S_IFLNK: { typechar = 'l'; break; }
        case S_IFREG: { typechar = '-'; break; }
        case S_IFSOCK: { typechar = 's'; break; }
        default: { typechar = '?'; break; }
    }
    // last modified time
    char lastmodtime[32] = { '\0' };
    strftime(lastmodtime, 32, "%b %d %R", localtime(&sb.st_mtim.tv_sec));
    // generate final output
    printf("%c%s %3lu %7s %7s %7s %s %s",
            typechar, getpermstr(sb.st_mode & 0777),
            sb.st_nlink,
            getpwuid(sb.st_uid)->pw_name, getgrgid(sb.st_gid)->gr_name,
            gethsize(sb.st_size),
            lastmodtime,
            splitname(abspath));
    // exit jobs
    free(abspath);
    return;
}


/* Helper Data Structure - Queue */

struct QueueNode {
    char                path[256];
    int                 depth;
    struct QueueNode   *next;
};

struct Queue {
    struct QueueNode   *start;
    struct QueueNode   *end;
} dirque;


void printque() {
    struct QueueNode *pqn = dirque.start;
    while (pqn != NULL) {
        printf("%s\n", pqn->path);
        pqn = pqn->next;
    }
}


bool emptyque(struct Queue q) {
    return q.start == NULL;
}


void enque(struct Queue *q, const char *path, int depth) {
    // allocate space
    if (q->start == NULL) {
        if ((q->start = malloc(sizeof(struct QueueNode))) == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        q->end = q->start;
    } else {
        if ((q->end->next = malloc(sizeof(struct QueueNode))) == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        q->end = q->end->next;
    }
    // regulate end
    q->end->next = NULL;
    // copy data, truncate if needed
    strncpy(q->end->path, path, 255);
    q->end->path[255] = '\0';
    q->end->depth = depth;
    return;
}


/*
 * :return: QueueNode. A later free is required!
 */
struct QueueNode *deque(struct Queue *q) {
    if (q->start == NULL) {
        return NULL;
    }
    struct QueueNode *retptr = q->start;
    // fix queue
    q->start = q->start->next;
    if (q->start == NULL) {
        q->end = NULL;
    }
    // close node
    retptr->next = NULL;
    return retptr;
}


/* Call Entry */

/*
 * Print the first directory is session's queue.
 * :param q: Pointer to directory queue. If queue is empty, nothing is printed.
 *      After a successful print, the first element in the queue is consumed.
 */
void _printdirqueue(struct Queue *q) {
    /* printque(); */
    struct QueueNode *dirno = deque(q);
    if (dirno == NULL) {
        return;
    }
    /* puts("after deque"); */
    /* printque(); */
    char *abspath_noslash = realpath(dirno->path, NULL);
    DIR *dp;
    if ((dp = opendir(abspath_noslash)) == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    printf("%s:\n", abspath_noslash);
    // list dir
    struct dirent *pdent;
    size_t total_size = 0;
    bool is_dir;
    size_t size;
    while ((pdent = readdir(dp)) != NULL) {
        // skip current directory and parent directory
        if (strcmp(".", pdent->d_name) == 0 || strcmp("..", pdent->d_name) == 0) {
            continue;
        }
        // filepath = abspath + "/" + filename
        char filepath[256];
        strcpy(filepath, abspath_noslash);
        strcat(filepath, "/");
        strcat(filepath, pdent->d_name);
        // print single file record
        printstat(filepath, &is_dir, &size);
        printf(" (%d)\n", dirno->depth);
        // summary jobs
        total_size += size;
        // append to queue if is a directory
        if (is_dir && strcmp(".git", splitname(filepath))) {
            enque(&dirque, filepath, dirno->depth + 1);
            /* printque(); */
        }
    }
    printf("total %s\n\n", gethsize(total_size));
    // exit jobs
    closedir(dp);
    free(dirno);
    free(abspath_noslash);
    return;
}


/*
 * Print directory and all subsequents.
 * :param path:
 * :param depth: [UNUSED]
 */
void printdir(const char *path, int depth) {
    char *abspath_noslash = realpath(path, NULL);
    if (abspath_noslash == NULL) {
        perror("realpath");
        exit(EXIT_FAILURE);
    }
    // en-queue the starting point
    enque(&dirque, abspath_noslash, depth);
    while (!emptyque(dirque)) {
        _printdirqueue(&dirque);
        /* printque(); */
        /* getchar(); */
    }
    return;
}


/* Main */

int main(const int argc, const char **argv) {
    if (argc != 2) {
        printf("Usage: PROG PATH\n");
        exit(EXIT_FAILURE);
    }
    printdir(argv[1], 0);
    return EXIT_SUCCESS;
}
