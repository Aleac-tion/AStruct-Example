# Update Log

**Performance under Extreme System Load (for a 1.5KB file):**
*   **CPU Load:** ~380 processes, ~9000 threads (i9-13900KF)
*   **Test Environment:** Not an idle lab test. Simulates real-world development pressure with 5-8 concurrent VS2022 instances, a VM on a secondary screen, and Ollama running DeepSeek-R1:32B for cross-machine parameter testing.
*   **Measured Performance:**
    *   `loaddata()`: ~60 µs
    *   First-time `getvalue()`: ~10 µs
    *   Cache-hit `getvalue()`: ~400 ns
    *   All storage-class operations: ~15 µs

**Readability Comparison:**

*AStruct Format:*
```AStruct
{
[title]:PlayerData
  {
  [header]:Weapon.Sword01.BaseStats
    {
    id=sword_01
    damage.physical=100
    damage.fire=50
    }
  }[]
  {
  [header]:Weapon.Sword01.Effect.Critical
    {
    type=critical
    value=0.2
    conditions.target.race=demon||Triggers when target race is demon
    conditions.target.health.below=0.3||Triggers when target health is below 30%
    }
  }[]
};
Equivalent JSON:

json
{
	"player": {
		"inventory": {
			"weapons": [
				{
					"id": "sword_01",
					"stats": {
						"damage": {
							"physical": 100,
							"fire": 50,
							"modifiers": [
								{
									"type": "critical",
									"value": 0.2,
									"conditions": {
										"target": {
											"race": "demon",
											"health": { "below": 0.3 }
										}
									}
								}
							]
						}
					}
				}
			]
		}
	}
}
```
Named AleacStruct (abbreviated AStruct) after the English name Aleactional.

Version Log
# v1.0 - 2025-09-30
First iteration. Implemented core functions getvalue(), loaddata(). Added path completion for :::/ or :::\, and inline || comments.

# v2.0 - 2025-10-05
Second iteration. Core algorithms optimized for peak performance with assistance from DeepSeek's code optimization and collaborators at MIT and Meike. Added indexed lookup for getvalue(). Changed file reading to faster binary reads. Optimized singlesaved() for one-shot save operations.
DOM-less Design: Data is handed off to member variables without copying and immediately released. All subsequent operations work on local variables, achieving semi-decoupling from storage.
Added std::map cache with a hash algorithm, enabling reads as fast as 400 nanoseconds.
Added preliminary exception handling.

# v3.0 - 2025-10-09
Third iteration. Introduced a special storage scheme using queue, future, condition_variable, std::pair, atomic, and thread to implement atomic, queued writes.
Frontend/Backend Decoupling: The main thread operates on memory while a background thread handles lock-free, queued I/O, preventing data corruption. The destructor uses future to ensure pending write tasks in the separate process are not lost if the main process closes, achieving complete decoupling of storage from the main process. This is an evolved/adapted Producer-Consumer model.
Added LRU cache policy to prevent unbounded growth; automatically erases cache.begin() when length exceeds the set limit.
Added Super Array parsing (@array@), introducing a DSL-level meta-storage format capable of storing complex constructs like Unreal Engine 5 Blueprints through strong abstraction.
Example Blueprint Key:
@array@[@array@[ParentClass, @array@[Camera, x=0, y=120, z=0, r=120, Distance=102], @array@[Capsule, mesh=:::/player.one]], atk=100, Health=100/MaxHealth=150, @array@[hp.type=int, name.type=Fstring/string]]

# v3.3 - 2025-10-11
Enhanced the third iteration. Added a complete set of APIs: DelKey, DelHeader, DelTitle, AppendHeader, AppendTitle, AddKey, Fixformat, CreateAStruct, AStruct::parseArray, AStruct::parseKey. Implemented comprehensive exception handling.

# v4.0 - 2025-10-13
Introduced the extension library AleaCook (requires #include <AleaCook>). Based on inheritance, it references OpenSSL but remains fully decoupled. The core AStruct library has zero dependencies; encryption is only added when manually included.
Usage:
cpp
AStruct as;
AleaCook cook(&as); // Transfers ownership of 'as' to 'cook'
cook.Cook(); // Encrypts plaintext .Astruct to binary .alcst
cook.UnCook(); // Decrypts .alcst into memory for operations
cook.Purity(); // Restores .alcst back to plaintext .Astruct
The Cook() process uses modern OpenSSL-AES256 encryption.

# v4.2 - 2025-10-13
Added a save-lock mechanism. In the encrypted binary state (Cooked), saving is disabled. Triggering Cook() forcibly replaces the source text, making tampered saves无效.
Only the destructor or an explicit Cook() call modifies the final archive. Between these events, operations use only memory, completely decoupled from disk.
UnCook() uses std::move() to store decrypted plaintext in this->structdata. With the save-lock active, all disk-write capabilities are disabled, confining memory modification performance to L1/L2/L3 cache levels.
Added close(); Calling as.close(); completely destroys in-memory data, including cache and settings.

# v5.0 - 2025-10-16
Introduced the Super Array variable AList. Use the << operator to build arrays, then convert to @array@ format with .toAstruct() for storage.
AList seamlessly connects to AStruct::getArray() for direct use (e.g., AList list = AStruct::getArray(...);).
Added list[0].Go() to parse nested arrays (e.g., @array@[@array@[a,b,c]]). Go() re-parses the content for easier access.

```
AList list = AStruct::getArray(as.getvalue(...));
// list[0].Go()[1] accesses the parsed inner array.
Use AList to construct Super Arrays: list << "a" << "b" << "c";.
Supports nesting: lists << "d" << list << "e"; yields @array@[d, @array@[a,b,c], e].
Added comprehensive error codes (numeric) for detailed lookup.
```

# v5.5 - 2025-10-20
AList now supports recursive Go() nesting (e.g., list[0].Go()[0].Go()[1].Go()[1]).
Added list[0].Parse(); to directly parse key=value strings.

# v6.0 - 2025-10-21
Enhanced error system. Error codes now return numeric values corresponding to a resident errorlist.Astruct file.
Usage:
cpp
AStruct why;
why.loaddata("your/lib/path/errorlist.Astruct");
std::string message = why.getvalue("error", "01", "01001"); // Returns error description
The errorlist.Astruct is updatable via GitHub (Aleactional account). Currently supports Chinese and English.
Failed error lookups generate a log file with timestamp, function name, file path, line number, and error code.

# v6.2 - 2025-10-22
Packaged features from v4.0 to v6.0 into a library (lib) with enhanced capabilities.
Strengthened AList's << operator for more abstract/powerful syntax (aligns with C++ stream conventions).
Dynamic AES256 key/IV generation, performance improvements. Writes now use AStruct's own evolved Producer-Consumer model.
32 APIs opened, covering full CRUD operations: Appendtitle(), Appendheader(), addkey(), Getvalue, autoparse, changevalue, delvalue, etc.

# v7.0 beta - 2025-11-08 to 09
Decoupled the AleaCook library, integrating its syntax into AStruct core. Encryption dependencies moved to a .dll plugin (AES256), making reverse engineering nearly impossible. Falls back gracefully if the DLL is not found.
AList templatized: AList << CustomType{...} is now possible if operator<< is overloaded for the type, enabling serialization of complex objects like Unreal UOBJECTs.

# v7.2 - 2025-11-10
Released utility functions:
std::vector<std::string> AStruct::static_splittext(text, delimiter);
bool AStruct::static_searchtext(source, target); (case-sensitive strict search)
char*/string AStruct::static_dir(); (returns current library directory)

# v8.0 beta - 2025-11-28 (30-min implementation)
Encryption finalized with AES-NI support. Enhanced resistance to channel attacks, power analysis, and timing attacks. Performance significantly improved.
Added robust defense mechanisms. Fixed re-encryption/decryption bugs.
Recommendation: Manage AleaCook instance lifecycle manually via pointers for complex use cases.
Core workflow remains: loaddata() -> AleaCook cooks(&as); -> cooks.Cook()/UnCook()/Purity().

# v9.0 alpha - 2025-12-19 (~1 hour implementation)
Added index-based accessors: getvalue("title", "header", int index) and beta_getvalue(...).
Introduced [Super Path] concept, extending :::/ to ::::/.
Example: ::::/config/conf.ini automatically memory-maps the target file into cache/memory.
changevalue("title", "header", "key", "data", "path") automatically prepends ::::/ to the path. Use false as the last argument to disable this.

# v9.0 - 2025-12-21
Architecture stable, no major bugs. Minor logic fixes.
Key Insight: Due to CPU/Windows 10+ optimizations, memory footprint expansion is likely far below 1.0% because the in-memory representation mirrors the AStruct format.
Core Operational Model:
as.loaddata("xxx.Astruct"); (One-time memory mapping -> pure memory)
as.changevalue("title", "header", "key", "value"); (In-memory modification, completes immediately)
Trigger: Asynchronous, atomic write task is enqueued to a background thread (zero perception, no management needed).
Total: 0 copies, 1 atomic modification, 1 triggered async background write.

 一次内存映射->纯内存->内存内容被修改(已完成) <-结束-> 触发异步写入将需要存入的内容任务加入后台队列(无感知/无需管理) 
		 
		 总共0次拷贝，1次原子修改，1次触发异步原子后台写入
