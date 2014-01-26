package no.uia.yannis11.echoes;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

public class MainLoop extends Thread
{
	private MainActivity parent;
	MainLoop(MainActivity parent, ArrayList<String> commands)
	{
		super();
		this.parent = parent;
		this.commands = commands;
	}
	
	private boolean running;
	public void setRunning(boolean running)
	{
		this.running = running;
	}
	
	private int input = 0;
	public void setInput(int i)
	{
		input = i;
	}
	
	private ArrayList<String> commands;// = new ArrayList<String>(Arrays.asList("+clock", "morning", "bing", "!name", "bing", "%1welcome", "%1name", "/"));
	
	private Map<String, Integer> labels = new HashMap<String, Integer>();
	
	private Map<String, Integer> sounds = new HashMap<String, Integer>();
	
	private boolean paused = false, waiting = false, canceled = false;
	
	public void setPaused(boolean paused) { this.paused = paused; };
	public void cancel() { this.canceled = true; };
	
	private int pos = 0;
	
	String option1, option2;
	int finalOption = 0;
	
	@Override
	public void run()
	{
		for (int i = 0; i < commands.size(); ++i)
		{
			String command = commands.get(i);

			if (command.charAt(0) == ':')
			{
				String label = command.substring(1);
				labels.put(label, i);
			}
		}
		
		while (pos < commands.size() && !canceled)
		{
			if (input > 0)
			{
				if (waiting)
				{
					switch (input)
					{
					case 1:
						waiting = false;
						pos = labels.get(option1);
						break;
					case 2:
						waiting = false;
						pos = labels.get(option2);
						break;
					}
				}
				
				input = 0;
			}
			
			if (!parent.cIsWaiting() && pos < commands.size() && !waiting && !paused)
			{
				String command = commands.get(pos);
				char first = command.charAt(0);
				String rest = command.substring(1);
				
				char effect = '0';
				if (first == '%')
				{
					effect = rest.charAt(0);
					command = command.substring(2);
					first = command.charAt(0);
					rest = command.substring(1);
				}
				
				int sound;
				switch (first)
				{
				case '+':
					if (!sounds.containsKey(rest))
					{
						String filename = "/sdcard/echoes/" + rest + ".mp3";
						sounds.put(rest, parent.cCreateSound(filename));
					}
					sound = sounds.get(rest);
					parent.cPlay(sound, true, false, effect);
					break;
				case '-':
					if (sounds.containsKey(rest))
					{
						sound = sounds.get(rest);
						parent.cStop(sound);
					}
					break;
				case '~':
					parent.cStopAll();
					break;
				case '!':
					sounds.put(rest, parent.cRecord(2));
					break;
				case '?':
					int p = rest.indexOf(':');
					option1 = rest.substring(0, p);
					option2 = rest.substring(p+1);
					waiting = true;
					break;
				case '#':
				case ':':
					break;
				case '>':
					pos = labels.get(rest);
					break;
				case '@':
					++finalOption;
					break;
				case '=':
					switch (finalOption)
					{
					case 0:
						pos = labels.get("end1");
						break;
					case 1:
						pos = labels.get("end2");
						break;
					case 2:
						pos = labels.get("end3");
						break;
					case 3:
						pos = labels.get("end4");
						break;
					}
					break;
				default:
					if (!sounds.containsKey(command))
					{
						String filename = "/sdcard/echoes/" + command + ".mp3";
						sounds.put(command, parent.cCreateSound(filename));
					}
					sound = sounds.get(command);
					parent.cPlay(sound, false, true, effect);
					break;
				}
				
				++pos;
			}
			
			parent.cUpdate();
			
			try
			{
				sleep(10);
			} catch (InterruptedException e)
			{
				e.printStackTrace();
			}
		}
		//parent.cEnd();
	}
}
