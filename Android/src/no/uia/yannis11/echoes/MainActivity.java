package no.uia.yannis11.echoes;

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
	private boolean active = false, paused = false;

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		final Button btnCancel = (Button) findViewById(R.id.btnCancel);
		final Button btnPlay = (Button) findViewById(R.id.btnPlay);
		btnCancel.setEnabled(false);
		btnCancel.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				if (active && loop != null)
				{
					loop.cancel();
					boolean retry = true;
					while (retry)
					{
						try
						{
							loop.join();
							retry = false;
						} catch (InterruptedException e) {}
					}
					cStopAll();
					
					active = false;
					
					btnPlay.setText(R.string.play);
					btnCancel.setEnabled(false);
				}
			}
		});

		btnPlay.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				if (!active)
				{
					// Start new game
					loop = new MainLoop(MainActivity.this, commands);
					loop.start();
					
					active = true;
					paused = false;
					
					btnPlay.setText(R.string.pause);
					btnCancel.setEnabled(true);
				}
				else if (!paused)
				{
					// Pause the game
					loop.setPaused(true);
					cSetPaused(true);
					
					paused = true;
					btnPlay.setText(R.string.resume);
				}
				else
				{
					// Resume the game
					loop.setPaused(false);
					cSetPaused(false);
					
					paused = false;
					btnPlay.setText(R.string.pause);
				}
			}
		});

		final Button btnLeft = (Button) findViewById(R.id.btnLeft);
		btnLeft.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				if (loop != null)
				{
					loop.setInput(1);
				}
			}
		});

		final Button btnRight = (Button) findViewById(R.id.btnRight);
		btnRight.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				if (loop != null)
				{
					loop.setInput(2);
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
	public native void cStopAll();
	public native void cPlay(int index, boolean loop, boolean wait, char effect);
	public native void cSetPaused(boolean paused);
	public native boolean cIsWaiting();
	public native int cRecord(int length);
	public native void cUpdate();
}
