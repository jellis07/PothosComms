// Copyright (c) 2015-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <iostream>
#include <json.hpp>

using json = nlohmann::json;

POTHOS_TEST_BLOCK("/comms/tests", test_simple_llc)
{
    const uint8_t port = 123;

    //create side A test blocks
    auto feederA = Pothos::BlockRegistry::make("/blocks/feeder_source", "uint8");
    feederA.call("setName", "feederA");
    auto collectorA = Pothos::BlockRegistry::make("/blocks/collector_sink", "uint8");
    collectorA.call("setName", "collectorA");
    auto llcA = Pothos::BlockRegistry::make("/comms/simple_llc");
    llcA.call("setName", "llcA");
    llcA.call("setRecipient", 0xB); //sends to size B
    llcA.call("setPort", port);
    auto macA = Pothos::BlockRegistry::make("/comms/simple_mac");
    macA.call("setName", "macA");
    macA.call("setMacId", 0xA);

    //create side B test blocks
    auto feederB = Pothos::BlockRegistry::make("/blocks/feeder_source", "uint8");
    feederB.call("setName", "feederB");
    auto collectorB = Pothos::BlockRegistry::make("/blocks/collector_sink", "uint8");
    collectorB.call("setName", "collectorB");
    auto llcB = Pothos::BlockRegistry::make("/comms/simple_llc");
    llcB.call("setName", "llcB");
    llcB.call("setRecipient", 0xA); //sends to size A
    llcB.call("setPort", port);
    auto macB = Pothos::BlockRegistry::make("/comms/simple_mac");
    macB.call("setName", "macB");
    macB.call("setMacId", 0xB);

    //create a test packet
    Pothos::Packet pktA2B;
    pktA2B.payload = Pothos::BufferChunk("uint8", 100);
    for (size_t i = 0; i < pktA2B.payload.elements(); i++)
        pktA2B.payload.as<unsigned char *>()[i] = std::rand() & 0xff;
    feederA.call("feedPacket", pktA2B);

    //create a test packet
    Pothos::Packet pktB2A;
    pktB2A.payload = Pothos::BufferChunk("uint8", 100);
    for (size_t i = 0; i < pktB2A.payload.elements(); i++)
        pktB2A.payload.as<unsigned char *>()[i] = std::rand() & 0xff;
    feederB.call("feedPacket", pktB2A);

    //setup the topology
    Pothos::Topology topology;

    //connect collector/feeder A to LLC A
    topology.connect(feederA, 0, llcA, "dataIn");
    topology.connect(llcA, "dataOut", collectorA, 0);

    //connect MAC A to LLC A
    topology.connect(llcA, "macOut", macA, "macIn");
    topology.connect(macA, "macOut", llcA, "macIn");

    //connect collector/feeder B to LLC B
    topology.connect(feederB, 0, llcB, "dataIn");
    topology.connect(llcB, "dataOut", collectorB, 0);

    //connect MAC B to LLC B
    topology.connect(llcB, "macOut", macB, "macIn");
    topology.connect(macB, "macOut", llcB, "macIn");

    //connect MAC A to MAC B
    topology.connect(macA, "phyOut", macB, "phyIn");
    topology.connect(macB, "phyOut", macA, "phyIn");

    //run the design
    topology.commit();
    POTHOS_TEST_TRUE(topology.waitInactive());
    //std::cout << topology.queryJSONStats() << std::endl;

    //check side A
    POTHOS_TEST_EQUAL(macA.call<unsigned long long>("getErrorCount"), 0);
    const std::vector<Pothos::Packet> packetsA = collectorA.call("getPackets");
    POTHOS_TEST_EQUAL(packetsA.size(), 1);
    const auto pktOutA0 = packetsA.at(0);
    POTHOS_TEST_EQUAL(pktB2A.payload.dtype, pktOutA0.payload.dtype);
    POTHOS_TEST_EQUAL(pktB2A.payload.elements(), pktOutA0.payload.elements());
    POTHOS_TEST_EQUALA(pktB2A.payload.as<const unsigned char *>(),
        pktOutA0.payload.as<const unsigned char *>(), pktOutA0.payload.elements());

    //check side B
    POTHOS_TEST_EQUAL(macB.call<unsigned long long>("getErrorCount"), 0);
    const std::vector<Pothos::Packet> packetsB = collectorB.call("getPackets");
    POTHOS_TEST_EQUAL(packetsB.size(), 1);
    const auto pktOutB0 = packetsB.at(0);
    POTHOS_TEST_EQUAL(pktA2B.payload.dtype, pktOutB0.payload.dtype);
    POTHOS_TEST_EQUAL(pktA2B.payload.elements(), pktOutB0.payload.elements());
    POTHOS_TEST_EQUALA(pktA2B.payload.as<const unsigned char *>(),
        pktOutB0.payload.as<const unsigned char *>(), pktOutB0.payload.elements());
}

POTHOS_TEST_BLOCK("/comms/tests", test_simple_llc_harsh)
{
    /*!
     * FIXME disabling this test for now because
     * its too random and tightly timed to operate
     * as a good unit test. May revisit in the future.
     */
    return;

    const uint8_t port = 123;

    //create side A test blocks
    auto feederA = Pothos::BlockRegistry::make("/blocks/feeder_source", "uint8");
    feederA.call("setName", "feederA");
    auto collectorA = Pothos::BlockRegistry::make("/blocks/collector_sink", "uint8");
    collectorA.call("setName", "collectorA");
    auto llcA = Pothos::BlockRegistry::make("/comms/simple_llc");
    llcA.call("setName", "llcA");
    llcA.call("setRecipient", 0xB); //sends to size B
    llcA.call("setPort", port);
    llcA.call("setResendTimeout", 0.1);
    llcA.call("setExpireTimeout", 1.0);
    auto macA = Pothos::BlockRegistry::make("/comms/simple_mac");
    macA.call("setName", "macA");
    macA.call("setMacId", 0xA);

    //create side B test blocks
    auto feederB = Pothos::BlockRegistry::make("/blocks/feeder_source", "uint8");
    feederB.call("setName", "feederB");
    auto collectorB = Pothos::BlockRegistry::make("/blocks/collector_sink", "uint8");
    collectorB.call("setName", "collectorB");
    auto llcB = Pothos::BlockRegistry::make("/comms/simple_llc");
    llcB.call("setName", "llcB");
    llcB.call("setRecipient", 0xA); //sends to size A
    llcB.call("setPort", port);
    llcB.call("setResendTimeout", 0.1);
    llcB.call("setExpireTimeout", 1.0);
    auto macB = Pothos::BlockRegistry::make("/comms/simple_mac");
    macB.call("setName", "macB");
    macB.call("setMacId", 0xB);

    //create dropper blocks for connection
    const double dropChance = 0.05; //chance of dropping out of 1.0
    auto dropperA2B = Pothos::BlockRegistry::make("/blocks/sporadic_dropper");
    dropperA2B.call("setProbability", dropChance); //chance of drop
    auto dropperB2A = Pothos::BlockRegistry::make("/blocks/sporadic_dropper");
    dropperB2A.call("setProbability", dropChance); //chance of drop

    //create the test plan for both feeders
    json testPlan;
    testPlan["enablePackets"] = true;
    testPlan["minBuffers"] = 500; //many packets
    testPlan["maxBuffers"] = 500; //many packets
    auto expectedA2B = feederA.call("feedTestPlan", testPlan.dump());
    auto expectedB2A = feederB.call("feedTestPlan", testPlan.dump());

    //setup the topology
    Pothos::Topology topology;

    //connect collector/feeder A to LLC A
    topology.connect(feederA, 0, llcA, "dataIn");
    topology.connect(llcA, "dataOut", collectorA, 0);

    //connect MAC A to LLC A
    topology.connect(llcA, "macOut", macA, "macIn");
    topology.connect(macA, "macOut", llcA, "macIn");

    //connect collector/feeder B to LLC B
    topology.connect(feederB, 0, llcB, "dataIn");
    topology.connect(llcB, "dataOut", collectorB, 0);

    //connect MAC B to LLC B
    topology.connect(llcB, "macOut", macB, "macIn");
    topology.connect(macB, "macOut", llcB, "macIn");

    //connect MAC A to MAC B through dropper blocks
    topology.connect(macA, "phyOut", dropperA2B, 0);
    topology.connect(dropperA2B, 0, macB, "phyIn");
    topology.connect(macB, "phyOut", dropperB2A, 0);
    topology.connect(dropperB2A, 0, macA, "phyIn");

    //run the design
    topology.commit();
    POTHOS_TEST_TRUE(topology.waitInactive(0.5, 0.0));
    //std::cout << topology.queryJSONStats() << std::endl;

    //check the results
    std::cout << "llcA resend count " << llcA.call<unsigned long long>("getResendCount") << std::endl;
    std::cout << "llcA expired count " << llcA.call<unsigned long long>("getExpiredCount") << std::endl;
    std::cout << "llcB resend count " << llcB.call<unsigned long long>("getResendCount") << std::endl;
    std::cout << "llcB expired count " << llcB.call<unsigned long long>("getExpiredCount") << std::endl;
    POTHOS_TEST_EQUAL(llcA.call<unsigned long long>("getExpiredCount"), 0);
    POTHOS_TEST_EQUAL(llcB.call<unsigned long long>("getExpiredCount"), 0);
    collectorA.call("verifyTestPlan", expectedB2A);
    collectorB.call("verifyTestPlan", expectedA2B);
}
