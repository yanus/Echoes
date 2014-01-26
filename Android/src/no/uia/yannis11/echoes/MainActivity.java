package no.uia.yannis11.echoes;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.Scanner;

import org.fmod.FMODAudioDevice;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class MainActivity extends Activity
{
	private FMODAudioDevice mFMODAudioDevice = new FMODAudioDevice();
	private MainLoop loop;
	private ArrayList<String> commands;

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		Button button = (Button) findViewById(R.id.button1);
		button.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				if (MainActivity.this.loop == null)
				{
					MainActivity.this.loop = new MainLoop(MainActivity.this, MainActivity.this.commands);
					MainActivity.this.loop.setRunning(true);
					MainActivity.this.loop.start();
				}
			}
		});

		button = (Button) findViewById(R.id.button2);
		button.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				if (MainActivity.this.loop != null)
				{
					MainActivity.this.loop.setInput(1);
				}
			}
		});

		button = (Button) findViewById(R.id.button3);
		button.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				if (MainActivity.this.loop != null)
				{
					MainActivity.this.loop.setInput(2);
				}
			}
		});
	}

	@Override
	protected void onStart()
	{
		super.onStart();
		mFMODAudioDevice.start();
		cInit();

		Scanner s = new Scanner(getResources().openRawResource(R.raw.script));
		commands = new ArrayList<String>();
		while (s.hasNext())
			commands.add(s.next());
		s.close();
	}

	@Override
	protected void onStop()
	{
		boolean retry = true;
		while (retry)
		{
			try
			{
				loop.join();
				retry = false;
			} catch (InterruptedException e) {}
		}
		cEnd();
		mFMODAudioDevice.stop();
		super.onStop();
	}

	static 
	{
		System.loadLibrary("fmodex");
		System.loadLibrary("Echoes");
	}

	public native void cInit();
	public native void cEnd();
	public native int cCreateSound(String filename);
	public native void cStop(int index);
	public native void cPlay(int index, boolean loop, boolean wait, char effect);
	public native void cSetPaused(boolean paused);
	public native boolean cIsWaiting();
	public native int cRecord(int length);
	public native void cUpdate();
}
