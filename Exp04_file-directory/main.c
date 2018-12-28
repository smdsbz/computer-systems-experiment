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
    sprintf(retbuf, "% 7.1f%c", filesize, units[unitidx]);
    return retbuf;
}


/*
 * Print status of the file system node.
 * :param path: File path.
 */
void printstat(const char *path, bool *is_dir, size_t *filsiz) {
    static struct stat sb;
    // get absolute path
    if (strnlen(path, 255) == 255) {
        exit(EXIT_FAILURE);
    }
    char *abspath = realpath(path, NULL);
    if (stat(abspath, &sb) == -1) {
        perror("fstat");
        exit(EXIT_FAILURE);
    }
    // filetype
    char typechar;
    switch (sb.st_mode & S_IFMT) {
        case S_IFBLK: { typechar = 'b'; break; }
        case S_IFCHR: { typechar = 'c'; break; }
        case S_IFDIR: { typechar = 'd'; break; }
        case S_IFLNK: { typechar = 'l'; break; }
        case S_IFREG: { typechar = '-'; break; }
        case S_IFSOCK: { typechar = 's'; break; }
    }
    // last modified time
    char lastmodtime[32] = { '\0' };
    strftime(lastmodtime, 32, "%b %d", localtime(&sb.st_mtim.tv_sec));
    // generate final output
    printf("%c%s %3lu %8s %8s %s %s %s\n",
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

typedef struct Queue {
    u_int32_t       data;
    struct Queue   *next;
} Queue;


/* Call Entry */

/*
 * Print directory and all subsequent directories, til specified depth.
 * :param dirna: Top / Starting directory name.
 * :param depth: Max recursive call times.
 */
void printdir(const char *dirna, int depth) {
    DIR *dp;

}


/* Main */

int main(const int argc, const char **argv) {
    printstat("./main.c");
    return EXIT_SUCCESS;
}
