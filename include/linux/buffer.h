#ifndef __BUFFER_H__
#define __BUFFER_H__

static inline int buffer_locked(struct buffer_head *bh)
{
	return bh->b_lock;
}

static inline int buffer_dirtied(struct buffer_head *bh)
{
	return bh->b_dirt;
}

static inline int buffer_uptodated(struct buffer_head *bh)
{
	return bh->b_uptodate;
}

#endif
