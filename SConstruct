env = Environment(
    CXX="c++",
    CCFLAGS="-g -std=c++11 -stdlib=libc++", ## -Weverything",

    # /usr/local/lib
    LIBS=["boost_system", "boost_regex-mt", "boost_thread-mt", "boost_coroutine-mt"],

    LINKFLAGS="-stdlib=libc++")

examples = [
    env.Program(target="bin/each", source=["examples/each.cpp"]),
    env.Program(target="bin/filter", source=["examples/filter.cpp"]),
    env.Program(target="bin/forever", source=["examples/forever.cpp"]),
    env.Program(target="bin/map", source=["examples/map.cpp"]),
    env.Program(target="bin/sequencer", source=["examples/sequencer.cpp"]),
    env.Program(target="bin/series", source=["examples/series.cpp"]),
    env.Program(target="bin/series-boost-asio", source=["examples/series-boost-asio.cpp"]),
    env.Program(target="bin/whilst", source=["examples/whilst.cpp"]),
    env.Program(target="bin/http-client", source=["examples/http-client.cpp"]),
    ]

tests = [
    env.Program(target="bin/maptest", source=["test/maptest.cpp"]),
    env.Program(target="bin/seriestest", source=["test/seriestest.cpp"]),
    ]

test_alias = Alias("test", tests, [t[0].path for t in tests])

# Simply required.  Without it, these are never considered out of date.
AlwaysBuild(test_alias)
AlwaysBuild(examples)

