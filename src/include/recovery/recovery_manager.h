#ifndef MINISQL_RECOVERY_MANAGER_H
#define MINISQL_RECOVERY_MANAGER_H

#include <map>
#include <unordered_map>
#include <vector>
#include <cassert>
#include "recovery/log_rec.h"

using KvDatabase = std::unordered_map<KeyType, ValType>;
using ATT = std::unordered_map<txn_id_t, lsn_t>;

struct CheckPoint {
    lsn_t checkpoint_lsn_{INVALID_LSN};
    ATT active_txns_{};
    KvDatabase persist_data_{};

    inline void AddActiveTxn(txn_id_t txn_id, lsn_t last_lsn) { active_txns_[txn_id] = last_lsn; }

    inline void AddData(KeyType key, ValType val) { persist_data_.emplace(std::move(key), val); }
};

class RecoveryManager {
public:
    /**
    * TODO: Student Implement
    * Done.
    */
    void Init(CheckPoint &last_checkpoint) {
      active_txns_ = last_checkpoint.active_txns_;
      data_ = last_checkpoint.persist_data_;
      persist_lsn_ = last_checkpoint.checkpoint_lsn_;
    }

    /**
    * TODO: Student Implement
    * Done.
    */
    void RedoPhase() {
      for (auto &now_log : log_recs_) {
        // printf("redo : %d\n", now_log.first);
        if (now_log.first < persist_lsn_) {
          continue;
        }
        switch (now_log.second->type_) { 
          case LogRecType::kInvalid: {
            assert(("Invalid log rec!", 0));
            break;
          }
          case LogRecType::kInsert: {
            data_[now_log.second->ins_key_] = now_log.second->ins_val_;
            break;
          }
          case LogRecType::kDelete: {
            data_.erase(now_log.second->del_key_);
            break;
          }
          case LogRecType::kUpdate: {
            data_.erase(now_log.second->old_key_);
            data_[now_log.second->new_key_] = now_log.second->new_val_;
            break;
          }
          case LogRecType::kBegin: {
            active_txns_[now_log.second->txn_id_] = now_log.first;
            break;
          }
          case LogRecType::kCommit: {
            active_txns_.erase(now_log.second->txn_id_);
            break;
          }
          case LogRecType::kAbort: {
            active_txns_.erase(now_log.second->txn_id_);
            UndoPhase_(now_log.first);
            break;
          }
        }
      }
    }

    void UndoPhase_(int tmp) {
      while (tmp != INVALID_TXN_ID) {
        // printf("%d\n", tmp);
        LogRecPtr now_log = log_recs_[tmp];
        switch (now_log->type_) {
          case LogRecType::kInvalid: {
            assert(("Invalid log rec!", 0));
            break;
          }
          case LogRecType::kInsert: {
            data_.erase(now_log->ins_key_);
            break;
          }
          case LogRecType::kDelete: {
            data_[now_log->del_key_] = now_log->del_val_;
            break;
          }
          case LogRecType::kUpdate: {
            data_.erase(now_log->new_key_);
            data_[now_log->old_key_] = now_log->old_val_;
            break;
          }
          case LogRecType::kBegin: {
            break;
          }
          case LogRecType::kCommit: {
            break;
          }
          case LogRecType::kAbort: {
            break;
          }
        }
        tmp = now_log->prev_lsn_;
      }
    }

    /**
    * TODO: Student Implement
    */
    void UndoPhase() {
      for (auto txn_ : active_txns_) {
        UndoPhase_(LogRec::prev_lsn_map_[txn_.first]);
      }
    }

    // used for test only
    void AppendLogRec(LogRecPtr log_rec) { log_recs_.emplace(log_rec->lsn_, log_rec); }

    // used for test only
    inline KvDatabase &GetDatabase() { return data_; }

private:
    std::map<lsn_t, LogRecPtr> log_recs_{};
    lsn_t persist_lsn_{INVALID_LSN};
    ATT active_txns_{};
    KvDatabase data_{};  // all data in database
};

#endif  // MINISQL_RECOVERY_MANAGER_H
