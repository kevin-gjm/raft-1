/**
 * Copyright (c) 2013, Willem-Hendrik Thiart
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * @file
 * @brief Representation of a peer
 * @author Willem Thiart himself@willemthiart.com
 * @version 0.1
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "raft.h"

typedef struct
{
    void* udata;

    int next_idx;
    int match_idx;

    /* 1 if node has voted for me */
    int voted_for_me;

    int id;
} raft_node_private_t;

raft_node_t* raft_node_new(void* udata, int id)
{
    raft_node_private_t* me;
    me = (raft_node_private_t*)calloc(1, sizeof(raft_node_private_t));
    me->udata = udata;
    me->next_idx = 1;
    me->match_idx = 0;
    me->id = id;
    return (raft_node_t*)me;
}

int raft_node_get_next_idx(raft_node_t* me_)
{
    raft_node_private_t* me = (raft_node_private_t*)me_;
    return me->next_idx;
}

void raft_node_set_next_idx(raft_node_t* me_, int nextIdx)
{
    raft_node_private_t* me = (raft_node_private_t*)me_;
    /* log index begins at 1 */
    me->next_idx = nextIdx < 1 ? 1 : nextIdx;
}

int raft_node_get_match_idx(raft_node_t* me_)
{
    raft_node_private_t* me = (raft_node_private_t*)me_;
    return me->match_idx;
}

void raft_node_set_match_idx(raft_node_t* me_, int matchIdx)
{
    raft_node_private_t* me = (raft_node_private_t*)me_;
    me->match_idx = matchIdx;
}

void* raft_node_get_udata(raft_node_t* me_)
{
    raft_node_private_t* me = (raft_node_private_t*)me_;
    return me->udata;
}

void raft_node_set_udata(raft_node_t* me_, void* udata)
{
    raft_node_private_t* me = (raft_node_private_t*)me_;
    me->udata = udata;
}

void raft_node_vote_for_me(raft_node_t* me_, const int vote)
{
    raft_node_private_t* me = (raft_node_private_t*)me_;
    me->voted_for_me = vote;
}

int raft_node_has_vote_for_me(raft_node_t* me_)
{
    raft_node_private_t* me = (raft_node_private_t*)me_;
    return me->voted_for_me;
}

int raft_node_get_id(raft_node_t* me_)
{
    raft_node_private_t* me = (raft_node_private_t*)me_;
    return me->id;
}
