#include "actuators/ConfigurableThrusterModels.h"
#include <iostream>
#include "data_tamer/data_tamer.hpp"

#include "data_tamer/sinks/mcap_sink.hpp"

int main()
{
    sf::td::ZeroOrder zeroOrder;
    sf::td::FirstOrder firstOrder(1.0);
    sf::td::Yoerger yoerger(0.037, 16.5);
    sf::td::Bessa bessa(1,1,1,1,1);

    double time = 0.0;
    double sp = 0.0;

    double outputZeroOrder = zeroOrder.f(time, sp);
    double outputFirstOrder = firstOrder.f(time, sp);
    double outputYoerger = yoerger.f(time, sp);
    double outputBessa = bessa.f(time, sp);

    // Multiple channels can use this sink. Data will be saved in mylog.mcap
  auto mcap_sink = std::make_shared<DataTamer::MCAPSink>("mylog.mcap");

  // Create a channel and attach a sink. A channel can have multiple sinks
  auto channel = DataTamer::LogChannel::create("dynamics");
  channel->addDataSink(mcap_sink);

  // You can register any arithmetic value. You are responsible for their lifetime!
  auto id1 = channel->registerValue("outputZeroOrder", &outputZeroOrder);
  auto id2 = channel->registerValue("outputFirstOrder", &outputFirstOrder);
    auto id3 = channel->registerValue("outputYoerger", &outputYoerger);
    auto id4 = channel->registerValue("outputBessa", &outputBessa);
    auto id0 = channel->registerValue("setpoint", &sp);

  // loop. First 10 seconds, setpoint is 0. Next 10 seconds setpoint is 20. Next 10 seconds, setpoint is again 0
   double dt = 0.001;

while(time < 90.0)
{
    if(time < 10.0)
    {
        sp = 0.0;
    }
    else if(time < 20.0)
    {
        sp = 50.0;
    }
    
    else if (time < 40)
    {
        sp = 0.0;
    }
    else if (time < 60)
    {
        sp = 450.0;
    }
    else 
    {
        sp = 0.0;
    }

    outputZeroOrder = zeroOrder.f(time, sp);
    outputFirstOrder = firstOrder.f(time, sp);
    outputYoerger = yoerger.f(time, sp);
    outputBessa = bessa.f(time, sp);

    std::chrono::duration<double> duration(time);
    std::chrono::nanoseconds ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
    channel->takeSnapshot(ns);

    std::cout << "ns: " << ns.count() << std::endl;


    time += dt;

    // sleep for dt seconds
    //std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(dt * 1000)));
}


    return 0;
}