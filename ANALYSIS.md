# Joint Analysis

1. Pick 3 of the 10 categories. For each, pick a language that gives it to you for free and say what that language pays for it. "Python has dictionaries" isn't an answer. What does Python's dictionary cost in memory or in speed compared to what you built, and where would you notice?

* **Arrays in Rust**
  * Rust provides built-in arrays with automatic bounds checking.
  * Rust arrays are limited to zero-based indexing. Our C array supports custom bounds by storing its lower bound, so our implementation uses slightly more metadata.
  * Rust's fixed-size array uses less memory, since its length is part of the type and it needs no lower bound field, while speed is similar. Rust's bounds checks can add a small runtime cost, although the compiler can often remove them when it can prove an index is valid.
  * The difference is mainly noticeable when custom bounds are needed or in extremely large numbers of indexed accesses.
* **Dict in Python**
  * Python provides dictionaries with hashing, collision handling, resizing, and key-value storage built in.
  * Python's dictionary stores general Python objects and has significant per-entry and table overhead. Our C map stores `dt_value` directly and uses a simpler representation.
  * Our C map uses less memory per entry, since values are stored unboxed. But it has a fixed 16 buckets and never resizes, so lookups slow down as the map grows, while Python resizes to keep lookups constant-time. Python trades memory for that speed and convenience.
  * The difference becomes noticeable when a program has many entries or performs a large number of insertions and lookups.
* **Str in Python**
  * Python provides strings with stored length, Unicode handling, and memory management built in. Like our `dt_str`, `len()` is a field read and a string can hold `\0`.
  * Every Python string is an object with tens of bytes of header, and it stores characters at 1, 2, or 4 bytes each depending on the widest character. Our `dt_str` stores raw bytes with only a length and capacity beside them.
  * Python strings are immutable, so `s += t` generally builds a new string and copies both parts. A loop of appends can become quadratic, which is why Python code uses `"".join()`. Our `dt_str_append` grows the buffer geometrically and appends in place, at the cost of spare capacity that can be up to half the buffer.
  * The difference is most noticeable with many small strings, or when a string is built up piece by piece in a loop.

2. You wrote the tag check in dt_value_as_int by hand. Some languages don't let you. They make the tagged union a language construct, so the compiler writes the check for you, refuses to compile a read that skips it, and refuses to compile a set of cases that misses one. Rust's enum and match work this way, and so do ML's datatypes and Swift's enumerations with associated values. What does the C version let you do that a compiler enforcing the check wouldn't, and is any of it worth wanting?

* **Read a member without checking the tag.** `v.as.string` compiles on a value tagged `DT_INT`. The union is public in `dt.h`, so the tag check in `dt_value_as_int` is only a convention for callers who use the readers. Nothing stops code from skipping them.
* **Build an inconsistent value.** We could set `tag = DT_STR` while the payload holds an integer. Rust, ML, and Swift make that state unrepresentable.
* **Add or ignore a case silently.** If an eleventh tag were added to `dt_tag`, a `switch` in the printer would still compile, and the new case would fall through unhandled unless a warning flag caught it. A compiler-enforced `match` refuses to compile.
* **Reinterpret the bytes.** Reading one union member as another is how low-level code does serialization, NaN-boxing, tagged pointers, or inspecting a float's bits.
* **Skip the check when it's redundant.** After the tag has been checked once, further reads of the payload need no second branch. C lets the programmer decide.

  **Is it worth wanting?** Mostly no. For example, `as str 42` would hand the printer an integer where an address belongs, and that is undefined behavior, not an error message. The useful part, direct control over layout and over when checks run, matters in allocators, kernels, VMs, embedded code, and binary formats. Even there, Rust keeps the same power behind unsafe, so the real difference is that the dangerous operation is opt-in and visible rather than the default. We'd want the default to be checked, and the unchecked escape hatch to be explicit.

3. Your `dt_map` keeps insertion order separately from the hash buckets, which is memory spent on something no lookup uses. Argue the other side: describe a design that drops it, say what breaks, and say whether you'd ship it.

* **Alternative design:** Drop the separate insertion-order structure and store entries only in the hash buckets. Iteration would traverse the buckets and their chains directly.
* **What breaks:** Iteration would no longer follow insertion order. The output order would depend on the hash function, bucket layout, and resizing, so adding entries or resizing the map could change the order in which they are observed. `dt_map_key_at` would also have to walk the buckets on every call, making printing quadratic.
* **Trade-off:** This saves memory and simplifies the implementation, but makes iteration order unpredictable.
* **Would we ship it?** No. Although insertion order is not needed for lookup, it is part of the map's observable behavior in printing. The extra memory, one pointer per key, is a reasonable cost for predictable iteration.

4. Compare access after release with an allocation that remains unreleased at the driver's final check. What damage can each cause in a long-running server? How does that answer change for a command-line tool that exits in a second?

* **Access after release.** Reading or releasing memory that has already been freed is undefined behavior. In a long-running server, the typical symptoms are wrong data, a crash far from the actual cause, or corrupted allocator records. It shows up unpredictably, sometimes much later than the mistake itself, which makes it hard to reproduce, since the program may work fine in testing. The worst case is silent corruption or a security hole, because the freed memory may now hold another request's data, and an attacker can exploit that to control what lands in the freed block. This is damage to correctness and security, not just availability.
* **Unreleased allocation (leak).** A leak means memory is never returned. In a long-running server, the typical symptom is memory use that grows steadily, followed by slowdown, swapping, or the OOM killer stepping in. Unlike the first failure, it shows up gradually and monotonically, so the hardest part is noticing it before it takes the process down. The worst case is a crash after hours or days of running. Even a small leak adds up. For example, tens of bytes lost per request at 1,000 requests per second comes to over a gigabyte per day. The damage is to availability, and it can be detected by watching memory over time.
* **For a command-line tool that exits in a second,** the answer changes for the leak but not for the access-after-release bug. A leak mostly stops mattering, because the operating system reclaims all of the process's memory at exit, and the damage is limited to the short lifetime of the run. Access after release is still just as serious. A short lifetime doesn't make wrong output, a corrupted file, or a crash acceptable, and a tool that handles untrusted input can be exploited in one second as easily as in one year. So a short run shrinks the cost of a leak to almost nothing, but it does nothing to reduce the danger of use after release.
* **In our implementation,** `dt_ref` turns the first failure into `DT_ERR_RELEASED` by checking the released flag before touching the cell, and the driver's final sweep reports the second as `DT_ERR_LEAK`. A real server gets neither check for free, so it has to rely on tools like AddressSanitizer in testing.