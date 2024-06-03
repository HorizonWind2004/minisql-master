#ifndef MINISQL_LOG_REC_H
#define MINISQL_LOG_REC_H

#include <unordered_map>
#include <utility>

#include "common/config.h"
#include "common/rowid.h"
#include "record/row.h"

enum class LogRecType {
    kInvalid,
    kInsert,
    kDelete,
    kUpdate,
    kBegin,
    kCommit,
    kAbort,
};

// used for testing only
using KeyType = std::string;
using ValType = int32_t;

/**
 * TODO: Student Implement
 */
struct LogRec {
    LogRec() = default;

    LogRecType type_{LogRecType::kInvalid};
    lsn_t lsn_{INVALID_LSN};
    lsn_t prev_lsn_{INVALID_LSN};
    txn_id_t txn_id_{INVALID_TXN_ID};
    KeyType del_key_{};
    ValType del_val_{};
    KeyType old_key_{};
    ValType old_val_{-1};
    KeyType new_key_{};
    ValType new_val_{-1};
    KeyType ins_key_{};
    ValType ins_val_{-1};
    /* used for testing only */
    static std::unordered_map<txn_id_t, lsn_t> prev_lsn_map_;
    static lsn_t next_lsn_;
};

std::unordered_map<txn_id_t, lsn_t> LogRec::prev_lsn_map_ = {};
lsn_t LogRec::next_lsn_ = 0;

typedef std::shared_ptr<LogRec> LogRecPtr;

/**
 * TODO: Student Implement
 * Done.
 */
static LogRecPtr CreateInsertLog(txn_id_t txn_id, KeyType ins_key, ValType ins_val) {
  LogRecPtr new_log = std::make_shared<LogRec>();
  new_log->type_ = LogRecType::kInsert;
  new_log->lsn_ = LogRec::next_lsn_++;
  new_log->prev_lsn_ = (LogRec::prev_lsn_map_.count(txn_id) ? LogRec::prev_lsn_map_[txn_id] : INVALID_LSN);
  LogRec::prev_lsn_map_[txn_id] = new_log->lsn_;
  new_log->txn_id_ = txn_id;
  new_log->ins_key_ = ins_key;
  new_log->ins_val_ = ins_val;
  return new_log;
}

/**
 * TODO: Student Implement
 * Done.
 */
static LogRecPtr CreateDeleteLog(txn_id_t txn_id, KeyType del_key, ValType del_val) {
  LogRecPtr new_log = std::make_shared<LogRec>();
  new_log->type_ = LogRecType::kDelete;
  new_log->lsn_ = LogRec::next_lsn_++;
  new_log->prev_lsn_ = (LogRec::prev_lsn_map_.count(txn_id) ? LogRec::prev_lsn_map_[txn_id] : INVALID_LSN);
  LogRec::prev_lsn_map_[txn_id] = new_log->lsn_;
  new_log->txn_id_ = txn_id;
  new_log->del_key_ = del_key;
  new_log->del_val_ = del_val;
  return new_log;
}

/**
 * TODO: Student Implement
 * Done.
 */
static LogRecPtr CreateUpdateLog(txn_id_t txn_id, KeyType old_key, ValType old_val, KeyType new_key, ValType new_val) {
  LogRecPtr new_log = std::make_shared<LogRec>();
  new_log->type_ = LogRecType::kUpdate;
  new_log->lsn_ = LogRec::next_lsn_++;
  new_log->prev_lsn_ = (LogRec::prev_lsn_map_.count(txn_id) ? LogRec::prev_lsn_map_[txn_id] : INVALID_LSN);
  LogRec::prev_lsn_map_[txn_id] = new_log->lsn_;
  new_log->txn_id_ = txn_id;
  new_log->old_key_ = old_key;
  new_log->old_val_ = old_val;
  new_log->new_key_ = new_key;
  new_log->new_val_ = new_val;
  return new_log;
}

/**
 * TODO: Student Implement
 * Done.
 */
static LogRecPtr CreateBeginLog(txn_id_t txn_id) {
  LogRecPtr new_log = std::make_shared<LogRec>();
  new_log->type_ = LogRecType::kBegin;
  new_log->lsn_ = LogRec::next_lsn_++;
  new_log->prev_lsn_ = (LogRec::prev_lsn_map_.count(txn_id) ? LogRec::prev_lsn_map_[txn_id] : INVALID_LSN);
  LogRec::prev_lsn_map_[txn_id] = new_log->lsn_;
  new_log->txn_id_ = txn_id;
  return new_log;
}

/**
 * TODO: Student Implement
 * Done.
 */
static LogRecPtr CreateCommitLog(txn_id_t txn_id) {
  LogRecPtr new_log = std::make_shared<LogRec>();
  new_log->type_ = LogRecType::kCommit;
  new_log->lsn_ = LogRec::next_lsn_++;
  new_log->prev_lsn_ = (LogRec::prev_lsn_map_.count(txn_id) ? LogRec::prev_lsn_map_[txn_id] : INVALID_LSN);
  LogRec::prev_lsn_map_[txn_id] = new_log->lsn_;
  new_log->txn_id_ = txn_id;
  return new_log;
}

/**
 * TODO: Student Implement
 * Done.
 */
static LogRecPtr CreateAbortLog(txn_id_t txn_id) {
  LogRecPtr new_log = std::make_shared<LogRec>();
  new_log->type_ = LogRecType::kAbort;
  new_log->lsn_ = LogRec::next_lsn_++;
  new_log->prev_lsn_ = (LogRec::prev_lsn_map_.count(txn_id) ? LogRec::prev_lsn_map_[txn_id] : INVALID_LSN);
  LogRec::prev_lsn_map_[txn_id] = new_log->lsn_;
  new_log->txn_id_ = txn_id;
  return new_log;
}

#endif  // MINISQL_LOG_REC_H
