
#include "concurrency/lock_manager.h"

#include <iostream>

#include "common/rowid.h"
#include "concurrency/txn.h"
#include "concurrency/txn_manager.h"

void LockManager::SetTxnMgr(TxnManager *txn_mgr) { txn_mgr_ = txn_mgr; }

/**
 * TODO: Done
 */
bool LockManager::LockShared(Txn *txn, const RowId &rid) {
  std::unique_lock<std::mutex>lock(latch_);//防止并发访问
  if(txn->GetIsolationLevel()==IsolationLevel::kReadUncommitted)//ReadUncommitted的level不加锁
  {
    txn->SetState(TxnState::kAborted);
    throw TxnAbortException(txn->GetTxnId(),AbortReason::kLockSharedOnReadUncommitted);
  }
  LockPrepare(txn, rid);

  LockRequestQueue &now_queue=lock_table_[rid];
  now_queue.EmplaceLockRequest(txn->GetTxnId(),LockMode::kShared);

  if(now_queue.is_writing_)
  {
    now_queue.cv_.wait(lock, [&now_queue, txn]{return !now_queue.is_writing_ || txn->GetState()==TxnState::kAborted;});
  }

  CheckAbort(txn,now_queue);

  txn->GetSharedLockSet().insert(rid);
  ++now_queue.sharing_cnt_;
  now_queue.GetLockRequestIter(txn->GetTxnId())->granted_=LockMode::kShared;//实际分配share锁

  return true;
}

/**
 * TODO: done
 */
bool LockManager::LockExclusive(Txn *txn, const RowId &rid) {
    std::unique_lock<std::mutex>lock(latch_);
    
    LockPrepare(txn, rid);

    LockRequestQueue &now_queue=lock_table_[rid];
    now_queue.EmplaceLockRequest(txn->GetTxnId(),LockMode::kExclusive);

    if(now_queue.is_writing_||now_queue.sharing_cnt_>0)
    {
      now_queue.cv_.wait(lock,[&now_queue, txn]{return txn->GetState()==TxnState::kAborted||(!now_queue.is_writing_&&now_queue.sharing_cnt_==0);});
    }
    CheckAbort(txn, now_queue);

    txn->GetExclusiveLockSet().insert(rid);
    now_queue.is_writing_=true;
    now_queue.GetLockRequestIter(txn->GetTxnId())->granted_=LockMode::kExclusive;

    return true;
}

/**
 * TODO: Done
 */
bool LockManager::LockUpgrade(Txn *txn, const RowId &rid) {
    std::unique_lock<std::mutex>lock(latch_);
    
    LockPrepare(txn,rid);

    LockRequestQueue &now_queue=lock_table_[rid];
    if(now_queue.is_upgrading_)
    {
      txn->SetState(TxnState::kAborted);
      throw TxnAbortException(txn->GetTxnId(),AbortReason::kUpgradeConflict);
    }

    auto iter=now_queue.GetLockRequestIter(txn->GetTxnId());

    if(iter->lock_mode_==LockMode::kExclusive&&iter->granted_==LockMode::kExclusive)return true;//already grant Exclusive

    iter->lock_mode_=LockMode::kExclusive;

    if(now_queue.is_writing_||now_queue.sharing_cnt_>1)
    {
      now_queue.is_upgrading_=true;
      now_queue.cv_.wait(lock,[&now_queue, txn]{return txn->GetState()==TxnState::kAborted||(!now_queue.is_writing_&&now_queue.sharing_cnt_==1);});
    }

    now_queue.is_upgrading_=false;
    CheckAbort(txn,now_queue);

    iter->granted_=LockMode::kExclusive;
    now_queue.sharing_cnt_=0;
    now_queue.is_writing_=true;

    txn->GetSharedLockSet().erase(rid);
    txn->GetExclusiveLockSet().insert(rid);

    return true;
}

/**
 * TODO: Student Implement
 */
bool LockManager::Unlock(Txn *txn, const RowId &rid) {
  std::unique_lock<std::mutex>lock(latch_);

  txn->GetSharedLockSet().erase(rid);
  txn->GetExclusiveLockSet().erase(rid);

  LockRequestQueue &now_queue=lock_table_[rid];
  auto iter=now_queue.GetLockRequestIter(txn->GetTxnId());
  LockMode now_lock_mode=iter->granted_;

  now_queue.EraseLockRequest(txn->GetTxnId());

  if(txn->GetState()==TxnState::kGrowing&&!(txn->GetIsolationLevel()==IsolationLevel::kReadCommitted&&now_lock_mode==LockMode::kShared))
  {
    txn->SetState(TxnState::kShrinking);
  }
  if(now_lock_mode==LockMode::kShared)
  {
    --now_queue.sharing_cnt_;
  }
  else if(now_lock_mode==LockMode::kExclusive)
  {
    now_queue.is_writing_=false;
  }
  now_queue.cv_.notify_all();
  
  return true;
}

/**
 * TODO: Done
 */
void LockManager::LockPrepare(Txn *txn, const RowId &rid) {//rid加入lock_table_中
  if(txn->GetState()==TxnState::kShrinking)
  {
    txn->SetState(TxnState::kAborted);
    throw TxnAbortException(txn->GetTxnId(),AbortReason::kLockOnShrinking);
  }
  if(lock_table_.find(rid)==lock_table_.end())
  {
    // lock_table_.emplace(rid,LockRequestQueue());
    lock_table_.emplace(std::piecewise_construct, std::forward_as_tuple(rid), std::forward_as_tuple());
  }
}

/**
 * TODO: Done
 */
void LockManager::CheckAbort(Txn *txn, LockManager::LockRequestQueue &req_queue) {
  if(txn->GetState()==TxnState::kAborted)
  {
    req_queue.EraseLockRequest(txn->GetTxnId());
    throw TxnAbortException(txn->GetTxnId(),AbortReason::kDeadlock);//等待资源时终止，为死锁检测
  }
}

/**
 * TODO: Done
 */
void LockManager::AddEdge(txn_id_t t1, txn_id_t t2) {waits_for_[t1].insert(t2);}

/**
 * TODO: Done
 */
void LockManager::RemoveEdge(txn_id_t t1, txn_id_t t2) {waits_for_[t1].erase(t2);}

/**
 * TODO: Done
 */
bool LockManager::HasCycle(txn_id_t &newest_tid_in_cycle) {
  visited_set_.clear();
  while(!visited_path_.empty())visited_path_.pop();
  
  std::set<txn_id_t>nodes;

  for(auto iter1:waits_for_)
  {
    nodes.insert(iter1.first);
    for(auto iter2:iter1.second)
    {
      nodes.insert(iter2);
    }
  }

  newest_tid_in_cycle=INVALID_TXN_ID;

  for(auto it:nodes)
  {
    if(visited_set_.find(it)!=visited_set_.end())continue;
    if(dfs(it))
    {
      while(!visited_path_.empty())
      {
        newest_tid_in_cycle=std::max(newest_tid_in_cycle,visited_path_.top());
        visited_path_.pop();
      }
      return true;
    }
  }
  
  return false;
}

bool LockManager::dfs(txn_id_t now)
{
  if(visited_set_.find(now)!=visited_set_.end())
  {
    return true;
  }

  visited_set_.insert(now);
  visited_path_.push(now);

  for(auto it:waits_for_[now])
  {
    if(dfs(it))return true;
  }

  visited_set_.erase(now);
  visited_path_.pop();

  return false;
}

void LockManager::DeleteNode(txn_id_t txn_id) {
    waits_for_.erase(txn_id);
    auto *txn = txn_mgr_->GetTransaction(txn_id);

    for(auto &iter:lock_table_)
    {
      for(auto &u:iter.second.req_list_)
      {
        if(u.txn_id_==txn_id&&u.granted_==LockMode::kNone)
        {
          for(auto &v:iter.second.req_list_)
          {
            if(v.granted_!=LockMode::kNone)
            {
              RemoveEdge(u.txn_id_,v.txn_id_);
            }
          }
        }
      }
    }
    
    for (auto &row_id: txn->GetSharedLockSet()) {
        for (auto &lock_req: lock_table_[row_id].req_list_) {
            if (lock_req.granted_ == LockMode::kNone) {
                RemoveEdge(lock_req.txn_id_, txn_id);
            }
        }
    }

    for (auto &row_id: txn->GetExclusiveLockSet()) {
        for (auto &lock_req: lock_table_[row_id].req_list_) {
            if (lock_req.granted_ == LockMode::kNone) {
                RemoveEdge(lock_req.txn_id_, txn_id);
            }
        }
    }
}

/**
 * TODO: Student Implement
 */
void LockManager::RunCycleDetection() {
  while(enable_cycle_detection_)
  {
    std::this_thread::sleep_for(cycle_detection_interval_);
    std::unique_lock<std::mutex> lock(latch_);
    std::unordered_map<txn_id_t, RowId> txn_to_rid;
    for(auto &iter:lock_table_)
    {
      for(auto &u:iter.second.req_list_)
      {
        txn_to_rid[u.txn_id_]=iter.first;
        if(u.granted_==LockMode::kNone)
        {
          for(auto &v:iter.second.req_list_)
          {
            if(v.granted_!=LockMode::kNone)
            {
              AddEdge(u.txn_id_,v.txn_id_);
            }
          }
        }
      }
    }
    txn_id_t to_be_deleted_id=INVALID_TXN_ID;
    while(HasCycle(to_be_deleted_id))
    {
      Txn *to_be_deleted=txn_mgr_->GetTransaction(to_be_deleted_id);
      to_be_deleted->SetState(TxnState::kAborted);
      DeleteNode(to_be_deleted_id);
      lock_table_[txn_to_rid[to_be_deleted_id]].cv_.notify_all();
    }
    waits_for_.clear();
  }
}


/**
 * TODO: Done
 */
std::vector<std::pair<txn_id_t, txn_id_t>> LockManager::GetEdgeList() {
    std::vector<std::pair<txn_id_t, txn_id_t>> result;
    for(auto iter:waits_for_)
    {
      for(auto iter2:iter.second)
      {
        result.push_back(std::make_pair(iter.first,iter2));
      }
    }
    return result;
}
