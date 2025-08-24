#define S_IFMT   0170000  /* bitmask */
#define S_IFREG  0100000  /* file */
#define S_IFDIR  0040000  /* directory */
#define S_IFCHR  0020000  /* char device */
#define S_IFBLK  0060000  /* block device */
#define S_IFLNK  0120000  /* symlink */
#define S_IFIFO  0010000  /* pipe */
#define S_IFSOCK 0140000  /* socket */

#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
