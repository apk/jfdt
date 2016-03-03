just file descriptors and timers
================================

A library for modularizing dealing with `select(3)` and
similar interfaces. Originally conceived in '94, and
implemented in a proprietary library, now reimplemented
to get some basic stuff running (again).

Other people call this an event library (`libevent`,
`libev`, `libuv`); this is a trivial remake with
the original API strategy, and without much regard
for performance.
