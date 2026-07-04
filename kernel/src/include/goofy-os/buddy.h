#pragma once

#define GET_BUDDY(pfn, order) (pfn ^ (1 << order))

#define BUDDY_ZERO 1

struct page;

void free_pages(struct page *page, int order);
void free_page(struct page *page);