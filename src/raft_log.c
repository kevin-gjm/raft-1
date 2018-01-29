/**
 * Copyright (c) 2013, Willem-Hendrik Thiart
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * @file
 * @brief ADT for managing Raft log entries (aka entries)
 * @author Willem Thiart himself@willemthiart.com
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "raft.h"
#include "raft_private.h"
#include "raft_log.h"


typedef struct
{
    /* the amount of elements in the array */
    unsigned long count;

    /* we compact the log, and thus need to increment the Base Log Index */
    unsigned long base;


    /* callbacks */
    raft_cbs_t *cb;
    void* raft;
} log_private_t;



int log_load_from_snapshot(log_t *me_, unsigned long idx, unsigned long term)
{
    log_private_t* me = (log_private_t*)me_;

    log_clear(me_);

    raft_entry_t ety;
    ety.data.len = 0;
    ety.id = 1;
    ety.term = term;
    ety.type = RAFT_LOGTYPE_SNAPSHOT;

    int e = log_append_entry(me_, &ety);
    if (e != 0)
    {
        assert(0);
        return e;
    }

    me->base = idx - 1;

    return 0;
}



log_t* log_new()
{
	log_private_t* me = (log_private_t*)calloc(1, sizeof(log_private_t));
    if (!me)
        return NULL;
    log_clear((log_t*)me);
    return (log_t*)me;
}

void log_set_callbacks(log_t* me_, raft_cbs_t* funcs, void* raft)
{
    log_private_t* me = (log_private_t*)me_;

    me->raft = raft;
    me->cb = funcs;
}

void log_clear(log_t* me_)
{
    log_private_t* me = (log_private_t*)me_;
    me->count = 0;
    me->base = 0;
}

/** TODO: rename log_append */
int log_append_entry(log_t* me_, raft_entry_t* ety)
{
    log_private_t* me = (log_private_t*)me_;
    unsigned long idx = me->base + me->count + 1;
    int e;

    if (me->cb && me->cb->log_offer)
    {
        void* ud = raft_get_udata(me->raft);
        e = me->cb->log_offer(me->raft, ud,ety, idx);
        if (0 != e)
            return e;
        raft_offer_log(me->raft, ety, idx);
    }

    me->count++;

    return 0;
}

//return log entries include the idx log.
//should free it yourself
raft_entry_t* log_get_from_idx(log_t* me_, unsigned long idx, unsigned long *n_etys)
{
    log_private_t* me = (log_private_t*)me_;

    assert(0 != idx);

    if (me->base + me->count < idx || idx < me->base)
    {
        *n_etys = 0;
        return NULL;
    }

    /* idx starts at 1 */
	unsigned long total_count=0;
    *n_etys = total_count = me->base + me->count - idx + 1;

	if(total_count ==0)
		return NULL;
	raft_entry_t *entries = (raft_entry_t*)calloc(total_count,sizeof(raft_entry_t));
	if(entries==NULL)
		return NULL;
	for(unsigned long i=0;i<total_count;i++)
	{
		log_get_at_idx(me_,idx+i,&entries[i]);
	}
    return entries;
}

int log_get_at_idx(log_t* me_, unsigned long idx,raft_entry_t* ety)
{
    log_private_t* me = (log_private_t*)me_;

    if (idx == 0)
        return -1;

    if (idx <= me->base)
        return -1;

    if (me->base + me->count < idx)
        return -1;

    /* idx starts at 1 */
	if (me->cb && me->cb->log_get)
	{
		me->cb->log_get(me->raft, raft_get_udata(me->raft), ety, idx);
		return 0;
	}
	return -1; 
}

unsigned long log_count(log_t* me_)
{
	log_private_t* me = ((log_private_t*)me_);

    return me->count+me->base;
}

int log_delete(log_t* me_, unsigned long idx)
{
    log_private_t* me = (log_private_t*)me_;

    if (0 == idx)
        return -1;

    if (idx < me->base)
        idx = me->base;

    for (; idx <= me->base + me->count && me->count;)
    {
        unsigned long idx_tmp = me->base + me->count;
		raft_entry_t entry={0};

		log_get_at_idx(me_, idx_tmp, &entry);
		raft_pop_log(me->raft, &entry, idx_tmp);
        if (me->cb && me->cb->log_pop)
        {
            int e = me->cb->log_pop(me->raft, raft_get_udata(me->raft),idx_tmp);
            if (0 != e)
                return e;
        }
        me->count--;
    }
    return 0;
}

int log_poll(log_t * me_)
{
    log_private_t* me = (log_private_t*)me_;
    unsigned long idx = me->base + 1;

    if (0 == me->count)
        return -1;


    if (me->cb && me->cb->log_poll)
    {
        int e = me->cb->log_poll(me->raft, raft_get_udata(me->raft), idx);
        if (0 != e)
            return e;
    }
    me->count--;
    me->base++;

    return 0;
}

void log_empty(log_t * me_)
{
    log_private_t* me = (log_private_t*)me_;

    me->count = 0;
}

void log_free(log_t * me_)
{
    log_private_t* me = (log_private_t*)me_;

    free(me);
}

unsigned long log_get_current_idx(log_t* me_)
{
    log_private_t* me = (log_private_t*)me_;
    return log_count(me_) + me->base;
}

unsigned long log_get_base(log_t* me_)
{
    return ((log_private_t*)me_)->base;
}
