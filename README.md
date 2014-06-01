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

Function | Concurrency | Executes vector<br> of functions | Applies single function<br>to data vector | Returns vector <br>of results
--------------------------------- | :---: | :---: | :---: | :---:
[`map`](#map)                     | limit = _n_ | no | yes | yes
[`series`](#series)               | 1 | yes | no | yes
[`parallel`](#parallel)           | no limit | yes | no | yes
[`parallelLimit`](#parallelLimit)  | limit = _n_ | yes | no | yes
[`whilst`](#whilst)               | 1 | no | no | no
[`doWhilst`](#doWhilst)           | 1 | no | no | no
[`until`](#until)                 | 1 | no | no | no
[`doUntil`](#doUntil)             | 1 | no | no | no
[`forever`](#forever)             | 1 | no | no | no

<a name="series" />
#### series
</a>

Invokes a series of tasks, and collects result values into a vector. As each task completes, it invokes a callback with an error code and a result value. If the error code is not `async::OK`, iteration stops. Once all tasks complete or there is an error, `final_callback` is called with the last error and the results vector.

Each task may invoke its callback immediately, or at some point in the future.

<a name="parallel" />
#### parallel

<a name="parallelLimit" />
#### parallelLimit

<a name="whilst" />
#### whilst

<a name="doWhilst" />
#### doWhilst

<a name="until" />
#### until

<a name="doUntil" />
#### doUntil

<a name="forever" />
#### forever




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
