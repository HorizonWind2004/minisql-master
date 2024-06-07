#include "recovery/recovery_manager.h"

#include "gtest/gtest.h"
#include "recovery/log_rec.h"

class RecoveryManagerTest : public testing::Test {
 protected:
  void SetUp() override {
    LogRec::prev_lsn_map_.clear();
    LogRec::next_lsn_ = 0;
  }

  void TearDown() override {}

 protected:
};

TEST_F(RecoveryManagerTest, RecoveryTest) {
  auto d0 = CreateBeginLog(0);                         // <T0 Start>;
  auto d1 = CreateUpdateLog(0, "A", 2000, "A", 2050);  // <T0, A, 2000, 2050>
  auto d2 = CreateDeleteLog(0, "B", 1000);             // <T0, B, 1000, ->
  auto d3 = CreateBeginLog(1);                         // <T1 Start>
  ASSERT_EQ(INVALID_LSN, d0->prev_lsn_);
  ASSERT_EQ(d0->lsn_, d1->prev_lsn_);
  ASSERT_EQ(d1->lsn_, d2->prev_lsn_);
  ASSERT_EQ(INVALID_LSN, d3->prev_lsn_);

  /*--------- CheckPoint ---------*/
  CheckPoint checkpoint;
  checkpoint.checkpoint_lsn_ = d3->lsn_;
  checkpoint.AddActiveTxn(0, d2->lsn_);
  checkpoint.AddActiveTxn(1, d3->lsn_);
  checkpoint.AddData("A", 2050);
  /*--------- CheckPoint ---------*/

  auto d4 = CreateInsertLog(1, "C", 600);  // <T1, C, -, 600>
  auto d5 = CreateCommitLog(1);            // <T1 Commit>
  ASSERT_EQ(d3->lsn_, d4->prev_lsn_);
  ASSERT_EQ(d4->lsn_, d5->prev_lsn_);

  auto d6 = CreateUpdateLog(0, "C", 600, "C", 700);  // <T0, C, 600, 700>
  auto d7 = CreateAbortLog(0);                       // <T0, Abort>
  ASSERT_EQ(d2->lsn_, d6->prev_lsn_);
  ASSERT_EQ(d6->lsn_, d7->prev_lsn_);

  auto d8 = CreateBeginLog(2);                        // <T2 Start>
  auto d9 = CreateInsertLog(2, "D", 30000);           // <T2, D, -, 30000>
  auto d10 = CreateUpdateLog(2, "C", 600, "C", 800);  // <T2, C, 600, 800>
  ASSERT_EQ(INVALID_LSN, d8->prev_lsn_);
  ASSERT_EQ(d8->lsn_, d9->prev_lsn_);
  ASSERT_EQ(d9->lsn_, d10->prev_lsn_);

  std::vector<LogRecPtr> logs = {d0, d1, d2, d3, d4, d5, d6, d7, d8, d9, d10};

  RecoveryManager recovery_mgr;
  recovery_mgr.Init(checkpoint);
  for (const auto &log : logs) {
    recovery_mgr.AppendLogRec(log);
  }
  auto &db = recovery_mgr.GetDatabase();

  recovery_mgr.RedoPhase();
  ASSERT_EQ(db["A"], 2000);
  ASSERT_EQ(db["B"], 1000);
  ASSERT_EQ(db["C"], 800);
  ASSERT_EQ(db["D"], 30000);

  recovery_mgr.UndoPhase();
  ASSERT_EQ(db["A"], 2000);
  ASSERT_EQ(db["B"], 1000);
  ASSERT_EQ(db["C"], 600);
  ASSERT_EQ(db.count("D"), 0);
}

TEST_F(RecoveryManagerTest, MyRecoveryTest) {
  auto d0 = CreateBeginLog(0);                                // <T0 Start>
  auto d1 = CreateBeginLog(1);                                // <T1 Start>
  auto d2 = CreateUpdateLog(0, "X", 500, "X", 550);           // <T0, X, 500, 550>
  auto d3 = CreateUpdateLog(1, "Y", 300, "Y", 350);           // <T1, Y, 300, 350>
  auto d4 = CreateCommitLog(0);                               // <T0 Commit>
  auto d5 = CreateUpdateLog(1, "X", 550, "X", 600);           // <T1, X, 550, 600>
  auto d6 = CreateUpdateLog(1, "Z", 200, "Z", 250);           // <T1, Z, 200, 250>
  auto d7 = CreateBeginLog(3);                                // <T3, Start>
  ASSERT_EQ(INVALID_LSN, d0->prev_lsn_);
  ASSERT_EQ(d0->lsn_, d2->prev_lsn_);
  ASSERT_EQ(d2->lsn_, d4->prev_lsn_);
  ASSERT_EQ(d3->lsn_, d5->prev_lsn_);
  ASSERT_EQ(INVALID_LSN, d7->prev_lsn_);
  
  /*--------- CheckPoint ---------*/
  CheckPoint checkpoint;
  checkpoint.checkpoint_lsn_ = d5->lsn_;
  checkpoint.AddActiveTxn(1, d5->lsn_);
  checkpoint.AddActiveTxn(3, d7->lsn_);
  checkpoint.AddData("X", 600);
  checkpoint.AddData("Y", 350);
  checkpoint.AddData("Z", 200);
  /*--------- CheckPoint ---------*/

  auto d8 = CreateBeginLog(2);                                // <T2 Start>
  auto d9 = CreateUpdateLog(2, "W", 300, "W", 450);           // <T2, W, 300, 450>
  auto d10 = CreateCommitLog(2);                              // <T2 Commit>
  ASSERT_EQ(INVALID_LSN, d8->prev_lsn_);
  ASSERT_EQ(d8->lsn_, d9->prev_lsn_);
  ASSERT_EQ(d9->lsn_, d10->prev_lsn_);

  std::vector<LogRecPtr> logs = {d0, d1, d2, d3, d4, d5, d6, d7, d8, d9, d10};

  RecoveryManager recovery_mgr;
  recovery_mgr.Init(checkpoint);
  for (const auto &log : logs) {
    recovery_mgr.AppendLogRec(log);
  }
  auto &db = recovery_mgr.GetDatabase();

  recovery_mgr.RedoPhase();
  ASSERT_EQ(db["X"], 600); // Expected to remain unchanged as T1 is aborted.
  ASSERT_EQ(db["Y"], 350); 
  ASSERT_EQ(db["Z"], 250);
  ASSERT_EQ(db["W"], 450); 

  recovery_mgr.UndoPhase();
  ASSERT_EQ(db["X"], 550); 
  ASSERT_EQ(db["Y"], 300); 
  ASSERT_EQ(db["Z"], 200);
  ASSERT_EQ(db["W"], 450); 
}