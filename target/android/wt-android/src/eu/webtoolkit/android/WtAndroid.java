/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

package eu.webtoolkit.android;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import android.app.Activity;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;
import android.webkit.JsResult;
import android.webkit.WebChromeClient;
import android.webkit.WebView;


public class WtAndroid extends Activity
{
  public static int startWt(Activity activity)
    {
        Log.d("WtAndroid::onCreate", "Extracting wt-assets.zip ...");

        String wtAssetsDir 
	  = activity.getFilesDir().getAbsolutePath() + "/wt-assets/";
    	try {
	  copyWtAssets(wtAssetsDir, activity.getAssets());
	} catch (IOException e) {
	  e.printStackTrace();
	}
	
	Log.d("WtAndroid::onCreate", "Finished extracting wt-assets.zip");
		
        List<String> args = new ArrayList<String>();
        args.add("app");
        
        args.add("--docroot");
        args.add(wtAssetsDir);
        args.add("--approot");
        args.add(wtAssetsDir);
        
        args.add("--http-address");
        args.add("127.0.0.1");
        args.add("--http-port");
        args.add("0");

	File tmpDir 
	  = new File(activity.getFilesDir().getAbsolutePath() + "/tmp");
	tmpDir.mkdir();
	args.add("-DWT_TMP_DIR=" + tmpDir.getAbsolutePath());
        
        Log.d("WtAndroid::onCreate", "Starting wt application ...");
        String[] argv = new String[args.size()];
        args.toArray(argv);
    	int httpPort = startwt(argv);
    	Log.d("WtAndroid::onCreate", 
	      "Started wt application on http-port " + httpPort);
	
	return httpPort;
    }
    
  private static void copyWtAssets(String wtAssetsDir, 
				   AssetManager am) throws IOException {
      if (!new File(wtAssetsDir).exists()) {
	new File(wtAssetsDir).mkdir();
    	
    	BufferedInputStream bis 
	  = new BufferedInputStream(am.open("wt-assets.zip"));
	ZipInputStream zis = new ZipInputStream(bis);
    		 try {
    			 byte[] buffer = new byte[1024];
    			 int count;

    		     ZipEntry ze;

    		     while ((ze = zis.getNextEntry()) != null) {
    		    	 String file = wtAssetsDir + ze.getName();
    		    	 if (ze.isDirectory()) {
    		    		 new File(file).mkdirs();
    		    	 } else {
    		    		 FileOutputStream fos 
				   = new FileOutputStream(file);
    		    		 while ((count = zis.read(buffer)) != -1) {
    		    			 fos.write(buffer, 0, count);
    		    		 }
    		    		 fos.close();
    		    	 }
    		     }
    		 } finally {
    		     zis.close();
    		 }
    	}
    }

    private static native int startwt(String[] argv);
    
    static {
        System.loadLibrary("wt-jni");
    }
}
