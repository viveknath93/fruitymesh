////////////////////////////////////////////////////////////////////////////////
// /****************************************************************************
// **
// ** Copyright (C) 2015-2020 M-Way Solutions GmbH
// ** Contact: https://www.blureange.io/licensing
// **
// ** This file is part of the Bluerange/FruityMesh implementation
// **
// ** $BR_BEGIN_LICENSE:GPL-EXCEPT$
// ** Commercial License Usage
// ** Licensees holding valid commercial Bluerange licenses may use this file in
// ** accordance with the commercial license agreement provided with the
// ** Software or, alternatively, in accordance with the terms contained in
// ** a written agreement between them and M-Way Solutions GmbH.
// ** For licensing terms and conditions see https://www.bluerange.io/terms-conditions. For further
// ** information use the contact form at https://www.bluerange.io/contact.
// **
// ** GNU General Public License Usage
// ** Alternatively, this file may be used under the terms of the GNU
// ** General Public License version 3 as published by the Free Software
// ** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
// ** included in the packaging of this file. Please review the following
// ** information to ensure the GNU General Public License requirements will
// ** be met: https://www.gnu.org/licenses/gpl-3.0.html.
// **
// ** $BR_END_LICENSE$
// **
// ****************************************************************************/
////////////////////////////////////////////////////////////////////////////////
#include "gtest/gtest.h"
#include "Utility.h"
#include "CherrySimTester.h"
#include "CherrySimUtils.h"
#include "Logger.h"
#include <string>
#include "GlobalState.h"
#include "Config.h"
#include "Node.h"

TEST(TestNode, TestCommands) {
    CherrySimTesterConfig testerConfig = CherrySimTester::CreateDefaultTesterConfiguration();
    SimConfiguration simConfig = CherrySimTester::CreateDefaultSimConfiguration();
    simConfig.terminalId = 0;
    //testerConfig.verbose = true;
    simConfig.nodeConfigName.insert({ "prod_sink_nrf52", 1});
    simConfig.nodeConfigName.insert({ "prod_mesh_nrf52", 1});
    simConfig.SetToPerfectConditions();
    CherrySimTester tester = CherrySimTester(testerConfig, simConfig);
    tester.Start();
    tester.SimulateUntilClusteringDone(100 * 1000);

    tester.sim->findNodeById(1)->gs.logger.enableTag("CM");
    tester.sim->findNodeById(2)->gs.logger.enableTag("CM");

    tester.SendTerminalCommand(1, "action 2 node ping");
    tester.SimulateUntilMessageReceived(10 * 1000, 1, "{\"type\":\"ping\",\"nodeId\":2,\"module\":0,\"requestHandle\":0}");
    tester.SendTerminalCommand(1, "action 2 node ping 100");
    tester.SimulateUntilMessageReceived(10 * 1000, 1, "{\"type\":\"ping\",\"nodeId\":2,\"module\":0,\"requestHandle\":100}");

    tester.SendTerminalCommand(1, "disconnect all");
    tester.SimulateUntilMessageReceived(10 * 1000, 1, "Cleaning up conn ");
    tester.SimulateUntilClusteringDone(100 * 1000);

    tester.SendTerminalCommand(1, "action 2 node discovery off");
    tester.SimulateUntilMessageReceived(10 * 1000, 2, "-- DISCOVERY OFF --");
    tester.SendTerminalCommand(1, "action 2 node discovery on");
    tester.SimulateUntilMessageReceived(10 * 1000, 2, "-- DISCOVERY HIGH --");

    tester.SendTerminalCommand(1, "reset");
    tester.SimulateForGivenTime(10 * 1000);
    tester.SimulateUntilClusteringDone(100 * 1000);

    ASSERT_EQ(tester.sim->nodes[0].restartCounter, 2);

    tester.SendTerminalCommand(1, "status");
    tester.SimulateUntilRegexMessageReceived(10 * 1000, 1, "Node BBBBB \\(nodeId: 1\\) vers: \\d+, NodeKey: 01:00:....:00:00");

    tester.SendTerminalCommand(1, "bufferstat");
    tester.SimulateUntilMessageReceived(10 * 1000, 1, "JOIN_ME Buffer:");

    tester.SendTerminalCommand(1, "datal r");
    tester.SimulateUntilMessageReceived(10 * 1000, 2, "11:08:8C");

    tester.SendTerminalCommand(1, "settime 1337 0");
    tester.SimulateGivenNumberOfSteps(1);
    ASSERT_EQ(tester.sim->findNodeById(1)->gs.timeManager.GetTime(), 1337);

    tester.SendTerminalCommand(1, "gettime");
    tester.SimulateUntilRegexMessageReceived(10 * 1000, 1, "Time is currently approx. 1970 years, 1 days, 00h:22m:17s,\\d+ ticks");
    
    tester.SendTerminalCommand(1, "stop");
    tester.SimulateUntilMessageReceived(10 * 1000, 1, "Stopping state machine.");
    tester.SendTerminalCommand(1, "start");
    tester.SimulateUntilMessageReceived(10 * 1000, 1, "Starting state machine.");

    NodeIndexSetter setter(0);
    BaseConnections bc = GS->cm.GetBaseConnections(ConnectionDirection::INVALID);
    ASSERT_EQ(bc.count, 1);
    tester.SendTerminalCommand(1, "gap_disconnect %d", bc.handles[0].GetConnection()->connectionHandle);


    tester.SimulateUntilMessageReceived(10 * 1000, 1, "Disconnected device 22");


    //tester.SendTerminalCommand(1, "update_iv 2 20");
    //tester.SimulateUntilMessageReceived(10 * 1000, 2, "New cluster id generated ");
    //tester.SimulateUntilClusteringDone(100 * 1000);

    tester.SimulateUntilClusteringDone(10 * 1000);

    tester.SendTerminalCommand(1, "stopterm");
    tester.SimulateGivenNumberOfSteps(1);
    ASSERT_EQ(tester.sim->findNodeById(1)->gs.config.terminalMode, TerminalMode::JSON);

    tester.SendTerminalCommand(1, "startterm");
    tester.SimulateGivenNumberOfSteps(1);
    ASSERT_EQ(tester.sim->findNodeById(1)->gs.config.terminalMode, TerminalMode::PROMPT);

    tester.SendTerminalCommand(1, "get_modules 2");
    tester.SimulateUntilMessageReceived(10 * 1000, 1, "{\"nodeId\":2,\"type\":\"module_list\",\"modules\":");

    tester.SendTerminalCommand(1, "sep");
    tester.SimulateUntilMessageReceived(10 * 1000, 1, "################################################################################");

    tester.SendTerminalCommand(1, "set_serial BRTCR");
    tester.SimulateUntilMessageReceived(100, 1, "Serial Number Index set to 364543");
    tester.SimulateUntilMessageReceived(10 * 1000, 1, "{\"type\":\"reboot\",\"reason\":19"); //19 is set_serial_success
    tester.SendTerminalCommand(1, "status");
    tester.SimulateUntilRegexMessageReceived(10 * 1000, 1, "Node BRTCR \\(nodeId: 1\\) vers: \\d+, NodeKey: 01:00:....:00:00");

    tester.SendTerminalCommand(1, "set_node_key 00:11:22:33:44:55:66:77:88:99:AA:BB:CC:DD:EE:FF");
    tester.SimulateUntilMessageReceived(100, 1, "Node Key set to 00:11:22:33:44:55:66:77:88:99:AA:BB:CC:DD:EE:FF");
    tester.SendTerminalCommand(1, "status");
    tester.SimulateUntilRegexMessageReceived(10 * 1000, 1, "Node BRTCR \\(nodeId: 1\\) vers: \\d+, NodeKey: 00:11:....:EE:FF");

    //Test if the serial number and node key was persisted.
    tester.SendTerminalCommand(1, "reset");
    tester.SimulateGivenNumberOfSteps(10);
    ASSERT_EQ(tester.sim->nodes[0].restartCounter, 4);
    tester.SendTerminalCommand(1, "status");
    tester.SimulateUntilRegexMessageReceived(10 * 1000, 1, "Node BRTCR \\(nodeId: 1\\) vers: \\d+, NodeKey: 00:11:....:EE:FF");

    tester.SendTerminalCommand(1, "action 0 node reset");
    tester.SimulateUntilMessageReceived(10 * 1000, 1, "Scheduled reboot in 10 seconds");
    tester.SimulateForGivenTime(20 * 1000);
    tester.SimulateUntilClusteringDone(10 * 1000);
    ASSERT_EQ(tester.sim->nodes[0].restartCounter, 5);

    tester.SendTerminalCommand(1, "component_sense 0 123 0 7 77 qrs=");
    tester.SimulateUntilMessageReceived(10 * 1000, 1, "{"
                "\"nodeId\":1,"
                "\"type\":\"component_sense\","
                "\"module\":123,"
                "\"requestHandle\":0,"
                "\"actionType\":0,"
                "\"component\":\"0x0007\","
                "\"register\":\"0x004D\","
                "\"payload\":\"qrs=\""
            "}");

    tester.SendTerminalCommand(1, "component_act 0 123 0 7 77 qrs=");
    tester.SimulateUntilMessageReceived(10 * 1000, 1, "component_act payload = AA:BB");

    tester.SendTerminalCommand(1, "get_plugged_in");
    tester.SimulateUntilMessageReceived(10 * 1000, 1, "plugged_in");

    tester.SimulateUntilClusteringDone(10 * 1000);
    tester.SendTerminalCommand(1, "action 2 node set_preferred_connections penalty 101 102 103 104 105 106 107 108");
    tester.SimulateUntilMessageReceived(10 * 1000, 1, "{\"type\":\"set_preferred_connections_result\",\"nodeId\":2,\"module\":0}");
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.amountOfPreferredPartnerIds, 8);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredConnectionMode, PreferredConnectionMode::PENALTY);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[0], 101);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[1], 102);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[2], 103);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[3], 104);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[4], 105);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[5], 106);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[6], 107);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[7], 108);
    tester.SendTerminalCommand(1, "action 2 node set_preferred_connections ignored 1337 42");
    tester.SimulateUntilMessageReceived(10 * 1000, 1, "{\"type\":\"set_preferred_connections_result\",\"nodeId\":2,\"module\":0}");
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.amountOfPreferredPartnerIds, 2);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredConnectionMode, PreferredConnectionMode::IGNORED);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[0], 1337);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[1], 42);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[2], 103);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[3], 104);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[4], 105);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[5], 106);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[6], 107);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[7], 108);
    tester.SendTerminalCommand(1, "action 2 node set_preferred_connections ignored");
    tester.SimulateUntilMessageReceived(10 * 1000, 1, "{\"type\":\"set_preferred_connections_result\",\"nodeId\":2,\"module\":0}");
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.amountOfPreferredPartnerIds, 0);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredConnectionMode, PreferredConnectionMode::IGNORED);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[0], 1337);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[1], 42);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[2], 103);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[3], 104);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[4], 105);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[5], 106);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[6], 107);
    ASSERT_EQ(tester.sim->nodes[1].gs.config.configuration.preferredPartnerIds[7], 108);
}

TEST(TestNode, TestCRCValidation)
{
    CherrySimTesterConfig testerConfig = CherrySimTester::CreateDefaultTesterConfiguration();
    SimConfiguration simConfig = CherrySimTester::CreateDefaultSimConfiguration();
    //testerConfig.verbose = true;
    simConfig.nodeConfigName.insert({ "prod_sink_nrf52", 1});
    simConfig.nodeConfigName.insert({ "prod_mesh_nrf52", 1});
    simConfig.SetToPerfectConditions();
    CherrySimTester tester = CherrySimTester(testerConfig, simConfig);
    tester.Start();
    tester.SimulateUntilClusteringDone(100 * 1000);

    //Make sure that the commands are working without CRC...
    NodeIndexSetter setter(0);
    GS->terminal.DisableCrcChecks();
    tester.SendTerminalCommand(1, "action this status get_status");
    tester.SimulateUntilMessageReceived(1 * 1000, 1, "\"type\":\"status\",\"");
    //...and don't return any CRC.
    tester.SendTerminalCommand(1, "action this status get_status");
    {
        Exceptions::DisableDebugBreakOnException disable;
        ASSERT_THROW(tester.SimulateUntilMessageReceived(1 * 1000, 1, "CRC:"), TimeoutException);
    }

    //Enable CRC checks
    tester.SendTerminalCommand(1, "enable_corruption_check");
    tester.SimulateUntilMessageReceived(1 * 1000, 1, "{\"type\":\"enable_corruption_check_response\",\"err\":0,\"check\":\"crc32\"}");

    //Once crc is enabled, commands require a CRC at the end.
    tester.SendTerminalCommand(1, "action this status get_status CRC: 3968018817");
    tester.SimulateUntilRegexMessageReceived(1 * 1000, 1, "\"initialized\":0\\} CRC: \\d+");

    //Missing CRC is no longer accepted
    {
        Exceptions::DisableDebugBreakOnException disable;
        tester.appendCrcToMessages = false;
        tester.SendTerminalCommand(1, "action this status get_status");
        ASSERT_THROW(tester.SimulateGivenNumberOfSteps(1), CRCMissingException);
        tester.appendCrcToMessages = true;

        //Empty the Command Buffer
        Exceptions::ExceptionDisabler<CRCMissingException> crcme;
        tester.SimulateGivenNumberOfSteps(1);
    }


    //Invalid CRC is not accepted
    {
        Exceptions::DisableDebugBreakOnException disable;
        tester.SendTerminalCommand(1, "action this status get_status CRC: 12345");
        ASSERT_THROW(tester.SimulateGivenNumberOfSteps(1), CRCInvalidException);
    }
}

TEST(TestNode, TestGenerateLoad) {
    CherrySimTesterConfig testerConfig = CherrySimTester::CreateDefaultTesterConfiguration();
    SimConfiguration simConfig = CherrySimTester::CreateDefaultSimConfiguration();
    //testerConfig.verbose = true;
    simConfig.nodeConfigName.insert({ "prod_sink_nrf52", 1});
    simConfig.nodeConfigName.insert({ "prod_mesh_nrf52", 2});
    simConfig.SetToPerfectConditions();
    CherrySimTester tester = CherrySimTester(testerConfig, simConfig);
    tester.Start();
    tester.SimulateUntilClusteringDone(100 * 1000);

    //Without request handle
    tester.SendTerminalCommand(1, "action 2 node generate_load 3 10 1 13");
    tester.SimulateUntilMessageReceived(10 * 1000, 2, "Generating load. Target: 3 size: 10 amount: 1 interval: 13 requestHandle: 0");
    tester.SimulateUntilMessageReceived(10 * 1000, 1, "{\"type\":\"start_generate_load_result\",\"nodeId\":2,\"requestHandle\":0}");
    tester.SimulateUntilMessageReceived(10 * 1000, 3, "{\"type\":\"generate_load_chunk\",\"nodeId\":2,\"size\":10,\"payloadCorrect\":1,\"requestHandle\":0}");

    //With request handle
    tester.SendTerminalCommand(1, "action 2 node generate_load 3 10 1 13 18");
    tester.SimulateUntilMessageReceived(10 * 1000, 2, "Generating load. Target: 3 size: 10 amount: 1 interval: 13 requestHandle: 18");
    tester.SimulateUntilMessageReceived(10 * 1000, 1, "{\"type\":\"start_generate_load_result\",\"nodeId\":2,\"requestHandle\":18}");
    tester.SimulateUntilMessageReceived(10 * 1000, 3, "{\"type\":\"generate_load_chunk\",\"nodeId\":2,\"size\":10,\"payloadCorrect\":1,\"requestHandle\":18}");

    //Multiple messages
    tester.SendTerminalCommand(1, "action 2 node generate_load 3 10 5 13");
    tester.SimulateUntilMessageReceived(10 * 1000, 2, "Generating load. Target: 3 size: 10 amount: 5 interval: 13 requestHandle: 0");
    tester.SimulateUntilMessageReceived(10 * 1000, 1, "{\"type\":\"start_generate_load_result\",\"nodeId\":2,\"requestHandle\":0}");
    for(int i = 0; i<5; i++) tester.SimulateUntilMessageReceived(10 * 1000, 3, "{\"type\":\"generate_load_chunk\",\"nodeId\":2,\"size\":10,\"payloadCorrect\":1,\"requestHandle\":0}");
}

TEST(TestNode, TestPreferredConnections) {
    CherrySimTesterConfig testerConfig = CherrySimTester::CreateDefaultTesterConfiguration();
    SimConfiguration simConfig = CherrySimTester::CreateDefaultSimConfiguration();
    simConfig.terminalId = 0;
    simConfig.preDefinedPositions = {
        {0.45, 0.45},
        {0.5 , 0.45},
        {0.55, 0.45},
        {0.45, 0.5 },
        {0.5 , 0.5 },
        {0.55, 0.5 },
        {0.45, 0.55},
        {0.5 , 0.55},
        {0.55, 0.55},
        {0.6 , 0.55},
    };

    //testerConfig.verbose = true;
    simConfig.nodeConfigName.insert({ "prod_sink_nrf52", 1});
    simConfig.nodeConfigName.insert({ "prod_mesh_nrf52", 9});
    simConfig.SetToPerfectConditions();
    CherrySimTester tester = CherrySimTester(testerConfig, simConfig);
    tester.Start();

    tester.SimulateUntilClusteringDone(100 * 1000);

    tester.SendTerminalCommand(1, "action this node set_preferred_connections ignored 2"); tester.SimulateForGivenTime(1 * 500);
    tester.SendTerminalCommand(1, "action 2 node set_preferred_connections ignored 1 3");  tester.SimulateForGivenTime(1 * 500);
    tester.SendTerminalCommand(1, "action 3 node set_preferred_connections ignored 2 4");  tester.SimulateForGivenTime(1 * 500);
    tester.SendTerminalCommand(1, "action 4 node set_preferred_connections ignored 3 5");  tester.SimulateForGivenTime(1 * 500);
    tester.SendTerminalCommand(1, "action 5 node set_preferred_connections ignored 4 6");  tester.SimulateForGivenTime(1 * 500);
    tester.SendTerminalCommand(1, "action 6 node set_preferred_connections ignored 5 7");  tester.SimulateForGivenTime(1 * 500);
    tester.SendTerminalCommand(1, "action 7 node set_preferred_connections ignored 6 8");  tester.SimulateForGivenTime(1 * 500);
    tester.SendTerminalCommand(1, "action 8 node set_preferred_connections ignored 7 9");  tester.SimulateForGivenTime(1 * 500);
    tester.SendTerminalCommand(1, "action 9 node set_preferred_connections ignored 8 10"); tester.SimulateForGivenTime(1 * 500);
    tester.SendTerminalCommand(1, "action 10 node set_preferred_connections ignored 9");   tester.SimulateForGivenTime(1 * 500);

    tester.SimulateForGivenTime(20 * 1000);    //Make sure that the nodes rebooted.
    tester.SimulateUntilClusteringDone(100 * 1000);

    for (u32 i = 0; i < tester.sim->getTotalNodes(); i++) {
        NodeIndexSetter setter(i);
        const nodeEntry* node = &(tester.sim->nodes[i]);
        const MeshConnections conns = node->gs.cm.GetMeshConnections(ConnectionDirection::INVALID);

        bool hasLowerIdConnected  = false;
        bool hasHigherIdConnected = false;

        if (i == 0)
        {
            hasLowerIdConnected = true; // Not actually true, but he has no lower id to connect to!
            ASSERT_EQ(conns.count, 1);
        }
        else if (i == tester.sim->getTotalNodes() - 1)
        {
            hasHigherIdConnected = true; //Not actually true, but he has no higher id to connect to!
            ASSERT_EQ(conns.count, 1);
        }
        else
        {
            ASSERT_EQ(conns.count, 2);
        }

        for (int j = 0; j < conns.count; j++) {
            const MeshConnection* conn = conns.handles[j].GetConnection();
            const int myId = i + 1;
            if      (conn->partnerId == myId + 1) hasHigherIdConnected = true;
            else if (conn->partnerId == myId - 1) hasLowerIdConnected  = true;
            else FAIL() << "Node " << myId << " was connected to" << conn->partnerId << "!";
        }

        ASSERT_TRUE(hasLowerIdConnected);
        ASSERT_TRUE(hasHigherIdConnected);
    }

}

//FIXME: Can be enabled again once we have refactored the high-to-low discovery (IOT-2953)
TEST(TestNode, DISABLED_TestDiscoveryStates) {
    CherrySimTesterConfig testerConfig = CherrySimTester::CreateDefaultTesterConfiguration();
    SimConfiguration simConfig = CherrySimTester::CreateDefaultSimConfiguration();
    simConfig.nodeConfigName.insert({ "prod_sink_nrf52", 1});
    simConfig.nodeConfigName.insert({ "prod_mesh_nrf52", 1});
    //testerConfig.verbose = true;
    CherrySimTester tester = CherrySimTester(testerConfig, simConfig);
    tester.Start();

    //Change the state to low discovery
    tester.SimulateUntilMessageReceived(10 * 60 * 1000, 2, "-- DISCOVERY LOW --");

    //Scan duty when asset tracking job is enabled and cluster is in low discovery
    tester.SimulateForGivenTime(30 * 1000);
    u16 interval = Conf::getInstance().meshScanIntervalHigh;
    ASSERT_EQ(MSEC_TO_UNITS(tester.sim->findNodeById(2)->state.scanIntervalMs, CONFIG_UNIT_0_625_MS), interval);

    //Switch to High discovery when receive enrollment
    tester.SendTerminalCommand(1, "action 2 enroll basic BBBBF 2 200");

    tester.SimulateUntilMessageReceived(1000, 2, "-- DISCOVERY HIGH --");

    //Send change to high discovery message after given time
    tester.SendTerminalCommand(1, "gap_disconnect 0");
    tester.SimulateUntilMessageReceived(50*1000, 2, "High Discovery message being sent after disconnect");

}

//Tests sending various length packets (split/not split) over normal prio queue concurrently
//with high prio packets over a MeshConnection to see if acknowledgement works as expected
TEST(TestNode, TestMeshConnectionPacketQueuing) {
    CherrySimTesterConfig testerConfig = CherrySimTester::CreateDefaultTesterConfiguration();
    SimConfiguration simConfig = CherrySimTester::CreateDefaultSimConfiguration();
    simConfig.terminalId = 0;
    //testerConfig.verbose = true;
    simConfig.nodeConfigName.insert({ "prod_sink_nrf52", 1});
    simConfig.nodeConfigName.insert({ "prod_mesh_nrf52", 1});
    simConfig.SetToPerfectConditions();
    CherrySimTester tester = CherrySimTester(testerConfig, simConfig);
    tester.Start();
    tester.SimulateUntilClusteringDone(10 * 1000);

    u8 buffer[100];

    connPacketHeader* header = (connPacketHeader*)buffer;
    header->messageType = MessageType::DATA_1;
    header->sender = 1;
    header->receiver = 2;

    u32* counter = (u32*)(buffer + SIZEOF_CONN_PACKET_HEADER);

    //We send split packets and high prio packets interleaved with each other and check
    //if an exception is thrown by the implementation, we add a counter to the packets
    int k = 0, l = 0;
    for (int i = 0; i < 500; i++)
    {
        //Normal prio packets
        if (PSRNGINT(0, 100) % 2 == 0) {

            *counter = k++;
            u16 len = 9 + PSRNGINT(0, 35);

            char bufferHex[400];
            Logger::convertBufferToHexString(buffer, len, bufferHex, sizeof(bufferHex));

            tester.SendTerminalCommand(1, "rawsend %s", bufferHex);
        }
        //High prio packets
        else {
            *counter = 10000 + l++;
            u16 len = 9;

            char bufferHex[400];
            Logger::convertBufferToHexString(buffer, len, bufferHex, sizeof(bufferHex));

            tester.SendTerminalCommand(1, "rawsend_high %s", bufferHex);
        }

        //Make sure that data is really transmitted every one in a while
        if (i>0 && i % 50 == 0) {
            printf("--------------------------------------------" EOL);
            tester.SimulateUntilMessageReceived(10 * 1000, 2, "Got Data packet");
        }

        tester.SimulateGivenNumberOfSteps(1);
    }
}

//Tests sending various length packets (split/not split) over normal prio queue concurrently
//with high prio packets over a MeshAccessConnection to see if acknowledgement works as expected
TEST(TestNode, TestMeshAccessConnectionPacketQueuing) {
    CherrySimTesterConfig testerConfig = CherrySimTester::CreateDefaultTesterConfiguration();
    SimConfiguration simConfig = CherrySimTester::CreateDefaultSimConfiguration();
    simConfig.terminalId = 0;
    //testerConfig.verbose = true;
    simConfig.nodeConfigName.insert({ "prod_sink_nrf52", 1});
    simConfig.nodeConfigName.insert({ "prod_mesh_nrf52", 1});
    simConfig.SetToPerfectConditions();
    CherrySimTester tester = CherrySimTester(testerConfig, simConfig);

    tester.sim->nodes[1].uicr.CUSTOMER[9] = 123; // Change default network id of node 2

    tester.Start();

    //Connect to node 2 using a mesh access connection and the network key
    tester.SendTerminalCommand(1, "action this ma connect 00:00:00:02:00:00 2 04:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
    
    //Wait until connection was set up
    tester.SimulateUntilMessageReceived(10 * 1000, 1, "Received remote mesh data");
    
    u8 buffer[100];

    connPacketHeader* header = (connPacketHeader*)buffer;
    header->messageType = MessageType::DATA_1;
    header->sender = 1;
    header->receiver = 2001; //virtual partner id

    u32* counter = (u32*)(buffer + SIZEOF_CONN_PACKET_HEADER);

    //We send split packets and high prio packets interleaved with each other and check
    //if an exception is thrown by the implementation, we add a counter to the packets
    int k = 0, l = 0;
    for (int i = 0; i < 500; i++)
    {
        //Normal prio packets
        if (PSRNGINT(0, 100) % 2 == 0) {

            *counter = k++;
            u16 len = 9 + PSRNGINT(0, 35);

            char bufferHex[400];
            Logger::convertBufferToHexString(buffer, len, bufferHex, sizeof(bufferHex));

            tester.SendTerminalCommand(1, "rawsend %s", bufferHex);
        }
        //High prio packets
        else {
            *counter = 10000 + l++;
            u16 len = 9;

            char bufferHex[400];
            Logger::convertBufferToHexString(buffer, len, bufferHex, sizeof(bufferHex));

            tester.SendTerminalCommand(1, "rawsend_high %s", bufferHex);
        }

        //Make sure that data is really transmitted every one in a while
        if (i > 0 && i % 50 == 0) {
            tester.SimulateUntilMessageReceived(10 * 1000, 2, "Got Data packet");
        }

        tester.SimulateGivenNumberOfSteps(1);
    }
}

TEST(TestNode, TestCapabilitySending) {
    CherrySimTesterConfig testerConfig = CherrySimTester::CreateDefaultTesterConfiguration();
    SimConfiguration simConfig = CherrySimTester::CreateDefaultSimConfiguration();
    simConfig.terminalId = 0;
    //testerConfig.verbose = true;
    simConfig.nodeConfigName.insert({ "prod_sink_nrf52", 1});
    simConfig.nodeConfigName.insert({ "prod_mesh_nrf52", 1});
    simConfig.SetToPerfectConditions();
    CherrySimTester tester = CherrySimTester(testerConfig, simConfig);

    tester.Start();
    tester.SimulateUntilClusteringDone(10 * 1000);
    tester.sim->enableTagForAll("VSMOD");

    tester.SendTerminalCommand(1, "request_capability 2");

    //Order matters, so we simulate for each message one by one.
    tester.SimulateUntilMessageReceived(10 * 1000, 2, "Capabilities are requested");
    tester.SimulateUntilRegexMessageReceived(10 * 1000, 1, "\\{\"nodeId\":2,\"type\":\"capability_entry\",\"index\":0,\"capabilityType\":2,\"manufacturer\":\"M-Way Solutions GmbH\",\"model\":\"BlueRange Node\",\"revision\":\"\\d+.\\d+.\\d+\"\\}");
    tester.SimulateUntilRegexMessageReceived(10 * 1000, 1, "\\{\"nodeId\":2,\"type\":\"capability_end\",\"amount\":\\d+\\}");
}

TEST(TestNode, TestRapidDisconnections) {
    CherrySimTesterConfig testerConfig = CherrySimTester::CreateDefaultTesterConfiguration();
    SimConfiguration simConfig = CherrySimTester::CreateDefaultSimConfiguration();
    //testerConfig.verbose = true;
    simConfig.nodeConfigName.insert({ "prod_sink_nrf52", 1});
    simConfig.nodeConfigName.insert({ "prod_mesh_nrf52", 1});
    simConfig.SetToPerfectConditions();
    CherrySimTester tester = CherrySimTester(testerConfig, simConfig);
    tester.Start();

    tester.SimulateUntilClusteringDone(10 * 1000);

    //Simulate for some time so that the mesh connection is deemed stable
    tester.SimulateForGivenTime(11 * 1000);

    //Simulate very frequent connection losses
    tester.sim->simConfig.connectionTimeoutProbabilityPerSec = 0.1;

    //We just simulate for some time to see if sth. in the node crashes
    tester.SimulateForGivenTime(20 * 1000);

}

//This test should check that no packets are dropped or duplicated during successful reestablishments
TEST(TestNode, TestReconnectionPacketQueuing) {
    CherrySimTesterConfig testerConfig = CherrySimTester::CreateDefaultTesterConfiguration();
    SimConfiguration simConfig = CherrySimTester::CreateDefaultSimConfiguration();

    //The reestablishment ist not optimized to work if the SoftDevice returns busy
    simConfig.sdBusyProbability = 0;

    //testerConfig.verbose = true;
    simConfig.nodeConfigName.insert({ "prod_sink_nrf52", 1});
    simConfig.nodeConfigName.insert({ "prod_mesh_nrf52", 1});
    simConfig.SetToPerfectConditions();
    CherrySimTester tester = CherrySimTester(testerConfig, simConfig);
    tester.Start();

    tester.sim->nodes[1].gs.logger.enableTag("DEBUGMOD");

    tester.SimulateUntilClusteringDone(10 * 1000);

    //Start a counter that can be used to check if all packets are arriving
    tester.SendTerminalCommand(1, "action this debug counter 2 10 100000");

    //Simulate a connection loss every few seconds
    for (int i = 0; i < 50; i++) {
        //Simulate for some time so that the mesh connection is deemed stable
        tester.SimulateForGivenTime(PSRNGINT(11000, 16000));

        //Iterate over all connections and simulate a timeout
        for (int j = 0; j < SIM_MAX_CONNECTION_NUM; j++) {
            tester.sim->DisconnectSimulatorConnection(&cherrySimInstance->nodes[0].state.connections[j], BLE_HCI_CONNECTION_TIMEOUT, BLE_HCI_CONNECTION_TIMEOUT);
        }
    }

    //Check that the counter is still at a correct value afterwards
    tester.SimulateUntilMessageReceived(200 * 1000, 2, "Counter correct at");
}

TEST(TestNode, TestReestablishmentTimesOut) {
    CherrySimTesterConfig testerConfig = CherrySimTester::CreateDefaultTesterConfiguration();
    SimConfiguration simConfig = CherrySimTester::CreateDefaultSimConfiguration();
    simConfig.terminalId = 0;
    //Place nodes such that they are only reachable in a line.
    simConfig.preDefinedPositions = { {0.2, 0.5}, {0.4, 0.55}, {0.6, 0.5}, {0.8, 0.55} };
    //testerConfig.verbose = true;
    simConfig.nodeConfigName.insert({ "prod_sink_nrf52", 1});
    simConfig.nodeConfigName.insert({ "prod_mesh_nrf52", 3});
    simConfig.SetToPerfectConditions();
    CherrySimTester tester = CherrySimTester(testerConfig, simConfig);
    tester.Start();

    //Wait until nodes 2, 3, and 4 are connected
    tester.SimulateUntilClusteringDone(100 * 1000);

    //Remove connection to node 4 by generating a timeout (Might happen if the disconnect before a reboot is not received)
    for (int i = 0; i < SIM_MAX_CONNECTION_NUM; i++) {
        tester.sim->DisconnectSimulatorConnection(&tester.sim->nodes[3].state.connections[i], BLE_HCI_CONNECTION_TIMEOUT, BLE_HCI_CONNECTION_TIMEOUT);
    }

    //Remove enrollment of node 4 to make it inactive
    tester.SendTerminalCommand(4, "action this enroll remove BBBBF");

    tester.SimulateUntilMessageReceived(5 * 1000, 4, "reboot");

    //Check that the reestablishment is tried a few times
    tester.SimulateUntilMessageReceived(5 * 1000, 3, "Connection Timeout");
    tester.SimulateUntilMessageReceived(5 * 1000, 3, "Connection Timeout");

    //Wait until node 3 adjusted its clusterSize (meaning it dropped the connection etc.)
    //This should happen after a timeout of currently 10 seconds
    u16 extendedTimeout = tester.sim->nodes[0].gs.config.meshExtendedConnectionTimeoutSec;
    tester.SimulateUntilMessageReceived(extendedTimeout * 1000, 3, "\"clusterSize\":3");
}