#include "actuators/ConfigurableThrusterModels.h"
#include <iostream>
#include "data_tamer/data_tamer.hpp"

#include "data_tamer/sinks/mcap_sink.hpp"

int main()
{
    sf::tm::BasicThrustConversion basic0001(0.001);
    sf::tm::BasicThrustConversion basic0005(0.005);
    sf::tm::BasicThrustConversion basic001(0.01);
    sf::tm::BasicThrustConversion basic005(0.05);
    sf::tm::BasicThrustConversion basic01(0.1);
    sf::tm::BasicThrustConversion basic05(0.5);

    sf::tm::DeadBandConversion db0001(0.001, 0.001 , - 30 * 30, 30 * 30);
    sf::tm::DeadBandConversion db0005(0.005, 0.005 , - 30 * 30, 30 * 30);
    sf::tm::DeadBandConversion db001(0.01, 0.01 , - 30 * 30, 30 * 30);
    sf::tm::DeadBandConversion db005(0.05, 0.05 , - 30 * 30, 30 * 30);
    sf::tm::DeadBandConversion db01(0.1, 0.1 , - 30 * 30, 30 * 30);
    sf::tm::DeadBandConversion db05(0.5, 0.5 , - 30 * 30, 30 * 30);

    double input;
    double output0001, output0005, output001, output005, output01, output05;
    double outputdb0001, outputdb0005, outputdb001, outputdb005, outputdb01, outputdb05;

    // Multiple channels can use this sink. Data will be saved in mylog.mcap
  auto mcap_sink = std::make_shared<DataTamer::MCAPSink>("mylog2.mcap");

  // Create a channel and attach a sink. A channel can have multiple sinks
  auto channel = DataTamer::LogChannel::create("conversions");
  channel->addDataSink(mcap_sink);

  // You can register any arithmetic value. You are responsible for their lifetime!
  auto id1 = channel->registerValue("input", &input);
    auto id2 = channel->registerValue("out0.001", &output0001);
    auto id3 = channel->registerValue("out0.005", &output0005);
    auto id4 = channel->registerValue("out0.01", &output001);
    auto id5 = channel->registerValue("out0.05", &output005);
    auto id6 = channel->registerValue("out0.1", &output01);
    auto id7 = channel->registerValue("out0.5", &output05);

    auto id8 = channel->registerValue("outdb0.001", &outputdb0001);
    auto id9 = channel->registerValue("outdb0.005", &outputdb0005);
    auto id10 = channel->registerValue("outdb0.01", &outputdb001);
    auto id11 = channel->registerValue("outdb0.05", &outputdb005);
    auto id12 = channel->registerValue("outdb0.1", &outputdb01);
    auto id13 = channel->registerValue("outdb0.5", &outputdb05);


  // loop. First 10 seconds, setpoint is 0. Next 10 seconds setpoint is 20. Next 10 seconds, setpoint is again 0
   double dt = 0.001;

   double start = -100;
    double end = 100;


    for (double i = start; i < end; i+=dt)
    {
        input = i;
        output0001 = basic0001.f(input);
        output0005 = basic0005.f(input);
        output001 = basic001.f(input);
        output005 = basic005.f(input);
        output01 = basic01.f(input);
        output05 = basic05.f(input);

        outputdb0001 = db0001.f(input);
        outputdb0005 = db0005.f(input);
        outputdb001 = db001.f(input);
        outputdb005 = db005.f(input);
        outputdb01 = db01.f(input);
        outputdb05 = db05.f(input);

    channel->takeSnapshot();
    std::cout << "input: " << input << std::endl;
    }

    return 0;


    // sleep for dt seconds
    //std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(dt * 1000)));
}

