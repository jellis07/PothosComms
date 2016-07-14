// Copyright (c) 2015-2015 Rinat Zakirov
// Copyright (c) 2016-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Remote.hpp>
#include <Poco/JSON/Object.h>
#include <iostream>

POTHOS_TEST_BLOCK("/comms/tests", test_differential_coding)
{
    //run the topology
    for(int symbols = 2; symbols != 512; symbols *= 2)
    {
        std::cout << "run the topology with " << symbols << " symbols" << std::endl;

        auto feeder = Pothos::BlockRegistry::make("/blocks/feeder_source", "uint8");
        auto collector = Pothos::BlockRegistry::make("/blocks/collector_sink", "uint8");
        auto encoder = Pothos::BlockRegistry::make("/comms/differential_encoder");
        auto decoder = Pothos::BlockRegistry::make("/comms/differential_decoder");

        //create a test plan
        Poco::JSON::Object::Ptr testPlan(new Poco::JSON::Object());
        testPlan->set("enableBuffers", true);
        testPlan->set("minValue", 0);
        testPlan->set("maxValue", symbols - 1);

        encoder.callProxy("setSymbols", symbols);
        decoder.callProxy("setSymbols", symbols);

        Pothos::Topology topology;
        topology.connect(feeder, 0, encoder, 0);
        topology.connect(encoder, 0, decoder, 0);
        topology.connect(decoder, 0, collector, 0);
        topology.commit();

        auto expected = feeder.callProxy("feedTestPlan", testPlan);
        POTHOS_TEST_TRUE(topology.waitInactive());

        std::cout << "verifyTestPlan!\n";
        collector.callVoid("verifyTestPlan", expected);
    }

    std::cout << "done!\n";
}
