# asyncpp

C++ async operations

### Requirements

* C++11.
* Boost, for examples.
* SCons, to build examples and tests.

### Control flow

#### series

Invokes a series of tasks, and collects result values into a vector.  As each task completes, it invokes a callback with an error code and a result value.  If the error code is not `async::OK`, iteration stops.  Once all tasks complete or there is an error, `final_callback` is called with the last error and the results vector.

Each task may invoke its callback immediately, or at some point in the future.

### Examples

Build using `scons`.

```
test/series.cpp
test/series-boost-asio.cpp
```

