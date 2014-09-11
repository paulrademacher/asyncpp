# asyncpp

Asyncpp is a C++ utility library for asynchronous or functional programming using modern C++ lambdas, without getting into callback hell.  It is ideally suited for use with <a href="http://www.boost.org/doc/libs/1_56_0/doc/html/boost_asio/overview/core/basics.html">Boost ASIO</a>.

This was inspired by the popular [async](https://github.com/caolan/async) Node.js library.

### What problem does this solve?

In asynchronous programming (e.g. network programming using Boost ASIO, where the thread
doesn't block on network calls) you lose the ability to pass data back to a calling
function via return values.  And if there are many chained asynchronous operations, or
some combination of serial and parallel asynchronous operations, then you quickly wind up
with a mess of callbacks.

This library helps by packaging several common patterns of asynchronous operations, to keep your code
clean and reasonable.

Here's a contrived example.  Imagine we have to call a blocking function three times in a row.  If
any of the invocations returns `false`, we want to return `false` to the caller.

```c++
bool func();

bool call_func_three_times() {
    for (int i = 0; i < 3; i++) {
        if (!func()) {
            return false;
        }
    }
    return true;
}
```

Now what if the functions are non-blocking?  Note that the following naive attempt would
not work:

```c++
bool func_async();

bool call_func_three_times_async() {
    for (int i = 0; i < 3; i++) {
        if (!func_async()) {  // Non-blocking.
            return false;
        }
    }
    return true;
}
```

It can't work because the non-blocking calls can't really return the final return value,
since the operations they perform complete at some point in the future.  Furthermore, the
above code is spawning three calls in parallel, not in series.

A proper version of the asynchronous code needs callbacks, and could look like this:

```c++
using KeepGoingCallback = std::function<void(bool keep_going)>;
using FinalCallback = std::function<void(bool return_code)>;

// This function  spawns some asynchronous activity, and eventually
// invokes 'callback' with the value true to keep going, or false to stop.
void func_async(KeepGoingCallback callback);

// Start the chain of invocations.  Eventually, 'final_callback'
// will be invoked with true if all three functions succeded,
// or false if there were any errors.
void call_function_three_times_async(FinalCallback final_callback) {
    func_async([final_callback](bool keep_going) {
        // This lambda is eventually invoked by func_async() when it's
        // done.  func_async() will pass it true or false, to trigger
        // the next step in the chain, or stop altogether.
        if (keep_going) {
            func_async([final_callback](bool keep_going) {
                // This lambda is also invoked by func_async().
                if (keep_going) {
                    func_async([final_callback](bool keep_going) {
                        // This lambda too.
                        if (keep_going) {
                            // We're done and all three calls succeeded.
                            final_callback(true);
                        } else {
                            // We're done but the third call failed.
                            final_callback(false);
                        }
                    });
                } else {
                    // The second call failed.  Stop.
                    final_callback(false);
                }
            });
        } else {
            // The first call failed.  Stop.
            final_callback(false);
        }
    });
}
```

This now *works*, but we're in callback hell.

#### Cleaned up with Asyncpp

Using **Asyncpp** library, the code becomes:

```c++
using KeepGoingCallback = std::function<void(bool keep_going)>;
using FinalCallback = std::function<void(bool return_code)>;

void func_async(KeepGoingCallback callback);

void call_function_three_times_async(FinalCallback final_callback) {
    async::ntimes(3, func_async, final_callback);
}
```

We've completely generalized the pattern of multiple serial calls to an asynchronous
function.  The **Asyncpp** library also has functions for parallel calls, parallel calls
with a limit on the number of simultaneous outstanding calls, loops, filters, and more.

#### Boost example

Here's an example using the Boost ASIO network library.  Instead of writing:

```c++
resolver.async_resolve(query, [=](error_code& err, ...) {
    // Do stuff, then:
    asio::async_connect(socket, iter, [=](error_code& err, ...) {
        // Do stuff, then:
        asio::async_write(socket, request, [=](error_code& err, ...) {
            // Do stuff, then:
            asio::asio_read_until(socket, response, "\r\n", [=](error_code& err, ...) {
                // Do stuff, then:
                asio::asio_read_until(socket, response, "\r\n", [=](error_code& err, ...) {
                    // Do stuff, then:
                    asio::asio_read_until(socket, response, "\r\n", [=](error_code& err, ...) {
                        // Do stuff, then:
                        asio::async_read_until(socket, response, "\r\n\r\n", [=](error_code& err, ...) {
                            // Keep nesting and nesting until your tab key breaks :-(
                        }
                    }
                }
            }
        }
    }
}
```

with **asyncpp** we can instead write this as a flat sequence of steps:

```c++
    using Callback = async::TaskCallback<int>;
    async::TaskVector<int> tasks {
        [](Callback next) {
            resolver.async_resolve(query,
                [=](error_code& err, ...) { next(async::OK); };
        }, [](Callback next) {
            // Do stuff, then:
            asio::async_connect(socket, iter,
                [=](error_code& err, ...) { next(async::OK); };
        }, [](Callback next) {
            // Do stuff, then:
            asio::async_write(socket, request,
                [=](error_code& err, ...) { next(async::OK); };
        }, [](Callback next) {
            // Do stuff, then:
            asio::async_read_until(socket, response, "\r\n",
                [=](error_code& err, ...) { next(async::OK); };
        }, [](Callback next) {
            // Do stuff, then:
            asio::async_read_until(socket, response, "\r\n",
                [=](error_code& err, ...) { next(async::OK); };
        }, [](Callback next) {
            // Do stuff, then:
            asio::async_read_until(socket, response, "\r\n",
                [=](error_code& err, ...) { next(async::OK); };
        }, [](Callback next) {
            // Do stuff, then:
            asio::async_read_until(socket, response, "\r\n\r\n",
                [=](error_code& err, ...) { next(async::OK); };
        }
    };
    async::series<int>(tasks);
```

### Functions

<a name="each">
#### each
</a>

Takes an input vector and a function, and applies that function to each element in the vector

<a name="map">
#### map
</a>

Takes an input vector and a function, and applies that function to each element in the vector.  Returns (via the final_callback) a new vector with the transformed values.

<a name="series">
#### series
</a>

Invokes a series of tasks, and collects result values into a vector. As each task completes, it invokes a callback with an error code and a result value. If the error code is not `async::OK`, iteration stops. Once all tasks complete or there is an error, `final_callback` is called with the last error and the results vector.

Each task may invoke its callback immediately, or at some point in the future.

<a name="parallel">
#### parallel
</a>

Invokes a series of tasks, each of which produces some output value, and aggregates the results into an output vector.

A task may run to completion immediately, or it may defer calling its completion callback.  The latter would be if using with an asynchronous execution framework like Boost ASIO.  If each task runs to completion immediately, then this call becomes equivalent to [`series`](#series).

There is no limit on how many calls may be outstanding at the same time.

<a name="parallelLimit">
#### parallelLimit
</a>

Same as [`parallel`](#parallel), but allows the setting of a limit on how many tasks may be concurrently outstanding.

<a name="filter">
#### filter
</a>

Takes a vector of input data, and passes each element through a test function that returns **true** or **false**.  If **true**, that element is added to an output vector.


<a name="reject">
#### reject
</a>

Similar to [`filter`](#filter), except each element is added to the output vector if the test function returns **false**.


<a name="whilst">
#### whilst
</a>

Takes two functions, `test` and `func`.  It repeatedly calls `test`, and if that returns **true**, then calls `func`.  It `test` ever returns **false** or `func` passes a non-OK (non-zero) error code to its callback, then the `whilst` function stops.

<a name="doWhilst">
#### doWhilst
</a>

Similar to [`whilst`](#whilst), except if follows `do..while` control flow.  That is, it calls `func` first, then `test`.  Otherwise, the same rules rules.

<a name="until">
#### until
</a>

Similar to [`whilst`](#whilst), except that instead of stopping when `test` returns **false**, it stops when `test` returns **true**.

<a name="doUntil">
#### doUntil
</a>

Similar to [`doWhilst`](#doWhilst), except that instead of stopping when `test` returns **false**, it stops when `test` returns **true**.

<a name="forever">
#### forever
</a>

Executes a function repetedly, until it passes a non-OK (non-zero) error code to its callback.  This is equivalent to [`whilst`](#whilst) with a `test` function that always returns **true**.

<a name="ntimes">
#### ntimes
</a>

Executes a function a given number of times, or until it passes a non-OK (non-zero) error code to its callback.

### Summary

Function | Concurrency | Executes vec of functions | Applies single function to data vec | Returns vec of results | Output vec same size as input
------------------------------- | :---:       | :-: | :-: | :-: | :-:
[each](#each)                   | limit = _n_ | no  | yes | no  | n/a
[map](#map)                     | limit = _n_ | no  | yes | yes | yes
[series](#series)               | 1           | yes | no  | yes | yes
[parallel](#parallel)           | no limit    | yes | no  | yes | yes
[parallelLimit](#parallelLimit) | limit = _n_ | yes | no  | yes | yes
[filter](#filter)               | limit = _n_ | no  | yes | yes | no
[reject](#reject)               | limit = _n_ | no  | yes | yes | no
[whilst](#whilst)               | 1           | no  | no  | no  | n/a
[doWhilst](#doWhilst)           | 1           | no  | no  | no  | n/a
[until](#until)                 | 1           | no  | no  | no  | n/a
[doUntil](#doUntil)             | 1           | no  | no  | no  | n/a
[forever](#forever)             | 1           | no  | no  | no  | n/a
[ntimes](#ntimes)               | 1           | no  | no  | no  | n/a

### Examples

Build using `scons`.  Binaries will be in `bin/` directory.

Examples are in [/examples](/examples) directory.

Run tests with `scons test`.


### Requirements

* C++11.
* SCons (`brew install scons`)
* Boost (`brew install boost`)

Tested with:

```
Apple LLVM version 5.1 (clang-503.0.40) (based on LLVM 3.4svn)
Target: x86_64-apple-darwin13.3.0
Thread model: posix
```

---------

Made with :horse: by Paul Rademacher.
