#pragma once

#define GET_BUDDY(pfn, order) (pfn ^ (1 << order))

#define BUDDY_ZERO 1

struct page;

void free_pages(struct page *page, int order);
void free_page(struct page *page);

struct page *alloc_pages(int order, int flags);
struct page *alloc_page();