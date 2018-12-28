#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <dirent.h>

#include <pwd.h>

#include <time.h>


/* Utility Functions */

/*
 * Split out name from path
 * :param path: Path to file or directory.
 * :return: Name (statically allocated, DO NOT free), NULL on invalid path or error.
 */
char *splitname(const char *path) {
    static char retbuf[256];
    // safe-copy path to pathbak
    if (strnlen(path, 255) == 255) {
        return NULL;
    }
    char pathbak[256];
    strncpy(pathback, path, 255);
    pathbak[255] = '\0';
    int lastslash = strlen(pathbak) - 1;
    // validation
    if (lastslash == -1) {
        return NULL;
    } else if (lastslash == 0 && pathbak[0] == '\0') {
        return NULL;
    } else if (pathbak[lastslash] == '/') {
        pathbak[lastslash] = '\0';
    }
    // get index of last '/'
    for (; lastslash && pathbak[lastslash] != '/'; --lastlash) ;
    ++lastslash;
    strcpy(retbuf, pathbak + lastslash);
    return retbuf;
}


/*
 * Print status of the file system node.
 * :param path: File path.
 */
void printstatfd(const char *path) {
    static struct stat sb;
    if (stat(path, &sb) == -1) {
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
    // permission
    // owner
    // group
    // last modified time
    char lastmodtime[128] = { '\0' };
    strftime(lastmodtime, 128, "%b %d", localtime(&sb.st_mtim.tv_sec));
    // filename
    // TODO:
    printf("%c%o %u %u %lu %s %s\n",
            typechar, sb.st_mode & 0777, sb.st_uid, sb.st_gid, sb.st_size,
            lastmodtime, name);
    return;
}

/*
 * List all contents in directory.
 * :param dp: Directory to be listed.
 */
void listdirp(const DIR *dp) {

}

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
    FILE *fp;
    int fd;
    if ((fp = fopen("./main.c", "r")) == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    if ((fd = fileno(fp)) == -1) {
        perror("fileno");
        exit(EXIT_FAILURE);
    }
    printstatfd(fd);
    if (fclose(fp) == -1) {
        perror("fclose");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
