#include "catalog/catalog.h"

void CatalogMeta::SerializeTo(char *buf) const {
  ASSERT(GetSerializedSize() <= PAGE_SIZE, "Failed to serialize catalog metadata to disk.");
  MACH_WRITE_UINT32(buf, CATALOG_METADATA_MAGIC_NUM);
  // std::cout<<"222"<<CATALOG_METADATA_MAGIC_NUM<<std::endl;
  // printf("%p\n",buf);
  buf += 4;
  MACH_WRITE_UINT32(buf, table_meta_pages_.size());
  buf += 4;
  MACH_WRITE_UINT32(buf, index_meta_pages_.size());
  buf += 4;
  for (auto iter : table_meta_pages_) {
    MACH_WRITE_TO(table_id_t, buf, iter.first);
    buf += 4;
    MACH_WRITE_TO(page_id_t, buf, iter.second);
    buf += 4;
  }
  for (auto iter : index_meta_pages_) {
    MACH_WRITE_TO(index_id_t, buf, iter.first);
    buf += 4;
    MACH_WRITE_TO(page_id_t, buf, iter.second);
    buf += 4;
  }
}

CatalogMeta *CatalogMeta::DeserializeFrom(char *buf) {
  // check valid
  uint32_t magic_num = MACH_READ_UINT32(buf);
  // printf("%p\n",buf);
  buf += 4;
  // std::cout<<"111"<<magic_num<<std::endl;
  ASSERT(magic_num == CATALOG_METADATA_MAGIC_NUM, "Failed to deserialize catalog metadata from disk.");
  // get table and index nums
  uint32_t table_nums = MACH_READ_UINT32(buf);
  buf += 4;
  uint32_t index_nums = MACH_READ_UINT32(buf);
  buf += 4;
  // create metadata and read value
  CatalogMeta *meta = new CatalogMeta();
  for (uint32_t i = 0; i < table_nums; i++) {
    auto table_id = MACH_READ_FROM(table_id_t, buf);
    buf += 4;
    auto table_heap_page_id = MACH_READ_FROM(page_id_t, buf);
    buf += 4;
    meta->table_meta_pages_.emplace(table_id, table_heap_page_id);
  }
  for (uint32_t i = 0; i < index_nums; i++) {
    auto index_id = MACH_READ_FROM(index_id_t, buf);
    buf += 4;
    auto index_page_id = MACH_READ_FROM(page_id_t, buf);
    buf += 4;
    meta->index_meta_pages_.emplace(index_id, index_page_id);
  }
  return meta;
}

/**
 * TODO: Student Implement
 */
uint32_t CatalogMeta::GetSerializedSize() const {
  uint32_t buf=0;
  buf += 4;
  buf += 4;
  buf += 4;
  for (auto iter : table_meta_pages_) {
    buf += 4;
    buf += 4;
  }
  for (auto iter : index_meta_pages_) {
    buf += 4;
    buf += 4;
  }
  return buf;
}

CatalogMeta::CatalogMeta() {}

/**
 * TODO: Student Implement
 */
CatalogManager::CatalogManager(BufferPoolManager *buffer_pool_manager, LockManager *lock_manager,
                               LogManager *log_manager, bool init)
    : buffer_pool_manager_(buffer_pool_manager), lock_manager_(lock_manager), log_manager_(log_manager) {
      next_index_id_.store(0);
      next_table_id_.store(0);
      if(init)
      {
        catalog_meta_=new CatalogMeta();
      }
      else
      {
        // std::cout<<111<<std::endl;
        Page *metapage=buffer_pool_manager_->FetchPage(CATALOG_META_PAGE_ID);
        // printf("FetchPage for CATALOG_META_PAGE_ID is %p\n",metapage);
        char* buf=metapage->GetData();
        // printf("Deserialize from %p\n",buf);
        catalog_meta_=CatalogMeta::DeserializeFrom(buf);
        buffer_pool_manager_->UnpinPage(CATALOG_META_PAGE_ID,false);
        for(auto iter : catalog_meta_->table_meta_pages_)
        {
          page_id_t table_page_id=iter.second;
          Page* table_page=buffer_pool_manager_->FetchPage(table_page_id);
          char *buf2=table_page->GetData();
          TableMetadata *table_metadata;
          TableMetadata::DeserializeFrom(buf2, table_metadata);
          TableHeap *table_heap=TableHeap::Create(buffer_pool_manager_,table_metadata->GetFirstPageId(),table_metadata->GetSchema(),log_manager_,lock_manager_);
          table_names_[table_metadata->GetTableName()]=table_metadata->GetTableId();
          TableInfo *table_info=TableInfo::Create();
          table_info->Init(table_metadata,table_heap);
          tables_[table_metadata->GetTableId()]=table_info;
          buffer_pool_manager->UnpinPage(table_page_id, false);
        }
        for(auto iter : catalog_meta_->index_meta_pages_)
        {
          page_id_t index_page_id=iter.second;
          Page* index_page=buffer_pool_manager_->FetchPage(index_page_id);
          char *buf2=index_page->GetData();
          IndexMetadata *index_metadata;
          IndexMetadata::DeserializeFrom(buf2, index_metadata);
          std::string table_name=tables_[index_metadata->GetTableId()]->GetTableName();
          index_names_[table_name][index_metadata->GetIndexName()]=index_metadata->GetIndexId();
          IndexInfo *index_info=IndexInfo::Create();
          index_info->Init(index_metadata,tables_[index_metadata->GetTableId()],buffer_pool_manager_);
          indexes_[index_metadata->GetIndexId()]=index_info;
          buffer_pool_manager_->UnpinPage(index_page_id, false);
        }
      }
}

CatalogManager::~CatalogManager() {
  FlushCatalogMetaPage();
  delete catalog_meta_;
  for (auto iter : tables_) {
    delete iter.second;
  }
  for (auto iter : indexes_) {
    delete iter.second;
  }
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::CreateTable(const string &table_name, TableSchema *schema, Txn *txn, TableInfo *&table_info) {
  if(table_names_.find(table_name)!=table_names_.end())
  {
    // std::cout<<1<<std::endl;
    return DB_TABLE_ALREADY_EXIST;
  }
  table_id_t new_table_id=catalog_meta_->GetNextTableId();
  table_names_[table_name]=new_table_id;

  Schema* now_schema=Schema::DeepCopySchema(schema);
  page_id_t new_page_id;
  Page *new_table_page=buffer_pool_manager_->NewPage(new_page_id);
  char *buf=new_table_page->GetData();
  catalog_meta_->table_meta_pages_[new_table_id]=new_page_id;
  TableMetadata *new_table_metadata=TableMetadata::Create(new_table_id, table_name,new_page_id,now_schema);
  new_table_metadata->SerializeTo(buf);
  TableHeap *new_table_heap=TableHeap::Create(buffer_pool_manager_, now_schema, txn, log_manager_, lock_manager_);
  table_info=TableInfo::Create();
  table_info->Init(new_table_metadata, new_table_heap);

  tables_[new_table_id]=table_info;
  buffer_pool_manager_->UnpinPage(new_page_id,true);
  return DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::GetTable(const string &table_name, TableInfo *&table_info) {
  auto it=table_names_.find(table_name);
  if(it==table_names_.end())
  {
    return DB_TABLE_NOT_EXIST;
  }
  table_info=tables_[it->second];
  return DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::GetTables(vector<TableInfo *> &tables) const {
  tables.reserve(tables_.size());
  for(auto iter : tables_)
  {
   tables.push_back(iter.second); 
  }
  return DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::CreateIndex(const std::string &table_name, const string &index_name,
                                    const std::vector<std::string> &index_keys, Txn *txn, IndexInfo *&index_info,
                                    const string &index_type) {//建立索引时，如果表中有数据，把数据插入索引
  if(table_names_.find(table_name)==table_names_.end())
  {
    return DB_TABLE_NOT_EXIST;
  }
  if(index_names_[table_name].find(index_name)!=index_names_[table_name].end())
  {
    return DB_INDEX_ALREADY_EXIST;
  }

  TableInfo *table_info=tables_[table_names_[table_name]];
  Schema *now_schema=table_info->GetSchema();
  std::vector<uint32_t>key_map;
  for(auto iter : index_keys)
  {
    uint32_t col_index;
    if(now_schema->GetColumnIndex(iter, col_index)==DB_COLUMN_NAME_NOT_EXIST)return DB_COLUMN_NAME_NOT_EXIST;
    else key_map.push_back(col_index);
  }

  index_id_t new_index_id=catalog_meta_->GetNextIndexId();
  index_names_[table_name][index_name]=new_index_id;

  page_id_t new_page_id;
  Page *new_index_page=buffer_pool_manager_->NewPage(new_page_id);
  // std::cout<<new_page_id<<std::endl;
  char *buf=new_index_page->GetData();
  catalog_meta_->index_meta_pages_[new_index_id]=new_page_id;

  IndexMetadata *new_index_metadata=IndexMetadata::Create(new_index_id,index_name, table_names_[table_name],key_map);
  new_index_metadata->SerializeTo(buf);
  index_info=IndexInfo::Create();
  index_info->Init(new_index_metadata,table_info, buffer_pool_manager_);
  TableIterator new_table_iterator=table_info->GetTableHeap()->Begin(txn);
  while(new_table_iterator!=table_info->GetTableHeap()->End())
  {
    Row *now_row=new_table_iterator.get_Row();
    Row key_row;
    now_row->GetKeyFromRow(table_info->GetSchema(),index_info->GetIndexKeySchema(),key_row);
    dberr_t Ins_message=index_info->GetIndex()->InsertEntry(key_row,new_table_iterator.get_Rid(),txn);
    if(Ins_message==DB_FAILED)return DB_FAILED;
    new_table_iterator++;
  }//向索引中插入数据
  indexes_[new_index_id]=index_info;

  buffer_pool_manager_->UnpinPage(new_page_id,true);

  return DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::GetIndex(const std::string &table_name, const std::string &index_name,
                                 IndexInfo *&index_info) const {
  if(table_names_.find(table_name)==table_names_.end())return DB_TABLE_NOT_EXIST;
  auto it=index_names_.at(table_name).find(index_name);
  if(it==index_names_.at(table_name).end())return DB_INDEX_NOT_FOUND;
  index_info=indexes_.at(it->second);
  return DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::GetTableIndexes(const std::string &table_name, std::vector<IndexInfo *> &indexes) const {
  if(table_names_.find(table_name)==table_names_.end())return DB_TABLE_NOT_EXIST;
  for(auto iter : index_names_.at(table_name))
  {
    indexes.push_back(indexes_.at(iter.second));
  }
  return DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::DropTable(const string &table_name) {
  if(table_names_.find(table_name)==table_names_.end())return DB_TABLE_NOT_EXIST;
  std::vector<IndexInfo *>Indexes;
  if(GetTableIndexes(table_name, Indexes)==DB_SUCCESS)
  {
    for(auto iter : Indexes)
    {
      DropIndex(table_name, iter->GetIndexName());
    }
  }
  table_id_t now_table_id=table_names_[table_name];
  page_id_t now_page_id=catalog_meta_->table_meta_pages_[now_table_id];
  buffer_pool_manager_->DeletePage(now_page_id);
  catalog_meta_->table_meta_pages_.erase(now_table_id);
  tables_.erase(now_table_id);
  table_names_.erase(table_name);
  return DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::DropIndex(const string &table_name, const string &index_name) {
  if(table_names_.find(table_name)==table_names_.end())return DB_TABLE_NOT_EXIST;
  auto it=index_names_.at(table_name).find(index_name);
  if(it==index_names_.at(table_name).end())return DB_INDEX_NOT_FOUND;
  index_id_t now_index_id=it->second;
  page_id_t now_page_id=catalog_meta_->index_meta_pages_[now_index_id];
  buffer_pool_manager_->DeletePage(now_page_id);
  catalog_meta_->index_meta_pages_.erase(now_index_id);
  indexes_.erase(now_index_id);
  index_names_.at(table_name).erase(index_name);
  return DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::FlushCatalogMetaPage() const {
  Page* meta_page=buffer_pool_manager_->FetchPage(CATALOG_META_PAGE_ID);
  if(meta_page==nullptr)return DB_FAILED;
  char *buf=meta_page->GetData();
  // printf("FetchPage for CATALOG_META_PAGE_ID is%p\n",meta_page);
  // printf("Serialize to %p\n",buf);
  catalog_meta_->SerializeTo(buf);
  buffer_pool_manager_->FlushPage(CATALOG_META_PAGE_ID);
  buffer_pool_manager_->UnpinPage(CATALOG_META_PAGE_ID,false);
  return DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::LoadTable(const table_id_t table_id, const page_id_t page_id) {
  if(tables_.find(table_id)!=tables_.end())return DB_TABLE_ALREADY_EXIST;
  catalog_meta_->table_meta_pages_[table_id]=page_id;
  Page *now_table_page=buffer_pool_manager_->FetchPage(page_id);
  char *buf=now_table_page->GetData();
  TableMetadata *now_table_metadata;
  TableMetadata::DeserializeFrom(buf, now_table_metadata);
  TableHeap *now_table_heap=TableHeap::Create(buffer_pool_manager_,page_id,now_table_metadata->GetSchema(),log_manager_, lock_manager_);
  table_names_[now_table_metadata->GetTableName()]=now_table_metadata->GetTableId();
  TableInfo *now_table_info=TableInfo::Create();
  now_table_info->Init(now_table_metadata,now_table_heap);
  tables_[table_id]=now_table_info;
  buffer_pool_manager_->UnpinPage(page_id, false);
  return DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::LoadIndex(const index_id_t index_id, const page_id_t page_id) {
  if(indexes_.find(index_id)!=indexes_.end())return DB_INDEX_ALREADY_EXIST;
  catalog_meta_->index_meta_pages_[index_id]=page_id;
  Page *now_index_page=buffer_pool_manager_->FetchPage(page_id);
  char *buf=now_index_page->GetData();
  IndexMetadata *now_index_metadata;
  IndexMetadata::DeserializeFrom(buf, now_index_metadata);
  std::string table_name=tables_[now_index_metadata->GetTableId()]->GetTableName();
  index_names_[table_name][now_index_metadata->GetIndexName()]=now_index_metadata->GetIndexId();
  IndexInfo *now_index_info=IndexInfo::Create();
  now_index_info->Init(now_index_metadata, tables_[now_index_metadata->GetTableId()],buffer_pool_manager_);
  indexes_[index_id]=now_index_info;
  buffer_pool_manager_->UnpinPage(page_id, false);
  return DB_SUCCESS;
}

/**
 * TODO: Student Implement
 */
dberr_t CatalogManager::GetTable(const table_id_t table_id, TableInfo *&table_info) {
  if(tables_.find(table_id)==tables_.end())return DB_TABLE_NOT_EXIST;
  table_info=tables_[table_id];
  return DB_SUCCESS;
}
