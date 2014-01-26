#include "stdafx.h"
#include <string>
#include <map>
#include <fstream>
#include <iterator>
#include <vector>

int main(int argc, char *argv[])
{
	unsigned int state = 0;
	bool waiting = false;
	int key;

	std::ifstream file("script.txt");
	std::vector<std::string> commands;
	std::map<std::string, unsigned int> labels;
	if (file.is_open())
	{
		std::copy(std::istream_iterator<std::string>(file), std::istream_iterator<std::string>(), std::back_inserter(commands));
		printf("Count: %d\n", commands.size());
	}
	for (std::vector<std::string>::iterator it = commands.begin(); it != commands.end(); ++it)
	{
		std::string command = *it;
		if (command.at(0) == ':')
		{
			std::string label = command.substr(1, std::string::npos);
			size_t position = it - commands.begin();
			labels[label] = position;
		}
	}
	//std::string commands[10] = { "+clock", "morning", "bing", "!name", "bing", "welcome", "name" };
	std::map<std::string, unsigned int> sounds;
	unsigned int pos = 0;

	SoundController *system = new SoundController();

	/*soundMorning = system->createSound("morning.wav");
	soundBing = system->createSound("bing.wav");
	soundWelcome = system->createSound("welcome.wav");
	soundClock = system->createSound("clock.wav");*/


	/*
	Main loop.
	*/
	bool paused = false;
	key = 0;
	std::string option1, option2;
	int finalOption = 0;
	while (key != 27)
	{
		if (_kbhit())
		{
			key = _getch();

			if (key == ' ')
			{
				paused = !paused;
				system->setPaused(paused);
				printf(paused ? "Paused\n" : "Resumed\n");
			}

			if (waiting)
			{
				switch (key)
				{
				case 75:
					printf("left\n");
					pos = labels[option1];
					waiting = false;
					break;
				case 77:
					printf("right\n");
					pos = labels[option2];
					waiting = false;
					break;
				}
			}
		}

		/*if (waiting)
		{
			if (!system->isWaiting())
			{
				++pos;
				printf("Position: %d\n", pos);
				waiting = false;
			}
		}*/


		if (!system->isWaiting() && pos < commands.size() && !waiting && !paused)
		{
			printf("Position: %d\n", pos);

			std::string command = commands[pos];
			printf("Command: %s\n", command.c_str());

			char first = command.at(0);
			std::string rest = command.substr(1, std::string::npos);

			char effect = '0';
			if (first == '%')
			{
				effect = rest.at(0);
				command = command.substr(2, std::string::npos);
				first = command.at(0);
				rest = command.substr(1, std::string::npos);
			}

			unsigned int sound;
			switch (first)
			{
			case '+':
				if (sounds.count(rest) == 0)
				{
					std::string filename = rest + ".wav";
					sounds[rest] = system->createSound(filename.c_str());
				}
				sound = sounds[rest];
				system->play(sound, true, false, effect);
				break;
			case '-':
				if (sounds.count(rest) == 1)
				{
					sound = sounds[rest];
					system->stop(sound);
				}
				break;
			case '!':
				sounds[rest] = system->record(2);
				break;
			case '\\':
				//system->enableEffect(false);
				break;
			case '?':
				{
					size_t p = rest.find_first_of(':');
					option1 = rest.substr(0, p);
					option2 = rest.substr(p+1, std::string::npos);
					printf("First: \"%s\" Second: \"%s\"\n", option1.c_str(), option2.c_str());
					waiting = true;
					break;
				}
			case ':':
				break;
			case '>':
				pos = labels[rest];
				break;
			case '@':
				++finalOption;
				break;
			case '=':
				switch (finalOption)
				{
				case 0:
					pos = labels["end1"];
					break;
				case 1:
					pos = labels["end2"];
					break;
				case 2:
					pos = labels["end3"];
					break;
				case 3:
					pos = labels["end4"];
					break;
				}
				break;
			default:
				if (sounds.count(command) == 0)
				{
					std::string filename = command + ".wav";
					sounds[command] = system->createSound(filename.c_str());
				}
				sound = sounds[command];
				system->play(sound, false, true, effect);
				break;
			}

			++pos;

			/*switch (state)
			{
			case 0:
			system->play(soundClock, true, false);
			system->play(soundMorning, false, true);
			waiting = true;
			break;
			case 1:
			system->play(soundBing, false, true);
			waiting = true;
			break;
			case 2:
			recName = system->record(2);
			waiting = true;
			break;
			case 3:
			system->play(soundBing, false, true);
			waiting = true;
			break;
			case 4:
			system->play(soundWelcome, false, true);
			waiting = true;
			break;
			case 5:
			system->play(recName, false, true);
			waiting = true;
			break;
			}*/
		}

		system->update();

		Sleep(10);

	}

	printf("\n");

	/*
	Shut down
	*/
	delete system;

	return 0;
}
