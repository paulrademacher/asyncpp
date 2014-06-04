# asyncpp

Asyncpp is a C++ utility library for asynchronous or functional programming using modern C++ lambdas, without getting into callback hell.

This is a C++ version of the [async](https://github.com/caolan/async) Node.js library.

This is useful, for example, with the Boost ASIO network library.  Instead of writing:

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

with **asyncpp** we can instead write this as a flat sequence of steps:


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

### Summary

Function | Concurrency | Executes vector<br> of functions | Applies single function<br>to data vector | Returns vector <br>of results | Output vector <br>same size as <br>input
--------------------------------- | :---:       | :-: | :-: | :-: | :-:
[`each`](#each)                   | limit = _n_ | no  | yes | no  | n/a
[`map`](#map)                     | limit = _n_ | no  | yes | yes | yes
[`series`](#series)               | 1           | yes | no  | yes | yes
[`parallel`](#parallel)           | no limit    | yes | no  | yes | yes
[`parallelLimit`](#parallelLimit) | limit = _n_ | yes | no  | yes | yes
[`filter`](#filter)               | limit = _n_ | no  | yes | yes | no
[`reject`](#reject)               | limit = _n_ | no  | yes | yes | no
[`whilst`](#whilst)               | 1           | no  | no  | no  | n/a
[`doWhilst`](#doWhilst)           | 1           | no  | no  | no  | n/a
[`until`](#until)                 | 1           | no  | no  | no  | n/a
[`doUntil`](#doUntil)             | 1           | no  | no  | no  | n/a
[`forever`](#forever)             | 1           | no  | no  | no  | n/a

### Examples

Build using `scons`.

    test/series.cpp
    test/series-boost-asio.cpp

### Requirements

* C++11.
* Boost, for examples.
* SCons, to build examples and tests.


---------

Made with :horse: by Paul Rademacher.
