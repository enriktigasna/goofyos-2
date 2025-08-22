struct cmdline {
	char *arg;
	struct cmdline *next;
};

extern struct cmdline kernel_cmdline;

void parse_cmdline();
char *cmdline_get(const char *value);
char *find_cmdline(char *query);
bool cmdline_contains(char *query);