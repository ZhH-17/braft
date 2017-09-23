// libraft - Quorum-based replication of states across machines.
// Copyright (c) 2017 Baidu.com, Inc. All Rights Reserved

// Author: Xiong Kai (xiongkai@baidu.com)
// Date: 2017/08/25 16:15:24

#ifndef  PUBLIC_RAFT_SNAPSHOT_THROTTLE_H
#define  PUBLIC_RAFT_SNAPSHOT_THROTTLE_H

#include <base/memory/ref_counted.h>                // base::RefCountedThreadSafe
#include "raft/util.h"

namespace raft {

// Abstract class with the function of throttling during heavy disk reading/writing
class SnapshotThrottle : public base::RefCountedThreadSafe<SnapshotThrottle> {
public:
    SnapshotThrottle() {}
    virtual ~SnapshotThrottle() {}
    // Get available throughput after throttled 
    // Must be thread-safe
    virtual size_t throttled_by_throughput(int64_t bytes) = 0;
private:
    DISALLOW_COPY_AND_ASSIGN(SnapshotThrottle);
    friend class base::RefCountedThreadSafe<SnapshotThrottle>;
};

// SnapshotThrottle with throughput threshold used in install_snapshot
class ThroughputSnapshotThrottle : public SnapshotThrottle {
public:
    ThroughputSnapshotThrottle(int64_t throttle_throughput_bytes, int64_t check_cycle);
    int64_t get_throughput() const { return _throttle_throughput_bytes; }
    int64_t get_cycle() const { return _check_cycle; }
    size_t throttled_by_throughput(int64_t bytes);

private:
    ~ThroughputSnapshotThrottle();
    // user defined throughput threshold for raft, bytes per second
    int64_t _throttle_throughput_bytes;
    // user defined check cycles of throughput per second
    int64_t _check_cycle;
    int64_t _last_throughput_check_time_us;
    int64_t _cur_throughput_bytes;
    raft_mutex_t _mutex;
};

inline int64_t caculate_check_time_us(int64_t current_time_us, 
        int64_t check_cycle) {
    int64_t base_aligning_time_us = 1000 * 1000 / check_cycle;
    return current_time_us / base_aligning_time_us * base_aligning_time_us;
}

} // namespace raft

#endif  // PUBLIC_RAFT_SNAPSHOT_THROTTLE_H