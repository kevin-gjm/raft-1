#ifndef RAFT_LOG_H_
#define RAFT_LOG_H_

typedef void* log_t;

log_t* log_new();

void log_set_callbacks(log_t* me_, raft_cbs_t* funcs, void* raft);

void log_free(log_t* me_);

void log_clear(log_t* me_);

/**
 * Add entry to log.
 * Don't add entry if we've already added this entry (based off ID)
 * Don't add entries with ID=0 
 * @return 0 if unsucessful; 1 otherwise */
int log_append_entry(log_t* me_, raft_entry_t* c);

/**
 * @return number of entries held within log */
unsigned long log_count(log_t* me_);

/**
 * Delete all logs from this log onwards */
int log_delete(log_t* me_, unsigned long idx);

/**
 * Empty the queue. */
void log_empty(log_t * me_);

/**
 * Remove oldest entry. Set *etyp to oldest entry on success. */
int log_poll(log_t * me_);

/** Get an array of entries from this index onwards.
 * This is used for batching.
 */
raft_entry_t* log_get_from_idx(log_t* me_, unsigned long idx, unsigned long *n_etys);

int log_get_at_idx(log_t* me_, unsigned long idx,raft_entry_t* ety);

unsigned long log_get_current_idx(log_t* me_);

int log_load_from_snapshot(log_t *me_, unsigned long idx, unsigned long term);

unsigned long log_get_base(log_t* me_);

#endif /* RAFT_LOG_H_ */
