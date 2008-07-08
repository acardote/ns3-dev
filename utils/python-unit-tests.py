import unittest
import ns3

class TestSimulator(unittest.TestCase):

    def testScheduleNow(self):
        def callback(args):
            self._args_received = args
            self._cb_time = ns3.Simulator.Now()
        ns3.Simulator.Destroy()
        self._args_received = None
        self._cb_time = None
        ns3.Simulator.ScheduleNow(callback, "args")
        ns3.Simulator.Run()
        self.assertEqual(self._args_received, "args")
        self.assertEqual(self._cb_time.GetSeconds(), 0.0)

    def testSchedule(self):
        def callback(args):
            self._args_received = args
            self._cb_time = ns3.Simulator.Now()
        ns3.Simulator.Destroy()
        self._args_received = None
        self._cb_time = None
        ns3.Simulator.Schedule(ns3.Seconds(123), callback, "args")
        ns3.Simulator.Run()
        self.assertEqual(self._args_received, "args")
        self.assertEqual(self._cb_time.GetSeconds(), 123.0)

    def testScheduleDestroy(self):
        def callback(args):
            self._args_received = args
            self._cb_time = ns3.Simulator.Now()
        ns3.Simulator.Destroy()
        self._args_received = None
        self._cb_time = None
        def null(): pass
        ns3.Simulator.Schedule(ns3.Seconds(123), null)
        ns3.Simulator.ScheduleDestroy(callback, "args")
        ns3.Simulator.Run()
        ns3.Simulator.Destroy()
        self.assertEqual(self._args_received, "args")
        self.assertEqual(self._cb_time.GetSeconds(), 123.0)

    if 0: # these tests are known to fail for now (pybindgen limitation)
        def testTime_EQ(self):
            self.assert_(ns3.Seconds(123) == ns3.Seconds(123))
        def testTime_GE(self):
            self.assert_(ns3.Seconds(123) >= ns3.Seconds(123))
        def testTime_LE(self):
            self.assert_(ns3.Seconds(123) <= ns3.Seconds(123))
        def testTime_GT(self):
            self.assert_(ns3.Seconds(124) > ns3.Seconds(123))
        def testTime_LT(self):
            self.assert_(ns3.Seconds(123) < ns3.Seconds(124))

    def testConfig(self):
        ns3.Config.Set("ns3::OnOffApplication::PacketSize", ns3.UintegerValue(123))
        ns3.Config.SetDefault("ns3::OnOffApplication::PacketSize", ns3.UintegerValue(123))
        # hm.. no Config.Get?

    if 0:
        # not yet: https://bugs.launchpad.net/pybindgen/+bug/246069
        def testSocket(self):
            node = ns3.Node()
            ns3.AddInternetStack(node)
            self._received_packet = None

            def rx_callback(socket):
                assert self._received_packet is None
                self._received_packet = socket.Recv()

            sink = ns3.Socket.CreateSocket(node, ns3.TypeId.LookupByName("ns3::UdpSocketFactory"))
            sink.Bind(ns3.InetSocketAddress(ns3.Ipv4Address.GetAny(), 80))
            sink.SetRecvCallback(rx_callback)

            source = ns3.Socket.CreateSocket(node, ns3.TypeId.LookupByName("ns3::UdpSocketFactory"))
            source.SendTo(ns3.InetSocketAddress(ns3.Ipv4Address.GetAny(), 80), ns3.Packet(19))

            ns3.Simulator.Run()
            self.assert_(self._received_packet is not None)
            self.assertEqual(self._received_packet.GetSize(), 19)
        

if __name__ == '__main__':
    unittest.main()
