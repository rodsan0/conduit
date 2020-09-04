TEST_CASE("Validity") { REPEAT {

  uit::Mesh<Spec> mesh{
    uit::RingTopologyFactory{}(num_threads),
    uit::AssignSegregated<uit::thread_id_t>{}
  };

  THREADED_BEGIN {

    auto input = mesh.GetSubmesh(thread_id)[0].GetInput(0);
    auto output = mesh.GetSubmesh(thread_id)[0].GetOutput(0);

    // 1/2 n * (n + 1)
    const int expected_sum = (std::kilo{}.num - 1) * std::kilo{}.num / 2;

    int sum{};

    for (MSG_T msg = 0; msg < std::kilo{}.num; ++msg) {

      output.Put(msg);

      const MSG_T received = input.JumpGet();
      REQUIRE( received <= expected_sum );
      REQUIRE( received >= 0 );
      sum += received;

    }

    while (sum != expected_sum) {
      const MSG_T received = input.JumpGet();
      REQUIRE( received <= expected_sum );
      REQUIRE( received >= 0);
      sum += received;
    }

    REQUIRE( sum == expected_sum );

    for (size_t i = 0; i < 10 * std::kilo{}.num; ++i) {
      REQUIRE( input.JumpGet() == 0 );
    }

  } THREADED_END


} }
