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
 * Print status of the file system node.
 * :param inode:
 */
void printstatfd(int fd) {
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
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
    // owner
    // group
    // last modified time
    char lastmodtime[128] = { '\0' };
    strftime(lastmodtime, 128, "%b %d", localtime(&sb.st_mtim.tv_sec));
    // filename
    printf("%c%o %u %u %lu %s [filename]\n",
            typechar, sb.st_mode & 0777, sb.st_uid, sb.st_gid, sb.st_size,
            lastmodtime);
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
