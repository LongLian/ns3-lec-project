#include "ns3/core-module.h"
#include <iostream>
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Hello5");

static void printHello(std::string word,std::string num,int frequency) {
	std::cout<<Simulator::Now()<<" Hello "<<word<<" -- "<<num<<std::endl;
	Simulator::Schedule(Seconds(frequency),&printHello,word,num,frequency);
}

int
main (int argc, char *argv[])
{
	CommandLine cmd;
	std::string name;
	std::string number;
	int freq;
	cmd.AddValue ("name", "my name 123123", name);
	cmd.AddValue ("number", "my name ", number);
	cmd.AddValue ("freq", "my", freq);
	cmd.Parse(argc,argv);

	printHello(name,number,freq);
	std::cout<<"hello commit"<<std::endl;

	Simulator::Stop(Seconds(50));
	Simulator::Run ();
	Simulator::Destroy ();
}

