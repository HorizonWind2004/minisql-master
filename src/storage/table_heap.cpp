#include "storage/table_heap.h"

/**
 * TODO: Student Implement
 * done
 */
bool TableHeap::InsertTuple(Row &row, Txn *txn) {
  page_id_t cur_page_id=first_page_id_,prev_page_id=INVALID_PAGE_ID;
  TablePage *page=nullptr;
  while(cur_page_id!=INVALID_PAGE_ID)
  {
    page=reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(cur_page_id));
    if(!page)return 0;//failed to fetch page
    page->RLatch();
    if(page->GetFreeSpaceRemaining_public()>=row.GetSerializedSize(schema_))
    {
      page->RUnlatch();
      break;
    }
    page->RUnlatch();
    prev_page_id=cur_page_id;
    cur_page_id=page->GetNextPageId();
    buffer_pool_manager_->UnpinPage(prev_page_id,false);
  }
  //create a new page
  if(cur_page_id==INVALID_PAGE_ID)
  {
    page=reinterpret_cast<TablePage *>(buffer_pool_manager_->NewPage(cur_page_id));
    if(!page)return 0;
    page->Init(cur_page_id,prev_page_id,log_manager_,txn);
  }
  page->WLatch();
  bool is_succeeded=page->InsertTuple(row, schema_, txn, lock_manager_, log_manager_);
  page->WUnlatch();
  buffer_pool_manager_->UnpinPage(cur_page_id, is_succeeded);
  return is_succeeded;
}

bool TableHeap::MarkDelete(const RowId &rid, Txn *txn) {
  // Find the page which contains the tuple.
  auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  // If the page could not be found, then abort the recovery.
  if (page == nullptr) {
    return false;
  }
  // Otherwise, mark the tuple as deleted.
  page->WLatch();
  page->MarkDelete(rid, txn, lock_manager_, log_manager_);
  page->WUnlatch();
  buffer_pool_manager_->UnpinPage(page->GetTablePageId(), true);
  return true;
}

/**
 * TODO: Student Implement
 * done
 */
bool TableHeap::UpdateTuple(Row &row, const RowId &rid, Txn *txn) {
  auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  if(!page)return 0;
  Row old_row(rid);
  page->WLatch();
  bool is_succeeded=page->UpdateTuple(row, &old_row, schema_, txn, lock_manager_, log_manager_);
  page->WUnlatch();
  buffer_pool_manager_->UnpinPage(rid.GetPageId(),is_succeeded);
  return is_succeeded;
}

/**
 * TODO: Student Implement
 * done
 */
void TableHeap::ApplyDelete(const RowId &rid, Txn *txn) {
  // Step1: Find the page which contains the tuple.
  auto page=reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  if(!page)
  {
    LOG(WARNING) << "Failed to get page in ApplyDelete"<<std::endl;
    return;
  }
  // Step2: Delete the tuple from the page.
  page->WLatch();
  page->ApplyDelete(rid, txn, log_manager_);
  page->WUnlatch();
  buffer_pool_manager_->UnpinPage(rid.GetPageId(),true);
}

void TableHeap::RollbackDelete(const RowId &rid, Txn *txn) {
  // Find the page which contains the tuple.
  auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  assert(page != nullptr);
  // Rollback to delete.
  page->WLatch();
  page->RollbackDelete(rid, txn, log_manager_);
  page->WUnlatch();
  buffer_pool_manager_->UnpinPage(page->GetTablePageId(), true);
}

/**
 * TODO: Student Implement
 * done
 */
bool TableHeap::GetTuple(Row *row, Txn *txn) {
  auto page=reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(row->GetRowId().GetPageId()));
  if(!page)return 0;
  page->RLatch();
  bool is_found=page->GetTuple(row,schema_, txn, lock_manager_);
  page->RUnlatch();
  buffer_pool_manager_->UnpinPage(row->GetRowId().GetPageId(),false);
  return is_found;
}

bool TableHeap::GetNextTupleRid(const RowId &rid, RowId *nxt_rid, Txn *txn)
{
  page_id_t cur_page_id=rid.GetPageId();
  TablePage *page=reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(cur_page_id));
  if(!page)
  {
    return 0;
  }
  page->RLatch();
  bool is_found=page->GetNextTupleRid(rid, nxt_rid);
  while(!is_found)
  {
    page_id_t nxt_page_id=page->GetNextPageId();
    page->RUnlatch();
    buffer_pool_manager_->UnpinPage(cur_page_id,false);
    if(nxt_page_id==INVALID_PAGE_ID)
    {
      return 0;
    }
    cur_page_id=nxt_page_id;
    page=reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(cur_page_id));
    if(!page)
    {
      return 0;
    }
    page->RLatch();
    is_found=page->GetFirstTupleRid(nxt_rid);
  }
  page->RUnlatch();
  buffer_pool_manager_->UnpinPage(cur_page_id, false);
  return 1;
}


void TableHeap::DeleteTable(page_id_t page_id) {
  if (page_id != INVALID_PAGE_ID) {
    auto temp_table_page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(page_id));  // 删除table_heap
    if (temp_table_page->GetNextPageId() != INVALID_PAGE_ID)
      DeleteTable(temp_table_page->GetNextPageId());
    buffer_pool_manager_->UnpinPage(page_id, false);
    buffer_pool_manager_->DeletePage(page_id);
  } else {
    DeleteTable(first_page_id_);
  }
}

/**
 * TODO: Student Implement
 */
TableIterator TableHeap::Begin(Txn *txn) {
  page_id_t cur_page_id=GetFirstPageId();
  while(cur_page_id!=INVALID_PAGE_ID)
  {
    TablePage *page=reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(cur_page_id));
    if(!page)
    {
      LOG(WARNING) << "Failed to get page in ApplyDelete"<<std::endl;
      return End();
    }
    page->RLatch();

    RowId first_rid;
    bool is_found=page->GetFirstTupleRid(&first_rid);
    if(is_found)
    {
      TableIterator first_iterator(this, first_rid, txn);
      page->RUnlatch();
      buffer_pool_manager_->UnpinPage(cur_page_id,false);
      return TableIterator(first_iterator);
    }
    page_id_t nxt_page_id=page->GetNextPageId();
    page->RUnlatch();
    buffer_pool_manager_->UnpinPage(cur_page_id, false);
    cur_page_id=nxt_page_id;
  }
  return End();
}

/**
 * TODO: Student Implement
 * done
 */
TableIterator TableHeap::End() { return TableIterator(nullptr, RowId(INVALID_PAGE_ID, 0), nullptr); }
