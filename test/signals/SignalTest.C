// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WObject.h>
#include <Wt/WSignal.h>

#include <iostream>
#include <functional>

BOOST_AUTO_TEST_CASE( test_signals1 )
{
  Wt::Signal<> signal;

  bool success = false;

  signal.connect(std::bind([&](){
    success = true;
  }));

  signal();

  BOOST_REQUIRE(success);
}

BOOST_AUTO_TEST_CASE( test_signals2 )
{
  Wt::Signals::connection connection;

  {
    Wt::Signal<> signal;
    connection = signal.connect([](){
      std::cout << "What this does does not matter" << std::endl;
    });
  }

  Wt::Signal<> signal2;
  signal2.connect([](){
    std::cout << "What this does also does not matter" << std::endl;
  });

  // signal2 will normally take the memory location of signal
  // If the connection ring is incorrectly implemented, this could
  // cause the check below to fail. In that case, valgrind will also
  // detect an invalid read.

  BOOST_REQUIRE(!connection.isConnected());

  connection.disconnect();

  BOOST_REQUIRE(!connection.isConnected());
}

BOOST_AUTO_TEST_CASE( test_signals3 )
{
  Wt::Signal<> signal;

  bool executed = false;

  auto connection = signal.connect([&](){
    executed = true;
  });

  Wt::Signals::connection connectionCopy = connection;

  BOOST_REQUIRE(connection.isConnected());
  BOOST_REQUIRE(connectionCopy.isConnected());
  BOOST_REQUIRE(signal.isConnected());

  signal();

  BOOST_REQUIRE(executed);

  connectionCopy.disconnect();

  BOOST_REQUIRE(!connection.isConnected());
  BOOST_REQUIRE(!connectionCopy.isConnected());
  BOOST_REQUIRE(!signal.isConnected());

  executed = false;

  signal();

  BOOST_REQUIRE(!executed);
}

BOOST_AUTO_TEST_CASE( test_signals4 )
{
  Wt::Signal<> signal;
  Wt::Signals::connection connection;

  connection = signal.connect([&](){
    connection.disconnect();
  });

  signal();

  BOOST_REQUIRE(!signal.isConnected());
  BOOST_REQUIRE(!connection.isConnected());
}

class DeleteMe : public Wt::WObject
{
public:
  Wt::Signal<> signal;
  void doDelete() {
    delete this;
  }
};

BOOST_AUTO_TEST_CASE( test_signals5 )
{
  auto dm = new DeleteMe();

  bool before = false;
  bool after = false;

  Wt::Signal<> signal;
  signal.connect([&](){
    before = true;
  });
  auto connection = signal.connect(dm, &DeleteMe::doDelete);
  signal.connect([&](){
    after = true;
  });

  signal();

  BOOST_REQUIRE(signal.isConnected());
  BOOST_REQUIRE(before);
  BOOST_REQUIRE(after);
  BOOST_REQUIRE(!connection.isConnected());

  before = false;
  after = false;

  signal();

  BOOST_REQUIRE(before);
  BOOST_REQUIRE(after);
}

BOOST_AUTO_TEST_CASE( test_signals6 )
{
  auto dm = new DeleteMe();

  bool before = false;
  bool after = false;

  auto conn1 = dm->signal.connect([&](){
    before = true;
  });
  auto conn2 = dm->signal.connect([dm](){
    delete dm;
  });
  auto conn3 = dm->signal.connect([&](){
    after = true;
  });

  dm->signal();

  BOOST_REQUIRE(before);
  BOOST_REQUIRE(after);
  BOOST_REQUIRE(!conn1.isConnected());
  BOOST_REQUIRE(!conn2.isConnected());
  BOOST_REQUIRE(!conn3.isConnected());
}

BOOST_AUTO_TEST_CASE( test_signals7 )
{
  auto dm = new DeleteMe();

  bool before = false;
  bool after = false;

  auto conn1 = dm->signal.connect([&](){
    before = true;
  });
  auto conn2 = dm->signal.connect(dm, &DeleteMe::doDelete);
  auto conn3 = dm->signal.connect([&](){
    after = true;
  });

  dm->signal();

  BOOST_REQUIRE(before);
  BOOST_REQUIRE(after);
  BOOST_REQUIRE(!conn1.isConnected());
  BOOST_REQUIRE(!conn2.isConnected());
  BOOST_REQUIRE(!conn3.isConnected());
}

BOOST_AUTO_TEST_CASE( test_signals8 )
{
  int before = 0;
  int after = 0;
  int i = 0;

  auto dm = new DeleteMe();

  dm->signal.connect([&](){
    ++before;
  });
  dm->signal.connect([&,dm](){
    ++i;
    if (i == 1) {
      dm->signal();
    } else if (i == 2) {
      delete dm;
    }
  });
  dm->signal.connect([&](){
    ++after;
  });

  dm->signal();

  BOOST_REQUIRE(before == 2);
  BOOST_REQUIRE(after == 2);
  BOOST_REQUIRE(i == 2);
}

// This test should not leak
BOOST_AUTO_TEST_CASE( test_signals9 )
{
  Wt::Signal<> signal;

  signal.connect([](){
    throw 4;
  });

  try {
    signal();
  } catch (int i) {
    BOOST_REQUIRE(i == 4);
  }
}

BOOST_AUTO_TEST_CASE( test_signals10 )
{
  int i = 0;

  Wt::Signal<> signal;
  signal.connect([&i,&signal]{
    signal.connect([&i]{
      ++i;
    });
  });

  signal();
  BOOST_REQUIRE(i == 0); // Still one (++i is a slot once)
  signal();
  BOOST_REQUIRE(i == 1); // ++i called once, ++i is a slot twice now
  signal();
  BOOST_REQUIRE(i == 3); // ++i called twice. ++i is a slot three times
}

BOOST_AUTO_TEST_CASE( test_signals11 )
{
  int i = 0;

  Wt::Signal<> signal;
  signal.connect([&i,&signal]{
    ++i;
    signal.connect([&i]{
      ++i;
    });
    if (i == 1)
      signal();
  });

  signal();
  // Sequence of events:
  // - ++i is called (-> i == 1)
  // - ++i slot is connected (once now)
  // - i == 1, so signal is emitted again
  //   - ++i is called (-> i == 2)
  //   - ++i slot is connected (twice now)
  //   - i != 1, so signal is NOT emitted again
  //   - first ++i slot is executed (-> i == 3)
  BOOST_REQUIRE_EQUAL(i, 3);
  i = 0;
  signal();
  // Sequence of events:
  // - ++i is called (-> i == 1)
  // - ++i slot is connected (three times now)
  // - i == 1, so signal is emitted again
  //   - ++i is called (-> i == 2)
  //   - ++i slot is connected (four times now)
  //   - i != 1, so signal is NOT emitted again
  //   - first i++ slot is executed (-> i == 3)
  //   - second i++ slot is executed (-> i == 4)
  //   - third i++ slot is executed (-> i == 5)
  // - first i++ slot is executed (-> i == 6)
  // - second i++ slot is executed (-> i == 7)
  BOOST_REQUIRE_EQUAL(i, 7);
}

BOOST_AUTO_TEST_CASE( test_signals12 )
{
  // Test self-assignment
  int i = 0;
  Wt::Signal<> signal;
  Wt::Signals::connection conn = signal.connect([&i]{ ++i; });
  BOOST_REQUIRE(conn.isConnected());
  signal();
  BOOST_REQUIRE_EQUAL(i, 1);
  conn = conn;
  BOOST_REQUIRE(conn.isConnected());
  signal();
  BOOST_REQUIRE_EQUAL(i, 2);
}

BOOST_AUTO_TEST_CASE( test_signals13 )
{
  // Test moved from state
  // Moved from state should be equivalent to default-constructed state
  Wt::Signal<> signal;
  Wt::Signals::connection conn = signal.connect([]{});
  BOOST_REQUIRE(conn.isConnected());
  Wt::Signals::connection conn2 = std::move(conn);
  BOOST_REQUIRE(!conn.isConnected());
  BOOST_REQUIRE(conn2.isConnected());
  conn = std::move(conn2);
  BOOST_REQUIRE(conn.isConnected());
  BOOST_REQUIRE(!conn2.isConnected());
}

#if __cplusplus >= 201402L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201402L)
BOOST_AUTO_TEST_CASE( test_signals14 )
{
  struct movable_bool_ref {
    movable_bool_ref(bool &b) noexcept
      : b_{&b}
    { }

    ~movable_bool_ref() noexcept
    {
      if (b_)
        *b_ = true;
    }

    movable_bool_ref(const movable_bool_ref& other) noexcept
      : b_{nullptr}
    {
      *this = other;
    }

    movable_bool_ref& operator=(const movable_bool_ref& other) noexcept
    {
      if (this == &other)
        return *this;

      b_ = other.b_;
      other.b_ = nullptr;

      return *this;
    }

    movable_bool_ref(movable_bool_ref&& other) noexcept
      : b_{nullptr}
    {
      *this = std::move(other);
    }

    movable_bool_ref& operator=(movable_bool_ref&& other) noexcept
    {
      if (this == &other)
        return *this;

      b_ = other.b_;
      other.b_ = nullptr;

      return *this;
    }

    mutable bool *b_;
  };

  Wt::Signal<> signal;
  Wt::Signals::connection conn;
  bool destroyed = false;

  conn = signal.connect([&conn, r = movable_bool_ref{destroyed}](){
    conn.disconnect();
  });

  BOOST_REQUIRE(!destroyed);
  BOOST_REQUIRE(signal.isConnected());

  signal();

  BOOST_REQUIRE(destroyed);
  BOOST_REQUIRE(!signal.isConnected());
}
#endif
