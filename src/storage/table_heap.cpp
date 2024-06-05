// #include "storage/table_heap.h"

// /**
//  * TODO: Student Implement
//  * done
//  */
// bool TableHeap::InsertTuple(Row &row, Txn *txn) {
//   uint32_t serialized_size=row.GetSerializedSize(schema_);
//   if(serialized_size>=TablePage::SIZE_MAX_ROW)return 0;
//   page_id_t cur_page_id=first_page_id_,prev_page_id=INVALID_PAGE_ID;
//   TablePage *page=reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(cur_page_id));
//   if(page==nullptr)return 0;
//   page->WLatch();
//   bool p;
//   while(!(p=page->InsertTuple(row,schema_,txn,lock_manager_,log_manager_)))
//   {
//     page->WUnlatch();
//     buffer_pool_manager_->UnpinPage(cur_page_id,0);
//     prev_page_id=cur_page_id;
//     cur_page_id=page->GetNextPageId();
//     // std::cout<<cur_page_id<<std::endl;
//     if(cur_page_id==INVALID_PAGE_ID)break;
//     page=reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(page->GetNextPageId()));
//     if(page==nullptr)break;
//     page->WLatch();
//   }
//   if(p)
//   {
//     page->WUnlatch();
//     // std::cout<<cur_page_id<<std::endl;
//     buffer_pool_manager_->UnpinPage(cur_page_id,1);
//     return 1;
//   }
//   // std::cout<<cur_page_id<<std::endl;
//   page=reinterpret_cast<TablePage *>(buffer_pool_manager_->NewPage(cur_page_id));
//   // std::cout<<cur_page_id<<std::endl;
//   if(page==nullptr)return 0;
//   page->Init(cur_page_id,prev_page_id, log_manager_,txn);
//   page->WLatch();
//   page->InsertTuple(row,schema_,txn,lock_manager_,log_manager_);
//   page->WUnlatch();
//   buffer_pool_manager_->UnpinPage(cur_page_id,1);

//   page=reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(prev_page_id));
//   page->WLatch();
//   page->SetNextPageId(cur_page_id);
//   page->WUnlatch();
//   buffer_pool_manager_->UnpinPage(prev_page_id,1);
//   return 1;
// }

// bool TableHeap::MarkDelete(const RowId &rid, Txn *txn) {
//   // std::cout<<111<<std::endl;
//   // std::cout<<rid.GetPageId()<<std::endl;
//   // Find the page which contains the tuple.
//   auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
//   // If the page could not be found, then abort the recovery.

//   // std::cout<<222<<std::endl;
//   if (page == nullptr) {
//     return false;
//   }
//   // Otherwise, mark the tuple as deleted.
//   page->WLatch();
//   page->MarkDelete(rid, txn, lock_manager_, log_manager_);
//   page->WUnlatch();
//   buffer_pool_manager_->UnpinPage(page->GetTablePageId(), true);

//   return true;
// }

// /**
//  * TODO: Student Implement
//  * done
//  */
// bool TableHeap::UpdateTuple(Row &row, const RowId &rid, Txn *txn) {
//   auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
//   if(!page)return 0;
//   Row old_row(rid);
//   page->WLatch();
//   bool is_succeeded=page->UpdateTuple(row, &old_row, schema_, txn, lock_manager_, log_manager_);
//   page->WUnlatch();
//   if(is_succeeded)
//   {
//     buffer_pool_manager_->UnpinPage(rid.GetPageId(),true);
//     return true;
//   }
//   else
//   {
//     buffer_pool_manager_->UnpinPage(rid.GetPageId(),false);
//     if(InsertTuple(row,txn))
//     {
//       MarkDelete(rid,txn);
//       return true;
//     }
//   }
//   // buffer_pool_manager_->UnpinPage(rid.GetPageId(),is_succeeded);
//   return is_succeeded;
// }

// /**
//  * TODO: Student Implement
//  * done
//  */
// void TableHeap::ApplyDelete(const RowId &rid, Txn *txn) {
//   // Step1: Find the page which contains the tuple.
//   auto page=reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
//   if(!page)
//   {
//     LOG(WARNING) << "Failed to get page in ApplyDelete"<<std::endl;
//     return;
//   }
//   // Step2: Delete the tuple from the page.
//   page->WLatch();
//   page->ApplyDelete(rid, txn, log_manager_);
//   page->WUnlatch();
//   buffer_pool_manager_->UnpinPage(rid.GetPageId(),true);
// }

// void TableHeap::RollbackDelete(const RowId &rid, Txn *txn) {
//   // Find the page which contains the tuple.
//   auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
//   assert(page != nullptr);
//   // Rollback to delete.
//   page->WLatch();
//   page->RollbackDelete(rid, txn, log_manager_);
//   page->WUnlatch();
//   buffer_pool_manager_->UnpinPage(page->GetTablePageId(), true);
// }

// /**
//  * TODO: Student Implement
//  * done
//  */
// bool TableHeap::GetTuple(Row *row, Txn *txn) {
//   auto page=reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(row->GetRowId().GetPageId()));
//   if(!page)return 0;
//   page->RLatch();
//   row->destroy();
//   // std::cout<<111<<std::endl;
//   bool is_found=page->GetTuple(row,schema_, txn, lock_manager_);
//   page->RUnlatch();
//   buffer_pool_manager_->UnpinPage(row->GetRowId().GetPageId(),false);
//   // std::cout<<222<<std::endl;
//   return is_found;
// }

// bool TableHeap::GetNextTupleRid(const RowId &rid, RowId *nxt_rid, Txn *txn)
// {
//   // std::cout<<111<<std::endl;
//   page_id_t cur_page_id=rid.GetPageId();
//   TablePage *page=reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(cur_page_id));
//   // std::cout<<222<<std::endl;
//   if(!page)
//   {
//     return 0;
//   }
//   page->RLatch();
//   bool is_found=page->GetNextTupleRid(rid, nxt_rid);
//   while(!is_found)
//   {
//     page_id_t nxt_page_id=page->GetNextPageId();
//     page->RUnlatch();
//     buffer_pool_manager_->UnpinPage(cur_page_id,false);
//     if(nxt_page_id==INVALID_PAGE_ID)
//     {
//   // std::cout<<333<<std::endl;
//       return 0;
//     }
//     cur_page_id=nxt_page_id;
//     page=reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(cur_page_id));
//     if(!page)
//     {
//   // std::cout<<444<<std::endl;
//       return 0;
//     }
//     page->RLatch();
//     is_found=page->GetFirstTupleRid(nxt_rid);
//   }
//   page->RUnlatch();
//   buffer_pool_manager_->UnpinPage(cur_page_id, false);
//   // std::cout<<222<<std::endl;
//   return 1;
// }


// void TableHeap::DeleteTable(page_id_t page_id) {
//   if (page_id != INVALID_PAGE_ID) {
//     auto temp_table_page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(page_id));  // 删除table_heap
//     if (temp_table_page->GetNextPageId() != INVALID_PAGE_ID)
//       DeleteTable(temp_table_page->GetNextPageId());
//     buffer_pool_manager_->UnpinPage(page_id, false);
//     buffer_pool_manager_->DeletePage(page_id);
//   } else {
//     DeleteTable(first_page_id_);
//   }
// }

// /**
//  * TODO: Student Implement
//  */
// TableIterator TableHeap::Begin(Txn *txn) {
//   page_id_t cur_page_id=GetFirstPageId();
//   while(cur_page_id!=INVALID_PAGE_ID)
//   {
//     TablePage *page=reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(cur_page_id));
//     if(!page)
//     {
//       LOG(WARNING) << "Failed to get page in ApplyDelete"<<std::endl;
//       return End();
//     }
//     page->RLatch();

//     RowId first_rid;
//     bool is_found=page->GetFirstTupleRid(&first_rid);
//     if(is_found)
//     {
//       TableIterator first_iterator(this, first_rid, txn);
//       page->RUnlatch();
//       buffer_pool_manager_->UnpinPage(cur_page_id,false);
//       return TableIterator(first_iterator);
//     }
//     page_id_t nxt_page_id=page->GetNextPageId();
//     page->RUnlatch();
//     buffer_pool_manager_->UnpinPage(cur_page_id, false);
//     cur_page_id=nxt_page_id;
//   }
//   return End();
// }

// /**
//  * TODO: Student Implement
//  * done
//  */
// TableIterator TableHeap::End() { return TableIterator(nullptr, RowId(INVALID_PAGE_ID, 0), nullptr); }

#include "storage/table_heap.h"

/**
 * TODO: Student Implement
 */
bool TableHeap::InsertTuple(Row &row, Txn *txn) {
	uint32_t serialized_size = row.GetSerializedSize(schema_);
	if (serialized_size >= TablePage::SIZE_MAX_ROW) {
		return false;
	}

	page_id_t page_id_ = first_page_id_;
	auto page_ = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(page_id_));
	if (page_ == nullptr) {
		return false;
	}

	page_->WLatch();
	while (!page_->InsertTuple(row, schema_, txn, lock_manager_, log_manager_)) {
		page_->WUnlatch();
		buffer_pool_manager_->UnpinPage(page_id_, false);
		page_ = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(page_->GetNextPageId()));
		if (page_ != nullptr) {
			page_id_ = page_->GetTablePageId();
			page_->WLatch();
		} else {
			break;
		}
	}
	if (page_ != nullptr) {
		page_->WUnlatch();
		buffer_pool_manager_->UnpinPage(page_id_, true);
		return true;
	}

	page_id_t new_page_id_;
	auto new_page_ = reinterpret_cast<TablePage *>(buffer_pool_manager_->NewPage(new_page_id_));
	if (new_page_ == nullptr) {
		return false;
	}
	new_page_->Init(new_page_id_, page_id_, log_manager_, txn);
	new_page_->WLatch();
	new_page_->InsertTuple(row, schema_, txn, lock_manager_, log_manager_);
	new_page_->WUnlatch();
	buffer_pool_manager_->UnpinPage(new_page_id_, true);

	page_ = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(page_id_));
	page_->WLatch();
	page_->SetNextPageId(new_page_id_);
	page_->WUnlatch();
	buffer_pool_manager_->UnpinPage(page_id_, true);

	return true;
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
 */
bool TableHeap::UpdateTuple(Row &row, const RowId &rid, Txn *txn) {
	row.SetRowId(rid);
	page_id_t page_id_ = rid.GetPageId();
	auto page_ = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(page_id_));
	if (page_ == nullptr) {
		return false;
	}

	page_->WLatch();
	Row* old_row = new Row(rid);
	int status = page_->UpdateTuple(row, old_row, schema_, txn, lock_manager_, log_manager_);
	delete old_row;
	page_->WUnlatch();

	if (status == 1) {
		buffer_pool_manager_->UnpinPage(page_id_, true);
		return true;
	} else if (status == 0) {
		buffer_pool_manager_->UnpinPage(page_id_, false);
		if (InsertTuple(row, txn)) {
			MarkDelete(rid, txn);
			return true;
		}
	} else {
		buffer_pool_manager_->UnpinPage(page_id_, false);
	}
	return false;
}

/**
 * TODO: Student Implement
 */
void TableHeap::ApplyDelete(const RowId &rid, Txn *txn) {
	// Step1: Find the page which contains the tuple.
	page_id_t page_id_ = rid.GetPageId();
	auto page_ = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(page_id_));
	if (page_ == nullptr) {
		return;
	}
	// Step2: Delete the tuple from the page.
	page_->WLatch();
	page_->ApplyDelete(rid, txn, log_manager_);
	page_->WUnlatch();
	buffer_pool_manager_->UnpinPage(page_id_, true);
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
 */
bool TableHeap::GetTuple(Row *row, Txn *txn) {
	page_id_t page_id_ = row->GetRowId().GetPageId();
	auto page_ = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(page_id_));
	if (page_ == nullptr) {
		return false;
	}

	page_->RLatch();
	row->destroy();
	bool Success = page_->GetTuple(row, schema_, txn, lock_manager_);
	page_->RUnlatch();
	buffer_pool_manager_->UnpinPage(page_id_, false);

	return Success;
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
	page_id_t page_id_ = first_page_id_;
	auto page_ = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(page_id_));

	while (page_ != nullptr) {
		page_->RLatch();
		RowId first_rid_;
		if (page_->GetFirstTupleRid(&first_rid_)) {
			page_->RUnlatch();
			buffer_pool_manager_->UnpinPage(page_id_, false);
			return TableIterator(this, first_rid_, txn);
		}
		page_->RUnlatch();
		buffer_pool_manager_->UnpinPage(page_id_, false);
		page_id_ = page_->GetNextPageId();
		page_ = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(page_id_));
	}

	return End();
}

/**
 * TODO: Student Implement
 */
TableIterator TableHeap::End() {
	return TableIterator(this, RowId(INVALID_PAGE_ID, 0), nullptr);
}
