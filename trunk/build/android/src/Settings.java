// This string is autogenerated by ChangeAppSettings.sh, do not change spaces amount
package org.wormux;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.KeyEvent;
import android.view.Window;
import android.view.WindowManager;
import android.widget.TextView;
import android.util.Log;
import java.io.*;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.res.Configuration;
import android.os.Environment;
import android.os.StatFs;
import java.util.Locale;

class Settings
{
  static String SettingsFileName = "libsdl-settings.cfg";
  static void Load( final MainActivity p )
  {
    try {
      ObjectInputStream settingsFile = new ObjectInputStream(new FileInputStream( p.getFilesDir().getAbsolutePath() + "/" + SettingsFileName ));
      Globals.DownloadToSdcard = settingsFile.readBoolean();
      Globals.AppNeedsArrowKeys = settingsFile.readBoolean();
      Globals.PhoneHasTrackball = settingsFile.readBoolean();

      startDownloader(p);
      return;
    } catch( FileNotFoundException e ) {
    } catch( SecurityException e ) {
    } catch ( IOException e ) {};

    // This code fails for both of my phones!
    /*
    Configuration c = new Configuration();
    c.setToDefaults();

    if( c.navigation == Configuration.NAVIGATION_TRACKBALL ||
      c.navigation == Configuration.NAVIGATION_DPAD ||
      c.navigation == Configuration.NAVIGATION_WHEEL )
    {
      Globals.AppNeedsArrowKeys = false;
    }

    System.out.println( "libSDL: Phone keypad type: " +
        (
        c.navigation == Configuration.NAVIGATION_TRACKBALL ? "Trackball" :
        c.navigation == Configuration.NAVIGATION_DPAD ? "Dpad" :
        c.navigation == Configuration.NAVIGATION_WHEEL ? "Wheel" :
        c.navigation == Configuration.NAVIGATION_NONAV ? "None" :
        "Unknown" ) );
    */

    long freeSdcard = 0;
    long freePhone = 0;
    try {
      StatFs sdcard = new StatFs(Environment.getExternalStorageDirectory().getPath());
      StatFs phone = new StatFs(Environment.getDataDirectory().getPath());
      freeSdcard = (long)sdcard.getAvailableBlocks() * sdcard.getBlockSize() / 1024 / 1024;
      freePhone = (long)phone.getAvailableBlocks() * phone.getBlockSize() / 1024 / 1024;
    }catch(Exception e) {}

    final CharSequence[] items = {"Phone storage - " + String.valueOf(freePhone) + " Mb free", "SD card - " + String.valueOf(freeSdcard) + " Mb free"};

    AlertDialog.Builder builder = new AlertDialog.Builder(p);
    builder.setTitle("Where to download application data (42MB)");
    builder.setSingleChoiceItems(items, -1, new DialogInterface.OnClickListener()
    {
      public void onClick(DialogInterface dialog, int item)
      {
        Globals.DownloadToSdcard = (item == 1);

        dialog.dismiss();
        showKeyboardConfig(p);
      }
    });
    AlertDialog alert = builder.create();
    alert.setOwnerActivity(p);
    alert.show();

  };

  static void showKeyboardConfig(final MainActivity p)
  {
    if( ! Globals.AppNeedsArrowKeys )
    {
      Save(p);
      startDownloader(p);
      return;
    }

    final CharSequence[] items = {"Arrows / joystick / dpad", "Trackball", "None, only touchscreen"};

    AlertDialog.Builder builder = new AlertDialog.Builder(p);
    builder.setTitle("What kind of navigation keys does your phone have?");
    builder.setSingleChoiceItems(items, -1, new DialogInterface.OnClickListener()
    {
      public void onClick(DialogInterface dialog, int item)
      {
        Globals.AppNeedsArrowKeys = (item == 2);
        Globals.PhoneHasTrackball = (item == 1);

        dialog.dismiss();
        showAccelermoeterConfig(p);
      }
    });
    AlertDialog alert = builder.create();
    alert.setOwnerActivity(p);
    alert.show();
  }

  static void showAccelermoeterConfig(final MainActivity p)
  {
    if( Globals.AppNeedsArrowKeys || Globals.AppUsesJoystick )
    {
      Save(p);
      startDownloader(p);
      return;
    }

    final CharSequence[] items = {"Do not use accelerometer", "Use accelerometer as navigation keys"};

    AlertDialog.Builder builder = new AlertDialog.Builder(p);
    builder.setTitle("You may optionally use accelerometer as another navigation keys");
    builder.setSingleChoiceItems(items, -1, new DialogInterface.OnClickListener()
    {
      public void onClick(DialogInterface dialog, int item)
      {
        Globals.AppNeedsArrowKeys = (item == 1);

        Save(p);
        dialog.dismiss();
        startDownloader(p);
      }
    });
    AlertDialog alert = builder.create();
    alert.setOwnerActivity(p);
    alert.show();
  }

  static void Save(final MainActivity p)
  {
    try {
      ObjectOutputStream out = new ObjectOutputStream(p.openFileOutput( SettingsFileName, p.MODE_WORLD_READABLE ));
      out.writeBoolean(Globals.DownloadToSdcard);
      out.writeBoolean(Globals.AppNeedsArrowKeys);
      out.writeBoolean(Globals.PhoneHasTrackball);
      out.close();
    } catch( FileNotFoundException e ) {
    } catch( SecurityException e ) {
    } catch ( IOException e ) {};
  }


  static void Apply()
  {
    nativeIsSdcardUsed( Globals.DownloadToSdcard ? 1 : 0 );

    if( Globals.PhoneHasTrackball )
      nativeSetTrackballUsed();
    if( Globals.AppUsesMouse )
      nativeSetMouseUsed();
    if( Globals.AppUsesJoystick && !Globals.AppNeedsArrowKeys )
      nativeSetJoystickUsed();
    if( Globals.AppUsesMultitouch )
      nativeSetMultitouchUsed();
    String lang = new String(Locale.getDefault().getLanguage());
    //if( Locale.getDefault().getCountry().length() > 0 )
    //  lang = lang + "_" + Locale.getDefault().getCountry();
    System.out.println( "libSDL: setting envvar LANG to '" + lang + "'");
    nativeSetEnv( "LANG", lang );
    // TODO: get current user name and set envvar USER, the API is not available on Android 1.6 so I don't bother with this
  }

  static void startDownloader(MainActivity p)
  {
    class Callback implements Runnable
    {
      public MainActivity Parent;
      public void run()
      {
        Parent.startDownloader();
      }
    }
    Callback cb = new Callback();
    cb.Parent = p;
    p.runOnUiThread(cb);
  };


  private static native int nativeIsSdcardUsed(int flag);
  private static native int nativeSetTrackballUsed();
  private static native int nativeSetMouseUsed();
  private static native int nativeSetJoystickUsed();
  private static native int nativeSetMultitouchUsed();
  public static native void nativeSetEnv(final String name, final String value);
}

