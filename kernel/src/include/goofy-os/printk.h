#pragma once
int printk(const char *fmt, ...);
int sprintf(char *buf, const char *fmt, ...);
extern struct spinlock printk_lock;