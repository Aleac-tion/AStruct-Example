# The English content is below the text, please scroll down.
# 测试API性能
## 系统压力环境
# **进程数**: 362
# **线程数**: 7890
# **句柄数**: 199,431
# **并发应用**: 赛博朋克2077+ 原神+ 12个Edge浏览器 + 不等
# **SSD状态**: 严重争用
# 测试硬件 I9-13900KF 7100MB/S-M.2


*CPP:*
```AStruct
// 测试1: 基础读取性能（首次+缓存）
for(int i=0; i<100; i++) {
    as.getvalue("T","A",std::to_string(i));  // 首次读取: 525μs/100次
}
for(int i=0; i<100; i++) {                   // 缓存命中: 8μs/100次
    as.getvalue("T","A",std::to_string(i));
}

// 测试2: 长key性能验证
for(int i=0; i<100; i++) {                   // 长key首次: 952μs/100次
    as.getvalue("T","A",std::to_string(i)+"long_txt_key_test");
}
for(int i=0; i<100; i++) {                   // 长key缓存: 11μs/100次
    as.getvalue("T","A",std::to_string(i)+"long_txt_key_test");
}

// 测试3: 纯内存修改性能
as.addkey("T","A","key","value");           // addkey: 10μs/次
for(int i=0; i<100; i++) {                   // 100次修改: 753μs总耗时
    as.changeValue("T","A","key",std::to_string(i));
}

// 测试4: 带文件保存的极限测试
for(int i=0; i<100; i++) {                   // 最坏情况: 触发200次文件写入(100次修改存档+100次超级路径文本)
    as.changeValue("T","A","key",std::to_string(i),
                   "configs/"+std::to_string(i)+".txt",true);
}
as.getvalue("T","A","key");                  // 最终验证: 输出"99"
                                             // 1026μs总耗时

                                             // 后台以vs2026监视大概消耗了10秒
                                             由于我内存频率较低3600mhz和IO被大量阻塞，时长恐怕不准
                                             但前台仍然只有1026us的感知

// 测试5: 批量创建测试
for(int i=0; i<100; i++) {                   // 100次addkey: 1000μs
    as.addkey("T","A",std::to_string(i),std::to_string(i));
}

AStruct模型（异步零阻塞）
用户感知耗时 = Σ(内存操作时间)
实际分解:
- 内存修改: 100 × 7.4μs = 740μs
- 队列入队: 100 × 1μs = 100μs
- 异步通知: 100 × 0.1μs = 10μs
```

# API Performance Test
## System Stress Environment
*   **Processes:** 362
*   **Threads:** 7,890
*   **Handles:** 199,431
*   **Concurrent Applications:** Cyberpunk 2077 + Genshin Impact + 12+ Edge browser instances, etc.
*   **SSD State:** Severe resource contention
*   **Test Hardware:** I9-13900KF, 7100 MB/s M.2 SSD

```cpp
// Test 1: Basic Read Performance (First-time + Cache)
for(int i=0; i<100; i++) {
    as.getvalue("T","A",std::to_string(i));  // First read: 525μs total for 100 ops
}
for(int i=0; i<100; i++) {                   // Cache hit: 8μs total for 100 ops
    as.getvalue("T","A",std::to_string(i));
}

// Test 2: Long Key Performance Validation
for(int i=0; i<100; i++) {                   // Long key, first read: 952μs total for 100 ops
    as.getvalue("T","A",std::to_string(i)+"long_txt_key_test");
}
for(int i=0; i<100; i++) {                   // Long key, cache hit: 11μs total for 100 ops
    as.getvalue("T","A",std::to_string(i)+"long_txt_key_test");
}

// Test 3: Pure In-Memory Modification Performance
as.addkey("T","A","key","value");           // Single addkey: ~10μs
for(int i=0; i<100; i++) {                   // 100 modifications: 753μs total
    as.changeValue("T","A","key",std::to_string(i));
}

// Test 4: Extreme Test with File Saving (Worst-case)
for(int i=0; i<100; i++) {                   // Worst-case: triggers 200 file writes (100 archive modifications + 100 Super Path texts)
    as.changeValue("T","A","key",std::to_string(i),
                   "configs/"+std::to_string(i)+".txt", true);
}
as.getvalue("T","A","key");                  // Final verification: outputs "99"
                                             // Total foreground perceived time: 1026μs

                                             // Background monitoring in VS2026 showed ~10 seconds of actual I/O activity.
                                             // Due to lower memory frequency (3600MHz) and heavy IO contention, the background duration may be imprecise.
                                             // However, the foreground perception remained only 1026μs.

// Test 5: Batch Creation Test
for(int i=0; i<100; i++) {                   // 100 addkey operations: 1000μs total
    as.addkey("T","A",std::to_string(i),std::to_string(i));
}
AStruct Model (Asynchronous Non-Blocking)
User-perceived duration = Σ(In-memory operation time)
Actual decomposition:

In-memory modifications: 100 × 7.4μs = 740μs

Queue enqueuing: 100 × 1μs = 100μs

Asynchronous notification: 100 × 0.1μs = 10μs
