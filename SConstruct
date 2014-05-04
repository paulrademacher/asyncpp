env = Environment(
    CXX="c++",
    CCFLAGS="-g -std=c++11 -stdlib=libc++", ## -Weverything",

    # /usr/local/lib
    LIBS=["boost_system", "boost_regex-mt", "boost_thread-mt", "boost_coroutine-mt"],

    LINKFLAGS="-stdlib=libc++")

env.Program(
    target="seriestest",
    source=["test/seriestest.cpp"])
