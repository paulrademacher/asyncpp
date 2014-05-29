// Common test utils and macros.

#define BEGIN_SEQUENCER_TEST(name) \
    BOOST_AUTO_TEST_CASE(name) { \
      BOOST_REQUIRE_EQUAL(async::get_sequencer_state_count(), 0); \
      // Run the sequencer code in its own scope, before then checking \
      // get_sequencer_state_count()==0.

#define END_SEQUENCER_TEST() \
  } \
  BOOST_CHECK_EQUAL(async::get_sequencer_state_count(), 0);


