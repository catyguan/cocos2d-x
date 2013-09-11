package org.cocos2dx.lib;

import android.content.Context;

public class Cocos2dxApp {
	
	public static final int FS_TYPE_All = 0;
	public static final int FS_TYPE_RESOURCE = 1;
	public static final int FS_TYPE_APPDATA = 2;
	public static final int FS_TYPE_LUA = 3;
	public static final int FS_TYPE_TEMP = 4;
	
	private Cocos2dxGLSurfaceView mGLSurfaceView;
	private Context context;
	
	public Cocos2dxApp(final Context context) {
		this.context = context;
		mGLSurfaceView = new Cocos2dxGLSurfaceView(this, context);
	}
	
	public Cocos2dxGLSurfaceView view()
	{
		return mGLSurfaceView;
	}
	
	public void onResume() {
		mGLSurfaceView.onResume();
	}
	
	public void onPause() {
		mGLSurfaceView.onPause();
	}
	
	public void startup() {
		nativeStartup();
	}
	
	public void run(long time) {
		nativeRun(time);
	}
	
	public void runOnAppThread(final Runnable pRunnable) {
		mGLSurfaceView.queueEvent(pRunnable);
	}
	
	public void handleOnPause() {
		nativeOnPause();
	}

	public void handleOnResume() {
		nativeOnResume();
	}

	public void config(String name, int v) {
		nativeConfig(name, 0, v, null);
	}
	public void config(String name, String v) {
		nativeConfig(name, 1, 0, v);
	}
	public void config(String name, boolean v) {
		nativeConfig(name, 2, v?1:0, null);
	}
	public void addFileSystem(int type, String path, boolean readonly) {
		nativeAddFileSystem(type, path, readonly);
	}
	
	private static native void nativeStartup();
	private static native void nativeRun(long time);
	private static native void nativeOnPause();
	private static native void nativeOnResume();
	private static native void nativeConfig(String name, int type, int v1, String v2);
	private static native void nativeAddFileSystem(int type, String path, boolean readonly);
	
	
}
