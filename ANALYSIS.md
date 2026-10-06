# Joint Analysis

1. Pick 3 of the 10 categories. For each, pick a language that gives it to you for free and say what that language pays for it. "Python has dictionaries" isn't an answer. What does Python's dictionary cost in memory or in speed compared to what you built, and where would you notice?

* **Arrays in Rust**
  * Rust provides built-in arrays with automatic bounds checking.
  * Rust arrays are limited to zero-based indexing. Our C array supports custom bounds by storing an offset, so our implementation uses slightly more metadata.
  * Rust's built-in array is probably slightly better for memory usage, while speed should be similar. Rust's bounds checks can add a small runtime cost, although the compiler can often remove them when it can prove an index is valid.
  * The difference is mainly noticeable when custom bounds are needed or in extremely large numbers of indexed accesses.
* **Dict in Python**
  * Python provides dictionaries with hashing, collision handling, resizing, and key-value storage built in.
  * Python's dictionary stores general Python objects and has significant per-entry and table overhead. Our C map stores `dt_value` directly and uses a simpler representation.
  * Our C map is probably better for both memory usage and lookup speed, especially for simple values. Python trades this performance for flexibility and convenience.
  * The difference becomes noticeable when a program has many entries or performs a large number of insertions and lookups.
* **List in Java**
  * Java provides LinkedLIst, including node management and automatic memory reclamation through garbage collection.
  * Java's linked-list nodes are heap-allocated objects with object and reference overhead, and the garbage collector must eventually reclaim them. Our C list uses manually allocated nodes and frees them explicitly.
  * Our C list is probably more memory-efficient and has more predictable performance. Java avoids the cost of manual memory management but adds object and garbage-collection overhead.
  * The difference is most noticeable with large lists or programs that frequently create and discard many list nodes.

3. Your `dt_map` keeps insertion order separately from the hash buckets, which is memory spent on something no lookup uses. Argue the other side: describe a design that drops it, say what breaks, and say whether you'd ship it.

* **Alternative design:** Drop the separate insertion-order structure and store entries only in the hash buckets. Iteration would traverse the buckets and their chains directly.
* **What breaks:** Iteration would no longer follow insertion order. The output order would depend on the hash function, bucket layout, and resizing, so adding entries or resizing the map could change the order in which they are observed.
* **Trade-off:** This saves memory and simplifies the implementation, but makes iteration order unpredictable.
* **Would I ship it?** No. Although insertion order is not needed for lookup, it is part of the map's observable behavior in printing. The extra memory is a reasonable cost for predictable iteration while retaining average constant-time lookup.
