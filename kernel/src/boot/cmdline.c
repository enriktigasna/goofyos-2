#include <goofy-os/boot.h>
#include <goofy-os/cmdline.h>
#include <goofy-os/printk.h>
#include <goofy-os/slab.h>
#include <stdbool.h>
#include <string.h>

#define MAX_CMDLINE_COMMAND 512
#define WORD(a) (a && a != ' ')

/**
 * All arguments are either key-value or just key
 * Example:
 *
 * UUID=xxxx-xxxx-xxxx-xxx
 * or
 * initrd
 */

struct cmdline kernel_cmdline;

void build_args(char *cmd) {
	struct cmdline *curr_list = &kernel_cmdline;
	char curr_word[MAX_CMDLINE_COMMAND];
	int curr_len = 0;
	int i;

	for (i = 0; cmd[i]; i++) {
		if (cmd[i] == ' ') {
			curr_list->arg = kzalloc(curr_len + 1);
			curr_word[curr_len] = '\0';

			strcpy(curr_list->arg, curr_word);
			curr_list->next = kzalloc(sizeof(struct cmdline));
			curr_list = curr_list->next;
			curr_len = 0;
			continue;
		}
		curr_word[curr_len] = cmd[i];
		curr_len++;
	}

	curr_word[curr_len] = '\0';
	curr_list->arg = kzalloc(curr_len + 1);
	strcpy(curr_list->arg, curr_word);
}

void make_key(char *current) {
	for (int i = 0; current[i]; i++) {
		/* code */
		if (current[i] == '=') {
			current[i] = '\0';
			return;
		}
	}
}

/**
 * Dynamically allocates and returns value.
 * Needs to be freed to prevent memory leaks!
 */
char *make_val(char *current) {
	int i;
	for (i = 0; current[i]; i++) {
		/* code */
		if (current[i] == '=') {
			break;
		}
	}

	if (current[i] == '=')
		i++;

	char *ret = kzalloc(strlen(&current[i]) + 1);
	strcpy(ret, &current[i]);
	return ret;
}

char *find_cmdline(char *query) {
	char key[MAX_CMDLINE_COMMAND];
	for (struct cmdline *curr = &kernel_cmdline; curr; curr = curr->next) {
		strcpy(key, curr->arg);
		make_key(key);
		printk("Key %s\n", key);

		if (!strncmp(key, query, MAX_CMDLINE_COMMAND)) {
			int keylen = strlen(key);
			return make_val(curr->arg + keylen);
		}
	}
	return NULL;
}

bool cmdline_contains(char *query) {
	char *res = find_cmdline(query);
	bool contains = (bool)res;
	kfree(res);
	return contains;
}

void parse_cmdline() {
	if (!__limine_cmdline_response) {
		return;
	}

	if (strlen(__limine_cmdline_response->cmdline) > MAX_CMDLINE_COMMAND) {
		// TODO panic logic
		return;
	}

	build_args(__limine_cmdline_response->cmdline);
}

// char *cmdline_get(const char *value) {}